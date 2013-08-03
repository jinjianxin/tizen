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

#ifndef BROWSER_CLASS_H
#define BROWSER_CLASS_H

#include "browser-config.h"
#include "browser-view.h"

class Browser_Geolocation;
class Browser_Network_Manager;
class Browser_Policy_Decision_Maker;
class Browser_View;
class Browser_Window;
class Browser_User_Agent_DB;

class Browser_Class {
public:
	Browser_Class(Evas_Object *win, Evas_Object *navi_bar, Evas_Object *bg);
	~Browser_Class(void);

	Eina_Bool init(void);
	Eina_Bool launch(const char *url, Eina_Bool new_window = EINA_FALSE);

#ifdef APP_IN_APP
	Eina_Bool is_app_in_app(void);
#endif
#if defined(HORIZONTAL_UI)
	void rotate(int degree);
	Eina_Bool is_available_to_rotate(void);
#endif

	Browser_Window *create_new_window(Eina_Bool created_by_user = EINA_FALSE);
	void delete_window(Browser_Window *delete_window, Browser_Window *parent);
	void delete_window(Browser_Window *window);
	void change_order(std::vector<Browser_Window *> window_list);
	std::vector<Browser_Window *> get_window_list(void) { return m_window_list; }
	void set_focused_window(Browser_Window *window
#if defined(FEATURE_MOST_VISITED_SITES)
		, Eina_Bool show_most_visited_sites = EINA_TRUE
#endif
	);
	void pause(void);
	void resume(void);
	void reset(void);
	void ewk_view_init(Evas_Object *ewk_view);
	void ewk_view_deinit(Evas_Object *ewk_view);
	std::string get_user_agent(void);
	void clean_up_windows(void);
	Browser_Window *create_deleted_window(int index);
	Browser_Geolocation *get_geolocation(void) { return m_geolocation; }
private:
	static Eina_Bool __create_network_manager_idler_cb(void *data);
	Eina_Bool _set_ewk_view_options(Evas_Object *ewk_view);
	Eina_Bool _set_ewk_view_options_listener(void);
	Eina_Bool _set_user_agent(Evas_Object *ewk_view);
	/* prefernece changed callback functions */
	static void __preference_changed_cb(const char *key, void *data);
	/* vconf changed callback functions */
	static void __vconf_changed_cb(keynode_t *keynode, void *data);
	Eina_Bool _set_http_accepted_language_header(Evas_Object *ewk_view);

	/* Ecore timer callback functions */
	static Eina_Bool __clean_up_windows_timer_cb(void *data);

	Evas_Object *m_win;
	Evas_Object *m_navi_bar;
	Evas_Object *m_bg;

	Browser_View *m_browser_view;
	Browser_Window *m_focused_window;
	Browser_User_Agent_DB *m_user_agent_db;
	Browser_Policy_Decision_Maker *m_download_policy;
	Browser_Geolocation *m_geolocation;
	Browser_Network_Manager *m_network_manager;

	std::vector<Browser_Window *> m_window_list;
	Ecore_Timer *m_clean_up_windows_timer;
};
#endif /* BROWSER_CLASS_H */

