// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/browser/shell_login_dialog.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "net/base/auth.h"
#include "net/url_request/url_request.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/jni/jni_bridge.h"

using content::BrowserThread;
using content::ResourceDispatcherHost;

namespace seaturtle {

ShellLoginDialog::ShellLoginDialog(
    net::AuthChallengeInfo* auth_info,
    net::URLRequest* request) : auth_info_(auth_info),
                                request_(request) {
  STDCHECK_ON_THREAD(IO);
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&ShellLoginDialog::PrepDialog, this,
                 base::ASCIIToUTF16(auth_info->challenger.ToString()),
                 base::UTF8ToUTF16(auth_info->realm)));
}

void ShellLoginDialog::OnRequestCancelled() {
  STDCHECK_ON_THREAD(IO);
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&ShellLoginDialog::PlatformRequestCancelled, this));
}

void ShellLoginDialog::UserAcceptedAuth(const base::string16& username,
                                        const base::string16& password) {
  STDCHECK_ON_THREAD(UI);
  BrowserThread::PostTask(
      BrowserThread::IO, FROM_HERE,
      base::Bind(&ShellLoginDialog::SendAuthToRequester, this,
                 true, username, password));
}

void ShellLoginDialog::UserCancelledAuth() {
  STDCHECK_ON_THREAD(UI);
  BrowserThread::PostTask(
      BrowserThread::IO, FROM_HERE,
      base::Bind(&ShellLoginDialog::SendAuthToRequester, this,
                 false, base::string16(), base::string16()));
}

ShellLoginDialog::~ShellLoginDialog() {
  // Cannot post any tasks here; this object is going away and cannot be
  // referenced/dereferenced.
}

void ShellLoginDialog::PrepDialog(const base::string16& host,
                                  const base::string16& realm) {
  STDCHECK_ON_THREAD(UI);
  jni::Params p;
  p.set_type(jni::Params::HTTP_AUTH);
  p.mutable_http_auth()->set_native_object(reinterpret_cast<intptr_t>(this));
  p.mutable_http_auth()->set_is_cancel(false);
  p.mutable_http_auth()->set_host(UTF16ToUTF8(host));
  p.mutable_http_auth()->set_realm(UTF16ToUTF8(realm));
  jni::Invoke(p);
}

void ShellLoginDialog::SendAuthToRequester(bool success,
                                           const base::string16& username,
                                           const base::string16& password) {
  STDCHECK_ON_THREAD(IO);
  if (success)
    request_->SetAuth(net::AuthCredentials(username, password));
  else
    request_->CancelAuth();
  ResourceDispatcherHost::Get()->ClearLoginDelegateForRequest(request_);
}

void ShellLoginDialog::PlatformRequestCancelled() {
  STDCHECK_ON_THREAD(UI);
  jni::Params p;
  p.set_type(jni::Params::HTTP_AUTH);
  p.mutable_http_auth()->set_native_object(reinterpret_cast<intptr_t>(this));
  p.mutable_http_auth()->set_is_cancel(true);
  jni::Invoke(p);
}

// static
void ShellLoginDialog::ProcessResponse(const jni::HttpAuthResponse& response) {
  ShellLoginDialog* sld =
      reinterpret_cast<ShellLoginDialog*>(response.native_object());
  if (response.is_user_cancelled()) {
    sld->UserCancelledAuth();
  } else {
    sld->UserAcceptedAuth(base::UTF8ToUTF16(response.username()),
        base::UTF8ToUTF16(response.password()));
  }
}

}  // namespace seaturtle
