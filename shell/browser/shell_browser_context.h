// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_BROWSER_SHELL_BROWSER_CONTEXT_H_
#define SEATURTLE_SHELL_BROWSER_SHELL_BROWSER_CONTEXT_H_

#include "base/lazy_instance.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"
#include "net/url_request/url_request_job_factory.h"

namespace content {
class DownloadManagerDelegate;
class ResourceContext;
}

namespace seaturtle {

class ShellDownloadManagerDelegate;
class ShellURLRequestContextGetter;

class ShellBrowserContext : public content::BrowserContext {
 public:
  static ShellBrowserContext* Get();
  ShellBrowserContext();
  virtual ~ShellBrowserContext();

  virtual base::FilePath GetPath() const OVERRIDE;
  virtual bool IsOffTheRecord() const OVERRIDE;
  virtual content::DownloadManagerDelegate* GetDownloadManagerDelegate() OVERRIDE;
  virtual net::URLRequestContextGetter* GetRequestContext() OVERRIDE;
  virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;
  virtual net::URLRequestContextGetter* GetMediaRequestContext() OVERRIDE;
  virtual net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;
  virtual net::URLRequestContextGetter*
      GetMediaRequestContextForStoragePartition(
          const base::FilePath& partition_path,
          bool in_memory) OVERRIDE;
  virtual content::ResourceContext* GetResourceContext() OVERRIDE;
  virtual quota::SpecialStoragePolicy* GetSpecialStoragePolicy() OVERRIDE;
  virtual content::BrowserPluginGuestManager* GetGuestManager() OVERRIDE;
  virtual content::PushMessagingService* GetPushMessagingService() OVERRIDE;
  virtual content::SSLHostStateDelegate* GetSSLHostStateDelegate() OVERRIDE;

 private:
  class ShellResourceContext;
  friend struct base::DefaultLazyInstanceTraits<ShellBrowserContext>;

  // Performs initialization of the ShellBrowserContext while IO is still
  // allowed on the current thread.
  void InitWhileIOAllowed();

  base::FilePath path_;
  scoped_ptr<ShellResourceContext> resource_context_;
  scoped_ptr<ShellDownloadManagerDelegate> download_manager_delegate_;

  DISALLOW_COPY_AND_ASSIGN(ShellBrowserContext);
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_BROWSER_SHELL_BROWSER_CONTEXT_H_
