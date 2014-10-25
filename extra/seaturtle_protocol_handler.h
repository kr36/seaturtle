// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_EXTRA_SEATURTLE_PROTOCOL_HANDLER_H_
#define SEATURTLE_EXTRA_SEATURTLE_PROTOCOL_HANDLER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "net/url_request/url_request_job_factory.h"

namespace seaturtle {

// Implements a ProtocolHandler for Seaturtle jobs.
class SeaturtleProtocolHandler :
    public net::URLRequestJobFactory::ProtocolHandler {
 public:
  SeaturtleProtocolHandler();
  virtual net::URLRequestJob* MaybeCreateJob(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) const OVERRIDE;
  virtual bool IsSafeRedirectTarget(const GURL& location) const OVERRIDE;
 private:
  DISALLOW_COPY_AND_ASSIGN(SeaturtleProtocolHandler);
};

}  // namespace seaturtle

#endif  // SEATURTLE_EXTRA_SEATURTLE_PROTOCOL_HANDLER_H_
