// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/app/android/library_loader_hooks.h"

#include "base/command_line.h"
#include "content/public/app/android_library_loader_hooks.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/jni/jni_bridge.h"

namespace seaturtle {

bool LibraryLoaded(JNIEnv* env, jclass clazz) {
  if (!content::LibraryLoaded(env, clazz)) {
    return false;
  }
  seaturtle::jni::Initialize();
  CommandLine* cl = CommandLine::ForCurrentProcess();
  STLOG() << "seaturtle command line: " << cl->GetCommandLineString();
  return true;
}

}  // namespace seaturtle
