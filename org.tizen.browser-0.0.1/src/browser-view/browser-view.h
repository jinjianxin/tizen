/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef BROWSER_VIEW_H
#define BROWSER_VIEW_H

#include "browser-common-view.h"
#include "browser-config.h"
#if defined(FEATURE_MOST_VISITED_SITES)
#include "browser-most-visited.h"
#include "most-visited-sites-db.h"
#endif

class Browser_Bookmark_DB;
class Browser_Context_Menu;
class Browser_Class;
class Browser_Find_Word;
class Browser_Multi_Window_View;
class Browser_Scissorbox_View;
class Browser_Settings_Class;
class Most_Visited_Sites;
class Most_Visited_Sites_DB;
class Browser_Window;

/* edit mode state check */
typedef enum _edit_mode {
	BR_NO_EDIT_MODE 	= 0,
	BR_URL_ENTRY_EDIT_MODE,
	BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF,
	BR_FIND_WORD_MODE
} edit_mode;

class Browser_View : public Browser_Common_View {
	friend class Browser_Class;
	friend class Browser_Find_Word;
	friend class Browser_Multi_Window_View;
	friend class Most_Visited_Sites;
	friend class Browser_Scissorbox_View;
public:
	Browser_View(Evas_Object *win, Evas_Object *navi_bar, Evas_Object *bg, Browser_Class *browser);
	~Browser_View(void);

	Eina_Bool init(void);
	void launch(const char *url);
	void set_focused_window(Browser_Window *window
#if defined(FEATURE_MOST_VISITED_SITES)
		, Eina_Bool show_most_visited_sites = EINA_TRUE
#endif
		);
	Browser_Window *get_focused_window(void) { return m_focused_window; }
	void load_url(const char *url);
	string get_title(Browser_Window *window);
	string get_title(void);
	string get_url(void);
	string get_url(Browser_Window *window);
	void return_to_browser_view(Eina_Bool saved_most_visited_sites_item = EINA_FALSE);
	void unset_navigationbar_title_object(Eina_Bool is_unset);

#if defined(HORIZONTAL_UI)
	void rotate(int degree);
	Eina_Bool is_available_to_rotate(void);
#endif
	void pause(void);
	void resume(void);
	void reset(void);
	void set_full_sreen(Eina_Bool enable);

	void suspend_ewk_view(Evas_Object *ewk_view);
	void resume_ewk_view(Evas_Object *ewk_view);

	void set_edit_mode(edit_mode mode) { m_edit_mode = mode; }
#if defined(FEATURE_MOST_VISITED_SITES)
	Eina_Bool is_most_visited_sites_running(void);
#endif
	Evas_Object *get_favicon(const char *url);
	void delete_non_user_created_windows(void);
	Eina_Bool _is_loading(void);
	Eina_Bool launch_find_word_with_text(const char *text_to_find);
private:
	typedef enum _homepage_mode {
#if defined(FEATURE_MOST_VISITED_SITES)
		BR_START_MODE_MOST_VISITED_SITES	= 0,
#endif
		BR_START_MODE_RECENTLY_VISITED_SITE,
		BR_START_MODE_CUSTOMIZED_URL,
		BR_START_MODE_EMPTY_PAGE,
		BR_START_MODE_UNKOWN
	} homepage_mode;

	/* ewk view event callback functions. */
	static void __create_window_cb(void *data, Evas_Object *obj, void *event_info);
	static void __close_window_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_started_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_progress_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __did_finish_load_for_frame_cb(void *data, Evas_Object *obj, void *event_info);
	static void __process_crashed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_committed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_nonempty_layout_finished_cb(void *data, Evas_Object *obj, void *event_info);

	static void __url_editfield_share_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	/* imf event callback functions */
	static void __url_entry_imf_event_cb(void *data, Ecore_IMF_Context *ctx, int value);
	static void __find_word_entry_imf_event_cb(void *data, Ecore_IMF_Context *ctx, int value);

	/* edje object event callback functions */
	static void __url_entry_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
	static void __refresh_button_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
	static void __context_menu_bg_mouse_down_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

	/* evas event callback functions */
	static void __url_entry_focus_out_cb(void *data, Evas *e, void *event_info);

	/* ewk view evas object event callback functions */
	static void __ewk_view_mouse_down_cb(void* data, Evas* evas, Evas_Object* obj, void* ev);
	static void __ewk_view_mouse_up_cb(void* data, Evas* evas, Evas_Object* obj, void* ev);

	/* idler callback functions */
#if defined(HORIZONTAL_UI)
	static Eina_Bool __rotate_multi_window_cb(void *data);
#endif
	static Eina_Bool __webview_layout_resize_idler_cb(void *data);
#if defined(FEATURE_MOST_VISITED_SITES)
	static Eina_Bool __capture_most_visited_sites_screen_shot_idler_cb(void *data);
#endif
	static Eina_Bool __close_window_idler_cb(void *data);
	static Eina_Bool __scroller_bring_in_idler_cb(void *data);
	static Eina_Bool __show_scissorbox_view_idler_cb(void *data);

	void _update_back_forward_buttons(void);

	static void __backward_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __forward_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	/* evas object smart callback functions */
	static void __ewk_view_edge_top_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ewk_view_scroll_down_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ewk_view_scroll_up_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ewk_view_enter_full_screen_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ewk_view_exit_full_screen_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ewk_view_vibration_vibrate_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ewk_view_vibration_cancel_cb(void *data, Evas_Object *obj, void *event_info);

	static void __scroller_resize_cb(void* data, Evas* evas, Evas_Object* obj, void* ev);
	static void __url_layout_mouse_down_cb(void *data, Evas* evas, Evas_Object *obj,
										void *event_info);

	/* elementary event callback functions */
	static void __title_back_button_clicked_cb(void *data , Evas_Object *obj, void *event_info);
	static void __url_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info);
	static void __find_word_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info);
	static void __find_word_entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __go_to_bookmark_cb(void *data, Evas_Object *obj, void *event_info);
	static void __backward_cb(void *data, Evas_Object *obj, void *event_info);
	static void __forward_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_bookmark_cb(void *data, Evas_Object *obj, void *event_info);
	static void __multi_window_cb(void *data, Evas_Object *obj, void *event_info);
	static void __new_window_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_cb(void *data, Evas_Object *obj, void *event_info);
	static void __scroller_scroll_cb(void *data, Evas_Object *obj, void *event_info);
	static void __send_via_message_cb(void *data, Evas_Object *obj, void *event_info);
	static void __download_manager_cb(void *data, Evas_Object *obj, void *event_info);
	static void __send_via_email_cb(void *data, Evas_Object *obj, void *event_info);
	static void __post_to_sns_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_context_popup_dismissed_cb(void *data, Evas_Object *obj,
									void *event_info);
	static void __url_entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __internet_settings_cb(void *data, Evas_Object *obj, void *event_info);
	static void __find_word_cb(void *data, Evas_Object *obj, void *event_info);
	static void __find_word_cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __find_word_prev_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __find_word_next_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clean_up_windows_test_cb(void *data, Evas_Object *obj, void *event_info);
#ifdef ZOOM_BUTTON
	static void __zoom_in_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __zoom_out_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __zoom_button_timeout_cb(void *data);
	Eina_Bool _create_zoom_buttons(void);
#endif
	static void __expand_option_header_cb(void *data, Evas_Object *obj, void *event_info);
	static void __private_cb(void *data, Evas_Object *obj, void *event_info);
	static void __private_check_change_cb(void *data, Evas_Object *obj, void *event_info);
	static void __bookmark_cb(void *data, Evas_Object *obj, void *event_info);
	static void __bookmark_icon_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __option_header_url_layout_mouse_down_cb(void *data, Evas* evas,
								Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info);
	static void __dim_area_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

	static void __find_word_erase_button_clicked_cb(void *data, Evas_Object *obj,
								const char *emission, const char *source);

	/* elm transit callback functions */
	static void __new_window_transit_finished_cb(void *data, Elm_Transit *transit);

	/* normal member functions */
	Eina_Bool _create_main_layout(void);
	Evas_Object *_create_url_layout(void);
	Evas_Object *_create_option_header_url_layout(void);
	Evas_Object *_create_control_bar(void);
	Eina_Bool _show_more_context_popup(void);
	void _stop_loading(void);
	void _reload(void);
	void _set_navigationbar_title(const char *title);
	void _set_url_entry(const char *url, Eina_Bool set_secrue_icon = EINA_TRUE);
	Eina_Bool _is_option_header_expanded(void);
	void _set_controlbar_back_forward_status(void);
	void _navigationbar_title_clicked(void);
	void _load_start(void);
	void _load_finished(void);
	void _set_multi_window_controlbar_text(int count);
	/* get activated url entry, if the option header is expanded, return url entry in option header */
	Evas_Object *_get_activated_url_entry(void);
	void _set_edit_mode(edit_mode mode);
	edit_mode _get_edit_mode(void) { return m_edit_mode; }
	homepage_mode _get_homepage_mode(void) { return m_homepage_mode; }
	/* set homepage from homepage vconf */
	void _set_homepage_mode(void);
	void _set_secure_icon(void);
	Eina_Bool _set_favicon(void);
	Eina_Bool _show_brightness_popup(void);
	Eina_Bool _call_download_manager(void);
	Eina_Bool _call_internet_settings(void);
	void _pop_other_views(void);
#if defined(FEATURE_MOST_VISITED_SITES)
	Eina_Bool _show_most_visited_sites(Eina_Bool is_show);
	Eina_Bool _capture_most_visited_sites_item_screen_shot(void);
#endif
	Eina_Bool _show_new_window_effect(Evas_Object *current_ewk_view,
							Evas_Object *new_ewk_view);
#if defined(HORIZONTAL_UI)
	void _rotate_multi_window(void);
#endif
	void _destroy_more_context_popup(void);
	void _enable_browser_scroller_scroll(void);
	void _enable_webview_scroll(void);
	void _navigationbar_visible_set_signal(Eina_Bool visible);
	void _navigationbar_visible_set(Eina_Bool visible);
	Eina_Bool _navigationbar_visible_get(void);
	Evas_Object *_create_find_word_layout(void);
	void _update_find_word_index_text(const char *index_text, int index, int max_match_cnt);
	void _jump_to_top(void);
	void _hide_scroller_url_layout(void);
	Eina_Bool _show_scissorbox_view(void);
	void _destroy_scissorbox_view(void);
	Eina_Bool _haptic_device_init(void);
	Eina_Bool _haptic_device_play(long haptic_play_time);
	Eina_Bool _haptic_device_stop(void);
	Eina_Bool _haptic_device_close(void);
	Evas_Object *_create_brightness_control_bar(void);
	Eina_Bool _show_brightness_layout(void);
	void _destroy_brightness_layout(void);

	Evas_Object *m_main_layout;
	Evas_Object *m_scroller;
	Evas_Object *m_content_box;
	Evas_Object *m_dummy_loading_progressbar;
	Evas_Object *m_conformant;
	Evas_Object *m_title_back_button;

	/* url layout member variables */
	Evas_Object *m_url_layout;
	Evas_Object *m_url_entry_layout;
	Evas_Object *m_url_edit_field;
	Evas_Object *m_cancel_button;
	Evas_Object *m_url_progressbar;
	Evas_Object *m_url_progresswheel;
	Evas_Object *m_option_header_layout;
	Evas_Object *m_find_word_entry_layout;
	Evas_Object *m_find_word_edit_field;
	Evas_Object *m_find_word_cancel_button;
	Evas_Object *m_find_word_prev_button;
	Evas_Object *m_find_word_next_button;
	Evas_Object *m_option_header_find_word_layout;

	/* url layout which is inserted to navigation bar option header member variables */
	Evas_Object *m_option_header_url_layout;
	Evas_Object *m_option_header_url_entry_layout;
	Evas_Object *m_option_header_url_edit_field;
	Evas_Object *m_option_header_cancel_button;
	Evas_Object *m_option_header_url_progressbar;
	Evas_Object *m_option_header_url_progresswheel;

	Evas_Object *m_multi_window_title_button;

	/* control bar member variables */
	Evas_Object *m_control_bar;
//	Elm_Object_Item *m_backward_button;
	Elm_Object_Item *m_forward_button;
	Elm_Object_Item *m_add_bookmark_button;
	Elm_Object_Item *m_more_button;
	Elm_Object_Item *m_multi_window_button;
	Elm_Object_Item *m_share_controlbar_button;
	Elm_Object_Item *m_new_window_button;
	Evas_Object *m_back_button;

	/* state check */
	edit_mode m_edit_mode;
	homepage_mode m_homepage_mode;

	Browser_Window *m_focused_window;
	Eina_Bool m_is_scrolling;
	int m_scroller_region_y;
	Evas_Object *m_more_context_popup;
#if defined(FEATURE_MOST_VISITED_SITES)
	Most_Visited_Sites *m_most_visited_sites;
	Most_Visited_Sites_DB *m_most_visited_sites_db;
	/* To update the clicked most visited sites item info such as snapshot, url, title. */
	Most_Visited_Sites_DB::most_visited_sites_entry m_selected_most_visited_sites_item_info;
#endif
	Browser_Settings_Class *m_browser_settings;

	Elm_Transit *m_new_window_transit;
	Browser_Window *m_created_new_window;

	std::string m_last_visited_url;

	Elm_Object_Item *m_navi_it;
	Browser_Find_Word *m_find_word;
	Browser_Context_Menu *m_context_menu;
	Browser_Scissorbox_View *m_scissorbox_view;
#if defined(HORIZONTAL_UI)
	int m_rotate_degree;
	Ecore_Timer *m_multi_window_rotate_timer;
	Eina_Bool m_is_rotated;
#endif
	Eina_Bool m_is_full_screen;
#ifdef ZOOM_BUTTON
	Evas_Object *m_zoom_in_button;
	Evas_Object *m_zoom_out_button;
	Ecore_Timer *m_zoom_button_timer;
#endif

	Evas_Object *m_title_backward_button;
	Evas_Object *m_title_forward_button;
	Evas_Object *m_option_header_title_backward_button;
	Evas_Object *m_option_header_title_forward_button;

	Ecore_Idler *m_resize_idler;
	Eina_Bool m_is_private;
	Eina_Bool m_is_private_item_pressed;
	Evas_Object *m_private_check;
	Evas_Object *m_bookmark_on_off_icon;

	Evas_Object *m_brightness_control_bar;
	Elm_Object_Item *m_brightness_auto_it;
	Elm_Object_Item *m_brightness_manual_it;

	Elm_Object_Item *m_default_seg_it;
	Elm_Object_Item *m_low_seg_it;
	Elm_Object_Item *m_medium_seg_it;
	Elm_Object_Item *m_high_seg_it;

	int m_vibration_device_handle_id;

	Eina_Bool m_is_multi_window_grid_mode;
};
#endif /* BROWSER_VIEW_H */

