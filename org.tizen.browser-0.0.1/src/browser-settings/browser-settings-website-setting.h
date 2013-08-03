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

#ifndef BROWSER_SETTINGS_WEBSITE_SETTING_H
#define BROWSER_SETTINGS_WEBSITE_SETTING_H

#include "browser-geolocation-db.h"
#include "browser-common-view.h"
#include "browser-config.h"
#include "browser-settings-main-view.h"

class Browser_Settings_Website_Setting : public Browser_Common_View {
public:
	Browser_Settings_Website_Setting(Browser_Settings_Main_View *main_view);
	~Browser_Settings_Website_Setting(void);

	Eina_Bool init(void);

	struct website_setting_item {
		std::string url;
		Eina_Bool geolocation;
		Eina_Bool allow;
		Eina_Bool storage;
		void *origin;
		void *user_data;
	};
	typedef enum _menu_type {
		BR_MENU_LOCATION,
		BR_MENU_WEB_STORAGE,
		BR_MENU_UNKNOWN
	} menu_type;
private:
	typedef struct _genlist_callback_data {
		menu_type type;
		void *user_data;
		void *cp;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Eina_Bool _create_main_layout(void);
	void _website_list_get(void);
	void _show_details_list(website_setting_item *item_info);
	Eina_Bool _show_clear_location_confirm_popup(void);
	Eina_Bool _show_clear_web_storage_confirm_popup(void);
	Eina_Bool _show_delete_all_website_list_confirm_popup(void);
	Eina_Bool _show_delete_all_details_list_confirm_popup(void);
	void _refresh_website_list(void);
	void _refresh_details_list(website_setting_item *item_info);

	/* genlist callback functions */
	static Evas_Object *__genlist_icon_get(void *data, Evas_Object *obj, const char *part);
	static char *__genlist_label_get(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__details_genlist_icon_get(void *data, Evas_Object *obj, const char *part);
	static char *__details_genlist_label_get(void *data, Evas_Object *obj, const char *part);

	/* Elementary event callback functions */
	static void __item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __details_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_all_website_list_conbar_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_all_details_list_conbar_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	static void __application_cache_origin_get_cb(Eina_List* origins, void* user_data);
	static void __web_storage_origin_get_cb(Eina_List* origins, void* user_data);
	static void __web_database_origin_get_cb(Eina_List* origins, void* user_data);

	/*Popup event callback function */
	static void __clear_location_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_clear_location_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clear_web_storage_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_clear_web_storage_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_all_website_list_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_delete_all_website_list_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_all_details_list_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_delete_all_details_list_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);


	Evas_Object *m_genlist;
	Evas_Object *m_details_genlist;
	Evas_Object *m_back_button;
	Evas_Object *m_back_button_details;
	Evas_Object *m_clear_confirm_popup;
	Evas_Object *m_delete_all_website_list_confirm_popup;
	Evas_Object *m_delete_all_details_list_confirm_popup;
	Evas_Object *m_bottom_control_bar;
	Evas_Object *m_details_bottom_control_bar;
	Elm_Object_Item *m_delete_all_website_list_conbar_item;
	Elm_Object_Item *m_delete_all_details_conbar_item;
	Elm_Genlist_Item_Class m_1_text_2_icon_item_class;
	Elm_Genlist_Item_Class m_1_text_1_icon_item_class;

	genlist_callback_data m_location_item_callback_data;
	genlist_callback_data m_web_storage_item_callback_data;

	vector<Browser_Geolocation_DB::geolocation_info *> m_geolocation_list;
	vector<website_setting_item *> m_website_setting_list;
	website_setting_item *m_current_website_item_data;
	website_setting_item *m_current_item_data;

	Browser_Settings_Main_View *m_main_view;

	/* Workaround - flags for origin callbacks for checking callbacks are called.
	   This patch is needed because all origin callbacks are called asyncronously*/
	bool m_is_called_appcache_cb_flag;
	bool m_is_called_webstorage_cb_flag;
	bool m_is_called_webdb_cb_flag;
};

#endif /* BROWSER_SETTINGS_WEBSITE_SETTING_H */

