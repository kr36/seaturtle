// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/browser/shell_resource_dispatcher_host_delegate.h"

#include "base/logging.h"
#include "url/gurl.h"

#include "seaturtle/shell/browser/shell_login_dialog.h"
#include "seaturtle/extra/base.h"
#include "seaturtle/jni/jni_bridge.h"

namespace seaturtle {

ShellResourceDispatcherHostDelegate::ShellResourceDispatcherHostDelegate() {
}

ShellResourceDispatcherHostDelegate::~ShellResourceDispatcherHostDelegate() {
}

bool ShellResourceDispatcherHostDelegate::HandleExternalProtocol(
    const GURL& url,
    int child_id,
    int route_id) {
  STLOG() << "HandleExternalProtocol " << url;
  // TODO(cy) pass these in from java, use a set of strings
  // actually maybe just let java decide if it's going to handle it.
  // how often is this getting called?
  if (url.SchemeIs("intent")
      || url.SchemeIs("market")
      || url.SchemeIs("mailto")
      || url.SchemeIs("tel")
      || url.SchemeIs("sip")
      || url.SchemeIs("sms")) {
    jni::Params p;
    p.set_type(jni::Params::INTENT_URL);
    p.set_intent_url(url.spec());
    jni::Invoke(p);
    return true;
  }
  return false;
}

content::ResourceDispatcherHostLoginDelegate*
ShellResourceDispatcherHostDelegate::CreateLoginDelegate(
    net::AuthChallengeInfo* auth_info, net::URLRequest* request) {
  return new ShellLoginDialog(auth_info, request);
}

}  // namespace seaturtle
