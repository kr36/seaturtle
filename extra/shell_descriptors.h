// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_EXTRA_SHELL_DESCRIPTORS_H_
#define SEATURTLE_EXTRA_SHELL_DESCRIPTORS_H_

#include "content/public/common/content_descriptors.h"

// This is a list of global descriptor keys to be used with the
// base::GlobalDescriptors object (see base/posix/global_descriptors.h)
enum {
  kSeaturtlePakDescriptor = kContentIPCDescriptorMax + 1,
  kSeaturtleBlockingDataDescriptor,
};

#endif  // SEATURTLE_EXTRA_SHELL_DESCRIPTORS_H_
