// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_EXTRA_GFX_UTIL_H_

#define SEATURTLE_EXTRA_GFX_UTIL_H_

class SkBitmap;

namespace jni {
class Image;
}

namespace seaturtle {

bool SkBitmapToProto(const SkBitmap& bitmap, jni::Image* proto);

}  // namespace seaturtle

#endif  // SEATURTLE_EXTRA_GFX_UTIL_H_
