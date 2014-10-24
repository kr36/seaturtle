// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_BROWSER_SHELL_JAVASCRIPT_DIALOG_MANAGER_H_
#define SEATURTLE_SHELL_BROWSER_SHELL_JAVASCRIPT_DIALOG_MANAGER_H_

#include <string>

#include "base/callback_forward.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/javascript_dialog_manager.h"

namespace seaturtle {

namespace jni {
class JavascriptDialogClosed;
}

class ShellJavaScriptDialogManager : public content::JavaScriptDialogManager {
 public:
  ShellJavaScriptDialogManager();
  virtual ~ShellJavaScriptDialogManager();

  // JavaScriptDialogManager:
  virtual void RunJavaScriptDialog(
      content::WebContents* web_contents,
      const GURL& origin_url,
      const std::string& accept_lang,
      content::JavaScriptMessageType javascript_message_type,
      const base::string16& message_text,
      const base::string16& default_prompt_text,
      const DialogClosedCallback& callback,
      bool* did_suppress_message) OVERRIDE;

  virtual void RunBeforeUnloadDialog(
      content::WebContents* web_contents,
      const base::string16& message_text,
      bool is_reload,
      const DialogClosedCallback& callback) OVERRIDE;

  virtual void CancelActiveAndPendingDialogs(
      content::WebContents* web_contents) OVERRIDE;

  virtual void WebContentsDestroyed(
      content::WebContents* web_contents) OVERRIDE;

  static void DialogClosed(const jni::JavascriptDialogClosed& p);

 private:
  scoped_ptr<DialogClosedCallback> last_callback_;

  DISALLOW_COPY_AND_ASSIGN(ShellJavaScriptDialogManager);
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_BROWSER_SHELL_JAVASCRIPT_DIALOG_MANAGER_H_
