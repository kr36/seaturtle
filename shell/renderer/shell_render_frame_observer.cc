// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/renderer/shell_render_frame_observer.h"

#include "content/public/renderer/render_frame.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebScriptSource.h"

#include "seaturtle/block/blocker.h"
#include "seaturtle/extra/shell_messages.h"
#include "seaturtle/extra/base.h"

using blink::WebScriptSource;
using content::RenderFrame;
using content::RenderFrameObserver;

namespace seaturtle {

ShellRenderFrameObserver::ShellRenderFrameObserver(RenderFrame* render_frame)
    : RenderFrameObserver(render_frame) {}

bool ShellRenderFrameObserver::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ShellRenderFrameObserver, message)
    IPC_MESSAGE_HANDLER(SeaturtleMsg_InjectCssInFrame, OnInjectCssInFrame)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void ShellRenderFrameObserver::OnInjectCssInFrame() {
  blink::WebFrame* wf = render_frame()->GetWebFrame();
  STLOG() << "injecting css in frame " << GURL(wf->document().url());
  Blocker::Singleton()->InjectStyleSheets(wf);
}

}  // namespace seaturtle
