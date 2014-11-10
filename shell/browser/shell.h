// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_BROWSER_SHELL_H_
#define SEATURTLE_SHELL_BROWSER_SHELL_H_

#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "ipc/ipc_channel.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/size.h"
#include "ui/base/window_open_disposition.h"

#include "content/public/browser/javascript_dialog_manager.h"

#include "seaturtle/shell/browser/shell_browser_context.h"
#include "seaturtle/protos/jni.pb.h"

class GURL;

namespace net {
class URLRequest;
}  // namespace net

namespace content {
class SiteInstance;
class WebContents;
}  // namespace content

namespace seaturtle {

class ShellJavaScriptDialogManager;

namespace jni {
class Params;
class ShellCommand;
class ShellCommandResponse;
class FindInPage;
}  // namespace jni

class Shell : public content::WebContentsDelegate,
              public content::WebContentsObserver {
 public:
  virtual ~Shell();

  static jni::Params* LaunchShell(intptr_t parent);
  static Shell* ForWebContents(const content::WebContents* wc);
  static Shell* ForRenderProcessAndView(int render_process_id,
      int render_view_id);
  static Shell* ForRenderProcessAndFrame(int render_process_id,
      int render_frame_id);
  static Shell* ForURLRequest(const net::URLRequest& request);
  static Shell* ForRenderViewHost(const content::RenderViewHost* rvh);
  static Shell* ForPtr(intptr_t ptr);
  static jni::Params* DispatchShellCommand(const jni::ShellCommand& sc);
  static void UpdateAllWebPreferences();

  void SendCommand(jni::ShellCommand::Command c,
      const jni::ShellCommand& sc = jni::ShellCommand());
  void LogEvent(jni::LogEvent_Type t, bool is_blocked,
      const std::string& desc = "");

  // Overrides
  virtual content::JavaScriptDialogManager*
      GetJavaScriptDialogManager() OVERRIDE;
  virtual ShellJavaScriptDialogManager* GetShellJavaScriptDialogManager();

  virtual void LoadingStateChanged(content::WebContents* source,
      bool to_different_document) OVERRIDE;
  virtual void LoadProgressChanged(content::WebContents* source,
      double progress) OVERRIDE;
  virtual void CloseContents(content::WebContents* source) OVERRIDE;
  virtual void AddNewContents(content::WebContents* source,
                              content::WebContents* new_contents,
                              WindowOpenDisposition disposition,
                              const gfx::Rect& initial_pos,
                              bool user_gesture,
                              bool* was_blocked) OVERRIDE;
  virtual content::WebContents* OpenURLFromTab(content::WebContents* source,
      const content::OpenURLParams& params) OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& message,
                                 content::RenderFrameHost* render_frame_host);

  virtual void DidNavigateMainFrame(
      const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) OVERRIDE;
  virtual void DidFinishLoad(content::RenderFrameHost* rfh,
                             const GURL& validated_url) OVERRIDE;

  virtual void DidUpdateFaviconURL(
      const std::vector<content::FaviconURL>& candidates) OVERRIDE;

  virtual void ToggleFullscreenModeForTab(content::WebContents* web_contents,
                                          bool enter_fullscreen) OVERRIDE;
  virtual bool IsFullscreenForTabOrPending(
      const content::WebContents* web_contents) const;
  // TODO(cy): RendererUnresponsive WorkerCrashed?
  // implement activate/deactivate contents ?
  // Handle context menu, title was set "
 private:
  // THIS IS HERE SO THAT IT IS DESTROYED LAST. Specifically, webcontents
  // must get destroyed before this.
  linked_ptr<ShellBrowserContext> sbc_;

  static std::vector<Shell*> all_shells_;
  static jni::Params* CreateShell(content::WebContents* web_contents,
      intptr_t parent, linked_ptr<ShellBrowserContext> sbc);
  static void FaviconDownloaded(const GURL& site_url, int id, int http_status,
      const GURL& image_url, const std::vector<SkBitmap>& images,
      const std::vector<gfx::Size>& sizes);

  void OnNotifyDidCreateDocumentElement(content::RenderFrameHost* rfh);
  void Close();
  jni::ShellCommandResponse* GetLog();
  jni::ShellCommandResponse* GetSSLStatus();
  jni::ShellCommandResponse* GetSSLDetails();
  jni::ShellCommandResponse* GetVisibleUrl();
  void DownloadFavicon();
  jni::Params* HandleShellCommand(const jni::ShellCommand& sc);
  void Load(const std::string& surl, bool as_desktop);
  void Reload(bool as_desktop);
  void UpdateWebPreferences();
  void FindInPage(const jni::FindInPage& fip);

  scoped_ptr<GURL> favicon_candidate_;
  base::Lock log_lock_;  // TODO(cy) RW lock?
  jni::Log log_;

  explicit Shell(content::WebContents* web_contents);
  scoped_ptr<ShellJavaScriptDialogManager> jdm_;
  scoped_ptr<content::WebContents> wc_;
  scoped_refptr<content::SiteInstance> si_;
  bool is_fullscreen_;

  intptr_t parent_shell_;
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_BROWSER_SHELL_H_
