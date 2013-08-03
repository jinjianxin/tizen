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


#ifndef BROWSER_ADD_TO_BOOKMARK_VIEW_H
#define BROWSER_ADD_TO_BOOKMARK_VIEW_H

#include "browser-common-view.h"
#include "browser-config.h"

class Browser_Add_To_Bookmark_View : public Browser_Common_View {
public:
	Browser_Add_To_Bookmark_View(string title, string url,
			Eina_Bool is_edit_mode = EINA_FALSE, int current_folder_id = BROWSER_BOOKMARK_MAIN_FOLDER_ID);
	~Browser_Add_To_Bookmark_View(void);

	Eina_Bool init(void);

	void return_to_add_to_bookmark_view(int changed_folder_id);
private:
	typedef enum _field_type{
		TITLE_EDIT_FIELD	= 0,
		URL_EDIT_FIELD,
		UNKOWN_FIELD
	} field_type;

	typedef struct _genlist_item_param {
		field_type type;
		Browser_Add_To_Bookmark_View *add_to_bookmark_view;
	} genlist_item_param;

	Eina_Bool _create_main_layout(void);
	Evas_Object *_create_content_genlist(void);
	Eina_Bool _save_bookmark_item(const char *title, const char *url);
	Eina_Bool _modify_bookmark_item(const char *title, const char *url);

	void _done_button_clicked(const char *title, const char* url);

	/* genlist data callback functions */
	static char *__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);

	/* elementary event callback functions */
	static void __cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __done_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void _select_folder_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info);

	Elm_Genlist_Item_Class m_edit_field_item_class;
	Elm_Genlist_Item_Class m_seperator_item_class;
	Elm_Genlist_Item_Class m_folder_item_class;
	genlist_item_param m_title_param;
	genlist_item_param m_url_param;

	Evas_Object *m_genlist;
	Evas_Object *m_conformant;
	Evas_Object *m_title_cancel_button;
	Evas_Object *m_title_done_button;
	Evas_Object *m_title_edit_field;
	Evas_Object *m_url_edit_field;

	string m_title;
	string m_url;
	Eina_Bool m_is_edit_mode;
	int m_folder_id_to_save;
	int m_current_folder_id;

	Elm_Object_Item* m_navi_it;

	std::string m_input_title;
	std::string m_input_url;
};
#endif /* BROWSER_ADD_TO_BOOKMARK_VIEW_H */

