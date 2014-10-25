// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/extra/process_cache.h"

#include "base/lazy_instance.h"

#include "seaturtle/jni/jni_bridge.h"

using seaturtle::jni::Params;

namespace seaturtle {

namespace {
  base::LazyInstance<ProcessCache>  g_singleton = LAZY_INSTANCE_INITIALIZER;
}

// static
ProcessCache* ProcessCache::Singleton() {
  return g_singleton.Pointer();
}

std::string ProcessCache::GetApplicationLocale() {
  // TODO(cy) make thread safe
  if (locale_.empty()) {
    Params req;
    Params resp;
    req.set_type(Params::LOCALE);
    jni::Invoke(req, &resp);
    locale_ = resp.locale();
  }
  return locale_;
}

std::string ProcessCache::GetAcceptLangs() {
  // TODO(cy) make thread safe
  if (accept_langs_.empty()) {
    Params req;
    Params resp;
    req.set_type(Params::ACCEPT_LANGS);
    jni::Invoke(req, &resp);
    accept_langs_ = resp.accept_langs();
  }
  return accept_langs_;
}

}  // namespace seaturtle
