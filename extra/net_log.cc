// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/extra/net_log.h"

namespace seaturtle {

NetLog::NetLog() {}
NetLog::~NetLog() {}

// static
NetLog* NetLog::Get() {
  return NULL;
}

}  // namespace seaturtle

