// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_EXTRA_NET_LOG_H_
#define SEATURTLE_EXTRA_NET_LOG_H_

#include "net/base/net_log.h"

namespace seaturtle {

class NetLog : public net::NetLog {
 public:
  static NetLog* Get();

  NetLog();
  virtual ~NetLog();

 private:
  DISALLOW_COPY_AND_ASSIGN(NetLog);
};


}  // namespace seaturtle

#endif  // SEATURTLE_EXTRA_NET_LOG_H_
