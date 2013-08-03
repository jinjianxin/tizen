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


#include "browser-settings-class.h"
#include "browser-settings-website-setting.h"
#include "browser-view.h"

Browser_Settings_Website_Setting::Browser_Settings_Website_Setting(Browser_Settings_Main_View *main_view)
:
	m_genlist(NULL)
	,m_details_genlist(NULL)
	,m_back_button(NULL)
	,m_back_button_details(NULL)
	,m_clear_confirm_popup(NULL)
	,m_delete_all_website_list_confirm_popup(NULL)
	,m_delete_all_details_list_confirm_popup(NULL)
	,m_bottom_control_bar(NULL)
	,m_details_bottom_control_bar(NULL)
	,m_delete_all_website_list_conbar_item(NULL)
	,m_delete_all_details_conbar_item(NULL)
	,m_current_website_item_data(NULL)
	,m_current_item_data(NULL)
	,m_is_called_appcache_cb_flag(false)
	,m_is_called_webstorage_cb_flag(false)
	,m_is_called_webdb_cb_flag(false)
	,m_main_view(main_view)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Settings_Website_Setting::~Browser_Settings_Website_Setting(void)
{
	BROWSER_LOGD("[%s]", __func__);

	for (int i = 0 ; i < m_website_setting_list.size() ; i++) {
		if (m_website_setting_list[i])
			delete m_website_setting_list[i];
	}
	m_website_setting_list.clear();
}

Eina_Bool Browser_Settings_Website_Setting::init(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!_create_main_layout()) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Evas_Object *Browser_Settings_Website_Setting::__genlist_icon_get(void *data,
						Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	BROWSER_LOGD("part = [%s]", part);

	Browser_Geolocation_DB::geolocation_info *info = (Browser_Geolocation_DB::geolocation_info *)data;

	if (part && strlen(part) > 0) {
		if (!strncmp(part, "elm.icon.1", strlen("elm.icon.1"))) {
			Evas_Object *favicon = NULL;
			favicon = m_data_manager->get_browser_view()->get_favicon(info->url.c_str());
			if (favicon)
				return favicon;
			else {
				Evas_Object *default_icon = elm_icon_add(obj);
				if (!default_icon)
					return NULL;
				if (!elm_icon_file_set(default_icon, BROWSER_IMAGE_DIR"/faviconDefault.png", NULL)) {
					BROWSER_LOGE("elm_icon_file_set is failed.\n");
					return NULL;
				}
				evas_object_size_hint_aspect_set(default_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
				return default_icon;
			}
		} else if (!strncmp(part,"elm.icon.2", strlen("elm.icon.2"))) {
			Evas_Object *location_icon = elm_icon_add(obj);
			if (!location_icon)
				return NULL;

			if (info->geolocation) {
				if (info->allow) {
					if (!elm_icon_file_set(location_icon, BROWSER_IMAGE_DIR"/I01_icon_location_allowed.png", NULL)) {
						BROWSER_LOGE("elm_icon_file_set is failed.\n");
						return NULL;
					}
					evas_object_size_hint_aspect_set(location_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
					return location_icon;
				} else {
					if (!elm_icon_file_set(location_icon, BROWSER_IMAGE_DIR"/I01_icon_location_denied.png", NULL)) {
						BROWSER_LOGE("elm_icon_file_set is failed.\n");
						return NULL;
					}
					evas_object_size_hint_aspect_set(location_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
					return location_icon;
				}
			}
		} else if (!strncmp(part,"elm.icon.3", strlen("elm.icon.3"))) {
			Evas_Object *storage_icon = elm_icon_add(obj);
			if (!storage_icon)
				return NULL;
			if (info->storage) {
				if (!elm_icon_file_set(storage_icon, BROWSER_IMAGE_DIR"/I01_icon_web_storage.png", NULL)) {
					BROWSER_LOGE("elm_icon_file_set is failed.\n");
					return NULL;
				}
				evas_object_size_hint_aspect_set(storage_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
				return storage_icon;
			}
		}
	}

	return NULL;
}

char *Browser_Settings_Website_Setting::__genlist_label_get(void *data,
					Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	BROWSER_LOGD("part = [%s]", part);

	Browser_Geolocation_DB::geolocation_info *info = (Browser_Geolocation_DB::geolocation_info *)data;
	const char *url = info->url.c_str();

	if (part && strlen(part) > 0) {
		if (!strncmp(part,"elm.text", strlen("elm.text"))) {
			if (url && strlen(url))
				return strdup(url);
		}
	}

	return NULL;
}

Evas_Object *Browser_Settings_Website_Setting::__details_genlist_icon_get(void *data,
						Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	BROWSER_LOGD("part = [%s]", part);

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Website_Setting::menu_type type = callback_data->type;
	Browser_Settings_Website_Setting *website_setting = NULL;

	Browser_Geolocation_DB::geolocation_info *info =
		(Browser_Geolocation_DB::geolocation_info *)(callback_data->user_data);

	if (part && strlen(part) > 0) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			if (type == BR_MENU_LOCATION) {
				BROWSER_LOGD("location menu\n");
				Evas_Object *location_icon = elm_icon_add(obj);
				if (!location_icon)
					return NULL;
				if (info->geolocation) {
					if (info->allow) {
						if (!elm_icon_file_set(location_icon, BROWSER_IMAGE_DIR"/I01_icon_location_allowed_small.png", NULL)) {
							BROWSER_LOGE("elm_icon_file_set is failed.\n");
							return NULL;
						}
						evas_object_size_hint_aspect_set(location_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
						return location_icon;
					} else {
						if (!elm_icon_file_set(location_icon, BROWSER_IMAGE_DIR"/I01_icon_location_denied_small.png", NULL)) {
							BROWSER_LOGE("elm_icon_file_set is failed.\n");
							return NULL;
						}
						evas_object_size_hint_aspect_set(location_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
						return location_icon;
					}
				}
			} else if (type == BR_MENU_WEB_STORAGE) {
				BROWSER_LOGD("web storage menu.\n");
				Evas_Object *storage_icon = elm_icon_add(obj);
				if (!storage_icon)
					return NULL;
				if (info->storage) {
					if (!elm_icon_file_set(storage_icon, BROWSER_IMAGE_DIR"/I01_icon_web_storage_small.png", NULL)) {
						BROWSER_LOGE("elm_icon_file_set is failed.\n");
						return NULL;
					}
					evas_object_size_hint_aspect_set(storage_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
					return storage_icon;
				}
			}else {
				BROWSER_LOGD("wrong menu index.\n");
				return NULL;
			}
		}
	}
	return NULL;
}

char *Browser_Settings_Website_Setting::__details_genlist_label_get(void *data,
					Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	BROWSER_LOGD("part = [%s]", part);
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Website_Setting::menu_type type = callback_data->type;
	Browser_Settings_Website_Setting *website_setting = NULL;

	Browser_Geolocation_DB::geolocation_info *info =
		(Browser_Geolocation_DB::geolocation_info *)(callback_data->user_data);
	const char *url = info->url.c_str();

	if (part && strlen(part) > 0) {
		if (!strncmp(part,"elm.text", strlen("elm.text"))) {
			if (type == BR_MENU_LOCATION) {
				BROWSER_LOGD("location menu.\n");
				return strdup(BR_STRING_CLEAR_LOCATION_ACCESS);
			} else if (type == BR_MENU_WEB_STORAGE) {
				BROWSER_LOGD("web storage menu.\n");
				return strdup(BR_STRING_CLEAR_STORED_DATA);
			}else {
				BROWSER_LOGD("wrong menu index.\n");
				return NULL;
			}
		}
	}
	return NULL;
}

void Browser_Settings_Website_Setting::__item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Website_Setting *website_setting = (Browser_Settings_Website_Setting *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	website_setting->m_current_website_item_data = NULL;
	website_setting->m_current_website_item_data = (website_setting_item *)elm_object_item_data_get(it);

	website_setting->_show_details_list(website_setting->m_current_website_item_data);
	elm_genlist_item_selected_set(it, EINA_FALSE);
}

void Browser_Settings_Website_Setting::__details_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Website_Setting::menu_type type = callback_data->type;
	Browser_Settings_Website_Setting *website_setting = NULL;
	website_setting = (Browser_Settings_Website_Setting *)(callback_data->cp);

	website_setting->m_current_item_data = (website_setting_item *)callback_data->user_data;
	if (type == BR_MENU_LOCATION) {
		website_setting->_show_clear_location_confirm_popup();
	} else if (type == BR_MENU_WEB_STORAGE) {
		website_setting->_show_clear_web_storage_confirm_popup();
	}

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

Eina_Bool Browser_Settings_Website_Setting::_show_clear_location_confirm_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_clear_confirm_popup)
		evas_object_del(m_clear_confirm_popup);

	m_clear_confirm_popup = elm_popup_add(m_details_genlist);
	if (!m_clear_confirm_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}

	evas_object_size_hint_weight_set(m_clear_confirm_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_text_set(m_clear_confirm_popup, BR_STRING_CLEAR_LOCATION_ACCESS_FOR_THIS_WEBSITE);

	Evas_Object *ok_button = elm_button_add(m_clear_confirm_popup);
	if (!ok_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(ok_button, BR_STRING_YES);
	elm_object_part_content_set(m_clear_confirm_popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __clear_location_confirm_response_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_clear_confirm_popup);
	elm_object_text_set(cancel_button, BR_STRING_NO);
	elm_object_part_content_set(m_clear_confirm_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __cancel_clear_location_confirm_response_cb, this);

	evas_object_show(m_clear_confirm_popup);

	return EINA_TRUE;
}

Eina_Bool Browser_Settings_Website_Setting::_show_clear_web_storage_confirm_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_clear_confirm_popup)
		evas_object_del(m_clear_confirm_popup);

	m_clear_confirm_popup = elm_popup_add(m_details_genlist);
	if (!m_clear_confirm_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}

	evas_object_size_hint_weight_set(m_clear_confirm_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_text_set(m_clear_confirm_popup, BR_STRING_DELETE_ALL_STORED_DATA_BY_THIS_WEBSITE);

	Evas_Object *ok_button = elm_button_add(m_clear_confirm_popup);
	if (!ok_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(ok_button, BR_STRING_YES);
	elm_object_part_content_set(m_clear_confirm_popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __clear_web_storage_confirm_response_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_clear_confirm_popup);
	elm_object_text_set(cancel_button, BR_STRING_NO);
	elm_object_part_content_set(m_clear_confirm_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __cancel_clear_web_storage_confirm_response_cb, this);

	evas_object_show(m_clear_confirm_popup);

	return EINA_TRUE;
}

Eina_Bool Browser_Settings_Website_Setting::_show_delete_all_website_list_confirm_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_delete_all_website_list_confirm_popup)
		evas_object_del(m_delete_all_website_list_confirm_popup);

	m_delete_all_website_list_confirm_popup = elm_popup_add(m_genlist);
	if (!m_delete_all_website_list_confirm_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}

	evas_object_size_hint_weight_set(m_delete_all_website_list_confirm_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_text_set(m_delete_all_website_list_confirm_popup, BR_STRING_DELETE_ALL_WEBSITE_DATA_AND_LOCATION_PERMISSIONS);

	Evas_Object *ok_button = elm_button_add(m_delete_all_website_list_confirm_popup);
	if (!ok_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(ok_button, BR_STRING_YES);
	elm_object_part_content_set(m_delete_all_website_list_confirm_popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __delete_all_website_list_confirm_response_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_delete_all_website_list_confirm_popup);
	elm_object_text_set(cancel_button, BR_STRING_NO);
	elm_object_part_content_set(m_delete_all_website_list_confirm_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __cancel_delete_all_website_list_confirm_response_cb, this);

	evas_object_show(m_delete_all_website_list_confirm_popup);

	return EINA_TRUE;
}

Eina_Bool Browser_Settings_Website_Setting::_show_delete_all_details_list_confirm_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_delete_all_details_list_confirm_popup)
		evas_object_del(m_delete_all_details_list_confirm_popup);

	m_delete_all_details_list_confirm_popup = elm_popup_add(m_details_genlist);
	if (!m_delete_all_details_list_confirm_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}

	evas_object_size_hint_weight_set(m_delete_all_details_list_confirm_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_text_set(m_delete_all_details_list_confirm_popup, BR_STRING_DELETE_ALL_WEBSITE_DATA_AND_LOCATION_PERMISSIONS);

	Evas_Object *ok_button = elm_button_add(m_delete_all_details_list_confirm_popup);
	if (!ok_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(ok_button, BR_STRING_YES);
	elm_object_part_content_set(m_delete_all_details_list_confirm_popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __delete_all_details_list_confirm_response_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_delete_all_details_list_confirm_popup);
	elm_object_text_set(cancel_button, BR_STRING_NO);
	elm_object_part_content_set(m_delete_all_details_list_confirm_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __cancel_delete_all_details_list_confirm_response_cb, this);

	evas_object_show(m_delete_all_details_list_confirm_popup);

	return EINA_TRUE;
}

void Browser_Settings_Website_Setting::__clear_location_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Website_Setting *website_setting = (Browser_Settings_Website_Setting *)data;
	if (website_setting->m_clear_confirm_popup) {
		evas_object_del(website_setting->m_clear_confirm_popup);
		website_setting->m_clear_confirm_popup = NULL;
	}

	Browser_Geolocation_DB geolocation_db;

	geolocation_db.remove_geolocation_data(website_setting->m_current_item_data->url.c_str());
	website_setting->m_current_item_data->geolocation = false;

	if(website_setting->m_current_item_data->storage == false) {
		website_setting->__back_button_clicked_cb(NULL, NULL, NULL);
		website_setting->_refresh_website_list();
	} else {
		website_setting->_refresh_details_list(website_setting->m_current_item_data);
	}

	website_setting->show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);
}

void Browser_Settings_Website_Setting::__cancel_clear_location_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Website_Setting *website_setting = (Browser_Settings_Website_Setting *)data;
	if (website_setting->m_clear_confirm_popup) {
		evas_object_del(website_setting->m_clear_confirm_popup);
		website_setting->m_clear_confirm_popup = NULL;
	}
}

void Browser_Settings_Website_Setting::__clear_web_storage_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Website_Setting *website_setting = (Browser_Settings_Website_Setting *)data;
	if (website_setting->m_clear_confirm_popup) {
		evas_object_del(website_setting->m_clear_confirm_popup);
		website_setting->m_clear_confirm_popup = NULL;
	}

	/* Clear cache */
	Ewk_Context *ewk_context = ewk_context_default_get();
	ewk_context_application_cache_delete(ewk_context, (Ewk_Security_Origin*)(website_setting->m_current_item_data->origin));
	ewk_context_web_database_delete(ewk_context, (Ewk_Security_Origin*)(website_setting->m_current_item_data->origin));
	ewk_context_web_storage_origin_delete(ewk_context, (Ewk_Security_Origin*)(website_setting->m_current_item_data->origin));
	website_setting->m_current_item_data->storage = false;

	if(website_setting->m_current_item_data->geolocation == false) {
		website_setting->__back_button_clicked_cb(NULL, NULL, NULL);
		website_setting->_refresh_website_list();
	} else
		website_setting->_refresh_details_list(website_setting->m_current_item_data);

	website_setting->show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);
}

void Browser_Settings_Website_Setting::__cancel_clear_web_storage_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Website_Setting *website_setting = (Browser_Settings_Website_Setting *)data;
	if (website_setting->m_clear_confirm_popup) {
		evas_object_del(website_setting->m_clear_confirm_popup);
		website_setting->m_clear_confirm_popup = NULL;
	}
}

void Browser_Settings_Website_Setting::__delete_all_website_list_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Website_Setting *website_setting = (Browser_Settings_Website_Setting *)data;
	if (website_setting->m_delete_all_website_list_confirm_popup) {
		evas_object_del(website_setting->m_delete_all_website_list_confirm_popup);
		website_setting->m_delete_all_website_list_confirm_popup = NULL;
	}

	/* Clear Geolocation */
	Browser_Geolocation_DB geolocation_db;
	geolocation_db.remove_all_geolocation_data();
	/* Clear cache */
	Ewk_Context *ewk_context = ewk_context_default_get();
	ewk_context_cache_clear(ewk_context);
	ewk_context_web_indexed_database_delete_all(ewk_context);
	ewk_context_application_cache_delete_all(ewk_context);
	ewk_context_web_storage_delete_all(ewk_context);
	ewk_context_web_database_delete_all(ewk_context);

	website_setting->_refresh_website_list();

	website_setting->show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);
}

void Browser_Settings_Website_Setting::__cancel_delete_all_website_list_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Website_Setting *website_setting = (Browser_Settings_Website_Setting *)data;
	if (website_setting->m_delete_all_website_list_confirm_popup) {
		evas_object_del(website_setting->m_delete_all_website_list_confirm_popup);
		website_setting->m_delete_all_website_list_confirm_popup = NULL;
	}
}

void Browser_Settings_Website_Setting::__delete_all_details_list_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Website_Setting *website_setting = (Browser_Settings_Website_Setting *)data;
	if (website_setting->m_delete_all_details_list_confirm_popup) {
		evas_object_del(website_setting->m_delete_all_details_list_confirm_popup);
		website_setting->m_delete_all_details_list_confirm_popup = NULL;
	}

	website_setting_item *info = website_setting->m_current_website_item_data;
	if (info->geolocation) {
		Browser_Geolocation_DB geolocation_db;
		geolocation_db.remove_geolocation_data(info->url.c_str());
		info->geolocation = false;
	}

	if (info->storage) {
		/* Clear cache */
		Ewk_Context *ewk_context = ewk_context_default_get();
		ewk_context_application_cache_delete(ewk_context,
			(Ewk_Security_Origin*)(info->origin));
		ewk_context_web_database_delete(ewk_context,
			(Ewk_Security_Origin*)(info->origin));
		ewk_context_web_storage_origin_delete(ewk_context,
			(Ewk_Security_Origin*)(info->origin));
		info->storage = false;
	}

	website_setting->__back_button_clicked_cb(NULL, NULL, NULL);
	website_setting->_refresh_website_list();

	website_setting->show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);
}

void Browser_Settings_Website_Setting::__cancel_delete_all_details_list_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Website_Setting *website_setting = (Browser_Settings_Website_Setting *)data;
	if (website_setting->m_delete_all_details_list_confirm_popup) {
		evas_object_del(website_setting->m_delete_all_details_list_confirm_popup);
		website_setting->m_delete_all_details_list_confirm_popup = NULL;
	}
}
void Browser_Settings_Website_Setting::_show_details_list(website_setting_item *item_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if(!item_info)
		return;

	m_details_genlist = elm_genlist_add(m_navi_bar);
	if (!m_details_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return;
	}

	evas_object_show(m_details_genlist);

	m_1_text_1_icon_item_class.item_style = "1text.1icon";
	m_1_text_1_icon_item_class.func.text_get = __details_genlist_label_get;
	m_1_text_1_icon_item_class.func.content_get = __details_genlist_icon_get;
	m_1_text_1_icon_item_class.func.state_get = NULL;
	m_1_text_1_icon_item_class.func.del = NULL;

	if (item_info->geolocation) {
		m_location_item_callback_data.type = BR_MENU_LOCATION;
		m_location_item_callback_data.cp = this;
		m_location_item_callback_data.user_data = item_info;
		m_location_item_callback_data.it = elm_genlist_item_append(m_details_genlist,
			&m_1_text_1_icon_item_class, &m_location_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
			__details_item_selected_cb, &m_location_item_callback_data);
	}

	if (item_info->storage) {
		m_web_storage_item_callback_data.type = BR_MENU_WEB_STORAGE;
		m_web_storage_item_callback_data.cp = this;
		m_web_storage_item_callback_data.user_data = item_info;
		m_web_storage_item_callback_data.it = elm_genlist_item_append(m_details_genlist,
				&m_1_text_1_icon_item_class, &m_web_storage_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
				__details_item_selected_cb, &m_web_storage_item_callback_data);
	}

	m_back_button_details = elm_button_add(m_details_genlist);
	if (!m_back_button_details) {
		BROWSER_LOGE("elm_button_add failed");
		return;
	}
	elm_object_style_set(m_back_button_details, "browser/bookmark_controlbar_back");
	evas_object_show(m_back_button_details);
	evas_object_smart_callback_add(m_back_button_details, "clicked", __back_button_clicked_cb, this);

	Elm_Object_Item *navi_it = elm_naviframe_item_push(m_navi_bar, item_info->url.c_str(),
							m_back_button_details, NULL, m_details_genlist, "browser_titlebar");

	m_details_bottom_control_bar = elm_toolbar_add(m_navi_bar);
	if (!m_details_bottom_control_bar) {
		BROWSER_LOGE("elm_toolbar_add failed");
		return;
	}
	elm_object_style_set(m_details_bottom_control_bar, "browser/default");
	elm_toolbar_shrink_mode_set(m_details_bottom_control_bar, ELM_TOOLBAR_SHRINK_EXPAND);

	m_delete_all_details_conbar_item = elm_toolbar_item_append(m_details_bottom_control_bar,
							NULL, BR_STRING_DELETE_ALL,
							__delete_all_details_list_conbar_item_clicked_cb, this);
	if (elm_genlist_items_count(m_details_genlist) == 0) {
		elm_object_item_disabled_set(m_delete_all_details_conbar_item, EINA_TRUE);
	}

	Elm_Object_Item *empty_item = elm_toolbar_item_append(m_details_bottom_control_bar, NULL, NULL, NULL, NULL);
	elm_object_item_disabled_set(empty_item, EINA_TRUE);

	empty_item = elm_toolbar_item_append(m_details_bottom_control_bar, NULL, NULL, NULL, NULL);
	elm_object_item_disabled_set(empty_item, EINA_TRUE);

	evas_object_show(m_details_bottom_control_bar);
	elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_CONTROLBAR, m_details_bottom_control_bar);
}

void Browser_Settings_Website_Setting::_refresh_details_list(website_setting_item *item_info)
{
	elm_genlist_clear(m_details_genlist);

	if (item_info->geolocation) {
		m_location_item_callback_data.type = BR_MENU_LOCATION;
		m_location_item_callback_data.cp = this;
		m_location_item_callback_data.user_data = item_info;
		m_location_item_callback_data.it = elm_genlist_item_append(m_details_genlist,
			&m_1_text_1_icon_item_class, &m_location_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
			__details_item_selected_cb, &m_location_item_callback_data);
	}

	if (item_info->storage) {
		m_web_storage_item_callback_data.type = BR_MENU_WEB_STORAGE;
		m_web_storage_item_callback_data.cp = this;
		m_web_storage_item_callback_data.user_data = item_info;
		m_web_storage_item_callback_data.it = elm_genlist_item_append(m_details_genlist,
				&m_1_text_1_icon_item_class, &m_web_storage_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
				__details_item_selected_cb, &m_web_storage_item_callback_data);
	}
}

void Browser_Settings_Website_Setting::__back_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (elm_naviframe_bottom_item_get(m_navi_bar)
	    != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);
}

void Browser_Settings_Website_Setting::__delete_all_website_list_conbar_item_clicked_cb(void *data,
								Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Settings_Website_Setting *website_setting = (Browser_Settings_Website_Setting *)data;

	website_setting->_show_delete_all_website_list_confirm_popup();
}

void Browser_Settings_Website_Setting::__delete_all_details_list_conbar_item_clicked_cb(void *data,
								Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Settings_Website_Setting *website_setting = (Browser_Settings_Website_Setting *)data;

	website_setting->_show_delete_all_details_list_confirm_popup();
}

void Browser_Settings_Website_Setting::__application_cache_origin_get_cb(Eina_List* origins, void* user_data)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Settings_Website_Setting *website_setting = (Browser_Settings_Website_Setting *)user_data;

	Eina_List *list = NULL;
	void *list_data = NULL;

	EINA_LIST_FOREACH(origins, list, list_data) {
		BROWSER_LOGD("list_data=%d", list_data);
		if (list_data) {
			Ewk_Security_Origin *origin = (Ewk_Security_Origin *)list_data;;
			BROWSER_LOGD("<<< host = [%s]", ewk_security_origin_host_get(origin));

			Eina_Bool duplicated = EINA_FALSE;
			for (int i = 0 ; i < website_setting->m_website_setting_list.size() ; i++) {
				if (!website_setting->m_website_setting_list[i]->url.empty()
				    && !strcmp(website_setting->m_website_setting_list[i]->url.c_str(), ewk_security_origin_host_get(origin))) {
					duplicated = EINA_TRUE;
					website_setting->m_website_setting_list[i]->storage = EINA_TRUE;
					website_setting->m_website_setting_list[i]->origin = origin;

					elm_genlist_item_update((Elm_Object_Item *)(website_setting->m_website_setting_list[i]->user_data));
					break;
				}
			}

			if (!duplicated) {
				website_setting_item *item = new(nothrow) website_setting_item;
				item->allow = EINA_FALSE;
				item->geolocation = EINA_FALSE;
				item->storage = EINA_TRUE;
				item->url = std::string(ewk_security_origin_host_get(origin));
				item->origin = origin;

				item->user_data = (void*)elm_genlist_item_append(website_setting->m_genlist, &website_setting->m_1_text_2_icon_item_class,
										item, NULL, ELM_GENLIST_ITEM_NONE,
										__item_selected_cb, website_setting);
				website_setting->m_website_setting_list.push_back(item);
			}
		}
	}

	/* enable the deleteall button at every origin callback
		because these callbacks are invoked asyncronously*/
	if (website_setting->m_website_setting_list.size() > 0) {
		if (website_setting->m_delete_all_website_list_conbar_item != NULL)
			elm_object_item_disabled_set(website_setting->m_delete_all_website_list_conbar_item, EINA_FALSE);
	}

	website_setting->m_is_called_appcache_cb_flag = true;
	if ((website_setting->m_is_called_appcache_cb_flag == true)
	    && (website_setting->m_is_called_webstorage_cb_flag == true)
	    && (website_setting->m_is_called_webdb_cb_flag == true)) {
		if (website_setting->m_website_setting_list.size() == 0) {
			website_setting->__back_button_clicked_cb(NULL, NULL, NULL);
		}
	}
}

void Browser_Settings_Website_Setting::__web_storage_origin_get_cb(Eina_List* origins, void* user_data)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Settings_Website_Setting *website_setting = (Browser_Settings_Website_Setting *)user_data;

	Eina_List *list = NULL;
	void *list_data = NULL;

	EINA_LIST_FOREACH(origins, list, list_data) {
		BROWSER_LOGD("list_data=%d", list_data);
		if (list_data) {
			Ewk_Security_Origin *origin = (Ewk_Security_Origin *)list_data;;
			BROWSER_LOGD("<<< host = [%s]", ewk_security_origin_host_get(origin));

			Eina_Bool duplicated = EINA_FALSE;
			for (int i = 0 ; i < website_setting->m_website_setting_list.size() ; i++) {
				if (!website_setting->m_website_setting_list[i]->url.empty()
				    && !strcmp(website_setting->m_website_setting_list[i]->url.c_str(), ewk_security_origin_host_get(origin))) {
					duplicated = EINA_TRUE;
					website_setting->m_website_setting_list[i]->storage = EINA_TRUE;
					website_setting->m_website_setting_list[i]->origin = origin;

					elm_genlist_item_update((Elm_Object_Item *)(website_setting->m_website_setting_list[i]->user_data));
					break;
				}
			}

			if (!duplicated) {
				website_setting_item *item = new(nothrow) website_setting_item;
				item->allow = EINA_FALSE;
				item->geolocation = EINA_FALSE;
				item->storage = EINA_TRUE;
				item->url = std::string(ewk_security_origin_host_get(origin));
				item->origin = origin;

				item->user_data = (void*)elm_genlist_item_append(website_setting->m_genlist, &website_setting->m_1_text_2_icon_item_class,
										item, NULL, ELM_GENLIST_ITEM_NONE,
										__item_selected_cb, website_setting);
				website_setting->m_website_setting_list.push_back(item);
			}
		}
	}

	/* enable the deleteall button at every origin callback
		because these callbacks are invoked asyncronously*/
	if (website_setting->m_website_setting_list.size() > 0) {
		if (website_setting->m_delete_all_website_list_conbar_item != NULL)
			elm_object_item_disabled_set(website_setting->m_delete_all_website_list_conbar_item, EINA_FALSE);
	}

	website_setting->m_is_called_webstorage_cb_flag = true;
	if ((website_setting->m_is_called_appcache_cb_flag == true)
	    && (website_setting->m_is_called_webstorage_cb_flag == true)
	    && (website_setting->m_is_called_webdb_cb_flag == true)) {
		if (website_setting->m_website_setting_list.size() == 0) {
			website_setting->__back_button_clicked_cb(NULL, NULL, NULL);
		}
	}
}

void Browser_Settings_Website_Setting::__web_database_origin_get_cb(Eina_List* origins, void* user_data)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Settings_Website_Setting *website_setting = (Browser_Settings_Website_Setting *)user_data;

	Eina_List *list = NULL;
	void *list_data = NULL;

	EINA_LIST_FOREACH(origins, list, list_data) {
		BROWSER_LOGD("list_data=%d", list_data);
		if (list_data) {
			Ewk_Security_Origin *origin = (Ewk_Security_Origin *)list_data;
			BROWSER_LOGD("<<< host = [%s]", ewk_security_origin_host_get(origin));

			Eina_Bool duplicated = EINA_FALSE;
			for (int i = 0 ; i < website_setting->m_website_setting_list.size() ; i++) {
				if (!website_setting->m_website_setting_list[i]->url.empty()
				    && !strcmp(website_setting->m_website_setting_list[i]->url.c_str(), ewk_security_origin_host_get(origin))) {
					duplicated = EINA_TRUE;
					website_setting->m_website_setting_list[i]->storage = EINA_TRUE;
					website_setting->m_website_setting_list[i]->origin = origin;

					elm_genlist_item_update((Elm_Object_Item *)(website_setting->m_website_setting_list[i]->user_data));
					break;
				}
			}

			if (!duplicated) {
				website_setting_item *item = new(nothrow) website_setting_item;
				item->allow = EINA_FALSE;
				item->geolocation = EINA_FALSE;
				item->storage = EINA_TRUE;
				item->url = std::string(ewk_security_origin_host_get(origin));
				item->origin = origin;

				item->user_data = (void*)elm_genlist_item_append(website_setting->m_genlist, &website_setting->m_1_text_2_icon_item_class,
										item, NULL, ELM_GENLIST_ITEM_NONE,
										__item_selected_cb, website_setting);
				website_setting->m_website_setting_list.push_back(item);

			}
		}
	}

	/* enable the deleteall button at every origin callback
		because these callbacks are invoked asyncronously*/
	if (website_setting->m_website_setting_list.size() > 0) {
		if (website_setting->m_delete_all_website_list_conbar_item != NULL)
			elm_object_item_disabled_set(website_setting->m_delete_all_website_list_conbar_item, EINA_FALSE);
	}

	website_setting->m_is_called_webdb_cb_flag = true;
	if ((website_setting->m_is_called_appcache_cb_flag == true)
	    && (website_setting->m_is_called_webstorage_cb_flag == true)
	    && (website_setting->m_is_called_webdb_cb_flag == true)) {
		if (website_setting->m_website_setting_list.size() == 0) {
			website_setting->__back_button_clicked_cb(NULL, NULL, NULL);
		}
	}
}

void Browser_Settings_Website_Setting::_website_list_get(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_is_called_appcache_cb_flag = false;
	m_is_called_webstorage_cb_flag = false;
	m_is_called_webdb_cb_flag = false;
	Ewk_Context *context = ewk_context_default_get();
	ewk_context_application_cache_origins_get(context, __application_cache_origin_get_cb, this);
	ewk_context_web_storage_origins_get(context, __web_storage_origin_get_cb, this);
	ewk_context_web_database_origins_get(context, __web_database_origin_get_cb, this);

	Browser_Geolocation_DB geolocation_db;

	geolocation_db.get_geolocation_info_list(m_geolocation_list);

	BROWSER_LOGD("geolocation_list size=%d", m_geolocation_list.size());

	for (int i = 0 ; i < m_geolocation_list.size() ; i++) {
		m_geolocation_list[i]->user_data = (void* )elm_genlist_item_append(m_genlist, &m_1_text_2_icon_item_class,
								m_geolocation_list[i], NULL, ELM_GENLIST_ITEM_NONE,
								__item_selected_cb, this);

		m_website_setting_list.push_back((website_setting_item *)(m_geolocation_list[i]));
	}
}

void Browser_Settings_Website_Setting::_refresh_website_list(void)
{
	BROWSER_LOGD("[%s]", __func__);

	for (int i = 0 ; i < m_website_setting_list.size() ; i++) {
		if (m_website_setting_list[i])
			delete m_website_setting_list[i];
	}
	m_website_setting_list.clear();
	m_geolocation_list.clear();

	elm_genlist_clear(m_genlist);

	_website_list_get();

	if (m_website_setting_list.size() == 0) {
		if (m_delete_all_website_list_conbar_item != NULL)
			elm_object_item_disabled_set(m_delete_all_website_list_conbar_item, EINA_TRUE);
	}
}

Eina_Bool Browser_Settings_Website_Setting::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_genlist = elm_genlist_add(m_navi_bar);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return EINA_FALSE;
	}

	evas_object_show(m_genlist);

	m_back_button = elm_button_add(m_genlist);
	if (!m_back_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_back_button, "browser/bookmark_controlbar_back");
	evas_object_show(m_back_button);
	evas_object_smart_callback_add(m_back_button, "clicked", __back_button_clicked_cb, this);

	m_1_text_2_icon_item_class.item_style = "1text.3icon.2";
	m_1_text_2_icon_item_class.func.text_get = __genlist_label_get;
	m_1_text_2_icon_item_class.func.content_get = __genlist_icon_get;
	m_1_text_2_icon_item_class.func.state_get = NULL;
	m_1_text_2_icon_item_class.func.del = NULL;

	_website_list_get();

	Elm_Object_Item *navi_it = elm_naviframe_item_push(m_navi_bar, BR_STRING_WEBSITE_SETTINGS,
							m_back_button, NULL, m_genlist, "browser_titlebar");

	m_bottom_control_bar = elm_toolbar_add(m_navi_bar);
	if (!m_bottom_control_bar) {
		BROWSER_LOGE("elm_toolbar_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_bottom_control_bar, "browser/default");
	elm_toolbar_shrink_mode_set(m_bottom_control_bar, ELM_TOOLBAR_SHRINK_EXPAND);

	m_delete_all_website_list_conbar_item = elm_toolbar_item_append(m_bottom_control_bar,
							NULL, BR_STRING_DELETE_ALL,
							__delete_all_website_list_conbar_item_clicked_cb, this);

	if (!m_delete_all_website_list_conbar_item) {
		BROWSER_LOGE("elm_toolbar_item_append failed");
		return EINA_FALSE;
	}

	if (m_website_setting_list.size() == 0) {
		elm_object_item_disabled_set(m_delete_all_website_list_conbar_item, EINA_TRUE);
	}

	Elm_Object_Item *empty_item = elm_toolbar_item_append(m_bottom_control_bar, NULL, NULL, NULL, NULL);
	elm_object_item_disabled_set(empty_item, EINA_TRUE);

	empty_item = elm_toolbar_item_append(m_bottom_control_bar, NULL, NULL, NULL, NULL);
	elm_object_item_disabled_set(empty_item, EINA_TRUE);

	evas_object_show(m_bottom_control_bar);
	elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_CONTROLBAR, m_bottom_control_bar);

	return EINA_TRUE;
}

