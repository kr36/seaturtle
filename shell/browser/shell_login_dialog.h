// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_BROWSER_SHELL_LOGIN_DIALOG_H_
#define SEATURTLE_SHELL_BROWSER_SHELL_LOGIN_DIALOG_H_

#include "base/compiler_specific.h"
#include "base/strings/string16.h"
#include "content/public/browser/resource_dispatcher_host_login_delegate.h"

namespace net {
class AuthChallengeInfo;
class URLRequest;
}

namespace seaturtle {

namespace jni {
class HttpAuthResponse;
}

// This class provides a dialog box to ask the user for credentials. Useful in
// ResourceDispatcherHostDelegate::CreateLoginDelegate.
class ShellLoginDialog : public content::ResourceDispatcherHostLoginDelegate {
 public:
  // Threading: IO thread.
  ShellLoginDialog(net::AuthChallengeInfo* auth_info, net::URLRequest* request);

  // ResourceDispatcherHostLoginDelegate implementation:
  // Threading: IO thread.
  virtual void OnRequestCancelled() OVERRIDE;


  static void ProcessResponse(const jni::HttpAuthResponse& response);

 protected:
  // Threading: any
  virtual ~ShellLoginDialog();

 private:
  // Called from OnRequestCancelled if the request was cancelled.
  // Threading: UI thread.
  void PlatformRequestCancelled();

  // Sets up dialog creation.
  // Threading: UI thread.
  void PrepDialog(const base::string16& host, const base::string16& realm);

  // Sends the authentication to the requester.
  // Threading: IO thread.
  void SendAuthToRequester(bool success,
                           const base::string16& username,
                           const base::string16& password);

  // Called by the platform specific code when the user responds. Public because
  // the aforementioned platform specific code may not have access to private
  // members. Not to be called from client code.
  // Threading: UI thread.
  void UserAcceptedAuth(const base::string16& username,
                        const base::string16& password);
  void UserCancelledAuth();

  // Who/where/what asked for the authentication.
  // Threading: IO thread.
  scoped_refptr<net::AuthChallengeInfo> auth_info_;

  // The request that wants login data.
  // Threading: IO thread.
  net::URLRequest* request_;
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_BROWSER_SHELL_LOGIN_DIALOG_H_
