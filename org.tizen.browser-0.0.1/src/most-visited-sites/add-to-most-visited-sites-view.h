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

#ifndef ADD_TO_MOST_VISITED_SITES_VIEW_H
#define ADD_TO_MOST_VISITED_SITES_VIEW_H

#include "browser-common-view.h"
#include "browser-config.h"
#include "browser-history-db.h"
#include "browser-history-layout.h"
#include "most-visited-sites.h"

class Add_To_Most_Visited_Sites_View : public Browser_Common_View {
public:
	Add_To_Most_Visited_Sites_View(Most_Visited_Sites *most_visited_sites);
	~Add_To_Most_Visited_Sites_View(void);

	Eina_Bool init(void);
private:
	Eina_Bool _create_main_layout(void);
	Evas_Object *_create_history_genlist(void);
	Eina_Bool _save_button_clicked(void);

	/* history genlist callback functions. */
	static char *__genlist_label_get_cb(void *data, Evas_Object *obj,
								const char *part);
	static char *__genlist_date_label_get_cb(void *data, Evas_Object *obj,
								const char *part);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj,
								const char *part);

	/* Elementary event callback functions */
	static void __history_item_clicked_cb(void *data, Evas_Object *obj,
								void *event_info);
	static void __cancel_button_clicked_cb(void *data, Evas_Object *obj,
								void *event_info);
	static void __save_button_clicked_cb(void *data, Evas_Object *obj,
								void *event_info);

	Evas_Object *m_conformant;
	Evas_Object *m_content_box;
	Evas_Object *m_url_edit_field;
	Evas_Object *m_history_genlist;
	Evas_Object *m_cancel_button;
	Evas_Object *m_save_button;
	vector<char *> m_history_date_label_list;
	vector<Browser_History_DB::history_item *> m_history_list;
	Date m_last_date;

	Elm_Genlist_Item_Class m_history_genlist_item_class;
	Elm_Genlist_Item_Class m_history_group_title_class;

	Most_Visited_Sites *m_most_visited_sites;
};

#endif /* BROWSER_ADD_TO_SPEED_DIAL_VIEW_H */

