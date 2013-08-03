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
#include "browser-settings-edit-homepage-view.h"

Browser_Settings_Edit_Homepage_View::Browser_Settings_Edit_Homepage_View(Browser_Settings_Main_View *main_view)
:
	m_main_view(main_view)
	,m_conformant(NULL)
	,m_genlist(NULL)
	,m_edit_field(NULL)
	,m_done_button(NULL)
	,m_cancel_button(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Settings_Edit_Homepage_View::~Browser_Settings_Edit_Homepage_View(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_conformant)
		evas_object_del(m_conformant);
	m_conformant = NULL;
}

void Browser_Settings_Edit_Homepage_View::__back_button_clicked_cb(void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (elm_naviframe_bottom_item_get(m_navi_bar)
	    != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);
}

void Browser_Settings_Edit_Homepage_View::__done_button_clicked_cb(void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Edit_Homepage_View *edit_homepage_view = NULL;
	edit_homepage_view = (Browser_Settings_Edit_Homepage_View *)data;

	Evas_Object *entry = br_elm_editfield_entry_get(edit_homepage_view->m_edit_field);
	char *homepage = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));
	if (homepage) {
		br_preference_set_str(USER_HOMEPAGE_KEY, homepage);
		br_preference_set_str(HOMEPAGE_KEY, USER_HOMEPAGE);
		free(homepage);
	}

	if (elm_naviframe_bottom_item_get(m_navi_bar)
	    != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);
}

void Browser_Settings_Edit_Homepage_View::__cancel_button_clicked_cb(void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (elm_naviframe_bottom_item_get(m_navi_bar)
	    != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);
}

Eina_Bool Browser_Settings_Edit_Homepage_View::init(void)
{
	BROWSER_LOGD("[%s]", __func__);
	return _create_main_layout();
}

void Browser_Settings_Edit_Homepage_View::__edit_field_changed_cb(void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_Settings_Edit_Homepage_View *edit_homepage_view = NULL;
	edit_homepage_view = (Browser_Settings_Edit_Homepage_View *)data;

	const char *url = elm_entry_entry_get(obj);
	if (!url || !strlen(url)) {
		elm_object_disabled_set(edit_homepage_view->m_done_button, EINA_TRUE);
		return;
	}

	Eina_Bool only_has_space = EINA_FALSE;
	int space_count = 0;
	for (int i = 0 ; i < strlen(url) ; i++) {
		if (url[i] == ' ')
			space_count++;
	}
	if (space_count == strlen(url))
		only_has_space = EINA_TRUE;

	if (only_has_space) {
		elm_object_disabled_set(edit_homepage_view->m_done_button, EINA_TRUE);
		return;
	}

	elm_object_disabled_set(edit_homepage_view->m_done_button, EINA_FALSE);
}

Eina_Bool Browser_Settings_Edit_Homepage_View::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	elm_win_conformant_set(m_win, EINA_TRUE);

	m_conformant = elm_conformant_add(m_win);
	if (!m_conformant) {
		BROWSER_LOGE("elm_conformant_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_conformant, "internal_layout");
	evas_object_size_hint_align_set(m_conformant, EVAS_HINT_FILL, EVAS_HINT_FILL);

	m_genlist = elm_genlist_add(m_conformant);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return EINA_FALSE;
	}

	m_item_class.item_style = "1icon";
	m_item_class.func.text_get = NULL;
	m_item_class.func.content_get = __genlist_icon_get_cb;
	m_item_class.func.state_get = NULL;
	m_item_class.func.del = NULL;

	elm_genlist_item_append(m_genlist, &m_item_class, this, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	evas_object_show(m_genlist);
	elm_object_content_set(m_conformant, m_genlist);

	m_navi_it = elm_naviframe_item_push(m_navi_bar, BR_STRING_HOMEPAGE, NULL, NULL,
											m_conformant, "browser_titlebar");

	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_PREV_BTN, NULL);

	m_done_button = elm_button_add(m_conformant);
	if (!m_done_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_done_button, "browser/title_button");
	elm_object_text_set(m_done_button, BR_STRING_DONE);
	evas_object_show(m_done_button);
	evas_object_smart_callback_add(m_done_button, "clicked", __done_button_clicked_cb, this);
	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_TITLE_RIGHT_BTN, m_done_button);

	m_cancel_button = elm_button_add(m_conformant);
	if (!m_cancel_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_cancel_button, "browser/title_button");
	elm_object_text_set(m_cancel_button, BR_STRING_CANCEL);
	evas_object_show(m_cancel_button);
	evas_object_smart_callback_add(m_cancel_button, "clicked", __cancel_button_clicked_cb, this);
	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_TITLE_LEFT_BTN, m_cancel_button);

	return EINA_TRUE;
}

Evas_Object *Browser_Settings_Edit_Homepage_View::__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	Browser_Settings_Edit_Homepage_View *edit_homepage_view = (Browser_Settings_Edit_Homepage_View *)data;

	if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
		edit_homepage_view->m_edit_field = br_elm_editfield_add(obj, EINA_TRUE);
		if (!edit_homepage_view->m_edit_field) {
			BROWSER_LOGE("elm_editfield_add failed");
			return NULL;
		}
		br_elm_editfield_entry_single_line_set(edit_homepage_view->m_edit_field, EINA_TRUE);
		br_elm_editfield_label_set(edit_homepage_view->m_edit_field, BR_STRING_URL);

		char *homepage = NULL;
		if (br_preference_get_str(USER_HOMEPAGE_KEY, &homepage) == false)
			BROWSER_LOGE("failed to get %s preference\n", USER_HOMEPAGE_KEY);

		elm_entry_entry_set(br_elm_editfield_entry_get(edit_homepage_view->m_edit_field), homepage);

		evas_object_smart_callback_add(br_elm_editfield_entry_get(edit_homepage_view->m_edit_field),
						"changed", __edit_field_changed_cb, edit_homepage_view);

		ecore_idler_add(edit_homepage_view->__set_focus_editfield_idler_cb, edit_homepage_view->m_edit_field);
		return edit_homepage_view->m_edit_field;
	}

	return NULL;
}


