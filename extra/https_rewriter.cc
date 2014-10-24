// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/extra/https_rewriter.h"

// TODO use c++11 regex, it supports ECMAscript syntax.
//#include <regex>

#include "base/file_util.h"
#include "base/stl_util.h"
#include "base/strings/string_util.h"
#include "net/url_request/url_request.h"
#include "net/base/net_errors.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/extra/file_util.h"
#include "seaturtle/extra/settings.h"
#include "seaturtle/protos/https.pb.h"

using base::FilePath;
using seaturtle::https::RuleList;
using seaturtle::https::RuleSet;
using seaturtle::https::Rule;
using seaturtle::settings::HttpsRewrite;

namespace seaturtle {

namespace {

base::LazyInstance<HttpsRewriter> g_singleton = LAZY_INSTANCE_INITIALIZER;

const char* kUserDataKey = "seaturtle-https-rewrite";

class RewriteUserData : public base::SupportsUserData::Data {
  public:
    virtual ~RewriteUserData() {}
    // TODO(ckatrak) switch to vector<>
    base::hash_set<std::string> prev_rewrites_;
};

void DoRewrite(const std::string& rewrite,
    net::URLRequest* request, GURL *new_url) {
  STDCHECK(new_url != NULL);
  *new_url = GURL(rewrite);
  STLOG() << "rewrote: " << request->url().spec() << " to " << rewrite;
  STDCHECK(new_url->is_valid());
  base::SupportsUserData::Data* ud = request->GetUserData(kUserDataKey);
  RewriteUserData* rud;
  // Note we have to do the static_cast later. It looks like static_casting NULL
  // on android devices "succeeds" and produces a non-NULL result!
  if (ud != NULL) {
    rud = static_cast<RewriteUserData*>(ud);
  } else {
    rud = new RewriteUserData();
    request->SetUserData(kUserDataKey, rud);
  }
  rud->prev_rewrites_.insert(request->url().spec());
  // TODO(ckatrak) log an event
}

}  // namespace

// static
HttpsRewriter* HttpsRewriter::Get() {
  return g_singleton.Pointer();
}

HttpsRewriter::HttpsRewriter() {}

void HttpsRewriter::UpdateRulesWithPath(const std::string& path) {
  STLOG() << "updating rules: " << path;
  RuleList rl;
  base::File f;
  f.Initialize(FilePath::FromUTF8Unsafe(path),
      base::File::FLAG_OPEN | base::File::FLAG_READ);
  CHECK(f.IsValid());
  {
    std::string raw;
    CHECK(ReadFileToString(f, &raw));
    if (raw.empty()) {
      STLOG() << "got empty rules file!";
    } else {
      CHECK(rl.ParseFromString(raw));
    }
  }
  STLOG() << "got " << rl.rule_set_size() << " rule sets";
  UpdateRules(rl);
}

void HttpsRewriter::UpdateRules(const https::RuleList& rl) {
  exact_host_.clear();
  super_host_.clear();
  for (int i = 0; i < rl.rule_set_size(); i++) {
    const RuleSet& rs = rl.rule_set(i);
    linked_ptr<CompiledRuleSet> crs(new CompiledRuleSet(rs));
    if (crs->empty()) {
      STNOTREACHED() << "empty https ruleset";
      continue;
    }
    for (int j = 0; j < rs.host_size(); j++) {
      const std::string& host = rs.host(j);
      size_t pos = host.find('*');
      if (pos == 0) {
        // TODO(ckatrak) sort this out in the dat preprocessor
        std::string* super = new std::string(host, 2);
        // STLOG() << "adding ruleset for super host: " << *super;
        crs->hosts_.push_back(super);
        super_host_[base::StringPiece(*super)] = crs;
      } else if (pos != std::string::npos) {
        STNOTREACHED() << "skipping host: " << host;
        continue;
      } else {
        std::string* exact = new std::string(host);
        // STLOG() << "adding ruleset for exact host: " << exact;
        crs->hosts_.push_back(exact);
        exact_host_[base::StringPiece(*exact)] = crs;
      }
    }
  }
}

HttpsRewriter::CompiledRuleSet::CompiledRuleSet(const RuleSet& rs) {
  for (int i = 0; i < rs.rule_size(); i++) {
    const Rule& r = rs.rule(i);
    // STLOG() << "compiling rule from: " << r.from() << " to: " << r.to();
    RewriteRule rr(new re2::RE2(r.from()),
        new std::string(r.to()));
    CHECK(rr.first->ok()) << rr.first->error();
    std::string error;
    // TODO(ckatrak) golang doesn't have a great way to handle this.
    // Keep an eye on these.
    if (!rr.first->CheckRewriteString(*(rr.second), &error)) {
      STLOG() << "bad rewrite: " << error << " from: "<< r.from() << " to: " << r.to();
    }
    rules_.push_back(rr);
  }
}

bool HttpsRewriter::CompiledRuleSet::MaybeRewrite(
    net::URLRequest* request, GURL* new_url) const {
  const GURL& old_url = request->url();
  STDCHECK(!old_url.SchemeIs("https"));
  std::string rewrite = old_url.spec();
  for (unsigned int i = 0; i < rules_.size(); i++) {
    const RewriteRule& rule = rules_[i];
    if (re2::RE2::Replace(&rewrite, *rule.first, *rule.second)) {
      DoRewrite(rewrite, request, new_url);
      return true;
    }
  }
  // STLOG() << "no rewrite match for url: " << old_url.spec();
  return false;
}

int HttpsRewriter::MaybeRewrite(
    net::URLRequest* request, GURL* new_url) const {
  ST_SETTINGS(all_settings);
  const HttpsRewrite& settings = all_settings->https_rewrite();
  if (!settings.enabled()) {
    return net::OK;
  }
  const GURL& old_url = request->url();
  if (!old_url.SchemeIs("http")) {
    return net::OK;
  }

  base::SupportsUserData::Data* ud = request->GetUserData(kUserDataKey);
  if (ud != NULL) {
    // See note above about static casts.
    RewriteUserData* rud = static_cast<RewriteUserData*>(ud);
    if (ContainsKey(rud->prev_rewrites_, old_url.spec())) {
      STLOG() << "skipping rewrite for previously rewritten url: " <<
        old_url;
      if (settings.https_only()) {
        return net::ERR_UNSAFE_REDIRECT;
      }
      return net::OK;
    }
  }
  bool rewrote = false;
  std::string upper_host = old_url.host();
  base::StringToLowerASCII(&upper_host);
  base::StringPiece host(upper_host);
  HostToRules::const_iterator it = exact_host_.find(host);
  if (it != exact_host_.end()) {
    rewrote = it->second->MaybeRewrite(request, new_url);
  }
  if (!rewrote) {
    rewrote = MaybeRewriteForSuperHost(host, request, new_url);
  }
  if (settings.https_only() && !rewrote) {
    STLOG() << "forcing rewrite of http url to https: " << old_url;
    std::string rewrite("https");
    rewrite.append(old_url.spec(), 4, std::string::npos);
    DoRewrite(rewrite, request, new_url);
  }
  return net::OK;
}

bool HttpsRewriter::MaybeRewriteForSuperHost(
    base::StringPiece host, net::URLRequest* request, GURL* new_url) const {
  const GURL& old_url = request->url();
  STDCHECK(!old_url.SchemeIs("https"));
  while (true) {
    // We do this to avoid checking some TLDs.
    size_t pos = host.find('.');
    if (pos == base::StringPiece::npos) {
      break;
    }
    HostToRules::const_iterator it = super_host_.find(host);
    if (it != super_host_.end()) {
      if (it->second->MaybeRewrite(request, new_url)) {
        return true;
      }
    }
    // else we could stop checking here, but lets keep going
    pos++;
    if (pos >= host.size()) {
      break;
    }
    host.remove_prefix(pos);
  }
  return false;
}

bool HttpsRewriter::CompiledRuleSet::empty() const {
  return rules_.empty();
}

HttpsRewriter::CompiledRuleSet::~CompiledRuleSet() {
  STLDeleteContainerPairPointers(rules_.begin(), rules_.end());
  STLDeleteContainerPointers(hosts_.begin(), hosts_.end());
}

}  // namespace seaturtle
