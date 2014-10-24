// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/jni/jni_bridge.h"

#include <string>

#include "seaturtle/block/blocker.h"
#include "seaturtle/extra/fetch.h"
#include "seaturtle/protos/jni.pb.h"
#include "seaturtle/shell/browser/shell.h"
#include "seaturtle/shell/browser/shell_login_dialog.h"
#include "seaturtle/shell/browser/shell_javascript_dialog_manager.h"
#include "seaturtle/shell/browser/shell_browser_context.h"
#include "seaturtle/shell/browser/shell_content_browser_client.h"
#include "seaturtle/shell/browser/shell_url_request_context_getter.h"
#include "seaturtle/extra/proxy_config_service.h"
#include "base/android/jni_android.h"

#include "seaturtle/extra/settings.h"
#include "seaturtle/extra/split_cookie_store.h"
#include "seaturtle/extra/base.h"
#include "seaturtle/extra/https_rewriter.h"

namespace seaturtle {
namespace jni {

jclass bridge_cls = NULL;
jmethodID bridge_mid = NULL;

void Initialize() {
  JNIEnv* env = base::android::AttachCurrentThread();
  bridge_cls = env->FindClass("co/kr36/krypton/jni/Bridge");
  CHECK(bridge_cls != NULL);
  bridge_cls = reinterpret_cast<jclass>(env->NewGlobalRef(bridge_cls));
  CHECK(bridge_cls != NULL);
  bridge_mid = env->GetStaticMethodID(bridge_cls, "nativeToJava", "([B)[B");
  CHECK(bridge_mid != NULL);
}

void invokeWithArg(JNIEnv* env, jbyteArray* arg, Params* response) {
  STDCHECK(env != NULL);
  STDCHECK(bridge_cls != NULL);
  STDCHECK(bridge_mid != NULL);
  jbyteArray jba = (jbyteArray) env->CallStaticObjectMethod(
      bridge_cls, bridge_mid, *arg);
  CHECK(env->ExceptionOccurred() == NULL) << "exception during bridge call";
  if (response != NULL && jba != NULL) {
    int len = env->GetArrayLength(jba);
    unsigned char *buffer = (unsigned char*)
      malloc(sizeof(unsigned char) * len);
    env->GetByteArrayRegion(jba, 0, len, reinterpret_cast<jbyte*>(buffer));
    env->DeleteLocalRef(jba);
    CHECK(response->ParseFromArray(buffer, len)) << "bad bridge proto";
    free(buffer);
  }
}

void invokeBytes(const char* data, int len, Params* response) {
  // TODO(cy) Check that we are never on the Network Thread.
  // DCHECK_CURRENTLY_ON(BrowserThread::UI);
  JNIEnv* env = base::android::AttachCurrentThread();
  jbyteArray arg = env->NewByteArray(len);
  env->SetByteArrayRegion(arg, 0, len,
      reinterpret_cast<const jbyte*>(data));
  invokeWithArg(env, &arg, response);
  env->DeleteLocalRef(arg);
}

void Invoke(const Params& req, Params* resp) {
  std::string s = req.SerializeAsString();
  invokeBytes(s.c_str(), s.length(), resp);
}

}  // namespace jni
}  // namespace seaturtle

extern "C" {
JNIEXPORT jbyteArray JNICALL Java_co_kr36_krypton_jni_Bridge_javaToNative(
    JNIEnv *env, jclass cls, jbyteArray jba) {
  int len = env->GetArrayLength(jba);
  unsigned char *buffer = (unsigned char*) malloc(sizeof(unsigned char) * len);
  env->GetByteArrayRegion(jba, 0, len, reinterpret_cast<jbyte*>(buffer));
  seaturtle::jni::Params params;
  CHECK(params.ParseFromArray(buffer, len)) << "bad bridge proto";
  free(buffer);
  seaturtle::jni::Params* response = NULL;
  switch (params.type()) {
    case seaturtle::jni::Params::LAUNCH_SHELL:
        response = seaturtle::Shell::LaunchShell(params.launch_shell());
        break;
    case seaturtle::jni::Params::UPDATE_BLOCKING_RULES:
        seaturtle::Blocker::Singleton()->UpdateRulesWithPath(
            params.update_blocking_rules());
        break;
    case seaturtle::jni::Params::HTTP_AUTH_RESPONSE:
        seaturtle::ShellLoginDialog::ProcessResponse(
            params.http_auth_response());
        break;
    case seaturtle::jni::Params::HTTP_FETCH:
        response = seaturtle::Fetcher::Fetch(params.http_fetch());
        break;
    case seaturtle::jni::Params::JAVASCRIPT_DIALOG_CLOSED:
        seaturtle::ShellJavaScriptDialogManager::DialogClosed(
            params.javascript_dialog_closed());
        break;
    case seaturtle::jni::Params::CERTIFICATE_ERROR_RESPONSE:
        seaturtle::ShellContentBrowserClient::AllowCertificateErrorResponse(
            params.certificate_error_response());
        break;
    case seaturtle::jni::Params::UPDATE_SETTINGS:
        seaturtle::UpdateGlobalSettings(params.update_settings());
        break;
    case seaturtle::jni::Params::SHELL_COMMAND:
        response = seaturtle::Shell::DispatchShellCommand(
            params.shell_command());
        break;
    case seaturtle::jni::Params::PROXY_CONFIG_CHANGED:
        seaturtle::ProxyConfigService::SetConfig(params.proxy_config());
        break;
    case seaturtle::jni::Params::UPDATE_HTTPS_RULES:
        seaturtle::HttpsRewriter::Get()->UpdateRulesWithPath(
            params.update_https_rules());
        break;
    case seaturtle::jni::Params::UPDATE_FAVORITES_RESPONSE:
        seaturtle::ShellURLRequestContextGetter::GetSplitCookieStore()->
          UpdateFavorites(params.update_favorites_response());
        break;
    case seaturtle::jni::Params::CLEAR_DATA:
        seaturtle::ShellURLRequestContextGetter::ClearData();
        break;
    case seaturtle::jni::Params::UPDATE_BLOCKING_WHITELIST:
        seaturtle::Blocker::Singleton()->UpdateWhitelist(
            params.update_blocking_whitelist());
        break;
    default:
        STNOTREACHED() << "unknown bridge type " << params.type();
  }
  if (response == NULL) {
    return NULL;
  }
  std::string s = response->SerializeAsString();
  jbyteArray ret = env->NewByteArray(s.length());
  env->SetByteArrayRegion(ret, 0, s.length(),
      reinterpret_cast<const jbyte*>(s.c_str()));
  delete response;
  return ret;
}
}
