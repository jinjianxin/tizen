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


#ifndef BROWSER_SETTINGS_EDIT_HOMEPAGE_H
#define BROWSER_SETTINGS_EDIT_HOMEPAGE_H

#include "browser-common-view.h"
#include "browser-config.h"
#include "browser-settings-main-view.h"

class Browser_Settings_Edit_Homepage_View : public Browser_Common_View {
public:
	Browser_Settings_Edit_Homepage_View(Browser_Settings_Main_View *main_view);
	~Browser_Settings_Edit_Homepage_View(void);

	Eina_Bool init(void);
private:
	Eina_Bool _create_main_layout(void);

	/* elementary event callback functions */
	static void __back_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __done_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_field_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);

	Evas_Object *m_conformant;
	Evas_Object *m_genlist;
	Evas_Object *m_edit_field;
	Evas_Object *m_done_button;
	Evas_Object *m_cancel_button;

	Elm_Genlist_Item_Class m_item_class;
	Elm_Object_Item *m_navi_it;

	Browser_Settings_Main_View *m_main_view;
};

#endif /* BROWSER_SETTINGS_EDIT_HOMEPAGE_H */

