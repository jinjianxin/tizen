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
  *@file most-visited-sites-view.cpp
  *@brief 最常访问view实现
  */


#include "add-to-most-visited-sites-view.h"
#include "browser-data-manager.h"

Add_To_Most_Visited_Sites_View::Add_To_Most_Visited_Sites_View(Most_Visited_Sites *most_visited_sites)
:	m_conformant(NULL)
	,m_content_box(NULL)
	,m_url_edit_field(NULL)
	,m_history_genlist(NULL)
	,m_cancel_button(NULL)
	,m_save_button(NULL)
	,m_most_visited_sites(most_visited_sites)
{
	BROWSER_LOGD("[%s]", __func__);
}

Add_To_Most_Visited_Sites_View::~Add_To_Most_Visited_Sites_View(void)
{
	BROWSER_LOGD("[%s]", __func__);

	for(int i = 0 ; i < m_history_list.size() ; i++ ) {
		if (m_history_list[i])
			delete m_history_list[i];
	}

	for(int i = 0 ; i < m_history_date_label_list.size() ; i++ ) {
		if (m_history_date_label_list[i])
			delete m_history_date_label_list[i];
	}
	m_history_date_label_list.clear();
}

Eina_Bool Add_To_Most_Visited_Sites_View::init(void)
{
	BROWSER_LOGD("[%s]", __func__);
	return _create_main_layout();
}

Evas_Object *Add_To_Most_Visited_Sites_View::__genlist_icon_get_cb(void *data, Evas_Object *obj,
										const char *part)
{
	if (!data || !part)
		return NULL;

	Browser_History_DB::history_item *item = (Browser_History_DB::history_item *)data;

	if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
		Evas_Object *default_favicon = elm_icon_add(obj);
		if (!default_favicon) {
			BROWSER_LOGE("elm_icon_add is failed");
			return NULL;
		}
		elm_icon_file_set(default_favicon, BROWSER_IMAGE_DIR"/faviconDefault.png", NULL);
		evas_object_size_hint_aspect_set(default_favicon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return default_favicon;
	}
}

char *Add_To_Most_Visited_Sites_View::__genlist_label_get_cb(void *data, Evas_Object *obj,
									const char *part)
{
	if (!data)
		return NULL;

	Browser_History_DB::history_item *item = (Browser_History_DB::history_item *)data;
	if (!item)
		return NULL;

	char *mark_up = NULL;

	BROWSER_LOGD("item->title.c_str()=[%s]", item->title.c_str());
	if (!strncmp(part, "elm.text.1", strlen("elm.text.1"))
		|| !strncmp(part, "elm.base.text", strlen("elm.base.text"))
		|| !strncmp(part, "elm.slide.text.1", strlen("elm.slide.text.1")))
	{
		mark_up = elm_entry_utf8_to_markup(item->title.c_str());
		return mark_up;
	} else if (!strncmp(part, "elm.text.2", strlen("elm.text.2"))
		|| !strncmp(part, "elm.slide.text.2", strlen("elm.slide.text.2"))) {
		mark_up = elm_entry_utf8_to_markup(item->url.c_str());
		return mark_up;
	}

	return NULL;
}

char *Add_To_Most_Visited_Sites_View::__genlist_date_label_get_cb(void *data, Evas_Object *obj,
										const char *part)
{
	if (!data)
		return NULL;

	char *date_label = (char *)data;
	if (!strncmp(part, "elm.text", strlen("elm.text"))) {
		Date date;
		char buffer[BROWSER_MAX_DATE_LEN] = {0, };

		sscanf(date_label, "%d-%d-%d",
			&date.year, &date.month, &date.day);
		BROWSER_LOGD("%d %d %d\n", date.day, date.month, date.year);
		time_t raw_time;
		struct tm *time_info;
		time(&raw_time);
		time_info = localtime(&raw_time);
		int day_gap = -1;
		BROWSER_LOGD("\ncurrent time : %d %d %d\n", time_info->tm_mday, (time_info->tm_mon+1),
									(time_info->tm_year+1900));
		if (time_info->tm_year == date.year -1900 && time_info->tm_mon == date.month - 1)
			day_gap = time_info->tm_mday - date.day;
		else {
			time_info->tm_year = date.year - 1900;
			time_info->tm_mon = date.month - 1;
		}
		time_info->tm_mday = date.day;
		mktime(time_info);

		if (day_gap == 0)
			strftime(buffer, BROWSER_MAX_DATE_LEN, "Today - %a, %b %d, %Y", time_info);
		else if (day_gap == 1)
			strftime(buffer, 40, "Yesterday - %a, %b %d, %Y", time_info);
		else
			strftime(buffer, 40, "%a, %b %d, %Y", time_info);
		return strdup(buffer);
	}
	return NULL;
}

void Add_To_Most_Visited_Sites_View::__history_item_clicked_cb(void *data, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Add_To_Most_Visited_Sites_View *add_to_most_visited_sites_view = NULL;
	add_to_most_visited_sites_view = (Add_To_Most_Visited_Sites_View *)data;

	Elm_Object_Item *seleted_item = elm_genlist_selected_item_get(obj);
	Elm_Object_Item *index_item = elm_genlist_first_item_get(obj);
	int index = 0;

	/* calculate the index of the selected item */
	while (index_item != seleted_item) {
		/* Do not count date label item */
		if (elm_genlist_item_select_mode_get(index_item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
			index++;

		index_item = elm_genlist_item_next_get(index_item);
	}

	Evas_Object *entry = br_elm_editfield_entry_get(add_to_most_visited_sites_view->m_url_edit_field);
	char *mark_up = elm_entry_utf8_to_markup(add_to_most_visited_sites_view->m_history_list[index]->url.c_str());
	if (mark_up) {
		elm_entry_entry_set(entry, mark_up);
		free(mark_up);
	}
}

Evas_Object *Add_To_Most_Visited_Sites_View::_create_history_genlist(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *genlist = elm_genlist_add(m_conformant);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	memset(&m_history_genlist_item_class, 0x00, sizeof(Elm_Genlist_Item_Class));
	m_history_genlist_item_class.item_style = "2text.1icon.4";
	m_history_genlist_item_class.func.text_get = __genlist_label_get_cb;
	m_history_genlist_item_class.func.content_get = __genlist_icon_get_cb;
	m_history_genlist_item_class.func.state_get = NULL;
	m_history_genlist_item_class.func.del = NULL;

	memset(&m_history_group_title_class, 0x00, sizeof(Elm_Genlist_Item_Class));
	m_history_group_title_class.item_style = "grouptitle";
	m_history_group_title_class.func.text_get = __genlist_date_label_get_cb;
	m_history_group_title_class.func.content_get = NULL;
	m_history_group_title_class.func.state_get = NULL;
	m_history_group_title_class.func.del = NULL;

	m_history_list.clear();
	m_history_date_label_list.clear();

	Eina_Bool ret = m_data_manager->get_history_db()->get_history_list(m_history_list);
	if (!ret) {
		BROWSER_LOGE("get_history_list failed");
		return NULL;
	}

	unsigned int history_count = m_history_list.size();

	Date date;
	Elm_Object_Item *it = NULL;
	for(int i = 0 ; i < history_count; i++) {
		sscanf(m_history_list[i]->date.c_str(), "%d-%d-%d", &date.year, &date.month, &date.day);
		m_history_list[i]->user_data = (void *)this;
		if (m_last_date != date) {
			m_last_date = date;
			char *labe_item = strdup(m_history_list[i]->date.c_str());
			m_history_date_label_list.push_back(labe_item);
			it = elm_genlist_item_append(genlist, &m_history_group_title_class,
						labe_item, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);

			elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		}
		elm_genlist_item_append(genlist, &m_history_genlist_item_class,
					m_history_list[i], it, ELM_GENLIST_ITEM_NONE,
					__history_item_clicked_cb, this);
	}

	return genlist;
}

void Add_To_Most_Visited_Sites_View::__cancel_button_clicked_cb(void *data, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Add_To_Most_Visited_Sites_View *add_to_most_visited_sites_view = NULL;
	add_to_most_visited_sites_view = (Add_To_Most_Visited_Sites_View *)data;

	if (elm_naviframe_bottom_item_get(m_navi_bar) != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);
}

Eina_Bool Add_To_Most_Visited_Sites_View::_save_button_clicked(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *entry = br_elm_editfield_entry_get(m_url_edit_field);
	char* url = _trim(elm_entry_markup_to_utf8(elm_entry_entry_get(entry)));
	if (!url || !strlen(url)) {
		show_msg_popup(BR_STRING_EMPTY);
		return EINA_FALSE;
	}

	std::string full_url = std::string(url);

	if (_has_url_sheme(url)) {
		char *token = strstr(url, BROWSER_URL_SCHEME_CHECK);
		if (token && strlen(token) == 3) {
			/* If input url has only sheme such as 'http://', 'https://' */
			show_msg_popup(BR_STRING_EMPTY);
			return EINA_FALSE;
		}
	} else
		full_url = std::string(BROWSER_HTTP_SCHEME) + std::string(url);

	if (m_most_visited_sites->m_selected_item->url && strlen(m_most_visited_sites->m_selected_item->url)) {
		if (!strcmp(full_url.c_str(), m_most_visited_sites->m_selected_item->url)) {
			/* If the url is not changed in case of modify speed dial item. */
			if (elm_naviframe_bottom_item_get(m_navi_bar)
			    != elm_naviframe_top_item_get(m_navi_bar))
				elm_naviframe_item_pop(m_navi_bar);
			return EINA_FALSE;
		}
	}

	std::string history_id;
	history_id = m_most_visited_sites->m_most_visited_sites_db->get_history_id_by_url(full_url.c_str());

	char *screen_shot_path = NULL;
	if (history_id.empty())
		screen_shot_path = strdup("default_noscreenshot");
	else
		screen_shot_path = strdup(history_id.c_str());

	if (!screen_shot_path) {
		BROWSER_LOGE("strdup failed");
		return EINA_FALSE;
	}

	int index = 0;
	Elm_Object_Item *it = elm_gengrid_first_item_get(m_most_visited_sites->m_gengrid);
	do {
		if (it == m_most_visited_sites->m_selected_item->item)
			break;
		index++;
	} while (it = elm_gengrid_item_next_get(it));

	if (!m_most_visited_sites->m_most_visited_sites_db->save_most_visited_sites_item(index, full_url.c_str(),
						full_url.c_str(), screen_shot_path)) {
		BROWSER_LOGE("save_most_visited_sites_item failed");
		return EINA_FALSE;
	}

	free(screen_shot_path);

	if (!m_most_visited_sites->_reload_items()) {
		BROWSER_LOGE("m_most_visited_sites->_reload_items failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

void Add_To_Most_Visited_Sites_View::__save_button_clicked_cb(void *data, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Add_To_Most_Visited_Sites_View *add_to_most_visited_sites_view = NULL;
	add_to_most_visited_sites_view = (Add_To_Most_Visited_Sites_View *)data;

	if (!add_to_most_visited_sites_view->_save_button_clicked()) {
		BROWSER_LOGE("_save_button_clicked failed");
		return;
	}

	if (elm_naviframe_bottom_item_get(m_navi_bar) != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);
}

Eina_Bool Add_To_Most_Visited_Sites_View::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	elm_win_conformant_set(m_win, EINA_TRUE);

	m_conformant = elm_conformant_add(m_navi_bar);
	if (!m_conformant) {
		BROWSER_LOGE("elm_conformant_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_conformant, "internal_layout");
	evas_object_size_hint_weight_set(m_conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_conformant, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_conformant);

	m_content_box = elm_box_add(m_conformant);
	if (!m_content_box) {
		BROWSER_LOGE("elm_box_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_align_set(m_content_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(m_content_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(m_content_box);

	elm_object_content_set(m_conformant, m_content_box);

	m_url_edit_field = br_elm_editfield_add(m_conformant, EINA_TRUE);
	if (!m_url_edit_field) {
		BROWSER_LOGE("elm_editfield_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_align_set(m_url_edit_field, EVAS_HINT_FILL, EVAS_HINT_FILL);
	br_elm_editfield_entry_single_line_set(m_url_edit_field, EINA_TRUE);
	br_elm_editfield_label_set(m_url_edit_field, BR_STRING_URL);
	evas_object_show(m_url_edit_field);

	Evas_Object *entry = br_elm_editfield_entry_get(m_url_edit_field);
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_URL);

	if (m_most_visited_sites->m_selected_item->url && strlen(m_most_visited_sites->m_selected_item->url))
		/* If modify selected speed dial item. */
		elm_entry_entry_set(entry, m_most_visited_sites->m_selected_item->url);
	else
		/* If add empty speed dial item. */
		elm_entry_entry_set(entry, BROWSER_HTTP_SCHEME);

	/* To show ime.
	  * __set_focus_editfield_idler_cb is defined in browser_common_view class. */
	ecore_idler_add(__set_focus_editfield_idler_cb, m_url_edit_field);

	elm_box_pack_end(m_content_box, m_url_edit_field);

	m_history_genlist = _create_history_genlist();
	if (!m_history_genlist) {
		BROWSER_LOGE("_create_history_genlist failed");
		return EINA_FALSE;
	}
	elm_box_pack_end(m_content_box, m_history_genlist);
	evas_object_show(m_history_genlist);

	Elm_Object_Item *navi_it = elm_naviframe_item_push(m_navi_bar, BR_STRING_ADD_TO_MOST_VISITED_SITES,
						NULL, NULL, m_conformant, "browser_titlebar");
	elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_PREV_BTN, NULL);

	m_cancel_button = elm_button_add(m_conformant);
	if (!m_cancel_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_cancel_button, "browser/title_button");
	elm_object_text_set(m_cancel_button, BR_STRING_CANCEL);
	evas_object_show(m_cancel_button);
	evas_object_smart_callback_add(m_cancel_button, "clicked", __cancel_button_clicked_cb, this);
	elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_TITLE_LEFT_BTN, m_cancel_button);

	m_save_button = elm_button_add(m_conformant);
	if (!m_save_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_save_button, "browser/title_button");
	elm_object_text_set(m_save_button, BR_STRING_SAVE);
	evas_object_show(m_save_button);
	evas_object_smart_callback_add(m_save_button, "clicked", __save_button_clicked_cb, this);
	elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_TITLE_RIGHT_BTN, m_save_button);

	return EINA_TRUE;
}

