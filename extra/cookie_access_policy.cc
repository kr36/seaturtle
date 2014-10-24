// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/extra/cookie_access_policy.h"

#include "net/base/net_errors.h"

#include "seaturtle/shell/browser/shell.h"
#include "seaturtle/extra/base.h"
#include "seaturtle/jni/jni_bridge.h"

using seaturtle::jni::LogEvent;

namespace seaturtle {

namespace {
base::LazyInstance<CookieAccessPolicy>::Leaky g_lazy_instance;
}  // namespace

CookieAccessPolicy::~CookieAccessPolicy() {
}

CookieAccessPolicy::CookieAccessPolicy()
  : policy_(net::StaticCookiePolicy::BLOCK_ALL_THIRD_PARTY_COOKIES) {
}

// static
CookieAccessPolicy* CookieAccessPolicy::GetInstance() {
  return g_lazy_instance.Pointer();
}

bool CookieAccessPolicy::OnCanGetCookies(const net::URLRequest& request,
                                         const net::CookieList& cookie_list) {
  bool allowed = policy_.CanGetCookies(
      request.url(), request.first_party_for_cookies()) == net::OK;
  STLOG() << "can get cookie: "
    << (allowed ? "true" : "false") << " " << request.url()
    << " first-party: " << request.first_party_for_cookies();
  return allowed;
}

bool CookieAccessPolicy::OnCanSetCookie(const net::URLRequest& request,
                                          const std::string& cookie_line,
                                          net::CookieOptions* options) {
  bool allowed = policy_.CanSetCookie(
      request.url(), request.first_party_for_cookies()) == net::OK;
  STLOG() << "can set cookie: "
    << (allowed ? "true" : "false") << " " << request.url()
    << " first-party: " << request.first_party_for_cookies();
  Shell* shell = Shell::ForURLRequest(request);
  if (shell) {
    shell->LogEvent(LogEvent::COOKIE, !allowed,
        request.url().spec());
  }
  return allowed;
}

bool CookieAccessPolicy::AllowGetCookie(const GURL& url,
                                          const GURL& first_party,
                                          const net::CookieList& cookie_list,
                                          content::ResourceContext* context,
                                          int render_process_id,
                                          int render_frame_id) {
  bool allowed = policy_.CanGetCookies(url, first_party) == net::OK;
  STLOG() << "allow get cookie: "
    << (allowed ? "true" : "false") << " " << url
    << " first-party: " << first_party;
  // TODO(cy) log an event?
  return allowed;
}

bool CookieAccessPolicy::AllowSetCookie(const GURL& url,
                                          const GURL& first_party,
                                          const std::string& cookie_line,
                                          content::ResourceContext* context,
                                          int render_process_id,
                                          int render_frame_id,
                                          net::CookieOptions* options) {
  bool allowed = policy_.CanSetCookie(url, first_party) == net::OK;
  STLOG() << "allow set cookie: "
    << (allowed ? "true" : "false") << " " << url
    << " first-party: " << first_party;
  // TODO(cy) log an event?
  return allowed;
}

}  // namespace seaturtle
