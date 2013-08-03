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

/**
  *@file most-visited-sites.cpp
  *@brief 最常访问界面实现
  */

#include "browser-add-to-bookmark-view.h"
#include "add-to-most-visited-sites-view.h"
#include "browser-data-manager.h"
#include "most-visited-sites.h"
#include "most-visited-sites-db.h"
#include "browser-view.h"
#include "browser-common-view.h"

#define MOST_VISITED_SITES_ITEM_WIDTH	(226 * elm_scale_get())
#define MOST_VISITED_SITES_ITEM_HEIGHT	(273 * elm_scale_get())

Most_Visited_Sites::Most_Visited_Sites(Evas_Object *navi_bar, Browser_View *browser_view,
						Most_Visited_Sites_DB *most_visited_sites_db)
:
	m_navi_bar(navi_bar)
	,m_browser_view(browser_view)
	,m_gengrid(NULL)
	,m_most_visited_sites_db(most_visited_sites_db)
	,m_selected_item(NULL)
	,m_context_popup(NULL)
	,m_is_drag(EINA_FALSE)
	,m_guide_text_main_layout(NULL)
	,m_guide_text_label(NULL)
	,m_never_show_check_box(NULL)
	,m_never_show_text_label(NULL)
	,m_ok_button(NULL)
	,m_main_layout(NULL)
	,m_setting_button(NULL)
	,m_edit_mode(EINA_FALSE)
	,m_done_button(NULL)
	,m_is_item_clicked(EINA_FALSE)
//	,m_gengrid_item_class(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
	m_item_list.clear();
}

Most_Visited_Sites::~Most_Visited_Sites(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_main_layout)
		evas_object_del(m_main_layout);

//	if (m_gengrid_item_class)
//		elm_gengrid_item_class_free(m_gengrid_item_class);

	m_entry_list.clear();
	m_most_visited_list.clear();

	for(int i = 0 ; i < m_item_list.size() ; i++) {
		if (m_item_list[i]) {
			if (m_item_list[i]->screen_shot)
				evas_object_del(m_item_list[i]->screen_shot);
			if (m_item_list[i]->layout)
				evas_object_del(m_item_list[i]->layout);
			if (m_item_list[i]->history_id)
				free(m_item_list[i]->history_id);
			if (m_item_list[i]->url)
				free(m_item_list[i]->url);
			if (m_item_list[i]->title)
				free(m_item_list[i]->title);
			delete m_item_list[i];
		}
	}
}


Evas_Object *Most_Visited_Sites::__get_most_visited_sites_item_layout(most_visited_sites_item *item)
{
	if (!elm_layout_file_set(item->layout, BROWSER_EDJE_DIR"/most-visited-sites.edj",
								"most_visited_sites_item")) {
		BROWSER_LOGE("elm_layout_file_set failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(item->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(item->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	item->screen_shot = elm_icon_add(item->layout);
	if (!item->screen_shot) {
		BROWSER_LOGE("elm_icon_add failed");
		return NULL;
	}

	if (item->history_id) {
		std::string screen_shot_file_path;
		screen_shot_file_path = std::string(BROWSER_SCREEN_SHOT_DIR)
							+ std::string(item->history_id);
		if (!elm_icon_file_set(item->screen_shot, screen_shot_file_path.c_str(), NULL)) {
			/* If no screen shot, set the no image default icon. */
			if (!item->tack) {
				/* If the item is from history. */
				Elm_Object_Item *it = elm_gengrid_first_item_get(m_gengrid);
				do {
					most_visited_sites_item* p_item = (most_visited_sites_item*)elm_object_item_data_get(it);
					if (p_item && p_item->tack) {
						if (p_item->url && item->url && (strlen(p_item->url) == strlen(item->url))
						    && !strncmp(p_item->url, item->url, strlen(p_item->url))) {
							if (p_item->history_id) {
								screen_shot_file_path.clear();
								screen_shot_file_path = std::string(BROWSER_SCREEN_SHOT_DIR)
												+ std::string(p_item->history_id);
								elm_icon_file_set(item->screen_shot,
										screen_shot_file_path.c_str(), NULL);
							}
							break;
						}
					}
				} while (it = elm_gengrid_item_next_get(it));
			}
		}
		if (item->url) {
			elm_object_part_content_set(item->layout, "elm.swallow.snapshot", item->screen_shot);
			//evas_object_smart_callback_add(item->screen_shot, "clicked", __item_clicked_cb, item);
		}
	}
	else {
		/* If empty slot. */
		if (!elm_layout_file_set(item->layout, BROWSER_EDJE_DIR"/most-visited-sites.edj",
									"most_visited_sites_empty_item")) {
			BROWSER_LOGE("elm_layout_file_set failed");
			return NULL;
		}
		evas_object_size_hint_weight_set(item->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(item->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		//evas_object_smart_callback_add(item->screen_shot, "clicked", __empty_item_clicked_cb, item);
	}

	if (item->url) {
		if (item->title) {
			edje_object_part_text_set(elm_layout_edje_get(item->layout),
							"title_label", item->title);
			edje_object_signal_emit(elm_layout_edje_get(item->layout),
							"titlearea", "");
		}
		if (m_most_visited_sites_db->is_in_bookmark(item->url, NULL))
			edje_object_signal_emit(elm_layout_edje_get(item->layout), "bookmark_icon,on,signal", "");
		if (item->tack)
			edje_object_signal_emit(elm_layout_edje_get(item->layout), "tack_icon", "");
		else
			edje_object_signal_emit(elm_layout_edje_get(item->layout), "hide,pin_icon", "");
	}
	evas_object_show(item->screen_shot);
	evas_object_show(item->layout);

	edje_object_signal_callback_add(elm_layout_edje_get(item->layout), "mouse,clicked,1",
			"elm.image.delete_button", __delete_button_clicked_cb, item);

	edje_object_signal_callback_add(elm_layout_edje_get(item->layout), "mouse,clicked,1",
			"bookmark_icon_bg", __bookmark_button_clicked_cb, item);

	if (m_edit_mode)
		edje_object_signal_emit(elm_layout_edje_get(item->layout), "show,delete_button,signal", "");

	return item->layout;
}

void Most_Visited_Sites::__bookmark_button_clicked_cb(void *data, Evas_Object *obj,
							const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);

	most_visited_sites_item *item = (most_visited_sites_item *)data;
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)(item->data);

	if (!most_visited_sites->m_edit_mode)
		return;

	Browser_Bookmark_DB *bookmark_db = m_data_manager->create_bookmark_db();
	if (!bookmark_db) {
		BROWSER_LOGE("create_bookmark_db failed");
		return;
	}

	int bookmark_id = -1;
	if (most_visited_sites->m_most_visited_sites_db->is_in_bookmark(item->url, &bookmark_id)) {
		edje_object_signal_emit(elm_layout_edje_get(item->layout), "bookmark_icon,off,signal", "");
		if (bookmark_id != -1)
			bookmark_db->delete_bookmark(bookmark_id);
		most_visited_sites->m_browser_view->show_notify_popup(BR_STRING_REMOVED_TO_BOOKMARKS, 3, EINA_TRUE);
	} else {
		edje_object_signal_emit(elm_layout_edje_get(item->layout), "bookmark_icon,on,signal", "");
		if (item->title && item->url)
			bookmark_db->save_bookmark(BROWSER_BOOKMARK_MAIN_FOLDER_ID, item->title, item->url);
		most_visited_sites->m_browser_view->show_notify_popup(BR_STRING_ADDED_TO_BOOKMARKS, 3, EINA_TRUE);
	}

	m_data_manager->destroy_bookmark_db();
}

void Most_Visited_Sites::__delete_button_clicked_cb(void *data, Evas_Object *obj,
							const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);

	most_visited_sites_item *item = (most_visited_sites_item *)data;
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)(item->data);

	if (!most_visited_sites->_delete_selected_item())
		BROWSER_LOGE("_delete_selected_item failed");
}

Eina_Bool Most_Visited_Sites::_empty_item_clicked(most_visited_sites_item *item)
{
	BROWSER_LOGD("[%s]", __func__);
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)(item->data);
	Browser_Data_Manager *data_manager = most_visited_sites->m_browser_view->get_data_manager();
	Add_To_Most_Visited_Sites_View* add_to_most_visited_sites = NULL;

	if (m_is_drag)
		return EINA_FALSE;

	add_to_most_visited_sites = data_manager->create_add_to_most_visited_sites_view(most_visited_sites);
	if (!add_to_most_visited_sites) {
		BROWSER_LOGE("create_add_to_most_visited_sites_view failed");
		return EINA_FALSE;
	}
	if (!add_to_most_visited_sites->init()) {
		BROWSER_LOGE("add_to_most_visited_sites->init failed");
		data_manager->destroy_add_to_most_visited_sites_view();
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

/*void Most_Visited_Sites::__empty_item_clicked_cb(void *data, Evas_Object *obj,
								void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	most_visited_sites_item *item = (most_visited_sites_item *)data;
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)(item->data);
	if (!most_visited_sites->_empty_item_clicked(item))
		BROWSER_LOGE("_empty_item_clicked failed");
}*/

void Most_Visited_Sites::__item_press_cb(void *data, Evas *evas, Evas_Object *obj,
									void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!data)
		return;
	most_visited_sites_item *item = (most_visited_sites_item *)data;
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)(item->data);
	edje_object_signal_emit(elm_layout_edje_get(item->layout), "tile_pressed", "");
	most_visited_sites->m_selected_item = item;
}

void Most_Visited_Sites::__item_release_cb(void *data, Evas *evas, Evas_Object *obj,
									void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!data)
		return;
	most_visited_sites_item *item = (most_visited_sites_item *)data;
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)(item->data);
	edje_object_signal_emit(elm_layout_edje_get(item->layout), "tile_released", "");

	most_visited_sites->m_is_drag = EINA_FALSE;
}

Eina_Bool Most_Visited_Sites::__load_url_idler_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!data) {
		BROWSER_LOGE("data is NULL");
		return EINA_FALSE;
	}

	most_visited_sites_item *item = (most_visited_sites_item *)data;
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)(item->data);

	if (!item->url){
		BROWSER_LOGE("url in Most_Visited_Sites class is NULL");
		return EINA_FALSE;
	}

	std::string selected_url = std::string(item->url);
	BROWSER_LOGD("selected_url=[%s]", selected_url.c_str());
	most_visited_sites->m_browser_view->load_url(selected_url.c_str());

	return ECORE_CALLBACK_CANCEL;
}

void Most_Visited_Sites::__item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	most_visited_sites_item *item = (most_visited_sites_item *)data;
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)(item->data);

	if (most_visited_sites->m_is_item_clicked) {
		BROWSER_LOGE("item is already clicked");
		return;
	}

	if (!item->history_id || !item->url) {
		BROWSER_LOGE("empty item");
		return;
	}

	most_visited_sites->m_selected_item = item;

	if (most_visited_sites->m_edit_mode) {
		BROWSER_LOGE("edit mode");
		elm_gengrid_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

#if 0
	if (!most_visited_sites->m_selected_item) {
		BROWSER_LOGE("most_visited_sites->m_selected_item is null");
		return;
	}
#endif
	edje_object_signal_emit(elm_layout_edje_get(item->layout), "tile_released", "");

	elm_gengrid_item_selected_set(most_visited_sites->m_selected_item->item, EINA_FALSE);

	if (!most_visited_sites->m_is_drag) {
		if (item->history_id && item->url) {
			most_visited_sites->m_browser_view->m_selected_most_visited_sites_item_info.id.clear();
			most_visited_sites->m_browser_view->m_selected_most_visited_sites_item_info.title.clear();
			most_visited_sites->m_browser_view->m_selected_most_visited_sites_item_info.url.clear();

			if (item->history_id && strlen(item->history_id))
				most_visited_sites->m_browser_view->m_selected_most_visited_sites_item_info.id = std::string(item->history_id);

			int index = 0;
			Elm_Object_Item *it = elm_gengrid_first_item_get(most_visited_sites->m_gengrid);
			do {
				if (it == most_visited_sites->m_selected_item->item)
					break;
				index++;
			} while (it = elm_gengrid_item_next_get(it));

			most_visited_sites->m_browser_view->m_selected_most_visited_sites_item_info.index = index;
			if (item->title && strlen(item->title))
				most_visited_sites->m_browser_view->m_selected_most_visited_sites_item_info.title = std::string(item->title);
			if (item->url && strlen(item->url))
				most_visited_sites->m_browser_view->m_selected_most_visited_sites_item_info.url = std::string(item->url);

			std::string selected_url = std::string(item->url);
			/* m_browser_view->load_url should be called after accessing most_visited_sites instance.
				It deletes the most_visited_sites instance. The most_visited_sites pointer is invalid after calling load_url(). */
			most_visited_sites->m_is_item_clicked = EINA_TRUE;
			ecore_idler_add(__load_url_idler_cb, item);
		}
	}
}

void Most_Visited_Sites::__context_popup_dismissed_cb(void *data, Evas_Object *obj,
									void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)data;
	if (most_visited_sites->m_context_popup) {
		evas_object_del(most_visited_sites->m_context_popup);
		most_visited_sites->m_context_popup = NULL;
	}
}

void Most_Visited_Sites::__context_popup_unpin_clicked_cb(void *data, Evas_Object *obj,
									void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)data;
	if (most_visited_sites->m_context_popup) {
		evas_object_del(most_visited_sites->m_context_popup);
		most_visited_sites->m_context_popup = NULL;
	}

	int index = 0;
	Elm_Object_Item *it = elm_gengrid_first_item_get(most_visited_sites->m_gengrid);
	do {
		if (it == most_visited_sites->m_selected_item->item)
			break;
		index++;
	} while (it = elm_gengrid_item_next_get(it));

	if (!most_visited_sites->m_most_visited_sites_db->delete_most_visited_sites_item(most_visited_sites->m_selected_item->url)) {
		BROWSER_LOGE("delete_most_visited_sites_item failed");
		return;
	}

	edje_object_signal_emit(elm_layout_edje_get(most_visited_sites->m_selected_item->layout),
								"hide,pin_icon", "");
	most_visited_sites->m_selected_item->tack = EINA_FALSE;

	std::string history_id;
	history_id = most_visited_sites->m_most_visited_sites_db->get_history_id_by_url(most_visited_sites->m_selected_item->url);
	BROWSER_LOGD("history id=[%s]", history_id.c_str());
	if (history_id.empty()) {
		std::string screen_shot_path;
		screen_shot_path.clear();
		screen_shot_path = std::string(BROWSER_SCREEN_SHOT_DIR)
					+ std::string(most_visited_sites->m_selected_item->history_id);
		remove(screen_shot_path.c_str());
		most_visited_sites->_reload_items();
	}
}

void Most_Visited_Sites::__context_popup_pin_clicked_cb(void *data, Evas_Object *obj,
									void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)data;
	if (most_visited_sites->m_context_popup) {
		evas_object_del(most_visited_sites->m_context_popup);
		most_visited_sites->m_context_popup = NULL;
	}

	int index = 0;
	Elm_Object_Item *it = elm_gengrid_first_item_get(most_visited_sites->m_gengrid);
	do {
		if (it == most_visited_sites->m_selected_item->item)
			break;
		index++;
	} while (it = elm_gengrid_item_next_get(it));

	if (!most_visited_sites->m_most_visited_sites_db->save_most_visited_sites_item(index, most_visited_sites->m_selected_item->url,
							most_visited_sites->m_selected_item->title,
							most_visited_sites->m_selected_item->history_id)) {
		BROWSER_LOGE("save_most_visited_sites_item failed");
		return;
	}

	most_visited_sites->m_selected_item->tack = EINA_TRUE;
	edje_object_signal_emit(elm_layout_edje_get(most_visited_sites->m_selected_item->layout),
								"tack_icon", "");
}

void Most_Visited_Sites::__context_popup_modify_clicked_cb(void *data, Evas_Object *obj,
									void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)data;
	if (most_visited_sites->m_context_popup) {
		evas_object_del(most_visited_sites->m_context_popup);
		most_visited_sites->m_context_popup = NULL;
	}

	Browser_Data_Manager *data_manager = most_visited_sites->m_browser_view->get_data_manager();
	Add_To_Most_Visited_Sites_View* add_to_most_visited_sites_view = NULL;
	add_to_most_visited_sites_view = data_manager->create_add_to_most_visited_sites_view(most_visited_sites);
	if (!add_to_most_visited_sites_view) {
		BROWSER_LOGE("create_add_to_most_visited_sites_view failed");
		return;
	}
	if (!add_to_most_visited_sites_view->init()) {
		BROWSER_LOGE("add_to_most_visited_sites_view->init failed");
		data_manager->destroy_add_to_most_visited_sites_view();
		return;
	}
}

Eina_Bool Most_Visited_Sites::_delete_selected_item(void)
{
	BROWSER_LOGD("[%s]", __func__);

	Eina_Bool is_ranked = EINA_FALSE;

	if (!m_selected_item || !m_selected_item->item) {
		BROWSER_LOGD("m_selected_item || m_selected_item->item is NULL");
		return EINA_FALSE;
	}

	if (m_selected_item->tack) {
		int index = 0;
		Elm_Object_Item *it = elm_gengrid_first_item_get(m_gengrid);
		do {
			if (it == m_selected_item->item)
				break;
			index++;
		} while (it = elm_gengrid_item_next_get(it));


		BROWSER_LOGD("delete_most_visited_sites_item index = %d", m_selected_item->item);
		if (!m_most_visited_sites_db->delete_most_visited_sites_item(m_selected_item->url)) {
			BROWSER_LOGE("delete_most_visited_sites_item failed");
			return EINA_FALSE;
		}

		std::string history_id;
		history_id = m_most_visited_sites_db->get_history_id_by_url(m_selected_item->url);

		BROWSER_LOGD("delete_most_visited_sites_item history_id = %s, atoi(m_selected_item->history_id) = %d", history_id.c_str(), atoi(m_selected_item->history_id));
		if (!m_most_visited_sites_db->delete_most_visited_sites_history_item(atoi(m_selected_item->history_id))) {
			BROWSER_LOGE("delete_most_visited_sites_history_item failed");
			return EINA_FALSE;
		}

		is_ranked = EINA_TRUE;
	} else {
		BROWSER_LOGD("delete_most_visited_sites_item atoi(m_selected_item->history_id) = %d", atoi(m_selected_item->history_id));
		if (!m_most_visited_sites_db->delete_most_visited_sites_history_item(atoi(m_selected_item->history_id))) {
			BROWSER_LOGE("delete_most_visited_sites_history_item failed");
			return EINA_FALSE;
		}

		is_ranked = EINA_TRUE;

		Elm_Object_Item *it = elm_gengrid_first_item_get(m_gengrid);
		do {
			most_visited_sites_item* p_item = (most_visited_sites_item *)elm_object_item_data_get(it);
			if (p_item->tack) {    // only search in tacked items. (mysites items)
				if (m_selected_item->url && p_item->url && (strlen(m_selected_item->url) == strlen(p_item->url))
				    && !strcmp(m_selected_item->url, p_item->url)) {
					is_ranked = EINA_FALSE;
					break;
				}
			}
		} while (it = elm_gengrid_item_next_get(it));
	}

	if (is_ranked) {
		BROWSER_LOGD("remove screen shot [%s]", m_selected_item->url);
		std::string screen_shot_path;
		screen_shot_path = std::string(BROWSER_SCREEN_SHOT_DIR)
					+ std::string(m_selected_item->history_id);
		remove(screen_shot_path.c_str());
	}

	elm_object_item_del(m_selected_item->item);

	most_visited_sites_item *item = new(nothrow) most_visited_sites_item;
	if (!item) {
		BROWSER_LOGE("new(nothrow) most_visited_sites_item is failed");
		return EINA_FALSE;
	}
	item->history_id = NULL;
	item->url = NULL;
	item->title = NULL;
	item->tack = EINA_FALSE;
	item->layout = NULL;
	item->screen_shot = NULL;
	item->data = (void *)this;
	item->item = elm_gengrid_item_append(m_gengrid, &m_gengrid_item_class, item, __item_clicked_cb, item);
	m_item_list.push_back(item);
#if 0
	if (!_reload_items()) {
		BROWSER_LOGE("_reload_items failed");
		return EINA_FALSE;
	}
#endif
	return EINA_TRUE;
}

void Most_Visited_Sites::__context_popup_delete_clicked_cb(void *data, Evas_Object *obj,
									void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)data;
	if (most_visited_sites->m_context_popup) {
		evas_object_del(most_visited_sites->m_context_popup);
		most_visited_sites->m_context_popup = NULL;
	}

	if (!most_visited_sites->_delete_selected_item())
		BROWSER_LOGE("_delete_selected_item failed");
}

void Most_Visited_Sites::__context_popup_bookmark_clicked_cb(void *data, Evas_Object *obj,
									void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)data;
	if (most_visited_sites->m_context_popup) {
		evas_object_del(most_visited_sites->m_context_popup);
		most_visited_sites->m_context_popup = NULL;
	}

	Browser_Data_Manager *data_manager = most_visited_sites->m_browser_view->get_data_manager();
	std::string url = std::string(most_visited_sites->m_selected_item->url);
	std::string title = std::string(most_visited_sites->m_selected_item->title);

	if (!data_manager->create_add_to_bookmark_view(title, url)) {
		BROWSER_LOGE("create_add_to_bookmark_view failed");
		return;
	}

	if (!data_manager->get_add_to_bookmark_view()->init())
		data_manager->destroy_add_to_bookmark_view();
}

Eina_Bool Most_Visited_Sites::_show_item_context_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!m_selected_item || !m_selected_item->item
	    || !m_selected_item->history_id) {
		BROWSER_LOGE("Invalid item");
		return EINA_FALSE;
	}
	m_context_popup = elm_ctxpopup_add(m_gengrid);
	if (!m_context_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_context_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_ctxpopup_hover_parent_set(m_context_popup, m_gengrid);
	elm_ctxpopup_direction_priority_set(m_context_popup, ELM_CTXPOPUP_DIRECTION_LEFT,
					ELM_CTXPOPUP_DIRECTION_RIGHT, ELM_CTXPOPUP_DIRECTION_UP,
					ELM_CTXPOPUP_DIRECTION_DOWN);

	evas_object_smart_callback_add(m_context_popup,"dismissed",
						__context_popup_dismissed_cb, this);

	if (m_selected_item->tack)
		elm_ctxpopup_item_append(m_context_popup, BR_STRING_UNLOCK, NULL,
					__context_popup_unpin_clicked_cb, this);
	else
		elm_ctxpopup_item_append(m_context_popup, BR_STRING_LOCK, NULL,
					__context_popup_pin_clicked_cb, this);

	elm_ctxpopup_item_append(m_context_popup, BR_STRING_MODIFY, NULL,
					__context_popup_modify_clicked_cb, this);
	elm_ctxpopup_item_append(m_context_popup, BR_STRING_BOOKMARK, NULL,
					__context_popup_bookmark_clicked_cb, this);
	elm_ctxpopup_item_append(m_context_popup, BR_STRING_DELETE, NULL,
					__context_popup_delete_clicked_cb, this);

	Evas_Coord item_x = 0;
	Evas_Coord item_y = 0;
	Evas_Coord item_w = 0;
	Evas_Coord item_h = 0;
	evas_object_geometry_get(m_selected_item->layout, &item_x, &item_y, &item_w, &item_h);
	evas_object_move(m_context_popup, item_x + (item_w / 3), item_y + (item_h / 4));
	evas_object_show(m_context_popup);

	return EINA_TRUE;
}

void Most_Visited_Sites::__item_longpressed_cb(void *data, Evas_Object *obj,
								void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	most_visited_sites_item *item = (most_visited_sites_item *)elm_object_item_data_get(it);
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)data;

	most_visited_sites->m_is_drag = EINA_TRUE;
	most_visited_sites->m_selected_item = item;

#if 0
	edje_object_signal_emit(elm_layout_edje_get(most_visited_sites->m_selected_item->layout), "tile_released", "");

	if (!most_visited_sites->_show_item_context_popup())
		BROWSER_LOGE("_show_item_context_popup failed");
#endif
}

void Most_Visited_Sites::__item_drag_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)data;
	static int count = 0;
	count++;
	/* Workaround.
	  * If user drags the item, destroy the context popup.
	  * Let's assume that if the 'changed' callback is called more than 15,
	  * the user may drag the item to reoder. */
	if (count > 15) {
		if (most_visited_sites->m_context_popup) {
			evas_object_del(most_visited_sites->m_context_popup);
			most_visited_sites->m_context_popup = NULL;
		}
		count = 0;
	}
}

Eina_Bool Most_Visited_Sites::__gengrid_icon_get_idler_cb(void *data)
{
	most_visited_sites_item *item = (most_visited_sites_item *)data;
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)(item->data);
	if (!most_visited_sites->__get_most_visited_sites_item_layout(item))
		BROWSER_LOGE("__get_most_visited_sites_item_layout failed");

	return ECORE_CALLBACK_CANCEL;
}

Evas_Object *Most_Visited_Sites::__gengrid_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	most_visited_sites_item *item = (most_visited_sites_item *)data;
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)(item->data);
	if (!strncmp(part, "elm.swallow.icon", strlen("elm.swallow.icon"))) {
		item->layout = elm_layout_add(obj);
		if (!item->layout) {
			BROWSER_LOGE("elm_layout_add failed");
			return NULL;
		}
		ecore_idler_add(__gengrid_icon_get_idler_cb, item);
		return item->layout;
	}
	return NULL;
}

Eina_Bool Most_Visited_Sites::_reload_items(void)
{
	BROWSER_LOGD("[%s]", __func__);
	elm_gengrid_clear(m_gengrid);
	m_entry_list.clear();
	m_most_visited_list.clear();

	for(int i = 0 ; i < m_item_list.size() ; i++) {
		if (m_item_list[i]) {
			if (m_item_list[i]->screen_shot)
				evas_object_del(m_item_list[i]->screen_shot);
			if (m_item_list[i]->layout)
				evas_object_del(m_item_list[i]->layout);
			if (m_item_list[i]->history_id)
				free(m_item_list[i]->history_id);
			if (m_item_list[i]->url)
				free(m_item_list[i]->url);
			if (m_item_list[i]->title)
				free(m_item_list[i]->title);
			delete m_item_list[i];
		}
	}
	m_item_list.clear();

	if (!m_most_visited_sites_db->get_most_visited_sites_list(m_entry_list)) {
		BROWSER_LOGE("get_most_visited_sites_list failed");
		return EINA_FALSE;
	}

	int item_count = BROWSER_MOST_VISITED_SITES_ITEM_MAX;
	item_count = item_count - m_entry_list.size();
	if (!m_most_visited_sites_db->get_most_visited_list(m_most_visited_list, m_entry_list, item_count)) {
		BROWSER_LOGE("get_most_visited_list failed");
		return EINA_FALSE;
	}

	std::vector<Most_Visited_Sites_DB::most_visited_sites_entry>::iterator most_visited_sites_iterator;
	std::vector<Most_Visited_Sites_DB::most_visited_sites_entry>::iterator most_visited_iterator;

	most_visited_sites_iterator = m_entry_list.begin();
	most_visited_iterator = m_most_visited_list.begin();

	int entry_index = 0;
	int most_visited_index = 0;

	for (int i = 0 ; i < BROWSER_MOST_VISITED_SITES_ITEM_MAX ; i++) {
		most_visited_sites_item *item = new(nothrow) most_visited_sites_item;
		if (!item) {
			BROWSER_LOGE("new most_visited_sites_item failed");
			return EINA_FALSE;
		}
		if (entry_index < m_entry_list.size())/* && m_entry_list[entry_index].index == i) */{
			item->history_id = strdup(m_entry_list[entry_index].id.c_str());
			item->url = strdup(m_entry_list[entry_index].url.c_str());
			item->title = strdup(m_entry_list[entry_index].title.c_str());
			item->tack = EINA_TRUE;
			entry_index++;
		} else if (most_visited_index < m_most_visited_list.size()) {
			item->history_id = strdup(m_most_visited_list[most_visited_index].id.c_str());
			item->url = strdup(m_most_visited_list[most_visited_index].url.c_str());
			item->title = strdup(m_most_visited_list[most_visited_index].title.c_str());
			item->tack = EINA_FALSE;
			most_visited_index++;
		} else {
			item->history_id = NULL;
			item->url = NULL;
			item->title = NULL;
			item->tack = EINA_FALSE;
		}
		item->layout = NULL;
		item->screen_shot = NULL;
		item->data = (void *)this;
		item->item = elm_gengrid_item_append(m_gengrid, &m_gengrid_item_class, item, __item_clicked_cb, item);
		m_item_list.push_back(item);
	}

	return EINA_TRUE;
}

Eina_Bool Most_Visited_Sites::_item_moved(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_selected_item->url && strlen(m_selected_item->url)) {
		if (!m_context_popup) {
			m_selected_item->tack = EINA_TRUE;
			edje_object_signal_emit(elm_layout_edje_get(m_selected_item->layout),
									"tack_icon", "");
		}
	}

	int index = 0;
	Elm_Object_Item *it = elm_gengrid_first_item_get(m_gengrid);
	do {
		if (!m_most_visited_sites_db->delete_most_visited_sites_item(m_selected_item->url))
			BROWSER_LOGE("fail to delete slot %d", index);

		most_visited_sites_item *item = (most_visited_sites_item *)elm_object_item_data_get(it);
		if (item && item->tack) {
			if (!m_most_visited_sites_db->save_most_visited_sites_item(index, item->url,
							item->title, item->history_id)) {
				BROWSER_LOGE("save_most_visited_sites_item failed");
			}
		}
		index++;
	} while (it = elm_gengrid_item_next_get(it));

	m_is_drag = EINA_FALSE;

	return EINA_TRUE;
}

void Most_Visited_Sites::__item_moved_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)data;
	if (!most_visited_sites->_item_moved())
		BROWSER_LOGE("_item_moved failed");
}

Eina_Bool Most_Visited_Sites::is_guide_text_running(void)
{
	if (m_guide_text_main_layout)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

void Most_Visited_Sites::destroy_guide_text_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_guide_text_label) {
		evas_object_del(m_guide_text_label);
		m_guide_text_label = NULL;
	}
	if (m_never_show_check_box) {
		evas_object_del(m_never_show_check_box);
		m_never_show_check_box = NULL;
	}
	if (m_ok_button) {
		evas_object_del(m_ok_button);
		m_ok_button = NULL;
	}
	if (m_never_show_text_label) {
		evas_object_del(m_never_show_text_label);
		m_never_show_text_label = NULL;
	}
	if (m_guide_text_main_layout) {
		evas_object_del(m_guide_text_main_layout);
		m_guide_text_main_layout = NULL;
	}
}

void Most_Visited_Sites::__guide_text_ok_button_clicked_cb(void *data, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)data;
	Eina_Bool never_show = elm_check_state_get(most_visited_sites->m_never_show_check_box);
	if (never_show) {
		br_preference_set_bool(SHOW_MY_SITES_GUIDE, false);
	}

	elm_object_part_content_unset(most_visited_sites->m_browser_view->m_main_layout, "elm.swallow.mysites_guide");
	edje_object_signal_emit(elm_layout_edje_get(most_visited_sites->m_browser_view->m_main_layout),
							"hide,most_visited_sites_guide_text,signal", "");
	most_visited_sites->destroy_guide_text_main_layout();
}

Evas_Object *Most_Visited_Sites::create_guide_text_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_guide_text_main_layout = elm_layout_add(m_navi_bar);
	if (!m_guide_text_main_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	if (!elm_layout_file_set(m_guide_text_main_layout, BROWSER_EDJE_DIR"/most-visited-sites.edj",
								"guide_text")) {
		BROWSER_LOGE("elm_layout_file_set failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(m_guide_text_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_guide_text_main_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_guide_text_main_layout);

	m_guide_text_label = elm_label_add(m_guide_text_main_layout);
	if (!m_guide_text_label) {
		BROWSER_LOGE("elm_label_add failed");
		return NULL;
	}
	evas_object_size_hint_align_set(m_guide_text_label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(m_guide_text_label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_label_line_wrap_set(m_guide_text_label, ELM_WRAP_WORD);
	std::string guide_text = std::string("<font_size=32><color=#101010>") + std::string(BR_STRING_MY_SITES_GUIDE_TEXT)
										+ std::string("</color></font_size>");
	elm_object_text_set(m_guide_text_label, guide_text.c_str());
	elm_object_part_content_set(m_guide_text_main_layout, "elm.swallow.guide_text_label", m_guide_text_label);
	evas_object_show(m_guide_text_label);

	m_never_show_check_box = elm_check_add(m_guide_text_main_layout);
	if (!m_never_show_check_box) {
		BROWSER_LOGE("elm_check_add failed");
		return NULL;
	}
	elm_object_part_content_set(m_guide_text_main_layout, "elm.swallow.never_show_check_box", m_never_show_check_box);
	evas_object_show(m_never_show_check_box);

	m_never_show_text_label = elm_label_add(m_guide_text_main_layout);
	if (!m_never_show_text_label) {
		BROWSER_LOGE("elm_label_add failed");
		return NULL;
	}
	evas_object_size_hint_align_set(m_never_show_text_label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(m_never_show_text_label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_label_line_wrap_set(m_never_show_text_label, ELM_WRAP_WORD);

	std::string never_show_text = std::string("<font_size=30><color=#FFFFFF>") + std::string(BR_STRING_DO_NOT_ASK_AGAIN)
											+ std::string("</color></font_size>");

	elm_object_text_set(m_never_show_text_label, never_show_text.c_str());
	elm_object_part_content_set(m_guide_text_main_layout, "elm.swallow.never_show_text", m_never_show_text_label);

	/* To be able to set the check box even touch on 'never show' text for convenience. */
	edje_object_signal_callback_add(elm_layout_edje_get(m_guide_text_main_layout), "mouse,clicked,1",
					"elm.swallow.never_show_text_click_area", __never_show_text_clicked_cb, this);

	evas_object_show(m_never_show_text_label);

	m_ok_button = elm_button_add(m_guide_text_main_layout);
	if (!m_ok_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(m_ok_button, "browser/ok_button");
	elm_object_text_set(m_ok_button, BR_STRING_OK);
	elm_object_part_content_set(m_guide_text_main_layout, "elm.swallow.ok_button", m_ok_button);
	evas_object_smart_callback_add(m_ok_button, "clicked", __guide_text_ok_button_clicked_cb, this);
	evas_object_show(m_ok_button);

	return m_guide_text_main_layout;
}

void Most_Visited_Sites::__never_show_text_clicked_cb(void *data, Evas_Object *obj,
							const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)data;
	Eina_Bool never_show = elm_check_state_get(most_visited_sites->m_never_show_check_box);
	elm_check_state_set(most_visited_sites->m_never_show_check_box, !never_show);
}

#if defined(HORIZONTAL_UI)
void Most_Visited_Sites::rotate(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_browser_view->is_landscape()) {
		elm_gengrid_align_set(m_gengrid, 0.5, 0.0);
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "rotate,landscape,signal", "");
	} else {
		elm_gengrid_align_set(m_gengrid, 0.5, 0.5);
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "rotate,portrait,signal", "");
	}
}
#endif

void Most_Visited_Sites::_set_edit_mode(Eina_Bool edit_mode)
{
	BROWSER_LOGD("[%s]", __func__);
	m_edit_mode = edit_mode;

	Elm_Object_Item *it = elm_gengrid_first_item_get(m_gengrid);
	while (it) {
		most_visited_sites_item *item_data = (most_visited_sites_item *)elm_object_item_data_get(it);

		if (edit_mode)
			edje_object_signal_emit(elm_layout_edje_get(item_data->layout), "show,delete_button,signal", "");
		else
			edje_object_signal_emit(elm_layout_edje_get(item_data->layout), "hide,delete_button,signal", "");

		it = elm_gengrid_item_next_get(it);
	}

	if (!edit_mode)
		elm_genlist_realized_items_update(m_gengrid);
}

void Most_Visited_Sites::__setting_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)data;
	elm_object_part_content_unset(most_visited_sites->m_main_layout, "elm.swallow.setting_button");
	evas_object_hide(most_visited_sites->m_setting_button);

	elm_object_part_content_set(most_visited_sites->m_main_layout, "elm.swallow.setting_button", most_visited_sites->m_done_button);
	evas_object_show(most_visited_sites->m_done_button);

	most_visited_sites->_set_edit_mode(EINA_TRUE);
}

void Most_Visited_Sites::__done_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	Most_Visited_Sites *most_visited_sites = (Most_Visited_Sites *)data;
	elm_object_part_content_unset(most_visited_sites->m_main_layout, "elm.swallow.setting_button");
	evas_object_hide(most_visited_sites->m_done_button);

	elm_object_part_content_set(most_visited_sites->m_main_layout, "elm.swallow.setting_button", most_visited_sites->m_setting_button);
	evas_object_show(most_visited_sites->m_setting_button);

	most_visited_sites->_set_edit_mode(EINA_FALSE);
}

Evas_Object *Most_Visited_Sites::create_most_visited_sites_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_main_layout = elm_layout_add(m_navi_bar);
	if (!m_main_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	if (!elm_layout_file_set(m_main_layout, BROWSER_EDJE_DIR"/most-visited-sites.edj",
				"most_visited_sites_layout")) {
		BROWSER_LOGE("Can not set layout most_visited_sites_layout\n");
		return NULL;
	}
	evas_object_size_hint_weight_set(m_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_main_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_main_layout);

	m_gengrid = elm_gengrid_add(m_main_layout);
	if (!m_gengrid) {
		BROWSER_LOGE("elm_gengrid_add failed");
		return NULL;
	}

	elm_object_part_content_set(m_main_layout, "elm.swallow.gengrid", m_gengrid);

	int window_w = 0;
	evas_object_geometry_get(m_browser_view->m_win, NULL, NULL, &window_w, NULL);

	elm_gengrid_item_size_set(m_gengrid, MOST_VISITED_SITES_ITEM_WIDTH, MOST_VISITED_SITES_ITEM_HEIGHT);

#if defined(HORIZONTAL_UI)
	if (m_browser_view->is_landscape())
		elm_gengrid_align_set(m_gengrid, 0.5, 0.0);
	else
#endif
		elm_gengrid_align_set(m_gengrid, 0.5, 0.5);

	elm_gengrid_horizontal_set(m_gengrid, EINA_FALSE);
	elm_gengrid_bounce_set(m_gengrid, EINA_FALSE, EINA_FALSE);
	elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
	//elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_NONE);

//	evas_object_smart_callback_add(m_gengrid, "longpressed", __item_longpressed_cb, this);
	evas_object_smart_callback_add(m_gengrid, "changed", __item_drag_cb, this);
	evas_object_smart_callback_add(m_gengrid, "moved", __item_moved_cb, this);

//	elm_gengrid_reorder_mode_set(m_gengrid, EINA_TRUE);
//	m_gengrid_item_class = elm_gengrid_item_class_new();
//	m_gengrid_item_class->item_style = "default_grid/browser/most_visited_sites";
	m_gengrid_item_class.item_style = "default_grid";
	m_gengrid_item_class.func.text_get = NULL;
	m_gengrid_item_class.func.content_get = __gengrid_icon_get_cb;
	m_gengrid_item_class.func.state_get = NULL;
	m_gengrid_item_class.func.del = NULL;

	if (!_reload_items()) {
		BROWSER_LOGE("new _reload_items failed");
		return NULL;
	}

	evas_object_show(m_gengrid);

	m_setting_button = elm_button_add(m_gengrid);
	if (!m_setting_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(m_setting_button, "browser/most_visited_sites_setting");

	elm_object_part_content_set(m_main_layout, "elm.swallow.setting_button", m_setting_button);
	evas_object_smart_callback_add(m_setting_button, "clicked", __setting_button_clicked_cb, this);
	evas_object_show(m_setting_button);

	m_done_button = elm_button_add(m_gengrid);
	if (!m_done_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(m_done_button, "browser/most_visited_sites_done");
	elm_object_text_set(m_done_button, BR_STRING_DONE);
	evas_object_smart_callback_add(m_done_button, "clicked", __done_button_clicked_cb, this);
	evas_object_hide(m_done_button);

	return m_main_layout;
}

