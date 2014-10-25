// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/extra/file_util.h"

#include "base/files/file.h"
#include "seaturtle/extra/base.h"

namespace seaturtle {

bool ReadFileToString(base::File* file,
                      std::string* contents) {
  STDCHECK(contents);
  char buf[1 << 16];
  size_t len;
  while ((len = file->ReadAtCurrentPos(buf, sizeof(buf))) > 0) {
    contents->append(buf, len);
  }
  file->Close();
  return len >= 0;
}

}  // namespace seaturtle
