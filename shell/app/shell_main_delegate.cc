// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/app/shell_main_delegate.h"

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "content/public/browser/browser_main_runner.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "base/posix/global_descriptors.h"

#include "seaturtle/extra/shell_descriptors.h"
#include "seaturtle/block/blocker.h"
#include "seaturtle/extra/base.h"
#include "seaturtle/extra/constants.h"
#include "seaturtle/shell/browser/shell_content_browser_client.h"
#include "seaturtle/shell/renderer/shell_content_renderer_client.h"

namespace seaturtle {

ShellMainDelegate::ShellMainDelegate() {
}

ShellMainDelegate::~ShellMainDelegate() {
}

bool ShellMainDelegate::BasicStartupComplete(int* exit_code) {
  logging::LoggingSettings settings;
  settings.logging_dest= logging::LOG_TO_SYSTEM_DEBUG_LOG;
  logging::InitLogging(settings);
  logging::SetLogItems(true, true, true, true);
  content::SetContentClient(&content_client_);
  return false;
}

void ShellMainDelegate::PreSandboxStartup() {
  InitializeResourceBundle();
}

int ShellMainDelegate::RunProcess(
    const std::string& process_type,
    const content::MainFunctionParams& main_function_params) {
  if (!process_type.empty())
    return -1;

  browser_runner_.reset(content::BrowserMainRunner::Create());
  return browser_runner_->Initialize(main_function_params);
}

void ShellMainDelegate::InitializeResourceBundle() {
  STLOG() << "initializing resource bundle";
  // In the Android case, the renderer runs with a different UID and can never
  // access the file system.  So we are passed a file descriptor to the
  // ResourceBundle pak at launch time.
  bool had_any_descriptor = false;
  int blocking_dat_fd =
      base::GlobalDescriptors::GetInstance()->MaybeGet(
          kSeaturtleBlockingDataDescriptor);
  if (blocking_dat_fd >= 0) {
    had_any_descriptor = true;
    Blocker::Singleton()->UpdateRulesWithFd(blocking_dat_fd);
  }
  int pak_fd = base::GlobalDescriptors::GetInstance()->MaybeGet(
      kSeaturtlePakDescriptor);
  if (pak_fd >= 0) {
    had_any_descriptor = true;
    ui::ResourceBundle::InitSharedInstanceWithPakFileRegion(
        base::File(pak_fd), base::MemoryMappedFile::Region::kWholeFile);
    ResourceBundle::GetSharedInstance().AddDataPackFromFile(
        base::File(pak_fd), ui::SCALE_FACTOR_100P);
  }

  if (had_any_descriptor) {
    return;
  }

  base::FilePath pak_file;
  base::FilePath pak_dir;
  bool got_path = PathService::Get(base::DIR_ANDROID_APP_DATA, &pak_dir);
  CHECK(got_path);
  pak_dir = pak_dir.Append(FILE_PATH_LITERAL(kPakFileDir));
  pak_file = pak_dir.Append(FILE_PATH_LITERAL(kPakFileSeaturtle));
  ui::ResourceBundle::InitSharedInstanceWithPakPath(pak_file);
}

content::ContentBrowserClient*
    ShellMainDelegate::CreateContentBrowserClient() {
  browser_client_.reset(new ShellContentBrowserClient);
  return browser_client_.get();
}

content::ContentRendererClient*
    ShellMainDelegate::CreateContentRendererClient() {
  renderer_client_.reset(new ShellContentRendererClient);
  return renderer_client_.get();
}

}  // namespace seaturtle
