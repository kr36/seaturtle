// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_COMMON_SHELL_SEATURTLE_CLIENT_H_
#define SEATURTLE_SHELL_COMMON_SHELL_SEATURTLE_CLIENT_H_

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "content/public/common/content_client.h"

namespace seaturtle {

class ShellContentClient : public content::ContentClient {
 public:
  virtual ~ShellContentClient();

  virtual std::string GetUserAgent() const OVERRIDE;
  virtual base::string16 GetLocalizedString(int message_id) const OVERRIDE;
  virtual base::StringPiece GetDataResource(
      int resource_id,
      ui::ScaleFactor scale_factor) const OVERRIDE;
  virtual base::RefCountedStaticMemory* GetDataResourceBytes(
      int resource_id) const OVERRIDE;
  virtual gfx::Image& GetNativeImageNamed(int resource_id) const OVERRIDE;
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_COMMON_SHELL_SEATURTLE_CLIENT_H_
