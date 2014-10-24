// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/browser/shell_quota_permission_context.h"

#include "url/gurl.h"
#include "webkit/common/quota/quota_types.h"

#include "seaturtle/extra/base.h"

using content::StorageQuotaParams;

namespace seaturtle {

ShellQuotaPermissionContext::ShellQuotaPermissionContext() {}

void ShellQuotaPermissionContext::RequestQuotaPermission(
    const StorageQuotaParams& params,
    int render_process_id,
    const PermissionCallback& callback) {
  STLOG() << "requesting quota permission: " << params.origin_url;
  callback.Run(QUOTA_PERMISSION_RESPONSE_DISALLOW);
}

ShellQuotaPermissionContext::~ShellQuotaPermissionContext() {}

}  // namespace seaturtle
