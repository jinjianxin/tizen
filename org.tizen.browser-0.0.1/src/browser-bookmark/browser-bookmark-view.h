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



#ifndef BROWSER_BOOKMARK_VIEW_H
#define BROWSER_BOOKMARK_VIEW_H

#include "browser-bookmark-db.h"
#include "browser-common-view.h"
#include "browser-config.h"

class Browser_Bookmark_DB;

class Browser_Bookmark_View : public Browser_Common_View {
	friend class Browser_History_Layout;
	friend class Browser_Add_To_Bookmark_View;
	friend class Browser_New_Folder_View;
public:
	Browser_Bookmark_View(void);
	~Browser_Bookmark_View(void);

	Eina_Bool init(void);
	void return_to_bookmark_view(int added_bookmark_id = -1);
	Eina_Bool append_bookmark_item(const char *title, const char *url);
	void delete_bookmark_item(int bookmark_id);
protected:
	void history_item_clicked(const char *url);
private:
	typedef enum _view_mode {
		BOOKMARK_VIEW	= 0,
		HISTORY_VIEW
	} view_mode;

	typedef enum _controlbar_type {
		BOOKMARK_VIEW_DEFAULT	= 0,
		BOOKMARK_VIEW_EDIT_MODE,
		HISTORY_VIEW_DEFAULT,
		HISTORY_VIEW_EDIT_MODE
	} controlbar_type;

	Eina_Bool _create_main_layout(void);
	Evas_Object *_create_main_folder_genlist(void);
	Evas_Object *_create_sub_folder_genlist(int folder_id);
	void _go_to_sub_foler(const char *folder_name);
	void _go_up_to_main_folder(void);
	void _set_navigationbar_title(const char *title);
	void _set_view_mode(view_mode mode);
	void _set_edit_mode(Eina_Bool edit_mode);
	void _show_selection_info(void);
	void _delete_selected_bookmark(void);
	void _show_empty_content_layout(Eina_Bool is_empty);
	void _reorder_bookmark_item(int order_index, Eina_Bool is_move_down);
	Eina_Bool _is_empty_folder(string folder_name);
	Evas_Object *_get_current_folder_genlist(void);
	void _delete_bookmark_item_from_folder_list(Browser_Bookmark_DB::bookmark_item *item);
	vector<Browser_Bookmark_DB::bookmark_item *> _get_current_folder_item_list(void);
	void _show_select_processing_popup(void);
	void _show_delete_processing_popup(void);
	Evas_Object *_show_delete_confirm_popup(void);
	void _delete_bookmark_item_by_slide_button(Browser_Bookmark_DB::bookmark_item *item);
	Eina_Bool _set_controlbar_type(controlbar_type type);

	/* ecore timer callback functions */
	static Eina_Bool __select_processing_popup_timer_cb(void *data);
	static Eina_Bool __delete_processing_popup_timer_cb(void *data);

	/* evas object event callback functions. */
	static void __edit_mode_select_all_clicked_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);

	/* elementary event callback functions. */
	static void __select_processing_popup_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_processing_popup_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __slide_edit_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __slide_delete_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __slide_share_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_bookmark_item_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __rename_folder_unfocus_cb(void *data, Evas_Object *obj, void *event_info);
	static void __rename_folder_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info);
	static void __rename_folder_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_mode_item_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_mode_select_all_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __upper_folder_clicked_cb(void *data, Evas_Object *obj, void *eventInfo);
	static void __drag_genlist_cb(void *data, Evas_Object *obj, void *event_info);
#if defined(GENLIST_SWEEP)
	static void __sweep_cancel_genlist_cb(void *data, Evas_Object *obj, void *event_info);
	static void __sweep_left_genlist_cb(void *data, Evas_Object *obj, void *event_info);
	static void __sweep_right_genlist_cb(void *data, Evas_Object *obj, void *event_info);
#endif
	static void __back_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_controlbar_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __controlbar_tab_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __bookmark_item_clicked_cb(void *data, Evas_Object *obj, void *eventInfo);
	static void __delete_confirm_response_by_slide_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_confirm_response_by_slide_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_confirm_response_by_edit_mode_cb(void *data, Evas_Object *obj, void *event_info);
	static void __create_folder_button(void *data, Evas_Object *obj, void *event_info);
	static void __delete_controlbar_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_to_bookmark_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info);

	/* genlist callback functions. */
	static char *__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_move_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_conformant;
	Evas_Object *m_main_folder_genlist;
	Evas_Object *m_sub_folder_conformant;
	Evas_Object *m_sub_folder_genlist;
	Evas_Object *m_content_layout;
	Evas_Object *m_main_layout;
	Evas_Object *m_sub_main_layout;
	Evas_Object *m_genlist_content_layout;
	Evas_Object *m_genlist_content_box;
	Evas_Object *m_top_control_bar;
	Evas_Object *m_bottom_control_bar;
	Evas_Object *m_back_button;
	Evas_Object *m_upper_folder_list;
	Evas_Object *m_upper_folder_icon;
	Evas_Object *m_edit_mode_select_all_layout;
	Evas_Object *m_edit_mode_select_all_check_button;
	Evas_Object *m_no_contents;
	Evas_Object *m_rename_edit_field;

	Elm_Object_Item *m_bookmark_delete_controlbar_item;
	Elm_Object_Item *m_bookmark_edit_controlbar_item;
	Elm_Object_Item *m_create_folder_controlbar_item;

	Elm_Genlist_Item_Class m_bookmark_genlist_item_class;
	Elm_Genlist_Item_Class m_folder_genlist_item_class;

	vector<Browser_Bookmark_DB::bookmark_item *> m_main_folder_list;
	vector<Browser_Bookmark_DB::bookmark_item *> m_sub_folder_list;

	view_mode m_view_mode;
	int m_current_folder_id;
	string m_current_bookmark_navigationbar_title;
	Elm_Object_Item *m_current_genlist_item_to_edit;

	/* For select all processing popup. */
	Evas_Object *m_processing_progress_bar;
	Ecore_Timer *m_processing_popup_timer;
	Elm_Object_Item *m_processed_it;
	int m_processed_count;
	int m_total_item_count;
	Evas_Object *m_processing_popup;
	Evas_Object *m_processing_popup_layout;
	Eina_Bool m_select_all_check_value;

	Elm_Object_Item *m_current_sweep_item;
	Elm_Object_Item *m_navi_it;

	Evas_Object *m_delete_confirm_popup;
};
#endif /* BROWSER_BOOKMARK_VIEW_H */

