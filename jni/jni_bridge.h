// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_JNI_JNI_BRIDGE_H_
#define SEATURTLE_JNI_JNI_BRIDGE_H_

// For convience.
#include "seaturtle/protos/jni.pb.h"

namespace seaturtle {
namespace jni {

void Initialize();
void Invoke(const Params& req, Params* resp = NULL);

}  // namespace jni
}  // namespace seaturtle

#endif  // SEATURTLE_JNI_JNI_BRIDGE_H_
