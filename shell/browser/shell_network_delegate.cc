// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/browser/shell_network_delegate.h"

#include <string>

#include "base/files/file_path.h"
#include "net/base/net_errors.h"
#include "net/url_request/url_request.h"

#include "seaturtle/extra/cookie_access_policy.h"
#include "seaturtle/extra/https_rewriter.h"
#include "seaturtle/extra/base.h"
#include "seaturtle/extra/fetch.h"
#include "seaturtle/shell/browser/shell.h"
#include "seaturtle/block/blocker.h"
#include "seaturtle/jni/jni_bridge.h"

using seaturtle::Blocker;
using seaturtle::jni::LogEvent;

namespace seaturtle {

ShellNetworkDelegate::ShellNetworkDelegate() {
}

ShellNetworkDelegate::~ShellNetworkDelegate() {
}

int ShellNetworkDelegate::OnBeforeURLRequest(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    GURL* new_url) {
  int ret = net::OK;
  if (Fetcher::IsAutocompleteRequest(*request)) {
    return ret;
  }
  Blocker* b = seaturtle::Blocker::Singleton();
  if (!b->IsWhitelisted(request->first_party_for_cookies())
      && !request->url().DomainIs(
        request->first_party_for_cookies().host().c_str())
      && b->ShouldBlockThirdPartyRequest(request->url())) {
    STLOG() << "blocking third party request: " << request->url();
    ret = net::ERR_BLOCKED_BY_CLIENT;
    // b->NotifyFrameRequestBlocked(*request);
  }
  // TODO(cy) this probably should happen on the UI thread instead.
  // Infact I found a FromID method that does a DCHECK for the UI thread...
  // use GetMessageLoopProxyForThread
  Shell* shell = Shell::ForURLRequest(*request);
  if (shell) {
    shell->LogEvent(LogEvent::REQUEST, ret != net::OK,
        request->url().spec());
  } else {
    // TODO(cy) Fuck, not sure why this still happens.
    // clicking and a Google search result hits this code path.
    //  STNOTREACHED() << "no shell for request: " << request->url();
  }
  if (ret == net::OK) {
    if (b->ShouldClearReferrer()) {
      request->SetReferrer("");
    }
    return HttpsRewriter::Get()->MaybeRewrite(request, new_url);
  }
  // TODO(cy) revisit an https only option
  // if (ret == net::OK && request->url().scheme() == "http") {
  //  if (request->url().scheme() == "http") {
  //    CHECK(new_url != NULL);
  //    (*new_url) = GURL("https:" + request->url().GetContent());
  //  } else if (request->url().scheme() == "ws") {
  //    (*new_url) = GURL("wss:" + request->url().GetContent());
  //  }
  // }
  return ret;
}

int ShellNetworkDelegate::OnBeforeSendHeaders(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    net::HttpRequestHeaders* headers) {
  return net::OK;
}

void ShellNetworkDelegate::OnSendHeaders(
    net::URLRequest* request,
    const net::HttpRequestHeaders& headers) {
}

int ShellNetworkDelegate::OnHeadersReceived(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    const net::HttpResponseHeaders* original_response_headers,
    scoped_refptr<net::HttpResponseHeaders>* override_response_headers,
    GURL* allowed_unsafe_redirect_url) {
  return net::OK;
}

void ShellNetworkDelegate::OnBeforeRedirect(net::URLRequest* request,
                                            const GURL& new_location) {
}

void ShellNetworkDelegate::OnResponseStarted(net::URLRequest* request) {
}

void ShellNetworkDelegate::OnRawBytesRead(const net::URLRequest& request,
                                          int bytes_read) {
}

void ShellNetworkDelegate::OnCompleted(net::URLRequest* request, bool started) {
}

void ShellNetworkDelegate::OnURLRequestDestroyed(net::URLRequest* request) {
}

void ShellNetworkDelegate::OnPACScriptError(int line_number,
                                            const base::string16& error) {
}

ShellNetworkDelegate::AuthRequiredResponse ShellNetworkDelegate::OnAuthRequired(
    net::URLRequest* request,
    const net::AuthChallengeInfo& auth_info,
    const AuthCallback& callback,
    net::AuthCredentials* credentials) {
  return AUTH_REQUIRED_RESPONSE_NO_ACTION;
}

bool ShellNetworkDelegate::OnCanGetCookies(const net::URLRequest& request,
                                           const net::CookieList& cookie_list) {
  return CookieAccessPolicy::GetInstance()->OnCanGetCookies(
      request, cookie_list);
}

bool ShellNetworkDelegate::OnCanSetCookie(const net::URLRequest& request,
                                          const std::string& cookie_line,
                                          net::CookieOptions* options) {
  return CookieAccessPolicy::GetInstance()->OnCanSetCookie(
      request, cookie_line, options);
}

bool ShellNetworkDelegate::OnCanAccessFile(const net::URLRequest& request,
                                           const base::FilePath& path) const {
  Shell* shell = Shell::ForURLRequest(request);
  if (shell) {
    shell->LogEvent(LogEvent::FILE, true, path.AsUTF8Unsafe());
  } else {
    // STNOTREACHED() << "no shell for request: " << request.url();
  }
  return false;
}

bool ShellNetworkDelegate::OnCanThrottleRequest(
    const net::URLRequest& request) const {
  return false;
}

int ShellNetworkDelegate::OnBeforeSocketStreamConnect(
    net::SocketStream* socket,
    const net::CompletionCallback& callback) {
  return net::OK;
}

}  // namespace seaturtle
