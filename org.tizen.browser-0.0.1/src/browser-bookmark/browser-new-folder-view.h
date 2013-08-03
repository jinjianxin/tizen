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


#ifndef BROWSER_NEW_FOLDER_VIEW_H
#define BROWSER_NEW_FOLDER_VIEW_H

#include "browser-common-view.h"
#include "browser-config.h"

class Browser_New_Folder_View : public Browser_Common_View {
public:
	Browser_New_Folder_View(void);
	~Browser_New_Folder_View(void);

	Eina_Bool init(void);
private:
	Eina_Bool _create_main_layout(void);
	Eina_Bool _create_new_folder(const char *folder_name);
	string _get_default_new_folder_name(void);

	/* Elementary event callback functions */
	static void __cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __save_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info);

	/* genlist event callback functions */
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);

	Evas_Object *m_genlist;
	Evas_Object *m_conformant;
	Evas_Object *m_save_button;
	Evas_Object *m_cancel_button;
	Evas_Object *m_folder_name_edit_field;

	Elm_Genlist_Item_Class m_item_class;
	Elm_Object_Item *m_navi_it;

	string m_folder_name;
};

#endif /* BROWSER_NEW_FOLDER_VIEW_H */

