// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_RENDERER_SHELL_CONTENT_RENDERER_CLIENT_H_
#define SEATURTLE_SHELL_RENDERER_SHELL_CONTENT_RENDERER_CLIENT_H_

#include <string>

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/renderer/content_renderer_client.h"
#include "seaturtle/extra/base.h"

namespace seaturtle {

class ShellContentRendererClient : public ::content::ContentRendererClient {
 public:
  ShellContentRendererClient() {}
  virtual ~ShellContentRendererClient() {}

  virtual void RenderViewCreated(content::RenderView* render_view) OVERRIDE;
  virtual void RenderFrameCreated(content::RenderFrame* render_frame) OVERRIDE;
  virtual bool HasErrorPage(int http_status_code,
                            std::string* error_domain);
  virtual void GetNavigationErrorStrings(
      content::RenderView* render_view,
      blink::WebFrame* frame,
      const blink::WebURLRequest& failed_request,
      const blink::WebURLError& error,
      std::string* error_html,
      base::string16* error_description);
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_RENDERER_SHELL_CONTENT_RENDERER_CLIENT_H_
