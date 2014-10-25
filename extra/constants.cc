// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/extra/constants.h"

namespace seaturtle {

// These should match up /w the Java side
const char kBaseDir[] = "seaturtle";
const char kPakFileDir[] = "paks";
const char kPakFileSeaturtle[] = "seaturtle.pak";

const char kStateDir[] = "state";
const char kCacheDir[] = "cache";
const char kHstsDir[] = "hsts";
const char kCookieStore[] = "cookie_store";
const char kKryptonScheme[] = "krypton";
const char kBadUrlUrl[] = "krypton://bad-url";
const char kBadUrlParam[] = "url";

//  WATCH update UA for each chromium update.
//  Youtube/Gmail stops working if you mess with this too much.
const char kUserAgentMobile[] =
  "Mozilla/5.0 (Linux; Android 4.4.4; NA Build/NA) "
  "AppleWebKit/537.36 (KHTML, like Gecko) "
  "Chrome/38.0.2125.102 Mobile Safari/537.36";
const char kUserAgentDesktop[] =
  "Mozilla/5.0 (X11; Linux x86_64) "
  "AppleWebKit/537.36 (KHTML, like Gecko) "
  "Chrome/38.0.2125.102 Safari/537.36";

const int kIdealFaviconSize = 48;
}  // namespace seaturtle
