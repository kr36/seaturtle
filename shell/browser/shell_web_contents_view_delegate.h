// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_BROWSER_SHELL_WEB_CONTENTS_VIEW_DELEGATE_H_
#define SEATURTLE_SHELL_BROWSER_SHELL_WEB_CONTENTS_VIEW_DELEGATE_H_

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"

namespace seaturtle {

// TODO(cy) this can probably be moved into shell.cc instead.
class ShellWebContentsViewDelegate : public ::content::WebContentsViewDelegate {
 public:
  explicit ShellWebContentsViewDelegate(::content::WebContents* web_contents);
  virtual ~ShellWebContentsViewDelegate();

  virtual void ShowContextMenu(::content::RenderFrameHost* render_frame_host,
      const ::content::ContextMenuParams& params) OVERRIDE;
  virtual ::content::WebDragDestDelegate* GetDragDestDelegate() OVERRIDE;

 private:
  ::content::WebContents* web_contents_;

  DISALLOW_COPY_AND_ASSIGN(ShellWebContentsViewDelegate);
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_BROWSER_SHELL_WEB_CONTENTS_VIEW_DELEGATE_H_
