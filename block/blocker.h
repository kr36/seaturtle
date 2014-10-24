// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_BLOCK_BLOCKER_H_

#define SEATURTLE_BLOCK_BLOCKER_H_

#include <string>
#include <vector>

#include "base/lazy_instance.h"
#include "base/memory/linked_ptr.h"
#include "base/strings/string_piece.h"
#include "base/containers/hash_tables.h"

#include "seaturtle/protos/jni.pb.h"

// Move this into extra

class GURL;

namespace base {
class File;
class FileDescriptor;
}

namespace blink {
class WebFrame;
class WebString;
}

namespace content {
class RenderProcessHost;
}

namespace net {
class URLRequest;
}

namespace seaturtle {

namespace jni {
class UpdateBlockingWhitelist;
}

namespace blocking {
class RuleList;
}

class Blocker {
 public:
  static Blocker* Singleton();

  // Only used by browser process
  void UpdateWhitelist(const jni::UpdateBlockingWhitelist& ubw);
  void UpdateRulesWithPath(const std::string& path);
  bool ShouldBlockThirdPartyRequest(const GURL& url) const;
  bool ShouldClearReferrer() const;
  base::FileDescriptor GetBlockingDataFd() const;

  // Only used by render process
  void UpdateRulesWithFd(int fd);
  void InjectStyleSheets(blink::WebFrame* frame) const;
  bool IsWhitelisted(const GURL& url) const;

 private:
  friend struct base::DefaultLazyInstanceTraits<Blocker>;
  Blocker();
  void UpdateRules(base::File& f, bool is_browser_process);

  // Only used by browser process
  bool IsHostEvilThirdParty(const base::StringPiece& host) const;
  void UpdateRulesForBrowserProcess(const blocking::RuleList& rl);
  std::string file_path_;
  // The linked pointer is used to keep the string data in memory,
  // the string piece is a reference to that memory.
  base::hash_map<base::StringPiece, linked_ptr<std::string> > domain_blacklist_;

  // Only used by render process
  void UpdateRulesForRenderProcess(const blocking::RuleList& rl);
  std::vector<blink::WebString> global_stylesheets_;

  // The linked pointer is used to keep the string data in memory,
  // the string piece is a reference to that memory.
  typedef std::pair<linked_ptr<std::string>, std::string> DomainStylesheetValue;
  typedef base::hash_map<base::StringPiece, DomainStylesheetValue>
    DomainStylesheetMap;
  DomainStylesheetMap domain_stylesheets_;
  typedef std::vector<std::string> WhitelistPaths;
  typedef base::hash_map<std::string, WhitelistPaths > WhitelistMap;
  WhitelistMap whitelist_;

  DISALLOW_COPY_AND_ASSIGN(Blocker);
};

}  // namespace seaturtle

#endif  // SEATURTLE_BLOCK_BLOCKER_H_
