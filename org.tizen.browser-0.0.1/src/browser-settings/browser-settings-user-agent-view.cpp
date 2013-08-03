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


#include "browser-settings-main-view.h"
#include "browser-settings-user-agent-view.h"

Browser_Settings_User_Agent_View::Browser_Settings_User_Agent_View(Browser_Settings_Main_View *main_view)
:	m_main_view(main_view)
	,m_back_button(NULL)
	,m_genlist(NULL)
	,m_tizen_checkbox(NULL)
	,m_chrome_checkbox(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Settings_User_Agent_View::~Browser_Settings_User_Agent_View(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Eina_Bool Browser_Settings_User_Agent_View::init(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!_create_main_layout()) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

void Browser_Settings_User_Agent_View::__back_button_clicked_cb(void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (elm_naviframe_bottom_item_get(m_navi_bar)
	    != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);
}

char *Browser_Settings_User_Agent_View::__genlist_label_get_cb(void *data,
						Evas_Object *obj, const char *part)
{
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_User_Agent_View::user_agent_type type = callback_data->type;

	if (!strncmp(part, "elm.text", strlen("elm.text"))) {
		if (type == TIZEN)
			return strdup("Tizen");
		else if (type == CHROME)
			return strdup("Chrome 20");
	}

	return NULL;
}

Evas_Object *Browser_Settings_User_Agent_View::__genlist_icon_get_cb(void *data,
							Evas_Object *obj, const char *part)
{
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_User_Agent_View *user_agent_view = NULL;
	user_agent_view = (Browser_Settings_User_Agent_View *)(callback_data->user_data);
	Browser_Settings_User_Agent_View::user_agent_type type = callback_data->type;

	if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
		if (type == TIZEN) {
			user_agent_view->m_tizen_checkbox = elm_check_add(obj);
			if (!user_agent_view->m_tizen_checkbox) {
				BROWSER_LOGE("elm_check_add failed");
				return NULL;
			}
			char *user_agent = vconf_get_str(USERAGENT_KEY);
			if (!user_agent || strncmp(user_agent, "Chrome 20", strlen("Chrome 20"))) {
				elm_check_state_set(user_agent_view->m_tizen_checkbox, EINA_TRUE);
				if (user_agent)
					free(user_agent);
			}

			evas_object_propagate_events_set(user_agent_view->m_tizen_checkbox, EINA_FALSE);
			evas_object_smart_callback_add(user_agent_view->m_tizen_checkbox, "changed",
									__check_changed_cb, callback_data);

			return user_agent_view->m_tizen_checkbox;
		} else if (type == CHROME) {
			user_agent_view->m_chrome_checkbox = elm_check_add(obj);
			if (!user_agent_view->m_chrome_checkbox) {
				BROWSER_LOGE("elm_check_add failed");
				return NULL;
			}
			char *user_agent = vconf_get_str(USERAGENT_KEY);
			if (user_agent) {
				if (!strncmp(user_agent, "Chrome 20", strlen("Chrome 20")))
					elm_check_state_set(user_agent_view->m_chrome_checkbox, EINA_TRUE);

				free(user_agent);
			}

			evas_object_propagate_events_set(user_agent_view->m_chrome_checkbox, EINA_FALSE);
			evas_object_smart_callback_add(user_agent_view->m_chrome_checkbox, "changed",
									__check_changed_cb, callback_data);

			return user_agent_view->m_chrome_checkbox;
		}
	}

	return NULL;
}

void Browser_Settings_User_Agent_View::__check_changed_cb( void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_User_Agent_View *user_agent_view = NULL;
	user_agent_view = (Browser_Settings_User_Agent_View *)(callback_data->user_data);
	Browser_Settings_User_Agent_View::user_agent_type type = callback_data->type;

	const char *key = USERAGENT_KEY;
	Eina_Bool state = elm_check_state_get(obj);
	if (state) {
		if (type == TIZEN) {
			elm_check_state_set(user_agent_view->m_tizen_checkbox, EINA_TRUE);
			elm_check_state_set(user_agent_view->m_chrome_checkbox, EINA_FALSE);
			if (vconf_set_str(USERAGENT_KEY, "Tizen") < 0)
				BROWSER_LOGE("vconf_set_str failed");
		} else {
			elm_check_state_set(user_agent_view->m_tizen_checkbox, EINA_FALSE);
			elm_check_state_set(user_agent_view->m_chrome_checkbox, EINA_TRUE);
			if (vconf_set_str(USERAGENT_KEY, "Chrome 20") < 0)
				BROWSER_LOGE("vconf_set_str failed");
		}
	} else {
		if (type == TIZEN) {
			if (!elm_check_state_get(user_agent_view->m_chrome_checkbox))
				elm_check_state_set(user_agent_view->m_tizen_checkbox, EINA_TRUE);
		} else {
			if (!elm_check_state_get(user_agent_view->m_tizen_checkbox))
				elm_check_state_set(user_agent_view->m_chrome_checkbox, EINA_TRUE);
		}
	}
}

void Browser_Settings_User_Agent_View::__item_selected_cb(void *data,
					Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_User_Agent_View *user_agent_view = NULL;
	user_agent_view = (Browser_Settings_User_Agent_View *)(callback_data->user_data);
	Browser_Settings_User_Agent_View::user_agent_type type = callback_data->type;

	if (type == TIZEN) {
		elm_check_state_set(user_agent_view->m_tizen_checkbox, EINA_TRUE);
		elm_check_state_set(user_agent_view->m_chrome_checkbox, EINA_FALSE);
		if (vconf_set_str(USERAGENT_KEY, "Tizen") < 0)
			BROWSER_LOGE("vconf_set_str failed");
	} else {
		elm_check_state_set(user_agent_view->m_tizen_checkbox, EINA_FALSE);
		elm_check_state_set(user_agent_view->m_chrome_checkbox, EINA_TRUE);
		if (vconf_set_str(USERAGENT_KEY, "Chrome 20") < 0)
			BROWSER_LOGE("vconf_set_str failed");
	}

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

Eina_Bool Browser_Settings_User_Agent_View::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_genlist = elm_genlist_add(m_navi_bar);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_genlist);

	m_1_text_1_icon_item_class.item_style = "1text.1icon.2";
	m_1_text_1_icon_item_class.func.text_get = __genlist_label_get_cb;
	m_1_text_1_icon_item_class.func.content_get = __genlist_icon_get_cb;
	m_1_text_1_icon_item_class.func.state_get = NULL;
	m_1_text_1_icon_item_class.func.del = NULL;

	m_tizen_item_callback_data.type = TIZEN;
	m_tizen_item_callback_data.user_data = this;
	m_tizen_item_callback_data.it = elm_genlist_item_append(m_genlist, &m_1_text_1_icon_item_class,
						&m_tizen_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__item_selected_cb, &m_tizen_item_callback_data);

	m_chrome_item_callback_data.type = CHROME;
	m_chrome_item_callback_data.user_data = this;
	m_chrome_item_callback_data.it = elm_genlist_item_append(m_genlist, &m_1_text_1_icon_item_class,
						&m_chrome_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__item_selected_cb, &m_chrome_item_callback_data);

	m_back_button = elm_button_add(m_genlist);
	if (!m_back_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_back_button, "browser/bookmark_controlbar_back");
	evas_object_show(m_back_button);
	evas_object_smart_callback_add(m_back_button, "clicked", __back_button_clicked_cb, this);

	Elm_Object_Item *navi_it = elm_naviframe_item_push(m_navi_bar, BR_STRING_USER_AGENT,
							m_back_button, NULL, m_genlist, "browser_titlebar");

	return EINA_TRUE;
}

