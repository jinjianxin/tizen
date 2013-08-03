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

/**
  *@file browser-view.cpp
  *@brief 创建浏览器器UI
  */

#include "browser-add-to-bookmark-view.h"
#include "browser-bookmark-view.h"
#include "browser-class.h"
#include "browser-config.h"
#include "browser-context-menu.h"
#include "browser-history-db.h"
#include "browser-find-word.h"
#include "browser-multi-window-view.h"
#include "browser-scissorbox-view.h"
#include "browser-settings-class.h"
#if defined(FEATURE_MOST_VISITED_SITES)
#include "most-visited-sites.h"
#include "most-visited-sites-db.h"
#endif
#include "browser-string.h"
#include "browser-view.h"
#include "browser-window.h"
#include <devman.h>

extern "C" {
#include <shortcut.h>
}

Browser_View::Browser_View(Evas_Object *win, Evas_Object *navi_bar, Evas_Object *bg, Browser_Class *browser)
:
	m_main_layout(NULL)
	,m_scroller(NULL)
	,m_content_box(NULL)
	,m_dummy_loading_progressbar(NULL)
	,m_conformant(NULL)
	,m_url_layout(NULL)
	,m_url_entry_layout(NULL)
	,m_url_edit_field(NULL)
	,m_url_progressbar(NULL)
	,m_url_progresswheel(NULL)
	,m_cancel_button(NULL)
	,m_control_bar(NULL)
	,m_back_button(NULL)
	,m_forward_button(NULL)
	,m_add_bookmark_button(NULL)
	,m_more_button(NULL)
	,m_option_header_url_layout(NULL)
	,m_find_word_entry_layout(NULL)
	,m_find_word_edit_field(NULL)
	,m_option_header_layout(NULL)
	,m_find_word_cancel_button(NULL)
	,m_option_header_find_word_layout(NULL)
	,m_option_header_url_entry_layout(NULL)
	,m_option_header_cancel_button(NULL)
	,m_find_word_prev_button(NULL)
	,m_find_word_next_button(NULL)
	,m_option_header_url_progressbar(NULL)
	,m_option_header_url_progresswheel(NULL)
	,m_edit_mode(BR_NO_EDIT_MODE)
	,m_homepage_mode(BR_START_MODE_UNKOWN)
#if defined(HORIZONTAL_UI)
	,m_rotate_degree(0)
	,m_multi_window_rotate_timer(NULL)
	,m_is_rotated(EINA_FALSE)
#endif
	,m_scroller_region_y(0)
	,m_is_scrolling(EINA_FALSE)
	,m_focused_window(NULL)
	,m_more_context_popup(NULL)
#if defined(FEATURE_MOST_VISITED_SITES)
	,m_most_visited_sites(NULL)
	,m_most_visited_sites_db(NULL)
#endif
	,m_new_window_transit(NULL)
	,m_created_new_window(NULL)
	,m_browser_settings(NULL)
	,m_navi_it(NULL)
	,m_find_word(NULL)
	,m_scissorbox_view(NULL)
	,m_share_controlbar_button(NULL)
	,m_context_menu(NULL)
	,m_multi_window_button(NULL)
	,m_new_window_button(NULL)
	,m_is_full_screen(EINA_FALSE)
#ifdef ZOOM_BUTTON
	,m_zoom_in_button(NULL)
	,m_zoom_out_button(NULL)
	,m_zoom_button_timer(NULL)
#endif
	,m_title_backward_button(NULL)
	,m_title_forward_button(NULL)
	,m_option_header_title_backward_button(NULL)
	,m_option_header_title_forward_button(NULL)
	,m_resize_idler(NULL)
	,m_is_private(EINA_FALSE)
	,m_private_check(NULL)
	,m_is_private_item_pressed(EINA_FALSE)
	,m_bookmark_on_off_icon(NULL)
	,m_brightness_control_bar(NULL)
	,m_brightness_auto_it(NULL)
	,m_brightness_manual_it(NULL)
	,m_default_seg_it(NULL)
	,m_low_seg_it(NULL)
	,m_medium_seg_it(NULL)
	,m_high_seg_it(NULL)
	,m_vibration_device_handle_id(-1)
	,m_is_multi_window_grid_mode(EINA_FALSE)
{
	BROWSER_LOGD("[%s]", __func__);
	m_win = win;
	m_navi_bar = navi_bar;
	m_bg = bg;
	m_browser = browser;
}

Browser_View::~Browser_View()
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_title_back_button)
		evas_object_del(m_title_back_button);

	ug_destroy_all();

	br_preference_set_str(LAST_VISITED_URL_KEY, m_last_visited_url.c_str());

	m_data_manager->destroy_history_db();
	m_data_manager->destroy_geolocation_db();

	if (m_data_manager) {
		delete m_data_manager;
		m_data_manager = NULL;
	}
#if defined(FEATURE_MOST_VISITED_SITES)
	if (m_most_visited_sites) {
		delete m_most_visited_sites;
		m_most_visited_sites = NULL;
	}
	if (m_most_visited_sites_db) {
		delete m_most_visited_sites_db;
		m_most_visited_sites_db = NULL;
	}
#endif
	if (m_new_window_transit) {
		elm_transit_del(m_new_window_transit);
		m_new_window_transit = NULL;
	}
	if (m_browser_settings) {
		delete m_browser_settings;
		m_browser_settings = NULL;
	}
	if (m_find_word) {
		delete m_find_word;
		m_find_word = NULL;
	}
	if (m_context_menu) {
		delete m_context_menu;
		m_context_menu = NULL;
	}
	if (m_scissorbox_view) {
		delete m_scissorbox_view;
		m_scissorbox_view = NULL;
	}
#if defined(HORIZONTAL_UI)
	if (m_multi_window_rotate_timer) {
		ecore_timer_del(m_multi_window_rotate_timer);
		m_multi_window_rotate_timer = NULL;
	}
#endif
#ifdef ZOOM_BUTTON
	if (m_zoom_in_button)
		evas_object_del(m_zoom_in_button);
	if (m_zoom_out_button)
		evas_object_del(m_zoom_out_button);
	if (m_zoom_button_timer) {
		ecore_timer_del(m_zoom_button_timer);
		m_zoom_button_timer = NULL;
	}
#endif
}

Eina_Bool Browser_View::init(void)
{
	BROWSER_LOGD("[%s]", __func__);
	/* set homepage from homepage vconf */
	_set_homepage_mode();

	m_data_manager = new(nothrow) Browser_Data_Manager;
	if (!m_data_manager) {
		BROWSER_LOGE("new Browser_Data_Manager failed");
		return EINA_FALSE;
	}

	m_data_manager->set_browser_view(this);

	if (!m_data_manager->create_history_db()) {
		BROWSER_LOGE("m_data_manager->create_history_db failed");
		return EINA_FALSE;
	}

	if (!m_data_manager->create_geolocation_db()) {
		BROWSER_LOGE("m_data_manager->create_geolocation_db failed");
		return EINA_FALSE;
	}

#if defined(FEATURE_MOST_VISITED_SITES)
	m_most_visited_sites_db = new(nothrow) Most_Visited_Sites_DB;
	if (!m_most_visited_sites_db) {
		BROWSER_LOGE("new Most_Visited_Sites_DB failed");
		return EINA_FALSE;
	}
#endif

	m_find_word = new(nothrow) Browser_Find_Word(this);
	if (!m_find_word) {
		BROWSER_LOGE("new Browser_Find_Word failed");
		return EINA_FALSE;
	}

	m_context_menu = new(nothrow) Browser_Context_Menu(this);
	if (!m_context_menu) {
		BROWSER_LOGE("new Browser_Context_Menu failed");
		return EINA_FALSE;
	}

	//定义在ug-1/ui-gadget.h文件中
    UG_INIT_EFL(m_win, UG_OPT_INDICATOR_ENABLE);

	char *last_url = NULL;
	if (br_preference_get_str(LAST_VISITED_URL_KEY, &last_url) == false) {
		BROWSER_LOGE("failed to get %s preference", LAST_VISITED_URL_KEY);
		return EINA_FALSE;
	}
	if (last_url) {
		m_last_visited_url = std::string(last_url);
		free(last_url);
	}
	if (_haptic_device_init() == EINA_FALSE) {
		BROWSER_LOGE("Failed to init haptic device");
		return EINA_FALSE;
	}

	/* create brower view layout */
	return _create_main_layout();
}

/**
 * @brief Browser_View::launch 加载网页 被m_browser_view调用
 * @param url 加载的网页
 */
void Browser_View::launch(const char *url)
{
	/* Destroy all other views except browser view. */
	_pop_other_views();

	/*Workaround.
	  * When keypad is running via url entry, if browser goes to background by home key.
	  * Then relaunch the browser by aul. The webpage is loading but the keypad is still running.
	  * So give focus to cancel button not to invoke the keypad. */
	elm_object_focus_set(m_cancel_button, EINA_TRUE);

	if (url && strlen(url))
		load_url(url);
#if defined(FEATURE_MOST_VISITED_SITES)
	else if (m_homepage_mode == BR_START_MODE_MOST_VISITED_SITES)
		load_url(BROWSER_MOST_VISITED_SITES_URL);
#endif
	else if (m_homepage_mode == BR_START_MODE_RECENTLY_VISITED_SITE) {
		char *homepage = NULL;
		if (br_preference_get_str(LAST_VISITED_URL_KEY, &homepage) == false) {
			BROWSER_LOGD("failed to get %s preference", LAST_VISITED_URL_KEY);
			return;
		}
		if (homepage) {
			load_url(homepage);
			free(homepage);
		} else
#if defined(FEATURE_MOST_VISITED_SITES)
			load_url(BROWSER_MOST_VISITED_SITES_URL);
#else
			load_url(BROWSER_BLANK_PAGE_URL);
#endif
	} else if (m_homepage_mode == BR_START_MODE_CUSTOMIZED_URL) {
		char *user_homepage = NULL;
		if (br_preference_get_str(USER_HOMEPAGE_KEY, &user_homepage) ==false) {
			BROWSER_LOGE("failed to get %s preference\n", USER_HOMEPAGE_KEY);
			return;
		}
		if (user_homepage) {
			load_url(user_homepage);
			free(user_homepage);
		} else
#if defined(FEATURE_MOST_VISITED_SITES)
			load_url(BROWSER_MOST_VISITED_SITES_URL);
#else
			load_url(BROWSER_BLANK_PAGE_URL);
#endif
	} else if (m_homepage_mode == BR_START_MODE_EMPTY_PAGE) {
		load_url(BROWSER_BLANK_PAGE_URL);
	}

}

/**
 * @brief Browser_View::set_full_sreen 设置全屏
 * @param enable 是否全屏
 */
void Browser_View::set_full_sreen(Eina_Bool enable)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_is_full_screen == enable)
		return;

	m_is_full_screen = enable;

	if (m_is_full_screen) {
		elm_win_indicator_mode_set(m_win, ELM_WIN_INDICATOR_HIDE);
		_hide_scroller_url_layout();
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "enter,full_screen,signal", "");
		_enable_webview_scroll();

		elm_object_part_content_unset(m_focused_window->m_ewk_view_layout, "elm.swallow.webview");

		evas_object_move(m_focused_window->m_ewk_view, 0, 0);
	} else {
		elm_win_indicator_mode_set(m_win, ELM_WIN_INDICATOR_SHOW);

		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "exit,full_screen,signal", "");

		elm_object_part_content_set(m_focused_window->m_ewk_view_layout, "elm.swallow.webview", m_focused_window->m_ewk_view);

		_navigationbar_visible_set_signal(EINA_TRUE);
	}

	ecore_idler_add(__webview_layout_resize_idler_cb, this);
}

#if defined(FEATURE_MOST_VISITED_SITES)
Eina_Bool Browser_View::is_most_visited_sites_running(void)
{
	if (m_most_visited_sites)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}
#endif

void Browser_View::return_to_browser_view(Eina_Bool saved_most_visited_sites_item)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!_is_loading()) {
		if (get_title().empty())
			_set_navigationbar_title(get_url().c_str());
		else
			_set_navigationbar_title(get_title().c_str());
	}
#if defined(FEATURE_MOST_VISITED_SITES)
	if (is_most_visited_sites_running()) {
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
					"show,control_bar,no_animation,signal", "");
		_set_navigationbar_title(BR_STRING_MOST_VISITED_SITES);
	}
	if (saved_most_visited_sites_item && is_most_visited_sites_running())
		/* If longpress on speed dial item, then add to bookmark. */
		edje_object_signal_emit(elm_layout_edje_get(m_most_visited_sites->m_selected_item->layout),
										"bookmark_icon", "");
#endif
	_set_secure_icon();

}

void Browser_View::_pop_other_views(void)
{
	BROWSER_LOGD("[%s]", __func__);
	_destroy_more_context_popup();

	/* if browser is runing background behind ug which browser invoked,
	destroy the previous ug and show the browser view. */
	ug_destroy_all();

	/* If multi-window is running, destroy it to show browser view. */
	if (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW))
		m_data_manager->get_multi_window_view()->close_multi_window();

	/* Pop all other views except browser view. */
	elm_naviframe_item_pop_to(m_navi_it);

	if (m_scissorbox_view)
		_destroy_scissorbox_view();
}

/* set homepage from homepage vconf */
void Browser_View::_set_homepage_mode(void)
{
	char *homepage = NULL;
	if (br_preference_get_str(HOMEPAGE_KEY, &homepage) == false) {
		BROWSER_LOGE("failed to get %s preference\n", HOMEPAGE_KEY);
		return;
	}
	BROWSER_LOGD("homepage=[%s]", homepage);

	if (!homepage) {
		BROWSER_LOGE("homepage is null");
#if defined(FEATURE_MOST_VISITED_SITES)
		m_homepage_mode = BR_START_MODE_MOST_VISITED_SITES;
#else
		m_homepage_mode = BR_START_MODE_EMPTY_PAGE;
#endif
		return;
	}

#if defined(FEATURE_MOST_VISITED_SITES)
	if (!strncmp(homepage, MOST_VISITED_SITES,
				strlen(MOST_VISITED_SITES)))
		m_homepage_mode = BR_START_MODE_MOST_VISITED_SITES;
	else
#endif
	if (!strncmp(homepage, RECENTLY_VISITED_SITE,
			strlen(RECENTLY_VISITED_SITE)))
		m_homepage_mode = BR_START_MODE_RECENTLY_VISITED_SITE;
	else if (!strncmp(homepage, EMPTY_PAGE,
			strlen(EMPTY_PAGE)))
		m_homepage_mode = BR_START_MODE_EMPTY_PAGE;
	else
		m_homepage_mode = BR_START_MODE_CUSTOMIZED_URL;

	free(homepage);
	BROWSER_LOGD("m_homepage_mode=%d", m_homepage_mode);
}

void Browser_View::__new_window_transit_finished_cb(void *data, Elm_Transit *transit)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	if (!browser_view->m_new_window_transit)
		return;

	browser_view->m_new_window_transit = NULL;
	browser_view->m_browser->set_focused_window(browser_view->m_created_new_window
#if defined(FEATURE_MOST_VISITED_SITES)
			, EINA_FALSE
#endif
			);
	browser_view->m_created_new_window = NULL;
}

Eina_Bool Browser_View::_show_new_window_effect(Evas_Object *current_ewk_view,
								Evas_Object *new_ewk_view)
{
	int scroller_x = 0;
	int scroller_y = 0;
	int scroller_w = 0;
	int scroller_h = 0;
	elm_scroller_region_get(m_scroller, &scroller_x, &scroller_y, &scroller_w, &scroller_h);
	elm_scroller_region_show(m_scroller ,scroller_x, 0, scroller_w, scroller_h);

	m_browser->set_focused_window(m_created_new_window
#if defined(FEATURE_MOST_VISITED_SITES)
		, EINA_FALSE
#endif
		);

	return EINA_TRUE;
}

Evas_Object *Browser_View::get_favicon(const char *url)
{
	if (!url || !strlen(url))
		return NULL;

	Ewk_Context* context = ewk_view_context_get(m_focused_window->m_ewk_view);
	Evas_Object *favicon = ewk_context_icon_database_icon_object_add(context, url, evas_object_evas_get(m_focused_window->m_ewk_view));

	return favicon;
}

Eina_Bool Browser_View::_set_favicon(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_edit_mode == BR_URL_ENTRY_EDIT_MODE
	    || m_edit_mode == BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF) {
		BROWSER_LOGD("Edit mode");
		return EINA_FALSE;
	}

#if defined(FEATURE_MOST_VISITED_SITES)
	if (is_most_visited_sites_running()) {
		BROWSER_LOGD("is_most_visited_sites_running");
		return EINA_FALSE;
	}
#endif

	double progress = ewk_view_load_progress_get(m_focused_window->m_ewk_view);
	if (progress < 1.0f && progress > 0.05f) {
		BROWSER_LOGD("loadin status");
		return EINA_FALSE;
	}

	Evas_Object *favicon = get_favicon(get_url().c_str());
	Evas_Object *option_header_favicon = get_favicon(get_url().c_str());

	if (favicon) {
		if (m_focused_window->m_favicon)
			evas_object_del(m_focused_window->m_favicon);

		m_focused_window->m_favicon = favicon;
	} else {
		Evas_Object *default_icon = elm_icon_add(m_focused_window->m_ewk_view);
		if (!default_icon) {
			BROWSER_LOGE("elm_icon_add is failed.");
			return EINA_FALSE;
		}
		if (!elm_icon_file_set(default_icon, BROWSER_IMAGE_DIR"/faviconDefault.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.");
			return EINA_FALSE;
		}
		evas_object_size_hint_aspect_set(default_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		if (m_focused_window->m_favicon)
			evas_object_del(m_focused_window->m_favicon);

		m_focused_window->m_favicon = default_icon;
	}

	if (option_header_favicon) {
		if (m_focused_window->m_option_header_favicon)
			evas_object_del(m_focused_window->m_option_header_favicon);

		m_focused_window->m_option_header_favicon = option_header_favicon;
	} else {
		Evas_Object *default_icon = elm_icon_add(m_focused_window->m_ewk_view);
		if (!default_icon) {
			BROWSER_LOGE("elm_icon_add is failed.");
			return EINA_FALSE;
		}
		if (!elm_icon_file_set(default_icon, BROWSER_IMAGE_DIR"/faviconDefault.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.");
			return EINA_FALSE;
		}
		evas_object_size_hint_aspect_set(default_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		if (m_focused_window->m_option_header_favicon)
			evas_object_del(m_focused_window->m_option_header_favicon);

		m_focused_window->m_option_header_favicon = default_icon;
	}

	if (elm_object_part_content_get(m_url_entry_layout, "elm.swallow.favicon"))
		elm_object_part_content_unset(m_url_entry_layout, "elm.swallow.favicon");
	if (elm_object_part_content_get(m_option_header_url_entry_layout,
						"elm.swallow.favicon"))
		elm_object_part_content_unset(m_option_header_url_entry_layout,
						"elm.swallow.favicon");

	elm_object_part_content_set(m_url_entry_layout, "elm.swallow.favicon",
							m_focused_window->m_favicon);
	elm_object_part_content_set(m_option_header_url_entry_layout, "elm.swallow.favicon",
							m_focused_window->m_option_header_favicon);

	edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout),
								"show,favicon,signal", "");
	edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
								"show,favicon,signal", "");

	_set_secure_icon();

	return EINA_TRUE;
}

Eina_Bool Browser_View::__close_window_idler_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	Browser_View *browser_view = (Browser_View *)data;
	Browser_Class *browser = browser_view->m_browser;

	if (browser_view->m_focused_window->m_parent)
		browser->delete_window(browser_view->m_focused_window,
				browser_view->m_focused_window->m_parent);
	else
		browser->delete_window(browser_view->m_focused_window);

	int scroller_x = 0;
	int scroller_w = 0;
	int scroller_h = 0;
	elm_scroller_region_get(browser_view->m_scroller, &scroller_x, NULL, &scroller_w, &scroller_h);
	elm_scroller_region_show(browser_view->m_scroller ,scroller_x, 0,
								scroller_w, scroller_h);
	return ECORE_CALLBACK_CANCEL;
}

void Browser_View::__close_window_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Browser_Class *browser = browser_view->m_browser;

	/* The window should be deleted by idler. The webkit recommands to delete by idler. */
	ecore_idler_add(__close_window_idler_cb, browser_view);
}

Eina_Bool Browser_View::_haptic_device_init(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_vibration_device_handle_id == BROWSER_HAPTIC_DEVICE_HANDLE) {
		BROWSER_LOGD("device handle for haptic is already set");
		return EINA_FALSE;
	}

	if (haptic_initialize() != HAPTIC_ERROR_NONE) {
		BROWSER_LOGD("failed to initiate haptic device");
		return EINA_FALSE;
	}

	m_vibration_device_handle_id = BROWSER_HAPTIC_DEVICE_HANDLE;

	return EINA_TRUE;
}

Eina_Bool Browser_View::_haptic_device_play(long haptic_play_time)
{
	BROWSER_LOGD("[%s], requested time is [%dms]", __func__, haptic_play_time);

	if (m_vibration_device_handle_id != BROWSER_HAPTIC_DEVICE_HANDLE) {
		BROWSER_LOGD("device handle for haptic is not set. Unable to play haptic");
		return EINA_FALSE;
	}

	if (haptic_vibrate_monotone(BROWSER_HAPTIC_DEVICE_HANDLE, haptic_play_time, HAPTIC_LEVEL_AUTO) != HAPTIC_ERROR_NONE) {
		BROWSER_LOGD("haptic_vibrate_monotone failed.");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool Browser_View::_haptic_device_stop(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_vibration_device_handle_id != BROWSER_HAPTIC_DEVICE_HANDLE) {
		BROWSER_LOGD("device handle for haptic is not set. Unable to play haptic");
		return EINA_FALSE;
	}

	if (haptic_stop_device(BROWSER_HAPTIC_DEVICE_HANDLE) != HAPTIC_ERROR_NONE) {
		BROWSER_LOGD("haptic_stop_device failed.");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool Browser_View::_haptic_device_close(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (haptic_deinitialize() != HAPTIC_ERROR_NONE) {
		BROWSER_LOGD("haptic_deinitialize failed.");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

/**
 * @brief Browser_View::__create_window_cb  在网页中打开新的标签页
 * @param data 所带参数
 * @param obj 信号发送对象
 * @param event_info   信号自身所带信息
 */
void Browser_View::__create_window_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Browser_Class *browser = browser_view->m_browser;

	std::vector<Browser_Window *> window_list = browser->get_window_list();
	if (window_list.size() >= BROWSER_MULTI_WINDOW_MAX_COUNT) {
		/* If the multi window is max, delete the first one. */
		/* if the first window is focused, delete second one(oldest one except first one) */
		if (browser_view->m_focused_window != window_list[0])
			browser->delete_window(window_list[0]);
		else
			browser->delete_window(window_list[1]);
	}

	browser_view->m_created_new_window = browser->create_new_window();
	if (!browser_view->m_created_new_window) {
		BROWSER_LOGE("create_new_window failed");
		return;
	}
	/* initialize the created webview first to connect ewk event callback functions such as load start, progress etc. */
	browser->ewk_view_init(browser_view->m_created_new_window->m_ewk_view);

	/* Set the caller window. */
	browser_view->m_created_new_window->m_parent = browser_view->m_focused_window;

	/* Destroy previous multi window item snapshot.
	  * This is because the snapshot of multi window item can't be captured in this case. */
#if defined(HORIZONTAL_UI)
	if (browser_view->m_focused_window->m_landscape_snapshot_image) {
		evas_object_del(browser_view->m_focused_window->m_landscape_snapshot_image);
		browser_view->m_focused_window->m_landscape_snapshot_image = NULL;
	}
#endif
	if (browser_view->m_focused_window->m_portrait_snapshot_image) {
		evas_object_del(browser_view->m_focused_window->m_portrait_snapshot_image);
		browser_view->m_focused_window->m_portrait_snapshot_image = NULL;
	}

	if (!browser_view->_show_new_window_effect(browser_view->m_focused_window->m_ewk_view,
					browser_view->m_created_new_window->m_ewk_view))
		BROWSER_LOGE("_show_new_window_effect failed");

	*((Evas_Object **)event_info) = browser_view->m_created_new_window->m_ewk_view;
}

void Browser_View::__load_started_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);	
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Evas_Object *url_progressbar = browser_view->m_url_progressbar;
	Evas_Object *progressbar_wheel = browser_view->m_url_progresswheel;
	Evas_Object *option_header_url_progressbar = browser_view->m_option_header_url_progressbar;
	Evas_Object *option_header_progressbar_wheel = browser_view->m_option_header_url_progresswheel;

	double progress = ewk_view_load_progress_get(browser_view->m_focused_window->m_ewk_view);
	if (progress <= 0.0f)
		progress = 0.05f;
	elm_progressbar_pulse(progressbar_wheel, EINA_TRUE);
	elm_progressbar_pulse(option_header_progressbar_wheel, EINA_TRUE);

	elm_progressbar_value_set(url_progressbar, progress);
	elm_progressbar_value_set(option_header_url_progressbar, progress);

	browser_view->_load_start();
}

void Browser_View::_navigationbar_visible_set_signal(Eina_Bool visible)
{
	BROWSER_LOGD("visible=%d", visible);

	if (visible && (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW) || m_is_full_screen || m_brightness_control_bar))
		return;

	if (m_navi_it != elm_naviframe_top_item_get(m_navi_bar))
		return;

	evas_object_data_set(m_navi_bar, "visible", (Eina_Bool *)visible);

	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);

	if (visible)
		elm_object_item_signal_emit(top_it, ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_INSTANT_OPEN);
	else
		elm_object_item_signal_emit(top_it, ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_INSTANT_CLOSE);
}

void Browser_View::_navigationbar_visible_set(Eina_Bool visible)
{
	if (visible && (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW) || m_is_full_screen))
		return;

	if (m_navi_it != elm_naviframe_top_item_get(m_navi_bar))
		return;

	evas_object_data_set(m_navi_bar, "visible", (Eina_Bool *)visible);

	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
	if (visible)
		elm_object_item_signal_emit(top_it, ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_OPEN);
	else
		elm_object_item_signal_emit(top_it, ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_CLOSE);
}

Eina_Bool Browser_View::_navigationbar_visible_get(void)
{
	Eina_Bool* visible = (Eina_Bool *)evas_object_data_get(m_navi_bar, "visible");
	BROWSER_LOGD("visible = %d", visible);
	if (visible == (Eina_Bool *)EINA_TRUE)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

void Browser_View::_load_start(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_dummy_loading_progressbar) {
		elm_object_part_content_unset(m_main_layout, "elm.swallow.waiting_progress");
		evas_object_del(m_dummy_loading_progressbar);
		m_dummy_loading_progressbar = NULL;
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
					"hide,waiting_progressbar,signal", "");
	}

	if (!m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW))
		_set_navigationbar_title(BR_STRING_LOADING);

	edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "loading,on,signal", "");
	edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
				"loading,on,signal", "");

	_navigationbar_visible_set_signal(EINA_TRUE);

	/* Hide the secure lock icon in title bar. */
	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_ICON, NULL);

	/* Destroy & hide favicon when load start. */
	edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout),
						"hide,favicon,signal", "");
	edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
						"hide,favicon,signal", "");
	if (elm_object_part_content_get(m_url_entry_layout, "elm.swallow.favicon"))
		elm_object_part_content_unset(m_url_entry_layout, "elm.swallow.favicon");
	if (elm_object_part_content_get(m_option_header_url_entry_layout,
						"elm.swallow.favicon"))
		elm_object_part_content_unset(m_option_header_url_entry_layout,
						"elm.swallow.favicon");

	if (m_focused_window->m_favicon) {
		evas_object_del(m_focused_window->m_favicon);
		m_focused_window->m_favicon = NULL;
	}
	if (m_focused_window->m_option_header_favicon) {
		evas_object_del(m_focused_window->m_option_header_favicon);
		m_focused_window->m_option_header_favicon = NULL;
	}

	/* This makes the ewk view unfocused so that the ime can't be invoked. */
	evas_object_focus_set(m_focused_window->m_ewk_view_layout, EINA_TRUE);

	if (_get_edit_mode() != BR_NO_EDIT_MODE)
		_set_edit_mode(BR_NO_EDIT_MODE);

	/* For deleted window because of unused case like low memory. */
	m_focused_window->m_url.clear();
	m_focused_window->m_title.clear();

	set_full_sreen(EINA_FALSE);

	elm_object_focus_set(m_option_header_cancel_button, EINA_TRUE);
	elm_object_signal_emit(m_option_header_url_edit_field, "ellipsis_show,signal", "elm");
	elm_object_signal_emit(m_url_edit_field, "ellipsis_show,signal", "elm");

	if (m_scissorbox_view)
		_destroy_scissorbox_view();
	_destroy_more_context_popup();
}

void Browser_View::__load_committed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	if (browser_view->_get_edit_mode() != BR_NO_EDIT_MODE)
		return;

	browser_view->_set_url_entry(browser_view->get_url().c_str());
}

void Browser_View::__load_nonempty_layout_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	if (!browser_view->_is_loading())
		return;

#ifdef BROWSER_SCROLLER_BOUNCING
	/* If the first content is displayed, hide the url layout in browser scroller like safari. */
	int scroller_x = 0;
	int scroller_w = 0;
	int scroller_h = 0;
	elm_scroller_region_get(browser_view->m_scroller, &scroller_x, NULL, &scroller_w, &scroller_h);
	elm_scroller_region_show(browser_view->m_scroller ,scroller_x, 0,
								scroller_w, scroller_h);
#endif
}

void Browser_View::__title_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	if (browser_view->_get_edit_mode() != BR_NO_EDIT_MODE)
		return;

	if (!m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW))
		browser_view->_set_navigationbar_title(browser_view->get_title().c_str());
}

void Browser_View::__load_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Evas_Object *progressbar_wheel = browser_view->m_url_progresswheel;
	Evas_Object *option_header_progressbar_wheel = browser_view->m_option_header_url_progresswheel;

	elm_progressbar_pulse(progressbar_wheel, EINA_FALSE);
	elm_progressbar_pulse(option_header_progressbar_wheel, EINA_FALSE);

	/*
	* For the first time, the background color is white initially.
	* If the background is not displayed yet, show the grey background.
	* This code is executed only one time at launching time.
	*/
	const char* state = edje_object_part_state_get(elm_layout_edje_get(browser_view->m_main_layout),
							"contents_bg", NULL);
	if(state && !strncmp(state, "default", strlen("default")))
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout),
					"show,grey_background,signal", "");

	if (browser_view->m_edit_mode != BR_URL_ENTRY_EDIT_MODE
	    && browser_view->m_edit_mode != BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF) {
		/* change the url layout for normal mode. (change the reload icon etc) */
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_url_entry_layout), "loading,off,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_option_header_url_entry_layout),
					"loading,off,signal", "");
	}

	if (!m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW))
		browser_view->_set_navigationbar_title(browser_view->get_title().c_str());

	/* This is because,
	  * finish progress event can be come after finish progress frame event at some pages.
	  * In this case, the title & favicon display error happens, becaue these are handled in finish progress frame.
	  * So do that again here. */
	if (!browser_view->_set_favicon())
		BROWSER_LOGE("_set_favicon failed");
	browser_view->_set_secure_icon();
	browser_view->_load_finished();
}

static Eina_Bool __kill_browser_timer_cb(void *data)
{
	elm_exit();
	return ECORE_CALLBACK_CANCEL;
}

void Browser_View::__process_crashed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->show_msg_popup(BR_STRING_WEBPROCESS_CRASH, 3);
	ecore_timer_add(3, __kill_browser_timer_cb, NULL);
}

void Browser_View::__did_finish_load_for_frame_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	if (!m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW))
		browser_view->_set_navigationbar_title(browser_view->get_title().c_str());

	browser_view->_load_finished();
}

/**
 * @brief Browser_View::_set_secure_icon
 * jinjainxin
 */

void Browser_View::_set_secure_icon(void)
{
  std::string url = get_url();
	BROWSER_LOGD("url=[%s]", url.c_str());

	if (m_focused_window->m_secure_icon == NULL) {
		m_focused_window->m_secure_icon = elm_icon_add(m_focused_window->m_ewk_view);
		if (!m_focused_window->m_secure_icon) {
			BROWSER_LOGE("elm_icon_add is failed.");
			return;
		}

		if (!elm_icon_file_set(m_focused_window->m_secure_icon, BROWSER_IMAGE_DIR"/lock.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.");
			return;
		}
		evas_object_size_hint_aspect_set(m_focused_window->m_secure_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		elm_object_part_content_set(m_url_entry_layout, "elm.swallow.secure_icon",
				m_focused_window->m_secure_icon);
	}

	if (m_focused_window->m_option_header_secure_icon == NULL) {
		m_focused_window->m_option_header_secure_icon = elm_icon_add(m_focused_window->m_ewk_view);
		if (!m_focused_window->m_option_header_secure_icon) {
			BROWSER_LOGE("elm_icon_add is failed.");
			return;
		}

		if (!elm_icon_file_set(m_focused_window->m_option_header_secure_icon, BROWSER_IMAGE_DIR"/lock.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.");
			return;
		}
		evas_object_size_hint_aspect_set(m_focused_window->m_option_header_secure_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		elm_object_part_content_set(m_option_header_url_entry_layout, "elm.swallow.secure_icon",
				m_focused_window->m_option_header_secure_icon);
	}

	if (url.c_str() && url.length()) {
		if (!strncmp(url.c_str(), BROWSER_HTTPS_SCHEME, strlen(BROWSER_HTTPS_SCHEME))) {
			edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout),
									"show,secure_icon,signal", "");
			edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
									"show,secure_icon,signal", "");
			return;
		}
	}
	edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "hide,secure_icon,signal", "");
	edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
								"hide,secure_icon,signal", "");

}

#if defined(FEATURE_MOST_VISITED_SITES)
Eina_Bool Browser_View::_capture_most_visited_sites_item_screen_shot(void)
{
	BROWSER_LOGD("[%s]", __func__);

	Eina_Bool is_ranked = EINA_FALSE;
	std::string screen_shot_path;
	std::string current_url = get_url();
	screen_shot_path.clear();

	if (!m_selected_most_visited_sites_item_info.url.empty()) {
		std::string history_id;
		history_id = m_most_visited_sites_db->get_history_id_by_url(get_url().c_str());
		if (!history_id.empty()) {
			std::string history_title;
			history_title = m_most_visited_sites_db->get_history_title_by_id(history_id.c_str());
			if (!history_title.empty()) {
				if (!m_most_visited_sites_db->save_most_visited_sites_item(m_selected_most_visited_sites_item_info.index,
						get_url().c_str(), history_title.c_str(), history_id.c_str()))
					BROWSER_LOGE("save_most_visited_sites_item failed");
			} else {
				if (!m_most_visited_sites_db->save_most_visited_sites_item(m_selected_most_visited_sites_item_info.index,
						get_url().c_str(), m_selected_most_visited_sites_item_info.title.c_str(),
						history_id.c_str()))
					BROWSER_LOGE("save_most_visited_sites_item failed");
			}
		}

		m_selected_most_visited_sites_item_info.url.clear();
		m_selected_most_visited_sites_item_info.title.clear();
		m_selected_most_visited_sites_item_info.id.clear();
	}

	std::vector<Most_Visited_Sites_DB::most_visited_sites_entry> most_visited_sites_list;
	if (!m_most_visited_sites_db->get_most_visited_sites_list(most_visited_sites_list)) {
		BROWSER_LOGE("get_most_visited_sites_list failed");
		return EINA_FALSE;
	}

	for (int i = 0 ; i < most_visited_sites_list.size() ; i++) {
		if (!current_url.empty() && !most_visited_sites_list[i].url.empty()
		     && (current_url.length() == most_visited_sites_list[i].url.length())
		     && !strncmp(most_visited_sites_list[i].url.c_str(), current_url.c_str(), current_url.length())) {
			is_ranked = EINA_TRUE;
			screen_shot_path = std::string(BROWSER_SCREEN_SHOT_DIR)
							+ most_visited_sites_list[i].id;
			if (most_visited_sites_list[i].id.length() > strlen(DEFAULT_ICON_PREFIX) &&
			    !strncmp(DEFAULT_ICON_PREFIX, most_visited_sites_list[i].id.c_str(),
			    				strlen(DEFAULT_ICON_PREFIX))) {
			    	/* If default icon */
				std::string history_id;
				history_id = m_most_visited_sites_db->get_history_id_by_url(most_visited_sites_list[i].url.c_str());
				if (!history_id.empty()) {
					if (m_most_visited_sites_db->save_most_visited_sites_item(most_visited_sites_list[i].index,
						most_visited_sites_list[i].url.c_str(), most_visited_sites_list[i].title.c_str(),
						history_id.c_str())) {
						screen_shot_path = std::string(BROWSER_SCREEN_SHOT_DIR)
									+ most_visited_sites_list[i].id;
						remove(screen_shot_path.c_str());
						screen_shot_path = std::string(BROWSER_SCREEN_SHOT_DIR)
									+ history_id;
					}
				}
			}
			break;
		}
	}

	if (!is_ranked) {
		std::vector<Most_Visited_Sites_DB::most_visited_sites_entry> most_visited_list;
		if (!m_most_visited_sites_db->get_most_visited_list(most_visited_list, most_visited_sites_list,
							BROWSER_MOST_VISITED_SITES_ITEM_MAX * 2)) {
			BROWSER_LOGE("get_most_visited_list failed");
			return EINA_FALSE;
		}
		for (int i = 0; i < most_visited_list.size(); i++)
		{
			if (most_visited_list[i].url == current_url) {
				is_ranked = EINA_TRUE;
				screen_shot_path = std::string(BROWSER_SCREEN_SHOT_DIR)
									+ most_visited_list[i].id;
				break;
			}
		}
	}

	if (is_ranked && screen_shot_path.size() > 0) {
		int focused_ewk_view_w = 0;
		int focused_ewk_view_h = 0;
		evas_object_geometry_get(m_focused_window->m_ewk_view, NULL, NULL,
							&focused_ewk_view_w, &focused_ewk_view_h);

		int window_w = 0;
		evas_object_geometry_get(m_win, NULL, NULL, &window_w, NULL);
		int item_w = window_w / 3;
		int item_h = item_w + 10 * elm_scale_get();

		double ratio = (double)((double)item_h / (double)item_w);

		Eina_Rectangle snapshot_rect;
		snapshot_rect.x = snapshot_rect.y = 0;
		snapshot_rect.w = focused_ewk_view_w;
		snapshot_rect.h = focused_ewk_view_w * ratio;

		Evas_Object *snapshot = ewk_view_screenshot_contents_get(m_focused_window->m_ewk_view,
					snapshot_rect, 0.33, evas_object_evas_get(m_navi_bar));
		if (!snapshot) {
			BROWSER_LOGE("ewk_view_screenshot_contents_get failed");
			return EINA_FALSE;
		}
		uint8_t *pixels = (uint8_t *)(evas_object_image_data_get(snapshot, EINA_TRUE));

		int surface_width = 0;
		int surface_height = 0;
		evas_object_image_size_get(snapshot, &surface_width, &surface_height);
		BROWSER_LOGD("<<<< surface_width = %d, surface_height = %d >>>", surface_width, surface_height);

		cairo_surface_t *snapshot_surface = cairo_image_surface_create_for_data(pixels, CAIRO_FORMAT_RGB24,
								surface_width, surface_height,
								cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, surface_width));
		if (!snapshot_surface) {
			BROWSER_LOGE("cairo_image_surface_create_for_data failed");
			return EINA_FALSE;
		}

		if (cairo_surface_write_to_png(snapshot_surface, screen_shot_path.c_str())
							!= CAIRO_STATUS_SUCCESS)
			BROWSER_LOGD("cairo_surface_write_to_png FAILED");
		cairo_surface_destroy(snapshot_surface);
	}
	return EINA_TRUE;
}

Eina_Bool Browser_View::__capture_most_visited_sites_screen_shot_idler_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	Browser_View *browser_view = (Browser_View *)data;
	if (!browser_view->_capture_most_visited_sites_item_screen_shot())
		BROWSER_LOGE("_capture_most_visited_sites_item_screen_shot failed");

	return ECORE_CALLBACK_CANCEL;
}
#endif

void Browser_View::_load_finished(void)
{
	BROWSER_LOGD("[%s]", __func__);

	/* enable or disable back,forward controlbar buttons */
	_set_controlbar_back_forward_status();

	if (!_set_favicon())
		BROWSER_LOGE("_set_favicon failed");
	_set_secure_icon();

	/* Add current url to history */
	Eina_Bool is_full = EINA_FALSE;

	/* Save last visited url to save this when browser exits. */
	m_last_visited_url = get_url();

	if (m_data_manager->get_history_db() && !m_is_private) {
		m_data_manager->get_history_db()->save_history(m_last_visited_url.c_str(),
							get_title().c_str(), &is_full);
		if (is_full)
			BROWSER_LOGE("history is full, delete the first one");
	}

	_hide_scroller_url_layout();

#if defined(FEATURE_MOST_VISITED_SITES)
	ecore_idler_add(__capture_most_visited_sites_screen_shot_idler_cb, this);
#endif

	elm_object_focus_set(m_option_header_cancel_button, EINA_TRUE);
	elm_object_signal_emit(m_option_header_url_edit_field, "ellipsis_show,signal", "elm");
	elm_object_signal_emit(m_url_edit_field, "ellipsis_show,signal", "elm");

	_update_back_forward_buttons();
}

void Browser_View::_hide_scroller_url_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (_get_edit_mode() != BR_NO_EDIT_MODE
#if defined(FEATURE_MOST_VISITED_SITES)
	    || is_most_visited_sites_running()
#endif
	)
		return;

	_navigationbar_visible_set_signal(EINA_FALSE);

	int browser_scroller_x = 0;
	int browser_scroller_w = 0;
	int browser_scroller_h = 0;
	elm_scroller_region_get(m_scroller, &browser_scroller_x, NULL,
					&browser_scroller_w, &browser_scroller_h);

	int url_layout_h = 0;
	evas_object_geometry_get(m_url_layout, NULL, NULL, NULL, &url_layout_h);
	elm_scroller_region_show(m_scroller, browser_scroller_x, url_layout_h,
					browser_scroller_w, browser_scroller_h);
}

void Browser_View::__load_progress_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Evas_Object *url_progressbar = browser_view->m_url_progressbar;
	Evas_Object *option_header_url_progressbar = browser_view->m_option_header_url_progressbar;

	double progress = ewk_view_load_progress_get(browser_view->m_focused_window->m_ewk_view);
	elm_progressbar_value_set(url_progressbar, progress);
	elm_progressbar_value_set(option_header_url_progressbar, progress);

	if (progress == 1.0f)
		__load_finished_cb(data, obj, NULL);
}

void Browser_View::__ewk_view_mouse_down_cb(void* data, Evas* evas, Evas_Object* obj, void* ev)
{
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Evas_Event_Mouse_Down event = *(Evas_Event_Mouse_Down *)ev;
	Evas_Object *main_layout = browser_view->m_main_layout;
	Evas_Object *ewk_view = browser_view->m_focused_window->m_ewk_view;

	browser_view->m_context_menu->set_pressed_position_x(event.output.x);
	browser_view->m_context_menu->set_pressed_position_y(event.output.y);

#if defined(FEATURE_MOST_VISITED_SITES)
	if (browser_view->is_most_visited_sites_running())
		return;
#endif

#ifdef BROWSER_SCROLLER_BOUNCING
	/* activate webview firstly when user starts to touch. */
	browser_view->_enable_webview_scroll();
#endif

	if (!browser_view->_is_loading() && browser_view->_get_edit_mode() != BR_FIND_WORD_MODE)
		browser_view->_navigationbar_visible_set(EINA_FALSE);

	browser_view->m_is_scrolling = EINA_TRUE;

	BROWSER_LOGD("__ewk_view_mouse_down_cb");

#ifdef ZOOM_BUTTON
	double scale_factor = ewk_view_scale_get(browser_view->m_focused_window->m_ewk_view);
	double min_scale = 0;
	double max_scale = 0;

	bool zoom_button_flag = true;
	if (br_preference_get_bool(ZOOM_BUTTON_KEY, &zoom_button_flag) == false)
		BROWSER_LOGE("failed to get ZOOM_BUTTON_KEY value\n");

	BROWSER_LOGD("zoom_button_flag =%d", zoom_button_flag);

	if (zoom_button_flag == false)
		return;

	ewk_view_scale_range_get(browser_view->m_focused_window->m_ewk_view,
					&min_scale, &max_scale);

	if (min_scale < scale_factor)
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout), "show,zoom_out_buttons,signal", "");
	if (max_scale > scale_factor)
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout), "show,zoom_in_buttons,signal", "");

	if (browser_view->m_zoom_button_timer)
		ecore_timer_del(browser_view->m_zoom_button_timer);
	browser_view->m_zoom_button_timer = ecore_timer_add(3, __zoom_button_timeout_cb, browser_view);
#endif
}

void Browser_View::__ewk_view_enter_full_screen_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->set_full_sreen(EINA_TRUE);
}

void Browser_View::__ewk_view_exit_full_screen_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->set_full_sreen(EINA_FALSE);
}

void Browser_View::__ewk_view_vibration_vibrate_cb(void *data, Evas_Object *obj, void *event_info)
{
	Browser_View *browser_view = (Browser_View*)data;
	const long vibration_time = *((const long *)event_info);

	BROWSER_LOGD("__ewk_view_vibration_vibrate_cb : play time is [%ld]", vibration_time);

	browser_view->_haptic_device_play(vibration_time);
}

void Browser_View::__ewk_view_vibration_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	Browser_View *browser_view = (Browser_View*)data;

	browser_view->_haptic_device_stop();
}

void Browser_View::__ewk_view_mouse_up_cb(void* data, Evas* evas, Evas_Object* obj, void* ev)
{
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	browser_view->m_is_scrolling = EINA_FALSE;
}

void Browser_View::__ewk_view_edge_top_cb(void *data, Evas_Object *obj, void *event_info)
{
#ifdef BROWSER_SCROLLER_BOUNCING
	BROWSER_LOGD("[%s]", __func__);

	if (!data)
		return;
	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_enable_browser_scroller_scroll();

	/* If user do flicking the mouse with scroll up, bring in the browser scroller to y=0.	*/
	if (!browser_view->m_is_scrolling) {
		BROWSER_LOGD("<< elm_scroller_region_bring_in >>");
		int browser_scroller_x = 0;
		int browser_scroller_w = 0;
		int browser_scroller_h = 0;
		elm_scroller_region_get(browser_view->m_scroller, &browser_scroller_x, NULL,
						&browser_scroller_w, &browser_scroller_h);

		elm_scroller_region_bring_in(browser_view->m_scroller, browser_scroller_x, 0,
						browser_scroller_w, browser_scroller_h);
	}
#endif
}

void Browser_View::__ewk_view_scroll_down_cb(void *data, Evas_Object *obj, void *event_info)
{
#ifdef BROWSER_SCROLLER_BOUNCING
	if (!data)
		return;
	Browser_View *browser_view = (Browser_View *)data;

	int scroller_x = 0;
	int scroller_y = 0;
	int scroller_w = 0;
	int scroller_h = 0;
	int url_layout_h = 0;
	elm_scroller_region_get(browser_view->m_scroller, &scroller_x, &scroller_y, &scroller_w, &scroller_h);
	evas_object_geometry_get(browser_view->m_url_layout, NULL, NULL, NULL, &url_layout_h);

	if (scroller_y < url_layout_h) {
		/* We have to hide urlbar when webview's contents are scrolled down by flicking. */
		if (!browser_view->m_is_scrolling)
			elm_scroller_region_show(browser_view->m_scroller, scroller_x, url_layout_h, scroller_w, scroller_h);
		else
			browser_view->_enable_browser_scroller_scroll();
	} else {
		browser_view->_enable_webview_scroll();
	}
#endif
}

void Browser_View::__ewk_view_scroll_up_cb(void *data, Evas_Object *obj, void *event_info)
{
#ifdef BROWSER_SCROLLER_BOUNCING
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_enable_webview_scroll();
#endif
}

void Browser_View::_set_navigationbar_title(const char *title)
{
	BROWSER_LOGD("title=[%s]", title);
	edje_object_part_text_set(elm_layout_edje_get(m_url_layout),
					"title_text", title);
	edje_object_part_text_set(elm_layout_edje_get(m_option_header_url_layout),
					"title_text", title);
}


/**
 * @brief Browser_View::_set_url_entry 在url_entry显示要加载的网址
 * @param url  网址
 * @param set_secrue_icon 是否显示图标
 */
void Browser_View::_set_url_entry(const char *url, Eina_Bool set_secrue_icon)
{
	BROWSER_LOGD("url=[%s]", url);
	if (url && strlen(url)) {
		std::string url_without_http_scheme;
		if (strstr(url, BROWSER_HTTP_SCHEME) && strlen(url) > strlen(BROWSER_HTTP_SCHEME))
			url_without_http_scheme = std::string(url + strlen(BROWSER_HTTP_SCHEME));
		else
			url_without_http_scheme = std::string(url);

		char *mark_up_url = elm_entry_utf8_to_markup(url_without_http_scheme.c_str());
		if (mark_up_url) {
			Evas_Object *entry = br_elm_editfield_entry_get(m_url_edit_field);
			elm_entry_entry_set(entry, mark_up_url);
			entry = br_elm_editfield_entry_get(m_option_header_url_edit_field);
			elm_entry_entry_set(entry, mark_up_url);

			edje_object_part_text_set(elm_layout_edje_get(m_option_header_url_edit_field),
							"elm.text.ellipsis", mark_up_url);
			edje_object_part_text_set(elm_layout_edje_get(m_url_edit_field),
							"elm.text.ellipsis", mark_up_url);
			free(mark_up_url);
		}
	} else {
		Evas_Object *entry = br_elm_editfield_entry_get(m_url_edit_field);
#if defined(FEATURE_MOST_VISITED_SITES)
		elm_entry_entry_set(entry, BROWSER_MOST_VISITED_SITES_URL);
#else
		elm_entry_entry_set(entry, BROWSER_BLANK_PAGE_URL);
#endif
		entry = br_elm_editfield_entry_get(m_option_header_url_edit_field);
#if defined(FEATURE_MOST_VISITED_SITES)
		elm_entry_entry_set(entry, BROWSER_MOST_VISITED_SITES_URL);

		edje_object_part_text_set(elm_layout_edje_get(m_option_header_url_edit_field),
						"elm.text.ellipsis", BROWSER_MOST_VISITED_SITES_URL);
		edje_object_part_text_set(elm_layout_edje_get(m_url_edit_field),
						"elm.text.ellipsis", BROWSER_MOST_VISITED_SITES_URL);
#else
		elm_entry_entry_set(entry, BROWSER_BLANK_PAGE_URL);

		edje_object_part_text_set(elm_layout_edje_get(m_option_header_url_edit_field),
						"elm.text.ellipsis", BROWSER_BLANK_PAGE_URL);
		edje_object_part_text_set(elm_layout_edje_get(m_url_edit_field),
						"elm.text.ellipsis", BROWSER_BLANK_PAGE_URL);
#endif
	}

	if (set_secrue_icon)
		_set_secure_icon();
}

void Browser_View::_stop_loading(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_focused_window->m_ewk_view)
		ewk_view_stop(m_focused_window->m_ewk_view);
}

/**
 * @brief Browser_View::_reload 重新加载当前网页
 */
void Browser_View::_reload(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_focused_window->m_ewk_view)
		ewk_view_reload(m_focused_window->m_ewk_view);
}

/**
 * @brief Browser_View::set_focused_window 设置窗口焦点，显示到最前
 * @param window 要显示的window
 * @param show_most_visited_sites
 */
void Browser_View::set_focused_window(Browser_Window *window
#if defined(FEATURE_MOST_VISITED_SITES)
	,Eina_Bool show_most_visited_sites
#endif
	)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_focused_window && m_focused_window->m_ewk_view_layout) {
		elm_box_unpack(m_content_box, m_focused_window->m_ewk_view_layout);
		evas_object_hide(m_focused_window->m_ewk_view_layout);
	}

	if (m_focused_window && m_focused_window->m_favicon) {
		if (elm_object_part_content_get(m_url_entry_layout, "elm.swallow.favicon"))
			elm_object_part_content_unset(m_url_entry_layout, "elm.swallow.favicon");

		evas_object_hide(m_focused_window->m_favicon);
	}
	if (m_focused_window && m_focused_window->m_option_header_favicon) {
		if (elm_object_part_content_get(m_option_header_url_entry_layout, "elm.swallow.favicon"))
			elm_object_part_content_unset(m_option_header_url_entry_layout, "elm.swallow.favicon");

		evas_object_hide(m_focused_window->m_option_header_favicon);
	}

	m_focused_window = window;

	if (!m_focused_window->m_ewk_view_layout) {
		m_focused_window->m_ewk_view_layout = elm_layout_add(m_navi_bar);
		if (!m_focused_window->m_ewk_view_layout) {
			BROWSER_LOGE("elm_layout_add failed!");
			return;
		}
		if (!elm_layout_file_set(m_focused_window->m_ewk_view_layout,
					BROWSER_EDJE_DIR"/browser-view-main.edj",
					"browser/browser-view-webview")) {
			BROWSER_LOGE("elm_layout_file_set failed", BROWSER_EDJE_DIR);
			return;
		}
		evas_object_size_hint_weight_set(m_focused_window->m_ewk_view_layout,
						EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(m_focused_window->m_ewk_view_layout,
						EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_box_pack_end(m_content_box, m_focused_window->m_ewk_view_layout);
		evas_object_show(m_focused_window->m_ewk_view_layout);

		elm_object_part_content_set(m_focused_window->m_ewk_view_layout, "elm.swallow.webview",
					m_focused_window->m_ewk_view);
		evas_object_show(m_focused_window->m_ewk_view);


		/* Workaround.
		  * If launch the browser by aul, the grey bg is displayed shortly. */
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
						"hide,grey_background,signal", "");
		/* Workaround.
		  * The webview layout is not resized whenever repack to content box.
		  * So resize the webview layout whenever repack. */
		m_resize_idler = ecore_idler_add(__webview_layout_resize_idler_cb, this);
	} else {
		elm_box_pack_end(m_content_box, m_focused_window->m_ewk_view_layout);
		evas_object_show(m_focused_window->m_ewk_view_layout);
		evas_object_show(m_focused_window->m_ewk_view);
	}

	if (!_is_loading()) {
		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "loading,off,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
					"loading,off,signal", "");
		elm_progressbar_pulse(m_url_progresswheel, EINA_FALSE);
		elm_progressbar_pulse(m_option_header_url_progresswheel, EINA_FALSE);
	}

	/* show or hide favicon in url layout. */
	if (m_focused_window->m_favicon
#if defined(FEATURE_MOST_VISITED_SITES)
		&& !is_most_visited_sites_running()
#endif
		) {
		if (elm_object_part_content_get(m_url_entry_layout, "elm.swallow.favicon"))
			elm_object_part_content_unset(m_url_entry_layout, "elm.swallow.favicon");

		elm_object_part_content_set(m_url_entry_layout, "elm.swallow.favicon",
							m_focused_window->m_favicon);
		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout),
							"show,favicon,signal", "");
		evas_object_show(m_focused_window->m_favicon);
		_set_secure_icon();
	} else {
		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout),
							"hide,favicon,signal", "");
	}

	/* show or hide favicon in option header url layout. */
	if (m_focused_window->m_option_header_favicon
#if defined(FEATURE_MOST_VISITED_SITES)
		&& !is_most_visited_sites_running()
#endif
		) {
		if (elm_object_part_content_get(m_option_header_url_entry_layout, "elm.swallow.favicon"))
			elm_object_part_content_unset(m_option_header_url_entry_layout, "elm.swallow.favicon");

		elm_object_part_content_set(m_option_header_url_entry_layout, "elm.swallow.favicon",
							m_focused_window->m_option_header_favicon);
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
							"show,favicon,signal", "");
		_set_secure_icon();
		evas_object_show(m_focused_window->m_option_header_favicon);
	} else {
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
							"hide,favicon,signal", "");
	}

#if defined(FEATURE_MOST_VISITED_SITES)
	if (show_most_visited_sites) {
		if (get_url().empty())
			_show_most_visited_sites(EINA_TRUE);
		else
			_show_most_visited_sites(EINA_FALSE);
	}
#endif

	std::vector<Browser_Window *> window_list = m_browser->get_window_list();
	_set_multi_window_controlbar_text(window_list.size());

	if (!_set_favicon())
		BROWSER_LOGE("_set_favicon failed");

	_update_back_forward_buttons();

	/* Without this code, the url is empty shortly when create deleted-window in multi window. */
	if (get_url().empty() && !m_focused_window->m_url.empty())
		_set_url_entry(m_focused_window->m_url.c_str());
	else
		_set_url_entry(get_url().c_str());

	if (window_list.size() >= BROWSER_MULTI_WINDOW_MAX_COUNT)
		elm_object_item_disabled_set(m_new_window_button, EINA_TRUE);
	else
		elm_object_item_disabled_set(m_new_window_button, EINA_FALSE);
}

#if defined(FEATURE_MOST_VISITED_SITES)
Eina_Bool Browser_View::_show_most_visited_sites(Eina_Bool is_show)
{
	BROWSER_LOGD("[%s]is_show=%d", __func__, is_show);
	if (is_show) {
		m_selected_most_visited_sites_item_info.url.clear();
		m_selected_most_visited_sites_item_info.title.clear();
		m_selected_most_visited_sites_item_info.id.clear();

		if (!m_most_visited_sites) {
			m_most_visited_sites = new(nothrow) Most_Visited_Sites(m_navi_bar, this, m_most_visited_sites_db);
			if (!m_most_visited_sites) {
				BROWSER_LOGE("new Most_Visited_Sites failed");
				return EINA_FALSE;
			}

			Evas_Object *most_visited_sites_layout = m_most_visited_sites->create_most_visited_sites_main_layout();
			if (!most_visited_sites_layout) {
				BROWSER_LOGE("create_most_visited_sites_main_layout failed");
				if (m_most_visited_sites) {
					delete m_most_visited_sites;
					m_most_visited_sites = NULL;
				}
				return EINA_FALSE;
			}
			elm_object_part_content_set(m_main_layout, "elm.swallow.most_visited_sites", most_visited_sites_layout);
			edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
							"show,most_visited_sites,signal", "");
			edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
							"show,control_bar,no_animation,signal", "");

			/* change the url layout for normal mode. (change the reload icon etc) */
			edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "loading,off,signal", "");
			edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
						"loading,off,signal", "");

			edje_object_part_text_set(elm_layout_edje_get(m_url_entry_layout),
							"url_guide_text", BR_STRING_URL_GUIDE_TEXT);
			edje_object_part_text_set(elm_layout_edje_get(m_option_header_url_entry_layout),
							"url_guide_text", BR_STRING_URL_GUIDE_TEXT);

			edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "url_guide_text,on,signal", "");
			edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
								"url_guide_text,on,signal", "");

			_set_navigationbar_title(BR_STRING_MOST_VISITED_SITES);
			_set_url_entry(BROWSER_MOST_VISITED_SITES_URL);

			/* If the speed dial is displayed for the first time, destroy the progress wheel. */
			if (m_dummy_loading_progressbar) {
				elm_object_part_content_unset(m_main_layout, "elm.swallow.waiting_progress");
				evas_object_del(m_dummy_loading_progressbar);
				m_dummy_loading_progressbar = NULL;
				edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
							"hide,waiting_progressbar,signal", "");
			}

			elm_object_item_disabled_set(m_add_bookmark_button, EINA_TRUE);
		}

		int scroller_x = 0;
		int scroller_w = 0;
		int scroller_h = 0;
		elm_scroller_region_get(m_scroller, &scroller_x, NULL, &scroller_w, &scroller_h);
		elm_scroller_region_show(m_scroller ,scroller_x, 0, scroller_w, scroller_h);

		_navigationbar_visible_set_signal(EINA_TRUE);

		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout),
							"hide,favicon,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
							"hide,favicon,signal", "");

#ifdef BROWSER_SCROLLER_BOUNCING
		BROWSER_LOGD("<< lock browser scroller >>");
		elm_object_scroll_freeze_pop(m_scroller);
		elm_object_scroll_freeze_push(m_scroller);
#endif
	} else {
		if (m_most_visited_sites) {
			if (elm_object_part_content_get(m_main_layout, "elm.swallow.most_visited_sites"))
				elm_object_part_content_unset(m_main_layout, "elm.swallow.most_visited_sites");
			edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
							"hide,most_visited_sites,signal", "");
			edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
						"hide,most_visited_sites_guide_text,signal", "");

			edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "url_guide_text,off,signal", "");
			edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
										"url_guide_text,off,signal", "");

			delete m_most_visited_sites;
			m_most_visited_sites = NULL;

			elm_object_item_disabled_set(m_add_bookmark_button, EINA_FALSE);
		}
	}
	return EINA_TRUE;
}
#endif

/**
 * @brief Browser_View::load_url  根据输入网页加载网址
 * @param url 所请求的网址
 */
void Browser_View::load_url(const char *url)
{
	BROWSER_LOGD("[%s]", __func__);
#if defined(FEATURE_MOST_VISITED_SITES)
	if (!url || !strlen(url)) {
		if (!_show_most_visited_sites(EINA_TRUE))
			BROWSER_LOGE("_show_most_visited_sites EINA_TRUE failed");
		return;

	} else {
		if (!_show_most_visited_sites(EINA_FALSE))
			BROWSER_LOGE("_show_most_visited_sites EINA_FALSE failed");
	}
#endif

	Evas_Object *edit_field_entry = br_elm_editfield_entry_get(_get_activated_url_entry()); //获得网页输入框
	evas_object_smart_callback_del(edit_field_entry, "changed", __url_entry_changed_cb);

    /**
     * @brief full_url 完整的网址
     */
	std::string full_url;
	if (_has_url_sheme(url))
		full_url = url;
	else
		full_url = std::string(BROWSER_HTTP_SCHEME) + std::string(url);

	BROWSER_LOGD("full_url = [%s]", full_url.c_str());

    _set_url_entry(full_url.c_str());

    /**
     * @brief ewk_view_uri_set加载网页
     */
    ewk_view_uri_set(m_focused_window->m_ewk_view, full_url.c_str());
}

/**
 * @brief Browser_View::get_title 获得指定网页的title
 * @param window 传入的Browser_Window参数
 * @return 返回网页的title
 */
string Browser_View::get_title(Browser_Window *window)
{
	const char *title = ewk_view_title_get(window->m_ewk_view);
	BROWSER_LOGD("title = [%s]", title);

	if (!window->m_ewk_view && !window->m_title.empty())
		return window->m_title;

	if (!window->m_ewk_view)
		return std::string();

	if (!title) {
		if (!window->m_title.empty())
			return window->m_title;
		else
			return std::string();
	}

	return std::string(title);
}

/**
 * @brief Browser_View::get_title 获得当前网页的title
 * @return
 */
string Browser_View::get_title(void)
{
	const char *title = ewk_view_title_get(m_focused_window->m_ewk_view);
	BROWSER_LOGD("title = [%s]", title);

	if (!title && m_focused_window->m_title.empty())
		return std::string();
	else if (!title && !m_focused_window->m_title.empty())
		return m_focused_window->m_title;

	if (title && strlen(title))
		return std::string(title);

	return std::string();
}

/**
 * @brief Browser_View::get_url  获得链接地址
 * @param window 指定的Browser_Window对象
 * @return   返回链接
 */

string Browser_View::get_url(Browser_Window *window)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!window->m_ewk_view && !window->m_url.empty())
		return window->m_url;

	if (!window->m_ewk_view)
		return std::string();

	const char *uri = ewk_view_uri_get(window->m_ewk_view);
	BROWSER_LOGD("uri = [%s]", uri);

	if (!uri || strlen(uri) == 0)
		return std::string();

	return std::string(uri);
}

/**
 * @brief Browser_View::get_url 获得当前的链接地址
 * @return  返回链接地址
 */
string Browser_View::get_url(void)
{
	if (!m_focused_window) {
		BROWSER_LOGE("m_focused_window is NULL");
		return std::string();
	}

	const char *uri = ewk_view_uri_get(m_focused_window->m_ewk_view);
	BROWSER_LOGD("uri = [%s]", uri);

	if (!uri || strlen(uri) == 0)
		return std::string();

	return std::string(uri);
}

/* If multi window is running, unset the navigation title object,
  * else set the navigation title object to show title object arrow.  */
void Browser_View::unset_navigationbar_title_object(Eina_Bool is_unset)
{
	BROWSER_LOGD("[%s] is_unset =%d", __func__, is_unset);
	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
	if (is_unset) {
		_navigationbar_visible_set_signal(EINA_FALSE);
		elm_object_item_part_content_unset(top_it, ELM_NAVIFRAME_ITEM_OPTIONHEADER);
		evas_object_hide(m_option_header_layout);
	} else {
		Evas_Object *title_object = NULL;
		title_object = elm_object_item_part_content_get(top_it, ELM_NAVIFRAME_ITEM_OPTIONHEADER);
		if (!title_object) {
			elm_object_item_part_content_set(top_it, ELM_NAVIFRAME_ITEM_OPTIONHEADER,
									m_option_header_layout);
			evas_object_show(m_option_header_layout);
		}
#if defined(FEATURE_MOST_VISITED_SITES)
		if (!is_most_visited_sites_running())
			_navigationbar_visible_set_signal(EINA_FALSE);
#endif
	}
}

void Browser_View::__go_to_bookmark_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	if (!m_data_manager->create_bookmark_view()) {
		BROWSER_LOGE("m_data_manager->create_bookmark_view failed");
		return;
	}

	if (!m_data_manager->get_bookmark_view()->init()) {
		BROWSER_LOGE("m_data_manager->get_bookmark_view()->init failed");
		m_data_manager->destroy_bookmark_view();
	}

	browser_view->_destroy_more_context_popup();
}

void Browser_View::__backward_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	__title_back_button_clicked_cb(data, NULL, NULL);

	browser_view->_destroy_more_context_popup();
}

void Browser_View::__forward_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_destroy_more_context_popup();
	if (browser_view->m_focused_window->m_ewk_view
		&& ewk_view_forward_possible(browser_view->m_focused_window->m_ewk_view))
		ewk_view_forward(browser_view->m_focused_window->m_ewk_view);
}

void Browser_View::__url_editfield_share_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	const char *selected_text = elm_entry_selection_get(br_elm_editfield_entry_get(browser_view->m_option_header_url_edit_field));

	if (!selected_text || !strlen(selected_text)) {
		BROWSER_LOGD("There is no selected_text. Share the URL");
		selected_text = (const char *)browser_view->get_url().c_str();
	}

	if (browser_view->_show_share_popup(selected_text))
		BROWSER_LOGE("_show_share_popup failed");
}

Eina_Bool Browser_View::__show_scissorbox_view_idler_cb(void *data)
{
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	Browser_View *browser_view = (Browser_View *)data;

	if (browser_view->m_scissorbox_view)
		delete browser_view->m_scissorbox_view;

	browser_view->m_scissorbox_view = new(nothrow) Browser_Scissorbox_View(browser_view);
	if (!browser_view->m_scissorbox_view) {
		BROWSER_LOGE("new Browser_Scissorbox_View failed");
		return ECORE_CALLBACK_CANCEL;
	}
	if (!browser_view->m_scissorbox_view->init()) {
		BROWSER_LOGE("m_scissorbox_view->init failed");
		delete browser_view->m_scissorbox_view;
		browser_view->m_scissorbox_view = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	elm_object_part_content_set(browser_view->m_main_layout, "elm.swallow.scissorbox",
						browser_view->m_scissorbox_view->get_layout());
	edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout),
							"show,scissorbox,signal", "");

	edje_object_signal_emit(elm_layout_edje_get(browser_view->m_url_layout),
							"show,scissorbox,signal", "");
	edje_object_signal_emit(elm_layout_edje_get(browser_view->m_option_header_url_layout),
							"show,scissorbox,signal", "");


	elm_object_part_content_unset(browser_view->m_main_layout, "elm.swallow.control_bar");
	evas_object_hide(browser_view->m_control_bar);

	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool Browser_View::_show_scissorbox_view(void)
{
	BROWSER_LOGD("[%s]", __func__);

	ecore_idler_add(__show_scissorbox_view_idler_cb, this);

	return EINA_TRUE;
}

void Browser_View::_destroy_scissorbox_view(void)
{
	elm_object_part_content_unset(m_main_layout, "elm.swallow.scissorbox");
	edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "hide,scissorbox,signal", "");

	edje_object_signal_emit(elm_layout_edje_get(m_url_layout),
							"hide,scissorbox,signal", "");
	edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_layout),
							"hide,scissorbox,signal", "");

	elm_object_part_content_unset(m_main_layout, "elm.swallow.control_bar");

	if (m_scissorbox_view) {
		delete m_scissorbox_view;
		m_scissorbox_view = NULL;
	}

	elm_object_part_content_set(m_main_layout, "elm.swallow.control_bar", m_control_bar);
	evas_object_show(m_control_bar);
}

void Browser_View::__private_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->m_is_private = !browser_view->m_is_private;

	browser_view->m_is_private_item_pressed = EINA_TRUE;

	elm_check_state_set(browser_view->m_private_check, browser_view->m_is_private);

	if (browser_view->m_is_private) {
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_url_entry_layout), "private,on,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_option_header_url_entry_layout), "private,on,signal", "");
	} else {
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_url_entry_layout), "private,off,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_option_header_url_entry_layout), "private,off,signal", "");
	}

	Ewk_Setting *setting = ewk_view_setting_get(browser_view->m_focused_window->m_ewk_view);
	ewk_setting_private_browsing_set(setting, browser_view->m_is_private);
}

void Browser_View::__bookmark_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	int bookmark_id = -1;
	if(!m_data_manager->get_history_db()->is_in_bookmark(browser_view->get_url().c_str(), &bookmark_id)) {
		if (!elm_icon_file_set(browser_view->m_bookmark_on_off_icon, BROWSER_IMAGE_DIR"/I01_icon_bookmark_on.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.\n");
			return;
		}
		m_data_manager->create_bookmark_db()->save_bookmark(BROWSER_BOOKMARK_MAIN_FOLDER_ID,
					browser_view->get_title().c_str(), browser_view->get_url().c_str());
		browser_view->show_notify_popup(BR_STRING_ADDED_TO_BOOKMARKS, 3, EINA_TRUE);
	} else {
		if (!elm_icon_file_set(browser_view->m_bookmark_on_off_icon, BROWSER_IMAGE_DIR"/I01_icon_bookmark_off.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.\n");
			return;
		}
		m_data_manager->create_bookmark_db()->delete_bookmark(bookmark_id);
		browser_view->show_notify_popup(BR_STRING_REMOVED_TO_BOOKMARKS, 3, EINA_TRUE);
	}

	m_data_manager->destroy_bookmark_db();
}

void Browser_View::__bookmark_icon_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
}

void Browser_View::__private_check_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Eina_Bool state = elm_check_state_get((Evas_Object*)obj);
	browser_view->m_is_private = state;

	if (browser_view->m_is_private_item_pressed) {
		browser_view->m_is_private = !browser_view->m_is_private;
		elm_check_state_set(browser_view->m_private_check, browser_view->m_is_private);
		browser_view->m_is_private_item_pressed = EINA_FALSE;
	}

	if (browser_view->m_is_private) {
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_url_entry_layout), "private,on,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_option_header_url_entry_layout), "private,on,signal", "");
	} else {
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_url_entry_layout), "private,off,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_option_header_url_entry_layout), "private,off,signal", "");
	}

	Ewk_Setting *setting = ewk_view_setting_get(browser_view->m_focused_window->m_ewk_view);
	ewk_setting_private_browsing_set(setting, browser_view->m_is_private);
}

void Browser_View::__expand_option_header_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	browser_view->_navigationbar_title_clicked();
}
/**
 * @brief Browser_View::__add_bookmark_cb 增加书签
 * @param data
 * @param obj
 * @param event_info
 */

void Browser_View::__add_bookmark_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	if (!m_data_manager->create_add_to_bookmark_view(browser_view->get_title(), browser_view->get_url())) {
		BROWSER_LOGE("m_data_manager->create_add_to_bookmark_view failed");
		return;
	}

	if (!m_data_manager->get_add_to_bookmark_view()->init())
		m_data_manager->destroy_add_to_bookmark_view();
}

#if defined(HORIZONTAL_UI)
void Browser_View::_rotate_multi_window(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!m_data_manager->create_multi_window_view()) {
		BROWSER_LOGE("m_data_manager->create_multi_window_view failed");
		return;
	}

	if (!m_data_manager->get_multi_window_view()->init(0.0, m_is_multi_window_grid_mode)) {
		m_data_manager->destroy_multi_window_view();
		BROWSER_LOGE("get_multi_window_view()->init failed");
		return;
	}

	ewk_view_suspend(m_focused_window->m_ewk_view);

	_navigationbar_visible_set_signal(EINA_FALSE);

	/* Hide the secure lock icon in title bar. */
	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
	elm_object_item_part_content_set(top_it, ELM_NAVIFRAME_ITEM_ICON, NULL);

	if (_get_edit_mode() != BR_NO_EDIT_MODE)
		_set_edit_mode(BR_NO_EDIT_MODE);

	m_is_multi_window_grid_mode = EINA_FALSE;
}
#endif

/**
 * @brief Browser_View::__multi_window_cb 窗口切换
 * @param data
 * @param obj
 * @param event_info
 */
void Browser_View::__multi_window_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_destroy_more_context_popup();

#if defined(FEATURE_MOST_VISITED_SITES)
	if (browser_view->m_most_visited_sites && browser_view->m_most_visited_sites->is_guide_text_running())
		return;
#endif

	if (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW)) {
		BROWSER_LOGD("close multi window");
		m_data_manager->get_multi_window_view()->close_multi_window();
		return;
	}

	if (!m_data_manager->create_multi_window_view()) {
		BROWSER_LOGE("m_data_manager->create_multi_window_view failed");
		return;
	}

	if (!m_data_manager->get_multi_window_view()->init()) {
		m_data_manager->destroy_multi_window_view();
		BROWSER_LOGE("get_multi_window_view()->init failed");
		return;
	}

	/* Exception case to call ewk_view_suspend directly.
	  * That's because the webpage is cleared, when call ewk_view_visibility_set suspend_ewk_view */
	ewk_view_suspend(browser_view->m_focused_window->m_ewk_view);
	ewk_view_visibility_set(browser_view->m_focused_window->m_ewk_view, EINA_FALSE);

	browser_view->_navigationbar_visible_set_signal(EINA_FALSE);

	/* Hide the secure lock icon in title bar. */
	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
	elm_object_item_part_content_set(top_it, ELM_NAVIFRAME_ITEM_ICON, NULL);

	if (browser_view->_get_edit_mode() != BR_NO_EDIT_MODE)
		browser_view->_set_edit_mode(BR_NO_EDIT_MODE);
}

/**
 * @brief Browser_View::__new_window_cb  新增一个标签页
 * @param data 传入参数
 * @param obj 发送信号本身对象
 * @param event_info 信号自身信息
 */
void Browser_View::__new_window_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Browser_Class *browser = browser_view->m_browser;

	if (browser_view->m_resize_idler)
		return;

	std::vector<Browser_Window *> window_list = browser->get_window_list();
	if (window_list.size() >= BROWSER_MULTI_WINDOW_MAX_COUNT) {
		/* If the multi window is max, delete the first one. */
		/* if the first window is focused, delete second one(oldest one except first one) */
		if (browser_view->m_focused_window != window_list[0])
			browser->delete_window(window_list[0]);
		else
			browser->delete_window(window_list[1]);
	}

	browser_view->m_created_new_window = browser->create_new_window(EINA_TRUE);
	if (!browser_view->m_created_new_window) {
		BROWSER_LOGE("create_new_window failed");
		return;
	}
	/* initialize the created webview first to connect ewk event callback functions such as load start, progress etc. */
	ewk_view_suspend(browser_view->m_focused_window->m_ewk_view);

#if defined(HORIZONTAL_UI)
		if (browser_view->m_focused_window->m_landscape_snapshot_image) {
			evas_object_del(browser_view->m_focused_window->m_landscape_snapshot_image);
			browser_view->m_focused_window->m_landscape_snapshot_image = NULL;
		}
#endif
		if (browser_view->m_focused_window->m_portrait_snapshot_image) {
			evas_object_del(browser_view->m_focused_window->m_portrait_snapshot_image);
			browser_view->m_focused_window->m_portrait_snapshot_image = NULL;
		}

#if defined(HORIZONTAL_UI)
		if (browser_view->is_landscape())
			browser_view->m_focused_window->m_landscape_snapshot_image = browser_view->_capture_snapshot(browser_view->m_focused_window, BROWSER_MULTI_WINDOW_ITEM_RATIO);
		else
#endif
			browser_view->m_focused_window->m_portrait_snapshot_image = browser_view->_capture_snapshot(browser_view->m_focused_window, BROWSER_MULTI_WINDOW_ITEM_RATIO);

#if defined(FEATURE_MOST_VISITED_SITES)
	if (!browser_view->_show_most_visited_sites(EINA_FALSE))
		BROWSER_LOGE("_show_most_visited_sites EINA_FALSE failed");
#endif

	/* initialize the created webview first to connect ewk event callback functions such as load start, progress etc. */
	browser->ewk_view_init(browser_view->m_created_new_window->m_ewk_view);

	if (!browser_view->_show_new_window_effect(browser_view->m_focused_window->m_ewk_view,
					browser_view->m_created_new_window->m_ewk_view))
        BROWSER_LOGE("_show_new_window_effect failed");

#if defined(FEATURE_MOST_VISITED_SITES)
	browser_view->load_url(BROWSER_MOST_VISITED_SITES_URL);
#else
	browser_view->load_url(BROWSER_BLANK_PAGE_URL);
#endif
	return;
}

Eina_Bool Browser_View::_call_internet_settings(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_browser_settings = new(nothrow) Browser_Settings_Class;
	if (!m_browser_settings) {
		BROWSER_LOGE("new Browser_Settings_Class failed");
		return EINA_FALSE;
	}
	if (!m_browser_settings->init()) {
		BROWSER_LOGE("m_browser_settings->init failed");
		delete m_browser_settings;
		m_browser_settings = NULL;

		return EINA_FALSE;
	}
	return EINA_TRUE;
}

void Browser_View::__internet_settings_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_destroy_more_context_popup();

	if (!browser_view->_call_internet_settings())
		BROWSER_LOGE("_call_internet_settings failed");
}

void Browser_View::_update_find_word_index_text(const char *index_text, int index, int max_match_cnt)
{
	BROWSER_LOGD("[%s]", __func__);

	edje_object_part_text_set(elm_layout_edje_get(m_option_header_find_word_layout), "elm.index_text", index_text);

	if (max_match_cnt == 0 || max_match_cnt == 1) {
		elm_object_disabled_set(m_find_word_prev_button, EINA_TRUE);
		elm_object_disabled_set(m_find_word_next_button, EINA_TRUE);
	} else if (index == 0 || index == 1) {
		elm_object_disabled_set(m_find_word_prev_button, EINA_TRUE);
		elm_object_disabled_set(m_find_word_next_button, EINA_FALSE);
		if (index == 0 || (index == 1 && elm_object_focus_get(m_find_word_cancel_button)))
			elm_object_focus_set(m_find_word_cancel_button, EINA_TRUE);
	} else if (index == max_match_cnt) {
		elm_object_disabled_set(m_find_word_prev_button, EINA_FALSE);
		elm_object_disabled_set(m_find_word_next_button, EINA_TRUE);
		elm_object_focus_set(m_find_word_cancel_button, EINA_TRUE);
	} else {
		if (elm_object_disabled_get(m_find_word_prev_button))
			elm_object_disabled_set(m_find_word_prev_button, EINA_FALSE);
		if (elm_object_disabled_get(m_find_word_next_button))
			elm_object_disabled_set(m_find_word_next_button, EINA_FALSE);
	}
}
void Browser_View::__find_word_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_destroy_more_context_popup();

	if (browser_view->_get_edit_mode() == BR_FIND_WORD_MODE)
		return;

	/* find word layout is only in naviframe optino header.
	  * So expand the option header to show find word layout. */
	browser_view->_navigationbar_visible_set_signal(EINA_TRUE);

	browser_view->_set_edit_mode(BR_FIND_WORD_MODE);

	elm_object_focus_set(browser_view->m_find_word_edit_field, EINA_TRUE);

	edje_object_part_text_set(elm_layout_edje_get(browser_view->m_option_header_find_word_layout), "elm.index_text", "0/0");
}

/**
 * @brief Browser_View::_call_download_manager 调用download_manager下载文件
 * @return
 */

Eina_Bool Browser_View::_call_download_manager(void)
{
	service_h service_handle = NULL;
	BROWSER_LOGD("[%s]", __func__);

	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}
	
	if (!service_handle) {
		BROWSER_LOGE("service handle is NULL");
		return EINA_FALSE;
	}

	if (service_set_operation(service_handle, SERVICE_OPERATION_DOWNLOAD) < 0) {
		BROWSER_LOGE("Fail to set service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_add_extra_data(service_handle, "mode", "view") < 0) {
		BROWSER_LOGE("Fail to set extra data");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	service_destroy(service_handle);

	return EINA_TRUE;
}

void Browser_View::__download_manager_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_destroy_more_context_popup();

	if (!browser_view->_call_download_manager())
		BROWSER_LOGE("_call_download_manager failed");
}

void Browser_View::__send_via_message_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_destroy_more_context_popup();

	if (!browser_view->_send_via_message(browser_view->get_url(), std::string()))
		BROWSER_LOGE("_send_via_message failed");
}

void Browser_View::__send_via_email_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_destroy_more_context_popup();

	if (!browser_view->_send_via_email(browser_view->get_url()))
		BROWSER_LOGE("_send_via_email failed");
}

void Browser_View::__post_to_sns_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View*)data;

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	const char *label = elm_object_item_text_get(it);
	BROWSER_LOGD("label=[%s]", label);
	if (label && strlen(label)) {
		if (!browser_view->_post_to_sns(std::string(label), browser_view->get_url()))
			BROWSER_LOGE("_post_to_sns failed");
	}

	browser_view->_destroy_more_context_popup();
}

void Browser_View::_destroy_more_context_popup(void)
{
	if (m_more_context_popup) {
		evas_object_del(m_more_context_popup);
		m_more_context_popup = NULL;
	}

	edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "hide,more_context_bg,signal", "");
}

void Browser_View::__more_context_popup_dismissed_cb(void *data, Evas_Object *obj,
									void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_destroy_more_context_popup();
}

void Browser_View::__clean_up_windows_test_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_destroy_more_context_popup();

	m_browser->clean_up_windows();
}

Eina_Bool Browser_View::_show_more_context_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);

	_destroy_more_context_popup();

	m_more_context_popup = elm_ctxpopup_add(m_win);
	if (!m_more_context_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return EINA_FALSE;
	}

	elm_object_style_set(m_more_context_popup, "pass_event");

	edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,more_context_bg,signal", "");

	evas_object_size_hint_weight_set(m_more_context_popup, EVAS_HINT_EXPAND,
								EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(m_more_context_popup, "dismissed",
					__more_context_popup_dismissed_cb, this);

	Elm_Object_Item *sub_menu = NULL;
	elm_ctxpopup_item_append(m_more_context_popup, BR_STRING_DOWNLOAD_MANAGER, NULL,
							__download_manager_cb, this);

	sub_menu = elm_ctxpopup_item_append(m_more_context_popup, BR_STRING_FIND_ON_PAGE, NULL,
							__find_word_cb, this);
	if (
#if defined(FEATURE_MOST_VISITED_SITES)
		is_most_visited_sites_running() ||
#endif
		get_url().empty()
	    || _get_edit_mode() == BR_FIND_WORD_MODE)
		elm_object_item_disabled_set(sub_menu, EINA_TRUE);

	m_bookmark_on_off_icon = elm_icon_add(m_more_context_popup);

	if(m_data_manager->get_history_db()->is_in_bookmark(get_url().c_str(), NULL)) {
		if (!elm_icon_file_set(m_bookmark_on_off_icon, BROWSER_IMAGE_DIR"/I01_icon_bookmark_on.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.\n");
			return EINA_FALSE;
		}
	} else {
		if (!elm_icon_file_set(m_bookmark_on_off_icon, BROWSER_IMAGE_DIR"/I01_icon_bookmark_off.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.\n");
			return EINA_FALSE;
		}
	}
	evas_object_size_hint_aspect_set(m_bookmark_on_off_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

	evas_object_propagate_events_set(m_bookmark_on_off_icon, EINA_FALSE);
	evas_object_repeat_events_set(m_bookmark_on_off_icon, EINA_FALSE);
	evas_object_smart_callback_add(m_bookmark_on_off_icon, "clicked", __bookmark_icon_changed_cb, this);

	sub_menu = elm_ctxpopup_item_append(m_more_context_popup, BR_STRING_BOOKMARK, m_bookmark_on_off_icon,
							__bookmark_cb, this);
	if (
#if defined(FEATURE_MOST_VISITED_SITES)
		is_most_visited_sites_running() ||
#endif
		get_url().empty()
	    || _get_edit_mode() == BR_FIND_WORD_MODE)
		elm_object_item_disabled_set(sub_menu, EINA_TRUE);

	m_private_check = elm_check_add(m_more_context_popup);
	elm_object_style_set(m_private_check, "on&off");
	evas_object_smart_callback_add(m_private_check, "changed", __private_check_change_cb, this);

	elm_check_state_set(m_private_check, m_is_private);
	evas_object_propagate_events_set(m_private_check, EINA_FALSE);
	evas_object_show(m_private_check);

	elm_ctxpopup_item_append(m_more_context_popup, BR_STRING_PRIVATE, m_private_check,
								__private_cb, this);

	elm_ctxpopup_item_append(m_more_context_popup, BR_STRING_SETTINGS, NULL,
							__internet_settings_cb, this);

	elm_ctxpopup_hover_parent_set(m_more_context_popup, m_navi_bar);

	int controlbar_x = 0;
	int controlbar_y = 0;
	int controlbar_w = 0;
	int controlbar_h = 0;
	evas_object_geometry_get(m_control_bar, &controlbar_x, &controlbar_y, &controlbar_w, &controlbar_h);

	evas_object_move(m_more_context_popup, (controlbar_w / 5) * 3 + controlbar_w / 10, controlbar_y + (controlbar_h / 2));

	evas_object_show(m_more_context_popup);

	return EINA_TRUE;
}

void Browser_View::__more_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	if (browser_view->m_more_context_popup == NULL) {
		if (!browser_view->_show_more_context_popup())
			BROWSER_LOGE("_show_more_context_popup failed");
	} else {
		browser_view->_destroy_more_context_popup();
	}
	/* To give focus out signal to webkit(for destructing webkit context menu), give focus set in unvisible button */
	elm_object_focus_set(browser_view->m_option_header_cancel_button, EINA_TRUE);

	browser_view->_navigationbar_visible_set(EINA_TRUE);
}

void Browser_View::_set_controlbar_back_forward_status(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *ewk_view = m_focused_window->m_ewk_view;
	if (!ewk_view)
		return;

	if (
#if defined(FEATURE_MOST_VISITED_SITES)
		is_most_visited_sites_running() ||
#endif
		get_url().empty())
		elm_object_item_disabled_set(m_share_controlbar_button, EINA_TRUE);
	else
		elm_object_item_disabled_set(m_share_controlbar_button, EINA_FALSE);
}

void Browser_View::_set_multi_window_controlbar_text(int count)
{
	BROWSER_LOGD("[%s]", __func__);
	if (count == 1) {
		elm_toolbar_item_icon_set(m_multi_window_button, BROWSER_IMAGE_DIR"/01_controlbar_icon_multiview.png");
	} else {
		char icon_path[100] = {0, };
		snprintf(icon_path, sizeof(icon_path) - 1, "%s/01_controlbar_icon_multiview_0%d.png", BROWSER_IMAGE_DIR, count);
		elm_toolbar_item_icon_set(m_multi_window_button, icon_path);
	}
}

/**
 * @brief Browser_View::_create_control_bar 创建底部toolbar
 * @return  返回一个layout布局
 */
Evas_Object *Browser_View::_create_control_bar(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *controlbar_layout;
	controlbar_layout = elm_layout_add(m_navi_bar);
	if (!controlbar_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	if (!elm_layout_file_set(controlbar_layout, BROWSER_EDJE_DIR"/browser-view-control-bar.edj",
				"browser-view-controlbar")) {
		BROWSER_LOGE("Can not set layout browser-view-controlbar\n");
		return NULL;
	}
	evas_object_size_hint_weight_set(controlbar_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(controlbar_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(controlbar_layout);

	Evas_Object *control_bar;
	control_bar = elm_toolbar_add(m_navi_bar);
	if (control_bar) {
		elm_object_style_set(control_bar, "browser/browser-view");

		elm_toolbar_shrink_mode_set(control_bar, ELM_TOOLBAR_SHRINK_EXPAND);


		m_new_window_button = elm_toolbar_item_append(control_bar,
					BROWSER_IMAGE_DIR"/I01_controlbar_icon_new_window.png", NULL, __new_window_cb, this);
		if (!m_new_window_button) {
			BROWSER_LOGE("elm_toolbar_item_append failed");
			return NULL;
		}

		/* Multi Window Button */
		m_multi_window_button = elm_toolbar_item_append(control_bar,
					BROWSER_IMAGE_DIR"/01_controlbar_icon_multiview.png", NULL, __multi_window_cb, this);
		/* Bookmark Button */
		elm_toolbar_item_append(control_bar, BROWSER_IMAGE_DIR"/I01_controlbar_icon_bookmark.png",
						NULL, __go_to_bookmark_cb, this);
		/* More menu Button */
		m_more_button = elm_toolbar_item_append(control_bar,
					BROWSER_IMAGE_DIR"/I01_controlbar_icon_more.png", NULL, __more_cb, this);
		if (!m_more_button) {
			BROWSER_LOGE("elm_toolbar_item_append failed");
			return NULL;
		}

		elm_object_part_content_set(controlbar_layout, "elm.swallow.controlbar", control_bar);

		m_back_button = elm_button_add(control_bar);
		if (!m_back_button) {
			BROWSER_LOGE("elm_button_add failed");
			return NULL;
		}
		elm_object_style_set(m_back_button, "browser/browser_view_controlbar_back");

		elm_object_part_content_set(controlbar_layout, "elm.swallow.back_button", m_back_button);
		evas_object_smart_callback_add(m_back_button, "clicked", __backward_cb, this);
		evas_object_show(m_back_button);
		evas_object_show(control_bar);
	}

	return controlbar_layout;
}

/* 
* Create two same url layouts similar with other browsers like android & safari.
* The one(by _create_url_layout) is in the browser scroller and
* the other(by _create_option_header_url_layout) is in the navigation bar option header.
*/
Evas_Object *Browser_View::_create_option_header_url_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *url_layout;
	url_layout = elm_layout_add(m_navi_bar);
	if (url_layout) {
		if (!elm_layout_file_set(url_layout, BROWSER_EDJE_DIR"/browser-view-url-layout.edj",
					"browser/url_layout")) {
			BROWSER_LOGE("Can not set layout theme[browser/url_layout]\n");
			return NULL;
		}
		evas_object_size_hint_weight_set(url_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(url_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(url_layout);

		m_option_header_url_entry_layout = elm_layout_add(m_navi_bar);
		if (!m_option_header_url_entry_layout) {
			BROWSER_LOGE("elm_layout_add failed");
			return NULL;
		}
		if (!elm_layout_file_set(m_option_header_url_entry_layout, BROWSER_EDJE_DIR"/browser-view-url-layout.edj",
					"elm/browser/urlentry/default")) {
			BROWSER_LOGE("Can not set layout theme[browser, urlentry, default]\n");
			return NULL;
		}
		elm_object_part_content_set(url_layout, "elm.swallow.url", m_option_header_url_entry_layout);
		edje_object_signal_callback_add(elm_layout_edje_get(m_option_header_url_entry_layout),
						"mouse,clicked,1", "elm.swallow.entry", __url_entry_clicked_cb, this);

		edje_object_signal_callback_add(elm_layout_edje_get(m_option_header_url_entry_layout),
						"refresh_stop", "*", __refresh_button_clicked_cb, this);

		evas_object_show(m_option_header_url_entry_layout);

		m_option_header_url_edit_field = br_elm_url_editfield_add(m_navi_bar);
		if (!m_option_header_url_edit_field) {
			BROWSER_LOGE("elm_editfield_add failed");
			return NULL;
		}
		elm_entry_context_menu_item_add(br_elm_editfield_entry_get(m_option_header_url_edit_field),
								BR_STRING_CTXMENU_SHARE, NULL, ELM_ICON_NONE, __url_editfield_share_clicked_cb, this);

		elm_object_part_content_set(m_option_header_url_entry_layout, "elm.swallow.entry", m_option_header_url_edit_field);
//		br_elm_editfield_entry_single_line_set(m_option_header_url_edit_field, EINA_TRUE);
		br_elm_editfield_eraser_set(m_option_header_url_edit_field, EINA_FALSE);

		Evas_Object *edit_field_entry = br_elm_editfield_entry_get(m_option_header_url_edit_field);
		elm_entry_input_panel_layout_set(edit_field_entry, ELM_INPUT_PANEL_LAYOUT_URL);
		ecore_imf_context_input_panel_event_callback_add((Ecore_IMF_Context *)elm_entry_imf_context_get(edit_field_entry),
								ECORE_IMF_INPUT_PANEL_STATE_EVENT, __url_entry_imf_event_cb, this);

        /**
         * @brief evas_object_smart_callback_add 监听enter消息，点击enter时打开跳转到输入的网址
         */
		evas_object_smart_callback_add(edit_field_entry, "activated", __url_entry_enter_key_cb, this);
		evas_event_callback_add(evas_object_evas_get(m_option_header_url_edit_field), EVAS_CALLBACK_CANVAS_FOCUS_OUT,
					__url_entry_focus_out_cb, this);
		evas_object_show(m_option_header_url_edit_field);

		m_option_header_url_progressbar = elm_progressbar_add(m_navi_bar);
		if (!m_option_header_url_progressbar) {
			BROWSER_LOGE("elm_progressbar_add failed");
			return NULL;
		}
		elm_object_style_set(m_option_header_url_progressbar, "browser/loading");
		elm_object_part_content_set(m_option_header_url_entry_layout, "elm.swallow.progressbar", m_option_header_url_progressbar);
		elm_progressbar_value_set(m_option_header_url_progressbar, 0);
		evas_object_show(m_option_header_url_progressbar);

		m_option_header_url_progresswheel = elm_progressbar_add(m_navi_bar);
		if (!m_option_header_url_progresswheel) {
			BROWSER_LOGE("elm_progressbar_add failed");
			return NULL;
		}

		elm_object_style_set(m_option_header_url_progresswheel, "UIActivityIndicatorStyleWhite");

		elm_progressbar_pulse(m_option_header_url_progresswheel, EINA_FALSE);
		elm_object_part_content_set(m_option_header_url_entry_layout, "elm.swallow.progress", m_option_header_url_progresswheel);
		evas_object_show(m_option_header_url_progresswheel);

		m_option_header_cancel_button = elm_button_add(m_navi_bar);
		if (!m_option_header_cancel_button) {
			BROWSER_LOGE("elm_button_add failed");
			return NULL;
		}
		evas_object_size_hint_weight_set(m_option_header_cancel_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(m_option_header_cancel_button, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_style_set(m_option_header_cancel_button, "browser/cancel_button");
		elm_object_text_set(m_option_header_cancel_button, BR_STRING_CANCEL);
		elm_object_part_content_set(url_layout, "elm.swallow.cancel", m_option_header_cancel_button);
		evas_object_smart_callback_add(m_option_header_cancel_button, "clicked", __cancel_button_clicked_cb, this);
		evas_object_show(m_option_header_cancel_button);

		m_option_header_title_backward_button = elm_button_add(m_navi_bar);
		if (!m_option_header_title_backward_button) {
			BROWSER_LOGE("elm_button_add failed");
			return NULL;
		}
		elm_object_style_set(m_option_header_title_backward_button, "browser/backward");
		elm_object_part_content_set(url_layout, "elm.swallow.backward_button", m_option_header_title_backward_button);
		evas_object_smart_callback_add(m_option_header_title_backward_button, "clicked", __backward_button_clicked_cb, this);
		elm_object_focus_allow_set(m_option_header_title_backward_button, EINA_FALSE);
		evas_object_show(m_option_header_title_backward_button);
		elm_object_disabled_set(m_option_header_title_backward_button, EINA_TRUE);

		m_option_header_title_forward_button = elm_button_add(m_navi_bar);
		if (!m_option_header_title_forward_button) {
			BROWSER_LOGE("elm_button_add failed");
			return NULL;
		}
		elm_object_style_set(m_option_header_title_forward_button, "browser/forward");
		elm_object_part_content_set(url_layout, "elm.swallow.forward_button", m_option_header_title_forward_button);
		evas_object_smart_callback_add(m_option_header_title_forward_button, "clicked", __forward_button_clicked_cb, this);
		elm_object_focus_allow_set(m_option_header_title_forward_button, EINA_FALSE);
		evas_object_show(m_option_header_title_forward_button);
		elm_object_disabled_set(m_option_header_title_forward_button, EINA_TRUE);

		/* for jump to top. */
		evas_object_event_callback_add(url_layout, EVAS_CALLBACK_MOUSE_DOWN, __option_header_url_layout_mouse_down_cb, this);
	}

	return url_layout;
}

Evas_Object *Browser_View::_get_activated_url_entry(void)
{
	/* The edit field in option header url layout is only valid for edit. 
	  * If the edit field in browser scroller can have focus, there is so many focus issue.
	  * So just make the edit field in option header editable. */
	return m_option_header_url_edit_field;
}

/**
 * @brief Browser_View::__url_entry_imf_event_cb 输入法回调函数
 * @param data
 * @param ctx
 * @param value
 */

void Browser_View::__url_entry_imf_event_cb(void *data, Ecore_IMF_Context *ctx, int value)
{
	BROWSER_LOGD("value=%d", value);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	if (value == ECORE_IMF_INPUT_PANEL_STATE_HIDE) {
		Evas_Object *edit_field_entry;
		edit_field_entry = br_elm_editfield_entry_get(browser_view->_get_activated_url_entry());
		elm_object_focus_set(edit_field_entry, EINA_FALSE);

		if (browser_view->m_edit_mode != BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF
		    && browser_view->m_edit_mode != BR_FIND_WORD_MODE)
			browser_view->_set_edit_mode(BR_NO_EDIT_MODE);
		else
			return;
	} else if (value == ECORE_IMF_INPUT_PANEL_STATE_SHOW)
		/* If the focus of url entry is set automatically, the keypad is also displayed automatically. */
		/* eg. At url edit mode, lock the screen -> then unlock, the url entry will get focus. */
		/* The keypad is also invoked, so set the edit mode. */
		browser_view->_set_edit_mode(BR_URL_ENTRY_EDIT_MODE);
}

/**
 * @brief Browser_View::__url_entry_focus_out_cb 输入框失去输入焦点
 * @param data
 * @param e
 * @param event_info
 */
void Browser_View::__url_entry_focus_out_cb(void *data, Evas *e, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Evas_Object *edit_field_entry;
	edit_field_entry = br_elm_editfield_entry_get(browser_view->_get_activated_url_entry());

	Ecore_IMF_Context *ic = (Ecore_IMF_Context *)elm_entry_imf_context_get(edit_field_entry);
	if (!ic)
		return;

	ecore_imf_context_input_panel_hide(ic);

	if (browser_view->m_edit_mode != BR_FIND_WORD_MODE)
		browser_view->_set_edit_mode(BR_NO_EDIT_MODE);
}

/**
 * @brief Browser_View::_set_edit_mode 设置url_entry编辑模式
 * @param mode 模式
 */

void Browser_View::_set_edit_mode(edit_mode mode)
{
	BROWSER_LOGD("mode = %d", mode);

	if (m_edit_mode == mode)
		return;

#if defined(FEATURE_MOST_VISITED_SITES)
	if (mode == BR_NO_EDIT_MODE) {
		if (m_most_visited_sites) {
			edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "url_guide_text,on,signal", "");
			edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
								"url_guide_text,on,signal", "");
		}
	} else {
		if (m_most_visited_sites) {
			edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "url_guide_text,off,signal", "");
			edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
								"url_guide_text,off,signal", "");
		}
	}
#endif

	if (mode == BR_URL_ENTRY_EDIT_MODE || mode == BR_FIND_WORD_MODE
	    || mode == BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF) {
#ifdef BROWSER_SCROLLER_BOUNCING
		/* If edit mode, lock the browser scroller */
		_enable_webview_scroll();
#endif
		/* Make the browser scroller region y = 0 to show url bar all at edit mode. */
		int scroller_x = 0;
		int scroller_w = 0;
		int scroller_h = 0;
		elm_scroller_region_get(m_scroller, &scroller_x, NULL, &scroller_w, &scroller_h);
		elm_scroller_region_show(m_scroller ,scroller_x, 0, scroller_w, scroller_h);
	}

	if (mode == BR_URL_ENTRY_EDIT_MODE) {
		/* change layout of url layout for edit mode. */
		edje_object_signal_emit(elm_layout_edje_get(m_url_layout), "edit,url,on,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_layout),
					"edit,url,on,signal", "");

		/* change refresh icon in url entry for edit mode. */
		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "edit,url,on,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
					"edit,url,on,signal", "");

		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "hide,favicon,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
										"hide,favicon,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
								"edit,url,on,signal", "");
	} else if (mode == BR_NO_EDIT_MODE || mode == BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF) {
		if (m_edit_mode == BR_FIND_WORD_MODE) {
			edje_object_signal_emit(elm_layout_edje_get(m_option_header_layout), "hide,find_word_layout,signal", "");
			edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,control_bar,no_animation,signal", "");
			m_find_word->find_word("", Browser_Find_Word::BROWSER_FIND_WORD_FORWARD);
		} else {
			/* change layout of url layout for normal mode. */
			edje_object_signal_emit(elm_layout_edje_get(m_url_layout), "edit,url,off,signal", "");
			edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_layout),
						"edit,url,off,signal", "");

			/* change refresh icon in url entry for normal mode. */
			edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "edit,url,off,signal", "");
			edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
						"edit,url,off,signal", "");

#if defined(FEATURE_MOST_VISITED_SITES)
			if (!is_most_visited_sites_running())
#endif
			{
				if (m_focused_window->m_favicon)
					edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout),
											"show,favicon,signal", "");
				if (m_focused_window->m_option_header_favicon)
					edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
											"show,favicon,signal", "");
				_set_secure_icon();
			}

			edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
						"edit,url,off,signal", "");
			if (_is_loading()) {
				edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "loading,on,signal", "");
				edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
							"loading,on,signal", "");
			}
		}
	} else if (mode == BR_FIND_WORD_MODE) {
		Evas_Object *find_word_edit_field_entry = br_elm_editfield_entry_get(m_find_word_edit_field);
		elm_entry_entry_set(find_word_edit_field_entry, "");

		evas_object_smart_callback_del(find_word_edit_field_entry, "changed", __find_word_entry_changed_cb);
		evas_object_smart_callback_add(find_word_edit_field_entry, "changed", __find_word_entry_changed_cb, this);

		edje_object_signal_emit(elm_layout_edje_get(m_option_header_layout), "show,find_word_layout,signal", "");
	}

	m_edit_mode = mode;

	/* To show favicon, if click url entry while loading, then cancel case. */
	_set_favicon();
}

/* Workaround
  * The url entry of option header can't be focusable in case of switch TEXTBLOCK & elm entry.
  * So focus the url entry in idler callback.
  */
static Eina_Bool __url_entry_focus_idler_cb(void *data)
{
	Evas_Object *edit_field = (Evas_Object *)data;
	elm_object_focus_set(edit_field, EINA_TRUE);

	Evas_Object *entry = br_elm_editfield_entry_get(edit_field);
	elm_entry_cursor_end_set(entry);

	return ECORE_CALLBACK_CANCEL;
}

void Browser_View::__url_entry_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	if (browser_view->_is_loading())
		browser_view->_stop_loading();

	if (!browser_view->_navigationbar_visible_get()) {
		/* The edit field in option header is only for edit url. */
		browser_view->_navigationbar_visible_set_signal(EINA_TRUE);
	}

	browser_view->_destroy_more_context_popup();

	edit_mode mode = browser_view->_get_edit_mode();

    /**
      * jinjianxin
      */

    browser_view->_set_edit_mode(BR_URL_ENTRY_EDIT_MODE);

	if (mode == BR_NO_EDIT_MODE) {
		Evas_Object *entry = br_elm_editfield_entry_get(browser_view->m_option_header_url_edit_field);

		elm_object_focus_set(browser_view->m_option_header_url_edit_field, EINA_TRUE);

		const char *url_text = elm_entry_entry_get(entry);
		if (url_text && strlen(url_text))
			elm_entry_select_all(entry);
		elm_entry_cursor_end_set(entry);

//		ecore_idler_add(__url_entry_focus_idler_cb, browser_view->m_option_header_url_edit_field);
	}
}

Eina_Bool Browser_View::_is_option_header_expanded(void)
{
	return _navigationbar_visible_get();
}

void Browser_View::__cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Evas_Object *ewk_view = browser_view->m_focused_window->m_ewk_view;
	if (!ewk_view)
		return;

	browser_view->_set_edit_mode(BR_NO_EDIT_MODE);

	elm_object_signal_emit(browser_view->m_option_header_url_edit_field, "ellipsis_show,signal", "elm");
	elm_object_signal_emit(browser_view->m_url_edit_field, "ellipsis_show,signal", "elm");

	browser_view->_set_url_entry(browser_view->get_url().c_str());
}

void Browser_View::__refresh_button_clicked_cb(void *data, Evas_Object *obj,
						const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	if (browser_view->_get_edit_mode() == BR_URL_ENTRY_EDIT_MODE) {
		elm_entry_entry_set(br_elm_editfield_entry_get(browser_view->m_url_edit_field), "");
		elm_entry_entry_set(br_elm_editfield_entry_get(browser_view->m_option_header_url_edit_field), "");
	} else if(browser_view->_is_loading())
		browser_view->_stop_loading();
	else
		browser_view->_reload();
}

/**
 * @brief Browser_View::__url_entry_enter_key_cb  url_entry回调函数
 * @param data entry所带参数
 * @param obj  信号发出着本身
 * @param event_info 信号所带的参数
 */
void Browser_View::__url_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Evas_Object *ewk_view = browser_view->m_focused_window->m_ewk_view;
	Evas_Object *edit_field_entry;
	edit_field_entry = br_elm_editfield_entry_get(browser_view->_get_activated_url_entry());

	/* Workaround.
	  * Give focus to option header cancel button to hide imf. */
	elm_object_focus_set(browser_view->m_option_header_cancel_button, EINA_TRUE);

	char *url = elm_entry_markup_to_utf8(elm_entry_entry_get(edit_field_entry));
    url = "http://www.google.com";
	BROWSER_LOGD("input url = [%s]", url);

	if (url && strlen(url)) {
		browser_view->load_url(url);
		free(url);
	}
}


void Browser_View::__url_entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
}


void Browser_View::__url_layout_mouse_down_cb(void *data, Evas* evas, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Evas_Event_Mouse_Down event = *(Evas_Event_Mouse_Down *)event_info;

	Browser_View *browser_view = (Browser_View *)data;

	Evas_Object *entry = br_elm_editfield_entry_get(browser_view->m_url_edit_field);

	int entry_x = 0;
	int entry_y = 0;
	int entry_w = 0;
	int entry_h = 0;
	evas_object_geometry_get(browser_view->m_url_edit_field, &entry_x, &entry_y, &entry_w, &entry_h);

	if (event.output.x > entry_x && event.output.x < entry_x + entry_w &&
	    event.output.y > entry_y && event.output.y < entry_y + entry_h) {
		elm_object_signal_emit(browser_view->m_option_header_url_edit_field, "ellipsis_hide,signal", "elm");
		elm_object_signal_emit(browser_view->m_url_edit_field, "ellipsis_hide,signal", "elm");
	}

	edit_mode mode = browser_view->_get_edit_mode();
	if (mode == BR_URL_ENTRY_EDIT_MODE || mode == BR_FIND_WORD_MODE
		    || mode == BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF) {
#ifdef BROWSER_SCROLLER_BOUNCING
		BROWSER_LOGD("<< lock browser scroller >>");
		elm_object_scroll_freeze_pop(browser_view->m_scroller);
		elm_object_scroll_freeze_push(browser_view->m_scroller);
#endif
	}
}

/**
 * @brief Browser_View::__option_header_url_layout_mouse_down_cb 点击url输入框是弹出的毛玻璃效果
 * @param data  回调函数传入的参数
 * @param evas  evas对象
 * @param obj   obj对象
 * @param event_info
 */
void Browser_View::__option_header_url_layout_mouse_down_cb(void *data, Evas* evas, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Evas_Event_Mouse_Down event = *(Evas_Event_Mouse_Down *)event_info;
	Browser_View *browser_view = (Browser_View *)data;

	Evas_Object *entry = br_elm_editfield_entry_get(browser_view->m_option_header_url_edit_field);

	int entry_x = 0;
	int entry_y = 0;
	int entry_w = 0;
	int entry_h = 0;
	evas_object_geometry_get(browser_view->m_option_header_url_edit_field, &entry_x, &entry_y, &entry_w, &entry_h);

	if (event.output.x > entry_x && event.output.x < entry_x + entry_w &&
	    event.output.y > entry_y && event.output.y < entry_y + entry_h) {
         elm_object_signal_emit(browser_view->m_option_header_url_edit_field, "ellipsis_hide,signal", "elm");
         elm_object_signal_emit(browser_view->m_url_edit_field, "ellipsis_hide,signal", "elm");
	}

    int icon_y = 0;
       if (event.output.y < icon_y)
          browser_view->_jump_to_top();
}

void Browser_View::_update_back_forward_buttons(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (ewk_view_back_possible(m_focused_window->m_ewk_view)) {
		elm_object_disabled_set(m_title_backward_button, EINA_FALSE);
		elm_object_disabled_set(m_option_header_title_backward_button, EINA_FALSE);
	} else {
		elm_object_disabled_set(m_title_backward_button, EINA_TRUE);
		elm_object_disabled_set(m_option_header_title_backward_button, EINA_TRUE);
	}

	if (ewk_view_forward_possible(m_focused_window->m_ewk_view)) {
		elm_object_disabled_set(m_title_forward_button, EINA_FALSE);
		elm_object_disabled_set(m_option_header_title_forward_button, EINA_FALSE);
	} else {
		elm_object_disabled_set(m_title_forward_button, EINA_TRUE);
		elm_object_disabled_set(m_option_header_title_forward_button, EINA_TRUE);
	}
}

void Browser_View::__backward_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	ewk_view_back(browser_view->m_focused_window->m_ewk_view);
}

void Browser_View::__forward_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	ewk_view_forward(browser_view->m_focused_window->m_ewk_view);
}

/**
 * @brief Browser_View::_create_url_layout 创建url输入框
 * @return  返回创建的对象
 */

Evas_Object *Browser_View::_create_url_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *url_layout;
	url_layout = elm_layout_add(m_navi_bar);
	if (url_layout) {
		if (!elm_layout_file_set(url_layout, BROWSER_EDJE_DIR"/browser-view-url-layout.edj",
					"browser/url_layout")) {
			BROWSER_LOGE("Can not set layout theme[browser/url_layout]\n");
			return NULL;
		}
		evas_object_size_hint_weight_set(url_layout, EVAS_HINT_EXPAND, 0.0);
		evas_object_size_hint_align_set(url_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(url_layout);

		/* create url entry layout in url layout */
		m_url_entry_layout = elm_layout_add(m_navi_bar);
		if (!m_url_entry_layout) {
			BROWSER_LOGE("elm_layout_add failed");
			return NULL;
		}
		if (!elm_layout_file_set(m_url_entry_layout, BROWSER_EDJE_DIR"/browser-view-url-layout.edj",
					"elm/browser/urlentry/default")) {
			BROWSER_LOGE("Can not set layout theme[browser, urlentry, default]\n");
			return NULL;
		}
		elm_object_part_content_set(url_layout, "elm.swallow.url", m_url_entry_layout);
		evas_object_show(m_url_entry_layout);

		m_url_edit_field = br_elm_url_editfield_add(m_navi_bar);
		if (!m_url_edit_field) {
			BROWSER_LOGE("elm_editfield_add failed");
			return NULL;
		}
		elm_object_part_content_set(m_url_entry_layout, "elm.swallow.entry", m_url_edit_field);
//		br_elm_editfield_entry_single_line_set(m_url_edit_field, EINA_TRUE);
		br_elm_editfield_eraser_set(m_url_edit_field, EINA_FALSE);

		Evas_Object *edit_field_entry = br_elm_editfield_entry_get(m_url_edit_field);
		elm_entry_editable_set(edit_field_entry, EINA_FALSE);
		/* The edit field in browser scroller is only for display.
		  * The edit url is only supported in option header url entry. */
		elm_object_focus_allow_set(m_url_edit_field, EINA_FALSE);
		elm_object_focus_allow_set(edit_field_entry, EINA_FALSE);
		elm_entry_input_panel_enabled_set(edit_field_entry, EINA_FALSE);

		elm_entry_text_style_user_push(edit_field_entry, "DEFAULT='font_size=35 color=#3C3632 ellipsis=1'");
		evas_object_show(m_url_edit_field);

		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "disable_entry,signal", "");
		edje_object_signal_callback_add(elm_layout_edje_get(m_url_entry_layout), "mouse,clicked,1", "block_entry",
						__url_entry_clicked_cb, this);

		edje_object_signal_callback_add(elm_layout_edje_get(m_url_entry_layout), "refresh_stop", "*",
						__refresh_button_clicked_cb, this);		

		m_url_progressbar = elm_progressbar_add(m_navi_bar);
		if (!m_url_progressbar) {
			BROWSER_LOGE("elm_progressbar_add failed");
			return NULL;
		}
		elm_object_style_set(m_url_progressbar, "browser/loading");
		elm_object_part_content_set(m_url_entry_layout, "elm.swallow.progressbar", m_url_progressbar);
		elm_progressbar_value_set(m_url_progressbar, 0);
		evas_object_show(m_url_progressbar);

		m_url_progresswheel = elm_progressbar_add(m_navi_bar);
		if (!m_url_progresswheel) {
			BROWSER_LOGE("elm_progressbar_add failed");
			return NULL;
		}

		elm_object_style_set(m_url_progresswheel, "UIActivityIndicatorStyleWhite");
		elm_progressbar_pulse(m_url_progresswheel, EINA_FALSE);
		elm_object_part_content_set(m_url_entry_layout, "elm.swallow.progress", m_url_progresswheel);
		evas_object_show(m_url_progresswheel);

		m_cancel_button = elm_button_add(m_navi_bar);
		if (!m_cancel_button) {
			BROWSER_LOGE("elm_button_add failed");
			return NULL;
		}
		evas_object_size_hint_weight_set(m_cancel_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(m_cancel_button, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_style_set(m_cancel_button, "browser/cancel_button");
		elm_object_text_set(m_cancel_button, BR_STRING_CANCEL);
		elm_object_part_content_set(url_layout, "elm.swallow.cancel", m_cancel_button);
		evas_object_smart_callback_add(m_cancel_button, "clicked", __cancel_button_clicked_cb, this);
		evas_object_show(m_cancel_button);

		m_title_backward_button = elm_button_add(m_navi_bar);
		if (!m_title_backward_button) {
			BROWSER_LOGE("elm_button_add failed");
			return NULL;
		}
		elm_object_style_set(m_title_backward_button, "browser/backward");
		elm_object_part_content_set(url_layout, "elm.swallow.backward_button", m_title_backward_button);
		evas_object_smart_callback_add(m_title_backward_button, "clicked", __backward_button_clicked_cb, this);
		elm_object_focus_allow_set(m_title_backward_button, EINA_FALSE);
		evas_object_show(m_title_backward_button);
		elm_object_disabled_set(m_title_backward_button, EINA_TRUE);

		m_title_forward_button = elm_button_add(m_navi_bar);
		if (!m_title_forward_button) {
			BROWSER_LOGE("elm_button_add failed");
			return NULL;
		}
		elm_object_style_set(m_title_forward_button, "browser/forward");
		elm_object_part_content_set(url_layout, "elm.swallow.forward_button", m_title_forward_button);
		evas_object_smart_callback_add(m_title_forward_button, "clicked", __forward_button_clicked_cb, this);
		elm_object_focus_allow_set(m_title_forward_button, EINA_FALSE);
		evas_object_show(m_title_forward_button);
		elm_object_disabled_set(m_title_forward_button, EINA_TRUE);

		/* Workaround.
		 * When edit mode, if scroll down on url layout in browser view,
		 * the browser can be scrolled even though scroll locked.
		 * So, lock the browser scroller whenever touch on it if edit mode.
		 */
		evas_object_event_callback_add(url_layout, EVAS_CALLBACK_MOUSE_DOWN, __url_layout_mouse_down_cb, this);
	}

	return url_layout;
}

Evas_Object *Browser_View::_create_find_word_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *url_layout = elm_layout_add(m_navi_bar);
	if (!url_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	if (!elm_layout_file_set(url_layout, BROWSER_EDJE_DIR"/browser-view-find-word-layout.edj",
		"browser-view/find_word_layout")) {
		BROWSER_LOGE("Can not set layout theme[browser-view/find_word_layout]\n");
		return NULL;
	}
	evas_object_size_hint_weight_set(url_layout, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(url_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(url_layout);

	/* create url entry layout in url layout */
	m_find_word_entry_layout = elm_layout_add(m_navi_bar);
	if (!m_find_word_entry_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	if (!elm_layout_file_set(m_find_word_entry_layout, BROWSER_EDJE_DIR"/browser-view-find-word-layout.edj",
				"browser-view/find_word_editfield_layout")) {
		BROWSER_LOGE("browser-view/find_word_editfield_layout failed");
		return NULL;
	}
	elm_object_part_content_set(url_layout, "elm.swallow.url", m_find_word_entry_layout);
	evas_object_show(m_find_word_entry_layout);

	edje_object_part_text_set(elm_layout_edje_get(url_layout), "title_text", BR_STRING_FIND_ON_PAGE);

	m_find_word_edit_field = br_elm_find_word_editfield_add(m_navi_bar);
	if (!m_find_word_edit_field) {
		BROWSER_LOGE("elm_editfield_add failed");
		return NULL;
	}
	elm_object_part_content_set(m_find_word_entry_layout, "elm.swallow.entry", m_find_word_edit_field);
	evas_object_show(m_find_word_edit_field);

	edje_object_signal_emit(elm_layout_edje_get(m_find_word_edit_field), "find_word,signal", "elm");

//	br_elm_editfield_entry_single_line_set(m_find_word_edit_field, EINA_TRUE);
	br_elm_editfield_eraser_set(m_find_word_edit_field, EINA_TRUE);

	elm_object_signal_callback_add(m_find_word_edit_field, "elm,eraser,clicked", "elm",
						__find_word_erase_button_clicked_cb, this);

	Evas_Object *find_word_edit_field_entry = br_elm_editfield_entry_get(m_find_word_edit_field);
	elm_entry_entry_set(find_word_edit_field_entry, "");
	evas_object_smart_callback_add(find_word_edit_field_entry, "activated", __find_word_entry_enter_key_cb, this);
	elm_entry_input_panel_layout_set(find_word_edit_field_entry, ELM_INPUT_PANEL_LAYOUT_NORMAL);
	elm_entry_prediction_allow_set(find_word_edit_field_entry, EINA_FALSE);
	ecore_imf_context_input_panel_event_callback_add((Ecore_IMF_Context *)elm_entry_imf_context_get(find_word_edit_field_entry),
			ECORE_IMF_INPUT_PANEL_STATE_EVENT, __find_word_entry_imf_event_cb, this);
	elm_entry_text_style_user_push(find_word_edit_field_entry, "DEFAULT='font_size=35 color=#3C3632 ellipsis=1'");
	evas_object_show(m_find_word_edit_field);

	m_find_word_cancel_button = elm_button_add(m_navi_bar);
	if (!m_find_word_cancel_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(m_find_word_cancel_button, "browser/cancel_button");
	elm_object_text_set(m_find_word_cancel_button, BR_STRING_CANCEL);
	elm_object_part_content_set(url_layout, "elm.swallow.cancel", m_find_word_cancel_button);
	evas_object_show(m_find_word_cancel_button);

	evas_object_smart_callback_add(m_find_word_cancel_button, "clicked", __find_word_cancel_button_clicked_cb, this);

	m_find_word_prev_button = elm_button_add(m_navi_bar);
	if (!m_find_word_prev_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(m_find_word_prev_button, "browser/find_word_prev_but");
	elm_object_part_content_set(url_layout, "elm.swallow.find_word_prev", m_find_word_prev_button);
	evas_object_show(m_find_word_prev_button);
	evas_object_smart_callback_add(m_find_word_prev_button, "clicked", __find_word_prev_button_clicked_cb, this);

	m_find_word_next_button = elm_button_add(m_navi_bar);
	if (!m_find_word_next_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(m_find_word_next_button, "browser/find_word_next_but");
	elm_object_part_content_set(url_layout, "elm.swallow.find_word_next", m_find_word_next_button);
	evas_object_show(m_find_word_next_button);
	evas_object_smart_callback_add(m_find_word_next_button, "clicked", __find_word_next_button_clicked_cb, this);

	const char *current_theme = elm_theme_get(NULL);
	if (current_theme && strstr(current_theme, "white")) {
	} else {
//		edje_object_signal_emit(elm_layout_edje_get(m_find_word_entry_layout),
//									"black_theme,signal", "");
	}

	return url_layout;
}

void Browser_View::__find_word_erase_button_clicked_cb(void *data, Evas_Object *obj,
								const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_View *browser_view = (Browser_View *)data;
	edje_object_part_text_set(elm_layout_edje_get(browser_view->m_option_header_find_word_layout), "elm.index_text", "0/0");
}

void Browser_View::__find_word_cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_set_edit_mode(BR_NO_EDIT_MODE);
	browser_view->m_find_word->find_word("", Browser_Find_Word::BROWSER_FIND_WORD_FORWARD);
}

void Browser_View::__find_word_prev_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout), "hide,control_bar,no_animation,signal", "");
	Evas_Object *edit_field_entry = br_elm_editfield_entry_get(browser_view->m_find_word_edit_field);
	const char *find_word = elm_entry_entry_get(edit_field_entry);
	if (!find_word || !strlen(find_word))
		return;

	int index = browser_view->m_find_word->find_word(find_word, Browser_Find_Word::BROWSER_FIND_WORD_BACKWARD);
	if (index == 1)
		elm_object_focus_set(browser_view->m_find_word_cancel_button, EINA_TRUE);
}
void Browser_View::__find_word_next_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout), "hide,control_bar,no_animation,signal", "");
	Evas_Object *edit_field_entry = br_elm_editfield_entry_get(browser_view->m_find_word_edit_field);
	const char *find_word = elm_entry_entry_get(edit_field_entry);
	if (!find_word || !strlen(find_word))
		return;

	int index = browser_view->m_find_word->find_word(find_word, Browser_Find_Word::BROWSER_FIND_WORD_FORWARD);
	if (index == browser_view->m_find_word->get_match_max_value())
		elm_object_focus_set(browser_view->m_find_word_cancel_button, EINA_TRUE);
}

void Browser_View::__find_word_entry_imf_event_cb(void *data, Ecore_IMF_Context *ctx, int value)
{
	BROWSER_LOGD("value=%d", value);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Evas_Object *find_word_editfield_entry = br_elm_editfield_entry_get(browser_view->m_find_word_edit_field);

	if (value == ECORE_IMF_INPUT_PANEL_STATE_HIDE)
		elm_object_focus_set(find_word_editfield_entry, EINA_FALSE);
	else
		elm_object_focus_set(find_word_editfield_entry, EINA_TRUE);
}

void Browser_View::_enable_browser_scroller_scroll(void)
{
#ifdef BROWSER_SCROLLER_BOUNCING
	if (!ewk_view_vertical_panning_hold_get(m_focused_window->m_ewk_view)
	     && !_is_loading() && !m_is_full_screen) {
		BROWSER_LOGD("<< unlock browser scroller, lock ewk view >>");
		elm_object_scroll_freeze_pop(m_scroller);
		ewk_view_vertical_panning_hold_set(m_focused_window->m_ewk_view, EINA_TRUE);
	}
#endif
}

void Browser_View::_enable_webview_scroll(void)
{
#ifdef BROWSER_SCROLLER_BOUNCING
	if (ewk_view_vertical_panning_hold_get(m_focused_window->m_ewk_view)) {
		BROWSER_LOGD("<< lock browser scroller, unlock ewk view >>");
		elm_object_scroll_freeze_pop(m_scroller);
		elm_object_scroll_freeze_push(m_scroller);
		ewk_view_vertical_panning_hold_set(m_focused_window->m_ewk_view, EINA_FALSE);
	}
#endif
}

void Browser_View::__find_word_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout), "hide,control_bar,no_animation,signal", "");
	Evas_Object *edit_field_entry = br_elm_editfield_entry_get(browser_view->m_find_word_edit_field);
	const char *find_word = elm_entry_entry_get(edit_field_entry);
	if (!find_word || !strlen(find_word))
		return;

	browser_view->m_find_word->init_index();
	browser_view->m_find_word->find_word(find_word, Browser_Find_Word::BROWSER_FIND_WORD_FORWARD);

	elm_object_focus_set(edit_field_entry, EINA_FALSE);
	BROWSER_LOGD("find_word=[%s]", find_word);
}

void Browser_View::__find_word_entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Evas_Object *edit_field_entry = br_elm_editfield_entry_get(browser_view->m_find_word_edit_field);
	const char *find_word = elm_entry_entry_get(edit_field_entry);

	browser_view->m_find_word->init_index();
	browser_view->m_find_word->find_word(find_word, Browser_Find_Word::BROWSER_FIND_WORD_FORWARD);
}


void Browser_View::_navigationbar_title_clicked(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
	Evas_Object *content = elm_object_item_content_get(top_it);
	if (content != m_main_layout)
		return;

	if (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW)
#if defined(FEATURE_MOST_VISITED_SITES)
	    || is_most_visited_sites_running()
#endif
	    || m_edit_mode != BR_NO_EDIT_MODE)
	    return;

	int scroller_x = 0;
	int scroller_y = 0;
	int scroller_w = 0;
	int scroller_h = 0;
	elm_scroller_region_get(m_scroller, &scroller_x, &scroller_y, &scroller_w, &scroller_h);

	if (scroller_y == 0) {
		/* scroller is on top, the url bar in scroller is fully being displayed.
		 * Then, just show/hide the tool bar. */
		const char* state = edje_object_part_state_get(elm_layout_edje_get(m_main_layout),
								"elm.swallow.control_bar", NULL);
		if(state && !strncmp(state, "default", strlen("default")))
			edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,control_bar,signal", "");
		else {
#ifdef BROWSER_SCROLLER_BOUNCING
			int url_layout_h = 0;
			evas_object_geometry_get(m_url_layout, NULL, NULL, NULL, &url_layout_h);
			elm_scroller_region_bring_in(m_scroller ,scroller_x, url_layout_h, scroller_w, scroller_h);
#endif
		}
	} else {
		if (_is_loading()) {
		} else {
			Eina_Bool visible = _navigationbar_visible_get();
#ifdef BROWSER_SCROLLER_BOUNCING
			_navigationbar_visible_set(!visible);
#endif
		}
	}
}


void Browser_View::_jump_to_top(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
	Evas_Object *content = elm_object_item_content_get(top_it);
	if (content != m_main_layout)
		return;

	if (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW)
#if defined(FEATURE_MOST_VISITED_SITES)
	    || is_most_visited_sites_running()
#endif
	    || m_edit_mode != BR_NO_EDIT_MODE)
		return;

	int browser_scroller_x = 0;
	int browser_scroller_w = 0;
	int browser_scroller_h = 0;
	elm_scroller_region_get(m_scroller, &browser_scroller_x, NULL,
				&browser_scroller_w, &browser_scroller_h);
	elm_scroller_region_show(m_scroller, browser_scroller_x, 0, browser_scroller_w, browser_scroller_h);

	ewk_view_top_of_contents_go(m_focused_window->m_ewk_view);
}

void Browser_View::delete_non_user_created_windows(void)
{
	BROWSER_LOGD("[%s]", __func__);
	/* Delete no-backward history window. */
	std::vector<Browser_Window *> window_list = m_browser->get_window_list();
	int window_count = window_list.size();

	if (window_count <= 1)
		return;

	int i = 0;
	for (i = 0 ; i < window_count ; i++) {
		BROWSER_LOGD("focusable_window = %d, window[%d]=%d", m_focused_window, i, window_list[i]);
		if (window_list[i]->m_ewk_view
		     &&!ewk_view_back_possible(window_list[i]->m_ewk_view)
		     && window_list[i] != m_focused_window
		     && window_list[i]->m_created_by_user == EINA_FALSE) {
			BROWSER_LOGD("delete window index=[%d]", i);
			m_browser->delete_window(window_list[i]);

			/* Set title & url with the focused window. */
			_set_navigationbar_title(get_title().c_str());
			_set_url_entry(get_url().c_str());
			_set_controlbar_back_forward_status();
			if (!_set_favicon())
				BROWSER_LOGE("_set_favicon failed");
		}
	}
}

void Browser_View::__title_back_button_clicked_cb(void *data , Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	if (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW)) {
		BROWSER_LOGD("close multi window");
		m_data_manager->get_multi_window_view()->close_multi_window();
		return;
	}

	Browser_View *browser_view = (Browser_View *)data;

	if (browser_view->m_focused_window->m_ewk_view
	    && ewk_view_back_possible(browser_view->m_focused_window->m_ewk_view))
	    	ewk_view_back(browser_view->m_focused_window->m_ewk_view);
	else {
		if (browser_view->m_focused_window->m_parent) {
			/* Save current window pointer to delete later. */
			Browser_Window *delete_window = browser_view->m_focused_window;
			browser_view->m_browser->set_focused_window(browser_view->m_focused_window->m_parent);
			browser_view->m_browser->delete_window(delete_window);

			/* Set title & url with the focused window. */
			browser_view->_set_navigationbar_title(browser_view->get_title().c_str());
			browser_view->_set_url_entry(browser_view->get_url().c_str());
			browser_view->_set_controlbar_back_forward_status();
			if (!browser_view->_set_favicon())
				BROWSER_LOGE("_set_favicon failed");
		} else {
			elm_win_lower(browser_view->m_win);

			browser_view->delete_non_user_created_windows();
		}
	}
}

void Browser_View::__scroller_scroll_cb(void *data, Evas_Object *obj, void *event_info)
{
#ifdef BROWSER_SCROLLER_BOUNCING
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	int browser_scroller_x = 0;
	int browser_scroller_y = 0;
	int browser_scroller_w = 0;
	int browser_scroller_h = 0;
	elm_scroller_region_get(browser_view->m_scroller, &browser_scroller_x, &browser_scroller_y,
				&browser_scroller_w, &browser_scroller_h);

	int url_layout_h = 0;
	evas_object_geometry_get(browser_view->m_url_layout, NULL, NULL, NULL, &url_layout_h);

	/* Workaround.
	 * If user scrolls up/down near the url bar edge, the movement of scroller is not smooth.
	 * It's because the browser scroller region y is also bouncing.
	 * So if the scroller region y is bigger than the height of url bar(65 pixel),
	 * make the region y to url bar height by force. */
	if (browser_scroller_y > url_layout_h)
		elm_scroller_region_show(browser_view->m_scroller, browser_scroller_x, url_layout_h,
					browser_scroller_w, browser_scroller_h);
#endif
}

Eina_Bool Browser_View::_is_loading(void)
{
	if (!m_focused_window || !m_focused_window->m_ewk_view)
		return EINA_FALSE;

	double progress = ewk_view_load_progress_get(m_focused_window->m_ewk_view);
//	BROWSER_LOGD("progress=%f", progress);
	if (progress == 1.0f || progress < 0.05f)
		return EINA_FALSE;
	else
		return EINA_TRUE;
}

Eina_Bool Browser_View::launch_find_word_with_text(const char *text_to_find)
{
	BROWSER_LOGD("\n");

	if (_get_edit_mode() == BR_FIND_WORD_MODE)
		return EINA_FALSE;

	_navigationbar_visible_set_signal(EINA_TRUE);
	_set_edit_mode(BR_FIND_WORD_MODE);

	if (!text_to_find || !strlen(text_to_find))
		elm_object_focus_set(m_find_word_edit_field, EINA_TRUE);
	else
		elm_object_focus_set(m_find_word_edit_field, EINA_FALSE);
	edje_object_part_text_set(elm_layout_edje_get(m_option_header_find_word_layout), "elm.index_text", "0/0");

	Evas_Object *find_word_edit_field_entry = br_elm_editfield_entry_get(m_find_word_edit_field);
	elm_entry_entry_set(find_word_edit_field_entry, text_to_find);

	return EINA_TRUE;
}

#if defined(HORIZONTAL_UI)
Eina_Bool Browser_View::is_available_to_rotate(void)
{
	BROWSER_LOGD("\n");
	if (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW)) {
		if (m_data_manager->get_multi_window_view()->is_reordering_mode())
			return EINA_FALSE;
	}

	if (m_scissorbox_view)
		return EINA_FALSE;

	return EINA_TRUE;
}

Eina_Bool Browser_View::__rotate_multi_window_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_View *browser_view = (Browser_View *)data;
	browser_view->m_multi_window_rotate_timer = NULL;
	browser_view->_rotate_multi_window();

	return ECORE_CALLBACK_CANCEL;
}

void Browser_View::rotate(int degree)
{
	BROWSER_LOGD("\n");
	/* In landscape mode, if the url bar is not displayed -> rotate to portrait mode.
	 * Then the url bar is displayed. To avoid this issue, remember the scroller region y,
	 * then restore it at portrait mode. */
	int scroller_region_y = 0;
	elm_scroller_region_get(m_scroller, NULL, &scroller_region_y, NULL, NULL);
	m_scroller_region_y = scroller_region_y;

	m_rotate_degree = degree;

#if defined(FEATURE_MOST_VISITED_SITES)
	if (m_most_visited_sites)
		m_most_visited_sites->rotate();
#endif
	if (m_scissorbox_view)
		_destroy_scissorbox_view();
}
#endif

void Browser_View::suspend_ewk_view(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	std::vector<Browser_Window *> window_list = m_browser->get_window_list();
	for (int i = 0 ; i < window_list.size() ; i++) {
		if (window_list[i]->m_ewk_view_layout) {
			edje_object_signal_emit(elm_layout_edje_get(window_list[i]->m_ewk_view_layout),
								"unresizable,signal", "");

		}
	}

	ewk_view_suspend(ewk_view);
	ewk_view_visibility_set(ewk_view, EINA_FALSE);
}

void Browser_View::resume_ewk_view(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW))
		return;

	ewk_view_resume(ewk_view);
	ewk_view_visibility_set(ewk_view, EINA_TRUE);
}

void Browser_View::pause(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_is_loading()) {
		/* Because the progress wheel in url bar has about 15% cpu consumption.
		  * So, pause the animation when browser goes to background. It's nonsense!*/
		if (m_option_header_url_progresswheel)
			elm_progressbar_pulse(m_option_header_url_progresswheel, EINA_FALSE);
		if (m_url_progresswheel)
			elm_progressbar_pulse(m_url_progresswheel, EINA_FALSE);
	}

	set_full_sreen(EINA_FALSE);

	ug_pause();

	suspend_ewk_view(m_focused_window->m_ewk_view);

	/* stop vibration */
	_haptic_device_stop();
}

void Browser_View::resume(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_is_loading()) {
		/* Because the progress wheel in url bar has about 15% cpu consumption.
		  * So, pause the animation when browser goes to background. It's nonsense! */
		if (m_option_header_url_progresswheel)
			elm_progressbar_pulse(m_option_header_url_progresswheel, EINA_TRUE);
		if (m_url_progresswheel)
			elm_progressbar_pulse(m_url_progresswheel, EINA_TRUE);
	}

	ug_resume();

	resume_ewk_view(m_focused_window->m_ewk_view);
}

void Browser_View::reset(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_focused_window && m_focused_window->m_ewk_view)
		resume();
}

Eina_Bool Browser_View::__webview_layout_resize_idler_cb(void *data)
{
	BROWSER_LOGD("\n");
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	Browser_View *browser_view = (Browser_View *)data;

	browser_view->m_resize_idler = NULL;

	edje_object_signal_emit(elm_layout_edje_get(browser_view->m_focused_window->m_ewk_view_layout),
						"resizable,signal", "");

	int content_w = 0;
	int content_h = 0;
#ifdef BROWSER_SCROLLER_BOUNCING	
	edje_object_part_geometry_get(elm_layout_edje_get(browser_view->m_main_layout),
				"elm.swallow.content", NULL, NULL, &content_w, &content_h);
#else
	edje_object_part_geometry_get(elm_layout_edje_get(browser_view->m_focused_window->m_ewk_view_layout),
				"elm.swallow.webview", NULL, NULL, &content_w, &content_h);
#endif
	if (browser_view->m_is_full_screen) {
		int browser_scroller_y = 0;
		evas_object_geometry_get(browser_view->m_scroller, NULL, &browser_scroller_y, NULL, NULL);
		evas_object_resize(browser_view->m_focused_window->m_ewk_view,
					content_w, content_h + browser_scroller_y);
	} else {
		evas_object_size_hint_min_set(browser_view->m_focused_window->m_ewk_view_layout,
					content_w, content_h);
		evas_object_resize(browser_view->m_focused_window->m_ewk_view_layout,
					content_w, content_h);
	}

	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool Browser_View::__scroller_bring_in_idler_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return ECORE_CALLBACK_CANCEL;
	Browser_View *browser_view = (Browser_View *)data;

	int url_layout_h = 0;
	evas_object_geometry_get(browser_view->m_url_layout, NULL, NULL, NULL, &url_layout_h);

	BROWSER_LOGD("<< elm_scroller_region_bring_in , url_layout_h=%d >>", url_layout_h);
	int browser_scroller_x = 0;
	int browser_scroller_w = 0;
	int browser_scroller_h = 0;			
	elm_scroller_region_get(browser_view->m_scroller, &browser_scroller_x, NULL,
					&browser_scroller_w, &browser_scroller_h);
	elm_scroller_region_show(browser_view->m_scroller, browser_scroller_x, url_layout_h,
					browser_scroller_w, browser_scroller_h);

	return ECORE_CALLBACK_CANCEL;
}

/* Workaround.
 * If user invokes the keypad via input field or url entry, resize the webview.
 * The only scroller resize is called when the keypad is launched.
 * Other elements like layout, conformant resize event doesn't come. */
void Browser_View::__scroller_resize_cb(void* data, Evas* evas, Evas_Object* obj, void* ev)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	if (browser_view->m_is_full_screen) {
		ecore_idler_add(__webview_layout_resize_idler_cb, browser_view);
		return;
	}

	int scroller_w = 0;
	int scroller_h = 0;
	evas_object_geometry_get(browser_view->m_scroller, NULL, NULL, &scroller_w, &scroller_h);
	std::vector<Browser_Window *> window_list = m_browser->get_window_list();

	for (int i = 0 ; i < window_list.size() ; i++) {
		if (window_list[i]->m_ewk_view_layout) {
			edje_object_signal_emit(elm_layout_edje_get(window_list[i]->m_ewk_view_layout),
								"resizable,signal", "");

			evas_object_size_hint_min_set(window_list[i]->m_ewk_view_layout,
										scroller_w, scroller_h);
			evas_object_resize(window_list[i]->m_ewk_view_layout,
									scroller_w, scroller_h);
		}
	}

#if defined(HORIZONTAL_UI)
	Evas_Object *edit_field_entry;
	edit_field_entry = br_elm_editfield_entry_get(browser_view->_get_activated_url_entry());
	Ecore_IMF_Context *ic = (Ecore_IMF_Context *)elm_entry_imf_context_get(edit_field_entry);
	/* If the keypad from webkit is invoked in landscape mode, the visible viewport is too narrow.
	  * So, hide the url bar in browser scroller by bring in.
	  * The direct bring in call doesn't work, so do it by idler. */
	if (ic && browser_view->is_landscape()) {
		Ecore_IMF_Input_Panel_State imf_state = ecore_imf_context_input_panel_state_get(ic);
		/* Strangley, The state of webkit's imf is show-state even if the ic is url entry.
		  * However, this is good for browser without webkit's ic get API. */
		if (imf_state == ECORE_IMF_INPUT_PANEL_STATE_SHOW) {
			browser_view->_enable_browser_scroller_scroll();
			ecore_idler_add(__scroller_bring_in_idler_cb, browser_view);
		}
	}
#endif

	if (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW)) {
		BROWSER_LOGD("close multi window");
		browser_view->m_is_multi_window_grid_mode = m_data_manager->get_multi_window_view()->_is_grid_mode();
		m_data_manager->get_multi_window_view()->close_multi_window();

#if defined(HORIZONTAL_UI)
		if (browser_view->m_multi_window_rotate_timer)
			ecore_timer_del(browser_view->m_multi_window_rotate_timer);
		browser_view->m_multi_window_rotate_timer = ecore_timer_add(0.2, __rotate_multi_window_cb, browser_view);
#endif
	}
}


#ifdef ZOOM_BUTTON
Eina_Bool Browser_View::__zoom_button_timeout_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->m_zoom_button_timer = NULL;

	edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout), "hide,zoom_in_buttons,signal", "");
	edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout), "hide,zoom_out_buttons,signal", "");

	return ECORE_CALLBACK_CANCEL;
}

void Browser_View::__zoom_out_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	double min_scale = 0;
	double max_scale = 0;
	double scale_factor = ewk_view_scale_get(browser_view->m_focused_window->m_ewk_view);
	ewk_view_scale_set(browser_view->m_focused_window->m_ewk_view, scale_factor - 0.5f, 0, 0);
	ewk_view_scale_range_get(browser_view->m_focused_window->m_ewk_view,
					&min_scale, &max_scale);

	if ((scale_factor - 0.5f) <= min_scale)
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout), "hide,zoom_out_buttons,signal", "");

	if (scale_factor == max_scale)
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout), "show,zoom_in_buttons,signal", "");

	if (browser_view->m_zoom_button_timer)
		ecore_timer_del(browser_view->m_zoom_button_timer);
	browser_view->m_zoom_button_timer = ecore_timer_add(3, __zoom_button_timeout_cb, browser_view);
}

void Browser_View::__zoom_in_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	double min_scale = 0;
	double max_scale = 0;
	double scale_factor = ewk_view_scale_get(browser_view->m_focused_window->m_ewk_view);
	ewk_view_scale_set(browser_view->m_focused_window->m_ewk_view, scale_factor + 0.5f, 0, 0);
	ewk_view_scale_range_get(browser_view->m_focused_window->m_ewk_view,
					&min_scale, &max_scale);

	if ((scale_factor + 0.5f) >= max_scale)
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout), "hide,zoom_in_buttons,signal", "");

	if (scale_factor == min_scale)
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout), "show,zoom_out_buttons,signal", "");

	if (browser_view->m_zoom_button_timer)
		ecore_timer_del(browser_view->m_zoom_button_timer);
	browser_view->m_zoom_button_timer = ecore_timer_add(3, __zoom_button_timeout_cb, browser_view);
}

Eina_Bool Browser_View::_create_zoom_buttons(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_zoom_out_button = elm_button_add(m_navi_bar);
	if (!m_zoom_out_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_zoom_out_button, "browser/zoom_out");
	elm_object_part_content_set(m_main_layout, "elm.swallow.zoom_out_button", m_zoom_out_button);
	evas_object_smart_callback_add(m_zoom_out_button, "clicked", __zoom_out_clicked_cb, this);
	evas_object_show(m_zoom_out_button);

	m_zoom_in_button = elm_button_add(m_navi_bar);
	if (!m_zoom_in_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}

	elm_object_style_set(m_zoom_in_button, "browser/zoom_in");
	elm_object_part_content_set(m_main_layout, "elm.swallow.zoom_in_button", m_zoom_in_button);
	evas_object_smart_callback_add(m_zoom_in_button, "clicked", __zoom_in_clicked_cb, this);
	evas_object_show(m_zoom_in_button);

	return EINA_TRUE;
}
#endif

void Browser_View::__naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	if (browser_view->m_navi_it != elm_naviframe_top_item_get(m_navi_bar))
		return;

	m_data_manager->destroy_bookmark_view();
	m_data_manager->destroy_history_layout();
	/* Add to bookmark, then cancel. */
	m_data_manager->destroy_add_to_bookmark_view();
#if defined(FEATURE_MOST_VISITED_SITES)
	m_data_manager->destroy_add_to_most_visited_sites_view();
#endif

	/* If return from browser settings. */
	if (browser_view->m_browser_settings) {
		delete browser_view->m_browser_settings;
		browser_view->m_browser_settings = NULL;
	}
}

void Browser_View::__dim_area_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	elm_object_focus_set(m_data_manager->get_browser_view()->m_option_header_cancel_button, EINA_TRUE);
}

void Browser_View::__context_menu_bg_mouse_down_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	browser_view->_destroy_more_context_popup();
}

/**
 * @brief Browser_View::_create_main_layout 创建UI
 * @return
 */
Eina_Bool Browser_View::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_main_layout = elm_layout_add(m_navi_bar);
	if (!m_main_layout) {
		BROWSER_LOGE("elm_layout_add failed!");
		return EINA_FALSE;
	}

	if (!elm_layout_file_set(m_main_layout, BROWSER_EDJE_DIR"/browser-view-main.edj",
				"browser/browser-view-main")) {
		BROWSER_LOGE("elm_layout_file_set failed", BROWSER_EDJE_DIR);
		return EINA_FALSE;
	}

	evas_object_size_hint_weight_set(m_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_main_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_main_layout);

	m_title_back_button = elm_button_add(m_navi_bar);
	if (!m_title_back_button) {
		BROWSER_LOGE("elm_button_add failed!");
		return EINA_FALSE;
	}

	evas_object_smart_callback_add(m_title_back_button, "clicked", __title_back_button_clicked_cb, this);

	m_navi_it = elm_naviframe_item_push(m_navi_bar, "", NULL, NULL, m_main_layout, NULL);

	evas_object_smart_callback_add(m_navi_bar, "transition,finished", __naviframe_pop_finished_cb, this);

	elm_object_style_set(m_title_back_button, "browser/title_back");
	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_TITLE_LEFT_BTN, m_title_back_button);

	evas_object_show(m_title_back_button);
	evas_object_show(m_navi_bar);

	m_scroller = elm_scroller_add(m_navi_bar);
	if (!m_scroller) {
		BROWSER_LOGE("elm_scroller_add failed!");
		return EINA_FALSE;
	}
	/* Do not propagate event to scroller's ancestor */
	/* not to call unnecessary other callbacks. */
	evas_object_propagate_events_set(m_scroller, EINA_FALSE);

	/* Do not use scroller's scrollbar, use webview's scrollbar instead */
	elm_scroller_policy_set(m_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	evas_object_size_hint_align_set(m_scroller, EVAS_HINT_FILL, 0.0);
	evas_object_size_hint_weight_set(m_scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
#ifdef BROWSER_SCROLLER_BOUNCING
//	elm_scroller_bounce_set(m_scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_bounce_set(m_scroller, EINA_FALSE, EINA_FALSE);
#else
	elm_scroller_bounce_set(m_scroller, EINA_FALSE, EINA_FALSE);
	elm_object_scroll_freeze_pop(m_scroller);
	elm_object_scroll_freeze_push(m_scroller);
#endif
	evas_object_show(m_scroller);

	evas_object_smart_callback_add(m_scroller, "scroll", __scroller_scroll_cb, this);
	evas_object_event_callback_add(m_scroller, EVAS_CALLBACK_RESIZE, __scroller_resize_cb, this);

	/* create content box which contains navigation layout & webview */
	m_content_box = elm_box_add(m_main_layout);
	if (!m_content_box) {
		BROWSER_LOGE("elm_box_add failed!");
		return EINA_FALSE;
	}

	elm_box_horizontal_set(m_content_box, EINA_FALSE);
	evas_object_size_hint_weight_set(m_content_box, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(m_content_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(m_scroller, m_content_box);
	evas_object_show(m_content_box);

	/* create dummy loading progress bar which is displayed at launching time shortly */
	m_dummy_loading_progressbar = elm_progressbar_add(m_navi_bar);
	if (!m_dummy_loading_progressbar) {
		BROWSER_LOGE("elm_progressbar_add failed!");
		return EINA_FALSE;
	}

	elm_object_style_set(m_dummy_loading_progressbar, "browser/loading_wheel");
	elm_progressbar_pulse(m_dummy_loading_progressbar, EINA_TRUE);
	evas_object_size_hint_weight_set(m_dummy_loading_progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_dummy_loading_progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(m_main_layout, "elm.swallow.waiting_progress", m_dummy_loading_progressbar);
	evas_object_show(m_dummy_loading_progressbar);

	/* create conformant */
	elm_win_conformant_set(m_win, EINA_TRUE);
	m_conformant = elm_conformant_add(m_main_layout);
	if (!m_conformant) {
		BROWSER_LOGE("elm_conformant_add failed!");
		return EINA_FALSE;
	}

	elm_object_style_set(m_conformant, "internal_layout");
	evas_object_size_hint_weight_set(m_conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_conformant, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(m_conformant, m_scroller);
	elm_object_part_content_set(m_main_layout, "elm.swallow.content", m_conformant);
	evas_object_show(m_conformant);

	m_url_layout = _create_url_layout();
	if (!m_url_layout) {
		BROWSER_LOGE("_create_url_layout failed!");
		return EINA_FALSE;
	}
	elm_box_pack_start(m_content_box, m_url_layout);

	m_control_bar = _create_control_bar();
	if (!m_control_bar) {
		BROWSER_LOGE("_create_control_bar failed!");
		return EINA_FALSE;
	}
	elm_object_part_content_set(m_main_layout, "elm.swallow.control_bar", m_control_bar);

	m_option_header_url_layout = _create_option_header_url_layout();
	if (!m_option_header_url_layout) {
		BROWSER_LOGE("_create_option_header_url_layout failed!");
		return EINA_FALSE;
	}

	m_option_header_layout = elm_layout_add(m_navi_bar);
	if (!m_option_header_layout) {
		BROWSER_LOGE("elm_layout_add failed!");
		return EINA_FALSE;
	}
	if (!elm_layout_file_set(m_option_header_layout, BROWSER_EDJE_DIR"/browser-view-url-layout.edj",
				"browser-view/option_header")) {
		BROWSER_LOGE("Can not set layout theme[browser/url_layout]\n");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_option_header_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_option_header_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_content_set(m_option_header_layout, "elm.swallow.url_layout", m_option_header_url_layout);
	evas_object_show(m_option_header_layout);

	m_option_header_find_word_layout = _create_find_word_layout();
	if (!m_option_header_find_word_layout) {
		BROWSER_LOGE("_create_find_word_layout failed");
		return EINA_FALSE;
	}
	elm_object_part_content_set(m_option_header_layout, "elm.swallow.find_word_layout", m_option_header_find_word_layout);

	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_OPTIONHEADER, m_option_header_layout);
	_navigationbar_visible_set_signal(EINA_FALSE);

#ifdef ZOOM_BUTTON
	if (!_create_zoom_buttons()) {
		BROWSER_LOGE("_create_zoom_buttons failed");
		return EINA_FALSE;
	}
#endif

	edje_object_signal_callback_add(elm_layout_edje_get(m_main_layout),
					"mouse,clicked,1", "elm.rect.content_dim", __dim_area_clicked_cb, this);

	edje_object_signal_callback_add(elm_layout_edje_get(m_main_layout),
					"mouse,down,1", "elm.rect.more_context_bg", __context_menu_bg_mouse_down_cb, this);

	const char *current_theme = elm_theme_get(NULL);
	if (current_theme && strstr(current_theme, "white")) {
	} else {
//		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "black_theme,signal", "");
//		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
//										"black_theme,signal", "");
	}

	return EINA_TRUE;
}

