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
  *@file browser-history-layout.cpp
  *@brief 浏览器历史界面
  */

#include "browser-add-to-bookmark-view.h"
#include "browser-view.h"
#include "browser-history-layout.h"

Date::Date() : day(0), month(0), year(0) {}

Date::Date(Date &date)
{
	this->day = date.day;
	this->month = date.month;
	this->year = date.year;
}

void Date::operator=(Date &date)
{
	this->day = date.day;
	this->month = date.month;
	this->year = date.year;
}

bool Date::operator==(Date &date)
{
	return (this->day == date.day && this->month == date.month && this->year == date.year);
}

bool Date::operator!=(Date &date)
{
	return (this->day != date.day || this->month != date.month || this->year != date.year);
}

Browser_History_Layout::Browser_History_Layout(void)
:	m_history_genlist(NULL)
	,m_edit_mode_select_all_layout(NULL)
	,m_edit_mode_select_all_check_button(NULL)
	,m_content_box(NULL)
	,m_no_history_label(NULL)
	,m_current_sweep_item(NULL)
	,m_processing_popup_timer(NULL)
	,m_processed_it(NULL)
	,m_processed_count(0)
	,m_total_item_count(0)
	,m_processing_popup(NULL)
	,m_processing_popup_layout(NULL)
	,m_processing_progress_bar(NULL)
	,m_select_all_check_value(EINA_FALSE)
	,m_sub_main_history_layout(NULL)
	,m_searchbar_layout(NULL)
	,m_searched_history_genlist(NULL)
	,m_no_content_search_result(NULL)
	,m_searchbar(NULL)
	,m_delete_confirm_popup(NULL)
	,m_is_bookmark_on_off_icon_clicked(EINA_FALSE)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_History_Layout::~Browser_History_Layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	hide_notify_popup_layout(m_sub_main_history_layout);

	for(int i = 0 ; i < m_history_list.size() ; i++ ) {
		if (m_history_list[i])
			delete m_history_list[i];
	}
	m_history_list.clear();

	for(int i = 0 ; i < m_history_date_label_list.size() ; i++ ) {
		if (m_history_date_label_list[i])
			free(m_history_date_label_list[i]);
	}
	m_history_date_label_list.clear();

	for(int i = 0 ; i < m_searched_history_date_label_list.size() ; i++ ) {
		if (m_searched_history_date_label_list[i])
			free(m_searched_history_date_label_list[i]);
	}
	m_searched_history_date_label_list.clear();

	for(int i = 0 ; i < m_searched_history_item_list.size() ; i++ ) {
		if (m_searched_history_item_list[i])
			delete m_searched_history_item_list[i];
	}
	m_searched_history_item_list.clear();

	if (m_delete_confirm_popup)
		evas_object_del(m_delete_confirm_popup);

}

Eina_Bool Browser_History_Layout::init(void)
{
	BROWSER_LOGD("[%s]", __func__);
	return _create_main_layout();
}

Eina_Bool Browser_History_Layout::_show_searched_history(const char *search_text)
{
	BROWSER_LOGD("search_text = [%s]", search_text);

	for(int i = 0 ; i < m_searched_history_item_list.size() ; i++ ) {
		if (m_searched_history_item_list[i])
			delete m_searched_history_item_list[i];
	}
	m_searched_history_item_list.clear();

	for(int i = 0 ; i < m_searched_history_date_label_list.size() ; i++ ) {
		if (m_searched_history_date_label_list[i])
			free(m_searched_history_date_label_list[i]);
	}
	m_searched_history_date_label_list.clear();

	if (!search_text || !strlen(search_text)) {
		if (m_searched_history_genlist || m_no_content_search_result) {
			elm_box_unpack_all(m_content_box);
			elm_box_pack_start(m_content_box, m_history_genlist);
			evas_object_show(m_history_genlist);

			if (m_searched_history_genlist) {
				evas_object_del(m_searched_history_genlist);
				m_searched_history_genlist = NULL;
			}

			if (m_no_content_search_result) {
				evas_object_del(m_no_content_search_result);
				m_no_content_search_result = NULL;
			}

			if (m_history_list.size() == 0) {
				if (m_no_history_label) {
					elm_box_unpack_all(m_content_box);
					elm_box_pack_start(m_content_box, m_no_history_label);
				}
			}

			return EINA_TRUE;
		}
	}

	Elm_Object_Item *it = elm_genlist_first_item_get(m_history_genlist);
	while (it) {
		Browser_History_DB::history_item *item = NULL;
		item = (Browser_History_DB::history_item *)elm_object_item_data_get(it);
		if (item && elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
			if (item->url.find(search_text) != string::npos || item->title.find(search_text) != string::npos) {
				Browser_History_DB::history_item *searched_item = NULL;
				searched_item = new(nothrow) Browser_History_DB::history_item;
				searched_item->id = item->id;
				searched_item->url = item->url;
				searched_item->title = item->title;
				searched_item->date = item->date;
				searched_item->is_delete = item->is_delete;
				searched_item->user_data = item->user_data;
				m_searched_history_item_list.push_back(searched_item);
			}
		}
		it = elm_genlist_item_next_get(it);
	}

	if (m_searched_history_item_list.size()) {
		if (m_no_content_search_result) {
			evas_object_del(m_no_content_search_result);
			m_no_content_search_result = NULL;
		}

		if (!m_searched_history_genlist) {
			m_searched_history_genlist = _create_history_genlist();
			if (!m_searched_history_genlist) {
				BROWSER_LOGE("_create_history_genlist failed");
				return EINA_FALSE;
			}
		}

		elm_genlist_clear(m_searched_history_genlist);

		Date date;
		Date last_date;
		for(int i = 0 ; i < m_searched_history_item_list.size() ; i++) {
			sscanf(m_searched_history_item_list[i]->date.c_str(), "%d-%d-%d", &date.year, &date.month, &date.day);
			m_searched_history_item_list[i]->user_data = (void *)this;
			if (last_date != date) {
				last_date = date;
				char *labe_item = strdup(m_searched_history_item_list[i]->date.c_str());
				if (!labe_item) {
					BROWSER_LOGE("strdup failed");
					return EINA_FALSE;
				}
				m_searched_history_date_label_list.push_back(labe_item);
				it = elm_genlist_item_append(m_searched_history_genlist, &m_history_group_title_class,
								labe_item, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);

				elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			}
			elm_genlist_item_append(m_searched_history_genlist, &m_history_genlist_item_class,
						m_searched_history_item_list[i], it, ELM_GENLIST_ITEM_NONE,
						__history_item_clicked_cb, this);
		}

		elm_box_unpack_all(m_content_box);
		evas_object_hide(m_history_genlist);
		elm_box_pack_start(m_content_box, m_searched_history_genlist);
		evas_object_show(m_searched_history_genlist);
	} else {
		BROWSER_LOGD("show no content");
		if (m_no_content_search_result)
			evas_object_del(m_no_content_search_result);
		m_no_content_search_result = elm_layout_add(m_content_box);
		if (!m_no_content_search_result) {
			BROWSER_LOGE("elm_layout_add failed");
			return EINA_FALSE;
		}
		elm_layout_theme_set(m_no_content_search_result, "layout", "nocontents", "search");
		elm_object_part_text_set(m_no_content_search_result, "elm.text", BR_STRING_NO_SEARCH_RESULT);
		evas_object_size_hint_weight_set(m_no_content_search_result, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(m_no_content_search_result, EVAS_HINT_FILL, EVAS_HINT_FILL);

		if (m_searched_history_genlist) {
			evas_object_del(m_searched_history_genlist);
			m_searched_history_genlist = NULL;
		}

		elm_box_unpack_all(m_content_box);
		evas_object_hide(m_history_genlist);
		elm_box_pack_start(m_content_box, m_no_content_search_result);
		evas_object_show(m_no_content_search_result);
	}

	return EINA_TRUE;
}

void Browser_History_Layout::__search_delay_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	const char *search_text = elm_entry_entry_get(obj);
	BROWSER_LOGD("search_text=[%s]", search_text);

	if (!data || !search_text)
		return;

	char *utf8_text = elm_entry_markup_to_utf8(search_text);
	if (!utf8_text)
		return;

	Browser_History_Layout *history_layout = (Browser_History_Layout *)data;
	if (!elm_entry_is_empty(obj))
		elm_object_signal_emit(history_layout->m_searchbar, "elm,state,guidetext,hide", "elm");
	if (!history_layout->_show_searched_history(utf8_text))
		BROWSER_LOGE("_show_searched_history failed");

	free(utf8_text);
}

void Browser_History_Layout::_enable_searchbar_layout(Eina_Bool enable)
{
	if (enable) {
		elm_layout_theme_set(m_searchbar_layout, "layout", "application", "searchbar_base");

		if (!m_searchbar) {
			m_searchbar = br_elm_searchbar_add(m_searchbar_layout);
			if (!m_searchbar)
				BROWSER_LOGD("br_elm_searchbar_add failed");
		}

		elm_object_part_content_set(m_searchbar_layout, "searchbar", m_searchbar);
		evas_object_show(m_searchbar);

		elm_object_signal_emit(m_searchbar_layout, "elm,state,show,searchbar", "elm");

		Evas_Object *searchbar_entry = br_elm_searchbar_entry_get(m_searchbar);
		elm_entry_input_panel_layout_set(searchbar_entry, ELM_INPUT_PANEL_LAYOUT_URL);

		evas_object_smart_callback_add(searchbar_entry, "changed", __search_delay_changed_cb, this);
	} else {
		for(int i = 0 ; i < m_searched_history_item_list.size() ; i++ ) {
			if (m_searched_history_item_list[i])
				delete m_searched_history_item_list[i];
		}
		m_searched_history_item_list.clear();

		for(int i = 0 ; i < m_searched_history_date_label_list.size() ; i++ ) {
			if (m_searched_history_date_label_list[i])
				free(m_searched_history_date_label_list[i]);
		}
		m_searched_history_date_label_list.clear();

		if (m_searched_history_genlist || m_no_content_search_result) {
			elm_box_unpack_all(m_content_box);
			elm_box_pack_start(m_content_box, m_history_genlist);
			evas_object_show(m_history_genlist);

			if (m_searched_history_genlist) {
				evas_object_del(m_searched_history_genlist);
				m_searched_history_genlist = NULL;
			}

			if (m_no_content_search_result) {
				evas_object_del(m_no_content_search_result);
				m_no_content_search_result = NULL;
			}
		}

		if (m_searchbar) {
			elm_object_part_content_unset(m_searchbar_layout, "searchbar");
			evas_object_hide(m_searchbar);
		}

		Evas_Object *searchbar_entry = br_elm_searchbar_entry_get(m_searchbar);
		evas_object_smart_callback_del(searchbar_entry, "changed", __search_delay_changed_cb);

		elm_layout_theme_set(m_searchbar_layout, "layout", "application", "noindicator");
	}
}

void Browser_History_Layout::_set_edit_mode(Eina_Bool edit_mode)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Bookmark_View *bookmark_view = m_data_manager->get_bookmark_view();

	hide_notify_popup_layout(m_sub_main_history_layout);

	_enable_searchbar_layout(!edit_mode);

	if (edit_mode) {
#if defined(GENLIST_SWEEP)
		br_elm_genlist_sweep_item_recover(m_history_genlist);
#endif
		elm_object_style_set(m_bg, "edit_mode");

		m_edit_mode_select_all_layout = elm_layout_add(m_content_box);
		if (!m_edit_mode_select_all_layout) {
			BROWSER_LOGE("elm_layout_add failed");
			return;
		}
		elm_layout_theme_set(m_edit_mode_select_all_layout, "genlist", "item", "select_all/default");
		evas_object_size_hint_weight_set(m_edit_mode_select_all_layout, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
		evas_object_size_hint_align_set(m_edit_mode_select_all_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_event_callback_add(m_edit_mode_select_all_layout, EVAS_CALLBACK_MOUSE_DOWN,
								__edit_mode_select_all_clicked_cb, this);

		m_edit_mode_select_all_check_button = elm_check_add(m_edit_mode_select_all_layout);
		if (!m_edit_mode_select_all_check_button) {
			BROWSER_LOGE("elm_layout_add failed");
			return;
		}
		evas_object_smart_callback_add(m_edit_mode_select_all_check_button, "changed",
						__edit_mode_select_all_check_changed_cb, this);
		evas_object_propagate_events_set(m_edit_mode_select_all_check_button, EINA_FALSE);

		elm_object_part_content_set(m_edit_mode_select_all_layout, "elm.icon", m_edit_mode_select_all_check_button);
		elm_object_text_set(m_edit_mode_select_all_layout, BR_STRING_SELECT_ALL);

		elm_box_pack_start(m_content_box, m_edit_mode_select_all_layout);
		evas_object_show(m_edit_mode_select_all_layout);

		elm_genlist_decorate_mode_set(m_history_genlist, EINA_TRUE);
		elm_genlist_select_mode_set(m_history_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);

		bookmark_view->_set_navigationbar_title(BR_STRING_SELECT_HISTORIES);

		if (!bookmark_view->_set_controlbar_type(Browser_Bookmark_View::HISTORY_VIEW_EDIT_MODE))
			BROWSER_LOGE("_set_controlbar_type HISTORY_VIEW_EDIT_MODE failed");
	} else {
		if (m_edit_mode_select_all_layout) {
			elm_box_unpack(m_content_box, m_edit_mode_select_all_layout);
			evas_object_del(m_edit_mode_select_all_layout);
			m_edit_mode_select_all_layout = NULL;
		}
		if (m_edit_mode_select_all_check_button) {
			evas_object_del(m_edit_mode_select_all_check_button);
			m_edit_mode_select_all_check_button = NULL;
		}
		if (m_no_history_label) {
			evas_object_del(m_no_history_label);
			m_no_history_label = NULL;
		}

		Browser_Bookmark_View *bookmark_view = m_data_manager->get_bookmark_view();
		if (!bookmark_view->_set_controlbar_type(Browser_Bookmark_View::HISTORY_VIEW_DEFAULT))
			BROWSER_LOGE("_set_controlbar_type failed");

		Elm_Object_Item *it = NULL;
		Browser_History_DB::history_item *item = NULL;
		it = elm_genlist_first_item_get(m_history_genlist);
		while(it) {
			item = (Browser_History_DB::history_item *)elm_object_item_data_get(it);
			item->is_delete = EINA_FALSE;
			it = elm_genlist_item_next_get(it);
		}

		elm_object_style_set(m_bg, "default");
		elm_genlist_decorate_mode_set(m_history_genlist, EINA_FALSE);
		elm_genlist_select_mode_set(m_history_genlist, ELM_OBJECT_SELECT_MODE_DEFAULT);

		bookmark_view->_set_navigationbar_title(BR_STRING_HISTORY);

		if (m_history_list.size() == 0) {
			elm_box_unpack_all(m_content_box);
			elm_genlist_clear(m_history_genlist);
			evas_object_hide(m_history_genlist);

			if (m_edit_mode_select_all_layout) {
				evas_object_del(m_edit_mode_select_all_layout);
				m_edit_mode_select_all_layout = NULL;
			}
			if (m_edit_mode_select_all_check_button) {
				evas_object_del(m_edit_mode_select_all_check_button);
				m_edit_mode_select_all_check_button = NULL;
			}

			m_no_history_label = elm_label_add(m_searchbar_layout);
			if (!m_no_history_label) {
				BROWSER_LOGE("elm_label_add failed");
				return;
			}

			std::string text = std::string("<color='#646464'>") + std::string(BR_STRING_NO_HISTORY);
			elm_object_text_set(m_no_history_label, text.c_str());
			evas_object_show(m_no_history_label);
			elm_box_pack_start(m_content_box, m_no_history_label);

			elm_object_item_disabled_set(bookmark_view->m_bookmark_edit_controlbar_item, EINA_TRUE);
		}
	}
}

void Browser_History_Layout::_show_selection_info(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Elm_Object_Item *it = elm_genlist_first_item_get(m_history_genlist);
	int selected_count = 0;
	int total_count = 0;
	while(it) {
		Browser_History_DB::history_item *item = NULL;
		item = (Browser_History_DB::history_item *)elm_object_item_data_get(it);
		if (item && elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
			if (item->is_delete)
				selected_count++;
			total_count++;
		}
		it = elm_genlist_item_next_get(it);
	}

	if (selected_count == 0) {
		elm_object_item_disabled_set(m_data_manager->get_bookmark_view()->m_bookmark_delete_controlbar_item, EINA_TRUE);
		hide_notify_popup_layout(m_sub_main_history_layout);
		return;
	} else
		elm_object_item_disabled_set(m_data_manager->get_bookmark_view()->m_bookmark_delete_controlbar_item, EINA_FALSE);

	if (selected_count == 1) {
		show_notify_popup_layout(BR_STRING_ONE_ITEM_SELECTED, 0, m_sub_main_history_layout);
	} else if (selected_count > 1) {
		char *small_popup_text = NULL;
		int string_len = strlen(BR_STRING_ITEMS_SELECTED) + 1;

		small_popup_text = (char *)malloc(string_len * sizeof(char));
		memset(small_popup_text, 0x00, string_len);

		snprintf(small_popup_text, string_len, BR_STRING_ITEMS_SELECTED, selected_count);
		show_notify_popup_layout(small_popup_text, 0, m_sub_main_history_layout);

		if (small_popup_text)
			free(small_popup_text);
		small_popup_text = 0x00;
	}
}

void Browser_History_Layout::_show_delete_processing_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_processing_popup)
		evas_object_del(m_processing_popup);

	m_processing_popup = elm_popup_add(m_searchbar_layout);
	if (!m_processing_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return;
	}

	if (m_processing_popup_layout)
		evas_object_del(m_processing_popup_layout);
	m_processing_popup_layout = elm_layout_add(m_processing_popup);
	if (!m_processing_popup_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return;
	}
	if (!elm_layout_file_set(m_processing_popup_layout, BROWSER_EDJE_DIR"/browser-bookmark-view.edj",
							"browser/popup_center_progressview")) {
		BROWSER_LOGE("elm_layout_file_set failed");
		return;
	}
	evas_object_size_hint_weight_set(m_processing_popup_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_processing_progress_bar = elm_progressbar_add(m_processing_popup);
	elm_object_style_set(m_processing_progress_bar, "list_progress");
	elm_progressbar_horizontal_set(m_processing_progress_bar, EINA_TRUE);
	evas_object_size_hint_align_set(m_processing_progress_bar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(m_processing_progress_bar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_value_set(m_processing_progress_bar, 0.0);

	if (m_processing_popup_timer)
		ecore_timer_del(m_processing_popup_timer);

	m_processed_count = 0;
	m_processed_it = elm_genlist_last_item_get(m_history_genlist);

	Elm_Object_Item *it = elm_genlist_first_item_get(m_history_genlist);
	m_total_item_count = 0;
	while (it) {
		Browser_History_DB::history_item *item = NULL;
		item = (Browser_History_DB::history_item *)elm_object_item_data_get(it);
		if (item && elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
			if(item->is_delete)
				m_total_item_count++;
		}
		it = elm_genlist_item_next_get(it);
	}

	m_processing_popup_timer = ecore_timer_add(0.3, __delete_processing_popup_timer_cb, this);
	evas_object_show(m_processing_progress_bar);

	elm_object_part_content_set(m_processing_popup_layout, "elm.swallow.content", m_processing_progress_bar);
	edje_object_part_text_set(elm_layout_edje_get(m_processing_popup_layout), "elm.title", BR_STRING_PROCESSING);
	edje_object_part_text_set(elm_layout_edje_get(m_processing_popup_layout), "elm.text.left", "");
	edje_object_part_text_set(elm_layout_edje_get(m_processing_popup_layout), "elm.text.right", "");

	elm_object_content_set(m_processing_popup, m_processing_popup_layout);

	Evas_Object *cancel_button = elm_button_add(m_processing_popup);
	if (!cancel_button) {
		BROWSER_LOGE("elm_button_add failed");
		return;
	}
	elm_object_text_set(cancel_button, BR_STRING_CANCEL);
	elm_object_part_content_set(m_processing_popup, "button1", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __delete_processing_popup_response_cb, this);

	evas_object_show(m_processing_popup);
}

void Browser_History_Layout::__delete_processing_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_History_Layout *history_layout = (Browser_History_Layout *)data;

	evas_object_del(history_layout->m_processing_popup);

	history_layout->m_processing_progress_bar = NULL;
	history_layout->m_processed_it = NULL;

	if (history_layout->m_processing_popup_timer) {
		ecore_timer_del(history_layout->m_processing_popup_timer);
		history_layout->m_processing_popup_timer = NULL;
	}

	history_layout->show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);

	if (history_layout->m_history_list.size() == 0) {
		elm_box_unpack_all(history_layout->m_content_box);
		elm_genlist_clear(history_layout->m_history_genlist);
		evas_object_hide(history_layout->m_history_genlist);

		if (history_layout->m_edit_mode_select_all_layout) {
			evas_object_del(history_layout->m_edit_mode_select_all_layout);
			history_layout->m_edit_mode_select_all_layout = NULL;
		}
		if (history_layout->m_edit_mode_select_all_check_button) {
			evas_object_del(history_layout->m_edit_mode_select_all_check_button);
			history_layout->m_edit_mode_select_all_check_button = NULL;
		}

		history_layout->m_no_history_label = elm_label_add(history_layout->m_searchbar_layout);
		if (!history_layout->m_no_history_label) {
			BROWSER_LOGE("elm_label_add failed");
			return;
		}

		std::string text = std::string("<color='#646464'>") + std::string(BR_STRING_NO_HISTORY);
		elm_object_text_set(history_layout->m_no_history_label, text.c_str());
		evas_object_show(history_layout->m_no_history_label);
		elm_box_pack_start(history_layout->m_content_box, history_layout->m_no_history_label);

		Browser_Bookmark_View *bookmark_view = m_data_manager->get_bookmark_view();
		if (!bookmark_view->_set_controlbar_type(Browser_Bookmark_View::HISTORY_VIEW_DEFAULT))
			BROWSER_LOGE("_set_controlbar_type failed");

		elm_object_item_disabled_set(bookmark_view->m_bookmark_edit_controlbar_item, EINA_TRUE);
	}
}

Eina_Bool Browser_History_Layout::__delete_processing_popup_timer_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	Browser_History_Layout *history_layout = (Browser_History_Layout *)data;
	Evas_Object* progress_bar = history_layout->m_processing_progress_bar;

	Eina_Bool check_value = history_layout->m_select_all_check_value;

	double value = elm_progressbar_value_get(progress_bar);
	if (value >= 1.0) {
		__delete_processing_popup_response_cb(history_layout, NULL, NULL);

		history_layout->m_processing_popup_timer = NULL;
		history_layout->show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);

		return ECORE_CALLBACK_CANCEL;
	}

	while (history_layout->m_processed_it) {
		Browser_History_DB::history_item *item = NULL;
		item = (Browser_History_DB::history_item *)elm_object_item_data_get(history_layout->m_processed_it);
		if (item) {			
			if(item->is_delete
			    && elm_genlist_item_select_mode_get(history_layout->m_processed_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
				Eina_Bool ret = m_data_manager->get_history_db()->delete_history(item->id);
				for(int index = 0 ; index < history_layout->m_history_list.size() ; index++) {
					if (history_layout->m_history_list[index]->id == item->id) {
						delete history_layout->m_history_list[index];
						history_layout->m_history_list.erase(history_layout->m_history_list.begin() + index);
					}
				}

				history_layout->m_processed_count++;
				Elm_Object_Item *it = history_layout->m_processed_it;
				history_layout->m_processed_it = elm_genlist_item_prev_get(history_layout->m_processed_it);
				elm_object_item_del(it);
				elm_genlist_realized_items_update(history_layout->m_history_genlist);
			} else
				history_layout->m_processed_it = elm_genlist_item_prev_get(history_layout->m_processed_it);
		} else
			history_layout->m_processed_it = elm_genlist_item_prev_get(history_layout->m_processed_it);

		if (history_layout->m_processed_count % 20 == 0)
			break;
	}

	elm_progressbar_value_set(history_layout->m_processing_progress_bar,
				(float)((float)history_layout->m_processed_count / (float)history_layout->m_total_item_count));

	char progress_text[20] = {0, };
	snprintf(progress_text, sizeof(progress_text) - 1, "%d / %d",
			history_layout->m_processed_count, history_layout->m_total_item_count);

	edje_object_part_text_set(elm_layout_edje_get(history_layout->m_processing_popup_layout),
				"elm.text.right", progress_text);

	return ECORE_CALLBACK_RENEW;
}

void Browser_History_Layout::_show_select_processing_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_processing_popup)
		evas_object_del(m_processing_popup);

	m_processing_popup = elm_popup_add(m_searchbar_layout);
	if (!m_processing_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return;
	}

	if (m_processing_popup_layout)
		evas_object_del(m_processing_popup_layout);
	m_processing_popup_layout = elm_layout_add(m_processing_popup);
	if (!m_processing_popup_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return;
	}
	if (!elm_layout_file_set(m_processing_popup_layout, BROWSER_EDJE_DIR"/browser-bookmark-view.edj",
							"browser/popup_center_progressview")) {
		BROWSER_LOGE("elm_layout_file_set failed");
		return;
	}
	evas_object_size_hint_weight_set(m_processing_popup_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_processing_progress_bar = elm_progressbar_add(m_processing_popup);
	elm_object_style_set(m_processing_progress_bar, "list_progress");
	elm_progressbar_horizontal_set(m_processing_progress_bar, EINA_TRUE);
	evas_object_size_hint_align_set(m_processing_progress_bar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(m_processing_progress_bar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_value_set(m_processing_progress_bar, 0.0);

	if (m_processing_popup_timer)
		ecore_timer_del(m_processing_popup_timer);

	m_processed_count = 0;
	m_processed_it = elm_genlist_first_item_get(m_history_genlist);

	m_processing_popup_timer = ecore_timer_add(0.1, __select_processing_popup_timer_cb, this);
	evas_object_show(m_processing_progress_bar);

	elm_object_part_content_set(m_processing_popup_layout, "elm.swallow.content", m_processing_progress_bar);
	edje_object_part_text_set(elm_layout_edje_get(m_processing_popup_layout), "elm.title", BR_STRING_PROCESSING);
	edje_object_part_text_set(elm_layout_edje_get(m_processing_popup_layout), "elm.text.left", "");
	edje_object_part_text_set(elm_layout_edje_get(m_processing_popup_layout), "elm.text.right", "");

	elm_object_content_set(m_processing_popup, m_processing_popup_layout);

	Evas_Object *cancel_button = elm_button_add(m_processing_popup);
	if (!cancel_button) {
		BROWSER_LOGE("elm_button_add failed");
		return;
	}
	elm_object_text_set(cancel_button, BR_STRING_CANCEL);
	elm_object_part_content_set(m_processing_popup, "button1", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __select_processing_popup_response_cb, this);

	evas_object_show(m_processing_popup);

	Eina_Bool check_value = elm_check_state_get(m_edit_mode_select_all_check_button);
	m_select_all_check_value = check_value;

	if (check_value)
		elm_check_state_set(m_edit_mode_select_all_check_button, !check_value);
}

void Browser_History_Layout::__select_processing_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_History_Layout *history_layout = (Browser_History_Layout *)data;

	evas_object_del(history_layout->m_processing_popup);

	history_layout->m_processing_progress_bar = NULL;
	history_layout->m_processed_it = NULL;

	if (history_layout->m_processing_popup_timer) {
		ecore_timer_del(history_layout->m_processing_popup_timer);
		history_layout->m_processing_popup_timer = NULL;
	}

	Elm_Object_Item *it = elm_genlist_first_item_get(history_layout->m_history_genlist);
	int check_count = 0;
	while (it) {
		Browser_History_DB::history_item *item = NULL;
		item = (Browser_History_DB::history_item *)elm_object_item_data_get(it);
		if (item) {
			if(item->is_delete && elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
				check_count++;
		}
		it = elm_genlist_item_next_get(it);
	}

	history_layout->_delete_date_only_label_genlist_item();

	if (check_count)
		elm_object_item_disabled_set(m_data_manager->get_bookmark_view()->m_bookmark_delete_controlbar_item, EINA_FALSE);
	else
		elm_object_item_disabled_set(m_data_manager->get_bookmark_view()->m_bookmark_delete_controlbar_item, EINA_TRUE);
}

Eina_Bool Browser_History_Layout::__select_processing_popup_timer_cb(void *data)
{
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	Browser_History_Layout *history_layout = (Browser_History_Layout *)data;
	Evas_Object* progress_bar = history_layout->m_processing_progress_bar;

	Eina_Bool check_value = history_layout->m_select_all_check_value;

	double value = elm_progressbar_value_get(progress_bar);
	if (value >= 1.0) {
		__select_processing_popup_response_cb(history_layout, NULL, NULL);

		history_layout->m_processing_popup_timer = NULL;
		elm_check_state_set(history_layout->m_edit_mode_select_all_check_button, !check_value);
		history_layout->_show_selection_info();

		return ECORE_CALLBACK_CANCEL;
	}

	while (history_layout->m_processed_it) {
		Browser_History_DB::history_item *item = NULL;
		item = (Browser_History_DB::history_item *)elm_object_item_data_get(history_layout->m_processed_it);		
		if (item && (elm_genlist_item_select_mode_get(history_layout->m_processed_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)) {
			item->is_delete = !check_value;
			elm_genlist_item_update(history_layout->m_processed_it);
			history_layout->m_processed_count++;
		}
		history_layout->m_processed_it = elm_genlist_item_next_get(history_layout->m_processed_it);

		if (history_layout->m_processed_count % 30 == 0)
			break;
	}

	elm_progressbar_value_set(history_layout->m_processing_progress_bar,
				(float)((float)history_layout->m_processed_count / (float)(history_layout->m_history_list.size())));

	char progress_text[20] = {0, };
	snprintf(progress_text, sizeof(progress_text) - 1, "%d / %d",
			history_layout->m_processed_count, history_layout->m_history_list.size());

	edje_object_part_text_set(elm_layout_edje_get(history_layout->m_processing_popup_layout),
				"elm.text.right", progress_text);

	return ECORE_CALLBACK_RENEW;
}

void Browser_History_Layout::__edit_mode_select_all_check_changed_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_History_Layout *history_layout = (Browser_History_Layout *)data;
	Eina_Bool value = elm_check_state_get(obj);

	if (history_layout->m_history_list.size() >= BROWSER_BOOKMARK_PROCESS_BLOCK_COUNT) {
		elm_check_state_set(obj, !value);
		history_layout->_show_select_processing_popup();
	} else {
		Elm_Object_Item *it = elm_genlist_first_item_get(history_layout->m_history_genlist);
		while (it) {
			Browser_History_DB::history_item *item = NULL;
			item = (Browser_History_DB::history_item *)elm_object_item_data_get(it);
			if (item) {				
				if (elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
					item->is_delete = value;
					elm_genlist_item_update(it);
				}
			}
			it = elm_genlist_item_next_get(it);
		}

		history_layout->_show_selection_info();
	}
}

void Browser_History_Layout::__edit_mode_select_all_clicked_cb(void *data, Evas *evas,
								Evas_Object *obj, void *event_info)
{	
	if (!data)
		return;

	Browser_History_Layout *history_layout = (Browser_History_Layout *)data;

	if (history_layout->m_history_list.size() >= BROWSER_BOOKMARK_PROCESS_BLOCK_COUNT) {
		history_layout->_show_select_processing_popup();
	} else {
		Eina_Bool value = elm_check_state_get(history_layout->m_edit_mode_select_all_check_button);
		elm_check_state_set(history_layout->m_edit_mode_select_all_check_button, !value);

		Elm_Object_Item *it = elm_genlist_first_item_get(history_layout->m_history_genlist);
		while (it) {
			Browser_History_DB::history_item *item = NULL;
			item = (Browser_History_DB::history_item *)elm_object_item_data_get(it);
			if (item) {
				/* Ignore the date label. */
				if (elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
					item->is_delete = !value;
					elm_genlist_item_update(it);
				}
			}
			it = elm_genlist_item_next_get(it);
		}

		history_layout->_show_selection_info();
	}
}

void Browser_History_Layout::__history_item_clicked_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_History_Layout *history_layout = (Browser_History_Layout *)data;

	if (history_layout->m_is_bookmark_on_off_icon_clicked) {
		history_layout->m_is_bookmark_on_off_icon_clicked = EINA_FALSE;
		Elm_Object_Item *selected_item = (Elm_Object_Item *)eventInfo;
		elm_genlist_item_selected_set(selected_item, EINA_FALSE);
		return;
	}

	Elm_Object_Item *seleted_item = elm_genlist_selected_item_get(obj);
	Elm_Object_Item *index_item = elm_genlist_first_item_get(obj);
	int index = 0;

	if (!elm_genlist_decorate_mode_get(obj)) {
		/* calculate the index of the selected item */
		while (index_item != seleted_item) {
			/* Do not count date label item */
			if (elm_genlist_item_select_mode_get(index_item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
				index++;

			index_item = elm_genlist_item_next_get(index_item);
		}

		const char *selected_url = NULL;
		if (history_layout->m_searched_history_genlist == obj)
			selected_url = history_layout->m_searched_history_item_list[index]->url.c_str();
		else
			selected_url = history_layout->m_history_list[index]->url.c_str();

		m_data_manager->get_bookmark_view()->history_item_clicked(selected_url);
	} else {
		Browser_History_DB::history_item *item = NULL;
		item = (Browser_History_DB::history_item *)elm_object_item_data_get(seleted_item);
		if (item && elm_genlist_item_select_mode_get(seleted_item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
			item->is_delete = !(item->is_delete);
			elm_genlist_item_update(seleted_item);
		}
		elm_genlist_item_selected_set(seleted_item, EINA_FALSE);

		int selected_count = 0;
		int total_count = 0;
		Elm_Object_Item *it = elm_genlist_first_item_get(history_layout->m_history_genlist);
		while(it) {
			item = (Browser_History_DB::history_item *)elm_object_item_data_get(it);
			if (elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
				if (item->is_delete)
					selected_count++;
				total_count++;
			}
			it = elm_genlist_item_next_get(it);			
		}

		if (selected_count == total_count)
			elm_check_state_set(history_layout->m_edit_mode_select_all_check_button, EINA_TRUE);
		else
			elm_check_state_set(history_layout->m_edit_mode_select_all_check_button, EINA_FALSE);

		history_layout->_show_selection_info();
	}
}

void Browser_History_Layout::_delete_selected_history(void)
{
	BROWSER_LOGD("[%s]", __func__);

	Elm_Object_Item *it = elm_genlist_first_item_get(m_history_genlist);
	int delete_item_count = 0;
	Browser_History_DB::history_item *item = NULL;
	while (it) {
		item = (Browser_History_DB::history_item *)elm_object_item_data_get(it);
		if ((elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) && item->is_delete) {
			delete_item_count++;
		}
		it = elm_genlist_item_next_get(it);
	}

	if (delete_item_count >= BROWSER_BOOKMARK_PROCESS_BLOCK_COUNT) {
		_show_delete_processing_popup();
	} else {
		Eina_List *delete_item_list = NULL;
		it = elm_genlist_first_item_get(m_history_genlist);
		int remain_count = 0;
		Browser_History_DB::history_item *item = NULL;

		while (it) {
			item = (Browser_History_DB::history_item *)elm_object_item_data_get(it);
			if (elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
				if (item->is_delete)
					delete_item_list = eina_list_append(delete_item_list, it);
				else
					remain_count++;
			}
			it = elm_genlist_item_next_get(it);
		}

		int count = eina_list_count(delete_item_list);
		for (int i = 0; i < count; i++) {
			it = (Elm_Object_Item *)eina_list_data_get(delete_item_list);
			item = (Browser_History_DB::history_item *)elm_object_item_data_get(it);
			/* Delete history item */
			Eina_Bool ret = m_data_manager->get_history_db()->delete_history(item->id);
			if (!ret)
				BROWSER_LOGD("history [%s] delete failed", item->title.c_str());

			delete_item_list = eina_list_next(delete_item_list);

			for(int index = 0 ; index < m_history_list.size() ; index++) {
				if (m_history_list[index]->id == item->id) {
					delete m_history_list[index];
					m_history_list.erase(m_history_list.begin() + index);
				}
			}

			elm_object_item_del(it);
		}
		eina_list_free(delete_item_list);

		elm_check_state_set(m_edit_mode_select_all_check_button, EINA_FALSE);

		_delete_date_only_label_genlist_item();

		if (remain_count == 0) {
			elm_box_unpack_all(m_content_box);
			elm_genlist_clear(m_history_genlist);
			evas_object_hide(m_history_genlist);

			if (m_edit_mode_select_all_layout) {
				evas_object_del(m_edit_mode_select_all_layout);
				m_edit_mode_select_all_layout = NULL;
			}
			if (m_edit_mode_select_all_check_button) {
				evas_object_del(m_edit_mode_select_all_check_button);
				m_edit_mode_select_all_check_button = NULL;
			}

			m_no_history_label = elm_label_add(m_searchbar_layout);
			if (!m_no_history_label) {
				BROWSER_LOGE("elm_label_add failed");
				return;
			}

			std::string text = std::string("<color='#646464'>") + std::string(BR_STRING_NO_HISTORY);
			elm_object_text_set(m_no_history_label, text.c_str());
			evas_object_show(m_no_history_label);
			elm_box_pack_start(m_content_box, m_no_history_label);

			Browser_Bookmark_View *bookmark_view = m_data_manager->get_bookmark_view();
			if (!bookmark_view->_set_controlbar_type(Browser_Bookmark_View::HISTORY_VIEW_DEFAULT))
				BROWSER_LOGE("_set_controlbar_type failed");

			elm_object_item_disabled_set(bookmark_view->m_bookmark_edit_controlbar_item, EINA_TRUE);
		}

		show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);
	}
}

void Browser_History_Layout::_reload_history_genlist(void)
{
	Elm_Object_Item *it = NULL;
	while (it = elm_genlist_first_item_get(m_history_genlist))
		elm_object_item_del(it);

	for(int i = 0 ; i < m_history_list.size() ; i++ ) {
		if (m_history_list[i])
			delete m_history_list[i];
	}
	elm_box_unpack_all(m_content_box);
	m_history_list.clear();	

	for(int i = 0 ; i < m_history_date_label_list.size() ; i++ ) {
		if (m_history_date_label_list[i])
			free(m_history_date_label_list[i]);
	}
	m_history_date_label_list.clear();

	Eina_Bool ret = m_data_manager->get_history_db()->get_history_list(m_history_list);
	if (!ret)
		BROWSER_LOGE("get_history_list failed");

	unsigned int history_count = m_history_list.size();
	BROWSER_LOGD("history_count=%d", history_count);

	Date date;
	for(int i = 0 ; i < history_count; i++) {
		sscanf(m_history_list[i]->date.c_str(), "%d-%d-%d", &date.year, &date.month, &date.day);
		m_history_list[i]->user_data = (void *)this;
		if (m_last_date != date) {
			m_last_date = date;
			char *labe_item = strdup(m_history_list[i]->date.c_str());
			if (!labe_item) {
				BROWSER_LOGE("strdup failed");
				return;
			}
			m_history_date_label_list.push_back(labe_item);
			it = elm_genlist_item_append(m_history_genlist, &m_history_group_title_class,
							labe_item, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);

			elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		}
		elm_genlist_item_append(m_history_genlist, &m_history_genlist_item_class, m_history_list[i], it,
					ELM_GENLIST_ITEM_NONE, __history_item_clicked_cb, this);
	}

	if (m_no_history_label) {
		evas_object_del(m_no_history_label);
		m_no_history_label = NULL;
	}

	if (m_edit_mode_select_all_check_button) {
		evas_object_del(m_edit_mode_select_all_check_button);
		m_edit_mode_select_all_check_button = NULL;
	}

	if (m_edit_mode_select_all_layout) {
		evas_object_del(m_edit_mode_select_all_layout);
		m_edit_mode_select_all_layout = NULL;
	}

	if (m_history_list.size() == 0) {
		evas_object_hide(m_history_genlist);

		m_no_history_label = elm_label_add(m_searchbar_layout);
		if (!m_no_history_label) {
			BROWSER_LOGE("elm_label_add failed");
			return;
		}

		std::string text = std::string("<color='#646464'>") + std::string(BR_STRING_NO_HISTORY);
		elm_object_text_set(m_no_history_label, text.c_str());
		evas_object_show(m_no_history_label);
		elm_box_pack_start(m_content_box, m_no_history_label);

		Browser_Bookmark_View *bookmark_view = m_data_manager->get_bookmark_view();
		if (!bookmark_view->_set_controlbar_type(Browser_Bookmark_View::HISTORY_VIEW_DEFAULT))
			BROWSER_LOGE("_set_controlbar_type failed");
		elm_object_item_disabled_set(bookmark_view->m_bookmark_edit_controlbar_item, EINA_TRUE);
	} else {
		elm_box_pack_start(m_content_box, m_history_genlist);
		evas_object_show(m_history_genlist);

		Browser_Bookmark_View *bookmark_view = m_data_manager->get_bookmark_view();
		elm_object_item_disabled_set(bookmark_view->m_bookmark_edit_controlbar_item, EINA_FALSE);
	}
}

Eina_Bool Browser_History_Layout::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	elm_object_style_set(m_bg, "default");	

	m_sub_main_history_layout = elm_layout_add(m_navi_bar);
	if (!m_sub_main_history_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}
	elm_layout_file_set(m_sub_main_history_layout,
				BROWSER_EDJE_DIR"/browser-bookmark-view.edj",
				"browser/selectioninfo");
	evas_object_size_hint_weight_set(
				m_sub_main_history_layout,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(
				m_sub_main_history_layout,
				EVAS_HINT_FILL, EVAS_HINT_FILL);

	m_searchbar_layout = elm_layout_add(m_sub_main_history_layout);
	if (!m_searchbar_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}

	_enable_searchbar_layout(EINA_TRUE);

	evas_object_show(m_searchbar_layout);

	m_content_box = elm_box_add(m_searchbar_layout);
	if (!m_content_box) {
		BROWSER_LOGE("elm_box_add failed");
		return EINA_FALSE;
	}

	m_history_genlist = _create_history_genlist();
	if (!m_history_genlist) {
		BROWSER_LOGE("_create_history_genlist failed");
		return EINA_FALSE;
	}

	m_history_genlist_item_class.decorate_item_style = "mode/slide2";
	m_history_genlist_item_class.item_style = "dialogue/2text.2icon.3";
	m_history_genlist_item_class.decorate_all_item_style = "dialogue/edit";
	m_history_genlist_item_class.func.text_get = __genlist_label_get_cb;
	m_history_genlist_item_class.func.content_get = __genlist_icon_get_cb;
	m_history_genlist_item_class.func.state_get = NULL;
	m_history_genlist_item_class.func.del = NULL;

	m_history_group_title_class.item_style = "dialogue/title";
	m_history_group_title_class.func.text_get = __genlist_date_label_get_cb;
	m_history_group_title_class.func.content_get = NULL;
	m_history_group_title_class.func.state_get = NULL;
	m_history_group_title_class.func.del = NULL;

	_reload_history_genlist();

	evas_object_show(m_content_box);
	elm_object_part_content_set(m_searchbar_layout, "elm.swallow.content", m_content_box);
	elm_object_part_content_set(m_sub_main_history_layout, "genlist.swallow.contents", m_searchbar_layout);

	return EINA_TRUE;
}

Evas_Object *Browser_History_Layout::_show_delete_confirm_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_delete_confirm_popup = elm_popup_add(m_navi_bar);
	if (!m_delete_confirm_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return NULL;
	}

	evas_object_size_hint_weight_set(m_delete_confirm_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(m_delete_confirm_popup, BR_STRING_DELETE_Q);

	Evas_Object *ok_button = elm_button_add(m_delete_confirm_popup);
	elm_object_text_set(ok_button, BR_STRING_YES);
	elm_object_part_content_set(m_delete_confirm_popup, "button1", ok_button);

	Evas_Object *cancel_button = elm_button_add(m_delete_confirm_popup);
	elm_object_text_set(cancel_button, BR_STRING_NO);
	elm_object_part_content_set(m_delete_confirm_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __cancel_confirm_response_by_slide_button_cb,
										m_delete_confirm_popup);

	evas_object_show(m_delete_confirm_popup);

	return ok_button;
}

#if defined(GENLIST_SWEEP)
void Browser_History_Layout::__sweep_right_genlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data || elm_genlist_decorate_mode_get(obj))
		return;
	else {
		elm_genlist_item_decorate_mode_set((Elm_Object_Item *)event_info, "slide", EINA_TRUE);
		elm_genlist_item_select_mode_set((Elm_Object_Item *)event_info, ELM_OBJECT_SELECT_MODE_NONE);
	}

	Browser_History_Layout *history_layout = (Browser_History_Layout *)data;
	if (history_layout->m_current_sweep_item)
		elm_genlist_item_select_mode_set(history_layout->m_current_sweep_item, ELM_OBJECT_SELECT_MODE_DEFAULT);
	history_layout->m_current_sweep_item = (Elm_Object_Item *)event_info;
}

void Browser_History_Layout::__sweep_cancel_genlist_cb(void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!data)
		return;

	Browser_History_Layout *history_layout = (Browser_History_Layout *)data;
	Elm_Object_Item *it = (Elm_Object_Item*)elm_genlist_decorated_item_get(obj);
	if (it) {
		elm_genlist_item_decorate_mode_set(it, "slide", EINA_FALSE);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DEFAULT);

		if (it == history_layout->m_current_sweep_item)
			history_layout->m_current_sweep_item = NULL;
	}
}

void Browser_History_Layout::__sweep_left_genlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data || elm_genlist_decorate_mode_get(obj))
		return;
	else {
		elm_genlist_item_decorate_mode_set((Elm_Object_Item *)event_info, "slide", EINA_FALSE);
		elm_genlist_item_select_mode_set((Elm_Object_Item *)event_info, ELM_OBJECT_SELECT_MODE_DEFAULT);
	}

	Browser_History_Layout *history_layout = (Browser_History_Layout *)data;
	history_layout->m_current_sweep_item = NULL;
}
#endif

char *Browser_History_Layout::__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	Browser_History_DB::history_item *item = (Browser_History_DB::history_item *)data;
	if (!item)
		return NULL;

	char *mark_up = NULL;

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

char *Browser_History_Layout::__genlist_date_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	char *date_label = (char *)data;

	if (!strncmp(part, "elm.text", strlen("elm.text"))) {
		Date date;
		char buffer[BROWSER_MAX_DATE_LEN] = {0, };

		sscanf(date_label, "%d-%d-%d",
			&date.year, &date.month, &date.day);
//		BROWSER_LOGD("%d %d %d\n", date.day, date.month, date.year);
		time_t raw_time;
		struct tm *time_info;
		time(&raw_time);
		time_info = localtime(&raw_time);
		int day_gap = -1;
//		BROWSER_LOGD("\ncurrent time : %d %d %d\n", time_info->tm_mday, (time_info->tm_mon+1), (time_info->tm_year+1900));
		if (time_info->tm_year == date.year -1900 && time_info->tm_mon == date.month - 1)
			day_gap = time_info->tm_mday - date.day;
		else {
			time_info->tm_year = date.year - 1900;
			time_info->tm_mon = date.month - 1;
		}
		time_info->tm_mday = date.day;
		mktime(time_info);

		if (day_gap == 0)
			strftime (buffer, BROWSER_MAX_DATE_LEN, "Today (%a, %b %d %Y)", time_info);
		else if (day_gap == 1)
			strftime (buffer, 40, "Yesterday (%a, %b %d %Y)", time_info);
		else
			strftime (buffer, 40, "%a, %b %d %Y", time_info);
		return strdup(buffer);
	}
	return NULL;
}

void Browser_History_Layout::__slide_add_to_bookmark_button_clicked_cb(void *data, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_History_DB::history_item *item = (Browser_History_DB::history_item *)data;
	Browser_History_Layout *history_layout = (Browser_History_Layout *)item->user_data;

	/* If add to bookmark is excuted in searched list, remove the searched list. */
	Evas_Object *searchbar = NULL;
	searchbar = edje_object_part_external_object_get(elm_layout_edje_get(history_layout->m_searchbar_layout),
										"searchbar");
	br_elm_searchbar_text_set(searchbar, "");

	if (!m_data_manager->create_add_to_bookmark_view(item->title, item->url)) {
		BROWSER_LOGE("create_add_to_bookmark_view failed");
		return;
	}

	if (!m_data_manager->get_add_to_bookmark_view()->init()) {
		BROWSER_LOGE("get_add_to_bookmark_view()->init() failed");
		m_data_manager->destroy_add_to_bookmark_view();
		return;
	}
}

void Browser_History_Layout::__slide_share_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_History_DB::history_item *item = (Browser_History_DB::history_item *)data;
	Browser_History_Layout *history_layout = (Browser_History_Layout *)item->user_data;
	if (!history_layout->_show_share_popup(item->url.c_str()))
		BROWSER_LOGE("_show_share_popup failed");
}

void Browser_History_Layout::_delete_history_item_by_slide_button(Browser_History_DB::history_item *item)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_History_Layout *history_layout = (Browser_History_Layout *)(item->user_data);

	int ret = EINA_TRUE;
	ret = m_data_manager->get_history_db()->delete_history(item->id);
	if (!ret) {
		BROWSER_LOGE("m_data_manager->get_history_db()->delete_history failed");
		return;
	}

	elm_object_item_del(history_layout->m_current_sweep_item);

	/* If the item is deleted in searched list, then delete it from original history genlist also. */
	Elm_Object_Item *it = elm_genlist_first_item_get(history_layout->m_history_genlist);
	while (it) {
		Browser_History_DB::history_item *item_data = NULL;
		item_data = (Browser_History_DB::history_item *)elm_object_item_data_get(it);
		if (item_data && (elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)) {
			if (item_data->id == item->id) {
				elm_object_item_del(it);
				break;
			}
		}
		it = elm_genlist_item_next_get(it);
	}

	for(int i = 0 ; i < history_layout->m_history_list.size() ; i++) {
		if (history_layout->m_history_list[i]->id == item->id) {
			delete history_layout->m_history_list[i];
			history_layout->m_history_list.erase(history_layout->m_history_list.begin() + i);
			break;
		}
	}

	if (history_layout->m_history_list.size() == 0) {
		history_layout->_reload_history_genlist();
	}

	history_layout->_delete_date_only_label_genlist_item();

	show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);
}

void Browser_History_Layout::__delete_confirm_response_by_edit_mode_cb(void *data, Evas_Object *obj,
								void *event_info)
{
	BROWSER_LOGD("event_info = %d", (int)event_info);

	if (!data)
		return;

	Browser_History_Layout *history_layout = (Browser_History_Layout *)data;

	if (history_layout->m_delete_confirm_popup) {
		evas_object_del(history_layout->m_delete_confirm_popup);
		history_layout->m_delete_confirm_popup = NULL;
	}

	history_layout->_delete_selected_history();

	if (history_layout->m_history_list.size() < BROWSER_BOOKMARK_PROCESS_BLOCK_COUNT) {
		history_layout->_set_edit_mode(EINA_FALSE);
	}
}

void Browser_History_Layout::__delete_confirm_response_by_slide_button_cb(void *data, Evas_Object *obj,
								void *event_info)
{
	BROWSER_LOGD("event_info = %d", (int)event_info);

	if (!data)
		return;

	Browser_History_DB::history_item *item = (Browser_History_DB::history_item *)data;
	Browser_History_Layout *history_layout = (Browser_History_Layout *)(item->user_data);
	if (history_layout->m_delete_confirm_popup) {
		evas_object_del(history_layout->m_delete_confirm_popup);
		history_layout->m_delete_confirm_popup = NULL;
	}

	history_layout->_delete_history_item_by_slide_button(item);
}

void Browser_History_Layout::__cancel_confirm_response_by_slide_button_cb(void *data, Evas_Object *obj,
								void *event_info)
{
	BROWSER_LOGD("event_info = %d", (int)event_info);

	if (!data)
		return;

	Evas_Object *popup = (Evas_Object *)data;
	evas_object_del(popup);
}

void Browser_History_Layout::_delete_date_only_label_genlist_item(void)
{
	BROWSER_LOGD("[%s]", __func__);

	Elm_Object_Item *it = elm_genlist_first_item_get(m_history_genlist);
	/* Remove date only label item */
	while (it) {
		if (elm_genlist_item_select_mode_get(it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
			Elm_Object_Item *tmp_it = elm_genlist_item_next_get(it);
			if (!tmp_it || elm_genlist_item_select_mode_get(tmp_it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
				tmp_it = it;
				it = elm_genlist_item_next_get(it);
				elm_object_item_del(tmp_it);
				continue;
			}
		}
		it = elm_genlist_item_next_get(it);
	}
}

void Browser_History_Layout::__slide_delete_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_History_DB::history_item *item = (Browser_History_DB::history_item *)data;
	Browser_History_Layout *history_layout = (Browser_History_Layout *)item->user_data;

	Evas_Object *ok_button = history_layout->_show_delete_confirm_popup();
	if (ok_button)
		evas_object_smart_callback_add(ok_button, "clicked",
			__delete_confirm_response_by_slide_button_cb, item);
}

void Browser_History_Layout::__bookmark_on_off_icon_clicked_cb(void* data, Evas* evas, Evas_Object* obj, void* ev)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_History_DB::history_item *item = (Browser_History_DB::history_item *)data;
	Browser_History_Layout *history_layout = (Browser_History_Layout *)item->user_data;

	history_layout->m_is_bookmark_on_off_icon_clicked = EINA_TRUE;

	int bookmark_id = -1;
	if (m_data_manager->get_history_db()->is_in_bookmark(item->url.c_str(), &bookmark_id)) {
		if (!elm_icon_file_set(obj, BROWSER_IMAGE_DIR"/I01_icon_bookmark_off.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.\n");
		}
		m_data_manager->get_bookmark_view()->delete_bookmark_item(bookmark_id);
	} else {
		if (!elm_icon_file_set(obj, BROWSER_IMAGE_DIR"/I01_icon_bookmark_on.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.\n");
		}
		m_data_manager->get_bookmark_view()->append_bookmark_item(item->title.c_str(), item->url.c_str());
	}
}

Evas_Object *Browser_History_Layout::__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	if (!data || !part)
		return NULL;

	Browser_History_DB::history_item *item = (Browser_History_DB::history_item *)data;
	Browser_History_Layout *history_layout = (Browser_History_Layout *)item->user_data;
	if (!history_layout)
		return NULL;

	if (!strncmp(part, "elm.icon.1", strlen("elm.icon.1"))) {
		Evas_Object *favicon = NULL;
		favicon = m_data_manager->get_browser_view()->get_favicon(item->url.c_str());
		if (favicon)
			return favicon;
		else {
			Evas_Object *default_favicon = elm_icon_add(obj);
			if (!default_favicon) {
				BROWSER_LOGE("elm_icon_add is failed");
				return NULL;
			}
			elm_icon_file_set(default_favicon, BROWSER_IMAGE_DIR"/faviconDefault.png", NULL);
			evas_object_size_hint_aspect_set(default_favicon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			return default_favicon;
		}
	} else if (!strncmp(part, "elm.icon.2", strlen("elm.icon.2"))) {
		Evas_Object *bookmark_icon = elm_icon_add(obj);
		if (m_data_manager->get_history_db()->is_in_bookmark(item->url.c_str(), NULL)) {
			if (!elm_icon_file_set(bookmark_icon, BROWSER_IMAGE_DIR"/I01_icon_bookmark_on.png", NULL)) {
				BROWSER_LOGE("elm_icon_file_set is failed.\n");
				return NULL;
			}
		} else {
			if (!elm_icon_file_set(bookmark_icon, BROWSER_IMAGE_DIR"/I01_icon_bookmark_off.png", NULL)) {
				BROWSER_LOGE("elm_icon_file_set is failed.\n");
				return NULL;
			}
		}
		evas_object_size_hint_aspect_set(bookmark_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		evas_object_propagate_events_set(bookmark_icon, EINA_FALSE);
		evas_object_repeat_events_set(bookmark_icon, EINA_FALSE);

		evas_object_event_callback_add(bookmark_icon, EVAS_CALLBACK_MOUSE_DOWN,
						__bookmark_on_off_icon_clicked_cb, item);

		return bookmark_icon;
	} else if (!strncmp(part, "elm.edit.icon.1", strlen("elm.edit.icon.1"))) {
		if (elm_genlist_decorate_mode_get(obj)) {
			Evas_Object *check_button = elm_check_add(obj);
			if (!check_button) {
				BROWSER_LOGE("elm_check_add is failed");
				return NULL;
			}
			elm_check_state_pointer_set(check_button, &(item->is_delete));
			elm_check_state_set(check_button, item->is_delete);
			evas_object_smart_callback_add(check_button, "changed", __edit_mode_item_check_changed_cb, item);
			return check_button;
		}
	} else if (!strncmp(part, "elm.slide.swallow.1", strlen("elm.slide.swallow.1"))) {
		Evas_Object *button = elm_button_add(obj);
		if (!button) {
			LOGD("elm_button_add() is failed.");
			return NULL;
		}
		elm_object_style_set(button, "sweep/multiline");
		elm_object_text_set(button, BR_STRING_ADD_TO_BOOKMARKS);
		evas_object_smart_callback_add(button, "clicked", __slide_add_to_bookmark_button_clicked_cb, item);
		return button;
	} else if (!strncmp(part, "elm.slide.swallow.2", strlen("elm.slide.swallow.2"))) {
		Evas_Object *button = elm_button_add(obj);
		if (!button) {
			LOGD("elm_button_add() is failed.");
			return NULL;
		}
		elm_object_style_set(button, "sweep/multiline");
		elm_object_text_set(button, BR_STRING_DELETE);
		evas_object_smart_callback_add(button, "clicked", __slide_delete_button_clicked_cb, item);
		return button;
	}

	return NULL;
}

void Browser_History_Layout::__edit_mode_item_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_History_DB::history_item *item = (Browser_History_DB::history_item *)data;
	Browser_History_Layout *history_layout = (Browser_History_Layout *)(item->user_data);
	int selected_count = 0;
	int total_count = 0;
	Elm_Object_Item *it = elm_genlist_first_item_get(history_layout->m_history_genlist);
	while(it) {
		Browser_History_DB::history_item *item = NULL;
		item = (Browser_History_DB::history_item *)elm_object_item_data_get(it);

		/* Ignore the date label. */
		if (elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
			total_count++;
			if (item->is_delete)
				selected_count++;
		}
		it = elm_genlist_item_next_get(it);
	}

	if (total_count == selected_count)
		elm_check_state_set(history_layout->m_edit_mode_select_all_check_button, EINA_TRUE);
	else
		elm_check_state_set(history_layout->m_edit_mode_select_all_check_button, EINA_FALSE);

	history_layout->_show_selection_info();
}

Evas_Object *Browser_History_Layout::_create_history_genlist(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *genlist = elm_genlist_add(m_searchbar_layout);
	if (genlist) {
		evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

#if defined(GENLIST_SWEEP)
		evas_object_smart_callback_add(genlist, "drag,start,right", __sweep_right_genlist_cb, this);
		evas_object_smart_callback_add(genlist, "drag,start,left", __sweep_left_genlist_cb, this);
		evas_object_smart_callback_add(genlist, "drag,start,up", __sweep_cancel_genlist_cb, this);
		evas_object_smart_callback_add(genlist, "drag,start,down", __sweep_cancel_genlist_cb, this);
#endif
	}

	return genlist;
}

