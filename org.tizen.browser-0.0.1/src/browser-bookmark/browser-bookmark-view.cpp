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
#include "browser-bookmark-view.h"
#include "browser-history-layout.h"
#include "browser-new-folder-view.h"
#include "browser-string.h"
#include "browser-view.h"

#define UPPER_FOLDER_ICON_SIZE	(64 * elm_scale_get())

Browser_Bookmark_View::Browser_Bookmark_View(void)
:	m_back_button(NULL)
	,m_top_control_bar(NULL)
	,m_conformant(NULL)
	,m_content_layout(NULL)
	,m_sub_main_layout(NULL)
	,m_genlist_content_layout(NULL)
	,m_genlist_content_box(NULL)
	,m_main_folder_genlist(NULL)
	,m_sub_folder_genlist(NULL)
	,m_main_layout(NULL)
	,m_upper_folder_list(NULL)
	,m_upper_folder_icon(NULL)
	,m_view_mode(BOOKMARK_VIEW)
	,m_edit_mode_select_all_layout(NULL)
	,m_edit_mode_select_all_check_button(NULL)
	,m_current_folder_id(BROWSER_BOOKMARK_MAIN_FOLDER_ID)
	,m_no_contents(NULL)
	,m_current_genlist_item_to_edit(NULL)
	,m_sub_folder_conformant(NULL)
	,m_processing_popup_timer(NULL)
	,m_processing_progress_bar(NULL)
	,m_processed_it(NULL)
	,m_processed_count(0)
	,m_total_item_count(0)
	,m_processing_popup(NULL)
	,m_processing_popup_layout(NULL)
	,m_select_all_check_value(EINA_FALSE)
	,m_bottom_control_bar(NULL)
	,m_bookmark_delete_controlbar_item(NULL)
	,m_bookmark_edit_controlbar_item(NULL)
	,m_create_folder_controlbar_item(NULL)
	,m_current_sweep_item(NULL)
	,m_navi_it(NULL)
	,m_delete_confirm_popup(NULL)
	,m_rename_edit_field(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
	m_current_bookmark_navigationbar_title = "";
}

Browser_Bookmark_View::~Browser_Bookmark_View(void)
{
	BROWSER_LOGD("[%s]", __func__);
	hide_notify_popup_layout(m_sub_main_layout);

	for(int i = 0 ; i < m_sub_folder_list.size() ; i++ ) {
		if (m_sub_folder_list[i]) {
			delete m_sub_folder_list[i];
			m_sub_folder_list.erase(m_sub_folder_list.begin() + i);
		}
	}
	m_sub_folder_list.clear();

	for(int i = 0 ; i < m_main_folder_list.size() ; i++ ) {
		if (m_main_folder_list[i]) {
			delete m_main_folder_list[i];
			m_main_folder_list.erase(m_main_folder_list.begin() + i);
		}
	}
	m_main_folder_list.clear();

	m_data_manager->destroy_bookmark_db();

	evas_object_smart_callback_del(m_navi_bar, "transition,finished", __naviframe_pop_finished_cb);

	if (m_delete_confirm_popup)
		evas_object_del(m_delete_confirm_popup);

}

Eina_Bool Browser_Bookmark_View::init(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!m_data_manager->create_bookmark_db()) {
		BROWSER_LOGE("m_data_manager->create_bookmark_db failed");
		return EINA_FALSE;
	}

	return _create_main_layout();
}

Eina_Bool Browser_Bookmark_View::append_bookmark_item(const char *title, const char *url)
{
	if (!url || !title) {
		BROWSER_LOGE("url or title is null");
		return EINA_FALSE;
	}

	if (!m_data_manager->get_bookmark_db()->save_bookmark(BROWSER_BOOKMARK_MAIN_FOLDER_ID, title, url)) {
		BROWSER_LOGE("save_bookmark failed");
		return EINA_FALSE;
	}

	Browser_Bookmark_DB::bookmark_item *item = new(nothrow) Browser_Bookmark_DB::bookmark_item;
	if (!item) {
		BROWSER_LOGE("new(nothrow) Browser_Bookmark_DB::bookmark_item failed");
		return EINA_FALSE;
	}

	int bookmark_id = -1;
	m_data_manager->get_bookmark_db()->get_bookmark_id_by_title_url(BROWSER_BOOKMARK_MAIN_FOLDER_ID, title,
				url, &bookmark_id);

	item->parent = BROWSER_BOOKMARK_MAIN_FOLDER_ID;
	item->user_data_1 = (void *)this;
	item->url = std::string(url);
	item->title = std::string(title);
	item->is_folder = EINA_FALSE;
	item->id = bookmark_id;
	item->user_data_2 = (void *)elm_genlist_item_append(m_main_folder_genlist, &m_bookmark_genlist_item_class,
							item, NULL, ELM_GENLIST_ITEM_NONE,
							__bookmark_item_clicked_cb, this);

	m_main_folder_list.push_back(item);

	_show_empty_content_layout(EINA_FALSE);

	return EINA_TRUE;
}

void Browser_Bookmark_View::delete_bookmark_item(int bookmark_id)
{
	BROWSER_LOGD("[%s]", __func__);
	int ret = EINA_TRUE;

	ret = m_data_manager->get_bookmark_db()->delete_bookmark(bookmark_id);
	if (!ret) {
		BROWSER_LOGE("bookmark_db->delete_bookmark failed");
		return;
	}

	Elm_Object_Item *it = elm_genlist_first_item_get(m_main_folder_genlist);
	while (it) {
		Browser_Bookmark_DB::bookmark_item *item = NULL;
		item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
		if (item->id == bookmark_id) {
			elm_object_item_del(it);
			break;
		}

		it = elm_genlist_item_next_get(it);
	}

	for(int index = 0 ; index <  m_main_folder_list.size() ; index++) {
		if (m_main_folder_list[index]->id == bookmark_id) {
			delete m_main_folder_list[index];
			m_main_folder_list.erase(m_main_folder_list.begin() + index);
			break;
		}
	}

	if (m_main_folder_list.size() == 0) {
		_show_empty_content_layout(EINA_TRUE);
	}
}

void Browser_Bookmark_View::return_to_bookmark_view(int added_bookmark_id)
{
	BROWSER_LOGD("added_bookmark_id = %d", added_bookmark_id);
	if (m_view_mode == BOOKMARK_VIEW) {
		/* In case that bookmark view -> Edit bookmark item, then return to bookmark view. */
		if (elm_genlist_decorate_mode_get(_get_current_folder_genlist())) {
			int delete_item_count = 0;
			int editable_item_count = 0;
			Elm_Object_Item *it = elm_genlist_first_item_get(_get_current_folder_genlist());
			while(it) {
				Browser_Bookmark_DB::bookmark_item *item = NULL;
				item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
				if (item->is_editable)
					editable_item_count++;
				if (item->is_delete)
					delete_item_count++;
				it = elm_genlist_item_next_get(it);
			}

			if (editable_item_count == delete_item_count)
				elm_check_state_set(m_edit_mode_select_all_check_button, EINA_TRUE);
			else
				elm_check_state_set(m_edit_mode_select_all_check_button, EINA_FALSE);

			if (delete_item_count == 0)
				elm_object_item_disabled_set(m_bookmark_delete_controlbar_item, EINA_TRUE);
		}

		int size = _get_current_folder_item_list().size();
		if (size == 0)
			_show_empty_content_layout(EINA_TRUE);
		else
			_show_empty_content_layout(EINA_FALSE);
	} else {
		if (!m_data_manager->get_history_layout())
			return;

		/* In case that history view -> add to bookmark by slide-right, then return to history view. */
		if (m_data_manager->get_history_layout()->m_current_sweep_item) {
			elm_genlist_item_decorate_mode_set(m_data_manager->get_history_layout()->m_current_sweep_item, "slide", EINA_FALSE);
			elm_genlist_item_select_mode_set(m_data_manager->get_history_layout()->m_current_sweep_item, ELM_OBJECT_SELECT_MODE_DEFAULT);
			m_data_manager->get_history_layout()->m_current_sweep_item = NULL;
		}
	}

	/* boomark view -> history -> add to bookmark by slide button, then done. */
	if (added_bookmark_id != -1) {
		Browser_Bookmark_DB::bookmark_item *item = m_data_manager->get_bookmark_db()->get_bookmark_item_by_id(added_bookmark_id);
		if (!item) {
			BROWSER_LOGE("get_bookmark_item_by_id failed");
			return;
		}

		Elm_Object_Item *it = elm_genlist_first_item_get(_get_current_folder_genlist());
		while (it) {
			Browser_Bookmark_DB::bookmark_item *exist_item = NULL;
			exist_item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
			if (exist_item->id == added_bookmark_id) {
				exist_item->url = item->url;
				exist_item->title = item->title;
				exist_item->parent = item->parent;
				elm_genlist_item_update(it);
				return;
			}
			it = elm_genlist_item_next_get(it);
		}

		if (m_current_folder_id == item->parent) {
			/* If the saved bookmark item is in the current folder, just add it to current folder. */
			Evas_Object *genlist = NULL;
			if (item->parent == BROWSER_BOOKMARK_MAIN_FOLDER_ID)
				genlist = m_main_folder_genlist;
			else
				genlist = m_sub_folder_genlist;

			item->user_data_1 = (void *)this;
			if (item->is_folder) {
				item->user_data_2 = (void *)elm_genlist_item_append(genlist, &m_folder_genlist_item_class,
									item, NULL, ELM_GENLIST_ITEM_NONE,
									__bookmark_item_clicked_cb, this);

			} else {
				item->user_data_2 = (void *)elm_genlist_item_append(genlist, &m_bookmark_genlist_item_class,
									item, NULL, ELM_GENLIST_ITEM_NONE,
									__bookmark_item_clicked_cb, this);
			}

			if (item->parent == BROWSER_BOOKMARK_MAIN_FOLDER_ID)
				m_main_folder_list.push_back(item);
			else
				m_sub_folder_list.push_back(item);
		} else if (m_current_folder_id != BROWSER_BOOKMARK_MAIN_FOLDER_ID
			   && item->parent == BROWSER_BOOKMARK_MAIN_FOLDER_ID){
			/* If the current folder is sub folder but the saved folder is main folder,
			  * add the saved bookmark item to main folder. */
			item->user_data_1 = (void *)this;
			if (item->is_folder) {
			item->user_data_2 = (void *)elm_genlist_item_append(m_main_folder_genlist, &m_folder_genlist_item_class,
									item, NULL, ELM_GENLIST_ITEM_NONE,
									__bookmark_item_clicked_cb, this);
			} else {
			item->user_data_2 = (void *)elm_genlist_item_append(m_main_folder_genlist, &m_bookmark_genlist_item_class,
									item, NULL, ELM_GENLIST_ITEM_NONE,
									__bookmark_item_clicked_cb, this);
			}
			m_main_folder_list.push_back(item);
		}

		/* If the bookmark is empty page, destroy the empty layout to show the bookmark list. */
		if (_get_current_folder_item_list().size() == 0)
			_show_empty_content_layout(EINA_TRUE);
		else
			_show_empty_content_layout(EINA_FALSE);
	}
}

void Browser_Bookmark_View::__back_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;
	Evas_Object *navi_bar = bookmark_view->m_navi_bar;

	if (elm_naviframe_bottom_item_get(navi_bar) != elm_naviframe_top_item_get(navi_bar))
		elm_naviframe_item_pop(navi_bar);

	/* Set the title if enter the add to bookmark while loading. */
	m_data_manager->get_browser_view()->return_to_browser_view();
}

void Browser_Bookmark_View::__edit_controlbar_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;
	Browser_History_Layout *history_layout = m_data_manager->get_history_layout();

	if (bookmark_view->m_view_mode == BOOKMARK_VIEW) {
		if (!elm_genlist_decorate_mode_get(bookmark_view->_get_current_folder_genlist()))
			/* Edit bookmark */
			bookmark_view->_set_edit_mode(EINA_TRUE);
		else {
			if (bookmark_view->m_rename_edit_field) {
				Browser_Bookmark_DB::bookmark_item *item = NULL;
				item = (Browser_Bookmark_DB::bookmark_item *)evas_object_data_get(bookmark_view->m_rename_edit_field, "item");
				Evas_Object *edit_field_entry = br_elm_editfield_entry_get(bookmark_view->m_rename_edit_field);
				elm_entry_entry_set(edit_field_entry, item->title.c_str());
			}

			/* Cancel Edit bookmark */
			bookmark_view->_set_edit_mode(EINA_FALSE);
		}
	} else if (history_layout) {
		if (!elm_genlist_decorate_mode_get(history_layout->m_history_genlist)) {
			/* Delete history */
			history_layout->_set_edit_mode(EINA_TRUE);
		} else {
			/* Cancel Delete bookmark */
			history_layout->_set_edit_mode(EINA_FALSE);
		}
	}

	if (bookmark_view->m_rename_edit_field)
		elm_object_focus_set(bookmark_view->m_rename_edit_field, EINA_FALSE);
}

void Browser_Bookmark_View::__delete_controlbar_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;
	Browser_History_Layout *history_layout = m_data_manager->get_history_layout();

	if (bookmark_view->m_view_mode == BOOKMARK_VIEW) {
		if (elm_genlist_decorate_mode_get(bookmark_view->_get_current_folder_genlist())) {
			/* Delete bookmark */
			Evas_Object *ok_button = bookmark_view->_show_delete_confirm_popup();
			if (ok_button)
				evas_object_smart_callback_add(ok_button, "clicked",
						__delete_confirm_response_by_edit_mode_cb, bookmark_view);

		}
	} else {
		if (history_layout && elm_genlist_decorate_mode_get(history_layout->m_history_genlist)) {
			/* Delete bookmark */
			Evas_Object *ok_button = history_layout->_show_delete_confirm_popup();
			if (ok_button)
				evas_object_smart_callback_add(ok_button, "clicked",
						Browser_History_Layout::__delete_confirm_response_by_edit_mode_cb,
						history_layout);
		}
	}
}

void Browser_Bookmark_View::_delete_bookmark_item_from_folder_list(Browser_Bookmark_DB::bookmark_item *item)
{
	if (m_current_folder_id == BROWSER_BOOKMARK_MAIN_FOLDER_ID) {
		for(int index = 0 ; index <  m_main_folder_list.size() ; index++) {
			if (m_main_folder_list[index] == item) {
				delete m_main_folder_list[index];
				m_main_folder_list.erase(m_main_folder_list.begin() + index);
			}
		}
	} else {
		for(int index = 0 ; index <  m_sub_folder_list.size() ; index++) {
			if (m_sub_folder_list[index] == item) {
				delete m_sub_folder_list[index];
				m_sub_folder_list.erase(m_sub_folder_list.begin() + index);
			}
		}
	}
}

void Browser_Bookmark_View::_delete_selected_bookmark(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Bookmark_DB *bookmark_db = m_data_manager->get_bookmark_db();
	Eina_List *delete_item_list = NULL;
	Elm_Object_Item *it = elm_genlist_first_item_get(_get_current_folder_genlist());
	int remain_count = 0;
	while(it) {
		Browser_Bookmark_DB::bookmark_item *item = NULL;
		item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
		if (item->is_delete)
			delete_item_list = eina_list_append(delete_item_list, it);
		else
			remain_count++;
		it = elm_genlist_item_next_get(it);
	}

	int count = eina_list_count(delete_item_list);

	if (count >= BROWSER_BOOKMARK_PROCESS_BLOCK_COUNT) {
		/* If delete more than 100 items, show the processing popup. */
		_show_delete_processing_popup();
	} else {
		for (int i = 0; i < count; i++) {
			it = (Elm_Object_Item *)eina_list_data_get(delete_item_list);
			Browser_Bookmark_DB::bookmark_item *item = NULL;
			Eina_Bool ret = EINA_TRUE;
			item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
			if (item->is_folder) {
				int folder_id = 0;
				ret = bookmark_db->get_folder_id(m_current_folder_id, item->title.c_str(), &folder_id);
				if (!ret) {
					BROWSER_LOGE("bookmark_db->get_folder_id failed");
					return;
				}
				ret = bookmark_db->delete_folder(folder_id);
				if (!ret) {
					BROWSER_LOGE("bookmark_db->delete_folder failed");
					return;
				}
			} else {
				ret = bookmark_db->delete_bookmark(item->id);
				if (!ret) {
					BROWSER_LOGE("bookmark_db->delete_bookmark failed");
					return;
				}
			}
			delete_item_list = eina_list_next(delete_item_list);

			_delete_bookmark_item_from_folder_list(item);

			elm_object_item_del(it);
		}
		eina_list_free(delete_item_list);

		elm_check_state_set(m_edit_mode_select_all_check_button, EINA_FALSE);

		if (remain_count == 0)
			_show_empty_content_layout(EINA_TRUE);

		show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);
	}
}

void Browser_Bookmark_View::__edit_mode_item_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_Bookmark_DB::bookmark_item *item = (Browser_Bookmark_DB::bookmark_item *)data;
	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)(item->user_data_1);
	int delete_item_count = 0;
	int editable_item_count = 0;

	Elm_Object_Item *it = elm_genlist_first_item_get(bookmark_view->_get_current_folder_genlist());
	while(it) {
		Browser_Bookmark_DB::bookmark_item *item = NULL;
		item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
		if (item->is_editable)
			editable_item_count++;
		if (item->is_delete)
			delete_item_count++;
		it = elm_genlist_item_next_get(it);
	}

	if (editable_item_count == delete_item_count)
		elm_check_state_set(bookmark_view->m_edit_mode_select_all_check_button, EINA_TRUE);
	else
		elm_check_state_set(bookmark_view->m_edit_mode_select_all_check_button, EINA_FALSE);

	bookmark_view->_show_selection_info();
}

void Browser_Bookmark_View::_show_delete_processing_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_processing_popup)
		evas_object_del(m_processing_popup);

	m_processing_popup = elm_popup_add(m_main_layout);
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
	m_processed_it = elm_genlist_last_item_get(_get_current_folder_genlist());

	Elm_Object_Item *it = elm_genlist_first_item_get(_get_current_folder_genlist());
	m_total_item_count = 0;
	while(it) {
		Browser_Bookmark_DB::bookmark_item *item = NULL;
		item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
		if (item) {
			if(item->is_editable && item->is_delete)
				m_total_item_count++;
		}
		it = elm_genlist_item_next_get(it);
	}

	m_processing_popup_timer = ecore_timer_add(0.1, __delete_processing_popup_timer_cb, this);
	evas_object_show(m_processing_progress_bar);

	elm_object_part_content_set(m_processing_popup_layout, "elm.swallow.content", m_processing_progress_bar);
	edje_object_part_text_set(elm_layout_edje_get(m_processing_popup_layout), "elm.title", BR_STRING_PROCESSING);
	edje_object_part_text_set(elm_layout_edje_get(m_processing_popup_layout), "elm.text.left", "");
	edje_object_part_text_set(elm_layout_edje_get(m_processing_popup_layout), "elm.text.right", "");

	elm_object_content_set(m_processing_popup, m_processing_popup_layout);

	Evas_Object *cancel_button = elm_button_add(m_processing_popup);
	elm_object_text_set(cancel_button, BR_STRING_CANCEL);
	elm_object_part_content_set(m_processing_popup, "button1", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __delete_processing_popup_response_cb, this);

	evas_object_show(m_processing_popup);
}

Eina_Bool Browser_Bookmark_View::__delete_processing_popup_timer_cb(void *data)
{
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;
	Evas_Object* progress_bar = bookmark_view->m_processing_progress_bar;

	Eina_Bool check_value = bookmark_view->m_select_all_check_value;

	double value = elm_progressbar_value_get(progress_bar);
	if (value >= 1.0) {
		__delete_processing_popup_response_cb(bookmark_view, NULL, NULL);

		bookmark_view->m_processing_popup_timer = NULL;
		bookmark_view->show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);

		return ECORE_CALLBACK_CANCEL;
	}

	while(bookmark_view->m_processed_it) {
		Browser_Bookmark_DB::bookmark_item *item = NULL;
		item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(bookmark_view->m_processed_it);
		if (item) {
			if(item->is_editable && item->is_delete) {
				Browser_Bookmark_DB *bookmark_db = m_data_manager->get_bookmark_db();
				int ret = EINA_TRUE;
				if (item->is_folder) {
					int folder_id = 0;
					ret = bookmark_db->get_folder_id(bookmark_view->m_current_folder_id,
										item->title.c_str(), &folder_id);
					if (!ret) {
						BROWSER_LOGE("bookmark_db->get_folder_id failed");
						return ECORE_CALLBACK_CANCEL;
					}
					ret = bookmark_db->delete_folder(folder_id);
					if (!ret) {
						BROWSER_LOGE("bookmark_db->delete_folder failed");
						return ECORE_CALLBACK_CANCEL;
					}
				} else {
					ret = bookmark_db->delete_bookmark(item->id);
					if (!ret) {
						BROWSER_LOGE("bookmark_db->delete_bookmark failed");
						return ECORE_CALLBACK_CANCEL;
					}
				}
				bookmark_view->_delete_bookmark_item_from_folder_list(item);
				bookmark_view->m_processed_count++;
				Elm_Object_Item *it = bookmark_view->m_processed_it;
				bookmark_view->m_processed_it = elm_genlist_item_prev_get(bookmark_view->m_processed_it);
				elm_object_item_del(it);
			} else
				bookmark_view->m_processed_it = elm_genlist_item_prev_get(bookmark_view->m_processed_it);
		} else
			bookmark_view->m_processed_it = elm_genlist_item_prev_get(bookmark_view->m_processed_it);

		if (bookmark_view->m_processed_count % 5 == 0)
			break;
	}

	elm_progressbar_value_set(bookmark_view->m_processing_progress_bar,
				(float)((float)bookmark_view->m_processed_count / (float)bookmark_view->m_total_item_count));

	char progress_text[20] = {0, };
	snprintf(progress_text, sizeof(progress_text) - 1, "%d / %d",
			bookmark_view->m_processed_count, bookmark_view->m_total_item_count);

	edje_object_part_text_set(elm_layout_edje_get(bookmark_view->m_processing_popup_layout),
				"elm.text.right", progress_text);

	return ECORE_CALLBACK_RENEW;
}

void Browser_Bookmark_View::_show_select_processing_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_processing_popup)
		evas_object_del(m_processing_popup);

	m_processing_popup = elm_popup_add(m_main_layout);
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
	m_processed_it = elm_genlist_first_item_get(_get_current_folder_genlist());

	Elm_Object_Item *it = elm_genlist_first_item_get(_get_current_folder_genlist());
	m_total_item_count = 0;
	while(it) {
		Browser_Bookmark_DB::bookmark_item *item = NULL;
		item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
		if (item) {
			if(item->is_editable)
				m_total_item_count++;
		}
		it = elm_genlist_item_next_get(it);
	}

	m_processing_popup_timer = ecore_timer_add(0.1, __select_processing_popup_timer_cb, this);
	evas_object_show(m_processing_progress_bar);

	elm_object_part_content_set(m_processing_popup_layout, "elm.swallow.content", m_processing_progress_bar);
	edje_object_part_text_set(elm_layout_edje_get(m_processing_popup_layout), "elm.title", BR_STRING_PROCESSING);
	edje_object_part_text_set(elm_layout_edje_get(m_processing_popup_layout), "elm.text.left", "");
	edje_object_part_text_set(elm_layout_edje_get(m_processing_popup_layout), "elm.text.right", "");

	elm_object_content_set(m_processing_popup, m_processing_popup_layout);

	Evas_Object *cancel_button = elm_button_add(m_processing_popup);
	elm_object_text_set(cancel_button, BR_STRING_CANCEL);
	elm_object_part_content_set(m_processing_popup, "button1", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __select_processing_popup_response_cb, this);

	evas_object_show(m_processing_popup);

	Eina_Bool check_value = elm_check_state_get(m_edit_mode_select_all_check_button);
	m_select_all_check_value = check_value;

	if (check_value)
		elm_check_state_set(m_edit_mode_select_all_check_button, !check_value);
}

Eina_Bool Browser_Bookmark_View::__select_processing_popup_timer_cb(void *data)
{
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;
	Evas_Object* progress_bar = bookmark_view->m_processing_progress_bar;

	Eina_Bool check_value = bookmark_view->m_select_all_check_value;

	double value = elm_progressbar_value_get(progress_bar);
	if (value >= 1.0) {
		__select_processing_popup_response_cb(bookmark_view, NULL, NULL);

		bookmark_view->m_processing_popup_timer = NULL;
		elm_check_state_set(bookmark_view->m_edit_mode_select_all_check_button, !check_value);
		bookmark_view->_show_selection_info();

		return ECORE_CALLBACK_CANCEL;
	}

	while(bookmark_view->m_processed_it) {
		Browser_Bookmark_DB::bookmark_item *item = NULL;
		item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(bookmark_view->m_processed_it);
		if (item) {
			if(item->is_editable) {
				item->is_delete = !check_value;
				elm_genlist_item_update(bookmark_view->m_processed_it);
				bookmark_view->m_processed_count++;
			}
		}
		bookmark_view->m_processed_it = elm_genlist_item_next_get(bookmark_view->m_processed_it);

		if (bookmark_view->m_processed_count % 30 == 0)
			break;
	}

	elm_progressbar_value_set(bookmark_view->m_processing_progress_bar,
				(float)((float)bookmark_view->m_processed_count / (float)bookmark_view->m_total_item_count));

	char progress_text[20] = {0, };
	snprintf(progress_text, sizeof(progress_text) - 1, "%d / %d",
			bookmark_view->m_processed_count, bookmark_view->m_total_item_count);

	edje_object_part_text_set(elm_layout_edje_get(bookmark_view->m_processing_popup_layout),
				"elm.text.right", progress_text);

	return ECORE_CALLBACK_RENEW;
}

void Browser_Bookmark_View::__delete_processing_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;

	bookmark_view->m_processing_progress_bar = NULL;
	bookmark_view->m_processed_it = NULL;

	evas_object_del(bookmark_view->m_processing_popup);

	if (bookmark_view->m_processing_popup_timer) {
		ecore_timer_del(bookmark_view->m_processing_popup_timer);
		bookmark_view->m_processing_popup_timer = NULL;
	}

	bookmark_view->show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);

	vector<Browser_Bookmark_DB::bookmark_item *> item_list = bookmark_view->_get_current_folder_item_list();
	if (item_list.size() == 0)
		bookmark_view->_show_empty_content_layout(EINA_TRUE);
}

void Browser_Bookmark_View::__select_processing_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;

	evas_object_del(bookmark_view->m_processing_popup);
	bookmark_view->m_processing_progress_bar = NULL;
	bookmark_view->m_processed_it = NULL;

	if (bookmark_view->m_processing_popup_timer) {
		ecore_timer_del(bookmark_view->m_processing_popup_timer);
		bookmark_view->m_processing_popup_timer = NULL;
	}

	Elm_Object_Item *it = elm_genlist_first_item_get(bookmark_view->_get_current_folder_genlist());
	int check_count = 0;
	while(it) {
		Browser_Bookmark_DB::bookmark_item *item = NULL;
		item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
		if (item) {
			if(item->is_editable && item->is_delete)
				check_count++;
		}
		it = elm_genlist_item_next_get(it);
	}

	if (check_count)
		elm_object_item_disabled_set(bookmark_view->m_bookmark_delete_controlbar_item, EINA_FALSE);
	else
		elm_object_item_disabled_set(bookmark_view->m_bookmark_delete_controlbar_item, EINA_TRUE);
}

void Browser_Bookmark_View::__edit_mode_select_all_clicked_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;
	int item_count = bookmark_view->_get_current_folder_item_list().size();
	BROWSER_LOGD("item_count=%d", item_count);

	if (item_count >= BROWSER_BOOKMARK_PROCESS_BLOCK_COUNT)
		bookmark_view->_show_select_processing_popup();
	else {
		Eina_Bool value = elm_check_state_get(bookmark_view->m_edit_mode_select_all_check_button);
		elm_check_state_set(bookmark_view->m_edit_mode_select_all_check_button, !value);

		Elm_Object_Item *it = elm_genlist_first_item_get(bookmark_view->_get_current_folder_genlist());
		while(it) {
			Browser_Bookmark_DB::bookmark_item *item = NULL;
			item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
			if (item) {
				if(item->is_editable) {
					item->is_delete = !value;
					elm_genlist_item_update(it);
				}
			}
			it = elm_genlist_item_next_get(it);
		}

		bookmark_view->_show_selection_info();
	}
}

void Browser_Bookmark_View::__edit_mode_select_all_check_changed_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;
	int item_count = bookmark_view->_get_current_folder_item_list().size();
	BROWSER_LOGD("item_count=%d", item_count);
	Eina_Bool value = elm_check_state_get(obj);

	if (item_count >= BROWSER_BOOKMARK_PROCESS_BLOCK_COUNT) {
		/* Workaround to share the same code with __edit_mode_select_all_clicked_cb */
		elm_check_state_set(obj, !value);
		bookmark_view->_show_select_processing_popup();
	} else {
		Elm_Object_Item *it = elm_genlist_first_item_get(bookmark_view->_get_current_folder_genlist());
		while(it) {
			Browser_Bookmark_DB::bookmark_item *item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
			if (item) {
				if(item->is_editable) {
					item->is_delete = value;
					elm_genlist_item_update(it);
				}
			}
			it = elm_genlist_item_next_get(it);
		}

		bookmark_view->_show_selection_info();
	}
}

void Browser_Bookmark_View::_show_selection_info(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int editable_item_count = 0;
	int editable_folder_count = 0;

	Elm_Object_Item *it = elm_genlist_first_item_get(_get_current_folder_genlist());
	while(it) {
		Browser_Bookmark_DB::bookmark_item *item = NULL;
		item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
		if (item->is_delete) {
			if (item->is_folder)
				editable_folder_count++;
			else
				editable_item_count++;
		}
		it = elm_genlist_item_next_get(it);
	}

	if (editable_folder_count == 0 && editable_item_count == 0) {
		elm_object_item_disabled_set(m_bookmark_delete_controlbar_item, EINA_TRUE);
		hide_notify_popup_layout(m_sub_main_layout);
		return;
	} else
		elm_object_item_disabled_set(m_bookmark_delete_controlbar_item, EINA_FALSE);

	if (editable_folder_count + editable_item_count == 1) {
		show_notify_popup_layout(BR_STRING_ONE_ITEM_SELECTED, 0, m_sub_main_layout);
	} else if (editable_folder_count + editable_item_count > 1) {
		char *small_popup_text = NULL;
		int string_len = strlen(BR_STRING_ITEMS_SELECTED) + 4; /* 4 : reserved for maximun count */

		small_popup_text = (char *)malloc(string_len * sizeof(char));
		memset(small_popup_text, 0x00, string_len);

		snprintf(small_popup_text, string_len, BR_STRING_ITEMS_SELECTED, editable_folder_count + editable_item_count);
		show_notify_popup_layout(small_popup_text, 0, m_sub_main_layout);

		if (small_popup_text)
			free(small_popup_text);
		small_popup_text = 0x00;
	}
}

void Browser_Bookmark_View::_set_edit_mode(Eina_Bool edit_mode)
{
	BROWSER_LOGD("[%s]", __func__);
	hide_notify_popup_layout(m_sub_main_layout);

	if (edit_mode) {
#if defined(GENLIST_SWEEP)
		if (m_current_folder_id = BROWSER_BOOKMARK_MAIN_FOLDER_ID)
			br_elm_genlist_sweep_item_recover(m_main_folder_genlist);
		else
			br_elm_genlist_sweep_item_recover(m_sub_folder_genlist);
#endif
		elm_object_style_set(m_bg, "edit_mode");

		m_edit_mode_select_all_layout = elm_layout_add(m_genlist_content_box);
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

		elm_box_pack_start(m_genlist_content_box, m_edit_mode_select_all_layout);
		evas_object_show(m_edit_mode_select_all_layout);

		Elm_Object_Item *it = NULL;
		Browser_Bookmark_DB::bookmark_item *item = NULL;
		it = elm_genlist_first_item_get(_get_current_folder_genlist());
		while(it) {
			item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
			if (!item->is_editable)
				elm_object_item_disabled_set(it, EINA_TRUE);

			it = elm_genlist_item_next_get(it);
		}
		elm_genlist_reorder_mode_set(_get_current_folder_genlist(), EINA_TRUE);
		elm_genlist_decorate_mode_set(_get_current_folder_genlist(), EINA_TRUE);
		elm_genlist_select_mode_set(_get_current_folder_genlist(), ELM_OBJECT_SELECT_MODE_ALWAYS);

		_set_navigationbar_title(BR_STRING_SELECT_ITEMS);

		if (m_current_folder_id != BROWSER_BOOKMARK_MAIN_FOLDER_ID) {
			edje_object_signal_emit(elm_layout_edje_get(m_genlist_content_layout),
								"hide,upper_folder,signal", "");
			evas_object_hide(m_upper_folder_list);
		}

		if (!_set_controlbar_type(BOOKMARK_VIEW_EDIT_MODE))
			BROWSER_LOGE("_set_controlbar_type(BOOKMARK_VIEW_DEFAULT) failed");
	}else {
		elm_object_style_set(m_bg, "default");

		if (m_edit_mode_select_all_check_button) {
			evas_object_del(m_edit_mode_select_all_check_button);
			m_edit_mode_select_all_check_button = NULL;
		}
		if (m_edit_mode_select_all_layout) {
			elm_box_unpack(m_genlist_content_box, m_edit_mode_select_all_layout);
			evas_object_del(m_edit_mode_select_all_layout);
			m_edit_mode_select_all_layout = NULL;
		}

		Elm_Object_Item *it = NULL;
		Browser_Bookmark_DB::bookmark_item *item = NULL;
		it = elm_genlist_first_item_get(_get_current_folder_genlist());
		while(it) {
			item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
			if (!item->is_editable)
				elm_object_item_disabled_set(it, EINA_FALSE);
			item->is_delete = EINA_FALSE;
			it = elm_genlist_item_next_get(it);
		}
		elm_genlist_decorate_mode_set(_get_current_folder_genlist(), EINA_FALSE);
		elm_genlist_reorder_mode_set(_get_current_folder_genlist(), EINA_FALSE);

		elm_genlist_select_mode_set(_get_current_folder_genlist(), ELM_OBJECT_SELECT_MODE_DEFAULT);

		_set_navigationbar_title(BR_STRING_BOOKMARKS);

		if (m_current_folder_id != BROWSER_BOOKMARK_MAIN_FOLDER_ID) {
			edje_object_signal_emit(elm_layout_edje_get(m_genlist_content_layout),
								"show,upper_folder,signal", "");
			evas_object_show(m_upper_folder_list);
		}

		if (!_set_controlbar_type(BOOKMARK_VIEW_DEFAULT))
			BROWSER_LOGE("_set_controlbar_type(BOOKMARK_VIEW_DEFAULT) failed");
	}
}

void Browser_Bookmark_View::_set_view_mode(view_mode mode)
{
	BROWSER_LOGD("mode=%d", mode);
	Evas_Object *history_layout = NULL;
	if (m_data_manager->get_history_layout())
		history_layout = m_data_manager->get_history_layout()->get_main_layout();

	m_view_mode = mode;
	if (mode == HISTORY_VIEW) {
		if (history_layout) {
			elm_object_part_content_set(m_main_layout, "elm.swallow.content1", history_layout);
			evas_object_show(history_layout);
		}

		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,content1,signal", "");

		_set_navigationbar_title(BR_STRING_HISTORY);

		if (elm_genlist_decorate_mode_get(m_data_manager->get_history_layout()->m_history_genlist))	 {
			if (!_set_controlbar_type(HISTORY_VIEW_EDIT_MODE))
				BROWSER_LOGE("_set_controlbar_type HISTORY_VIEW_EDIT_MODE failed");
		} else {
			if (!_set_controlbar_type(HISTORY_VIEW_DEFAULT))
				BROWSER_LOGE("_set_controlbar_type HISTORY_VIEW_DEFAULT failed");
		}
	} else if (mode == BOOKMARK_VIEW){
		if (history_layout)
			evas_object_hide(history_layout);
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,content0,signal", "");

		if (m_current_folder_id == BROWSER_BOOKMARK_MAIN_FOLDER_ID)
			_set_navigationbar_title(BR_STRING_BOOKMARKS);
		else {
			_set_navigationbar_title(m_current_bookmark_navigationbar_title.c_str());
		}

		if (elm_genlist_decorate_mode_get(_get_current_folder_genlist())) {
			if (!_set_controlbar_type(BOOKMARK_VIEW_EDIT_MODE))
				BROWSER_LOGE("_set_controlbar_type BOOKMARK_VIEW_EDIT_MODE failed");
		} else {
			if (!_set_controlbar_type(BOOKMARK_VIEW_DEFAULT))
				BROWSER_LOGE("_set_controlbar_type BOOKMARK_VIEW_DEFAULT failed");
		}
	}
}

void Browser_Bookmark_View::__controlbar_tab_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;
	Elm_Object_Item *selected_item = elm_toolbar_selected_item_get(obj);
	Elm_Object_Item *history_item = elm_toolbar_last_item_get(obj);
	if (!selected_item) {
		BROWSER_LOGD("no item selected");
		return;
	}

	if (selected_item == history_item) {
		BROWSER_LOGD("tab history");
		/* history selected */
		if (!m_data_manager->get_history_layout()) {
			if (!m_data_manager->create_history_layout()) {
				BROWSER_LOGE("m_data_manager->create_history_layout() failed");
				return;
			}

			if (!m_data_manager->get_history_layout()->init()) {
				BROWSER_LOGE("get_history_layout()->init() failed");
				return;
			}
		}

		bookmark_view->hide_notify_popup_layout(bookmark_view->m_sub_main_layout);
		bookmark_view->_set_view_mode(HISTORY_VIEW);

	} else {
		BROWSER_LOGD("tab bookmark");
		if (m_data_manager->get_history_layout()) {
			m_data_manager->get_history_layout()->hide_notify_popup_layout(
				m_data_manager->get_history_layout()->m_sub_main_history_layout);
			bookmark_view->_set_view_mode(BOOKMARK_VIEW);
		}
	}
}

/* bookmark -> edit(left option button -> edit button of each bookmark item. */
void Browser_Bookmark_View::__edit_bookmark_item_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_Bookmark_DB::bookmark_item *item = (Browser_Bookmark_DB::bookmark_item *)data;
	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)(item->user_data_1);

	/* Pass the selected genlist item as parameter when bookmark -> Edit item
	    because of increase performance. */
	bookmark_view->m_current_genlist_item_to_edit = (Elm_Object_Item *)(item->user_data_2);

	elm_check_state_set(bookmark_view->m_edit_mode_select_all_check_button, EINA_FALSE);

	bookmark_view->hide_notify_popup_layout(bookmark_view->m_sub_main_layout);

	bookmark_view->_set_edit_mode(EINA_FALSE);

	if (!m_data_manager->create_edit_bookmark_view(item->title, item->url,
					bookmark_view->m_current_folder_id)) {
		BROWSER_LOGE("create_edit_bookmark_view failed");
		return;
	}

	if (!m_data_manager->get_edit_bookmark_view()->init()) {
		BROWSER_LOGE("get_edit_bookmark_view()->init() failed");
		m_data_manager->destroy_edit_bookmark_view();
		return;
	}
}

void Browser_Bookmark_View::__rename_folder_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_Bookmark_DB::bookmark_item *item = (Browser_Bookmark_DB::bookmark_item *)data;
	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)(item->user_data_1);
	Elm_Object_Item *it = (Elm_Object_Item *)(item->user_data_2);
	elm_genlist_item_flip_set(it, EINA_TRUE);

	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	evas_object_data_set(bookmark_view->_get_current_folder_genlist(), "selected_it", it);
}

void Browser_Bookmark_View::__rename_folder_entry_enter_key_cb(void *data, Evas_Object *obj,
											void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Evas_Object *rename_edit_field = (Evas_Object *)data;
	elm_object_focus_set(rename_edit_field, EINA_FALSE);
}

void Browser_Bookmark_View::__rename_folder_unfocus_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Bookmark_DB::bookmark_item *item = (Browser_Bookmark_DB::bookmark_item *)data;
	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)item->user_data_1;

	Evas_Object *genlist = bookmark_view->_get_current_folder_genlist();

	Evas_Object *edit_field_entry = obj;
	char *markup_text = elm_entry_markup_to_utf8(elm_entry_entry_get(edit_field_entry));
	char *text = bookmark_view->_trim(markup_text);
	if (!text) {
		BROWSER_LOGE("elm_entry_markup_to_utf8 failed");
		return;
	}
	Elm_Object_Item *it = (Elm_Object_Item *)evas_object_data_get(genlist, "selected_it");

	Eina_Bool only_has_space = EINA_FALSE;
	int space_count = 0;
	if (text && strlen(text)) {
		for (int i = 0 ; i < strlen(text) ; i++) {
			if (text[i] == ' ')
				space_count++;
		}
		if (space_count == strlen(text))
			only_has_space = EINA_TRUE;
	}

	if (!text || strlen(text) == 0 || only_has_space) {
		bookmark_view->show_msg_popup(BR_STRING_ENTER_FOLDER_NAME);
		elm_entry_entry_set(edit_field_entry, item->title.c_str());
	} else {
		if (strcmp(text, item->title.c_str())) {
			if (!m_data_manager->get_bookmark_db()->is_duplicated(text)) {
				item->title = text;
				m_data_manager->get_bookmark_db()->modify_bookmark_title(item->id, text);
			} else {
				if (elm_genlist_decorate_mode_get(bookmark_view->_get_current_folder_genlist()))
					bookmark_view->show_msg_popup(BR_STRING_ALREADY_EXISTS);

				std::string folder_name = m_data_manager->get_bookmark_db()->get_folder_name_by_id(item->id);
				elm_entry_entry_set(edit_field_entry, folder_name.c_str());
			}
		}
	}
	elm_genlist_item_flip_set(it, EINA_FALSE);

	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DEFAULT);

	bookmark_view->m_rename_edit_field = NULL;

	free(markup_text);
}

void Browser_Bookmark_View::__drag_genlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
}

#if defined(GENLIST_SWEEP)
void Browser_Bookmark_View::__sweep_right_genlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;

	if (elm_genlist_decorate_mode_get(obj))
		return;
	else {
		elm_genlist_item_decorate_mode_set((Elm_Object_Item *)event_info, "slide", EINA_TRUE);
		elm_genlist_item_select_mode_set((Elm_Object_Item *)event_info, ELM_OBJECT_SELECT_MODE_NONE);
	}

	if (bookmark_view->m_current_sweep_item
	    && (Elm_Object_Item *)event_info != bookmark_view->m_current_sweep_item) {
		elm_genlist_item_select_mode_set(bookmark_view->m_current_sweep_item, ELM_OBJECT_SELECT_MODE_DEFAULT);
		bookmark_view->m_current_sweep_item = (Elm_Object_Item *)event_info;
	}
}

void Browser_Bookmark_View::__sweep_cancel_genlist_cb(void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!data)
		return;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;
	Elm_Object_Item *it = (Elm_Object_Item*)elm_genlist_decorated_item_get(obj);
	if (it) {
		elm_genlist_item_decorate_mode_set(it, "slide", EINA_FALSE);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DEFAULT);

		if (it == bookmark_view->m_current_sweep_item)
			bookmark_view->m_current_sweep_item = NULL;
	}
}

void Browser_Bookmark_View::__sweep_left_genlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;

	if (elm_genlist_decorate_mode_get(obj))
		return;
	else {
		elm_genlist_item_decorate_mode_set((Elm_Object_Item *)event_info, "slide", EINA_FALSE);
		elm_genlist_item_select_mode_set((Elm_Object_Item *)event_info, ELM_OBJECT_SELECT_MODE_DEFAULT);
	}

	bookmark_view->m_current_sweep_item = NULL;
}
#endif

char *Browser_Bookmark_View::__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part=%s", part);
	if (!data)
		return NULL;

	Browser_Bookmark_DB::bookmark_item *item = (Browser_Bookmark_DB::bookmark_item *)data;

	const char *title = item->title.c_str();
	BROWSER_LOGD("title(%p)=%s", title, title);
	const char *url = item->url.c_str();
	BROWSER_LOGD("url(%p)=%s", url, url);

	if (part && strlen(part) > 0) {
		if (!strncmp(part,"elm.text", strlen("elm.text"))
		    || !strncmp(part, "elm.base.text", strlen("elm.base.text"))
		    || !strncmp(part, "elm.slide.text.1", strlen("elm.slide.text.1"))) {
		    	if (title && strlen(title))
			    	return strdup(title);
			else
				return NULL;
		} else if (!strncmp(part, "elm.text.sub", strlen("elm.text.sub"))) {
			if (url && strlen(url))
				return strdup(url);
			else
				return NULL;
		}
	}

	return NULL;
}

Evas_Object *Browser_Bookmark_View::__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Bookmark_DB::bookmark_item *item = (Browser_Bookmark_DB::bookmark_item *)data;
	if (!data)
		return NULL;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)item->user_data_1;

	if (part && strlen(part) > 0) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			if (item->is_folder) {
				Evas_Object *folder_icon = elm_icon_add(obj);
				if (!elm_icon_file_set(folder_icon, BROWSER_IMAGE_DIR"/folder.png", NULL)) {
					BROWSER_LOGE("elm_icon_file_set is failed.\n");
					return NULL;
				}
				evas_object_size_hint_aspect_set(folder_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
				return folder_icon;
			} else {
				Evas_Object *favicon = NULL;
				favicon = m_data_manager->get_browser_view()->get_favicon(item->url.c_str());
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
			}
		} else if (!strncmp(part, "elm.slide.swallow.1", strlen("elm.slide.swallow.1"))) {
			Evas_Object *edit_button = elm_button_add(obj);
			if (!edit_button) {
				BROWSER_LOGE("elm_button_add is failed.\n");
				return NULL;
			}
			elm_object_style_set(edit_button, "text_only/sweep");
			elm_object_text_set(edit_button, BR_STRING_EDIT);
			if (!item->is_editable)
				elm_object_disabled_set(edit_button, EINA_TRUE);
			evas_object_smart_callback_add(edit_button, "clicked", __slide_edit_button_clicked_cb, item);
			return edit_button;
		} else if (!strncmp(part, "elm.slide.swallow.2", strlen("elm.slide.swallow.2"))) {
			Evas_Object *delete_button = elm_button_add(obj);
			if (!delete_button) {
				BROWSER_LOGE("elm_button_add is failed.\n");
				return NULL;
			}
			elm_object_style_set(delete_button, "text_only/sweep");
			elm_object_text_set(delete_button, BR_STRING_DELETE);
			if (!item->is_editable)
				elm_object_disabled_set(delete_button, EINA_TRUE);
			evas_object_smart_callback_add(delete_button, "clicked", __slide_delete_button_clicked_cb, item);
			return delete_button;
		}

		if (elm_genlist_decorate_mode_get(obj)) {
			if (!strncmp(part, "elm.edit.icon.1", strlen("elm.edit.icon.1"))) {
				Evas_Object *check_button = elm_check_add(obj);
				if (!check_button) {
					BROWSER_LOGE("elm_check_add is failed.\n");
					return NULL;
				}
				evas_object_propagate_events_set(check_button, EINA_FALSE);

				if (!item->is_editable)
					elm_object_disabled_set(check_button, EINA_TRUE);

				elm_check_state_pointer_set(check_button, &item->is_delete);
				evas_object_smart_callback_add(check_button, "changed",
								__edit_mode_item_check_changed_cb, item);
				return check_button;
			} else if (!strncmp(part, "elm.edit.icon.2", strlen("elm.edit.icon.2"))) {
				Evas_Object *button = elm_button_add(obj);
				if (!button) {
					BROWSER_LOGE("elm_button_add is failed.\n");
					return NULL;
				}
				evas_object_propagate_events_set(button, EINA_FALSE);

				if (!item->is_editable)
					elm_object_disabled_set(button, EINA_TRUE);
				if (item->is_folder) {
					elm_object_style_set(button, "rename");
					evas_object_smart_callback_add(button, "clicked",
								__rename_folder_button_clicked_cb, item);
				} else {
					elm_object_style_set(button, "reveal");
					evas_object_smart_callback_add(button, "clicked",
								__edit_bookmark_item_button_clicked_cb, item);
				}
				return button;
			} else if (!strncmp(part, "elm.flip.content", strlen("elm.flip.content"))) {
				Evas_Object *edit_field = br_elm_editfield_add(obj);
				br_elm_editfield_entry_single_line_set(edit_field, EINA_TRUE);
				br_elm_editfield_eraser_set(edit_field, EINA_TRUE);

				bookmark_view->m_rename_edit_field = edit_field;
				evas_object_data_set(bookmark_view->m_rename_edit_field, "item", item);

				Evas_Object *edit_field_entry = br_elm_editfield_entry_get(edit_field);
				elm_entry_entry_set(edit_field_entry, item->title.c_str());
				elm_entry_cursor_end_set(edit_field_entry);

				evas_object_smart_callback_add(edit_field_entry, "unfocused",
								__rename_folder_unfocus_cb, data);
				evas_object_smart_callback_add(edit_field_entry, "activated",
								__rename_folder_entry_enter_key_cb, edit_field);

				ecore_idler_add(__set_focus_editfield_idler_cb, edit_field);

				return edit_field;
			}
		}
	}

	return NULL;
}

Eina_Bool Browser_Bookmark_View::_is_empty_folder(string folder_name)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Bookmark_DB *bookmark_db = m_data_manager->get_bookmark_db();
	int folder_id;
	Eina_Bool ret = bookmark_db->get_folder_id(BROWSER_BOOKMARK_MAIN_FOLDER_ID, folder_name.c_str(), &folder_id);
	if (!ret) {
		BROWSER_LOGE("get_folder_id is failed.");
		return EINA_FALSE;
	}

	vector<Browser_Bookmark_DB::bookmark_item *> bookmark_list;
	ret = bookmark_db->get_bookmark_list(bookmark_list, folder_id);
	if (!ret) {
		BROWSER_LOGE("get_bookmark_list failed");
		return EINA_FALSE;
	}

	int count = bookmark_list.size();

	for(int i = 0 ; i < bookmark_list.size() ; i++ ) {
		if (bookmark_list[i])
			delete bookmark_list[i];
	}
	bookmark_list.clear();

	if(count)
		return EINA_FALSE;
	else
		return EINA_TRUE;
}

void Browser_Bookmark_View::__slide_edit_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Bookmark_DB::bookmark_item *item = (Browser_Bookmark_DB::bookmark_item *)data;
	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)(item->user_data_1);
	bookmark_view->m_current_genlist_item_to_edit = (Elm_Object_Item *)(item->user_data_2);

	if (item->is_folder) {
		/* edit folder by slide genlist item */
		if (bookmark_view->_is_empty_folder(item->title)) {
			bookmark_view->show_msg_popup(BR_STRING_EMPTY);
			return;
		}

		bookmark_view->_go_to_sub_foler(item->title.c_str());

		bookmark_view->_set_edit_mode(EINA_TRUE);

		Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
		elm_object_item_signal_emit(top_it, ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_INSTANT_OPEN);
		evas_object_data_set(m_navi_bar, "visible", (void *)EINA_TRUE);

		edje_object_signal_emit(elm_layout_edje_get(bookmark_view->m_main_layout),
					"expand_header_no_animation,signal", "");
	} else {
		/* edit bookmark item by slide genlist item */
		/* Pass the selected genlist item as parameter when bookmark -> Edit item
		    because of increase performance. */
		if (!m_data_manager->create_edit_bookmark_view(item->title, item->url,
							bookmark_view->m_current_folder_id)) {
			BROWSER_LOGE("create_edit_bookmark_view failed");
			return;
		}

		if (!m_data_manager->get_edit_bookmark_view()->init()) {
			BROWSER_LOGE("new get_edit_bookmark_view()->init() failed");
			m_data_manager->destroy_edit_bookmark_view();
			return;
		}
	}

	elm_genlist_item_decorate_mode_set(bookmark_view->m_current_genlist_item_to_edit, "slide", EINA_FALSE);
	elm_genlist_item_select_mode_set(bookmark_view->m_current_genlist_item_to_edit, ELM_OBJECT_SELECT_MODE_DEFAULT);
}

void Browser_Bookmark_View::__delete_confirm_response_by_slide_button_cb(void *data, Evas_Object *obj,
								void *event_info)
{
	BROWSER_LOGD("event_info = %d", (int)event_info);

	if (!data)
		return;

	Browser_Bookmark_DB::bookmark_item *item = (Browser_Bookmark_DB::bookmark_item *)data;
	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)(item->user_data_1);

	evas_object_del(bookmark_view->m_delete_confirm_popup);

	bookmark_view->_delete_bookmark_item_by_slide_button(item);
}

void Browser_Bookmark_View::__cancel_confirm_response_by_slide_button_cb(void *data, Evas_Object *obj,
								void *event_info)
{
	BROWSER_LOGD("event_info = %d", (int)event_info);

	if (!data)
		return;

	Evas_Object *popup = (Evas_Object *)data;

	evas_object_del(popup);
}

void Browser_Bookmark_View::__delete_confirm_response_by_edit_mode_cb(void *data, Evas_Object *obj,
								void *event_info)
{
	BROWSER_LOGD("event_info = %d", (int)event_info);

	if (!data)
		return;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;

	if (bookmark_view->m_delete_confirm_popup) {
		evas_object_del(bookmark_view->m_delete_confirm_popup);
		bookmark_view->m_delete_confirm_popup = NULL;
	}

	bookmark_view->_delete_selected_bookmark();

	bookmark_view->_set_edit_mode(EINA_FALSE);
}

Evas_Object *Browser_Bookmark_View::_show_delete_confirm_popup(void)
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

void Browser_Bookmark_View::_delete_bookmark_item_by_slide_button(Browser_Bookmark_DB::bookmark_item *item)
{
	BROWSER_LOGD("[%s]", __func__);
	Elm_Object_Item *it = (Elm_Object_Item *)(item->user_data_2);
	int ret = EINA_TRUE;

	if (item->is_folder) {
		int folder_id = 0;
		ret = m_data_manager->get_bookmark_db()->get_folder_id(m_current_folder_id,
									item->title.c_str(), &folder_id);
		if (!ret) {
			BROWSER_LOGE("bookmark_db->get_folder_id failed");
			return;
		}
		ret = m_data_manager->get_bookmark_db()->delete_folder(folder_id);
		if (!ret) {
			BROWSER_LOGE("bookmark_db->delete_folder failed");
			return;
		}
	} else {
		ret = m_data_manager->get_bookmark_db()->delete_bookmark(item->id);
		if (!ret) {
			BROWSER_LOGE("bookmark_db->delete_bookmark failed");
			return;
		}
	}

	_delete_bookmark_item_from_folder_list(item);

	elm_object_item_del(it);

	if (_get_current_folder_item_list().size() == 0) {
		_show_empty_content_layout(EINA_TRUE);
	}

	show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);
}

void Browser_Bookmark_View::__slide_share_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Bookmark_DB::bookmark_item *item = (Browser_Bookmark_DB::bookmark_item *)data;
	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)(item->user_data_1);
	if (!bookmark_view->_show_share_popup(item->url.c_str()))
		BROWSER_LOGE("_show_share_popup failed");
}

void Browser_Bookmark_View::__slide_delete_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Bookmark_DB::bookmark_item *item = (Browser_Bookmark_DB::bookmark_item *)data;
	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)(item->user_data_1);

	Evas_Object *ok_button = bookmark_view->_show_delete_confirm_popup();
	if (ok_button)
		evas_object_smart_callback_add(ok_button, "clicked",
			__delete_confirm_response_by_slide_button_cb, item);
}

void Browser_Bookmark_View::__genlist_move_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	BROWSER_LOGD("moved it = %d", it);

	Browser_Bookmark_DB::bookmark_item *item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
	Elm_Object_Item *next_it = elm_genlist_item_next_get(it);
	Browser_Bookmark_DB::bookmark_item *next_item = NULL;

	if (next_it)
		next_item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(next_it);

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)(item->user_data_1);

	Elm_Object_Item *index_it = NULL;
	Browser_Bookmark_DB::bookmark_item *test_item = NULL;
	it = elm_genlist_first_item_get(obj);
	while (index_it) {
		test_item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(index_it);
		BROWSER_LOGD("title=%s, order index=%d", test_item->title.c_str(), test_item->order_index);
		index_it = elm_genlist_item_next_get(index_it);
	}

	if (next_item && (item->order_index < next_item->order_index))
		/* move down */
		bookmark_view->_reorder_bookmark_item(item->order_index, EINA_TRUE);
	else
		/* move up */
		bookmark_view->_reorder_bookmark_item(item->order_index, EINA_FALSE);
}

void Browser_Bookmark_View::_reorder_bookmark_item(int order_index, Eina_Bool is_move_down)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Bookmark_DB *bookmark_db = m_data_manager->get_bookmark_db();

	Elm_Object_Item *it = NULL;
	Elm_Object_Item *next_it = NULL;
	Browser_Bookmark_DB::bookmark_item *item = NULL;
	Browser_Bookmark_DB::bookmark_item *next_item = NULL;
	Eina_Bool ret = EINA_TRUE;
	int index = order_index;

	if (is_move_down) {
		it = elm_genlist_first_item_get(_get_current_folder_genlist());
		while(it) {
			item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
			if (item->order_index > index) {
				ret = bookmark_db->modify_bookmark_order_index(item->id, m_current_folder_id, index);
				if (!ret)
					BROWSER_LOGD("modify_bookmark_order_index failed");
				int temp = item->order_index;
				item->order_index = index;
				index = temp;
				elm_object_item_data_set(it, item);
			} else if (item->order_index == order_index) {
				ret = bookmark_db->modify_bookmark_order_index(item->id, m_current_folder_id, index);
				item->order_index = index;
				elm_object_item_data_set(it, item);
				if (!ret)
					BROWSER_LOGD("modify_bookmark_order_index failed");
				break;
			}
			it = elm_genlist_item_next_get(it);
		}
	} else {
		index = order_index;
		it = elm_genlist_last_item_get(_get_current_folder_genlist());
		while(it) {
			item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
			if (item->order_index < index) {
				ret = bookmark_db->modify_bookmark_order_index(item->id, m_current_folder_id, index);
				if (!ret)
					BROWSER_LOGD("modify_bookmark_order_index failed");
				int temp = item->order_index;
				item->order_index = index;
				index = temp;
				elm_object_item_data_set(it, item);
			} else if (item->order_index == order_index) {
				ret = bookmark_db->modify_bookmark_order_index(item->id, m_current_folder_id, index);
				item->order_index = index;
				elm_object_item_data_set(it, item);
				if (!ret)
					BROWSER_LOGD("modify_bookmark_order_index failed");
				break;
			}
			it = elm_genlist_item_prev_get(it);
		}
	}
}

void Browser_Bookmark_View::history_item_clicked(const char *url)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_data_manager->get_browser_view())
		m_data_manager->get_browser_view()->load_url(url);

	if (elm_naviframe_bottom_item_get(m_navi_bar) != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);
}

void Browser_Bookmark_View::__bookmark_item_clicked_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;
	if (!data)
		return;

	Elm_Object_Item *seleted_item = elm_genlist_selected_item_get(obj);
	Browser_Bookmark_DB::bookmark_item *item = NULL;
	item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(seleted_item);

	if (!elm_genlist_decorate_mode_get(obj)) {
		if (!item->is_folder) {
			std::string url = std::string(item->url.c_str());

			if (m_data_manager->get_browser_view())
				m_data_manager->get_browser_view()->load_url(url.c_str());

			if (elm_naviframe_bottom_item_get(bookmark_view->m_navi_bar)
			    != elm_naviframe_top_item_get(bookmark_view->m_navi_bar))
				elm_naviframe_item_pop(bookmark_view->m_navi_bar);
		} else
			bookmark_view->_go_to_sub_foler(item->title.c_str());
	} else {
		Elm_Object_Item *it = elm_genlist_selected_item_get(obj);
		Browser_Bookmark_DB::bookmark_item *item = NULL;
		item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
		if (item && item->is_editable) {
			item->is_delete = !(item->is_delete);
			elm_genlist_item_update(it);
		}

		int delete_item_count = 0;
		int editable_item_count = 0;
		it = elm_genlist_first_item_get(bookmark_view->_get_current_folder_genlist());
		while(it) {
			if (item->is_editable)
				editable_item_count++;
			item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
			if (item->is_delete)
				delete_item_count++;
			it = elm_genlist_item_next_get(it);
		}

		if (editable_item_count == delete_item_count)
			elm_check_state_set(bookmark_view->m_edit_mode_select_all_check_button, EINA_TRUE);
		else
			elm_check_state_set(bookmark_view->m_edit_mode_select_all_check_button, EINA_FALSE);

		bookmark_view->_show_selection_info();
	}

	elm_genlist_item_selected_set(seleted_item, EINA_FALSE);
}

void Browser_Bookmark_View::__upper_folder_clicked_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;
	bookmark_view->_go_up_to_main_folder();

	bookmark_view->_set_navigationbar_title(BR_STRING_BOOKMARKS);
}

Evas_Object *Browser_Bookmark_View::_get_current_folder_genlist(void)
{
	if (m_current_folder_id == BROWSER_BOOKMARK_MAIN_FOLDER_ID)
		return m_main_folder_genlist;
	else
		return m_sub_folder_genlist;
}

/* This just returns the copy of item list instance.
  * Just refer to read. Do not modify(insert or delete) the contents of this return value.
  * It doesn't modify the actual m_main_folder_list, m_sub_folder_list contents. */
vector<Browser_Bookmark_DB::bookmark_item *> Browser_Bookmark_View::_get_current_folder_item_list(void)
{
	if (m_current_folder_id == BROWSER_BOOKMARK_MAIN_FOLDER_ID)
		return m_main_folder_list;
	else
		return m_sub_folder_list;
}

void Browser_Bookmark_View::_show_empty_content_layout(Eina_Bool is_empty)
{
	BROWSER_LOGD("is_empty=%d", is_empty);
	if (m_view_mode == BOOKMARK_VIEW) {
		if (!_set_controlbar_type(BOOKMARK_VIEW_DEFAULT))
			BROWSER_LOGE("_set_controlbar_type(BOOKMARK_VIEW_DEFAULT) failed");
	}

	if (is_empty) {
		if (m_edit_mode_select_all_check_button) {
			evas_object_del(m_edit_mode_select_all_check_button);
			m_edit_mode_select_all_check_button = NULL;
		}
		if (m_edit_mode_select_all_layout) {
			elm_box_unpack(m_genlist_content_box, m_edit_mode_select_all_layout);
			evas_object_del(m_edit_mode_select_all_layout);
			m_edit_mode_select_all_layout = NULL;
		}

		if (m_no_contents) {
			evas_object_del(m_no_contents);
			m_no_contents = NULL;
		}

		m_no_contents = elm_layout_add(m_main_layout);
		if (!m_no_contents) {
			BROWSER_LOGE("elm_layout_add failed");
		}
		elm_layout_theme_set(m_no_contents, "layout", "no_bookmark", "full");
		elm_object_part_text_set (m_no_contents, "elm.text", BR_STRING_NO_BOOKMARKS);
		evas_object_size_hint_weight_set(m_no_contents, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(m_no_contents, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_part_content_set(m_genlist_content_layout, "elm.swallow.no_content", m_no_contents);
		edje_object_signal_emit(elm_layout_edje_get(m_genlist_content_layout), "hide,content_box,signal", "");
		evas_object_show(m_no_contents);

		elm_object_item_disabled_set(m_bookmark_edit_controlbar_item, EINA_TRUE);
	}else {
		if (m_no_contents) {
			elm_object_part_content_unset(m_genlist_content_layout, "elm.swallow.no_content");
			evas_object_del(m_no_contents);
			m_no_contents = NULL;
			edje_object_signal_emit(elm_layout_edje_get(m_genlist_content_layout),
						"show,content_box,signal", "");
		}

		elm_object_item_disabled_set(m_bookmark_edit_controlbar_item, EINA_FALSE);
	}
}

void Browser_Bookmark_View::_go_up_to_main_folder(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_current_folder_id = BROWSER_BOOKMARK_MAIN_FOLDER_ID;

	elm_box_unpack(m_genlist_content_box, m_sub_folder_conformant);
	elm_box_pack_end(m_genlist_content_box, m_conformant);
	evas_object_show(m_conformant);

	edje_object_signal_emit(elm_layout_edje_get(m_genlist_content_layout), "hide,upper_folder,signal", "");

	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
	elm_object_item_signal_emit(top_it, ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_INSTANT_OPEN);
	evas_object_data_set(m_navi_bar, "visible", (void *)EINA_TRUE);

	for(int i = 0 ; i < m_sub_folder_list.size() ; i++) {
		if (m_sub_folder_list[i]) {
			delete m_sub_folder_list[i];
			m_sub_folder_list.erase(m_sub_folder_list.begin() + i);
		}
	}
	m_sub_folder_list.clear();

	if (m_upper_folder_icon) {
		evas_object_del(m_upper_folder_icon);
		m_upper_folder_icon = NULL;
	}
	if (m_upper_folder_list) {
		evas_object_del(m_upper_folder_list);
		m_upper_folder_list = NULL;
	}
	if (m_sub_folder_genlist) {
		evas_object_del(m_sub_folder_genlist);
		m_sub_folder_genlist = NULL;
	}
	if (m_sub_folder_conformant) {
		evas_object_del(m_sub_folder_conformant);
		m_sub_folder_conformant = NULL;
	}

	if (m_main_folder_list.size() == 0)
		_show_empty_content_layout(EINA_TRUE);
	else
		_show_empty_content_layout(EINA_FALSE);

	elm_object_item_disabled_set(m_create_folder_controlbar_item, EINA_FALSE);
}

void Browser_Bookmark_View::_go_to_sub_foler(const char *folder_name)
{
	int folder_id;
	string navigationbar_title = folder_name;
	Eina_Bool ret = m_data_manager->get_bookmark_db()->get_folder_id(BROWSER_BOOKMARK_MAIN_FOLDER_ID,
									folder_name, &folder_id);
	if (!ret) {
		BROWSER_LOGE("get_folder_id is failed.");
		return;
	}

	m_sub_folder_genlist = _create_sub_folder_genlist(folder_id);
	if (!m_sub_folder_genlist) {
		BROWSER_LOGD("_create_sub_folder_genlist failed");
		return;
	}

	m_current_bookmark_navigationbar_title = navigationbar_title.c_str();
	_set_navigationbar_title(navigationbar_title.c_str());

	elm_object_item_disabled_set(m_create_folder_controlbar_item, EINA_TRUE);

	if (!m_sub_folder_list.size())
		elm_object_item_disabled_set(m_bookmark_edit_controlbar_item, EINA_TRUE);
}

void Browser_Bookmark_View::_set_navigationbar_title(const char *title)
{
	BROWSER_LOGD("title=[%s]", title);
	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
	if (elm_object_item_content_get(top_it) == m_main_layout)
		elm_object_item_text_set(top_it, title);
}

Evas_Object *Browser_Bookmark_View::_create_sub_folder_genlist(int folder_id)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *genlist = elm_genlist_add(m_main_layout);
	if (genlist) {
		elm_box_unpack(m_genlist_content_box, m_conformant);
		evas_object_hide(m_conformant);

		m_sub_folder_conformant = elm_conformant_add(m_main_layout);
		if (!m_sub_folder_conformant) {
			BROWSER_LOGE("elm_conformant_add failed");
			return EINA_FALSE;
		}
		elm_object_style_set(m_sub_folder_conformant, "internal_layout");
		evas_object_size_hint_weight_set(m_sub_folder_conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(m_sub_folder_conformant, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(m_sub_folder_conformant);
		elm_box_pack_end(m_genlist_content_box, m_sub_folder_conformant);

		m_current_folder_id = folder_id;

		evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_genlist_block_count_set(genlist, BROWSER_BOOKMARK_GENLIST_BLOCK_COUNT);
		elm_genlist_homogeneous_set(genlist, EINA_TRUE);

#if defined(GENLIST_SWEEP)
		evas_object_smart_callback_add(genlist, "drag,start,up", __sweep_cancel_genlist_cb, genlist);
		evas_object_smart_callback_add(genlist, "drag,start,down", __sweep_cancel_genlist_cb, genlist);
		evas_object_smart_callback_add(genlist, "drag,start,right", __sweep_right_genlist_cb, this);
		evas_object_smart_callback_add(genlist, "drag,start,left", __sweep_left_genlist_cb, this);
#endif

		evas_object_smart_callback_add(genlist, "moved", __genlist_move_cb, this);

		elm_object_content_set(m_sub_folder_conformant, genlist);
		evas_object_show(genlist);

		m_upper_folder_list = elm_list_add(m_main_layout);
		if (m_upper_folder_list) {
			elm_list_bounce_set(m_upper_folder_list,  EINA_FALSE, EINA_FALSE);
			evas_object_size_hint_weight_set(m_upper_folder_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(m_upper_folder_list, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_show(m_upper_folder_list);
			elm_object_part_content_set(m_genlist_content_layout, "elm.swallow.upperlist", m_upper_folder_list);
			m_upper_folder_icon = elm_icon_add(m_upper_folder_list);
			if (!m_upper_folder_icon) {
				BROWSER_LOGE("elm_icon_add failed");
				return NULL;
			}
			if (!elm_icon_file_set(m_upper_folder_icon, BROWSER_IMAGE_DIR"/I01_icon_upper_folder.png", NULL))
				BROWSER_LOGE("elm_icon_file_set is failed. [%s]");

			evas_object_size_hint_min_set(m_upper_folder_icon, UPPER_FOLDER_ICON_SIZE, UPPER_FOLDER_ICON_SIZE);
			evas_object_resize(m_upper_folder_icon, UPPER_FOLDER_ICON_SIZE, UPPER_FOLDER_ICON_SIZE);
			elm_list_item_append(m_upper_folder_list, BR_STRING_BOOKMARKS, m_upper_folder_icon,
						NULL, __upper_folder_clicked_cb, this);
			evas_object_show(m_upper_folder_icon);
			edje_object_signal_emit(elm_layout_edje_get(m_genlist_content_layout), "show,upper_folder,signal", "");

			Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
			elm_object_item_signal_emit(top_it, ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_INSTANT_CLOSE);
			evas_object_data_set(m_navi_bar, "visible", (void *)EINA_FALSE);
		}

		/* Fill bookmark list */
		Eina_Bool ret = m_data_manager->get_bookmark_db()->get_bookmark_list(m_sub_folder_list,
											m_current_folder_id);
		if (!ret) {
			return NULL;
			BROWSER_LOGE("get_bookmark_list failed");
		}

		int size = m_sub_folder_list.size();
		BROWSER_LOGD("bookmark count=%d", size);
		for (int i = 0 ; i < size ; i++ ) {
			m_sub_folder_list[i]->user_data_1 = (void *)this;
			m_sub_folder_list[i]->user_data_2 = (void *)elm_genlist_item_append(genlist,
						&m_bookmark_genlist_item_class, m_sub_folder_list[i], NULL,
						ELM_GENLIST_ITEM_NONE, __bookmark_item_clicked_cb, this);
		}

		if (size == 0)
			_show_empty_content_layout(EINA_TRUE);
		else
			_show_empty_content_layout(EINA_FALSE);
	}

	return genlist;
}

Evas_Object *Browser_Bookmark_View::_create_main_folder_genlist(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *genlist = elm_genlist_add(m_main_layout);
	if (genlist) {
		m_current_folder_id = BROWSER_BOOKMARK_MAIN_FOLDER_ID;

		evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_genlist_block_count_set(genlist, BROWSER_BOOKMARK_GENLIST_BLOCK_COUNT);
		elm_genlist_homogeneous_set(genlist, EINA_TRUE);

#if defined(GENLIST_SWEEP)
		evas_object_smart_callback_add(genlist, "drag,start,up", __sweep_cancel_genlist_cb, genlist);
		evas_object_smart_callback_add(genlist, "drag,start,down", __sweep_cancel_genlist_cb, genlist);
		evas_object_smart_callback_add(genlist, "drag,start,right", __sweep_right_genlist_cb, this);
		evas_object_smart_callback_add(genlist, "drag,start,left", __sweep_left_genlist_cb, this);
#endif
		evas_object_smart_callback_add(genlist, "moved", __genlist_move_cb, this);

		m_bookmark_genlist_item_class.item_style = "1text.1icon.2";
		m_bookmark_genlist_item_class.decorate_item_style = "mode/slide2.text";
		m_bookmark_genlist_item_class.decorate_all_item_style = "edit_default";
		m_bookmark_genlist_item_class.func.text_get = __genlist_label_get_cb;
		m_bookmark_genlist_item_class.func.content_get = __genlist_icon_get_cb;
		m_bookmark_genlist_item_class.func.state_get = NULL;
		m_bookmark_genlist_item_class.func.del = NULL;
//		m_bookmark_genlist_item_class.func.moved = __genlist_move_cb;

		m_folder_genlist_item_class.item_style = "1text.1icon.2";
		m_folder_genlist_item_class.decorate_item_style = "mode/slide2.text";
		m_folder_genlist_item_class.decorate_all_item_style = "edit_default";
		m_folder_genlist_item_class.func.text_get = __genlist_label_get_cb;
		m_folder_genlist_item_class.func.content_get = __genlist_icon_get_cb;
		m_folder_genlist_item_class.func.state_get = NULL;
		m_folder_genlist_item_class.func.del = NULL;
		evas_object_show(genlist);

		/* Fill bookmark list */
		Eina_Bool ret = m_data_manager->get_bookmark_db()->get_bookmark_list(m_main_folder_list,
											m_current_folder_id);
		if (!ret) {
			return NULL;
			BROWSER_LOGE("get_bookmark_list failed");
		}

		int size = m_main_folder_list.size();
		BROWSER_LOGD("bookmark count=%d", size);
		for (int i = 0 ; i < size ; i++ ) {
			m_main_folder_list[i]->user_data_1 = (void *)this;
			if (m_main_folder_list[i]->is_folder) {
				m_main_folder_list[i]->user_data_2 = (void *)elm_genlist_item_append(genlist,
							&m_folder_genlist_item_class, m_main_folder_list[i], NULL,
							ELM_GENLIST_ITEM_NONE, __bookmark_item_clicked_cb, this);

			} else {
				m_main_folder_list[i]->user_data_2 = (void *)elm_genlist_item_append(genlist,
							&m_bookmark_genlist_item_class, m_main_folder_list[i], NULL,
							ELM_GENLIST_ITEM_NONE, __bookmark_item_clicked_cb, this);
			}
		}
	}

	return genlist;
}

void Browser_Bookmark_View::__create_folder_button(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;
	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;

	/* New folder */
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

void Browser_Bookmark_View::__add_to_bookmark_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;
	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;
	if (!m_data_manager->create_add_to_bookmark_view(m_data_manager->get_browser_view()->get_title(),
						m_data_manager->get_browser_view()->get_url(), bookmark_view->m_current_folder_id)) {
		BROWSER_LOGE("m_data_manager->create_add_to_bookmark_view failed");
		return;
	}

	if (!m_data_manager->get_add_to_bookmark_view()->init())
		m_data_manager->destroy_add_to_bookmark_view();
}

Eina_Bool Browser_Bookmark_View::_set_controlbar_type(controlbar_type type)
{
	if (m_bottom_control_bar)
		evas_object_del(m_bottom_control_bar);

	m_bookmark_delete_controlbar_item = NULL;
	m_create_folder_controlbar_item = NULL;

	if (type == BOOKMARK_VIEW_DEFAULT) {
		elm_object_style_set(m_bg, "default");

		Elm_Object_Item *navi_it = elm_naviframe_top_item_get(m_navi_bar);

		m_back_button = elm_button_add(m_main_layout);
		if (!m_back_button) {
			BROWSER_LOGE("elm_button_add failed");
			return EINA_FALSE;
		}
		elm_object_style_set(m_back_button, "browser/bookmark_controlbar_back");
		evas_object_smart_callback_add(m_back_button, "clicked", __back_button_clicked_cb, this);
		evas_object_show(m_back_button);
		elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_PREV_BTN, m_back_button);

		m_bottom_control_bar = elm_toolbar_add(m_main_layout);
		if (!m_bottom_control_bar) {
			BROWSER_LOGE("elm_toolbar_add failed");
			return EINA_TRUE;
		}
		elm_object_style_set(m_bottom_control_bar, "browser/default");
		elm_toolbar_shrink_mode_set(m_bottom_control_bar, ELM_TOOLBAR_SHRINK_EXPAND);

		m_create_folder_controlbar_item = elm_toolbar_item_append(m_bottom_control_bar,
							BROWSER_IMAGE_DIR"/I01_controlbar_icon_create.png",
							NULL, __add_to_bookmark_cb, this);
		m_bookmark_edit_controlbar_item = elm_toolbar_item_append(m_bottom_control_bar,
							BROWSER_IMAGE_DIR"/01_controlbar_icon_compose.png", NULL,
							__edit_controlbar_item_clicked_cb, this);
		m_create_folder_controlbar_item = elm_toolbar_item_append(m_bottom_control_bar,
							BROWSER_IMAGE_DIR"/01_controlbar_icon_create_folder.png",
							NULL, __create_folder_button, this);
		evas_object_show(m_bottom_control_bar);

		if (m_current_folder_id != BROWSER_BOOKMARK_MAIN_FOLDER_ID)
			elm_object_item_disabled_set(m_create_folder_controlbar_item, EINA_TRUE);

		if (!_get_current_folder_item_list().size())
			elm_object_item_disabled_set(m_bookmark_edit_controlbar_item, EINA_TRUE);

		elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_CONTROLBAR, m_bottom_control_bar);
	} else if (type == BOOKMARK_VIEW_EDIT_MODE || type == HISTORY_VIEW_EDIT_MODE) {
		elm_object_style_set(m_bg, "edit_mode");

		Elm_Object_Item *navi_it = elm_naviframe_top_item_get(m_navi_bar);
		elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_PREV_BTN, NULL);

		m_bottom_control_bar = elm_toolbar_add(m_main_layout);
		if (!m_bottom_control_bar) {
			BROWSER_LOGE("elm_toolbar_add failed");
			return EINA_FALSE;
		}
		elm_object_style_set(m_bottom_control_bar, "browser/default");
		elm_toolbar_shrink_mode_set(m_bottom_control_bar, ELM_TOOLBAR_SHRINK_EXPAND);

		Elm_Object_Item *empty_item = elm_toolbar_item_append(m_bottom_control_bar, NULL, NULL, NULL, NULL);
		elm_object_item_disabled_set(empty_item, EINA_TRUE);

		m_bookmark_delete_controlbar_item = elm_toolbar_item_append(m_bottom_control_bar,
									BROWSER_IMAGE_DIR"/01_controlbar_icon_delete.png",
									NULL, __delete_controlbar_item_clicked_cb, this);

		empty_item = elm_toolbar_item_append(m_bottom_control_bar, NULL, NULL, NULL, NULL);
		elm_object_item_disabled_set(empty_item, EINA_TRUE);

		elm_toolbar_item_append(m_bottom_control_bar, BROWSER_IMAGE_DIR"/01_controllbar_icon_close.png", NULL,
										__edit_controlbar_item_clicked_cb, this);

		if (type == BOOKMARK_VIEW_EDIT_MODE) {
			int delete_item_count = 0;
			Elm_Object_Item *it = elm_genlist_first_item_get(_get_current_folder_genlist());
			while(it) {
				Browser_Bookmark_DB::bookmark_item *item = NULL;
				item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(it);
				if (item->is_delete)
					delete_item_count++;
				it = elm_genlist_item_next_get(it);
			}

			if (delete_item_count)
				elm_object_item_disabled_set(m_bookmark_delete_controlbar_item, EINA_FALSE);
			else
				elm_object_item_disabled_set(m_bookmark_delete_controlbar_item, EINA_TRUE);
		} else {
			int delete_item_count = 0;
			Elm_Object_Item *it = elm_genlist_first_item_get(m_data_manager->get_history_layout()->m_history_genlist);
			while(it) {
				Browser_History_DB::history_item *item = NULL;
				item = (Browser_History_DB::history_item *)elm_object_item_data_get(it);

				/* Ignore the date label. */
				if (elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
					if (item->is_delete)
						delete_item_count++;
				}
				it = elm_genlist_item_next_get(it);
			}
			if (delete_item_count)
				elm_object_item_disabled_set(m_bookmark_delete_controlbar_item, EINA_FALSE);
			else
				elm_object_item_disabled_set(m_bookmark_delete_controlbar_item, EINA_TRUE);
		}

		evas_object_show(m_bottom_control_bar);
		elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_CONTROLBAR, m_bottom_control_bar);
	} else if (type == HISTORY_VIEW_DEFAULT) {
		elm_object_style_set(m_bg, "default");

		Elm_Object_Item *navi_it = elm_naviframe_top_item_get(m_navi_bar);

		m_back_button = elm_button_add(m_main_layout);
		if (!m_back_button) {
			BROWSER_LOGE("elm_button_add failed");
			return EINA_FALSE;
		}
		elm_object_style_set(m_back_button, "browser/bookmark_controlbar_back");
		evas_object_smart_callback_add(m_back_button, "clicked", __back_button_clicked_cb, this);
		evas_object_show(m_back_button);
		elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_PREV_BTN, m_back_button);

		m_bottom_control_bar = elm_toolbar_add(m_main_layout);
		if (!m_bottom_control_bar) {
			BROWSER_LOGE("elm_toolbar_add failed");
			return EINA_TRUE;
		}
		elm_object_style_set(m_bottom_control_bar, "browser/default");
		elm_toolbar_shrink_mode_set(m_bottom_control_bar, ELM_TOOLBAR_SHRINK_EXPAND);

		Elm_Object_Item *empty_item = elm_toolbar_item_append(m_bottom_control_bar, NULL, NULL, NULL, NULL);
		elm_object_item_disabled_set(empty_item, EINA_TRUE);

		m_bookmark_edit_controlbar_item = elm_toolbar_item_append(m_bottom_control_bar,
								BROWSER_IMAGE_DIR"/01_controlbar_icon_compose.png", NULL,
								__edit_controlbar_item_clicked_cb, this);

		empty_item = elm_toolbar_item_append(m_bottom_control_bar, NULL, NULL, NULL, NULL);
		elm_object_item_disabled_set(empty_item, EINA_TRUE);

		Browser_History_Layout *history_layout = m_data_manager->get_history_layout();
		if (history_layout) {
			if (!history_layout->m_history_list.size())
				elm_object_item_disabled_set(m_bookmark_edit_controlbar_item, EINA_TRUE);
			else
				elm_object_item_disabled_set(m_bookmark_edit_controlbar_item, EINA_FALSE);
		}

		evas_object_show(m_bottom_control_bar);

		elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_CONTROLBAR, m_bottom_control_bar);
	}

	elm_object_focus_allow_set(m_bottom_control_bar, EINA_FALSE);

	return EINA_TRUE;
}

void Browser_Bookmark_View::__naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Bookmark_View *bookmark_view = (Browser_Bookmark_View *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	if (bookmark_view->m_navi_it != elm_naviframe_top_item_get(m_navi_bar))
		return;

	/* Bookmark view -> Edit one bookmark item, then cancel. */
	m_data_manager->destroy_edit_bookmark_view();

	/* History view -> slide one item -> add to bookmark, then done. */
	m_data_manager->destroy_add_to_bookmark_view();

	m_data_manager->destroy_new_folder_view();
}

Eina_Bool Browser_Bookmark_View::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_main_layout = elm_layout_add(m_navi_bar);
	if (!m_main_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}
	if (!elm_layout_file_set(m_main_layout, BROWSER_EDJE_DIR"/browser-bookmark-view.edj",
					"bookmark/main_layout")) {
		BROWSER_LOGE("elm_layout_file_set failed");
		return EINA_FALSE;
	}

	elm_win_conformant_set(m_win, EINA_TRUE);
	m_conformant = elm_conformant_add(m_main_layout);
	if (!m_conformant) {
		BROWSER_LOGE("elm_conformant_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_conformant, "internal_layout");
	evas_object_size_hint_weight_set(m_conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_conformant, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_conformant);

	elm_object_style_set(m_bg, "default");
	m_sub_main_layout = elm_layout_add(m_main_layout);
	if (!m_sub_main_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}
	elm_layout_file_set(m_sub_main_layout,
				BROWSER_EDJE_DIR"/browser-bookmark-view.edj",
				"browser/selectioninfo");
	evas_object_size_hint_weight_set(
				m_sub_main_layout,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(
				m_sub_main_layout,
				EVAS_HINT_FILL, EVAS_HINT_FILL);

	m_genlist_content_layout = elm_layout_add(m_sub_main_layout);
	if (!m_genlist_content_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}
	if (!elm_layout_file_set(m_genlist_content_layout, BROWSER_EDJE_DIR"/browser-bookmark-view.edj",
					"bookmark/content_layout")) {
		BROWSER_LOGE("elm_layout_file_set failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_genlist_content_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(m_genlist_content_layout);
	edje_object_signal_emit(elm_layout_edje_get(m_genlist_content_layout), "show,content_box,signal", "");

	m_genlist_content_box = elm_box_add(m_genlist_content_layout);
	if (!m_genlist_content_box) {
		BROWSER_LOGE("elm_box_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_genlist_content_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(m_genlist_content_box);
	elm_object_part_content_set(m_genlist_content_layout, "elm.swallow.content_box", m_genlist_content_box);
	elm_box_pack_end(m_genlist_content_box, m_conformant);

	m_main_folder_genlist = _create_main_folder_genlist();
	if (!m_main_folder_genlist) {
		BROWSER_LOGE("_create_main_folder_genlist failed");
		return EINA_FALSE;
	}

	elm_object_content_set(m_conformant, m_main_folder_genlist);

	m_back_button = elm_button_add(m_main_layout);
	if (!m_back_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_back_button, "browser/bookmark_controlbar_back");
	evas_object_smart_callback_add(m_back_button, "clicked", __back_button_clicked_cb, this);
	evas_object_show(m_back_button);

	elm_object_part_content_set(m_sub_main_layout, "genlist.swallow.contents", m_genlist_content_layout);
	elm_object_part_content_set(m_main_layout, "elm.swallow.content", m_sub_main_layout);

	m_top_control_bar = elm_toolbar_add(m_main_layout);
	if (!m_top_control_bar) {
		BROWSER_LOGE("elm_toolbar_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_top_control_bar, "browser/tabbar");

	elm_toolbar_shrink_mode_set(m_top_control_bar, ELM_TOOLBAR_SHRINK_EXPAND);

	elm_object_part_content_set(m_main_layout, "elm.swallow.tabbar", m_top_control_bar);

	Elm_Object_Item *bookmark_tab_item = elm_toolbar_item_append(m_top_control_bar, NULL,
								BR_STRING_BOOKMARKS, __controlbar_tab_changed_cb, this);
	elm_toolbar_item_append(m_top_control_bar, NULL, BR_STRING_HISTORY, __controlbar_tab_changed_cb, this);

	elm_toolbar_select_mode_set(m_top_control_bar, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_toolbar_item_selected_set(bookmark_tab_item, EINA_TRUE);

	evas_object_show(m_top_control_bar);

	m_navi_it = elm_naviframe_item_push(m_navi_bar, NULL, m_back_button, NULL,
								m_main_layout, "browser/tabbar");

	evas_object_smart_callback_add(m_navi_bar, "transition,finished", __naviframe_pop_finished_cb, this);

	int size = m_main_folder_list.size();
	if (size == 0)
		_show_empty_content_layout(EINA_TRUE);
	else
		_show_empty_content_layout(EINA_FALSE);

	return EINA_TRUE;
}

