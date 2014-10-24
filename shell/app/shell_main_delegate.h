// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_APP_SHELL_MAIN_DELEGATE_H_
#define SEATURTLE_SHELL_APP_SHELL_MAIN_DELEGATE_H_

#include "base/memory/scoped_ptr.h"
#include "content/public/app/content_main_delegate.h"
#include "seaturtle/shell/common/shell_content_client.h"

namespace content {
class BrowserMainRunner;
}

namespace seaturtle {

class ShellContentBrowserClient;
class ShellContentRendererClient;

class ShellMainDelegate : public content::ContentMainDelegate {
 public:
  ShellMainDelegate();
  virtual ~ShellMainDelegate();

  virtual bool BasicStartupComplete(int* exit_code) OVERRIDE;
  virtual void PreSandboxStartup() OVERRIDE;
  virtual int RunProcess(
      const std::string& process_type,
      const content::MainFunctionParams& main_function_params) OVERRIDE;
  virtual content::ContentBrowserClient*
      CreateContentBrowserClient() OVERRIDE;
  virtual content::ContentRendererClient*
      CreateContentRendererClient() OVERRIDE;

  static void InitializeResourceBundle();

 private:
  scoped_ptr<ShellContentBrowserClient> browser_client_;
  scoped_ptr<ShellContentRendererClient> renderer_client_;
  ShellContentClient content_client_;

  scoped_ptr<content::BrowserMainRunner> browser_runner_;

  DISALLOW_COPY_AND_ASSIGN(ShellMainDelegate);
};

}  // namespace seaturtle
#endif // SEATURTLE_SHELL_APP_SHELL_MAIN_DELEGATE_H_
