// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_BROWSER_SHELL_URL_REQUEST_CONTEXT_GETTER_H_
#define SEATURTLE_SHELL_BROWSER_SHELL_URL_REQUEST_CONTEXT_GETTER_H_

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/url_request/url_request_job_factory.h"

namespace net {
class HostResolver;
class URLRequestContextStorage;
class SdchManager;
}

namespace seaturtle {

class SplitCookieStore;

class ShellURLRequestContextGetter : public net::URLRequestContextGetter {
 public:
  explicit ShellURLRequestContextGetter(const base::FilePath& base_path);
  ShellURLRequestContextGetter();  // For the fetcher.

  virtual net::URLRequestContext* GetURLRequestContext() OVERRIDE;
  virtual scoped_refptr<base::SingleThreadTaskRunner>
      GetNetworkTaskRunner() const OVERRIDE;
  
  net::HostResolver* host_resolver();

  static SplitCookieStore* GetSplitCookieStore();
  static void ClearData();

 protected:
  virtual ~ShellURLRequestContextGetter();

 private:
  base::FilePath base_path_;

  scoped_ptr<net::URLRequestContextStorage> storage_;
  // TODO(cy) in latest chrome this has been added to context storage.
  scoped_ptr<net::SdchManager> sdch_;
  scoped_ptr<net::URLRequestContext> url_request_context_;

  DISALLOW_COPY_AND_ASSIGN(ShellURLRequestContextGetter);
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_BROWSER_SHELL_URL_REQUEST_CONTEXT_GETTER_H_
