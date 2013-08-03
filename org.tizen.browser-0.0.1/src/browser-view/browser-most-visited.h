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

#ifndef BROWSER_MOST_VISITED_H
#define BROWSER_MOST_VISITED_H

#include "browser-config.h"

class Browser_History_DB;
class Browser_View;

class Browser_Most_Visited {
public:
	struct most_visited_item {
		std::string url;
		Evas_Object *layout;
		Evas_Object *item_button;
		Evas_Object *favicon;
		void *user_data;
	};

	Browser_Most_Visited(Evas_Object *navi_bar, Browser_History_DB *history_db, Browser_View *browser_view);
	~Browser_Most_Visited(void);

	Evas_Object *create_most_visited_layout(void);
#if defined(HORIZONTAL_UI)
	void rotate(int degree);
#endif
private:
	Eina_Bool _update_items(void);
	void _clear_items(void);
	Eina_Bool _show_guide_text(void);

	/* elementary event callback functions */
	static void _most_visited_item_clicked_cb(void *data, Evas_Object *obj,
							void *event_info);
	static void __done_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	/* evas object event callback functions */
	static void __item_mouse_down_cb(void* data, Evas* evas, Evas_Object* obj, void* ev);
	static void __item_mouse_up_cb(void* data, Evas* evas, Evas_Object* obj, void* ev);

	/* ecore timer callback functions */
	static Eina_Bool __longpress_timeout_cb(void* data);

	/* edje object event callback functions */
	static void __delete_icon_clicked_cb(void *data, Evas_Object *obj,
							const char *emission, const char *source);

	Evas_Object *m_navi_bar;
	Evas_Object *m_main_layout;
	Evas_Object *m_item_box;
	Evas_Object *m_done_button;
	Evas_Object *m_guide_text_label;
	Browser_History_DB *m_history_db;
	Browser_View *m_browser_view;
	Ecore_Timer *m_longpress_timer;
	std::vector<most_visited_item *> m_most_visited_list;
	Eina_Bool m_is_edit_mode;
};
#endif /* BROWSER_MOST_VISITED_H */

