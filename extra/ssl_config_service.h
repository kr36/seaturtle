// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_EXTRA_SSL_CONFIG_SERVICE_H_
#define SEATURTLE_EXTRA_SSL_CONFIG_SERVICE_H_

#include "net/base/net_export.h"
#include "net/ssl/ssl_config_service.h"

namespace seaturtle {

class SSLConfigService : public net::SSLConfigService {
 public:
  SSLConfigService();

  // Store default SSL config settings in |config|.
  virtual void GetSSLConfig(net::SSLConfig* config) OVERRIDE;

 private:
  virtual ~SSLConfigService();

  net::SSLConfig static_config_;

  DISALLOW_COPY_AND_ASSIGN(SSLConfigService);
};

}  // namespace seaturtle

#endif  // SEATURTLE_EXTRA_SSL_CONFIG_SERVICE_H_
