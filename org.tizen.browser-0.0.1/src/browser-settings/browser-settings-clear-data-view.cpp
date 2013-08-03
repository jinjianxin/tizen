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

#include "browser-data-manager.h"
#include "browser-history-db.h"
#include "browser-settings-clear-data-view.h"
#include "browser-view.h"

Browser_Settings_Clear_Data_View::Browser_Settings_Clear_Data_View(Browser_Settings_Main_View *main_view)
:
	m_main_view(main_view)
	,m_genlist(NULL)
	,m_select_all_check_box(NULL)
	,m_history_check_box(NULL)
	,m_cache_check_box(NULL)
	,m_cookie_check_box(NULL)
	,m_saved_id_password_check_box(NULL)
	,m_select_all_layout(NULL)
	,m_content_box(NULL)
	,m_back_button(NULL)
	,m_bottom_control_bar(NULL)
	,m_delete_controlbar_item(NULL)
	,m_cancel_controlbar_item(NULL)
	,m_delete_confirm_popup(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Settings_Clear_Data_View::~Browser_Settings_Clear_Data_View(void)
{
	BROWSER_LOGD("[%s]", __func__);

	hide_notify_popup();
}

Eina_Bool Browser_Settings_Clear_Data_View::init(void)
{
	return _create_main_layout();
}

void Browser_Settings_Clear_Data_View::__cancel_button_clicked_cb(void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (elm_naviframe_bottom_item_get(m_navi_bar) != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);
}

void Browser_Settings_Clear_Data_View::_delete_private_data(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (elm_check_state_get(m_history_check_box)) {
		/* Clear history */
		if (!m_data_manager->get_history_db()->clear_history())
			BROWSER_LOGE("get_history_db()->clear_history() failed");
	}
	if (elm_check_state_get(m_cache_check_box)) {
		/* Clear cache */
		Ewk_Context *ewk_context = ewk_context_default_get();
		ewk_context_cache_clear(ewk_context);
		ewk_context_web_indexed_database_delete_all(ewk_context);
		ewk_context_application_cache_delete_all(ewk_context);
		ewk_context_web_storage_delete_all(ewk_context);
		ewk_context_web_database_delete_all(ewk_context);
	}
	if (elm_check_state_get(m_cookie_check_box)) {
		/* Clear cookie */
		Ewk_Context *ewk_context = ewk_context_default_get();
		ewk_context_cookies_clear(ewk_context);
	}
	if (elm_check_state_get(m_saved_id_password_check_box)) {
		/* Clear saved id & password */
	}
}

void Browser_Settings_Clear_Data_View::__delete_confirm_response_cb(void *data, Evas_Object *obj,
								void *event_info)
{
	BROWSER_LOGD("event_info = %d", (int)event_info);

	if (!data)
		return;

	Browser_Settings_Clear_Data_View *clear_data_view = (Browser_Settings_Clear_Data_View *)data;

	evas_object_del(clear_data_view->m_delete_confirm_popup);

		clear_data_view->_delete_private_data();
		clear_data_view->show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);

		if (elm_naviframe_bottom_item_get(clear_data_view->m_navi_bar)
		    != elm_naviframe_top_item_get(clear_data_view->m_navi_bar))
			elm_naviframe_item_pop(clear_data_view->m_navi_bar);
	}

void Browser_Settings_Clear_Data_View::__cancel_confirm_response_cb(void *data, Evas_Object *obj,
								void *event_info)
{
	BROWSER_LOGD("event_info = %d", (int)event_info);

	if (!data)
		return;

	Browser_Settings_Clear_Data_View *clear_data_view = (Browser_Settings_Clear_Data_View *)data;

	evas_object_del(clear_data_view->m_delete_confirm_popup);
}

Evas_Object *Browser_Settings_Clear_Data_View::_show_delete_confirm_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_delete_confirm_popup = elm_popup_add(m_content_box);
	if (!m_delete_confirm_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return NULL;
	}

	evas_object_size_hint_weight_set(m_delete_confirm_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(m_delete_confirm_popup, BR_STRING_DELETE_Q);

	Evas_Object *ok_button = elm_button_add(m_delete_confirm_popup);
	if (!ok_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_text_set(ok_button, BR_STRING_YES);
	elm_object_part_content_set(m_delete_confirm_popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __delete_confirm_response_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_delete_confirm_popup);
	elm_object_text_set(cancel_button, BR_STRING_NO);
	elm_object_part_content_set(m_delete_confirm_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __cancel_confirm_response_cb, this);

	return m_delete_confirm_popup;
}

void Browser_Settings_Clear_Data_View::__delete_button_clicked_cb(void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	Browser_Settings_Clear_Data_View *clear_data_view = NULL;
	clear_data_view = (Browser_Settings_Clear_Data_View *)data;

	Evas_Object *popup = clear_data_view->_show_delete_confirm_popup();

	evas_object_show(popup);
}

char *Browser_Settings_Clear_Data_View::__genlist_label_get(void *data,
					Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Clear_Data_View *clear_data_view = NULL;
	clear_data_view = (Browser_Settings_Clear_Data_View *)callback_data->user_data;
	menu_type type = callback_data->type;

	if (type == BR_CLEAR_HISTORY) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_HISTORY);
	} else if (type == BR_CLEAR_CACHE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_CLEAR_CACHE);
	} else if (type == BR_CLEAR_COOKIE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_COOKIES);
	} else if (type == BR_CLEAR_SAVED_PASSWORD) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_CLEAR_PASSWORDS);
	}

	return NULL;
}

void Browser_Settings_Clear_Data_View::__item_check_changed_cb(void* data, Evas_Object* obj,
										void* event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Clear_Data_View *clear_data_view = NULL;
	clear_data_view = (Browser_Settings_Clear_Data_View *)data;

	int selected_item_count = elm_check_state_get(clear_data_view->m_history_check_box)
				 + elm_check_state_get(clear_data_view->m_cache_check_box)
				 + elm_check_state_get(clear_data_view->m_cookie_check_box)
				 + elm_check_state_get(clear_data_view->m_saved_id_password_check_box);

	clear_data_view->hide_notify_popup();

	if (selected_item_count) {
		if (selected_item_count == 4)
			elm_check_state_set(clear_data_view->m_select_all_check_box, EINA_TRUE);
		else
			elm_check_state_set(clear_data_view->m_select_all_check_box, EINA_FALSE);

		if (selected_item_count == 1) {
			clear_data_view->show_notify_popup(BR_STRING_ONE_ITEM_SELECTED, 0, EINA_TRUE);
		} else {
			char *small_popup_text = NULL;
			int string_len = strlen(BR_STRING_ITEMS_SELECTED) + 4; /* 4 : reserved for maximun count */

			small_popup_text = (char *)malloc(string_len * sizeof(char));
			memset(small_popup_text, 0x00, string_len);

			snprintf(small_popup_text, string_len, BR_STRING_ITEMS_SELECTED, selected_item_count);
			clear_data_view->show_notify_popup(small_popup_text, 0, EINA_TRUE);

			if (small_popup_text)
				free(small_popup_text);
			small_popup_text = 0x00;
		}
	} else
		elm_check_state_set(clear_data_view->m_select_all_check_box, EINA_FALSE);

	if (elm_check_state_get(clear_data_view->m_history_check_box)
	    || elm_check_state_get(clear_data_view->m_cache_check_box)
	    || elm_check_state_get(clear_data_view->m_cookie_check_box)
	    || elm_check_state_get(clear_data_view->m_saved_id_password_check_box))
		elm_object_item_disabled_set(clear_data_view->m_delete_controlbar_item, EINA_FALSE);
	else
		elm_object_item_disabled_set(clear_data_view->m_delete_controlbar_item, EINA_TRUE);
}

void Browser_Settings_Clear_Data_View::__select_all_changed_cb(void* data, Evas_Object* obj,
										void* event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Clear_Data_View *clear_data_view = NULL;
	clear_data_view = (Browser_Settings_Clear_Data_View *)data;

	Eina_Bool check_value = elm_check_state_get(obj);
	elm_check_state_set(clear_data_view->m_history_check_box, check_value);
	elm_check_state_set(clear_data_view->m_cache_check_box, check_value);
	elm_check_state_set(clear_data_view->m_cookie_check_box, check_value);
	elm_check_state_set(clear_data_view->m_saved_id_password_check_box, check_value);

	clear_data_view->hide_notify_popup();

	if (check_value) {
		char *small_popup_text = NULL;
		int string_len = strlen(BR_STRING_ITEMS_SELECTED) + 4; /* 4 : reserved for maximun count */

		small_popup_text = (char *)malloc(string_len * sizeof(char));
		memset(small_popup_text, 0x00, string_len);

		snprintf(small_popup_text, string_len, BR_STRING_ITEMS_SELECTED, 4);
		clear_data_view->show_notify_popup(small_popup_text, 0, EINA_TRUE);

		if (small_popup_text)
			free(small_popup_text);
		small_popup_text = 0x00;
		elm_object_item_disabled_set(clear_data_view->m_delete_controlbar_item, EINA_FALSE);
	} else
		elm_object_item_disabled_set(clear_data_view->m_delete_controlbar_item, EINA_TRUE);
}

Evas_Object *Browser_Settings_Clear_Data_View::__genlist_icon_get(void *data,
						Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Clear_Data_View *clear_data_view = NULL;	
	clear_data_view = (Browser_Settings_Clear_Data_View *)callback_data->user_data;
	menu_type type = callback_data->type;

	if (type == BR_CLEAR_HISTORY) {
		if(!strncmp(part, "elm.edit.icon.1", strlen("elm.edit.icon.1"))) {
			clear_data_view->m_history_check_box = elm_check_add(obj);
			if (clear_data_view->m_history_check_box) {
				evas_object_smart_callback_add(clear_data_view->m_history_check_box,
						"changed", __item_check_changed_cb, clear_data_view);
				evas_object_propagate_events_set(clear_data_view->m_history_check_box, EINA_FALSE);
			}
			return clear_data_view->m_history_check_box;
		}
	} else if (type == BR_CLEAR_CACHE) {
		if(!strncmp(part, "elm.edit.icon.1", strlen("elm.edit.icon.1"))) {
			clear_data_view->m_cache_check_box = elm_check_add(obj);
			if (clear_data_view->m_cache_check_box) {
				evas_object_smart_callback_add(clear_data_view->m_cache_check_box,
						"changed", __item_check_changed_cb, clear_data_view);
				evas_object_propagate_events_set(clear_data_view->m_cache_check_box, EINA_FALSE);
			}
			return clear_data_view->m_cache_check_box;
		}
	} else if (type == BR_CLEAR_COOKIE) {
		if(!strncmp(part, "elm.edit.icon.1", strlen("elm.edit.icon.1"))) {
			clear_data_view->m_cookie_check_box = elm_check_add(obj);
			if (clear_data_view->m_cookie_check_box) {
				evas_object_smart_callback_add(clear_data_view->m_cookie_check_box,
						"changed", __item_check_changed_cb, clear_data_view);
				evas_object_propagate_events_set(clear_data_view->m_cookie_check_box, EINA_FALSE);
			}
			return clear_data_view->m_cookie_check_box;
		}
	} else if (type == BR_CLEAR_SAVED_PASSWORD) {
		if(!strncmp(part, "elm.edit.icon.1", strlen("elm.edit.icon.1"))) {
			clear_data_view->m_saved_id_password_check_box = elm_check_add(obj);
			if (clear_data_view->m_saved_id_password_check_box) {
				evas_object_smart_callback_add(clear_data_view->m_saved_id_password_check_box,
						"changed", __item_check_changed_cb, clear_data_view);
				evas_object_propagate_events_set(clear_data_view->m_saved_id_password_check_box, EINA_FALSE);
			}
			return clear_data_view->m_saved_id_password_check_box;
		}
	}

	return NULL;
}

void Browser_Settings_Clear_Data_View::__select_all_layout_mouse_down_cb(void *data, Evas *evas,
								Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Clear_Data_View *clear_data_view = (Browser_Settings_Clear_Data_View *)data;

	elm_check_state_set(clear_data_view->m_select_all_check_box,
		!elm_check_state_get(clear_data_view->m_select_all_check_box));
	__select_all_changed_cb(clear_data_view, clear_data_view->m_select_all_check_box, NULL);
}

void Browser_Settings_Clear_Data_View::__genlist_item_selected_cb(void *data, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Clear_Data_View *clear_data_view = NULL;
	clear_data_view = (Browser_Settings_Clear_Data_View *)callback_data->user_data;
	menu_type type = callback_data->type;

	if (type == BR_CLEAR_HISTORY)
		elm_check_state_set(clear_data_view->m_history_check_box,
			!elm_check_state_get(clear_data_view->m_history_check_box));
	 else if (type == BR_CLEAR_CACHE)
		elm_check_state_set(clear_data_view->m_cache_check_box,
			!elm_check_state_get(clear_data_view->m_cache_check_box));
	 else if (type == BR_CLEAR_COOKIE)
		elm_check_state_set(clear_data_view->m_cookie_check_box,
			!elm_check_state_get(clear_data_view->m_cookie_check_box));
	 else if (type == BR_CLEAR_SAVED_PASSWORD)
		elm_check_state_set(clear_data_view->m_saved_id_password_check_box,
			!elm_check_state_get(clear_data_view->m_saved_id_password_check_box));

	__item_check_changed_cb(clear_data_view, NULL, NULL);

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

Evas_Object *Browser_Settings_Clear_Data_View::_create_genlist(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *genlist = elm_genlist_add(m_content_box);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	m_1text_item_class.item_style = "1text";
	m_1text_item_class.decorate_all_item_style = "edit_default";
	m_1text_item_class.func.del = NULL;
	m_1text_item_class.func.state_get = NULL;
	m_1text_item_class.func.text_get = __genlist_label_get;
	m_1text_item_class.func.content_get = __genlist_icon_get;

	m_clear_history_item_callback_data.type = BR_CLEAR_HISTORY;
	m_clear_history_item_callback_data.user_data = this;
	m_clear_history_item_callback_data.it = elm_genlist_item_append(genlist, &m_1text_item_class,
						&m_clear_history_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__genlist_item_selected_cb, &m_clear_history_item_callback_data);

	m_clear_cache_item_callback_data.type = BR_CLEAR_CACHE;
	m_clear_cache_item_callback_data.user_data = this;
	m_clear_cache_item_callback_data.it = elm_genlist_item_append(genlist, &m_1text_item_class,
						&m_clear_cache_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__genlist_item_selected_cb, &m_clear_cache_item_callback_data);

	m_clear_cookie_item_callback_data.type = BR_CLEAR_COOKIE;
	m_clear_cookie_item_callback_data.user_data = this;
	m_clear_cookie_item_callback_data.it = elm_genlist_item_append(genlist, &m_1text_item_class,
						&m_clear_cookie_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__genlist_item_selected_cb, &m_clear_cookie_item_callback_data);

	m_clear_saved_id_password_item_callback_data.type = BR_CLEAR_SAVED_PASSWORD;
	m_clear_saved_id_password_item_callback_data.user_data = this;
	m_clear_saved_id_password_item_callback_data.it = elm_genlist_item_append(genlist, &m_1text_item_class,
						&m_clear_saved_id_password_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__genlist_item_selected_cb, &m_clear_saved_id_password_item_callback_data);

	return genlist;
}

Eina_Bool Browser_Settings_Clear_Data_View::_create_main_layout(void)
{
	m_content_box = elm_box_add(m_navi_bar);
	if (!m_content_box) {
		BROWSER_LOGE("elm_box_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_content_box, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(m_content_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_content_box);

	m_genlist = _create_genlist();
	if (!m_genlist) {
		BROWSER_LOGE("_create_genlist failed");
		return EINA_FALSE;
	}
	evas_object_show(m_genlist);

	m_select_all_layout = elm_layout_add(m_content_box);
	if (!m_select_all_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}
	elm_layout_theme_set(m_select_all_layout, "genlist", "item", "select_all/default");
	evas_object_size_hint_weight_set(m_select_all_layout, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(m_select_all_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_event_callback_add(m_select_all_layout, EVAS_CALLBACK_MOUSE_DOWN,
							__select_all_layout_mouse_down_cb, this);
	evas_object_show(m_select_all_layout);

	m_select_all_check_box = elm_check_add(m_select_all_layout);
	if (!m_select_all_check_box) {
		BROWSER_LOGE("elm_check_add failed");
		return EINA_FALSE;
	}
	evas_object_smart_callback_add(m_select_all_check_box,
			"changed", __select_all_changed_cb, this);
	evas_object_propagate_events_set(m_select_all_check_box, EINA_FALSE);
	evas_object_show(m_select_all_check_box);

	elm_object_part_content_set(m_select_all_layout, "elm.icon", m_select_all_check_box);
	elm_object_text_set(m_select_all_layout, BR_STRING_SELECT_ALL);

	elm_box_pack_end(m_content_box, m_select_all_layout);
	elm_box_pack_end(m_content_box, m_genlist);

	elm_genlist_decorate_mode_set(m_genlist, EINA_TRUE);

	Elm_Object_Item *navi_it = elm_naviframe_item_push(m_navi_bar, BR_STRING_DELETE_BROWSING_DATA,
								NULL, NULL, m_content_box, "browser_titlebar");
	elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_PREV_BTN, NULL);

	m_bottom_control_bar = elm_toolbar_add(m_content_box);
	if (!m_bottom_control_bar) {
		BROWSER_LOGE("elm_toolbar_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_bottom_control_bar, "browser/default");
	elm_toolbar_shrink_mode_set(m_bottom_control_bar, ELM_TOOLBAR_SHRINK_EXPAND);

	m_delete_controlbar_item = elm_toolbar_item_append(m_bottom_control_bar,
							NULL, BR_STRING_DELETE,
							__delete_button_clicked_cb, this);
	elm_object_item_disabled_set(m_delete_controlbar_item, EINA_TRUE);

	Elm_Object_Item *empty_item = elm_toolbar_item_append(m_bottom_control_bar, NULL, NULL, NULL, NULL);
	elm_object_item_disabled_set(empty_item, EINA_TRUE);

	empty_item = elm_toolbar_item_append(m_bottom_control_bar, NULL, NULL, NULL, NULL);
	elm_object_item_disabled_set(empty_item, EINA_TRUE);

	m_cancel_controlbar_item = elm_toolbar_item_append(m_bottom_control_bar,
							NULL, BR_STRING_CANCEL,
							__cancel_button_clicked_cb, this);

	evas_object_show(m_bottom_control_bar);
	elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_CONTROLBAR, m_bottom_control_bar);

	return EINA_TRUE;
}

