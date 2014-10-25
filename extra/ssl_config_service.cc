// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/extra/ssl_config_service.h"

#include <algorithm>

#include "seaturtle/extra/base.h"
#include "seaturtle/extra/settings.h"

using net::SSLConfig;

namespace seaturtle {

uint16 ToContentSslVersion(int protoSslVersion) {
  switch (protoSslVersion) {
    case settings::SslConfig::SSL3:
      return net::SSL_PROTOCOL_VERSION_SSL3;
    case settings::SslConfig::TLS1:
      return net::SSL_PROTOCOL_VERSION_TLS1;
    case settings::SslConfig::TLS1_1:
      return net::SSL_PROTOCOL_VERSION_TLS1_1;
    case settings::SslConfig::TLS1_2:
      return net::SSL_PROTOCOL_VERSION_TLS1_2;
  }
  LOG(FATAL) << "Unknown ssl protocol: " << protoSslVersion;
  return 0;
}

#define COPY_SETTING(to, from, name) \
  to.name = from.name()

SSLConfigService::SSLConfigService() {
  STLOG() << "configuring ssl config";
  ST_SETTINGS(all_settings);
  const settings::SslConfig& s = all_settings->ssl_config();
  COPY_SETTING(static_config_, s, rev_checking_enabled);
  COPY_SETTING(static_config_, s, rev_checking_required_local_anchors);
  COPY_SETTING(static_config_, s, channel_id_enabled);
  COPY_SETTING(static_config_, s, false_start_enabled);
  COPY_SETTING(static_config_, s, signed_cert_timestamps_enabled);
  COPY_SETTING(static_config_, s, require_forward_secrecy);
  COPY_SETTING(static_config_, s, send_client_cert);
  COPY_SETTING(static_config_, s, verify_ev_cert);
  COPY_SETTING(static_config_, s, version_fallback);
  COPY_SETTING(static_config_, s, cert_io_enabled);

  static_config_.version_min =
    std::max(ToContentSslVersion(s.version_min()), net::kDefaultSSLVersionMin);
  static_config_.version_max = net::kDefaultSSLVersionMax;
}

void SSLConfigService::GetSSLConfig(SSLConfig* config) {
  *config = static_config_;
}

SSLConfigService::~SSLConfigService() {
}

}  // namespace seaturtle
