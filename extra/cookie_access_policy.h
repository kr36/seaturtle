// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_EXTRA_COOKIE_ACCESS_POLICY_H_
#define SEATURTLE_EXTRA_COOKIE_ACCESS_POLICY_H_

#include "base/basictypes.h"
#include "base/lazy_instance.h"
#include "base/synchronization/lock.h"
#include "net/base/static_cookie_policy.h"
#include "net/cookies/canonical_cookie.h"
#include "net/url_request/url_request.h"

namespace content {
class ResourceContext;
}

namespace net {
class CookieOptions;
}

class GURL;

namespace seaturtle {

class CookieAccessPolicy {
 public:
  static CookieAccessPolicy* GetInstance();

  // These are the functions called when operating over cookies from the
  // network. See NetworkDelegate for further descriptions.
  bool OnCanGetCookies(const net::URLRequest& request,
                       const net::CookieList& cookie_list);
  bool OnCanSetCookie(const net::URLRequest& request,
                      const std::string& cookie_line,
                      net::CookieOptions* options);

  // These are the functions called when operating over cookies from the
  // renderer. See ContentBrowserClient for further descriptions.
  bool AllowGetCookie(const GURL& url,
                      const GURL& first_party,
                      const net::CookieList& cookie_list,
                      content::ResourceContext* context,
                      int render_process_id,
                      int render_frame_id);
  bool AllowSetCookie(const GURL& url,
                      const GURL& first_party,
                      const std::string& cookie_line,
                      content::ResourceContext* context,
                      int render_process_id,
                      int render_frame_id,
                      net::CookieOptions* options);

 private:
  friend struct base::DefaultLazyInstanceTraits<CookieAccessPolicy>;
  net::StaticCookiePolicy policy_;

  CookieAccessPolicy();
  ~CookieAccessPolicy();
  DISALLOW_COPY_AND_ASSIGN(CookieAccessPolicy);
};

}  // namespace seaturtle

#endif  // SEATURTLE_EXTRA_COOKIE_ACCESS_POLICY_H_
