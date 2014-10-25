// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/block/blocker.h"

#include "base/logging.h"
#include "base/lazy_instance.h"
#include "base/stl_util.h"
#include "base/file_util.h"
#include "base/strings/string_util.h"
#include "net/url_request/url_request.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_frame_host.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebScriptSource.h"
#include "url/gurl.h"
#include "url/url_util.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/extra/file_util.h"
#include "seaturtle/extra/shell_messages.h"
#include "seaturtle/extra/settings.h"
#include "seaturtle/shell/browser/shell.h"
#include "seaturtle/protos/blocking.pb.h"
#include "seaturtle/protos/jni.pb.h"
#include "seaturtle/extra/constants.h"

using base::FilePath;

using content::RenderFrameHost;
using content::RenderProcessHost;
using content::ResourceRequestInfo;

namespace seaturtle {

using jni::UpdateBlockingWhitelist;
using blocking::RuleList;
using settings::Filters;

namespace {

base::LazyInstance<Blocker> g_singleton = LAZY_INSTANCE_INITIALIZER;

const char* kDisplayNone = " {display:none !important;}";
const int kMaxSelectorsPerStylesheet = 4096;

}  // namespace

// static
Blocker* Blocker::Singleton() {
  return g_singleton.Pointer();
}

Blocker::Blocker() {}

void Blocker::UpdateWhitelist(const jni::UpdateBlockingWhitelist& ubw) {
  whitelist_.clear();
  for (int i = 0; i < ubw.whitelisted_host_size(); i++) {
    const jni::WhitelistedHost& wh = ubw.whitelisted_host(i);
    WhitelistPaths paths;
    for (int j = 0; j < wh.path_size(); j++) {
      std::string path(wh.path(j));
      base::StringToLowerASCII(&path);
      paths.push_back(path);
    }

    std::string host = wh.host();
    base::StringToLowerASCII(&host);
    WhitelistMap::iterator it = whitelist_.find(host);
    if (it == whitelist_.end()) {
      whitelist_[host] = paths;
    } else {
      WhitelistPaths& current_paths = it->second;
      current_paths.insert(current_paths.end(), paths.begin(), paths.end());
    }
  }
}

bool Blocker::IsWhitelisted(const GURL& url) const {
  if (url.SchemeIs(kKryptonScheme)) {
    return true;
  }
  std::string host = url.host();
  base::StringToLowerASCII(&host);
  WhitelistMap::const_iterator it = whitelist_.find(host);
  if (it == whitelist_.end()) {
    return false;
  }
  const WhitelistPaths& paths = it->second;
  if (paths.empty()) {
    return true;
  }
  // TODO(cy) paths aren't case sensitive, do we care?
  std::string path = url.path();
  base::StringToLowerASCII(&path);
  return std::find(paths.begin(), paths.end(), path) != paths.end();
}

// Browser process functions.
base::FileDescriptor Blocker::GetBlockingDataFd() const {
  STDCHECK(!file_path_.empty());
  base::File file;
  file.Initialize(FilePath::FromUTF8Unsafe(file_path_),
      base::File::FLAG_OPEN | base::File::FLAG_READ);
  CHECK(file.IsValid());
  return base::FileDescriptor(file.Pass());
}

void Blocker::UpdateRulesWithPath(const std::string& path) {
  file_path_ = path;
  base::File file;
  file.Initialize(FilePath::FromUTF8Unsafe(path),
      base::File::FLAG_OPEN | base::File::FLAG_READ);
  CHECK(file.IsValid());
  UpdateRules(&file, true);
}

void Blocker::UpdateRulesWithFd(int fd) {
  base::File file(fd);
  STDCHECK(file.IsValid());
  UpdateRules(&file, false);
}

void Blocker::UpdateRules(base::File* f, bool is_browser_process) {
  RuleList rl;
  {
    std::string raw_rules;
    CHECK(ReadFileToString(f, &raw_rules));
    if (raw_rules.empty()) {
      STLOG() << "empty rules file!";
    } else {
      CHECK(rl.ParseFromString(raw_rules));
    }
  }
  if (is_browser_process) {
    UpdateRulesForBrowserProcess(rl);
  } else {
    UpdateRulesForRenderProcess(rl);
  }
}

void Blocker::UpdateRulesForBrowserProcess(const RuleList& rl) {
  domain_blacklist_.clear();
  for (int i = 0; i < rl.rule_size(); i++) {
    const blocking::Rule& rule = rl.rule(i);
    if (rule.block_third_party()) {
      // STLOG() << "adding evil third party: '" << rule.domain() << "'";
      // The keys are StringPieces, we need to keep the actual string in memory
      // somewhere else.
      linked_ptr<std::string> host(new std::string(rule.domain()));
      domain_blacklist_[base::StringPiece(*host.get())] = host;
    }
  }
  STLOG() << "browser rules update;"
    << " total: " << rl.rule_size()
    << " blacklisted domains: " << domain_blacklist_.size();
  // Shell::UpdateAllRenderers();
}

bool Blocker::IsHostEvilThirdParty(const base::StringPiece& host) const {
  if (ContainsKey(domain_blacklist_, host)) {
    return true;
  }
  return false;
}

bool Blocker::ShouldBlockThirdPartyRequest(const GURL& url) const {
  {
    ST_SETTINGS(s);
    if (!s->filters().blacklist_third_parties()) {
      return false;
    }
  }
  std::string upper_host = url.host();
  base::StringToLowerASCII(&upper_host);
  base::StringPiece host(upper_host);
  while (true) {
    // We do this to avoid checking some TLDs.
    size_t pos = host.find('.');
    if (pos == base::StringPiece::npos) {
      break;
    }
    if (IsHostEvilThirdParty(host)) {
      return true;
    }
    pos++;
    if (pos >= host.size()) {
      break;
    }
    host.remove_prefix(pos);
  }
  return false;
}

bool Blocker::ShouldClearReferrer() const {
  ST_SETTINGS(s);
  return s->filters().clear_referrer();
}

// Render process functions.
void Blocker::UpdateRulesForRenderProcess(const RuleList& rl) {
  global_stylesheets_.clear();
  domain_stylesheets_.clear();
  int num_global_selectors = 0;
  uint32 stylesheet_bytes = 0;
  std::string stylesheet;
  for (int i = 0; i < rl.rule_size(); i++) {
    const blocking::Rule& rule = rl.rule(i);
    if (rule.selector().empty()) {
      continue;
    }
    if (rule.domain().empty()) {
      // global stylesheet
      if (!stylesheet.empty()) {
        stylesheet.append(",");
      }
      stylesheet.append(rule.selector());
      num_global_selectors++;
      if (num_global_selectors % kMaxSelectorsPerStylesheet == 0) {
        stylesheet.append(kDisplayNone);
        global_stylesheets_.push_back(blink::WebString::fromUTF8(stylesheet));
        stylesheet_bytes += stylesheet.size();
        stylesheet.clear();
      }
    } else {
      // domain stylesheet
      DomainStylesheetMap::iterator it =
        domain_stylesheets_.find(rule.domain());
      if (it == domain_stylesheets_.end()) {
        linked_ptr<std::string> domain(new std::string(rule.domain()));

        domain_stylesheets_[base::StringPiece(*domain.get())] =
          std::make_pair(domain, rule.selector());
      } else {
        (*it).second.second.append(",");
        (*it).second.second.append(rule.selector());
      }
    }
  }
  if (!stylesheet.empty()) {
    stylesheet.append(kDisplayNone);
    global_stylesheets_.push_back(blink::WebString::fromUTF8(stylesheet));
    stylesheet_bytes += stylesheet.size();
  }
  STLOG() << "render rules update;"
    << " global selectors: " << num_global_selectors
    << " global stylesheets: " << global_stylesheets_.size()
    << " global stylesheet bytes: " << stylesheet_bytes
    << " domain stylesheets: " << domain_stylesheets_.size();
}

void Blocker::InjectStyleSheets(blink::WebFrame* frame) const {
  STLOG() << "injecting stylesheets";
  // Insert globals.
  for (size_t i = 0; i < global_stylesheets_.size(); i++) {
    frame->document().insertStyleSheet(global_stylesheets_[i]);
  }
  // Build a domain specific stylesheet.
  std::string stylesheet;
  std::string upper_host = GURL(frame->document().url()).host();
  base::StringToLowerASCII(&upper_host);
  base::StringPiece host(upper_host);
  while (true) {
    // We do this to avoid checking some TLDs.
    size_t pos = host.find('.');
    if (pos == base::StringPiece::npos) {
      break;
    }
    DomainStylesheetMap::const_iterator it =
      domain_stylesheets_.find(host);
    if (it != domain_stylesheets_.end()) {
      if (!stylesheet.empty()) {
        stylesheet.append(",");
      }
      stylesheet.append((*it).second.second);
    }
    pos++;
    if (pos >= host.size()) {
      break;
    }
    host.remove_prefix(pos);
  }
  if (!stylesheet.empty()) {
    stylesheet.append(kDisplayNone);
    STLOG() << "injecting domain specific stylesheet: " << stylesheet;
    frame->document().insertStyleSheet(blink::WebString::fromUTF8(stylesheet));
  }
}

}  // namespace seaturtle
