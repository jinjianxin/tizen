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


#include "browser-bookmark-db.h"
#include "browser-bookmark-view.h"
#include "browser-new-folder-view.h"
#include "browser-select-folder-view.h"

Browser_New_Folder_View::Browser_New_Folder_View(void)
:
	m_genlist(NULL)
	,m_conformant(NULL)
	,m_save_button(NULL)
	,m_cancel_button(NULL)
	,m_folder_name_edit_field(NULL)
	,m_navi_it(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_New_Folder_View::~Browser_New_Folder_View(void)
{
	BROWSER_LOGD("[%s]", __func__);
	evas_object_smart_callback_del(m_navi_bar, "transition,finished", __naviframe_pop_finished_cb);
}

Eina_Bool Browser_New_Folder_View::init(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_folder_name = _get_default_new_folder_name();
	return _create_main_layout();
}

void Browser_New_Folder_View::__cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_New_Folder_View *new_folder_view = (Browser_New_Folder_View *)data;

	if (elm_naviframe_bottom_item_get(new_folder_view->m_navi_bar)
	    != elm_naviframe_top_item_get(new_folder_view->m_navi_bar))
		elm_naviframe_item_pop(new_folder_view->m_navi_bar);
}

void Browser_New_Folder_View::__save_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_New_Folder_View *new_folder_view = (Browser_New_Folder_View *)data;
	Evas_Object *edit_field = new_folder_view->m_folder_name_edit_field;
	const char *input_text = elm_entry_entry_get(br_elm_editfield_entry_get(edit_field));

	char *utf8_text = elm_entry_markup_to_utf8(input_text);
	if (!utf8_text) {
		BROWSER_LOGD("elm_entry_markup_to_utf8 failed");
		return;
	}

	const char *folder_name = new_folder_view->_trim(utf8_text);

	if (!folder_name || strlen(folder_name) == 0) {
		new_folder_view->show_msg_popup(BR_STRING_ENTER_FOLDER_NAME);
		BROWSER_LOGD("new_folder_view->m_folder_name.c_str() = [%s]", new_folder_view->m_folder_name.c_str());
		elm_entry_entry_set(br_elm_editfield_entry_get(edit_field), new_folder_view->m_folder_name.c_str());
		free(utf8_text);
		return;
	} else {
		if (!new_folder_view->_create_new_folder(folder_name)) {
			BROWSER_LOGD("_create_new_folder failed");
			free(utf8_text);
			return;
		}
	}

	free(utf8_text);

	if (elm_naviframe_bottom_item_get(m_navi_bar) != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);

	if (m_data_manager->is_in_view_stack(BR_SELECT_FOLDER_VIEW)) {
		/* Add to bookmark -> Folder select -> New folder, then Done. */
		m_data_manager->get_select_folder_view()->return_to_select_folder_view();
	}

	Browser_Bookmark_View *bookmark_view = m_data_manager->get_bookmark_view();

	if (m_data_manager->is_in_view_stack(BR_BOOKMARK_VIEW)
	    && !m_data_manager->is_in_view_stack(BR_SELECT_FOLDER_VIEW))
		bookmark_view->return_to_bookmark_view();
}

Eina_Bool Browser_New_Folder_View::_create_new_folder(const char *folder_name)
{
	BROWSER_LOGD("folder_name=[%s]", folder_name);

	Browser_Bookmark_DB *bookmark_db = m_data_manager->get_bookmark_db();
	Browser_Bookmark_View *bookmark_view = m_data_manager->get_bookmark_view();

	if (bookmark_db->is_duplicated(folder_name)) {
		show_msg_popup(BR_STRING_ALREADY_EXISTS);
		return EINA_FALSE;
	}
	else if (bookmark_db->is_full()) {
		show_msg_popup(BR_STRING_WARNING, BR_STRING_WARNING_OVER_BOOKMARK_LIMIT, 3);
		return EINA_FALSE;
	}
	else {
		Eina_Bool ret = bookmark_db->save_folder(folder_name);
		if (!ret)
			BROWSER_LOGD("bookmark_db->save_folder failed");
	}

	if (m_data_manager->is_in_view_stack(BR_BOOKMARK_VIEW)) {
		/* If new folder is created, append the folder item to the main bookmark list. */
		vector<Browser_Bookmark_DB::bookmark_item *> folder_list;
		bookmark_db->get_folder_list(folder_list);
		for(int i = 0 ; i < folder_list.size() ; i++) {
			if (!strcmp(folder_list[i]->title.c_str(), folder_name)) {
				folder_list[i]->user_data_1 = (void *)bookmark_view;
				bookmark_view->m_main_folder_list.push_back(folder_list[i]);
				folder_list[i]->user_data_2 = (void *)elm_genlist_item_append(bookmark_view->m_main_folder_genlist,
						&(bookmark_view->m_folder_genlist_item_class), folder_list[i], NULL,
						ELM_GENLIST_ITEM_NONE, bookmark_view->__bookmark_item_clicked_cb, bookmark_view);

				break;
			}
		}

		for(int i = 0 ; i < folder_list.size() ; i++) {
			if (folder_list[i] && strcmp(folder_list[i]->title.c_str(), folder_name))
				delete folder_list[i];
		}

	}

	return EINA_TRUE;
}

void Browser_New_Folder_View::__title_entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Browser_New_Folder_View *new_folder_view = (Browser_New_Folder_View *)data;
	const char *title = elm_entry_entry_get(br_elm_editfield_entry_get(new_folder_view->m_folder_name_edit_field));

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

	if (!title || strlen(title) == 0 || only_has_space)
		elm_object_disabled_set(new_folder_view->m_save_button, EINA_TRUE);
	else
		elm_object_disabled_set(new_folder_view->m_save_button, EINA_FALSE);
}

Evas_Object *Browser_New_Folder_View::__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	Browser_New_Folder_View *new_folder_view = (Browser_New_Folder_View *)data;

	if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
		new_folder_view->m_folder_name_edit_field = br_elm_editfield_add(obj, EINA_TRUE);
		if (!new_folder_view->m_folder_name_edit_field) {
			BROWSER_LOGE("elm_editfield_add failed");
			return NULL;
		}
		br_elm_editfield_entry_single_line_set(new_folder_view->m_folder_name_edit_field, EINA_TRUE);
		br_elm_editfield_label_set(new_folder_view->m_folder_name_edit_field, BR_STRING_TITLE);
		br_elm_editfield_guide_text_set(new_folder_view->m_folder_name_edit_field, BR_STRING_ENTER_FOLDER_NAME);
		elm_entry_entry_set(br_elm_editfield_entry_get(new_folder_view->m_folder_name_edit_field),
								new_folder_view->m_folder_name.c_str());

		evas_object_smart_callback_add(br_elm_editfield_entry_get(new_folder_view->m_folder_name_edit_field),
						"changed", __title_entry_changed_cb, new_folder_view);

		ecore_idler_add(new_folder_view->__set_focus_editfield_idler_cb, new_folder_view->m_folder_name_edit_field);
		return new_folder_view->m_folder_name_edit_field;
	}

	return NULL;
}

string Browser_New_Folder_View::_get_default_new_folder_name(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int untitled_folder_count = 0;
	Eina_Bool ret = m_data_manager->get_bookmark_db()->get_untitled_folder_count(&untitled_folder_count);
	if (!ret) {
		BROWSER_LOGD("get_untitled_folder_count failed");
		return std::string("");
	}

	char buf[32] = {0, };
	snprintf(buf, sizeof(buf), "Folder_%02d", untitled_folder_count);

	return std::string(buf);
}

void Browser_New_Folder_View::__naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_New_Folder_View *new_folder_view = (Browser_New_Folder_View *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	if (new_folder_view->m_navi_it != elm_naviframe_top_item_get(m_navi_bar))
		return;
}

Eina_Bool Browser_New_Folder_View::_create_main_layout(void)
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

	m_navi_it = elm_naviframe_item_push(m_navi_bar, BR_STRING_CREATE_FOLDER, NULL, NULL,
											m_conformant, "browser_titlebar");

	evas_object_smart_callback_add(m_navi_bar, "transition,finished", __naviframe_pop_finished_cb, this);

	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_PREV_BTN, NULL);

	m_save_button = elm_button_add(m_conformant);
	if (!m_save_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_save_button, "browser/title_button");
	elm_object_text_set(m_save_button, BR_STRING_DONE);
	evas_object_show(m_save_button);
	evas_object_smart_callback_add(m_save_button, "clicked", __save_button_clicked_cb, this);
	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_TITLE_RIGHT_BTN, m_save_button);

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

