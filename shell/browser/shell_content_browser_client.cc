// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/browser/shell_content_browser_client.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file.h"
#include "base/path_service.h"
#include "content/browser/browser_url_handler_impl.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/url_constants.h"
#include "net/base/net_errors.h"
#include "net/cert/x509_certificate.h"
#include "net/ssl/ssl_info.h"
#include "net/url_request/url_request_context_getter.h"
#include "url/gurl.h"
#include "content/public/common/web_preferences.h"

#include "seaturtle/extra/cookie_access_policy.h"
#include "seaturtle/extra/shell_descriptors.h"
#include "seaturtle/extra/base.h"
#include "seaturtle/extra/constants.h"
#include "seaturtle/extra/settings.h"
#include "seaturtle/extra/seaturtle_protocol_handler.h"
#include "seaturtle/shell/browser/shell_browser_main_parts.h"
#include "seaturtle/shell/browser/shell.h"
#include "seaturtle/shell/browser/shell_browser_context.h"
#include "seaturtle/shell/browser/shell_web_contents_view_delegate.h"
#include "seaturtle/shell/browser/shell_resource_dispatcher_host_delegate.h"
#include "seaturtle/shell/browser/shell_quota_permission_context.h"
#include "seaturtle/shell/browser/shell_url_request_context_getter.h"
#include "seaturtle/extra/process_cache.h"
#include "seaturtle/jni/jni_bridge.h"
#include "seaturtle/block/blocker.h"


using content::BrowserMainParts;
using content::RenderProcessHost;
using content::BrowserContext;
using content::BrowserURLHandlerImpl;
using content::ProtocolHandlerMap;
using content::MainFunctionParams;
using content::URLRequestInterceptorScopedVector;
using content::RenderViewHost;
using content::ResourceDispatcherHost;
using content::ResourceContext;
using content::ResourceType;
using content::WebContentsViewDelegate;
using content::WebPreferences;
using content::QuotaPermissionContext;
using content::FileDescriptorInfo;
using content::WebContents;
using content::CertificateRequestResultType;

using net::X509Certificate;

using seaturtle::Shell;
using seaturtle::ProcessCache;
using seaturtle::jni::Params;
using seaturtle::jni::LogEvent;
using seaturtle::jni::CertificateError;
using seaturtle::jni::CertificateErrorResponse;
using seaturtle::settings::AllSettings;

namespace seaturtle {

ShellContentBrowserClient::ShellContentBrowserClient()
    : shell_browser_main_parts_(NULL) {
}

ShellContentBrowserClient::~ShellContentBrowserClient() {
}

BrowserMainParts* ShellContentBrowserClient::CreateBrowserMainParts(
    const MainFunctionParams& parameters) {
  shell_browser_main_parts_ = new ShellBrowserMainParts(parameters);
  return shell_browser_main_parts_;
}

void ShellContentBrowserClient::RenderProcessWillLaunch(
    RenderProcessHost* host) {
  // We used to IPC to the render process here.
}

net::URLRequestContextGetter* ShellContentBrowserClient::CreateRequestContext(
    BrowserContext* browser_context,
    ProtocolHandlerMap* protocol_handlers,
    URLRequestInterceptorScopedVector interceptors) {
  STLOG() << "CreateRequestContext";
  return new ShellURLRequestContextGetter(browser_context->GetPath());
}

net::URLRequestContextGetter*
ShellContentBrowserClient::CreateRequestContextForStoragePartition(
    BrowserContext* browser_context,
    const base::FilePath& partition_path,
    bool in_memory,
    ProtocolHandlerMap* protocol_handlers,
    URLRequestInterceptorScopedVector interceptors) {
  return NULL;
//  STLOG() << "CreateRequestContextForStoragePartition "
//    << partition_path.value() << " " << in_memory;
//  return new ShellURLRequestContextGetter(
//      browser_context->GetPath(), in_memory);
}

bool ShellContentBrowserClient::IsHandledURL(const GURL& url) {
  STLOG() << "is handled url: " << url;
  if (url.SchemeIs(kKryptonScheme)) {
    return true;
  }
  return false;
}

void ShellContentBrowserClient::AppendExtraCommandLineSwitches(
    CommandLine* command_line, int child_process_id) {
  // This used to have stuff...
}

void ShellContentBrowserClient::ResourceDispatcherHostCreated() {
  // This fixed issues with not rendering errors for weird URLs
  // "a://b" properly.
  // HandleExternalProtocol was returning true, it should be false by default
  // However we will likely need to add something back in for login, or
  // intent:// handling.
  resource_dispatcher_host_delegate_.reset(
      new ShellResourceDispatcherHostDelegate());
  ResourceDispatcherHost::Get()->SetDelegate(
      resource_dispatcher_host_delegate_.get());
}

std::string ShellContentBrowserClient::GetDefaultDownloadName() {
  STNOTREACHED() << "seaturtle does not use chromium downloads";
  return std::string();
}

WebContentsViewDelegate* ShellContentBrowserClient::GetWebContentsViewDelegate(
    WebContents* web_contents) {
  return new ShellWebContentsViewDelegate(web_contents);
}

QuotaPermissionContext*
ShellContentBrowserClient::CreateQuotaPermissionContext() {
  return new ShellQuotaPermissionContext();
}

void ShellContentBrowserClient::GetAdditionalMappedFilesForChildProcess(
    const CommandLine& command_line,
    int child_process_id,
    std::vector<content::FileDescriptorInfo>* mappings) {
  int flags = base::File::FLAG_OPEN | base::File::FLAG_READ;
  base::FilePath pak_file;
  bool r = PathService::Get(base::DIR_ANDROID_APP_DATA, &pak_file);
  CHECK(r);
  pak_file = pak_file.Append(FILE_PATH_LITERAL(kPakFileDir));
  pak_file = pak_file.Append(FILE_PATH_LITERAL(kPakFileSeaturtle));

  base::File f(pak_file, flags);
  STDCHECK(f.IsValid());
  mappings->push_back(
      content::FileDescriptorInfo(kSeaturtlePakDescriptor,
                                  base::FileDescriptor(f.Pass())));
  mappings->push_back(
      content::FileDescriptorInfo(kSeaturtleBlockingDataDescriptor,
          Blocker::Singleton()->GetBlockingDataFd()));
}


typedef base::Callback<void(bool)> AceCb;
void ShellContentBrowserClient::AllowCertificateError(
      int render_process_id,
      int render_frame_id,
      int cert_error,
      const net::SSLInfo& ssl_info,
      const GURL& request_url,
      ResourceType resource_type,
      bool overridable,
      bool strict_enforcement,
      bool expired_previous_descision,
      const AceCb& callback,
      CertificateRequestResultType* result) {
  STLOG() << "cert error: " << cert_error << " " <<
    net::ErrorToString(cert_error);
  if (resource_type != content::RESOURCE_TYPE_MAIN_FRAME) {
    // A sub-resource has a certificate error.  The user doesn't really
    // have a context for making the right decision, so block the
    // request hard, without an info bar to allow showing the insecure
    // content.
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY;
    Shell* s = Shell::ForRenderProcessAndFrame(render_process_id,
        render_frame_id);
    s->LogEvent(LogEvent::SUB_RESOURCE_SSL_ERROR, true,
        request_url.spec());
    return;
  }
  // TODO(cy) we're only allowed to override if override is true and
  // strict_enforcement is false?
  Params p;
  p.set_type(Params::CERTIFICATE_ERROR);
  CertificateError* ce = p.mutable_certificate_error();
  ce->set_native_callback(reinterpret_cast<intptr_t>(new AceCb(callback)));
  ce->set_render_process_id(render_process_id);
  ce->set_render_frame_id(render_frame_id);
  ce->set_cert_error(cert_error);
  ce->set_url(request_url.spec());
  if (ssl_info.cert.get() != NULL) {
    CHECK(X509Certificate::GetDEREncoded(
          ssl_info.cert.get()->os_cert_handle(),
          ce->mutable_der_encoded_cert()));
  }
  jni::Invoke(p);
}

void ShellContentBrowserClient::RequestGeolocationPermission(
    WebContents* web_contents,
    int bridge_id,
    const GURL& requesting_frame,
    bool user_gesture,
    base::Callback<void(bool)> result_callback,
    base::Closure* cancel_callback) {
  STLOG() << "request geolocation permission";
  Shell* s = Shell::ForWebContents(web_contents);
  if (s != NULL) {
    s->LogEvent(LogEvent::LOCATION, true, requesting_frame.spec());
  }
  result_callback.Run(false);
}

void ShellContentBrowserClient::RequestMidiSysExPermission(
    WebContents* web_contents,
    int bridge_id,
    const GURL& requesting_frame,
    bool user_gesture,
    base::Callback<void(bool)> result_callback,
    base::Closure* cancel_callback) {
  STLOG() << "request midisysex permission";
  // TODO(cy) log event?
  result_callback.Run(false);
}

void ShellContentBrowserClient::RequestProtectedMediaIdentifierPermission(
    WebContents* web_contents,
    const GURL& origin,
    base::Callback<void(bool)> result_callback,
    base::Closure* cancel_callback) {
  STLOG() << "request protected media identifier permission";
  // Todo(cy) log event?
  result_callback.Run(false);
}

void ShellContentBrowserClient::OverrideWebkitPrefs(
    RenderViewHost* render_view_host,
    const GURL& url,
    WebPreferences* p) {
  UpdateWebPreferences(p);
}

// static
void ShellContentBrowserClient::AllowCertificateErrorResponse(
    const CertificateErrorResponse& resp) {
  AceCb* cb = reinterpret_cast<AceCb*>(resp.native_callback());
  cb->Run(resp.allow());
  delete cb;
}

std::string ShellContentBrowserClient::GetApplicationLocale() {
  return ProcessCache::Singleton()->GetApplicationLocale();
}

// I believe this should just be a simple comma separated list, no q values.
// We use GenerateAcceptLanguageHeader elsewhere.
std::string ShellContentBrowserClient::GetAcceptLangs(
    BrowserContext* context) {
  return ProcessCache::Singleton()->GetAcceptLangs();
}

bool ShellContentBrowserClient::AllowAppCache(const GURL& manifest_url,
                            const GURL& first_party,
                            ResourceContext* context) {
  // TODO(cy) log an event
  STLOG() << "allow appcache";
  return false;
}

bool ShellContentBrowserClient::AllowGetCookie(const GURL& url,
                            const GURL& first_party,
                            const net::CookieList& cookie_list,
                            ResourceContext* context,
                            int render_process_id,
                            int render_frame_id) {
  return CookieAccessPolicy::GetInstance()->AllowGetCookie(
      url, first_party, cookie_list, context,
      render_process_id, render_frame_id);
}

bool ShellContentBrowserClient::AllowSetCookie(const GURL& url,
                            const GURL& first_party,
                            const std::string& cookie_line,
                            ResourceContext* context,
                            int render_process_id,
                            int render_frame_id,
                            net::CookieOptions* options) {
  return CookieAccessPolicy::GetInstance()->AllowSetCookie(
      url, first_party, cookie_line, context,
      render_process_id, render_frame_id, options);
}

bool ShellContentBrowserClient::AllowSaveLocalState(ResourceContext* context) {
  STLOG() << "allow savelocalstate";
  // TODO(cy) log an event?
  return false;
}

bool ShellContentBrowserClient::AllowWorkerDatabase(
    const GURL& url,
    const base::string16& name,
    const base::string16& display_name,
    unsigned long estimated_size,
    ResourceContext* context,
    const std::vector<std::pair<int, int> >& render_frames) {
  STLOG() << "allow worker database";
  // TODO(cy) log an event?
  return false;
}

void ShellContentBrowserClient::AllowWorkerFileSystem(
    const GURL& url,
    ResourceContext* context,
    const std::vector<std::pair<int, int> >& render_frames,
    base::Callback<void(bool)> callback) {
  STLOG() << "allow worker filesystem";
  // TODO(cy) log an event?
  callback.Run(false);
}

bool ShellContentBrowserClient::AllowWorkerIndexedDB(
    const GURL& url,
    const base::string16& name,
    ResourceContext* context,
    const std::vector<std::pair<int, int> >& render_frames) {
  STLOG() << "allow worker indexdb";
  // TODO(cy) log an event?
  return false;
}

void ShellContentBrowserClient::GetStoragePartitionConfigForSite(
      BrowserContext* browser_context,
      const GURL& site,
      bool can_be_default,
      std::string* partition_domain,
      std::string* partition_name,
      bool* in_memory) {
  /*STLOG() << "GetStoragePartitionConfigForSite " << site;
  *partition_domain = "st-default-partition-domain";
  *partition_name = site.spec();
  *in_memory = true;
  if (site.host() == "facebook.com") {
    *in_memory = false;
  }*/ 
  // TODO(cy) Essentially this controls when new request contexts are created?
  content::ContentBrowserClient::GetStoragePartitionConfigForSite(
      browser_context,
      site,
      can_be_default,
      partition_domain,
      partition_name,
      in_memory);
}

}  // namespace seaturtle
