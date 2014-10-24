// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/browser/shell_browser_context.h"

#include "base/lazy_instance.h"
#include "base/bind.h"
#include "base/environment.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/threading/thread.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"

#include "net/url_request/url_request_context.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/extra/constants.h"
#include "seaturtle/shell/browser/shell_url_request_context_getter.h"
#include "seaturtle/shell/browser/shell_download_manager_delegate.h"

using content::ResourceContext;
using content::BrowserThread;
using content::DownloadManagerDelegate;
using content::ProtocolHandlerMap;
using content::URLRequestInterceptorScopedVector;
using content::DownloadManager;

namespace seaturtle {

namespace {
  base::LazyInstance<ShellBrowserContext> singleton =
    LAZY_INSTANCE_INITIALIZER;
}

class ShellBrowserContext::ShellResourceContext : public ResourceContext {
 public:
  ShellResourceContext(ShellBrowserContext* sbc) : sbc_(sbc) {}
  virtual ~ShellResourceContext() {}

  // ResourceContext implementation:
  virtual net::HostResolver* GetHostResolver() OVERRIDE {
    return GetRequestContext()->host_resolver();
  }
  virtual net::URLRequestContext* GetRequestContext() OVERRIDE {
    return sbc_->GetRequestContext()->GetURLRequestContext();
  }
  virtual bool AllowMicAccess(const GURL& origin) OVERRIDE {
    return false;
  }
  virtual bool AllowCameraAccess(const GURL& origin) OVERRIDE {
    return false;
  }

 private:
  ShellBrowserContext* sbc_;
  DISALLOW_COPY_AND_ASSIGN(ShellResourceContext);
};

ShellBrowserContext::ShellBrowserContext()
    : resource_context_(new ShellResourceContext(this)) {
  STLOG() << "browser context created";
  InitWhileIOAllowed();
}

ShellBrowserContext::~ShellBrowserContext() {
  if (resource_context_) {
    BrowserThread::DeleteSoon(
      BrowserThread::IO, FROM_HERE, resource_context_.release());
  }
  STLOG() << "browser context destroyed";
}

// static
ShellBrowserContext* ShellBrowserContext::Get() {
  return singleton.Pointer();
}

void ShellBrowserContext::InitWhileIOAllowed() {
  STDCHECK_ON_THREAD(UI);
  CHECK(PathService::Get(base::DIR_ANDROID_APP_DATA, &path_));
  path_ = path_.Append(FILE_PATH_LITERAL(kBaseDir));
  if (!base::PathExists(path_))
    base::CreateDirectory(path_);
}

base::FilePath ShellBrowserContext::GetPath() const {
  STDCHECK_ON_THREAD(UI);
  return path_;
}

bool ShellBrowserContext::IsOffTheRecord() const {
  return true;
}

DownloadManagerDelegate* ShellBrowserContext::GetDownloadManagerDelegate()  {
  STDCHECK_ON_THREAD(UI);
  if (!download_manager_delegate_.get()) {
    download_manager_delegate_.reset(new ShellDownloadManagerDelegate());
  }
  return download_manager_delegate_.get();
}

net::URLRequestContextGetter* ShellBrowserContext::GetRequestContext()  {
  // STDCHECK_ON_THREAD(UI);
  return GetDefaultStoragePartition(this)->GetURLRequestContext();
}

net::URLRequestContextGetter*
    ShellBrowserContext::GetRequestContextForRenderProcess(
        int renderer_child_id)  {
  STDCHECK_ON_THREAD(UI);
  return GetRequestContext();
}

net::URLRequestContextGetter*
    ShellBrowserContext::GetMediaRequestContext()  {
  STDCHECK_ON_THREAD(UI);
  return GetRequestContext();
}

net::URLRequestContextGetter*
    ShellBrowserContext::GetMediaRequestContextForRenderProcess(
        int renderer_child_id)  {
  STDCHECK_ON_THREAD(UI);
  return GetRequestContext();
}

net::URLRequestContextGetter*
    ShellBrowserContext::GetMediaRequestContextForStoragePartition(
        const base::FilePath& partition_path,
        bool in_memory) {
  STDCHECK_ON_THREAD(UI);
  return GetRequestContext();
}

ResourceContext* ShellBrowserContext::GetResourceContext()  {
  STDCHECK_ON_THREAD(UI);
  return resource_context_.get();
}

quota::SpecialStoragePolicy* ShellBrowserContext::GetSpecialStoragePolicy() {
  STDCHECK_ON_THREAD(UI);
  return NULL;
}

content::BrowserPluginGuestManager* ShellBrowserContext::GetGuestManager() {
  STDCHECK_ON_THREAD(UI);
  return NULL;
}

content::PushMessagingService* ShellBrowserContext::GetPushMessagingService() {
  STDCHECK_ON_THREAD(UI);
  return NULL;
}

content::SSLHostStateDelegate* ShellBrowserContext::GetSSLHostStateDelegate() {
  STDCHECK_ON_THREAD(UI);
  return NULL;
}

}  // namespace seaturtle
