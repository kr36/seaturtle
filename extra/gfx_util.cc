// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/extra/gfx_util.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/protos/jni.pb.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace seaturtle {

using jni::Image;

bool SkBitmapToProto(const SkBitmap& bitmap, Image* proto) {
  if (bitmap.isNull()) {
    return false;
  }
  jni::Image_Config config;
  switch (bitmap.colorType()) {
    case kAlpha_8_SkColorType:
      config = Image::ALPHA_8;
      break;  
    case kARGB_4444_SkColorType:
      config = Image::ARGB_4444;
      break;  
    case kN32_SkColorType:
      config = Image::ARGB_8888;
      break;  
    case kRGB_565_SkColorType:
      config = Image::RGB_565;
      break;  
    case kUnknown_SkColorType:
    default:
      STLOG() << "invalid color type: " << bitmap.colorType();
      return false;
  }
  proto->set_config(config);
  proto->set_height(bitmap.height());
  proto->set_width(bitmap.width());
  proto->mutable_pixels()->assign(
      (char*) bitmap.getPixels(), bitmap.getSafeSize());
  return true;
}

}  // namespace seaturtle
