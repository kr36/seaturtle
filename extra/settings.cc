// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/extra/settings.h"

#include "base/lazy_instance.h"
#include "content/public/common/web_preferences.h"

#include "seaturtle/shell/browser/shell.h"
#include "seaturtle/extra/base.h"

using content::WebPreferences;

namespace seaturtle {

namespace {

class GlobalSettingsSingleton {
 public:
  static GlobalSettingsSingleton* GetInstance() {
    return Singleton<GlobalSettingsSingleton,
        LeakySingletonTraits<GlobalSettingsSingleton> >::get();
  }

  GlobalSettings settings_;
 private:
  friend class Singleton<GlobalSettingsSingleton>;
  friend struct DefaultSingletonTraits<GlobalSettingsSingleton>;

  GlobalSettingsSingleton() {}
  ~GlobalSettingsSingleton() {}

  DISALLOW_COPY_AND_ASSIGN(GlobalSettingsSingleton);
};

}  // namespace

GlobalSettings GetGlobalSettingsRef() {
  GlobalSettingsSingleton* gss = GlobalSettingsSingleton::GetInstance();
  STDCHECK(gss->settings_.get() != NULL);
  return gss->settings_;
}

void UpdateGlobalSettings(const settings::AllSettings& new_settings) {
  STLOG() << "updating global settings";
  GlobalSettings new_gs(new RefCountedAllSettings());
  new_gs.get()->data.MergeFrom(new_settings);

  GlobalSettingsSingleton* gss = GlobalSettingsSingleton::GetInstance();
  gss->settings_.swap(new_gs);
  Shell::UpdateAllWebPreferences();
}

#define COPY_SETTING(to, from, name) \
  to->name = from.name()
#define AND_SETTING(to, from, name) \
  to->name &= from.name()

void UpdateWebPreferences(WebPreferences* p) {
  // $CR/webkit/common/preferences.h
  ST_SETTINGS(as);
  const settings::WebPreferences& swp = as->web_preferences();

  // WATCH audit WebPreferences for every chromium update
  // standard_font_family_map
  // fixed_font_family_map
  // serif_font_Family_map
  // sans_serif_font_family_map
  // cursive_font_family_map
  // fantasy_font_family_map
  // pictograph_font_family_map
  // default_font_size
  // default_fixed_font_size
  // minimum_font_size
  // minimum_logical_font_size
  // default_encoding
  COPY_SETTING(p, swp, javascript_enabled);
  p->web_security_enabled = true;
  COPY_SETTING(p, swp, javascript_can_open_windows_automatically);
  COPY_SETTING(p, swp, loads_images_automatically);
  COPY_SETTING(p, swp, images_enabled);
  p->plugins_enabled = false;
  p->dom_paste_enabled = false;
  // shrinks_standalone_images_to_fit;
  // users_universal_detector;
  // text_areas_resizeable;
  p->java_enabled = false;
  p->allow_scripts_to_close_windows = false;
  COPY_SETTING(p, swp, remote_fonts_enabled);
  // javascript_can_access_clipboard; TODO disable
  p->xss_auditor_enabled = true;
  COPY_SETTING(p, swp, dns_prefetching_enabled);
  COPY_SETTING(p, swp, local_storage_enabled);
  COPY_SETTING(p, swp, databases_enabled);
  COPY_SETTING(p, swp, application_cache_enabled);
  // tabs_to_links
  p->hyperlink_auditing_enabled = true;
  // is_online
  p->allow_universal_access_from_file_urls = false;
  p->allow_file_access_from_file_urls = false;
  AND_SETTING(p, swp, webaudio_enabled);
  AND_SETTING(p, swp, experimental_webgl_enabled);
  p->pepper_3d_enabled = false;
  p->flash_3d_enabled = false;
  p->flash_stage3d_enabled = false;
  p->flash_stage3d_baseline_enabled = false;
  p->privileged_webgl_extensions_enabled = false;
  // webgl_errors_to_console_enabled
  // mock_scrollbars_enabled
  // layer_squashing_enabled
  // asynchronous_spell_checking_enabled
  // unified_textchecker_enabled
  AND_SETTING(p, swp, accelerated_2d_canvas_enabled);
  AND_SETTING(p, swp, accelerated_filters_enabled);
  // deferred_filters_enabled
  // container_culling_enabled
  // text_blobs_enabled
  COPY_SETTING(p, swp, allow_displaying_insecure_content);
  COPY_SETTING(p, swp, allow_running_insecure_content);
  p->password_echo_enabled = false;
  // should_print_backgrounds
  // should_clear_document_backgorund
  // enable_scroll_animator
  // css_variables_enabled
  // region_Based_columns_enabled
  // touch_enabled
  // device_supports_touch
  // device_support_mouse
  // touch_adjustment_enabled
  // pointer_events_max_touch_points
  // sync_xhr_in_documents_enabled
  // deferred_image_decoding_enabled
  // should_respect_image_orientation
  // number_of_cpu_cores
  // editing_behaviour
  // supports_multiple_windows
  // viewport_enabled
  // viewport_meta_enabled
  // main_frame_resizes_are_orientation_changes
  // initalize_at_minimum_page_scale
  // smart_insert_delete_enabled
  // spatial_navigation_enabled
  // pinch_virtual_viewort_enabled
  // pinch_overlay_Scrollbar_thinkness
  // use_solid_color_scrollbars
  // navigate_on_drag_drop
  // v8_cache_options
  // v8_script_Streaming_Enabled
  COPY_SETTING(p, swp, cookie_enabled);
  p->pepper_accelerated_video_decode_enabled = false;

  // ANDROID
  // text_autosizing_enabled
  // font_scale_factor
  // device_Scale_adjustment
  // force_enable_zoon
  // disallow_fullscreen_for_non_media_elements
  // fullscreen_supported
  // double_tap_to_zoom_enabled
  COPY_SETTING(p, swp, user_gesture_required_for_media_playback);
  // user_gesture_requred_for_media_playback
  // default_video_poster_url
  // supported_deprecated_target_density_dpi
  // use_legacy_background_size_shorthand_behavior
  // wide_viewport_quirk
  // use_wide_viewport
  // force_zero_layout_height
  // viewport_meta_layout_size_quirk
  // viewport_meta_merge_content_quirk
  // viewport_meta_non_user_Scalable_quirk
  // viewport_meta_zero_values_quirk
  // clobber_user_agent_initial_scale_quirk
  // ignore_main_frame_overflow_hidden_quirk
  // report_screen_size_in_physical_pixels_quirk
}

}  // namespace seaturtle
