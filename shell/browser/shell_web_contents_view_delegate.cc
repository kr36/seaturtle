// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/browser/shell_web_contents_view_delegate.h"

#include "base/command_line.h"
#include "content/public/browser/android/content_view_core.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/context_menu_params.h"
#include "seaturtle/jni/jni_bridge.h"

namespace seaturtle {

ShellWebContentsViewDelegate::ShellWebContentsViewDelegate(
    ::content::WebContents* web_contents)
    : web_contents_(web_contents) {
}

ShellWebContentsViewDelegate::~ShellWebContentsViewDelegate() {
}

void ShellWebContentsViewDelegate::ShowContextMenu(
    ::content::RenderFrameHost* render_frame_host,
    const ::content::ContextMenuParams& params) {
  if (params.is_editable && params.selection_text.empty()) {
    ::content::ContentViewCore* content_view_core =
        ::content::ContentViewCore::FromWebContents(web_contents_);
    if (content_view_core) {
      content_view_core->ShowPastePopup(params.selection_start.x(),
                                        params.selection_start.y());
    }
  } else {
    jni::Params p;
    p.set_type(jni::Params::CONTEXT_MENU);
    p.mutable_context_menu()->set_url(params.link_url.spec());
    p.mutable_context_menu()->set_content_url(params.src_url.spec());
    jni::Invoke(p);
  }
}

::content::WebDragDestDelegate* ShellWebContentsViewDelegate::GetDragDestDelegate() {
 return NULL;
}

}  // namespace seaturtle
