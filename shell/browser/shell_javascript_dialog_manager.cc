// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/browser/shell_javascript_dialog_manager.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/web_contents.h"
#include "net/base/net_util.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/shell/browser/shell.h"
#include "seaturtle/jni/jni_bridge.h"

using content::WebContents;
using content::JavaScriptMessageType;
using seaturtle::jni::Params;
using seaturtle::jni::JavascriptDialog;
using seaturtle::jni::JavascriptDialogClosed;

namespace seaturtle {

ShellJavaScriptDialogManager::ShellJavaScriptDialogManager() {
}

ShellJavaScriptDialogManager::~ShellJavaScriptDialogManager() {
}

void ShellJavaScriptDialogManager::RunJavaScriptDialog(
    WebContents* web_contents,
    const GURL& origin_url,
    const std::string& accept_lang,
    JavaScriptMessageType javascript_message_type,
    const base::string16& message_text,
    const base::string16& default_prompt_text,
    const DialogClosedCallback& callback,
    bool* did_suppress_message) {
  *did_suppress_message = false;
  // Todo support throttling.

  Params p;
  p.set_type(Params::JAVASCRIPT_DIALOG);
  JavascriptDialog* d = p.mutable_javascript_dialog();
  d->set_native_web_contents(reinterpret_cast<intptr_t>(web_contents));
  switch (javascript_message_type) {
    case content::JAVASCRIPT_MESSAGE_TYPE_ALERT:
      d->set_message_type(JavascriptDialog::ALERT);
      break;
    case content::JAVASCRIPT_MESSAGE_TYPE_CONFIRM:
      d->set_message_type(JavascriptDialog::CONFIRM);
      break;
    case content::JAVASCRIPT_MESSAGE_TYPE_PROMPT:
      d->set_message_type(JavascriptDialog::PROMPT);
      break;
    default:
      STNOTREACHED() << "unknown javascript message type " <<
        javascript_message_type;
      return;
  }
  d->set_url(origin_url.spec());
  d->set_accept_lang(accept_lang);
  d->set_text(UTF16ToUTF8(message_text));
  d->set_prompt(UTF16ToUTF8(default_prompt_text));
  last_callback_.reset(new DialogClosedCallback(callback));
  jni::Invoke(p);
}

void ShellJavaScriptDialogManager::RunBeforeUnloadDialog(
    WebContents* web_contents,
    const base::string16& message_text,
    bool is_reload,
    const DialogClosedCallback& callback) {
  Params p;
  p.set_type(Params::JAVASCRIPT_DIALOG);
  JavascriptDialog* d = p.mutable_javascript_dialog();
  d->set_native_web_contents(reinterpret_cast<intptr_t>(web_contents));
  d->set_message_type(JavascriptDialog::BEFORE_UNLOAD);
  d->set_text(UTF16ToUTF8(message_text));
  d->set_is_reload(is_reload);
  last_callback_.reset(new DialogClosedCallback(callback));
  jni::Invoke(p);
}

void ShellJavaScriptDialogManager::CancelActiveAndPendingDialogs(
    WebContents* web_contents) {
  last_callback_.reset();
}

void ShellJavaScriptDialogManager::WebContentsDestroyed(
    WebContents* web_contents) {
  last_callback_.reset();
}

// static
void ShellJavaScriptDialogManager::DialogClosed(
    const JavascriptDialogClosed& p) {
  Shell* shell = Shell::ForWebContents(
      reinterpret_cast<WebContents*>(p.native_web_contents()));
  if (shell == NULL) {
    STNOTREACHED() << "could not find shell for javascript dialog: ";
    // p.native_web_contents();
    return;
  }
  ShellJavaScriptDialogManager* dm = shell->GetShellJavaScriptDialogManager();
  CHECK(dm != NULL);
  if (dm->last_callback_.get() == NULL) {
    STNOTREACHED() << "callback for javascript dialog is null";
    return;
  }
  dm->last_callback_->Run(p.success(), base::UTF8ToUTF16(p.text()));;
  dm->last_callback_.reset();
}

}  // namespace seaturtle
