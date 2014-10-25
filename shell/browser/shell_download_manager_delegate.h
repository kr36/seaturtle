// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_BROWSER_SHELL_DOWNLOAD_MANAGER_DELEGATE_H_
#define SEATURTLE_SHELL_BROWSER_SHELL_DOWNLOAD_MANAGER_DELEGATE_H_

#include "content/public/browser/download_manager_delegate.h"

// Forked android_webview/browser/aw_download_manager_delegate.h

namespace seaturtle {

// Android WebView does not use Chromium downloads, so implement methods here to
// unconditionally cancel the download.
class ShellDownloadManagerDelegate : public content::DownloadManagerDelegate {
 public:
  virtual ~ShellDownloadManagerDelegate() {}

  // content::DownloadManagerDelegate implementation.
  virtual bool DetermineDownloadTarget(
      content::DownloadItem* item,
      const content::DownloadTargetCallback& callback) OVERRIDE;
  virtual bool ShouldCompleteDownload(
      content::DownloadItem* item,
      const base::Closure& complete_callback) OVERRIDE;
  virtual bool ShouldOpenDownload(
      content::DownloadItem* item,
      const content::DownloadOpenDelayedCallback& callback) OVERRIDE;
  virtual void GetNextId(const content::DownloadIdCallback& callback) OVERRIDE;
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_BROWSER_SHELL_DOWNLOAD_MANAGER_DELEGATE_H_
