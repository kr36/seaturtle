// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/extra/proxy_config_service.h"

#include "base/bind.h"
#include "base/location.h"
#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "base/sequenced_task_runner.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/jni/jni_bridge.h"

using net::ProxyConfig;
using base::SequencedTaskRunner;
using content::BrowserThread;

namespace seaturtle {

// static
std::vector<ProxyConfigService*> ProxyConfigService::all_configs_;

ProxyConfigService::ProxyConfigService() {
  STDCHECK_ON_THREAD(IO);
//    SequencedTaskRunner* network_task_runner) :
//    network_task_runner_(network_task_runner) {
  all_configs_.push_back(this);
//  STDCHECK(network_task_runner != NULL);
  jni::Params req;
  jni::Params resp;
  req.set_type(jni::Params::PROXY_CONFIG_REQUEST);
  jni::Invoke(req, &resp);
  SetConfigOnNetworkThread(resp.proxy_config());
}

ProxyConfigService::~ProxyConfigService() {
  for (size_t i = 0; i < all_configs_.size(); ++i) {
    if (all_configs_[i] == this) {
      all_configs_.erase(all_configs_.begin() + i);
      break;
    }
  }
}

void ProxyConfigService::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void ProxyConfigService::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

// static
void ProxyConfigService::SetConfig(
    const jni::ProxyConfig& config) {
  content::BrowserThread::UnsafeGetMessageLoopForThread(
      content::BrowserThread::IO)->message_loop_proxy()->PostTask(
        FROM_HERE,
        base::Bind(
            &ProxyConfigService::SetAllConfigsOnNetworkThread, config));
}

// static
void ProxyConfigService::SetAllConfigsOnNetworkThread(
    const jni::ProxyConfig& config) {
  STDCHECK_ON_THREAD(IO);
  for (size_t i = 0; i < all_configs_.size(); ++i) {
    all_configs_[i]->SetConfigOnNetworkThread(config);
  }
}

void ProxyConfigService::SetConfigOnNetworkThread(
    const jni::ProxyConfig& config) {
  STDCHECK_ON_THREAD(IO);
  ProxyConfig* pc = NULL;
  net::ProxyConfigService::ConfigAvailability state = net::ProxyConfigService::CONFIG_PENDING;
  if (config.state() == jni::ProxyConfig::VALID) {
    state = net::ProxyConfigService::CONFIG_VALID;
    pc = new ProxyConfig();
    pc->proxy_rules().ParseFromString(config.config());
    latest_.reset(pc);
  }
  FOR_EACH_OBSERVER(Observer, observers_, OnProxyConfigChanged(*pc, state));
}

net::ProxyConfigService::ConfigAvailability
    ProxyConfigService::GetLatestProxyConfig(net::ProxyConfig* config) {
  if (latest_.get() == NULL) {
    return net::ProxyConfigService::CONFIG_PENDING;
  }
  *config = *(latest_.get());
  return net::ProxyConfigService::CONFIG_VALID;
}

}  // namespace seaturtle
