// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_RENDERER_SHELL_RENDER_FRAME_OBSERVER_H_
#define SEATURTLE_SHELL_RENDERER_SHELL_RENDER_FRAME_OBSERVER_H_

#include "content/public/renderer/render_frame_observer.h"

namespace blink {
class WebFrame;
}

namespace content {
class RenderFrame;
}

namespace seaturtle {
class ShellRenderFrameObserver : public content::RenderFrameObserver {
 public:
  explicit ShellRenderFrameObserver(content::RenderFrame* render_frame);
  virtual ~ShellRenderFrameObserver() {}

  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
  void OnInjectCssInFrame();

  DISALLOW_COPY_AND_ASSIGN(ShellRenderFrameObserver);
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_RENDERER_SHELL_RENDER_FRAME_OBSERVER_H_
