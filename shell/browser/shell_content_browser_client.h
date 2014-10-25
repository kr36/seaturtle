// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_BROWSER_SHELL_CONTENT_BROWSER_CLIENT_H_
#define SEATURTLE_SHELL_BROWSER_SHELL_CONTENT_BROWSER_CLIENT_H_

#include <string>
#include <utility>
#include <vector>

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/content_browser_client.h"

namespace seaturtle {

class ShellBrowserContext;
class ShellBrowserMainParts;
class ShellResourceDispatcherHostDelegate;

namespace jni {
class CertificateErrorResponse;
}

// WATCH This is probably one of the most important classes to keep an eye on
// during chromium updates.
class ShellContentBrowserClient : public content::ContentBrowserClient {
 public:
  // Gets the current instance.
  // static ShellContentBrowserClient* Get();

  ShellContentBrowserClient();
  virtual ~ShellContentBrowserClient();

  // ContentBrowserClient overrides.
  virtual content::BrowserMainParts* CreateBrowserMainParts(
      const content::MainFunctionParams& parameters) OVERRIDE;

  virtual void RenderProcessWillLaunch(
      content::RenderProcessHost* host) OVERRIDE;

  virtual net::URLRequestContextGetter* CreateRequestContext(
      content::BrowserContext* browser_context,
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector interceptors) OVERRIDE;

  virtual net::URLRequestContextGetter*
      CreateRequestContextForStoragePartition(
        content::BrowserContext* browser_context,
        const base::FilePath& partition_path,
        bool in_memory,
        content::ProtocolHandlerMap* protocol_handlers,
        content::URLRequestInterceptorScopedVector interceptors) OVERRIDE;

  virtual bool IsHandledURL(const GURL& url) OVERRIDE;

  virtual void AppendExtraCommandLineSwitches(base::CommandLine* command_line,
                                              int child_process_id) OVERRIDE;

  virtual void ResourceDispatcherHostCreated() OVERRIDE;

  virtual std::string GetDefaultDownloadName() OVERRIDE;

  virtual content::WebContentsViewDelegate* GetWebContentsViewDelegate(
      content::WebContents* web_contents) OVERRIDE;

  virtual content::QuotaPermissionContext*
      CreateQuotaPermissionContext() OVERRIDE;

  virtual void GetAdditionalMappedFilesForChildProcess(
      const base::CommandLine& command_line,
      int child_process_id,
      std::vector<content::FileDescriptorInfo>* mappings) OVERRIDE;

  virtual void AllowCertificateError(
      int render_process_id,
      int render_frame_id,
      int cert_error,
      const net::SSLInfo& ssl_info,
      const GURL& request_url,
      content::ResourceType resource_type,
      bool overridable,
      bool strict_enforcement,
      bool expired_previous_decision,
      const base::Callback<void(bool)>& callback,
      content::CertificateRequestResultType* result) OVERRIDE;

  virtual void RequestGeolocationPermission(
      content::WebContents* web_contents,
      int bridge_id,
      const GURL& requesting_frame,
      bool user_gesture,
      base::Callback<void(bool)> result_callback,
      base::Closure* cancel_callback) OVERRIDE;

  virtual void RequestMidiSysExPermission(
      content::WebContents* web_contents,
      int bridge_id,
      const GURL& requesting_frame,
      bool user_gesture,
      base::Callback<void(bool)> result_callback,
      base::Closure* cancel_callback) OVERRIDE;

  virtual void RequestProtectedMediaIdentifierPermission(
      content::WebContents* web_contents,
      const GURL& origin,
      base::Callback<void(bool)> result_callback,
      base::Closure* cancel_callback) OVERRIDE;

  virtual void OverrideWebkitPrefs(content::RenderViewHost* render_view_host,
                                   const GURL& url,
                                   content::WebPreferences* prefs) OVERRIDE;

  virtual std::string GetApplicationLocale() OVERRIDE;

  virtual std::string GetAcceptLangs(content::BrowserContext* context) OVERRIDE;


  virtual bool AllowAppCache(const GURL& manifest_url,
                             const GURL& first_party,
                             content::ResourceContext* context) OVERRIDE;

  virtual bool AllowGetCookie(const GURL& url,
                              const GURL& first_party,
                              const net::CookieList& cookie_list,
                              content::ResourceContext* context,
                              int render_process_id,
                              int render_frame_id) OVERRIDE;

  virtual bool AllowSetCookie(const GURL& url,
                              const GURL& first_party,
                              const std::string& cookie_line,
                              content::ResourceContext* context,
                              int render_process_id,
                              int render_frame_id,
                              net::CookieOptions* options) OVERRIDE;

  virtual bool AllowSaveLocalState(content::ResourceContext* context) OVERRIDE;

  virtual bool AllowWorkerDatabase(
      const GURL& url,
      const base::string16& name,
      const base::string16& display_name,
      unsigned long estimated_size,
      content::ResourceContext* context,
      const std::vector<std::pair<int, int> >& render_frames) OVERRIDE;

  virtual void AllowWorkerFileSystem(
      const GURL& url,
      content::ResourceContext* context,
      const std::vector<std::pair<int, int> >& render_frames,
      base::Callback<void(bool)> callback) OVERRIDE;

  virtual bool AllowWorkerIndexedDB(
      const GURL& url,
      const base::string16& name,
      content::ResourceContext* context,
      const std::vector<std::pair<int, int> >& render_frames) OVERRIDE;

  virtual void GetStoragePartitionConfigForSite(
      content::BrowserContext* browser_context,
      const GURL& site,
      bool can_be_default,
      std::string* partition_domain,
      std::string* partition_name,
      bool* in_memory) OVERRIDE;

  ShellBrowserMainParts* shell_browser_main_parts() {
    return shell_browser_main_parts_;
  }

  static void AllowCertificateErrorResponse(
      const jni::CertificateErrorResponse& resp);

 private:
  scoped_ptr<ShellResourceDispatcherHostDelegate>
      resource_dispatcher_host_delegate_;

  ShellBrowserMainParts* shell_browser_main_parts_;
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_BROWSER_SHELL_CONTENT_BROWSER_CLIENT_H_
