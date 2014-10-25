// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_EXTRA_PROXY_CONFIG_SERVICE_H_
#define SEATURTLE_EXTRA_PROXY_CONFIG_SERVICE_H_

#include <vector>

#include "base/macros.h"
#include "base/observer_list.h"

#include "net/proxy/proxy_config.h"
#include "net/proxy/proxy_config_service.h"

namespace seaturtle {

namespace jni {
class ProxyConfig;
}

class ProxyConfigService :  public net::ProxyConfigService {
 public:
  ProxyConfigService();
  virtual ~ProxyConfigService();

  virtual void AddObserver(Observer* observer) OVERRIDE;
  virtual void RemoveObserver(Observer* observer) OVERRIDE;
  virtual net::ProxyConfigService::ConfigAvailability GetLatestProxyConfig(
      net::ProxyConfig* config) OVERRIDE;

  static void SetConfig(const jni::ProxyConfig& config);
 private:
  static void SetAllConfigsOnNetworkThread(const jni::ProxyConfig& config);
  static std::vector<ProxyConfigService*> all_configs_;

  void SetConfigOnNetworkThread(const jni::ProxyConfig& config);
  scoped_ptr<net::ProxyConfig> latest_;
  ObserverList<Observer> observers_;
  DISALLOW_COPY_AND_ASSIGN(ProxyConfigService);
};

}  // namespace seaturtle

#endif  // SEATURTLE_EXTRA_PROXY_CONFIG_SERVICE_H_
