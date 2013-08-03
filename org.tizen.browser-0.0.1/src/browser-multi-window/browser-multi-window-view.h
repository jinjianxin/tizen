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

#ifndef BROWSER_MULTI_WINDOW_VIEW_H
#define BROWSER_MULTI_WINDOW_VIEW_H

#include "browser-common-view.h"
#include "browser-config.h"

class Browser_Window;

class Browser_Multi_Window_View : public Browser_Common_View {
public:
	Browser_Multi_Window_View(void);
	~Browser_Multi_Window_View(void);

	Eina_Bool init(double duration = 0.5, Eina_Bool grid_mode = EINA_FALSE);
	void close_multi_window(void);

	typedef struct _gengrid_callback_param {
		void *multi_window_view;
		int index;
	} gengrid_callback_param;

	Eina_Bool _is_grid_mode(void);
	Eina_Bool is_reordering_mode(void) { return m_is_reordering; }
private:
	Eina_Bool _create_main_layout(void);
	Eina_Bool _show_zoom_out_effect(void);
	Eina_Bool _show_zoom_in_effect(Evas_Object *item);
	Eina_Bool _show_new_window_scroll_effect(void);
	Eina_Bool _show_zoom_in_new_window_effect(void);
	Eina_Bool _show_delete_window_scroll_effect(void);
	void _set_multi_window_mode(Eina_Bool is_multi_window);
	Eina_Bool _fill_multi_window_items(void);
	void _snapshot_clicked(Evas_Object *layout_edje);
	void __zoom_out_finished(void);
	void _scroller_animation_stop(void);
	int _get_scroll_page_size(void);
	void __delete_window_icon_clicked(Evas_Object *layout_edje);
	void _delete_window_scroll_finished(void);
	Eina_Bool _create_gengrid(void);
	Eina_Bool _show_grid_mode_zoom_in_effect(int index);
	Eina_Bool __delete_window_icon_grid_mode(int index);
	Eina_Bool _reorder_windows(void);
	void _show_grey_effect(void);
	Evas_Object *_create_control_bar(void);

	/* gengrid event callback functions */
	static Evas_Object *_get_gengrid_icon_cb(void *data, Evas_Object *obj, const char *part);

	/* elementary event callback functions */
	static void __close_multi_window_button_clicked_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __new_window_button_clicked_cb(void *data, Evas_Object *obj,
						void *event_info);
	static void __view_change_button_clicked_cb(void *data, Evas_Object *obj,
						void *event_info);
	static void __scroller_animation_stop_cb(void *data, Evas_Object *obj,
								void *event_info);
	static void __gengrid_item_moved_cb(void *data, Evas_Object *obj, void *event_info);
	static void __gengrid_item_longpress_cb(void *data, Evas_Object *obj, void *event_info);
	static void __index_selected_cb(void *data, Evas_Object *obj, void *event_info);

	/* transit del callback functions */
	static void __zoom_in_finished_cb(void *data, Elm_Transit *transit);
	static void __zoom_out_finished_cb(void *data, Elm_Transit *transit);
	static void __new_window_zoom_in_finished_cb(void *data, Elm_Transit *transit);
	static void __delete_window_scroll_finished_cb(void *data, Elm_Transit *transit);
	static void __new_window_scroll_finished_cb(void *data, Elm_Transit *transit);

	/* edje event callback functions */
	static void __snapshot_clicked_cb(void *data, Evas_Object *obj,
					const char *emission, const char *source);
	static void __delete_window_icon_clicked_cb(void *data, Evas_Object *obj,
					const char *emission, const char *source);
	static void __delete_window_icon_grid_mode_clicked_cb(void *data, Evas_Object *obj,
						const char *emission, const char *source);
	static void __snapshot_grid_mode_clicked_cb(void *data, Evas_Object *obj,
						const char *emission, const char *source);
	static void __delete_gengrid_cb(void *data, Evas_Object *obj, const char *emission,
						const char *source);

	/* Ecore idler callback functions */
	static Eina_Bool __zoom_out_effect_idler_cb(void *data);

	Evas_Object *m_main_layout;
	Evas_Object *m_scroller;
	Evas_Object *m_item_box;
	Evas_Object *m_zoom_effect_image;
	Evas_Object *m_flip_effect_image;
	Elm_Transit *m_zoom_transit;
	Elm_Transit *m_zoom_move_transit;
	Elm_Transit *m_scroll_move_transit;

	Evas_Object *m_dummy_front_item_layout;
	Evas_Object *m_dummy_front_item_snapshot;
	Evas_Object *m_dummy_end_item_layout;
	Evas_Object *m_dummy_end_item_snapshot;

	Evas_Object *m_page_control;

	int m_select_item_index;
	int m_current_position_index;
	std::vector<Evas_Object *> m_item_list;

	Evas_Object *m_gengrid;
	Elm_Gengrid_Item_Class m_gengrid_item_class;
	gengrid_callback_param m_callback_param[BROWSER_MULTI_WINDOW_MAX_COUNT];

	Eina_Bool m_is_reordering;
	Ecore_Idler *m_zoom_out_effect_idler;
	double m_zoom_out_duration;

	Elm_Object_Item *m_index_items[BROWSER_MULTI_WINDOW_MAX_COUNT];
	Elm_Object_Item *m_new_window_button;
	Elm_Object_Item *m_change_view_button;
	Evas_Object *m_cancel_button;
	Evas_Object *m_controlbar;

	Eina_Bool m_init_grid_mode;
};
#endif /* BROWSER_MULTI_WINDOW_VIEW_H */

