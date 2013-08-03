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


#include "browser-add-to-bookmark-view.h"
#include "browser-bookmark-db.h"
#include "browser-new-folder-view.h"
#include "browser-select-folder-view.h"

Browser_Select_Folder_View::Browser_Select_Folder_View(int current_folder_id)
:
	m_list(NULL)
	,m_conformant(NULL)
	,m_bottom_control_bar(NULL)
	,m_content_layout(NULL)
	,m_current_folder_id(current_folder_id)
	,m_main_folder_item(NULL)
	,m_cancel_button(NULL)
	,m_done_button(NULL)
	,m_navi_it(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Select_Folder_View::~Browser_Select_Folder_View(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_main_folder_item)
		delete m_main_folder_item;

	for(int i = 0 ; i < m_folder_list.size() ; i++ ) {
		if (m_folder_list[i])
			delete m_folder_list[i];
	}
	m_folder_list.clear();

	evas_object_smart_callback_del(m_navi_bar, "transition,finished", __naviframe_pop_finished_cb);
}

Eina_Bool Browser_Select_Folder_View::init(void)
{
	BROWSER_LOGD("[%s]", __func__);

	return _create_main_layout();
}

void Browser_Select_Folder_View::return_to_select_folder_view(void)
{
	BROWSER_LOGD("[%s]", __func__);

	_fill_folder_list();
}

void Browser_Select_Folder_View::__folder_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	Elm_Object_Item *it = (Elm_Object_Item *)elm_list_selected_item_get(obj);
	if (!it || !data)
		return;

	Browser_Bookmark_DB::bookmark_item *item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
	Evas_Object *group_radio_button = (Evas_Object *)data;
	Browser_Select_Folder_View *select_folder_view = (Browser_Select_Folder_View *)(item->user_data_1);
	int index = item->user_data_3;
	elm_radio_value_set(group_radio_button, index);

	select_folder_view->m_current_folder_id = item->id;

	elm_list_item_selected_set(it, EINA_FALSE);

	if (elm_naviframe_bottom_item_get(select_folder_view->m_navi_bar)
	    != elm_naviframe_top_item_get(select_folder_view->m_navi_bar))
		elm_naviframe_item_pop(select_folder_view->m_navi_bar);

	if (m_data_manager->get_edit_bookmark_view())
		m_data_manager->get_edit_bookmark_view()->return_to_add_to_bookmark_view(select_folder_view->m_current_folder_id);
	else if (m_data_manager->get_add_to_bookmark_view())
		m_data_manager->get_add_to_bookmark_view()->return_to_add_to_bookmark_view(select_folder_view->m_current_folder_id);
}

void Browser_Select_Folder_View::_fill_folder_list(void)
{
	BROWSER_LOGD("[%s]", __func__);
	for(int i = 0 ; i < m_folder_list.size() ; i++ ) {
		if (m_folder_list[i])
			delete m_folder_list[i];
	}

	m_folder_list.clear();
	elm_list_clear(m_list);

	Evas_Object *radio_group_button = elm_radio_add(m_list);
	if (!radio_group_button) {
		BROWSER_LOGE("elm_list_add failed");
		return;
	}

	m_main_folder_item = new(nothrow) Browser_Bookmark_DB::bookmark_item;
	if (!m_main_folder_item) {
		BROWSER_LOGE("new Browser_Bookmark_DB::bookmark_item failed");
		return;
	}

	int index = 0;
	elm_radio_state_value_set(radio_group_button, EINA_FALSE);

	m_main_folder_item->id = BROWSER_BOOKMARK_MAIN_FOLDER_ID;
	m_main_folder_item->title = std::string(BR_STRING_BOOKMARKS);
	m_main_folder_item->user_data_1 = this;
	m_main_folder_item->user_data_3 = index;
	elm_list_item_append(m_list, BR_STRING_BOOKMARKS, radio_group_button, NULL, NULL, (void *)m_main_folder_item);

	elm_radio_value_set(radio_group_button, index);

	if (m_data_manager->get_bookmark_db()->get_folder_list(m_folder_list)) {
		BROWSER_LOGE("m_folder_list.size()=%d", m_folder_list.size());
		for (int i = 0 ; i < m_folder_list.size() ; i++) {
			Evas_Object *radio_button = elm_radio_add(m_list);
			if (!radio_button) {
				BROWSER_LOGE("elm_radio_add failed");
				return;
			}
			m_folder_list[i]->user_data_1 = this;

			/* The default main bookmark folder's index is already counted. So increase the index by 1. */
			elm_radio_state_value_set(radio_button, i + 1);
			m_folder_list[i]->user_data_3 = i + 1;

			elm_list_item_append(m_list, m_folder_list[i]->title.c_str(), radio_button,
						NULL, NULL, (void*)m_folder_list[i]);
			elm_radio_group_add(radio_button, radio_group_button);

			if (m_current_folder_id == m_folder_list[i]->id)
				elm_radio_value_set(radio_group_button, i + 1);
		}
	}

	evas_object_smart_callback_add(m_list, "selected", __folder_item_selected_cb, radio_group_button);
	elm_list_go(m_list);
}

void Browser_Select_Folder_View::__new_folder_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Select_Folder_View *select_folder_view = (Browser_Select_Folder_View *)data;

	if (!m_data_manager->create_new_folder_view()) {
		BROWSER_LOGE("create_new_folder_view failed");
		return;
	}

	if (!m_data_manager->get_new_folder_view()->init()) {
		BROWSER_LOGE("get_new_folder_view()->init() failed");
		m_data_manager->destroy_new_folder_view();
		return;
	}
}

void Browser_Select_Folder_View::__cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_Select_Folder_View *select_folder_view = (Browser_Select_Folder_View *)data;

	if (elm_naviframe_bottom_item_get(select_folder_view->m_navi_bar)
	    != elm_naviframe_top_item_get(select_folder_view->m_navi_bar))
		elm_naviframe_item_pop(select_folder_view->m_navi_bar);
}

void Browser_Select_Folder_View::__done_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!data)
		return;

	Browser_Select_Folder_View *select_folder_view = (Browser_Select_Folder_View *)data;
	int selected_folder_id = select_folder_view->m_current_folder_id;

	if (elm_naviframe_bottom_item_get(select_folder_view->m_navi_bar)
	    != elm_naviframe_top_item_get(select_folder_view->m_navi_bar))
		elm_naviframe_item_pop(select_folder_view->m_navi_bar);

	if (m_data_manager->get_edit_bookmark_view())
		m_data_manager->get_edit_bookmark_view()->return_to_add_to_bookmark_view(selected_folder_id);
	else if (m_data_manager->get_add_to_bookmark_view())
		m_data_manager->get_add_to_bookmark_view()->return_to_add_to_bookmark_view(selected_folder_id);
}

void Browser_Select_Folder_View::__naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Select_Folder_View *select_folder_view = (Browser_Select_Folder_View *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	if (select_folder_view->m_navi_it != elm_naviframe_top_item_get(m_navi_bar))
		return;

	m_data_manager->destroy_new_folder_view();
}

Eina_Bool Browser_Select_Folder_View::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_content_layout = elm_layout_add(m_navi_bar);
	if (!m_content_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}
	if (!elm_layout_file_set(m_content_layout, BROWSER_EDJE_DIR"/browser-bookmark-view.edj",
					"bookmark/bookmark-folder-view-layout")) {
		BROWSER_LOGE("elm_layout_file_set failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_content_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_list = elm_list_add(m_content_layout);
	if (!m_list) {
		BROWSER_LOGE("_create_folder_list failed");
		return EINA_FALSE;
	}
	elm_list_mode_set(m_list, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(m_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_list, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_list);
	elm_object_part_content_set(m_content_layout, "elm.swallow.folderlist", m_list);

	_fill_folder_list();

	m_bottom_control_bar = elm_toolbar_add(m_content_layout);
	if (!m_bottom_control_bar) {
		BROWSER_LOGE("elm_toolbar_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_bottom_control_bar, "browser/default");
	elm_toolbar_shrink_mode_set(m_bottom_control_bar, ELM_TOOLBAR_SHRINK_EXPAND);

	evas_object_show(m_bottom_control_bar);

	elm_toolbar_item_append(m_bottom_control_bar, NULL, BR_STRING_CREATE_FOLDER, __new_folder_clicked_cb, this);
	elm_object_part_content_set(m_content_layout, "elm.swallow.controlbar", m_bottom_control_bar);
	evas_object_show(m_content_layout);

	m_navi_it = elm_naviframe_item_push(m_navi_bar, BR_STRING_FOLDER, NULL, NULL, m_content_layout, "browser_titlebar");
	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_PREV_BTN, NULL);

	evas_object_smart_callback_add(m_navi_bar, "transition,finished", __naviframe_pop_finished_cb, this);

	m_cancel_button = elm_button_add(m_content_layout);
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

