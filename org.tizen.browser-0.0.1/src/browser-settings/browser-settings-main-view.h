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

#ifndef BROWSER_SETTINGS_MAIN_VIEW_H
#define BROWSER_SETTINGS_MAIN_VIEW_H

#include "browser-config.h"
#include "browser-common-view.h"

class Browser_Settings_Edit_Homepage_View;
class Browser_Settings_Clear_Data_View;
class Browser_Settings_Website_Setting;

class Browser_Settings_User_Agent_View;

class Browser_Settings_Main_View : public Browser_Common_View {
public:
	typedef enum _menu_type
	{
		BR_HOMEPAGE_TITLE = 0,
		BR_HOMEPAGE_MENU,
#if defined(FEATURE_MOST_VISITED_SITES)
		BR_HOMEPAGE_SUBMENU_MOST_VISITED_SITES,
#endif
		BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE,
		BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE,
		BR_HOMEPAGE_SUBMENU_EMPTY_PAGE,
		BR_HOMEPAGE_SUBMENU_CURRENT_PAGE,
		BR_CONTENT_TITLE,
		BR_CONTENT_MENU_DEFAULT_VIEW_LEVEL,
		BR_CONTENT_SUBMENU_FIT_TO_WIDTH,
		BR_CONTENT_SUBMENU_READABLE,
		BR_CONTENT_MENU_RUN_JAVASCRIPT,
		BR_CONTENT_MENU_DISPLAY_IMAGES,
		BR_CONTENT_MENU_BLOCK_POPUP,
		BR_PRIVACY_TITLE,
		BR_PRIVATE_MENU_CLEAR_CACHE,
		BR_PRIVATE_MENU_CLEAR_HISTORY,
		BR_PRIVATE_MENU_SHOW_SECURITY_WARNINGS,
		BR_PRIVACY_MENU_ACCEPT_COOKIES,
		BR_PRIVATE_MENU_CLEAR_ALL_COOKIE_DATA,
		BR_PRIVACY_WEBSITE_SETTING,
		BR_PRIVACY_SUBMENU_ALWAYS_ASK,
		BR_PRIVACY_SUBMENU_ALWAYS_ON,
		BR_PRIVACY_SUBMENU_ALWAYS_OFF,
		BR_PRIVACY_MENU_CLEAR_PRIVATE_DATA,
		BR_PRIVACY_MENU_ENABLE_LOCATION,
		BR_PRIVACY_MENU_CLEAR_LOCATION_ACCESS,
		BR_MENU_CERTIFICATES,
		BR_MENU_RESET_TO_DEFAULT,
		BR_DEBUG_TITLE,
		BR_MENU_USER_AGENT,
		BR_MENU_UNKNOWN
	}menu_type;

	Browser_Settings_Main_View(void);
	~Browser_Settings_Main_View(void);

	Eina_Bool init(void);
	Evas_Object *get_genlist(void) { return m_genlist; }
private:
	typedef struct _genlist_callback_data {
		menu_type type;
		void *user_data;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Eina_Bool _create_main_layout(void);
	Evas_Object *_create_content_genlist(void);
	Eina_Bool _call_user_agent(void);
	Eina_Bool _show_reset_confirm_popup(void);
	void _reset_settings(void);

	Eina_Bool _show_clear_cache_confirm_popup(void);
	Eina_Bool _show_clear_history_confirm_popup(void);
	Eina_Bool _show_clear_all_cookie_data_confirm_popup(void);

	Eina_Bool _show_clear_location_confirm_popup(void);
	static void __clear_location_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clear_location_cancel_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);

	/* genlist callback functions */
	static Evas_Object *__genlist_icon_get(void *data, Evas_Object *obj, const char *part);
	static char *__genlist_label_get(void *data, Evas_Object *obj, const char *part);

	/* elementary event callback functions */
	static void __back_button_clicked_cb(void *data, Evas_Object* obj,
								void* event_info);
	static void __expandable_icon_clicked_cb(void *data, Evas_Object *obj,
								void *event_info);
	static void __on_off_check_clicked_cb(void *data, Evas_Object *obj,
								void *event_info);
	static void __homepage_sub_item_clicked_cb(void *data, Evas_Object *obj,
								void *event_info);
	static void __default_view_level_sub_item_clicked_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __run_javascript_check_changed_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __display_images_check_changed_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __block_popup_check_changed_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __show_security_warnings_check_changed_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __auto_save_id_pass_check_changed_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __auto_save_form_data_check_changed_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __accept_cookies_check_changed_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __enable_location_check_changed_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __reset_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clear_cache_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_clear_cache_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clear_history_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_clear_history_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clear_all_cookie_data_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_clear_all_cookie_data_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info);
	static void __application_cache_origin_get_cb(Eina_List* origins, void* user_data);
	static void __web_storage_origin_get_cb(Eina_List* origins, void* user_data);
	static void __web_database_origin_get_cb(Eina_List* origins, void* user_data);

	Evas_Object *m_genlist;
	Evas_Object *m_back_button;
	Elm_Genlist_Item_Class m_category_title_item_class;
	Elm_Genlist_Item_Class m_2_text_item_class;
	Elm_Genlist_Item_Class m_2_text_3_item_class;
	Elm_Genlist_Item_Class m_1_text_1_icon_item_class;
	Elm_Genlist_Item_Class m_1_text_item_class;
	Elm_Genlist_Item_Class m_2_text_1_icon_item_class;
	Elm_Genlist_Item_Class m_radio_text_item_class;
	Elm_Genlist_Item_Class m_seperator_item_class;

	/* Homepage */
	genlist_callback_data m_homepage_title_callback_data;
	genlist_callback_data m_homepage_item_callback_data;
#if defined(FEATURE_MOST_VISITED_SITES)
	genlist_callback_data m_most_visited_item_callback_data;
#endif
	genlist_callback_data m_recently_visited_item_callback_data;
	genlist_callback_data m_user_homepage_item_callback_data;
	genlist_callback_data m_current_page_item_callback_data;
	genlist_callback_data m_empty_page_item_callback_data;

	/* Display */
	genlist_callback_data m_display_title_callback_data;
	genlist_callback_data m_defailt_view_level_item_callback_data;
	genlist_callback_data m_fit_to_width_item_callback_data;
	genlist_callback_data m_readable_item_callback_data;

	/* Content */
	genlist_callback_data m_content_title_callback_data;
	genlist_callback_data m_run_javascript_item_callback_data;
	genlist_callback_data m_display_images_item_callback_data;
	genlist_callback_data m_block_popup_item_callback_data;

	/* Privacy */
	genlist_callback_data m_privacy_title_callback_data;
	genlist_callback_data m_clear_cache_item_callback_data;
	genlist_callback_data m_clear_history_item_callback_data;
	genlist_callback_data m_show_security_warnings_item_callback_data;
	genlist_callback_data m_accept_cookies_item_callback_data;
	genlist_callback_data m_clear_all_cookies_data_item_callback_data;
	genlist_callback_data m_website_setting_callback_data;
	genlist_callback_data m_clear_private_data_item_callback_data;
	genlist_callback_data m_enable_location_callback_data;
	genlist_callback_data m_clear_location_access_callback_data;

	/* Others */
	genlist_callback_data m_reset_item_callback_data;

	/* Debug */
	genlist_callback_data m_debug_title_callback_data;
	genlist_callback_data m_user_agent_item_callback_data;
	Evas_Object *m_homepage_radio_group;
	Evas_Object *m_default_view_level_radio_group;
	Evas_Object *m_auto_save_id_pass_radio_group;
	Evas_Object *m_run_javascript_check;
	Evas_Object *m_display_images_check;
	Evas_Object *m_block_popup_check;
	Evas_Object *m_clear_cache_confirm_popup;
	Evas_Object *m_clear_history_confirm_popup;
	Evas_Object *m_accept_cookies_check;
	Evas_Object *m_clear_all_cookies_data_confirm_popup;
	Evas_Object *m_enable_location_check;
	Evas_Object *m_clear_location_confirm_popup;
	Evas_Object *m_reset_confirm_popup;
	Evas_Object *m_show_security_warnings_check;
	Elm_Object_Item *m_navi_it;

	Browser_Settings_Edit_Homepage_View *m_edit_homepage_view;
	Browser_Settings_Clear_Data_View *m_clear_data_view;
	Browser_Settings_Website_Setting *m_website_setting;
	Browser_Settings_User_Agent_View *m_user_agent_view;
};

#endif /* BROWSER_SETTINGS_MAIN_VIEW_H */

