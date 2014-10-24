// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_BROWSER_SHELL_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
#define SEATURTLE_SHELL_BROWSER_SHELL_RESOURCE_DISPATCHER_HOST_DELEGATE_H_

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "content/public/browser/resource_dispatcher_host_delegate.h"

namespace seaturtle {

class ShellResourceDispatcherHostDelegate
    : public content::ResourceDispatcherHostDelegate {
 public:
  ShellResourceDispatcherHostDelegate();
  virtual ~ShellResourceDispatcherHostDelegate();

  virtual content::ResourceDispatcherHostLoginDelegate* CreateLoginDelegate(
      net::AuthChallengeInfo* auth_info, net::URLRequest* request) OVERRIDE;

  virtual bool HandleExternalProtocol(const GURL& url,
                                      int child_id,
                                      int route_id) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(ShellResourceDispatcherHostDelegate);
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_BROWSER_SHELL_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
