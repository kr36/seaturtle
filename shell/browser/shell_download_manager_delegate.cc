// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/browser/shell_download_manager_delegate.h"

#include "base/files/file_path.h"
#include "content/public/browser/download_danger_type.h"
#include "content/public/browser/download_item.h"
#include "url/gurl.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/jni/jni_bridge.h"

namespace seaturtle {

bool ShellDownloadManagerDelegate::DetermineDownloadTarget(
    content::DownloadItem* item,
    const content::DownloadTargetCallback& callback) {
  // Note this cancel is independent of the URLRequest cancel in
  // ShellResourceDispatcherHostDelegate::DownloadStarting. The request
  // could have already finished by the time DownloadStarting is called.
  callback.Run(base::FilePath() /* Empty file path for cancel */,
               content::DownloadItem::TARGET_DISPOSITION_OVERWRITE,
               content::DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS,
               base::FilePath());
  jni::Params p;
  p.set_type(jni::Params::START_DOWNLOAD);
  p.set_start_download(item->GetURL().spec());
  jni::Invoke(p);
  return true;
}

bool ShellDownloadManagerDelegate::ShouldCompleteDownload(
    content::DownloadItem* item,
    const base::Closure& complete_callback) {
  STNOTREACHED();
  return true;
}

bool ShellDownloadManagerDelegate::ShouldOpenDownload(
    content::DownloadItem* item,
    const content::DownloadOpenDelayedCallback& callback) {
  STNOTREACHED();
  return true;
}

void ShellDownloadManagerDelegate::GetNextId(
    const content::DownloadIdCallback& callback) {
  static uint32 next_id = content::DownloadItem::kInvalidId + 1;
  callback.Run(next_id++);
}

}  // namespace seaturtle
