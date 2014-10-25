// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_EXTRA_SETTINGS_H_
#define SEATURTLE_EXTRA_SETTINGS_H_

#include <string>

#include "base/macros.h"
#include "base/memory/ref_counted.h"

// for convience
#include "seaturtle/protos/setting.pb.h"

#define ST_SETTINGS(var) \
  seaturtle::GlobalSettings st_global_settings_ref = \
  seaturtle::GetGlobalSettingsRef(); \
  seaturtle::settings::AllSettings* var = &st_global_settings_ref.get()->data;

namespace content {
class WebPreferences;
}

namespace seaturtle {

namespace settings {
class AllSettings;
}

typedef base::RefCountedData<settings::AllSettings> RefCountedAllSettings;
typedef scoped_refptr<RefCountedAllSettings> GlobalSettings;

GlobalSettings GetGlobalSettingsRef();
void UpdateGlobalSettings(const settings::AllSettings& new_settings);
void UpdateWebPreferences(content::WebPreferences* p);

}  // namespace seaturtle

#endif  // SEATURTLE_EXTRA_SETTINGS_H_
