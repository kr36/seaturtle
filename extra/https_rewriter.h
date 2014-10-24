// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_EXTRA_HTTPS_REWRITER_H_

#define SEATURTLE_EXTRA_HTTPS_REWRITER_H_

#include <string>
#include <vector>

#include "url/gurl.h"
#include "base/lazy_instance.h"
#include "base/strings/string_piece.h"
#include "base/memory/linked_ptr.h"
#include "base/containers/hash_tables.h"
#include "third_party/re2/re2/re2.h"

namespace net {
class URLRequest;
}

namespace seaturtle {

namespace https {
class RuleList;
class RuleSet;
}

class HttpsRewriter {
 public:
  static HttpsRewriter* Get();

  void UpdateRulesWithPath(const std::string& path);
  int MaybeRewrite(net::URLRequest* request, GURL* new_url) const;

 private:
  bool MaybeRewriteForSuperHost(base::StringPiece host,
      net::URLRequest* request, GURL* new_url) const;

  typedef std::pair<re2::RE2*, std::string*> RewriteRule;
  class CompiledRuleSet {
   public:
    explicit CompiledRuleSet(const https::RuleSet& rs);
    bool empty() const;
    bool MaybeRewrite(net::URLRequest* request, GURL* new_url) const;
    ~CompiledRuleSet();
    std::vector<RewriteRule> rules_;
    std::vector<std::string*> hosts_;
   private:
    DISALLOW_COPY_AND_ASSIGN(CompiledRuleSet);
  };

  friend struct base::DefaultLazyInstanceTraits<HttpsRewriter>;
  HttpsRewriter();
  void UpdateRules(const https::RuleList& rl);

  typedef base::hash_map<base::StringPiece, linked_ptr<CompiledRuleSet> >
    HostToRules;
  HostToRules exact_host_;
  HostToRules super_host_;

  DISALLOW_COPY_AND_ASSIGN(HttpsRewriter);
};

}  // namespace seaturtle

#endif  // SEATURTLE_EXTRA_HTTPS_REWRITER_H_
