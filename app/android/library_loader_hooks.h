// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_APP_ANDROID_LIBRARY_LOADER_HOOKS_H_
#define SEATURTLE_APP_ANDROID_LIBRARY_LOADER_HOOKS_H_

#include <jni.h>

#include "base/basictypes.h"
#include "content/common/content_export.h"

namespace seaturtle {

bool LibraryLoaded(JNIEnv* env, jclass clazz);

}  // namespace seaturtle

#endif  // SEATURTLE_APP_ANDROID_LIBRARY_LOADER_HOOKS_H_
