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
  *@file browser-class.cpp
  *@brief 浏览器逻辑处理
  */

#include "browser-class.h"
#include "browser-context-menu.h"
#include "browser-find-word.h"
#include "browser-geolocation.h"
#include "browser-multi-window-view.h"
#include "browser-network-manager.h"
#include "browser-user-agent-db.h"
#include "browser-window.h"
#include "browser-policy-decision-maker.h"

Browser_Class::Browser_Class(Evas_Object *win, Evas_Object *navi_bar, Evas_Object *bg)
:
	m_win(win)
	,m_navi_bar(navi_bar)
	,m_bg(bg)
	,m_browser_view(NULL)
	,m_focused_window(NULL)
	,m_user_agent_db(NULL)
	,m_download_policy(NULL)
	,m_clean_up_windows_timer(NULL)
	,m_geolocation(NULL)
	,m_network_manager(NULL)
{
	m_window_list.clear();
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Class::~Browser_Class(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_browser_view)
		delete m_browser_view;
	if (m_user_agent_db)
		delete m_user_agent_db;
	if (m_download_policy)
		delete m_download_policy;
	if (m_geolocation)
		delete m_geolocation;
	if (m_network_manager)
		delete m_network_manager;

	for (int i = 0 ; i < m_window_list.size() ; i++) {
		if (m_window_list[i])
			delete m_window_list[i];
		m_window_list.erase(m_window_list.begin() + i);
	}

	if (m_clean_up_windows_timer)
		ecore_timer_del(m_clean_up_windows_timer);
}

Eina_Bool Browser_Class::__create_network_manager_idler_cb(void *data)
{
	Browser_Class *browser = (Browser_Class *)data;

	browser->m_network_manager = new(nothrow) Browser_Network_Manager;
	if (!browser->m_network_manager) {
		BROWSER_LOGE("new Browser_Network_Manager failed");
		return ECORE_CALLBACK_CANCEL;
	}
	if (!browser->m_network_manager->init(browser->m_browser_view)) {
		BROWSER_LOGE("m_network_manager->init failed");
		return ECORE_CALLBACK_CANCEL;
	}

	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool Browser_Class::init(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_user_agent_db = new(nothrow) Browser_User_Agent_DB;
	if (!m_user_agent_db) {
		BROWSER_LOGE("new Browser_User_Agent_DB failed");
		return EINA_FALSE;
	}

	m_browser_view = new(nothrow) Browser_View(m_win, m_navi_bar, m_bg, this);
	/* Create browser view layout */
	if (m_browser_view) {
        if (!m_browser_view->init()) { //调用Browser_View的初始化函数
			BROWSER_LOGE("m_browser_view->init failed");
			return EINA_FALSE;
		}
	} else
		return EINA_FALSE;

	m_download_policy = new(nothrow) Browser_Policy_Decision_Maker(m_navi_bar, m_browser_view);
	if (!m_download_policy) {
		BROWSER_LOGE("new Browser_Policy_Decision_Maker failed");
		return EINA_FALSE;
	}

	m_geolocation = new(nothrow) Browser_Geolocation;
	if (!m_geolocation) {
		BROWSER_LOGE("new Browser_Geolocation failed");
		return EINA_FALSE;
	}

	m_network_manager = new(nothrow) Browser_Network_Manager;
	if (!m_network_manager) {
		BROWSER_LOGE("new Browser_Network_Manager failed");
		return EINA_FALSE;
	}
	if (!m_network_manager->init(m_browser_view)) { //调用Browser_Network_Manager::init
		BROWSER_LOGE("m_network_manager->init failed");
		return EINA_FALSE;
	}

	if (!_set_ewk_view_options_listener()) {
		BROWSER_LOGE("_set_ewk_view_options_listener failed");
		return EINA_FALSE;
	}


    ewk_context_cache_model_set(ewk_context_default_get(), EWK_CACHE_MODEL_PRIMARY_WEBBROWSER);

	return EINA_TRUE;
}
/**
 *配置文件改变时的回调函数
 */
void Browser_Class::__preference_changed_cb(const char *key, void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Class *browser = (Browser_Class *)data;

	if (!key || strlen(key) == 0) {
		BROWSER_LOGD("vconf_keynode_get_name failed");
		return;
	}

	if (!strncmp(key, DEFAULT_VIEW_LEVEL_KEY, strlen(DEFAULT_VIEW_LEVEL_KEY))) {
		for (int i = 0 ; i < browser->m_window_list.size() ; i++) {
			char *default_level = NULL;
			if (br_preference_get_str(DEFAULT_VIEW_LEVEL_KEY, &default_level) == false) {
				BROWSER_LOGE("failed to get %s preference\n", DEFAULT_VIEW_LEVEL_KEY);
				return;
			}
			if (default_level && browser->m_window_list[i]->m_ewk_view) {
				Ewk_Setting *setting = ewk_view_setting_get(browser->m_window_list[i]->m_ewk_view);
				if (!strncmp(default_level, FIT_TO_WIDTH, strlen(FIT_TO_WIDTH)))
					ewk_setting_auto_fitting_set(setting, EINA_TRUE);
				else
					ewk_setting_auto_fitting_set(setting, EINA_FALSE);
				free(default_level);
				default_level = NULL;
			}
			if (default_level)
				free(default_level);
		}
	} else if (!strncmp(key, RUN_JAVASCRIPT_KEY, strlen(RUN_JAVASCRIPT_KEY))) {
		for (int i = 0 ; i < browser->m_window_list.size() ; i++) {
			if (browser->m_window_list[i]->m_ewk_view) {
				Ewk_Setting *setting = ewk_view_setting_get(browser->m_window_list[i]->m_ewk_view);
				bool run_javascript = 1;
				br_preference_get_bool(RUN_JAVASCRIPT_KEY, &run_javascript);
				if (run_javascript)
					ewk_setting_enable_scripts_set(setting, EINA_TRUE);
				else
					ewk_setting_enable_scripts_set(setting, EINA_FALSE);
			}
		}
	} else if (!strncmp(key, DISPLAY_IMAGES_KEY, strlen(DISPLAY_IMAGES_KEY))) {
		for (int i = 0 ; i < browser->m_window_list.size() ; i++) {
			if (browser->m_window_list[i]->m_ewk_view) {
				Ewk_Setting *setting = ewk_view_setting_get(browser->m_window_list[i]->m_ewk_view);
				bool display_images = 1;
				br_preference_get_bool(DISPLAY_IMAGES_KEY, &display_images);
				if (display_images)
					ewk_setting_auto_load_images_set(setting, EINA_TRUE);
				else
					ewk_setting_auto_load_images_set(setting, EINA_FALSE);
			}
		}
	} else if (!strncmp(key, ACCEPT_COOKIES_KEY, strlen(ACCEPT_COOKIES_KEY))) {
		bool accept_cookies = 1;
		br_preference_get_bool(ACCEPT_COOKIES_KEY, &accept_cookies);
		if (accept_cookies)
			ewk_context_cookies_policy_set(ewk_context_default_get(), EWK_COOKIE_JAR_ACCEPT_ALWAYS);
		else
			ewk_context_cookies_policy_set(ewk_context_default_get(), EWK_COOKIE_JAR_ACCEPT_NEVER);
	} else if (!strncmp(key, BLOCK_POPUP_KEY, strlen(BLOCK_POPUP_KEY))) {
		for (int i = 0 ; i < browser->m_window_list.size() ; i++) {
			if (browser->m_window_list[i]->m_ewk_view) {
				Ewk_Setting *setting = ewk_view_setting_get(browser->m_window_list[i]->m_ewk_view);
				bool block_popup = 0;
				br_preference_get_bool(BLOCK_POPUP_KEY, &block_popup);
				if (block_popup)
					ewk_setting_scripts_window_open_set(setting, EINA_FALSE);
				else
					ewk_setting_scripts_window_open_set(setting, EINA_TRUE);
			}
		}
	} else if (!strncmp(key, SHOW_SECURITY_WARNINGS_KEY, strlen(SHOW_SECURITY_WARNINGS_KEY))) {
			bool security_warnings = 1;
			br_preference_get_bool(SHOW_SECURITY_WARNINGS_KEY, &security_warnings);
	}
}

void Browser_Class::__vconf_changed_cb(keynode_t *keynode, void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Class *browser = (Browser_Class *)data;
	char *key = vconf_keynode_get_name(keynode);

	if (!key || strlen(key) == 0) {
		BROWSER_LOGD("vconf_keynode_get_name failed");
		return;
	}

	if (!strncmp(key, USERAGENT_KEY, strlen(USERAGENT_KEY))) {
		for (int i = 0 ; i < browser->m_window_list.size() ; i++) {
			if (browser->m_window_list[i]->m_ewk_view) {
				if (!browser->_set_user_agent(browser->m_window_list[i]->m_ewk_view))
					BROWSER_LOGE("_set_user_agent failed");
			}
		}
	}
}

Eina_Bool Browser_Class::_set_ewk_view_options_listener(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (vconf_notify_key_changed(USERAGENT_KEY, __vconf_changed_cb, this) < 0) {
		BROWSER_LOGE("user agent vconf_notify_key_changed failed");
		return EINA_FALSE;
	}

	if (br_preference_set_changed_cb(DEFAULT_VIEW_LEVEL_KEY, __preference_changed_cb, this) < 0) {
		BROWSER_LOGE("default view level br_preference_set_changed_cb failed");
		return EINA_FALSE;
	}
	if (br_preference_set_changed_cb(RUN_JAVASCRIPT_KEY, __preference_changed_cb, this) < 0) {
		BROWSER_LOGE("run javascript level br_preference_set_changed_cb failed");
		return EINA_FALSE;
	}
	if (br_preference_set_changed_cb(DISPLAY_IMAGES_KEY, __preference_changed_cb, this) < 0) {
		BROWSER_LOGE("display images level br_preference_set_changed_cb failed");
		return EINA_FALSE;
	}
	if (br_preference_set_changed_cb(ACCEPT_COOKIES_KEY, __preference_changed_cb, this) < 0) {
		BROWSER_LOGE("accept cookie br_preference_set_changed_cb failed");
		return EINA_FALSE;
	}
	if (!br_preference_set_changed_cb(BLOCK_POPUP_KEY, __preference_changed_cb, this)) {
		BROWSER_LOGE("BLOCK_POPUP_KEY vconf_notify_key_changed failed");
		return EINA_FALSE;
	}
	if (!br_preference_set_changed_cb(SHOW_SECURITY_WARNINGS_KEY, __preference_changed_cb, this)) {
		BROWSER_LOGE("SHOW_SECURITY_WARNINGS_KEY vconf_notify_key_changed failed");
		return EINA_FALSE;
	}
	return EINA_TRUE;
}
/**
 * @brief Browser_Class::_set_ewk_view_options 设置浏览器选项
 * @param ewk_view ewkview对象
 * @return  是否设置成功
 */

Eina_Bool Browser_Class::_set_ewk_view_options(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!_set_user_agent(ewk_view))
		BROWSER_LOGE("_set_user_agent failed");

	ewk_context_icon_database_path_set(ewk_view_context_get(ewk_view), BROWSER_FAVICON_DB_PATH);

	Ewk_Setting *setting = ewk_view_setting_get(ewk_view);
	ewk_setting_show_ime_on_autofocus_set(setting, EINA_FALSE);

	char *default_level = NULL;
	if (br_preference_get_str(DEFAULT_VIEW_LEVEL_KEY, &default_level) == false) {
		BROWSER_LOGE("failed to get %s preference", DEFAULT_VIEW_LEVEL_KEY);
		return EINA_FALSE;
	}
	if (default_level) {
		if (!strncmp(default_level, FIT_TO_WIDTH, strlen(FIT_TO_WIDTH)))
			ewk_setting_auto_fitting_set(setting, EINA_TRUE);
		else
			ewk_setting_auto_fitting_set(setting, EINA_FALSE);
		free(default_level);
	}

	bool run_javascript = 1;
	br_preference_get_bool(RUN_JAVASCRIPT_KEY, &run_javascript);
	if (run_javascript)
		ewk_setting_enable_scripts_set(setting, EINA_TRUE);
	else
		ewk_setting_enable_scripts_set(setting, EINA_FALSE);

	bool display_images = 1;
	br_preference_get_bool(DISPLAY_IMAGES_KEY, &display_images);
	if (display_images)
		ewk_setting_enable_plugins_set(setting, EINA_TRUE);
	else
		ewk_setting_enable_plugins_set(setting, EINA_FALSE);

	bool accept_cookies = 1;
	br_preference_get_bool(ACCEPT_COOKIES_KEY, &accept_cookies);
	if (accept_cookies)
		ewk_context_cookies_policy_set(ewk_context_default_get(), EWK_COOKIE_JAR_ACCEPT_ALWAYS);
	else
		ewk_context_cookies_policy_set(ewk_context_default_get(), EWK_COOKIE_JAR_ACCEPT_NEVER);

	bool block_popup = 0;
	br_preference_get_bool(BLOCK_POPUP_KEY, &block_popup);
	if (block_popup)
		ewk_setting_scripts_window_open_set(setting, EINA_FALSE);
	else
		ewk_setting_scripts_window_open_set(setting, EINA_TRUE);

	ewk_view_recording_surface_enable_set(ewk_view, EINA_TRUE);
	ewk_setting_layer_borders_enable_set(ewk_view_setting_get(ewk_view), false);
	ewk_setting_enable_plugins_set(setting, EINA_TRUE);

	if (!_set_http_accepted_language_header(ewk_view)) {
		BROWSER_LOGE("_set_http_accepted_language_header failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

/**
 * @brief Browser_Class::ewk_view_deinit 删除wekview的回调函数
 * @param ewk_view ewkview对象
 */

void Browser_Class::ewk_view_deinit(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);
    evas_object_smart_callback_del(ewk_view, "load,started", Browser_View::__load_started_cb);
	evas_object_smart_callback_del(ewk_view, "load,committed", Browser_View::__load_committed_cb);
	evas_object_smart_callback_del(ewk_view, "load,nonemptylayout,finished",
						Browser_View::__load_nonempty_layout_finished_cb);
	evas_object_smart_callback_del(ewk_view, "title,changed", Browser_View::__title_changed_cb);
	evas_object_smart_callback_del(ewk_view, "load,progress", Browser_View::__load_progress_cb);
	evas_object_smart_callback_del(ewk_view, "load,finished", Browser_View::__did_finish_load_for_frame_cb);
	evas_object_smart_callback_del(ewk_view, "process,crashed", Browser_View::__process_crashed_cb);
	evas_object_event_callback_del(ewk_view, EVAS_CALLBACK_MOUSE_DOWN,
					Browser_View::__ewk_view_mouse_down_cb);
	evas_object_event_callback_del(ewk_view, EVAS_CALLBACK_MOUSE_UP,
					Browser_View::__ewk_view_mouse_up_cb);
	evas_object_smart_callback_del(ewk_view, "edge,top",
					Browser_View::__ewk_view_edge_top_cb);
	evas_object_smart_callback_del(ewk_view, "edge,bottom",
					Browser_View::__ewk_view_scroll_down_cb);
	evas_object_smart_callback_del(ewk_view, "scroll,down",
					Browser_View::__ewk_view_scroll_down_cb);
	evas_object_smart_callback_del(ewk_view, "scroll,up",
					Browser_View::__ewk_view_scroll_up_cb);
	evas_object_smart_callback_del(ewk_view, "fullscreen,enterfullscreen",
					Browser_View::__ewk_view_enter_full_screen_cb);
	evas_object_smart_callback_del(ewk_view, "fullscreen,exitfullscreen",
					Browser_View::__ewk_view_exit_full_screen_cb);
	evas_object_smart_callback_del(ewk_view, "vibration,vibrate",
					Browser_View::__ewk_view_vibration_vibrate_cb);
	evas_object_smart_callback_del(ewk_view, "vibration,cancel",
					Browser_View::__ewk_view_vibration_cancel_cb);

	evas_object_smart_callback_del(ewk_view, "create,window", Browser_View::__create_window_cb);
	evas_object_smart_callback_del(ewk_view, "close,window", Browser_View::__close_window_cb);

	evas_object_smart_callback_del(ewk_view, "request,geolocation,permission",
					Browser_Geolocation::__geolocation_permission_request_cb);
	m_download_policy->deinit();
	m_browser_view->m_context_menu->deinit();
	m_browser_view->suspend_ewk_view(ewk_view);
}

/**
 * @brief Browser_Class::ewk_view_init  设置ewk对象回调函数
 * @param ewk_view ewk对象实例
 */
void Browser_Class::ewk_view_init(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);
	ewk_view_deinit(ewk_view);

	evas_object_smart_callback_add(ewk_view, "load,started",
					Browser_View::__load_started_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "load,committed",
					Browser_View::__load_committed_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "load,nonemptylayout,finished",
					Browser_View::__load_nonempty_layout_finished_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "title,changed",
					Browser_View::__title_changed_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "load,progress",
					Browser_View::__load_progress_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "load,finished",
					Browser_View::__did_finish_load_for_frame_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "process,crashed",
					Browser_View::__process_crashed_cb, m_browser_view);
	evas_object_event_callback_add(ewk_view, EVAS_CALLBACK_MOUSE_DOWN,
					Browser_View::__ewk_view_mouse_down_cb, m_browser_view);
	evas_object_event_callback_add(ewk_view, EVAS_CALLBACK_MOUSE_UP,
					Browser_View::__ewk_view_mouse_up_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "edge,top",
					Browser_View::__ewk_view_edge_top_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "edge,bottom",
					Browser_View::__ewk_view_scroll_down_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "scroll,down",
					Browser_View::__ewk_view_scroll_down_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "scroll,up",
					Browser_View::__ewk_view_scroll_up_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "fullscreen,enterfullscreen",
					Browser_View::__ewk_view_enter_full_screen_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "fullscreen,exitfullscreen",
					Browser_View::__ewk_view_exit_full_screen_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "vibration,vibrate",
					Browser_View::__ewk_view_vibration_vibrate_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "vibration,cancel",
					Browser_View::__ewk_view_vibration_cancel_cb, m_browser_view);

	evas_object_smart_callback_add(ewk_view, "create,window", Browser_View::__create_window_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view, "close,window", Browser_View::__close_window_cb, m_browser_view);

	evas_object_smart_callback_add(ewk_view, "request,geolocation,permission",
					Browser_Geolocation::__geolocation_permission_request_cb, m_browser_view);

	m_download_policy->init(ewk_view);
	m_browser_view->m_context_menu->init(ewk_view);
	m_geolocation->init(ewk_view);
	m_browser_view->resume_ewk_view(ewk_view);
}

/**
 * @brief Browser_Class::get_user_agent 获得当前设置的agent
 * @return 返回代理字符串
 */
std::string Browser_Class::get_user_agent(void)
{
	BROWSER_LOGD("[%s]", __func__);
	char *user_agent_title = vconf_get_str(USERAGENT_KEY);
	if (!user_agent_title) {
		BROWSER_LOGE("vconf_get_str(USERAGENT_KEY) failed.");
		user_agent_title = strdup(BROWSER_DEFAULT_USER_AGENT_TITLE);
		if (!user_agent_title) {
			BROWSER_LOGE("strdup(BROWSER_DEFAULT_USER_AGENT_TITLE) failed.");
			return std::string();
		}
	}

	char *user_agent = NULL;
	if (!m_user_agent_db->get_user_agent(user_agent_title, user_agent)) {
		BROWSER_LOGE("m_user_agent_db->get_user_agent failed");
	}

	if (user_agent_title)
		free(user_agent_title);

	if (user_agent) {
		std::string return_string = std::string(user_agent);
		free(user_agent);
		return return_string;
	} else
		return std::string();
}

/**
 * @brief Browser_Class::_set_user_agent  设置浏览器代理
 * @param ewk_view ewkview对象
 * @return 是否设置成功
 */
Eina_Bool Browser_Class::_set_user_agent(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

    char *user_agent_title = vconf_get_str(USERAGENT_KEY);
    if (!user_agent_title) {
        BROWSER_LOGE("vconf_get_str(USERAGENT_KEY) failed.");
        user_agent_title = strdup("Tizen");
        if (!user_agent_title) {
            BROWSER_LOGE("strdup(BROWSER_DEFAULT_USER_AGENT_TITLE) failed.");
            return EINA_FALSE;
        }
    }

#define TIZEN_USER_AGENT "Mozilla/5.0 (Linux; U; Tizen 2.0; en-us) AppleWebKit/537.1 (KHTML, like Gecko) Mobile TizenBrowser/2.0"
#define CHROME_USER_AGENT "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/536.11 (KHTML, like Gecko) Chrome/20.0.1132.57 Safari/536.11"

    if (strncmp(user_agent_title, "Chrome 20", strlen("Chrome 20"))) {
        if (!ewk_view_user_agent_set(ewk_view, TIZEN_USER_AGENT));
            BROWSER_LOGE("ewk_view_setting_user_agent_set failed");
    } else {
        if (!ewk_view_user_agent_set(ewk_view, CHROME_USER_AGENT));
            BROWSER_LOGE("ewk_view_setting_user_agent_set failed");
    }

	return EINA_TRUE;
}

/**
 * @brief Browser_Class::_set_http_accepted_language_header 设置http头
 * @param ewk_view ewkview对象
 * @return 是否设置成功
 */

Eina_Bool Browser_Class::_set_http_accepted_language_header(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	char *system_language_locale = NULL;
	char system_language[3] = {0,};

	system_language_locale = vconf_get_str(VCONFKEY_LANGSET);
	BROWSER_LOGD("system language and locale is [%s]\n", system_language_locale);
	if (!system_language_locale) {
		BROWSER_LOGD("Failed to get system_language, set as English");
		strncpy(system_language, "en", 2); /* Copy language set as english */
	} else {
		/* Copy language set from system using 2byte, ex)ko */
		strncpy(system_language, system_language_locale, 2);
	}

	if (system_language_locale)
		free(system_language_locale);

	if (!ewk_view_custom_header_add(ewk_view, "Accept-Language", system_language)) {
		BROWSER_LOGD("ewk_view_setting_custom_header_add is failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

/**
 * @brief Browser_Class::set_focused_window 设置当前网页焦点
 * @param window Browser_Window对象
 * @param show_most_visited_sites ？
 */

void Browser_Class::set_focused_window(Browser_Window *window
#if defined(FEATURE_MOST_VISITED_SITES)
	, Eina_Bool show_most_visited_sites
#endif
	)
{
	if (m_focused_window == window || !window)
		return;

	if (m_focused_window) {
		if (m_focused_window->m_favicon)
			evas_object_hide(m_focused_window->m_favicon);
		if (m_focused_window->m_option_header_favicon)
			evas_object_hide(m_focused_window->m_option_header_favicon);
		if (m_focused_window->m_ewk_view) {
			ewk_view_deinit(m_focused_window->m_ewk_view);
			evas_object_hide(m_focused_window->m_ewk_view);
		}
	}

	m_focused_window = window;

	/* If the ewk view is deleted because of unused case.(etc. low memory)
	  * create the ewk view and load url. */
	if (!m_focused_window->m_ewk_view) {
		int index = 0;
		for (index = 0 ; index < m_window_list.size() ; index++) {
			if (m_focused_window == m_window_list[index])
				break;
		}

		if (m_focused_window != create_deleted_window(index))
			BROWSER_LOGD("create_deleted_window failed");

		/* Workaround.
		  * If launch the browser by aul, the grey bg is displayed shortly. */
		edje_object_signal_emit(elm_layout_edje_get(m_browser_view->m_main_layout),
						"hide,grey_background,signal", "");

		ewk_view_init(m_focused_window->m_ewk_view);

		m_browser_view->set_focused_window(m_focused_window
#if defined(FEATURE_MOST_VISITED_SITES)
			, show_most_visited_sites
#endif
			);
		m_browser_view->load_url(m_focused_window->m_url.c_str());
	} else {
		ewk_view_init(m_focused_window->m_ewk_view);
		m_browser_view->set_focused_window(m_focused_window
#if defined(FEATURE_MOST_VISITED_SITES)
			, show_most_visited_sites
#endif
			);
	}
}

void Browser_Class::change_order(std::vector<Browser_Window *> window_list)
{
	BROWSER_LOGD("[%s]", __func__);
	m_window_list.clear();
	m_window_list = window_list;
}

/**
 * @brief Browser_Class::delete_window 删除标签页
 * @param delete_window  Browser_Window对象
 * @param parent Browser_Window父对象
 */

void Browser_Class::delete_window(Browser_Window *delete_window, Browser_Window *parent)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_window_list.size() <= 1)
		return;

	int index = 0;
	for (index = 0 ; index < m_window_list.size() ; index++) {
		if (delete_window == m_window_list[index])
			break;
	}

	for (int i = 0 ; i < m_window_list.size() ; i++) {
		if (delete_window == m_window_list[i]->m_parent)
			m_window_list[i]->m_parent = NULL;
	}

	if (parent)
		set_focused_window(parent
#if defined(FEATURE_MOST_VISITED_SITES)
		, EINA_FALSE
#endif
		);

	delete m_window_list[index];
	m_window_list.erase(m_window_list.begin() + index);

	m_browser_view->_set_multi_window_controlbar_text(m_window_list.size());
}

/**
 * @brief Browser_Class::delete_window 删除标签页
 * @param window 要删除的window
 */

void Browser_Class::delete_window(Browser_Window *window)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_window_list.size() <= 1)
		return;

	int index = 0;
	for (index = 0 ; index < m_window_list.size() ; index++) {
		if (window == m_window_list[index])
			break;
	}

	if (window == m_focused_window) {
		if (index == 0)
			set_focused_window(m_window_list[index + 1]);
		else
			set_focused_window(m_window_list[index - 1]);
	}

	for (int i = 0 ; i < m_window_list.size() ; i++) {
		if (window == m_window_list[i]->m_parent)
			m_window_list[i]->m_parent = NULL;
	}

	delete m_window_list[index];
	m_window_list.erase(m_window_list.begin() + index);

	m_browser_view->_set_multi_window_controlbar_text(m_window_list.size());
}

/* This destroy all windows except current focused window.
  * However, keep the snapshot and url, title. */

/**
 * @brief Browser_Class::clean_up_windows 删除所有窗口
 */

void Browser_Class::clean_up_windows(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_browser_view->m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW)) {
		BROWSER_LOGD("close multi window");
		m_browser_view->m_data_manager->get_multi_window_view()->close_multi_window();
	}

	for (int i = 0 ; i < m_window_list.size() ; i++) {
		if (m_focused_window != m_window_list[i]) {
			m_window_list[i]->m_url = m_browser_view->get_url(m_window_list[i]);
			m_window_list[i]->m_title = m_browser_view->get_title(m_window_list[i]);
			BROWSER_LOGD("delete [%d:%s] window,", i, m_window_list[i]->m_url.c_str());

			if (m_window_list[i]->m_favicon)
				evas_object_hide(m_window_list[i]->m_favicon);
			if (m_window_list[i]->m_option_header_favicon)
				evas_object_hide(m_window_list[i]->m_option_header_favicon);

			if (m_window_list[i]->m_ewk_view) {
				evas_object_del(m_window_list[i]->m_ewk_view);
				m_window_list[i]->m_ewk_view = NULL;
			}
			if (m_window_list[i]->m_ewk_view_layout) {
				evas_object_del(m_window_list[i]->m_ewk_view_layout);
				m_window_list[i]->m_ewk_view_layout = NULL;
			}
		}
	}

	/* Clear memory cache to reduce memory usage in case of low memory. */
	ewk_context_cache_clear(ewk_context_default_get());

	ewk_context_notify_low_memory(ewk_context_default_get());
//	m_browser_view->show_msg_popup("This is a test message. Low memory - clean up windows.", 5);
}

Browser_Window *Browser_Class::create_deleted_window(int index)
{
	if (m_window_list[index]->m_ewk_view == NULL) {
		m_window_list[index]->m_ewk_view = ewk_view_add(evas_object_evas_get(m_win));
		if (!m_window_list[index]->m_ewk_view) {
			BROWSER_LOGE("ewk_view_add failed");
			return NULL;
		}

		evas_object_color_set(m_window_list[index]->m_ewk_view, 255, 255, 255, 255);
#ifdef BROWSER_SCROLLER_BOUNCING
		/* The webview is locked initially. */
		ewk_view_vertical_panning_hold_set(m_window_list[index]->m_ewk_view, EINA_TRUE);
#endif
		evas_object_size_hint_weight_set(m_window_list[index]->m_ewk_view,
						EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(m_window_list[index]->m_ewk_view,
						EVAS_HINT_FILL, EVAS_HINT_FILL);

		if (!_set_ewk_view_options(m_window_list[index]->m_ewk_view))
			BROWSER_LOGE("_set_ewk_view_options failed");

		return m_window_list[index];
	}

	return NULL;
}
/**
 * @brief Browser_Class::create_new_window 创建一个新的标签页
 * @param created_by_user
 * @return 返回创建成功的标签页
 */

Browser_Window *Browser_Class::create_new_window(Eina_Bool created_by_user)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Window *window = new(nothrow) Browser_Window;
	if (!window) {
		BROWSER_LOGE("new Browser_Window failed");
		return NULL;
	}
	window->m_ewk_view = ewk_view_add(evas_object_evas_get(m_win));
	if (!window->m_ewk_view) {
		BROWSER_LOGE("ewk_view_add failed");
		return NULL;
	}

	evas_object_color_set(window->m_ewk_view, 255, 255, 255, 255);
	if (created_by_user)
		window->m_created_by_user = created_by_user;

#ifdef BROWSER_SCROLLER_BOUNCING
	/* The webview is locked initially. */
	ewk_view_vertical_panning_hold_set(window->m_ewk_view, EINA_TRUE);
#endif
	evas_object_size_hint_weight_set(window->m_ewk_view, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(window->m_ewk_view, EVAS_HINT_FILL, EVAS_HINT_FILL);

	if (!_set_ewk_view_options(window->m_ewk_view))
		BROWSER_LOGE("_set_ewk_view_options failed");

	m_window_list.push_back(window);

	return window;
}

/**
 * @brief Browser_Class::launch 加载新的标签页
 * @param url 加载地址
 * @param new_window_flag 是否是新窗口
 * @return 是否加载成功
 */

Eina_Bool Browser_Class::launch(const char *url, Eina_Bool new_window_flag)
{
	BROWSER_LOGD("url=[%s], new_window=%d", url, new_window_flag);

	m_browser_view->delete_non_user_created_windows();

	if (new_window_flag && m_window_list.size() >= BROWSER_MULTI_WINDOW_MAX_COUNT) {
		/* If the multi window is max, delete the first one in case of new window. */
		if (m_window_list[0] != m_focused_window)
			delete_window(m_window_list[0]);
		else
			delete_window(m_window_list[1]);
	}

	if (m_window_list.size() == 0 || new_window_flag) {
		Browser_Window *new_window = NULL;
		/* If browser is launched for the first time, create the first window. */
		if (m_window_list.size() == 0)
			new_window = create_new_window(EINA_TRUE);
		else
			new_window = create_new_window(EINA_FALSE);

		if (!new_window) {
			BROWSER_LOGD("create_new_window failed");
			return EINA_FALSE;
		}

		set_focused_window(new_window);
	}

	if (m_window_list.size())
		m_browser_view->launch(url); //开始请求网页

	return EINA_TRUE;
}

#if defined(HORIZONTAL_UI)
Eina_Bool Browser_Class::is_available_to_rotate(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_browser_view)
		return m_browser_view->is_available_to_rotate();
	else
		return EINA_FALSE;
}

void Browser_Class::rotate(int degree)
{
	BROWSER_LOGD("[%s]", __func__);
	m_browser_view->rotate(degree);
}
#endif

Eina_Bool Browser_Class::__clean_up_windows_timer_cb(void *data)
{
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	BROWSER_LOGD("[%s]", __func__);

	Browser_Class *browser = (Browser_Class *)data;
	browser->m_clean_up_windows_timer = NULL;

	browser->clean_up_windows();
	return ECORE_CALLBACK_CANCEL;
}

/**
 * @brief Browser_Class::pause 暂停应用程序 ，程序进入后台
 */

void Browser_Class::pause(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_browser_view->pause();
	m_download_policy->pause();

	if (m_clean_up_windows_timer)
		ecore_timer_del(m_clean_up_windows_timer);
	m_clean_up_windows_timer = ecore_timer_add(BROWSER_CLEAN_UP_WINDOWS_TIMEOUT,
						__clean_up_windows_timer_cb, this);
}

/**
 * @brief Browser_Class::resume 程序进入前台，恢复运行
 */

void Browser_Class::resume(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_browser_view->resume();

	if (m_clean_up_windows_timer) {
		ecore_timer_del(m_clean_up_windows_timer);
		m_clean_up_windows_timer = NULL;
	}
}

//重置
void Browser_Class::reset(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_clean_up_windows_timer) {
		ecore_timer_del(m_clean_up_windows_timer);
		m_clean_up_windows_timer = NULL;
	}

	m_browser_view->reset();
}

