// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_BROWSER_SHELL_BROWSER_MAIN_PARTS_H_
#define SEATURTLE_SHELL_BROWSER_SHELL_BROWSER_MAIN_PARTS_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/common/main_function_params.h"

namespace seaturtle {

class ShellBrowserContext;

class ShellBrowserMainParts : public content::BrowserMainParts {
 public:
  explicit ShellBrowserMainParts(const content::MainFunctionParams& parameters);
  virtual ~ShellBrowserMainParts();

  // BrowserMainParts overrides.
  virtual void PreEarlyInitialization() OVERRIDE;
  virtual void PostMainMessageLoopStart() OVERRIDE;
  virtual void PreMainMessageLoopRun() OVERRIDE;
  virtual bool MainMessageLoopRun(int* result_code) OVERRIDE;
  virtual void PostMainMessageLoopRun() OVERRIDE;

 private:
  const content::MainFunctionParams parameters_;
  bool run_message_loop_;

  DISALLOW_COPY_AND_ASSIGN(ShellBrowserMainParts);
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_BROWSER_SHELL_BROWSER_MAIN_PARTS_H_
