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
#include "browser-view.h"
#include "browser-select-folder-view.h"

Browser_Add_To_Bookmark_View::Browser_Add_To_Bookmark_View(string title, string url,
							Eina_Bool is_edit_mode, int current_folder_id)
:
	m_genlist(NULL)
	,m_conformant(NULL)
	,m_title_cancel_button(NULL)
	,m_title_done_button(NULL)
	,m_title_edit_field(NULL)
	,m_url_edit_field(NULL)
	,m_is_edit_mode(is_edit_mode)
	,m_folder_id_to_save(current_folder_id)
	,m_current_folder_id(current_folder_id)
	,m_navi_it(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
	m_title = title;
	m_url = url;
}

Browser_Add_To_Bookmark_View::~Browser_Add_To_Bookmark_View(void)
{
	BROWSER_LOGD("[%s]", __func__);

	/* If browser view -> add to bookmark, then back to browser view. */
	if (!m_data_manager->is_in_view_stack(BR_BOOKMARK_VIEW))
		m_data_manager->destroy_bookmark_db();

	evas_object_smart_callback_del(m_navi_bar, "transition,finished", __naviframe_pop_finished_cb);
}

Eina_Bool Browser_Add_To_Bookmark_View::init(void)
{
	BROWSER_LOGD("[%s]", __func__);

	/* If the bookmark view is in view stack, bookmark view -> edit bookmark item. */
	/* Otherwise, this case is browser view -> add to bookmark, then create the bookmar db */
	if (!m_data_manager->is_in_view_stack(BR_BOOKMARK_VIEW)) {
		if (!m_data_manager->create_bookmark_db()) {
			BROWSER_LOGE("m_data_manager->create_bookmark_db() failed");
			return EINA_FALSE;
		}
	}

	return _create_main_layout();
}

void Browser_Add_To_Bookmark_View::return_to_add_to_bookmark_view(int changed_folder_id)
{	
	BROWSER_LOGD("[%s]", __func__);

	/* If changed_folder_id is -1, just cancel the folder view. */
	if (changed_folder_id != -1) {
		m_folder_id_to_save = changed_folder_id;
		Elm_Object_Item *folder_it = elm_genlist_last_item_get(m_genlist);
		if (folder_it)
			elm_genlist_item_update(folder_it);
	}
}


void Browser_Add_To_Bookmark_View::__title_entry_changed_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Add_To_Bookmark_View *add_to_bookmark_view = (Browser_Add_To_Bookmark_View *)data;
	const char *title = elm_entry_entry_get(br_elm_editfield_entry_get(add_to_bookmark_view->m_title_edit_field));
	const char *url = elm_entry_entry_get(br_elm_editfield_entry_get(add_to_bookmark_view->m_url_edit_field));

	Eina_Bool only_has_space = EINA_FALSE;
	int space_count = 0;
	if (title && strlen(title)) {
		for (int i = 0 ; i < strlen(title) ; i++) {
			if (title[i] == ' ')
				space_count++;
		}
		if (space_count == strlen(title))
			only_has_space = EINA_TRUE;
	}

	space_count = 0;
	if (url && strlen(url)) {
		for (int i = 0 ; i < strlen(url) ; i++) {
			if (url[i] == ' ')
				space_count++;
		}
		if (space_count == strlen(url))
			only_has_space = EINA_TRUE;
	}

	char *text = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	if (!text || strlen(text) == 0 || !title || strlen(title) == 0
	    || !url || strlen(url) == 0 || only_has_space)
		elm_object_disabled_set(add_to_bookmark_view->m_title_done_button, EINA_TRUE);
	else
		elm_object_disabled_set(add_to_bookmark_view->m_title_done_button, EINA_FALSE);

	if (text)
		free(text);
}

char *Browser_Add_To_Bookmark_View::__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data || !part)
		return NULL;

	if (!strncmp(part, "elm.text.1", strlen("elm.text.1"))) {
		Browser_Add_To_Bookmark_View *add_to_bookmark_view = (Browser_Add_To_Bookmark_View *)data;
		Browser_Bookmark_DB *bookmark_db = m_data_manager->get_bookmark_db();
		int folder_id = add_to_bookmark_view->m_folder_id_to_save;

		if (add_to_bookmark_view->m_folder_id_to_save == BROWSER_BOOKMARK_MAIN_FOLDER_ID)
			return strdup(BR_STRING_BOOKMARKS);
		else {
			string default_folder_name = bookmark_db->get_folder_name_by_id(folder_id);
			return strdup(default_folder_name.c_str());
		}
	} else if (!strncmp(part, "elm.text.2", strlen("elm.text.2")))
		return strdup(BR_STRING_FOLDER);

	return NULL;
}

Evas_Object *Browser_Add_To_Bookmark_View::__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("[%s]", part);
	if (!data || !part)
		return NULL;

	genlist_item_param *param = (genlist_item_param *)data;
	field_type type = param->type;
	Browser_Add_To_Bookmark_View *add_to_bookmark_view = param->add_to_bookmark_view;
	string title = add_to_bookmark_view->m_title;
	string url = add_to_bookmark_view->m_url;

	if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
		if (type == TITLE_EDIT_FIELD) {
			add_to_bookmark_view->m_title_edit_field = br_elm_editfield_add(obj, EINA_TRUE);
			if (!add_to_bookmark_view->m_title_edit_field)
				return NULL;
			br_elm_editfield_entry_single_line_set(add_to_bookmark_view->m_title_edit_field, EINA_TRUE);
			br_elm_editfield_guide_text_set(add_to_bookmark_view->m_title_edit_field, BR_STRING_ENTER_BOOKMARK_NAME);
			br_elm_editfield_label_set(add_to_bookmark_view->m_title_edit_field, BR_STRING_TITLE);

			evas_object_smart_callback_add(br_elm_editfield_entry_get(add_to_bookmark_view->m_title_edit_field),
							"changed", __title_entry_changed_cb, add_to_bookmark_view);

			if (!add_to_bookmark_view->m_input_title.empty()) {
				elm_entry_entry_set(br_elm_editfield_entry_get(add_to_bookmark_view->m_title_edit_field),
									add_to_bookmark_view->m_input_title.c_str());
				add_to_bookmark_view->m_input_title.clear();
			} else {
				char *mark_up = elm_entry_utf8_to_markup(title.c_str());
				if (!mark_up) {
					BROWSER_LOGE("elm_entry_utf8_to_markup failed");
					return NULL;
				}
				elm_entry_entry_set(br_elm_editfield_entry_get(add_to_bookmark_view->m_title_edit_field), mark_up);

				if (mark_up)
					free(mark_up);
			}

			if (elm_entry_is_empty(br_elm_editfield_entry_get(add_to_bookmark_view->m_title_edit_field)))
				elm_object_signal_emit(add_to_bookmark_view->m_title_edit_field, "elm,state,guidetext,show", "elm");

			return add_to_bookmark_view->m_title_edit_field;
		} else {
			add_to_bookmark_view->m_url_edit_field = br_elm_editfield_add(obj, EINA_TRUE);
			if (!add_to_bookmark_view->m_url_edit_field)
				return NULL;
			br_elm_editfield_entry_single_line_set(add_to_bookmark_view->m_url_edit_field, EINA_TRUE);
			br_elm_editfield_label_set(add_to_bookmark_view->m_url_edit_field, BR_STRING_URL);
			elm_entry_input_panel_layout_set(br_elm_editfield_entry_get(add_to_bookmark_view->m_url_edit_field),
							ELM_INPUT_PANEL_LAYOUT_URL);

			evas_object_smart_callback_add(br_elm_editfield_entry_get(add_to_bookmark_view->m_url_edit_field),
							"changed", __title_entry_changed_cb, add_to_bookmark_view);

			if (!add_to_bookmark_view->m_input_url.empty()) {
				elm_entry_entry_set(br_elm_editfield_entry_get(add_to_bookmark_view->m_url_edit_field),
									add_to_bookmark_view->m_input_url.c_str());
				add_to_bookmark_view->m_input_url.clear();
			} else {
				if (url.empty())
					url = "http://";
				char *mark_up = elm_entry_utf8_to_markup(url.c_str());
				if (!mark_up) {
					BROWSER_LOGE("elm_entry_utf8_to_markup failed");
					return NULL;
				}
				elm_entry_entry_set(br_elm_editfield_entry_get(add_to_bookmark_view->m_url_edit_field), mark_up);
				if (mark_up)
					free(mark_up);
			}

			return add_to_bookmark_view->m_url_edit_field;
		}
	}

	return NULL;
}

Evas_Object *Browser_Add_To_Bookmark_View::_create_content_genlist(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *genlist = elm_genlist_add(m_conformant);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	elm_object_style_set(genlist, "dialogue");

	m_seperator_item_class.item_style = "dialogue/seperator.2";
	m_seperator_item_class.func.text_get = NULL;
	m_seperator_item_class.func.del = NULL;
	m_seperator_item_class.func.content_get = NULL;
	m_seperator_item_class.func.state_get = NULL;
	Elm_Object_Item *it = elm_genlist_item_append(genlist, &m_seperator_item_class, NULL, NULL,
								ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_edit_field_item_class.item_style = "dialogue/1icon";
	m_edit_field_item_class.func.text_get = NULL;
	m_edit_field_item_class.func.content_get = __genlist_icon_get_cb;
	m_edit_field_item_class.func.state_get = NULL;
	m_edit_field_item_class.func.del = NULL;

	m_title_param.type = TITLE_EDIT_FIELD;
	m_title_param.add_to_bookmark_view = this;
	elm_genlist_item_append(genlist, &m_edit_field_item_class, &m_title_param, NULL,
								ELM_GENLIST_ITEM_NONE, NULL, NULL);

	it = elm_genlist_item_append(genlist, &m_seperator_item_class, NULL, NULL,
								ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_url_param.type = URL_EDIT_FIELD;
	m_url_param.add_to_bookmark_view = this;
	elm_genlist_item_append(genlist, &m_edit_field_item_class, &m_url_param, NULL,
								ELM_GENLIST_ITEM_NONE, NULL, NULL);

	it = elm_genlist_item_append(genlist, &m_seperator_item_class, NULL, NULL,
								ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_folder_item_class.item_style = "dialogue/2text.2";
	m_folder_item_class.func.text_get = __genlist_label_get_cb;
	m_folder_item_class.func.content_get = NULL;
	m_folder_item_class.func.state_get = NULL;
	m_folder_item_class.func.del = NULL;

	elm_genlist_item_append(genlist, &m_folder_item_class, this, NULL, ELM_GENLIST_ITEM_NONE,
								_select_folder_clicked_cb, this);

	evas_object_show(genlist);

	return genlist;
}

void Browser_Add_To_Bookmark_View::_select_folder_clicked_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Add_To_Bookmark_View *add_to_bookmark_view = (Browser_Add_To_Bookmark_View *)data;
	if (!m_data_manager->create_select_folder_view(add_to_bookmark_view->m_folder_id_to_save)) {
		BROWSER_LOGE("m_data_manager->create_select_folder_view failed");
		return;
	}

	if (!m_data_manager->get_select_folder_view()->init()) {
		BROWSER_LOGE("get_select_folder_view()->init failed");
		m_data_manager->destroy_select_folder_view();
	}

	Elm_Object_Item *it = elm_genlist_selected_item_get(obj);
	elm_genlist_item_selected_set(it, EINA_FALSE);

	add_to_bookmark_view->m_input_title = std::string(elm_entry_entry_get(br_elm_editfield_entry_get(add_to_bookmark_view->m_title_edit_field)));
	add_to_bookmark_view->m_input_url = std::string(elm_entry_entry_get(br_elm_editfield_entry_get(add_to_bookmark_view->m_url_edit_field)));
}

void Browser_Add_To_Bookmark_View::__cancel_button_clicked_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Add_To_Bookmark_View *add_to_bookmark_view = (Browser_Add_To_Bookmark_View *)data;
	Evas_Object *navi_bar = add_to_bookmark_view->m_navi_bar;

	if (elm_naviframe_bottom_item_get(navi_bar) != elm_naviframe_top_item_get(navi_bar))
		elm_naviframe_item_pop(navi_bar);

	if (m_data_manager->is_in_view_stack(BR_ADD_TO_BOOKMARK_VIEW)) {
		/* Set the title if enter the add to bookmark while loading. */
		m_data_manager->get_browser_view()->return_to_browser_view();
	}

	if (m_data_manager->is_in_view_stack(BR_BOOKMARK_VIEW) && m_data_manager->get_bookmark_view())
		/* boomark view -> edit one bookmark item, then cancel. */		
		m_data_manager->get_bookmark_view()->return_to_bookmark_view();
}

/* bookmark -> edit -> edit each bookmark item then bookmark edit view -> done */
Eina_Bool Browser_Add_To_Bookmark_View::_modify_bookmark_item(const char *title, const char *url)
{	
	int bookmark_id = 0;
	int ret = EINA_TRUE;
	Browser_Bookmark_DB *bookmark_db = m_data_manager->get_bookmark_db();
	ret = bookmark_db->get_bookmark_id_by_title_url(m_current_folder_id, m_title.c_str(), m_url.c_str(), &bookmark_id);
	if (!ret)
		return EINA_FALSE;
	ret = bookmark_db->modify_bookmark(bookmark_id, m_folder_id_to_save, title, url);
	if (!ret)
		return EINA_FALSE;

	Browser_Bookmark_DB::bookmark_item *item = NULL;

	Browser_Bookmark_View *bookmark_view = m_data_manager->get_bookmark_view();
	Elm_Object_Item *bookmark_edit_it = bookmark_view->m_current_genlist_item_to_edit;
	item = (Browser_Bookmark_DB::bookmark_item *)elm_object_item_data_get(bookmark_edit_it);

	item->title = title;
	item->url = url;
	elm_object_item_data_set(bookmark_edit_it, item);

	if (m_current_folder_id != m_folder_id_to_save) {
		if (m_folder_id_to_save == BROWSER_BOOKMARK_MAIN_FOLDER_ID) {
			/* If the bookmark item in sub folder is moved to main folder,
			  * remove it from sub folder and append it to main folder. */
			for(int i = 0 ; i < bookmark_view->m_sub_folder_list.size() ; i++) {
				if (bookmark_view->m_sub_folder_list[i] == item) {
					bookmark_view->m_sub_folder_list.erase(bookmark_view->m_sub_folder_list.begin() + i);
					break;
				}
			}

			/* If the bookmark is empty page, destroy the empty layout to show the bookmark list. */
			if (bookmark_view->m_main_folder_list.size() == 0)
				bookmark_view->_show_empty_content_layout(EINA_TRUE);
			else
				bookmark_view->_show_empty_content_layout(EINA_FALSE);
		} else if (m_current_folder_id == BROWSER_BOOKMARK_MAIN_FOLDER_ID) {
			/* Move one bookmark item from main folder to sub folder. */
			for(int i = 0 ; i < bookmark_view->m_main_folder_list.size() ; i++) {
				if (bookmark_view->m_main_folder_list[i] == item) {
					delete bookmark_view->m_main_folder_list[i];
					bookmark_view->m_main_folder_list.erase(bookmark_view->m_main_folder_list.begin() + i);
					break;
				}
			}
		} else {
			/* If the bookmark item in sub folder is moved to another sub folder,
			  * just remove the item from the genlist and delete the malloced item.*/
			for(int i = 0 ; i < bookmark_view->m_sub_folder_list.size() ; i++) {
				if (bookmark_view->m_sub_folder_list[i] == item) {
					delete item;
					bookmark_view->m_sub_folder_list.erase(bookmark_view->m_sub_folder_list.begin() + i);
					break;
				}
			}
		}

		elm_object_item_del(bookmark_edit_it);
	}

	return EINA_TRUE;
}

Eina_Bool Browser_Add_To_Bookmark_View::_save_bookmark_item(const char *title, const char *url)
{
	BROWSER_LOGD("[%s]", __func__);
	return m_data_manager->get_bookmark_db()->save_bookmark(m_folder_id_to_save, title, url);
}

void Browser_Add_To_Bookmark_View::__done_button_clicked_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Add_To_Bookmark_View *add_to_bookmark_view = (Browser_Add_To_Bookmark_View *)data;
	Evas_Object *navi_bar = add_to_bookmark_view->m_navi_bar;
	Evas_Object *url_edit_field_entry = br_elm_editfield_entry_get(add_to_bookmark_view->m_url_edit_field);
	char *url = elm_entry_markup_to_utf8(elm_entry_entry_get(url_edit_field_entry));
	Evas_Object *title_edit_field_entry = br_elm_editfield_entry_get(add_to_bookmark_view->m_title_edit_field);
	char *title = elm_entry_markup_to_utf8(elm_entry_entry_get(title_edit_field_entry));

	if (!url || strlen(url) == 0) {
		add_to_bookmark_view->show_msg_popup(BR_STRING_ENTER_URL);
		return;
	}
	if (!title || strlen(title) == 0) {
		add_to_bookmark_view->show_msg_popup(BR_STRING_ENTER_TITLE);
		return;
	}

	add_to_bookmark_view->_done_button_clicked(title, url);

	if (url)
		free(url);
	if (title)
		free(title);
}

void Browser_Add_To_Bookmark_View::_done_button_clicked(const char *title, const char* url)
{
	Browser_Bookmark_DB *bookmark_db = m_data_manager->get_bookmark_db();
	Eina_Bool is_duplicate = bookmark_db->is_duplicated(m_folder_id_to_save, title, url);
	if (!is_duplicate) {
		if (bookmark_db->is_full()) {
			show_msg_popup(BR_STRING_WARNING,
					BR_STRING_WARNING_OVER_BOOKMARK_LIMIT, 5);
			return;
		} else {
			int ret = EINA_TRUE;
			if (!m_is_edit_mode)
				ret = _save_bookmark_item(title, url);
			else
				ret = _modify_bookmark_item(title, url);

			if (!ret) {
				show_msg_popup(BR_STRING_FAILED);
				return;
			} else {
				if (elm_naviframe_bottom_item_get(m_navi_bar) != elm_naviframe_top_item_get(m_navi_bar))
					elm_naviframe_item_pop(m_navi_bar);

				if (!m_data_manager->is_in_view_stack(BR_BOOKMARK_VIEW)) {
#if defined(FEATURE_MOST_VISITED_SITES)
					/* Set the title if enter the add to bookmark while loading. */
					if (m_data_manager->get_browser_view()->is_most_visited_sites_running())
						m_data_manager->get_browser_view()->return_to_browser_view(EINA_TRUE);
					else
#endif
						m_data_manager->get_browser_view()->return_to_browser_view();

#if defined(FEATURE_MOST_VISITED_SITES)
					if (m_data_manager->get_browser_view()->is_most_visited_sites_running())
						m_data_manager->get_browser_view()->show_notify_popup(BR_STRING_SAVED, 3,
													EINA_TRUE);
					else
#endif
						m_data_manager->get_browser_view()->show_notify_popup(BR_STRING_SAVED, 3);
				} else {
					int added_bookmark_id = -1;
					bookmark_db->get_bookmark_id_by_title_url(m_folder_id_to_save, title,
								url, &added_bookmark_id);
					BROWSER_LOGD("added_bookmark_id=%d", added_bookmark_id);
						/* History view -> slide one item -> add to bookmark, then done. */
					m_data_manager->get_bookmark_view()->return_to_bookmark_view(added_bookmark_id);

					m_data_manager->get_browser_view()->show_notify_popup(BR_STRING_SAVED, 3, EINA_TRUE);
				}
			}
		}
	} else
		show_msg_popup(BR_STRING_ALREADY_EXISTS);
}
void Browser_Add_To_Bookmark_View::__naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Add_To_Bookmark_View *add_to_bookmark_view = (Browser_Add_To_Bookmark_View *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	if (add_to_bookmark_view->m_navi_it != elm_naviframe_top_item_get(m_navi_bar))
		return;

	m_data_manager->destroy_select_folder_view();
}

Eina_Bool Browser_Add_To_Bookmark_View::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	elm_win_conformant_set(m_win, EINA_TRUE);
	m_conformant = elm_conformant_add(m_win);
	if (!m_conformant) {
		BROWSER_LOGE("elm_conformant_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_conformant, "internal_layout");
	evas_object_size_hint_weight_set(m_conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(m_conformant);

	m_genlist = _create_content_genlist();
	if (!m_genlist) {
		BROWSER_LOGE("_create_content_genlist failed");
		return EINA_FALSE;
	}

	elm_object_content_set(m_conformant, m_genlist);
	if (m_is_edit_mode)
		m_navi_it = elm_naviframe_item_push(m_navi_bar, BR_STRING_EDIT_BOOKMARK, NULL,
								NULL, m_conformant, "browser_titlebar");
	else
		m_navi_it = elm_naviframe_item_push(m_navi_bar, BR_STRING_ADD_TO_BOOKMARKS, NULL,
								NULL, m_conformant, "browser_titlebar");

	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_PREV_BTN, NULL);

	evas_object_smart_callback_add(m_navi_bar, "transition,finished", __naviframe_pop_finished_cb, this);

	m_title_cancel_button = elm_button_add(m_conformant);
	if (!m_title_cancel_button) {
		BROWSER_LOGE("elm_button_add failed!");
		return EINA_FALSE;
	}
	elm_object_style_set(m_title_cancel_button, "browser/title_button");
	elm_object_text_set(m_title_cancel_button, BR_STRING_CANCEL);
	evas_object_smart_callback_add(m_title_cancel_button, "clicked", __cancel_button_clicked_cb, this);

	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_TITLE_LEFT_BTN, m_title_cancel_button);
	evas_object_show(m_title_cancel_button);

	m_title_done_button = elm_button_add(m_conformant);
	if (!m_title_done_button) {
		BROWSER_LOGE("elm_button_add failed!");
		return EINA_FALSE;
	}
	elm_object_style_set(m_title_done_button, "browser/title_button");
	elm_object_text_set(m_title_done_button, BR_STRING_DONE);
	evas_object_smart_callback_add(m_title_done_button, "clicked", __done_button_clicked_cb, this);

	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_TITLE_RIGHT_BTN, m_title_done_button);
	evas_object_show(m_title_done_button);

	return EINA_TRUE;
}

