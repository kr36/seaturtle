// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/browser/shell_browser_main_parts.h"

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/main_function_params.h"
#include "grit/net_resources.h"
#include "net/base/net_module.h"
#include "net/base/net_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "url/gurl.h"

#include "net/android/network_change_notifier_factory_android.h"
#include "net/base/network_change_notifier.h"

#include "seaturtle/shell/browser/shell.h"
#include "seaturtle/shell/browser/shell_browser_context.h"

using content::BrowserContext;
using content::BrowserThread;
using content::MainFunctionParams;

namespace seaturtle {

namespace {

base::StringPiece PlatformResourceProvider(int key) {
  if (key == IDR_DIR_HEADER_HTML) {
    base::StringPiece html_data =
        ui::ResourceBundle::GetSharedInstance().GetRawDataResource(
            IDR_DIR_HEADER_HTML);
    return html_data;
  }
  return base::StringPiece();
}

}  // namespace

ShellBrowserMainParts::ShellBrowserMainParts(
    const MainFunctionParams& parameters)
    : BrowserMainParts(), parameters_(parameters), run_message_loop_(true) {}

ShellBrowserMainParts::~ShellBrowserMainParts() {
}

void ShellBrowserMainParts::PostMainMessageLoopStart() {
  base::MessageLoopForUI::current()->Start();
}

void ShellBrowserMainParts::PreEarlyInitialization() {
  net::NetworkChangeNotifier::SetFactory(
      new net::NetworkChangeNotifierFactoryAndroid());
  //  CommandLine::ForCurrentProcess()->AppendSwitch(
  //    cc::switches::kCompositeToMailbox);
}

void ShellBrowserMainParts::PreMainMessageLoopRun() {
  // browser_context_.reset(new ShellBrowserContext());
  net::NetModule::SetResourceProvider(PlatformResourceProvider);

  if (parameters_.ui_task) {
    parameters_.ui_task->Run();
    delete parameters_.ui_task;
    run_message_loop_ = false;
  }
}

bool ShellBrowserMainParts::MainMessageLoopRun(int* result_code)  {
  return !run_message_loop_;
}

void ShellBrowserMainParts::PostMainMessageLoopRun() {
  // browser_context_.reset();
}

}  // namespace seaturtle
