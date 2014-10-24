// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_EXTRA_FILE_UTIL_H_

#define SEATURTLE_EXTRA_FILE_UTIL_H_

#include <string>

namespace base {
  class File;
}

namespace seaturtle {

bool ReadFileToString(base::File& file, std::string* contents);

}  // namespace seaturtle

#endif // SEATURTLE_EXTRA_FILE_UTIL_H_
