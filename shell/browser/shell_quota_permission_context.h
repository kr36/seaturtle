// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_BROWSER_SHELL_QUOTA_PERMISSION_CONTEXT_H_
#define SEATURTLE_SHELL_BROWSER_SHELL_QUOTA_PERMISSION_CONTEXT_H_

#include "base/compiler_specific.h"
#include "content/public/browser/quota_permission_context.h"

namespace seaturtle {

class ShellQuotaPermissionContext : public content::QuotaPermissionContext {
 public:
  ShellQuotaPermissionContext();

  virtual void RequestQuotaPermission(
      const content::StorageQuotaParams& params,
      int render_process_id,
      const PermissionCallback& callback) OVERRIDE;

 private:
  virtual ~ShellQuotaPermissionContext();

  DISALLOW_COPY_AND_ASSIGN(ShellQuotaPermissionContext);
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_BROWSER_SHELL_QUOTA_PERMISSION_CONTEXT_H_
