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

#include "browser-view.h"
#include "browser-history-db.h"
#include "browser-most-visited.h"

Browser_Most_Visited::Browser_Most_Visited(Evas_Object *navi_bar, Browser_History_DB *history_db,
						Browser_View *browser_view)
:
	m_navi_bar(navi_bar)
	,m_history_db(history_db)
	,m_browser_view(browser_view)
	,m_main_layout(NULL)
	,m_item_box(NULL)
	,m_done_button(NULL)
	,m_longpress_timer(NULL)
	,m_guide_text_label(NULL)
	,m_is_edit_mode(EINA_FALSE)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Most_Visited::~Browser_Most_Visited(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_main_layout)
		evas_object_del(m_main_layout);
	if (m_item_box)
		evas_object_del(m_item_box);
}

void Browser_Most_Visited::_clear_items(void)
{
	BROWSER_LOGD("[%s]", __func__);
	elm_box_clear(m_item_box);
	for (int i = 0 ; i < m_most_visited_list.size() ; i++) {
		if (m_most_visited_list[i]) {
			if (m_most_visited_list[i]->layout)
				evas_object_del(m_most_visited_list[i]->layout);
			if (m_most_visited_list[i]->item_button)
				evas_object_del(m_most_visited_list[i]->item_button);
			if (m_most_visited_list[i]->favicon)
				evas_object_del(m_most_visited_list[i]->favicon);
			delete m_most_visited_list[i];
			m_most_visited_list.erase(m_most_visited_list.begin() + i);
		}
	}

	m_most_visited_list.clear();
}

void Browser_Most_Visited::_most_visited_item_clicked_cb(void *data, Evas_Object *obj,
									void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	most_visited_item *item = (most_visited_item *)data;
	Browser_Most_Visited *most_visited = (Browser_Most_Visited *)(item->user_data);

	if (!most_visited->m_is_edit_mode) {
		/* Give focus the hide the keypad. */
		elm_object_focus_set(most_visited->m_browser_view->m_option_header_cancel_button, EINA_TRUE);
		/* Load selected url only with normal mode not edit mode. */
		most_visited->m_browser_view->load_url(item->url.c_str());
	}
}

void Browser_Most_Visited::__item_mouse_down_cb(void* data, Evas* evas,
							Evas_Object* obj, void* ev)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Most_Visited *most_visited = (Browser_Most_Visited *)data;

	if (!most_visited->m_is_edit_mode) {
		/* If normal mode. */
		if (most_visited->m_longpress_timer)
			ecore_timer_del(most_visited->m_longpress_timer);
		most_visited->m_longpress_timer = ecore_timer_add(elm_longpress_timeout_get(),
								__longpress_timeout_cb, data);
	}
}

void Browser_Most_Visited::__item_mouse_up_cb(void* data, Evas* evas,
							Evas_Object* obj, void* ev)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_Most_Visited *most_visited = (Browser_Most_Visited *)data;

	if (most_visited->m_longpress_timer) {
		ecore_timer_del(most_visited->m_longpress_timer);
		most_visited->m_longpress_timer = NULL;
	}
}

Eina_Bool Browser_Most_Visited::__longpress_timeout_cb(void* data)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!data)
		return ECORE_CALLBACK_CANCEL;

	Browser_Most_Visited *most_visited = (Browser_Most_Visited *)data;
	most_visited->m_longpress_timer = NULL;

	most_visited->m_is_edit_mode = EINA_TRUE;

#if defined(HORIZONTAL_UI)
	if (most_visited->m_browser_view->is_landscape())
		edje_object_signal_emit(elm_layout_edje_get(most_visited->m_main_layout),
						"most_visited,landscape_edit_mode,signal", "");
	else
#endif
		/* Expand the background of most visited list. */
		edje_object_signal_emit(elm_layout_edje_get(most_visited->m_main_layout),
						"most_visited,edit_mode,signal", "");

	/* Show delete button on each item. */
	for (int i = 0 ; i < most_visited->m_most_visited_list.size() ; i++)
		edje_object_signal_emit(elm_layout_edje_get(most_visited->m_most_visited_list[i]->layout),
						"most_visited,edit_mode,signal", "");

	return ECORE_CALLBACK_CANCEL;
}

void Browser_Most_Visited::__done_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Most_Visited *most_visited = (Browser_Most_Visited *)data;
	if (most_visited->m_longpress_timer) {
		ecore_timer_del(most_visited->m_longpress_timer);
		most_visited->m_longpress_timer = NULL;
	}

	most_visited->m_is_edit_mode = EINA_FALSE;

#if defined(HORIZONTAL_UI)
	if (most_visited->m_browser_view->is_landscape())
		edje_object_signal_emit(elm_layout_edje_get(most_visited->m_main_layout),
						"most_visited,landscape_normal_mode,signal", "");
	else
#endif
		/* Downscale the background of most visited list. */
		edje_object_signal_emit(elm_layout_edje_get(most_visited->m_main_layout),
						"most_visited,normal_mode,signal", "");

	/* Hide delete button on each item. */
	for (int i = 0 ; i < most_visited->m_most_visited_list.size() ; i++)
		edje_object_signal_emit(elm_layout_edje_get(most_visited->m_most_visited_list[i]->layout),
						"most_visited,normal_mode,signal", "");
}

#if defined(HORIZONTAL_UI)
void Browser_Most_Visited::rotate(int degree)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_most_visited_list.size()) {
		m_is_edit_mode = EINA_FALSE;

		if (m_browser_view->is_landscape())
			edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
						"most_visited,landscape_normal_mode,signal", "");
		else
			/* Downscale the background of most visited list. */
			edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
							"most_visited,normal_mode,signal", "");
		/* Hide delete button on each item. */
		for (int i = 0 ; i < m_most_visited_list.size() ; i++)
			edje_object_signal_emit(elm_layout_edje_get(m_most_visited_list[i]->layout),
							"most_visited,normal_mode,signal", "");

		if (degree == 0)
			edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
							"most_visited,portrait,signal", "");
		else
			edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
							"most_visited,landscape,signal", "");
	}
}
#endif

Eina_Bool Browser_Most_Visited::_show_guide_text(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_guide_text_label) {
		elm_object_part_content_unset(m_main_layout, "elm.swallow.most_visited_guide_text");
		evas_object_del(m_guide_text_label);
	}

	m_guide_text_label = elm_label_add(m_item_box);
	if (!m_guide_text_label) {
		BROWSER_LOGE("elm_label_add failed");
		return EINA_FALSE;
	}

	elm_label_line_wrap_set(m_guide_text_label, ELM_WRAP_WORD);

	std::string guide_text = std::string("<color=#F3EFE9>") + std::string(BR_STRING_MOST_VISITED_GUIDE_TEXT)
				+ std::string("</color>");
	elm_object_text_set(m_guide_text_label, guide_text.c_str());

	elm_object_part_content_set(m_main_layout, "elm.swallow.most_visited_guide_text", m_guide_text_label);
	edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,guide_text,signal", "");
	evas_object_show(m_guide_text_label);

	return EINA_TRUE;
}

void Browser_Most_Visited::__delete_icon_clicked_cb(void *data, Evas_Object *obj,
						const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	most_visited_item *item = (most_visited_item *)data;
	Browser_Most_Visited *most_visited = (Browser_Most_Visited *)(item->user_data);

	elm_box_unpack(most_visited->m_item_box, item->layout);
	if (!most_visited->m_history_db->delete_history(item->url.c_str()))
		BROWSER_LOGE("delete_history failed");

	int index = 0;
	for (index = 0 ; index < most_visited->m_most_visited_list.size() ; index++) {
		if (most_visited->m_most_visited_list[index] == item)
			break;
	}

	if (most_visited->m_most_visited_list[index]->layout)
		evas_object_del(most_visited->m_most_visited_list[index]->layout);
	if (most_visited->m_most_visited_list[index]->item_button)
		evas_object_del(most_visited->m_most_visited_list[index]->item_button);
	if (most_visited->m_most_visited_list[index]->favicon)
		evas_object_del(most_visited->m_most_visited_list[index]->favicon);

	delete most_visited->m_most_visited_list[index];
	most_visited->m_most_visited_list.erase(most_visited->m_most_visited_list.begin() + index);

	if (most_visited->m_most_visited_list.size() == 0) {
#if defined(HORIZONTAL_UI)
		if (most_visited->m_browser_view->is_landscape())
			edje_object_signal_emit(elm_layout_edje_get(most_visited->m_main_layout),
						"most_visited,landscape_normal_mode,signal", "");
		else
#endif
			/* Downscale the background of most visited list. */
			edje_object_signal_emit(elm_layout_edje_get(most_visited->m_main_layout),
						"most_visited,normal_mode,signal", "");
		if (!most_visited->_show_guide_text())
			BROWSER_LOGE("_show_guide_text failed");
	}
}

Eina_Bool Browser_Most_Visited::_update_items(void)
{
	BROWSER_LOGD("[%s]", __func__);

	_clear_items();

	std::vector<Browser_History_DB::most_visited_item> most_visited_list;
	if (!m_history_db->get_most_visited_list(most_visited_list)) {
		BROWSER_LOGE("m_history_db->get_most_visited_list failed");
		return EINA_FALSE;
	}

	for (int i = 0 ; i < most_visited_list.size() ; i++) {
		most_visited_item *item = new(nothrow) most_visited_item;
		if (!item) {
			BROWSER_LOGE("new most_visited_item failed");
			return EINA_FALSE;
		}
		item->layout = NULL;
		item->item_button = NULL;
		item->favicon = NULL;

		item->url = std::string(most_visited_list[i].url.c_str());
		item->user_data = (void *)this;

		item->layout = elm_layout_add(m_navi_bar);
		if (!item->layout) {
			BROWSER_LOGE("elm_layout_add failed.");
			return EINA_FALSE;
		}
		if (!elm_layout_file_set(item->layout, BROWSER_EDJE_DIR"/browser-most-visited.edj",
					"browser/most-visited-item"))
			BROWSER_LOGE("elm_layout_file_set failed");
		evas_object_size_hint_weight_set(item->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(item->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		edje_object_signal_callback_add(elm_layout_edje_get(item->layout), "mouse,clicked,1",
						"delete_icon", __delete_icon_clicked_cb, item);

		elm_box_pack_end(m_item_box, item->layout);
		evas_object_show(item->layout);

		item->item_button = elm_button_add(m_navi_bar);
		if (!item->item_button) {
			BROWSER_LOGE("elm_button_add failed.");
			return EINA_FALSE;
		}
		elm_object_style_set(item->item_button, "browser/most_visited_item");
		elm_object_part_content_set(item->layout, "elm.swallow.button", item->item_button);

		/* Do no allow focus on most visited item.
		  * If the item can have focus, the keypad is hided automatically when user click the most visited item.
		  * It's because to be able to change to edit mode. */
		elm_object_focus_allow_set(item->item_button, EINA_FALSE);
		evas_object_smart_callback_add(item->item_button, "clicked", _most_visited_item_clicked_cb, item);
		evas_object_event_callback_add(item->item_button, EVAS_CALLBACK_MOUSE_DOWN,
								__item_mouse_down_cb, this);
		evas_object_event_callback_add(item->item_button, EVAS_CALLBACK_MOUSE_UP,
								__item_mouse_up_cb, this);
		evas_object_show(item->item_button);

		item->favicon = m_browser_view->get_favicon(item->url.c_str());
		if (!item->favicon) {
			item->favicon = elm_icon_add(m_navi_bar);
			if (!item->favicon) {
				BROWSER_LOGE("elm_icon_add failed.");
				return EINA_FALSE;
			}
			if (!elm_icon_file_set(item->favicon, BROWSER_IMAGE_DIR"/faviconDefault.png", NULL))
				BROWSER_LOGE("elm_icon_file_set is failed.\n");
		}

		elm_object_part_content_set(item->layout, "elm.swallow.favicon", item->favicon);
		evas_object_show(item->favicon);

		edje_object_part_text_set(elm_layout_edje_get(item->layout), "name",
						most_visited_list[i].title.c_str());

		m_most_visited_list.push_back(item);
	}

	if (most_visited_list.size() == 0)
		if (!_show_guide_text())
			BROWSER_LOGE("_show_guide_text failed");

	return EINA_TRUE;
}

Evas_Object *Browser_Most_Visited::create_most_visited_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_main_layout = elm_layout_add(m_navi_bar);
	if (!m_main_layout) {
		BROWSER_LOGE("elm_layout_add failed!");
		return NULL;
	}

	if (!elm_layout_file_set(m_main_layout, BROWSER_EDJE_DIR"/browser-most-visited.edj",
				"browser/most-visited")) {
		BROWSER_LOGE("elm_layout_file_set failed");
		return NULL;
	}
	evas_object_show(m_main_layout);

	m_item_box = elm_box_add(m_navi_bar);
	if (!m_item_box) {
		BROWSER_LOGE("elm_box_add failed");
		return NULL;
	}
	evas_object_size_hint_align_set(m_item_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_horizontal_set(m_item_box, EINA_TRUE);
	elm_box_homogeneous_set(m_item_box, EINA_TRUE);
	elm_object_part_content_set(m_main_layout, "elm.swallow.most_visited", m_item_box);
	evas_object_show(m_item_box);

	if (!_update_items()) {
		BROWSER_LOGE("_update_items failed!");
		return NULL;
	}

	edje_object_part_text_set(elm_layout_edje_get(m_main_layout),
				"elm.text.most_visited_title", BR_STRING_MOST_VISITED_SITES);

	m_done_button = elm_button_add(m_navi_bar);
	if (!m_done_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(m_done_button, "text_only/style1");
	elm_object_text_set(m_done_button, BR_STRING_DONE);
	/* Do no allow focus on done button.
	  * If the item can have focus, the keypad is hided automatically when user click done button. */
	elm_object_focus_allow_set(m_done_button, EINA_FALSE);
	evas_object_smart_callback_add(m_done_button, "clicked", __done_button_clicked_cb, this);
	elm_object_part_content_set(m_main_layout, "elm.swallow.done", m_done_button);

#if defined(HORIZONTAL_UI)
	if (m_browser_view->is_landscape())
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
							"most_visited,landscape,signal", "");
#endif

	return m_main_layout;
}

