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


#ifndef BROWSER_HISTORY_LAYOUT_H
#define BROWSER_HISTORY_LAYOUT_H

#include "browser-common-view.h"
#include "browser-config.h"
#include "browser-history-db.h"
#include "browser-bookmark-view.h"

class Date {
public:
	int year;
	int month;
	int day;

	Date();
	Date(Date &date);
	void operator=(Date &date);
	bool operator==(Date &date);
	bool operator!=(Date &date);
};

class Browser_History_Layout : public Browser_Common_View {
	friend class Browser_Bookmark_View;
public:
	Browser_History_Layout(void);
	~Browser_History_Layout(void);

	Eina_Bool init(void);
	Evas_Object *get_main_layout(void) { return m_sub_main_history_layout; }
protected:
	void _set_edit_mode(Eina_Bool edit_mode);

private:
	typedef struct _history_date_param {
		Date date;
		Browser_History_Layout *history_layout;
	} history_date_param;

	Eina_Bool _create_main_layout(void);
	Evas_Object *_create_history_genlist(void);
	void _reload_history_genlist(void);
	void _show_selection_info(void);
	void _delete_selected_history(void);
	Evas_Object *_show_delete_confirm_popup(void);
	void _delete_history_item_by_slide_button(Browser_History_DB::history_item *item);
	void _show_select_processing_popup(void);
	void _show_delete_processing_popup(void);
	void _enable_searchbar_layout(Eina_Bool enable);
	Eina_Bool _show_searched_history(const char *search_text);
	void _delete_date_only_label_genlist_item(void);

	/* elementary event callback functions. */
#if defined(GENLIST_SWEEP)
	static void __sweep_left_genlist_cb(void *data, Evas_Object *obj, void *event_info);
	static void __sweep_right_genlist_cb(void *data, Evas_Object *obj, void *event_info);
	static void __sweep_cancel_genlist_cb(void *data, Evas_Object *obj, void *event_info);
#endif
	static void __history_item_clicked_cb(void *data, Evas_Object *obj, void *eventInfo);
	static void __edit_mode_item_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_mode_select_all_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __slide_add_to_bookmark_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __slide_share_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __slide_delete_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_confirm_response_by_edit_mode_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_confirm_response_by_slide_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_confirm_response_by_slide_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_processing_popup_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_processing_popup_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __search_delay_changed_cb(void *data, Evas_Object *obj, void *event_info);

	static void __bookmark_on_off_icon_clicked_cb(void* data, Evas* evas, Evas_Object* obj, void* ev);

	/* evas object event callback functions */
	static void __edit_mode_select_all_clicked_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);

	/* genlist callback functions. */
	static char *__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static char *__genlist_date_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);

	/* ecore timer callback functions */
	static Eina_Bool __select_processing_popup_timer_cb(void *data);
	static Eina_Bool __delete_processing_popup_timer_cb(void *data);

	history_date_param m_date_param;
	Elm_Genlist_Item_Class m_history_genlist_item_class;
	Elm_Genlist_Item_Class m_history_group_title_class;

	Elm_Object_Item *m_current_sweep_item;

	Evas_Object *m_sub_main_history_layout;
	Evas_Object *m_searchbar_layout;
	Evas_Object *m_searchbar;
	Evas_Object *m_history_genlist;
	Evas_Object *m_searched_history_genlist;
	Evas_Object *m_no_content_search_result;
	Evas_Object *m_content_box;
	Evas_Object *m_edit_mode_select_all_layout;
	Evas_Object *m_edit_mode_select_all_check_button;
	Evas_Object *m_no_history_label;

	vector<Browser_History_DB::history_item *> m_history_list;
	vector<char *> m_history_date_label_list;
	vector<Browser_History_DB::history_item *> m_searched_history_item_list;
	vector<char *> m_searched_history_date_label_list;
	Date m_last_date;

	/* For select all processing popup. */
	Evas_Object *m_processing_progress_bar;
	Ecore_Timer *m_processing_popup_timer;
	Elm_Object_Item *m_processed_it;
	int m_processed_count;
	Evas_Object *m_processing_popup;
	Evas_Object *m_processing_popup_layout;
	Eina_Bool m_select_all_check_value;
	int m_total_item_count;

	Evas_Object *m_delete_confirm_popup;
	Eina_Bool m_is_bookmark_on_off_icon_clicked;
};
#endif /* BROWSER_HISTORY_LAYOUT_H */

