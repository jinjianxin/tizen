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


#include "browser-class.h"
#include "browser-geolocation.h"
#include "browser-geolocation-db.h"
#include "browser-view.h"

Browser_Geolocation::Browser_Geolocation(void)
:	m_popup(NULL)
	,m_check(NULL)
	,m_permission_request(0)
	,m_is_location_manager_valid(EINA_FALSE)
	,m_ewk_view(NULL)
{
	m_geolocation_db = get_data_manager()->get_geolocation_db();
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Geolocation::~Browser_Geolocation(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_popup)
		evas_object_del(m_popup);
}

Eina_Bool Browser_Geolocation::init(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!ewk_view)
		return EINA_FALSE;

	m_ewk_view = ewk_view;

	return EINA_TRUE;
}

void Browser_Geolocation::__popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Geolocation *geolocation = (Browser_Geolocation *)data;
	Eina_Bool state = elm_check_state_get(geolocation->m_check);
	if (state) {
		//store save state in the database
		geolocation->m_geolocation_db->save_geolocation_host(geolocation->m_host_string.c_str(), 1);
	}
	if (geolocation->m_popup) {
		evas_object_del(geolocation->m_popup);
		geolocation->m_popup = NULL;
	}

	ewk_geolocation_permission_request_allow_set(geolocation->m_permission_request, EINA_TRUE);
}

void Browser_Geolocation::__popup_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Geolocation *geolocation = (Browser_Geolocation *)data;
	Eina_Bool state = elm_check_state_get(geolocation->m_check);
	if (state) {
		//store save state in the database
		geolocation->m_geolocation_db->save_geolocation_host(geolocation->m_host_string.c_str(), 0);
	}
	if (geolocation->m_popup) {
		evas_object_del(geolocation->m_popup);
		geolocation->m_popup = NULL;
	}

	ewk_geolocation_permission_request_allow_set(geolocation->m_permission_request, EINA_FALSE);
}

Eina_Bool Browser_Geolocation::_show_request_confirm_popup(std::string msg)
{
	Evas_Object *layout = NULL;
	Evas_Object *label = NULL;

	if (m_popup)
		evas_object_del(m_popup);

	m_popup = elm_popup_add(m_navi_bar);
	if (!m_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}

	label = elm_label_add(m_popup);
	if (!label) {
		BROWSER_LOGE("elm_label_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, msg.c_str());
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(label);

	layout = elm_layout_add(m_popup);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}
	if (!elm_layout_file_set(layout, BROWSER_EDJE_DIR"/browser-popup.edj",
				"geolocation_popup")) {
		BROWSER_LOGE("elm_layout_file_set failed", BROWSER_EDJE_DIR);
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_check = elm_check_add(m_popup);
	if (!m_check) {
		BROWSER_LOGE("elm_check_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_align_set(m_check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(m_check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(m_check);

	elm_object_part_text_set(layout, "elm.text", BR_STRING_REMEMBER_PREFERENCE);
	elm_object_part_content_set(layout, "elm.swallow.content", label);
	elm_object_part_content_set(layout, "elm.swallow.end", m_check);

	evas_object_show(layout);
	elm_object_content_set(m_popup, layout);

	Evas_Object *ok_button = elm_button_add(m_popup);
	if (!ok_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(ok_button, BR_STRING_ALLOW);
	elm_object_part_content_set(m_popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __popup_response_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_popup);
	if (!cancel_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(cancel_button, BR_STRING_REFUSE);
	elm_object_part_content_set(m_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __popup_cancel_cb, this);

	evas_object_show(m_popup);

	return EINA_TRUE;
}

void Browser_Geolocation::__geolocation_permission_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data || !event_info)
		return;

	Ewk_Geolocation_Permission_Data *permission_data = (Ewk_Geolocation_Permission_Data *)event_info;

	Browser_View *browser_view = (Browser_View *)data;

	Browser_Geolocation *geolocation = m_browser->get_geolocation();

	const Ewk_Security_Origin *origin = ewk_geolocation_permission_request_origin_get(permission_data);
	geolocation->m_host_string = ewk_security_origin_host_get(origin);
	BROWSER_LOGD("Host string[%s]", geolocation->m_host_string.c_str());
	char *request_msg = NULL;
	int host_string_len = (geolocation->m_host_string).length();
	int popup_string_len = strlen(BR_STRING_REQUEST_LOCATION) + 2;// margin of %s

	request_msg = (char *)malloc((host_string_len + popup_string_len) * sizeof(char));
	memset(request_msg, 0, (host_string_len + popup_string_len));
	snprintf(request_msg, (host_string_len + popup_string_len), BR_STRING_REQUEST_LOCATION, (geolocation->m_host_string).c_str());

	geolocation->m_permission_request = ewk_geolocation_permission_request_get(permission_data);

	bool enable_location = true;
	br_preference_get_bool(ENABLE_LOCATION_KEY, &enable_location);

	if (enable_location) {
		int count;
		bool accept;
		count = geolocation->m_geolocation_db->get_geolocation_host_count(geolocation->m_host_string.c_str());
		if (count > 0) {
			BROWSER_LOGD("geolocation info is existed");
			// get the geolocation data and proceed with that and return without showing popup.
			geolocation->m_geolocation_db->get_geolocation_host(geolocation->m_host_string.c_str(), accept);
			if (accept)
				ewk_geolocation_permission_request_allow_set(geolocation->m_permission_request, EINA_TRUE);
			else
				ewk_geolocation_permission_request_allow_set(geolocation->m_permission_request, EINA_FALSE);

			if (request_msg)
				free(request_msg);

			return;
		}
		if (!geolocation->_show_request_confirm_popup(request_msg))
			BROWSER_LOGE("_show_request_confirm_popup failed");
	} else
		ewk_geolocation_permission_request_allow_set(geolocation->m_permission_request, EINA_FALSE);

	if (request_msg)
		free(request_msg);

}
