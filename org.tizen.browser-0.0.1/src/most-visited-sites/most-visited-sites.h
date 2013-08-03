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

#ifndef BROWSER_MOST_VISITED_SITES_H
#define BROWSER_MOST_VISITED_SITES_H

#include "browser-config.h"
#include "most-visited-sites-db.h"

class Add_To_Most_Visited_Sites_View;
class Browser_View;
class Most_Visited_Sites_DB;
class Browser_Common_View;

class Most_Visited_Sites : public Browser_Common_View {
	friend class Add_To_Most_Visited_Sites_View;
	friend class Browser_View;
public:
	struct most_visited_sites_item {
		Evas_Object *layout;
		Evas_Object *screen_shot;
		char *title;
		char *url;
		char *history_id;
		Elm_Object_Item *item;
		void *data;
		Eina_Bool tack;
	};

	Most_Visited_Sites(Evas_Object *navi_bar, Browser_View *browser_view,
					Most_Visited_Sites_DB *most_visited_sites_db);
	~Most_Visited_Sites(void);

	Evas_Object *create_most_visited_sites_main_layout(void);
	Evas_Object *create_guide_text_main_layout(void);
	void destroy_guide_text_main_layout(void);
	Eina_Bool is_guide_text_running(void);
	void rotate(void);
private:
	/* Elementary event callback functions */
	static void __item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __item_press_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
	static void __item_release_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
	static void __empty_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __context_popup_dismissed_cb(void *data, Evas_Object *obj,
									void *event_info);
	static void __context_popup_unpin_clicked_cb(void *data, Evas_Object *obj,
									void *event_info);
	static void __context_popup_pin_clicked_cb(void *data, Evas_Object *obj,
									void *event_info);
	static void __context_popup_modify_clicked_cb(void *data, Evas_Object *obj,
									void *event_info);
	static void __context_popup_delete_clicked_cb(void *data, Evas_Object *obj,
									void *event_info);
	static void __context_popup_bookmark_clicked_cb(void *data, Evas_Object *obj,
									void *event_info);
	static void __guide_text_ok_button_clicked_cb(void *data, Evas_Object *obj,
									void *event_info);

	/* edje event callback functions */
	static void __never_show_text_clicked_cb(void *data, Evas_Object *obj,
						const char *emission, const char *source);
	static void __delete_button_clicked_cb(void *data, Evas_Object *obj,
								const char *emission, const char *source);
	static void __bookmark_button_clicked_cb(void *data, Evas_Object *obj,
								const char *emission, const char *source);

	/* evas object smart callback functions */
	static void __item_longpressed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __item_drag_cb(void *data, Evas_Object *obj, void *event_info);
	static void __item_moved_cb(void *data, Evas_Object *obj, void *event_info);
	static void __setting_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __done_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	/* gengrid callback functions */
	static Evas_Object *__gengrid_icon_get_cb(void *data,
					Evas_Object *obj, const char *part);

	/* idler callback functions */
	static Eina_Bool __gengrid_icon_get_idler_cb(void *data);
	static Eina_Bool __load_url_idler_cb(void *data);

	Evas_Object *__get_most_visited_sites_item_layout(most_visited_sites_item *item);
	Eina_Bool _show_item_context_popup(void);
	Eina_Bool _delete_selected_item(void);
	Eina_Bool _empty_item_clicked(most_visited_sites_item *item);
	Eina_Bool _reload_items(void);
	Eina_Bool _item_moved(void);
	void _set_edit_mode(Eina_Bool edit_mode);

	Evas_Object *m_navi_bar;
	Browser_View *m_browser_view;
	Evas_Object *m_gengrid;
	Elm_Gengrid_Item_Class m_gengrid_item_class;

	Most_Visited_Sites_DB *m_most_visited_sites_db;
	std::vector<Most_Visited_Sites_DB::most_visited_sites_entry> m_entry_list;
	std::vector<Most_Visited_Sites_DB::most_visited_sites_entry> m_most_visited_list;
	std::vector<most_visited_sites_item *> m_item_list;

	Eina_Bool m_is_drag;
	most_visited_sites_item *m_selected_item;
	Evas_Object *m_context_popup;

	Evas_Object *m_guide_text_main_layout;
	Evas_Object *m_guide_text_label;
	Evas_Object *m_never_show_check_box;
	Evas_Object *m_never_show_text_label;
	Evas_Object *m_ok_button;

	Evas_Object *m_main_layout;
	Evas_Object *m_setting_button;
	Evas_Object *m_done_button;
	Eina_Bool m_edit_mode;

	Eina_Bool m_is_item_clicked;
};

#endif /* BROWSER_SPEED_DIAL_H */

