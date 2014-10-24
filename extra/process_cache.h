// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_EXTRA_PROCESS_CACHE_H_
#define SEATURTLE_EXTRA_PROCESS_CACHE_H_

#include "base/macros.h"
#include <string>

namespace seaturtle {

// TODO Wait this is dumb. What would be smarter is to do this in jni_bridge.
// You can just hold on the the response protos somewhere.
class ProcessCache {
 public:
  static ProcessCache* Singleton();

  ProcessCache() {};
  virtual ~ProcessCache() {};
  std::string GetAcceptLangs();
  std::string GetApplicationLocale();

 private:

  std::string locale_;
  std::string accept_langs_;

  DISALLOW_COPY_AND_ASSIGN(ProcessCache);
};


}  // namespace seaturtle

#endif  // SEATURTLE_EXTRA_PROCESS_CACHE_H_
