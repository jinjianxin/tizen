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
  *@file browser-multi-window-view.cpp
  *@brief 创建多窗口view
  */

#include "browser-class.h"
#include "browser-multi-window-view.h"
#include "browser-view.h"
#include "browser-window.h"

Browser_Multi_Window_View::Browser_Multi_Window_View(void)
:
	m_main_layout(NULL)
	,m_zoom_transit(NULL)
	,m_zoom_effect_image(NULL)
	,m_flip_effect_image(NULL)
	,m_zoom_move_transit(NULL)
	,m_scroll_move_transit(NULL)
	,m_scroller(NULL)
	,m_item_box(NULL)
	,m_dummy_front_item_layout(NULL)
	,m_dummy_end_item_layout(NULL)
	,m_dummy_front_item_snapshot(NULL)
	,m_dummy_end_item_snapshot(NULL)
	,m_select_item_index(0)
	,m_current_position_index(0)
	,m_page_control(NULL)
	,m_gengrid(NULL)
	,m_is_reordering(EINA_FALSE)
	,m_zoom_out_effect_idler(NULL)
	,m_zoom_out_duration(0.5)
	,m_change_view_button(NULL)
	,m_cancel_button(NULL)
	,m_controlbar(NULL)
	,m_new_window_button(NULL)
	,m_init_grid_mode(EINA_FALSE)
{
	BROWSER_LOGD("[%s]", __func__);
	memset(m_index_items, 0x00, sizeof(Elm_Object_Item *) * BROWSER_MULTI_WINDOW_MAX_COUNT);
}

Browser_Multi_Window_View::~Browser_Multi_Window_View(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_main_layout) {
		evas_object_del(m_main_layout);
		m_main_layout = NULL;
	}
	if (m_scroller) {
		evas_object_del(m_scroller);
		m_scroller = NULL;
	}
	if (m_item_box) {
		evas_object_del(m_item_box);
		m_item_box = NULL;
	}
	if (m_dummy_front_item_layout) {
		evas_object_del(m_dummy_front_item_layout);
		m_dummy_front_item_layout = NULL;
	}
	if (m_dummy_end_item_layout) {
		evas_object_del(m_dummy_end_item_layout);
		m_dummy_end_item_layout = NULL;
	}
	if (m_dummy_front_item_snapshot) {
		evas_object_del(m_dummy_front_item_snapshot);
		m_dummy_front_item_snapshot = NULL;
	}
	if (m_dummy_end_item_snapshot) {
		evas_object_del(m_dummy_end_item_snapshot);
		m_dummy_end_item_snapshot = NULL;
	}
	if (m_page_control) {
		evas_object_del(m_page_control);
		m_page_control = NULL;
	}
	if (m_zoom_transit) {
		elm_transit_del(m_zoom_transit);
		m_zoom_transit = NULL;
	}
	if (m_zoom_effect_image) {
		evas_object_del(m_zoom_effect_image);
		m_zoom_effect_image = NULL;
	}
	if (m_flip_effect_image) {
		evas_object_del(m_flip_effect_image);
		m_flip_effect_image = NULL;
	}
	if (m_zoom_move_transit) {
		elm_transit_del(m_zoom_move_transit);
		m_zoom_move_transit = NULL;
	}
	if (m_scroll_move_transit) {
		elm_transit_del(m_scroll_move_transit);
		m_scroll_move_transit = NULL;
	}
	if (m_zoom_out_effect_idler) {
		ecore_idler_del(m_zoom_out_effect_idler);
		m_zoom_out_effect_idler = NULL;
	}
	for (int i = 0 ; i < m_item_list.size() ; i++) {
		if (m_item_list[i])
			evas_object_del(m_item_list[i]);
	}
	m_item_list.clear();

	if (m_gengrid) {
		evas_object_del(m_gengrid);
		m_gengrid = NULL;
	}
}

Eina_Bool Browser_Multi_Window_View::init(double duration, Eina_Bool grid_mode)
{
	BROWSER_LOGD("[%s]", __func__);
	m_zoom_out_duration = duration;
	m_init_grid_mode = grid_mode;

	if (!_create_main_layout()) {
		BROWSER_LOGD("_create_main_layout failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

void Browser_Multi_Window_View::__zoom_out_finished_cb(void *data, Elm_Transit *transit)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;

	if (!multi_window_view->m_zoom_transit)
		return;

	multi_window_view->m_zoom_transit = NULL;
	multi_window_view->m_zoom_move_transit = NULL;

	if (multi_window_view->m_zoom_effect_image) {
		evas_object_del(multi_window_view->m_zoom_effect_image);
		multi_window_view->m_zoom_effect_image = NULL;
	}
	if (multi_window_view->m_flip_effect_image) {
		evas_object_del(multi_window_view->m_flip_effect_image);
		multi_window_view->m_flip_effect_image = NULL;
	}

	multi_window_view->__zoom_out_finished();
}

void Browser_Multi_Window_View::__zoom_out_finished(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int scroll_size = _get_scroll_page_size();
	int scroller_y = 0;
	int scroller_w = 0;
	int scroller_h = 0;
	elm_scroller_region_get(m_scroller, NULL, &scroller_y, &scroller_w, &scroller_h);
	elm_scroller_region_show(m_scroller, scroll_size * m_current_position_index,
						scroller_y, scroller_w, scroller_h);

	for (int i = 0 ; i < m_item_list.size() ; i++)
		evas_object_show(m_item_list[i]);

	Browser_View *browser_view = m_data_manager->get_browser_view();
	const char *title = browser_view->get_title().c_str();
	const char *url = browser_view->get_url().c_str();

	BROWSER_LOGD("title=[%s]", title);
	BROWSER_LOGD("url=[%s]", url);
	if (!title || strlen(title) == 0) {
		Evas_Object *entry = br_elm_editfield_entry_get(browser_view->m_option_header_url_edit_field);
		const char *entry_text = elm_entry_entry_get(entry);
		if (entry_text && strlen(entry_text)) {
			BROWSER_LOGD("[%s][%s]", entry_text, entry_text);
			char *markup = elm_entry_utf8_to_markup(entry_text);
			if (markup) {
				edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.title", markup);
				edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.url", markup);
				free(markup);
			}
		} else {
			edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.title", BR_STRING_EMPTY);
			edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.url", "");
		}
	} else {
		BROWSER_LOGD("[%s][%s]", title, url);

		edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.title", title);

		char *markup = NULL;
		markup = elm_entry_utf8_to_markup(url);
		if (markup) {
			edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.url", markup);
			free(markup);
		}
	}

	_show_grey_effect();
}

void Browser_Multi_Window_View::__zoom_in_finished_cb(void *data, Elm_Transit *transit)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;

	if (!multi_window_view->m_zoom_transit)
		return;

	multi_window_view->m_zoom_transit = NULL;
	multi_window_view->m_zoom_move_transit = NULL;

	if (multi_window_view->m_zoom_effect_image) {
		evas_object_del(multi_window_view->m_zoom_effect_image);
		multi_window_view->m_zoom_effect_image = NULL;
	}
	if (multi_window_view->m_flip_effect_image) {
		evas_object_del(multi_window_view->m_flip_effect_image);
		multi_window_view->m_flip_effect_image = NULL;
	}

	std::vector<Browser_Window *> window_list = m_browser->get_window_list();
	if (!window_list[multi_window_view->m_select_item_index]->m_ewk_view) {
		Browser_Window *deleted_window = NULL;
		deleted_window = m_browser->create_deleted_window(multi_window_view->m_select_item_index);
		if (deleted_window) {
			BROWSER_LOGD("************ deleted_window=%d", deleted_window);
			for (int i = 0 ; i < window_list.size(); i++)
				BROWSER_LOGD("************ window[%d]=%d", i, window_list[i]);
			Browser_View *browser_view = m_data_manager->get_browser_view();
			m_browser->set_focused_window(deleted_window);
			BROWSER_LOGD("*** load url=[%s]", deleted_window->m_url.c_str());
			browser_view->load_url(deleted_window->m_url.c_str());
		}
	} else
		m_browser->set_focused_window(window_list[multi_window_view->m_select_item_index]);

	__close_multi_window_button_clicked_cb(multi_window_view, NULL, NULL);
}

/*  zoom in effect when user selects mult window item. */
Eina_Bool Browser_Multi_Window_View::_show_zoom_in_effect(Evas_Object *item)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_zoom_transit) {
		BROWSER_LOGE("transition is already running");
		return EINA_FALSE;
	}
	Browser_Window *focused_window = m_data_manager->get_browser_view()->m_focused_window;

	int ewk_view_current_y = 0;
	evas_object_geometry_get(focused_window->m_ewk_view, NULL, &ewk_view_current_y, NULL, NULL);

	int index = 0;
	for (index = 0 ; index < m_item_list.size() ; index++) {
		if (m_item_list[index] == item)
			break;
	}

	evas_object_hide(item);

	/* Destroy speed dial little eariler because of blinking issue. */
	Browser_View *browser_view = m_data_manager->get_browser_view();

	std::vector<Browser_Window *> window_list = m_browser->get_window_list();

	int webview_x = 0;
	int webview_y = 0;
	int webview_w = 0;
	int webview_h = 0;
	evas_object_geometry_get(focused_window->m_ewk_view, &webview_x, &webview_y,
								&webview_w, &webview_h);

	int snapshot_x = 0;
	int snapshot_y = 0;
	int snapshot_w = 0;
	int snapshot_h = 0;
#if defined(HORIZONTAL_UI)
	if (is_landscape())
		evas_object_geometry_get(window_list[index]->m_landscape_snapshot_image,
					&snapshot_x, &snapshot_y, &snapshot_w, &snapshot_h);
	else
#endif
		evas_object_geometry_get(window_list[index]->m_portrait_snapshot_image,
					&snapshot_x, &snapshot_y, &snapshot_w, &snapshot_h);

	if (!window_list[index]->m_ewk_view || browser_view->get_url(window_list[index]).empty()) {
		m_zoom_effect_image = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
		if (!m_zoom_effect_image) {
			BROWSER_LOGE("evas_object_rectangle_add failed");
			return EINA_FALSE;
		}
		evas_object_resize(m_zoom_effect_image, snapshot_w, snapshot_h);
		evas_object_color_set(m_zoom_effect_image, 255, 255, 255, 255);

		m_flip_effect_image = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
		if (!m_flip_effect_image) {
			BROWSER_LOGE("evas_object_rectangle_add failed");
			return EINA_FALSE;
		}
		evas_object_resize(m_flip_effect_image, webview_w, webview_h);
		evas_object_color_set(m_flip_effect_image, 255, 255, 255, 255);
	} else {
		m_zoom_effect_image = _capture_snapshot(window_list[index],
						BROWSER_MULTI_WINDOW_ITEM_RATIO);
		if (!m_zoom_effect_image) {
			BROWSER_LOGE("_capture_snapshot failed");
			return EINA_FALSE;
		}

		m_flip_effect_image = _capture_snapshot(window_list[index], 1.0);
		if (!m_flip_effect_image) {
			BROWSER_LOGE("_capture_snapshot failed");
			return EINA_FALSE;
		}
	}
	evas_object_move(m_flip_effect_image, webview_x, webview_y);

	evas_object_move(m_zoom_effect_image, snapshot_x, snapshot_y);
	evas_object_show(m_zoom_effect_image);

	/* m_zoom_effect_image is only for zoom out effect for the first time. */
	m_zoom_transit = elm_transit_add();
	if (!m_zoom_transit) {
		BROWSER_LOGE("elm_transit_add failed");
		return EINA_FALSE;
	}
	/* Block event durring animation. */
	elm_transit_event_enabled_set(m_zoom_transit, EINA_TRUE);

	m_select_item_index = index;

#if defined(FEATURE_MOST_VISITED_SITES)
	if (!(browser_view->get_url(window_list[m_select_item_index]).empty()))
		browser_view->_show_most_visited_sites(EINA_FALSE);
#endif

	/* Without this code, the url is empty shortly when create deleted-window in multi window. */
	if (browser_view->get_url().empty() && !browser_view->m_focused_window->m_url.empty())
		browser_view->_set_url_entry(window_list[m_select_item_index]->m_url.c_str(), EINA_FALSE);
	else
		browser_view->_set_url_entry(browser_view->get_url(window_list[m_select_item_index]).c_str(), EINA_FALSE);

	elm_transit_object_add(m_zoom_transit, m_zoom_effect_image);
	elm_transit_object_add(m_zoom_transit, m_flip_effect_image);
	elm_transit_tween_mode_set(m_zoom_transit, ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL);
	elm_transit_objects_final_state_keep_set(m_zoom_transit, EINA_FALSE);
	elm_transit_effect_resizable_flip_add(m_zoom_transit, ELM_TRANSIT_EFFECT_FLIP_AXIS_Y, EINA_TRUE);
	elm_transit_duration_set(m_zoom_transit, 0.5);
	elm_transit_del_cb_set(m_zoom_transit, __zoom_in_finished_cb, this);
	elm_transit_go(m_zoom_transit);

#if defined(FEATURE_MOST_VISITED_SITES)
	/* If speed dial is running, just zoom in effect is enough. */
	if (browser_view->is_most_visited_sites_running())
		return EINA_TRUE;
#endif

	/* If the url bar in browser scroller is being displayed,
	 * the zoom animation should be move up because of the url bar gap.
	 * So add translation effect as well. */
	m_zoom_move_transit = elm_transit_add();
	if (!m_zoom_move_transit) {
		BROWSER_LOGE("elm_transit_add failed");
		return EINA_FALSE;
	}
	/* Block event durring animation. */
	elm_transit_event_enabled_set(m_zoom_move_transit, EINA_TRUE);

	int browser_scroller_y = 0;
	int url_layout_h = 0;
	elm_scroller_region_get(browser_view->m_scroller, NULL, &browser_scroller_y, NULL, NULL);
	evas_object_geometry_get(browser_view->m_url_layout, NULL, NULL, NULL, &url_layout_h);

	int scroller_y = 0;
	evas_object_geometry_get(browser_view->m_scroller, NULL, &scroller_y, NULL, NULL);

	if (browser_scroller_y > url_layout_h)
		browser_scroller_y = url_layout_h;

	elm_transit_object_add(m_zoom_move_transit, m_zoom_effect_image);
	elm_transit_object_add(m_zoom_move_transit, m_flip_effect_image);
	elm_transit_tween_mode_set(m_zoom_move_transit, ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL);
	elm_transit_objects_final_state_keep_set(m_zoom_move_transit, EINA_FALSE);

	elm_transit_effect_translation_add(m_zoom_move_transit, 0, 0, 0,
							(url_layout_h - browser_scroller_y - scroller_y - (6 * elm_scale_get())));

	elm_transit_duration_set(m_zoom_move_transit, 0.5);
	elm_transit_go(m_zoom_move_transit);

	return EINA_TRUE;
}

/*  zoom out effect for the first time when user excutes multi window. */
Eina_Bool Browser_Multi_Window_View::_show_zoom_out_effect(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Window *focused_window = m_data_manager->get_browser_view()->m_focused_window;

	int ewk_view_current_x = 0;
	int ewk_view_current_y = 0;
	int ewk_view_current_w = 0;
	int ewk_view_current_h = 0;
	evas_object_geometry_get(focused_window->m_ewk_view, &ewk_view_current_x, &ewk_view_current_y,
						&ewk_view_current_w, &ewk_view_current_h);

	Browser_View *browser_view = m_data_manager->get_browser_view();

	if (m_zoom_out_duration == 0) {
		Elm_Transit *tmp = m_zoom_transit = elm_transit_add();
		__zoom_out_finished_cb(this, NULL);
		elm_transit_del(tmp);
		return EINA_TRUE;
	}

	/* m_zoom_effect_image is only for zoom out effect for the first time. */
	m_zoom_transit = elm_transit_add();
	if (!m_zoom_transit) {
		BROWSER_LOGE("elm_transit_add failed");
		return EINA_FALSE;
	}
	/* Block event durring animation. */
	elm_transit_event_enabled_set(m_zoom_transit, EINA_TRUE);

	int snapshot_w = 0;
	int snapshot_h = 0;
	evas_object_geometry_get(m_zoom_effect_image, NULL, NULL, &snapshot_w, &snapshot_h);
	evas_object_move(m_zoom_effect_image, ewk_view_current_x, ewk_view_current_y);

	evas_object_show(m_zoom_effect_image);

	int current_snapshot_x = 0;
	int current_snapshot_y = 0;
	int current_snapshot_w = 0;
	int current_snapshot_h = 0;
	std::vector<Browser_Window *> window_list = m_browser->get_window_list();
#if defined(HORIZONTAL_UI)
	if (is_landscape())
		evas_object_geometry_get(window_list[0]->m_landscape_snapshot_image,
				&current_snapshot_x, &current_snapshot_y, &current_snapshot_w, &current_snapshot_h);
	else
#endif
		evas_object_geometry_get(window_list[0]->m_portrait_snapshot_image,
				&current_snapshot_x, &current_snapshot_y, &current_snapshot_w, &current_snapshot_h);

	evas_object_move(m_flip_effect_image, current_snapshot_x, current_snapshot_y);

	BROWSER_LOGD("current_snapshot_x=%d, current_snapshot_y=%d", current_snapshot_x, current_snapshot_y);
	BROWSER_LOGD("current_snapshot_w=%d, current_snapshot_h=%d", current_snapshot_w, current_snapshot_h);

	elm_transit_object_add(m_zoom_transit, m_zoom_effect_image);
	elm_transit_object_add(m_zoom_transit, m_flip_effect_image);
	elm_transit_tween_mode_set(m_zoom_transit, ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL);
	elm_transit_objects_final_state_keep_set(m_zoom_transit, EINA_FALSE);
	elm_transit_effect_resizable_flip_add(m_zoom_transit, ELM_TRANSIT_EFFECT_FLIP_AXIS_Y, EINA_TRUE);

	elm_transit_duration_set(m_zoom_transit, m_zoom_out_duration);
	elm_transit_del_cb_set(m_zoom_transit, __zoom_out_finished_cb, this);
	elm_transit_go(m_zoom_transit);

	/* If the url bar in browser scroller is being displayed,
	 * the zoom animation should be move up because of the url bar gap.
	 * So add translation effect as well. */
	m_zoom_move_transit = elm_transit_add();
	if (!m_zoom_move_transit) {
		BROWSER_LOGE("elm_transit_add failed");
		return EINA_FALSE;
	}
	/* Block event durring animation. */
	elm_transit_event_enabled_set(m_zoom_move_transit, EINA_TRUE);

	elm_transit_object_add(m_zoom_move_transit, m_zoom_effect_image);
	elm_transit_object_add(m_zoom_move_transit, m_flip_effect_image);
	elm_transit_tween_mode_set(m_zoom_move_transit, ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL);
	elm_transit_objects_final_state_keep_set(m_zoom_move_transit, EINA_FALSE);

	int browser_scroller_y = 0;
	int url_layout_h = 0;
	elm_scroller_region_get(browser_view->m_scroller, NULL, &browser_scroller_y, NULL, NULL);
	evas_object_geometry_get(browser_view->m_url_layout, NULL, NULL, NULL, &url_layout_h);

	int scroller_y = 0;
	evas_object_geometry_get(browser_view->m_scroller, NULL, &scroller_y, NULL, NULL);
	if (browser_scroller_y > url_layout_h)
		browser_scroller_y = url_layout_h;

	/* 4 * elm_scale_get() is to make the effect layout more smoothly. */
	elm_transit_effect_translation_add(m_zoom_move_transit, 0, 0, 0,
					(url_layout_h - browser_scroller_y - scroller_y - (6 * elm_scale_get())) * (-1));

	elm_transit_duration_set(m_zoom_move_transit, m_zoom_out_duration);
	elm_transit_go(m_zoom_move_transit);

	return EINA_TRUE;
}

void Browser_Multi_Window_View::_set_multi_window_mode(Eina_Bool is_multi_window)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_View *browser_view = m_data_manager->get_browser_view();

	if (is_multi_window) {
		/* The dimming background is done by
		 * 	edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout),
						"show,multi_window,signal", "");
		 * Move above code to __fill_multi_window_items_idler_cb */

		elm_object_part_content_set(browser_view->m_main_layout,
						"elm.swallow.multi_window", m_main_layout);

		/* Dim the multi window background. */
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
						"show,multi_window,signal", "");

		/* Unset navigationbar title object to hide the title bar arrow.  */
		browser_view->unset_navigationbar_title_object(EINA_TRUE);
	} else {
		/* Hide dim the multi window background. */
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
							"hide,multi_window,signal", "");
		/* Unset multi window layout from browser view. */
		elm_object_part_content_unset(browser_view->m_main_layout,
						"elm.swallow.multi_window");
		/* Hide the swallow of multi window in browser main layout. */
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout),
							"hide,multi_window,signal", "");

		/* Set navigationbar title object to show the title bar arrow.  */
		browser_view->unset_navigationbar_title_object(EINA_FALSE);

		/* Without this code, the url is empty shortly when create deleted-window in multi window. */
		if (browser_view->get_url().empty() && !browser_view->m_focused_window->m_url.empty())
			browser_view->_set_url_entry(browser_view->m_focused_window->m_url.c_str());
		else
			browser_view->_set_url_entry(browser_view->get_url().c_str());
		browser_view->_set_controlbar_back_forward_status();
	}
}

void Browser_Multi_Window_View::__snapshot_clicked_cb(void *data, Evas_Object *obj,
						const char *emission, const char *source)
{
	if (!data)
		return;

	BROWSER_LOGD("[%s]", __func__);
	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;

	if (multi_window_view->m_scroll_move_transit) {
		BROWSER_LOGE("new window transit is running");
		return;
	}

	multi_window_view->_snapshot_clicked(obj);
}

void Browser_Multi_Window_View::_snapshot_clicked(Evas_Object *layout_edje)
{
	int index = 0;
	for (index = 0 ; index < m_item_list.size() ; index++) {
		if (elm_layout_edje_get(m_item_list[index]) == layout_edje)
			break;
	}
	BROWSER_LOGD("m_current_position_index = %d, selected index = %d", m_current_position_index, index);

	if (m_current_position_index != index)
		return;

	std::vector<Browser_Window *> window_list = m_browser->get_window_list();
	if (!window_list[index]->m_ewk_view) {
		/* Workaround, the grey bg is displayed shortly when create deleted window. */
		Browser_View *browser_view = m_data_manager->get_browser_view();
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout),
							"hide,grey_background,signal", "");
	}

	if (!_show_zoom_in_effect(m_item_list[index])) {
		BROWSER_LOGE("_show_zoom_in_effect failed");
		return;
	}
}

void Browser_Multi_Window_View::__delete_window_icon_clicked_cb(void *data, Evas_Object *obj,
						const char *emission, const char *source)
{
	if (!data)
		return;

	BROWSER_LOGD("[%s]", __func__);
	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;
	multi_window_view->__delete_window_icon_clicked(obj);
}

void Browser_Multi_Window_View::__delete_window_icon_clicked(Evas_Object *layout_edje)
{
	int index = 0;
	for (index = 0 ; index < m_item_list.size() ; index++) {
		if (elm_layout_edje_get(m_item_list[index]) == layout_edje)
			break;
	}
	BROWSER_LOGD("selected index = %d", index);

	if (index != m_current_position_index)
		return;

	evas_object_hide(m_item_list[index]);

	if (m_item_list.size() == 2) {
		/* In case of the muti window is 2, when delete a window, hide the delete icon. */
		for (int i = 0 ; i < m_item_list.size() ; i++)
			edje_object_signal_emit(elm_layout_edje_get(m_item_list[i]),
				"hide,delete_window_icon,signal", "");
	}

	if (!_show_delete_window_scroll_effect())
		BROWSER_LOGE("_show_delete_window_scroll_effect failed");
}

void Browser_Multi_Window_View::__delete_window_scroll_finished_cb(void *data,
								Elm_Transit *transit)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;

	if (!multi_window_view->m_scroll_move_transit)
		return;

	multi_window_view->m_scroll_move_transit = NULL;

	multi_window_view->_delete_window_scroll_finished();
}

void Browser_Multi_Window_View::_delete_window_scroll_finished(void)
{
	BROWSER_LOGD("[%s]", __func__);
	elm_box_unpack(m_item_box, m_item_list[m_current_position_index]);
	evas_object_del(m_item_list[m_current_position_index]);
	m_item_list.erase(m_item_list.begin() + m_current_position_index);

	std::vector<Browser_Window *> window_list = m_browser->get_window_list();

	m_browser->delete_window(window_list[m_current_position_index]);

	BROWSER_LOGD("window_list.size() = %d", m_browser->get_window_list().size());
	if (m_current_position_index == m_browser->get_window_list().size())
		m_current_position_index = m_current_position_index - 1;

	if (m_page_control) {
		elm_object_part_content_unset(m_main_layout, "elm.swallow.page_control");
		evas_object_del(m_page_control);
	}

	m_page_control = elm_index_add(m_navi_bar);
	if (!m_page_control) {
		BROWSER_LOGD("elm_page_control_add failed");
		return;
	}
	elm_object_style_set(m_page_control, "pagecontrol");
	elm_index_horizontal_set(m_page_control, EINA_TRUE);
	elm_index_autohide_disabled_set(m_page_control, EINA_TRUE);

	memset(m_index_items, 0x00, sizeof(Elm_Object_Item *) * BROWSER_MULTI_WINDOW_MAX_COUNT);

	Elm_Object_Item *index_it = NULL;
	for (int i = 0 ; i < m_browser->get_window_list().size() ; i++) {
		char number[5] = {0, };
		sprintf(number, "%d", i + 1);

		if (m_current_position_index == i) {
			m_index_items[i] = index_it = elm_index_item_append(m_page_control, number, __index_selected_cb, this);
		} else {
			m_index_items[i] = elm_index_item_append(m_page_control, number, __index_selected_cb, this);
		}
	}

	elm_index_level_go(m_page_control, m_current_position_index);
	elm_index_item_selected_set(index_it, EINA_TRUE);

	elm_object_part_content_set(m_main_layout, "elm.swallow.page_control", m_page_control);
	evas_object_show(m_page_control);

	Browser_View *browser_view = m_data_manager->get_browser_view();

	BROWSER_LOGD("m_current_position_index = %d", m_current_position_index);

	window_list = m_browser->get_window_list();
	const char *title = browser_view->get_title(window_list[m_current_position_index]).c_str();
	const char *url = browser_view->get_url(window_list[m_current_position_index]).c_str();
	if (!title || strlen(title) == 0) {
		Evas_Object *entry = br_elm_editfield_entry_get(browser_view->m_option_header_url_edit_field);
		const char *entry_text = elm_entry_entry_get(entry);
		if (entry_text && strlen(entry_text)) {
			BROWSER_LOGD("[%s][%s]", entry_text, entry_text);
			char *markup = elm_entry_utf8_to_markup(entry_text);
			if (markup) {
				edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.title", markup);
				edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.url", markup);
				free(markup);
			}
		} else {
			edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.title", BR_STRING_EMPTY);
			edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.url", "");
		}
	} else {
		BROWSER_LOGD("[%s][%s]", title, url);
		char *markup = elm_entry_utf8_to_markup(title);
		if (markup) {
			edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.title", markup);
			free(markup);
		}

		markup = elm_entry_utf8_to_markup(url);
		if (markup) {
			edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.url", markup);
			free(markup);
		}
	}

	if (BROWSER_MULTI_WINDOW_MAX_COUNT > window_list.size())
		elm_object_item_disabled_set(m_new_window_button, EINA_FALSE);

	_show_grey_effect();
}

void Browser_Multi_Window_View::_show_grey_effect(void)
{
	BROWSER_LOGD("[%s]", __func__);
	for (int i = 0 ; i < m_item_list.size() ; i++) {
		if (i == m_current_position_index) {
			edje_object_signal_emit(elm_layout_edje_get(m_item_list[i]),
							"non_grey,snapshot,signal", "");
			if (m_item_list.size() > 1)
				edje_object_signal_emit(elm_layout_edje_get(m_item_list[i]),
						"show,delete_window_icon,signal", "");
		} else {
			edje_object_signal_emit(elm_layout_edje_get(m_item_list[i]),
							"grey,snapshot,signal", "");
		}
	}
}

Eina_Bool Browser_Multi_Window_View::_show_delete_window_scroll_effect(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int scroll_size = _get_scroll_page_size();

	m_scroll_move_transit = elm_transit_add();
	if (!m_scroll_move_transit) {
		BROWSER_LOGE("elm_transit_add failed");
		return EINA_FALSE;
	}
	/* Block event durring animation. */
	elm_transit_event_enabled_set(m_scroll_move_transit, EINA_TRUE);

	if (m_current_position_index != m_item_list.size() - 1) {
		/* If the deleted multi window item is not the last one. */
		for (int i = m_current_position_index + 1 ; i < m_item_list.size() ; i++)
			elm_transit_object_add(m_scroll_move_transit, m_item_list[i]);
		elm_transit_effect_translation_add(m_scroll_move_transit, 0, 0, scroll_size * (-1), 0);
	} else {
		elm_transit_object_add(m_scroll_move_transit, m_item_box);
		elm_transit_effect_translation_add(m_scroll_move_transit, 0, 0, scroll_size, 0);
	}

	elm_transit_tween_mode_set(m_scroll_move_transit, ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL);
	elm_transit_objects_final_state_keep_set(m_scroll_move_transit, EINA_TRUE);
	elm_transit_del_cb_set(m_scroll_move_transit, __delete_window_scroll_finished_cb, this);
	elm_transit_duration_set(m_scroll_move_transit, 0.7);
	elm_transit_go(m_scroll_move_transit);

	return EINA_TRUE;
}

Eina_Bool Browser_Multi_Window_View::_fill_multi_window_items(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int ewk_view_current_w = 0;
	int ewk_view_current_h = 0;
	evas_object_geometry_get(m_data_manager->get_browser_view()->m_focused_window->m_ewk_view,
					NULL, NULL, &ewk_view_current_w, &ewk_view_current_h);

	m_dummy_front_item_layout = elm_layout_add(m_item_box);
	if (!m_dummy_front_item_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}
	if (!elm_layout_file_set(m_dummy_front_item_layout, BROWSER_EDJE_DIR"/browser-multi-window.edj",
				"browser/multi-window/item"))
		BROWSER_LOGE("elm_layout_file_set failed");
	evas_object_size_hint_weight_set(m_dummy_front_item_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_dummy_front_item_layout, 0.5, 0.5);
	m_dummy_front_item_snapshot = elm_icon_add(m_dummy_front_item_layout);
	if (!m_dummy_front_item_snapshot) {
		BROWSER_LOGE("elm_icon_add failed");
		return EINA_FALSE;
	}
	int empty_item_width = 0;
	int padding = 22 * elm_scale_get();
	int snapshot_width = ewk_view_current_w * BROWSER_MULTI_WINDOW_ITEM_RATIO;
	int snapshot_height = ewk_view_current_h * BROWSER_MULTI_WINDOW_ITEM_RATIO;

	empty_item_width = (ewk_view_current_w - snapshot_width)/2 - padding;

	int scroll_size = _get_scroll_page_size();

	float page_relative = (float)((float)scroll_size/(float)ewk_view_current_w);
	elm_scroller_page_relative_set(m_scroller, page_relative, page_relative);

	evas_object_size_hint_min_set(m_dummy_front_item_snapshot, empty_item_width, snapshot_height);
	evas_object_resize(m_dummy_front_item_snapshot, empty_item_width, snapshot_height);
	elm_object_part_content_set(m_dummy_front_item_layout, "elm.swallow.snapshot",
							m_dummy_front_item_snapshot);
	elm_box_pack_end(m_item_box, m_dummy_front_item_snapshot);

	std::vector<Browser_Window *> window_list = m_browser->get_window_list();
	BROWSER_LOGD("window_list.size()=%d", window_list.size());
	for (int i = 0 ; i < window_list.size() ; i++) {
		Evas_Object *item = elm_layout_add(m_navi_bar);
		if (!item) {
			BROWSER_LOGE("elm_layout_add failed");
			return EINA_FALSE;
		}
		if (!elm_layout_file_set(item, BROWSER_EDJE_DIR"/browser-multi-window.edj",
						"browser/multi-window/item")) {
			BROWSER_LOGE("elm_layout_file_set failed");
			return EINA_FALSE;
		}
		evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(item, 0.5, 0.5);

		Evas_Object *snapshot_image = NULL;
#if defined(HORIZONTAL_UI)
		if (is_landscape())
			snapshot_image = window_list[i]->m_landscape_snapshot_image;
		else
#endif
			snapshot_image = window_list[i]->m_portrait_snapshot_image;

		elm_object_part_content_set(item, "elm.swallow.snapshot", snapshot_image);

		edje_object_signal_callback_add(elm_layout_edje_get(item), "mouse,clicked,1",
				"elm.swallow.snapshot", __snapshot_clicked_cb, this);

		edje_object_signal_callback_add(elm_layout_edje_get(item), "mouse,clicked,1",
				"elm.image.delete_window", __delete_window_icon_clicked_cb, this);

		if (window_list.size() > 1)
			edje_object_signal_emit(elm_layout_edje_get(item),
					"show,delete_window_icon,signal", "");

		elm_box_pack_end(m_item_box, item);

		m_item_list.push_back(item);
	}

	m_dummy_end_item_layout = elm_layout_add(m_item_box);
	if (!m_dummy_end_item_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}
	if (!elm_layout_file_set(m_dummy_end_item_layout, BROWSER_EDJE_DIR"/browser-multi-window.edj",
				"browser/multi-window/item"))
		BROWSER_LOGE("elm_layout_file_set failed");
	evas_object_size_hint_weight_set(m_dummy_end_item_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_dummy_end_item_layout, 0.5, 0.5);
	m_dummy_end_item_snapshot = elm_icon_add(m_dummy_end_item_layout);
	if (!m_dummy_end_item_snapshot) {
		BROWSER_LOGE("elm_icon_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_min_set(m_dummy_end_item_snapshot, empty_item_width, snapshot_height);
	evas_object_resize(m_dummy_end_item_snapshot, empty_item_width, snapshot_height);
	elm_object_part_content_set(m_dummy_end_item_layout, "elm.swallow.snapshot",
							m_dummy_end_item_snapshot);
	elm_box_pack_end(m_item_box, m_dummy_end_item_snapshot);

	return EINA_TRUE;
}

void Browser_Multi_Window_View::close_multi_window(void)
{
	BROWSER_LOGD("[%s]", __func__);
	__close_multi_window_button_clicked_cb(this, NULL, NULL);
}

void Browser_Multi_Window_View::__close_multi_window_button_clicked_cb(void *data,
								Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;

	if (multi_window_view->m_zoom_transit || multi_window_view->m_zoom_move_transit
	    || multi_window_view->m_scroll_move_transit) {
		BROWSER_LOGE("transit animation is running");
		return;
	}

	std::vector<Browser_Window *> window_list = m_browser->get_window_list();

	for (int i = 0 ; i < multi_window_view->m_item_list.size() ; i++) {
		/* The multi window item will be deleted,
		  * So unset the snapshot image from item layout.
		  * The snapshot is kept. */
		elm_object_part_content_unset(multi_window_view->m_item_list[i],
						"elm.swallow.snapshot");
#if defined(HORIZONTAL_UI)
		if (window_list[i]->m_landscape_snapshot_image) {
			evas_object_hide(window_list[i]->m_landscape_snapshot_image);
		}
#endif
		if (window_list[i]->m_portrait_snapshot_image) {
			evas_object_hide(window_list[i]->m_portrait_snapshot_image);
		}
	}
	Browser_View *browser_view = m_data_manager->get_browser_view();

	elm_object_part_content_set(browser_view->m_main_layout, "elm.swallow.control_bar", browser_view->m_control_bar);
	evas_object_show(browser_view->m_control_bar);

	if (multi_window_view->m_controlbar) {
		evas_object_del(multi_window_view->m_controlbar);
		multi_window_view->m_controlbar = NULL;
	}

	multi_window_view->_set_multi_window_mode(EINA_FALSE);

	m_data_manager->destroy_multi_window_view();

	browser_view->_set_multi_window_controlbar_text(window_list.size());

	browser_view->_update_back_forward_buttons();

	if (window_list.size() >= BROWSER_MULTI_WINDOW_MAX_COUNT)
		elm_object_item_disabled_set(browser_view->m_new_window_button, EINA_TRUE);
	else
		elm_object_item_disabled_set(browser_view->m_new_window_button, EINA_FALSE);

	/* Set the title if enter the multi window while loading. */
	browser_view->return_to_browser_view();

	browser_view->resume_ewk_view(browser_view->m_focused_window->m_ewk_view);
}

void Browser_Multi_Window_View::__new_window_scroll_finished_cb(void *data,
								Elm_Transit *transit)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;

	if (!multi_window_view->m_scroll_move_transit)
		return;

	multi_window_view->m_scroll_move_transit = NULL;

	if (!multi_window_view->_show_zoom_in_new_window_effect())
		BROWSER_LOGE("_show_zoom_in_new_window_effect failed");
}

void Browser_Multi_Window_View::__new_window_zoom_in_finished_cb(void *data,
								Elm_Transit *transit)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;

	if (!multi_window_view->m_zoom_transit)
		return;

	/* window_list[window_list.size() - 1] is new created window. */
	std::vector<Browser_Window *> window_list = m_browser->get_window_list();
	m_browser->set_focused_window(window_list[window_list.size() - 1]);

	multi_window_view->m_zoom_transit = NULL;

	if (multi_window_view->m_zoom_effect_image) {
		evas_object_del(multi_window_view->m_zoom_effect_image);
		multi_window_view->m_zoom_effect_image = NULL;
	}
	if (multi_window_view->m_flip_effect_image) {
		evas_object_del(multi_window_view->m_flip_effect_image);
		multi_window_view->m_flip_effect_image = NULL;
	}

	__close_multi_window_button_clicked_cb(multi_window_view, NULL, NULL);

	Browser_View *browser_view = m_data_manager->get_browser_view();
#if defined(FEATURE_MOST_VISITED_SITES)
	browser_view->load_url(BROWSER_MOST_VISITED_SITES_URL);
#else
	browser_view->load_url(BROWSER_BLANK_PAGE_URL);
#endif
}

Eina_Bool Browser_Multi_Window_View::_show_zoom_in_new_window_effect(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!m_browser->create_new_window(EINA_TRUE)) {
		BROWSER_LOGD("create_new_window failed");
		return EINA_FALSE;
	}

	std::vector<Browser_Window *> window_list = m_browser->get_window_list();

	m_zoom_effect_image = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
	if (!m_zoom_effect_image) {
		BROWSER_LOGE("evas_object_rectangle_add failed");
		return EINA_FALSE;
	}

	m_flip_effect_image = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
	if (!m_flip_effect_image) {
		BROWSER_LOGE("evas_object_rectangle_add failed");
		return EINA_FALSE;
	}

	int webview_x = 0;
	int webview_y = 0;
	int webview_w = 0;
	int webview_h = 0;
	Browser_View *browser_view = m_data_manager->get_browser_view();
	evas_object_geometry_get(browser_view->m_focused_window->m_ewk_view, &webview_x, &webview_y,
								&webview_w, &webview_h);

	evas_object_resize(m_flip_effect_image, webview_w, webview_h);
	evas_object_color_set(m_flip_effect_image, 255, 255, 255, 255);
	evas_object_move(m_flip_effect_image, webview_x, webview_y);

	int snapshot_x = 0;
	int snapshot_y = 0;
	int snapshot_w = 0;
	int snapshot_h = 0;
	int scroll_size = _get_scroll_page_size();

#if defined(HORIZONTAL_UI)
	if (is_landscape())
		evas_object_geometry_get(window_list[m_item_list.size() - 1]->m_landscape_snapshot_image,
					&snapshot_x, &snapshot_y, &snapshot_w, &snapshot_h);
	else
#endif
		evas_object_geometry_get(window_list[m_item_list.size() - 1]->m_portrait_snapshot_image,
					&snapshot_x, &snapshot_y, &snapshot_w, &snapshot_h);

	evas_object_resize(m_zoom_effect_image, snapshot_w, snapshot_h);
	evas_object_color_set(m_zoom_effect_image, 255, 255, 255, 255);
	evas_object_move(m_zoom_effect_image, snapshot_x + scroll_size, snapshot_y);
	evas_object_show(m_zoom_effect_image);

	/* m_zoom_effect_image is only for zoom out effect for the first time. */
	m_zoom_transit = elm_transit_add();
	if (!m_zoom_transit) {
		BROWSER_LOGE("elm_transit_add failed");
		return EINA_FALSE;
	}
	/* Block event durring animation. */
	elm_transit_event_enabled_set(m_zoom_transit, EINA_TRUE);

	elm_transit_object_add(m_zoom_transit, m_zoom_effect_image);
	elm_transit_object_add(m_zoom_transit, m_flip_effect_image);
	elm_transit_tween_mode_set(m_zoom_transit, ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL);
	elm_transit_objects_final_state_keep_set(m_zoom_transit, EINA_FALSE);
	elm_transit_effect_resizable_flip_add(m_zoom_transit, ELM_TRANSIT_EFFECT_FLIP_AXIS_Y, EINA_TRUE);
	elm_transit_duration_set(m_zoom_transit, 0.5);
	elm_transit_del_cb_set(m_zoom_transit, __new_window_zoom_in_finished_cb, this);
	elm_transit_go(m_zoom_transit);

	return EINA_TRUE;
}

Eina_Bool Browser_Multi_Window_View::_show_new_window_scroll_effect(void)
{
	BROWSER_LOGD("[%s]", __func__);

	int gap_to_end = m_item_list.size() - m_current_position_index;
	int scroll_size = _get_scroll_page_size();

	m_scroll_move_transit = elm_transit_add();
	if (!m_scroll_move_transit) {
		BROWSER_LOGE("elm_transit_add failed");
		return EINA_FALSE;
	}
	/* Block event durring animation. */
	elm_transit_event_enabled_set(m_scroll_move_transit, EINA_TRUE);

	elm_transit_object_add(m_scroll_move_transit, m_item_box);
	elm_transit_tween_mode_set(m_scroll_move_transit, ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL);
	elm_transit_objects_final_state_keep_set(m_scroll_move_transit, EINA_TRUE);
	elm_transit_effect_translation_add(m_scroll_move_transit, 0, 0,
						(scroll_size * gap_to_end) * (-1), 0);
	elm_transit_del_cb_set(m_scroll_move_transit, __new_window_scroll_finished_cb, this);

	if (_is_grid_mode())
		elm_transit_duration_set(m_scroll_move_transit, 0.1);
	else
	elm_transit_duration_set(m_scroll_move_transit, 0.7);
	elm_transit_go(m_scroll_move_transit);

	return EINA_TRUE;
}

void Browser_Multi_Window_View::__new_window_button_clicked_cb(void *data,
								Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;
	if (multi_window_view->m_scroll_move_transit || multi_window_view->m_zoom_transit) {
		BROWSER_LOGE("animation is running");
		return;
	}

	multi_window_view->_show_new_window_scroll_effect();

	Browser_View *browser_view = m_data_manager->get_browser_view();

	elm_object_part_content_unset(browser_view->m_main_layout, "elm.swallow.control_bar");

	elm_object_part_content_set(browser_view->m_main_layout, "elm.swallow.control_bar", browser_view->m_control_bar);

	if (multi_window_view->m_new_window_button)
		elm_object_item_disabled_set(multi_window_view->m_new_window_button, EINA_TRUE);
	if (multi_window_view->m_change_view_button)
		elm_object_item_disabled_set(multi_window_view->m_change_view_button, EINA_TRUE);
	elm_object_disabled_set(multi_window_view->m_cancel_button, EINA_TRUE);

	/* clear url entry filed when click new window button. */
	browser_view->_set_url_entry("");
	edje_object_signal_emit(elm_layout_edje_get(browser_view->m_url_entry_layout), "hide,favicon,signal", "");
	edje_object_signal_emit(elm_layout_edje_get(browser_view->m_option_header_url_entry_layout),
									"hide,favicon,signal", "");
	browser_view->_set_navigationbar_title("");
}

void Browser_Multi_Window_View::__scroller_animation_stop_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;
	multi_window_view->_scroller_animation_stop();
}

int Browser_Multi_Window_View::_get_scroll_page_size(void)
{
	BROWSER_LOGD("[%s]", __func__);

	int snapshot_w = 0;
	std::vector<Browser_Window *> window_list = m_browser->get_window_list();

#if defined(HORIZONTAL_UI)
	if (is_landscape())
		evas_object_geometry_get(window_list[m_current_position_index]->m_landscape_snapshot_image,
					NULL, NULL, &snapshot_w, NULL);
	else
#endif
		evas_object_geometry_get(window_list[m_current_position_index]->m_portrait_snapshot_image,
					NULL, NULL, &snapshot_w, NULL);

	int padding = 22 * elm_scale_get();

	int scroll_size = snapshot_w + (padding * 2);

	return scroll_size;
}

void Browser_Multi_Window_View::_scroller_animation_stop(void)
{
	int scroller_x = 0;
	elm_scroller_region_get(m_scroller, &scroller_x, NULL, NULL, NULL);

	int padding = 22 * elm_scale_get();
	int scroll_size = _get_scroll_page_size();

	/* Workaround.
	 * the scroll size is 337 or 338. It seems that the gap of floating operation.
	 * So decrease the scroll_size by padding. */
	m_current_position_index = scroller_x / (scroll_size - padding);
	BROWSER_LOGD("scroller_x = %d, scroll_size = %d, m_current_position_index = %d",
			scroller_x, scroll_size, m_current_position_index);

	Browser_View *browser_view = m_data_manager->get_browser_view();
	std::vector<Browser_Window *> window_list = m_browser->get_window_list();

	if ( m_current_position_index + 1 > window_list.size())
		m_current_position_index = window_list.size() - 1;

	if (!window_list[m_current_position_index]->m_ewk_view) {
		std::string title_string = window_list[m_current_position_index]->m_title;
		std::string url_string = window_list[m_current_position_index]->m_url;
		if (title_string.empty()) {
			edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.title", BR_STRING_EMPTY);
			edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.url", "");
		} else {
			BROWSER_LOGD("[%s][%s]", title_string.c_str(), url_string.c_str());
			char *markup = elm_entry_utf8_to_markup(title_string.c_str());
			if (markup) {
				edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.title", markup);
				free(markup);
			}

			markup = elm_entry_utf8_to_markup(url_string.c_str());
			if (markup) {
				edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.url", markup);
				free(markup);
			}
		}
	} else if (!window_list[m_current_position_index]->m_title.empty()) {
		BROWSER_LOGD("[%s][%s]", window_list[m_current_position_index]->m_title.c_str(), window_list[m_current_position_index]->m_url.c_str());
		/* If the ewk view is deleted because of unused case(etc. low memory),
		  * and it is re-created, then set the title. */
		char *markup = elm_entry_utf8_to_markup(window_list[m_current_position_index]->m_title.c_str());
		if (markup) {
			edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.title", markup);
			free(markup);
		}
		markup = elm_entry_utf8_to_markup(window_list[m_current_position_index]->m_url.c_str());
		if (markup) {
			edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.url", markup);
			free(markup);
		}
	} else {
		std::string title = browser_view->get_title(window_list[m_current_position_index]);
		std::string url = browser_view->get_url(window_list[m_current_position_index]);
		if (title.empty()) {
			edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.title", BR_STRING_EMPTY);
			edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.url", "");
		} else {
			std::string title_string = std::string(title);
			std::string url_string = std::string(url);

			char *markup = elm_entry_utf8_to_markup(title_string.c_str());
			BROWSER_LOGD("[%s][%s]", title_string.c_str(), url_string.c_str());
			if (markup) {
				edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.title", markup);
				free(markup);
			}

			markup = elm_entry_utf8_to_markup(url_string.c_str());
			if (markup) {
				edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "elm.url", markup);
				free(markup);
			}
		}
	}

	elm_index_level_go(m_page_control, m_current_position_index);

	if (m_index_items[m_current_position_index])
		elm_index_item_selected_set(m_index_items[m_current_position_index], EINA_TRUE);
	BROWSER_LOGD("elm_index_level_go, m_current_position_index = %d", m_current_position_index);

	_show_grey_effect();
}

Eina_Bool Browser_Multi_Window_View::_is_grid_mode(void)
{
	BROWSER_LOGD("[%s]", __func__);
	const char* state = edje_object_part_state_get(elm_layout_edje_get(m_main_layout),
								"elm.swallow.scroller", NULL);
	if (state && !strncmp(state, "default", strlen("default")))
		return EINA_FALSE;
	else
		return EINA_TRUE;
}

/*  zoom in effect when user selects mult window item. */
Eina_Bool Browser_Multi_Window_View::_show_grid_mode_zoom_in_effect(int index)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Window *focused_window = m_data_manager->get_browser_view()->m_focused_window;

	std::vector<Browser_Window *> window_list = m_browser->get_window_list();
	Elm_Object_Item *gengrid_item = elm_gengrid_first_item_get(m_gengrid);
	for (int i = 1 ; i < window_list.size() ; i++) {
		if (index + 1 == i)
			break;
		gengrid_item = elm_gengrid_item_next_get(gengrid_item);
	}

	const Evas_Object *snapshot_layout = elm_object_item_part_content_get(gengrid_item, "elm.swallow.icon");

	int snapshot_x = 0;
	int snapshot_y = 0;
	int snapshot_w = 0;
	int snapshot_h = 0;
	evas_object_geometry_get(snapshot_layout, &snapshot_x, &snapshot_y, &snapshot_w, &snapshot_h);

	evas_object_hide((Evas_Object *)snapshot_layout);

	int current_ewk_view_x = 0;
	int current_ewk_view_y = 0;
	int current_ewk_view_w = 0;
	int current_ewk_view_h = 0;
	Browser_View *browser_view = m_data_manager->get_browser_view();
	evas_object_geometry_get(browser_view->m_focused_window->m_ewk_view, &current_ewk_view_x, &current_ewk_view_y,
								&current_ewk_view_w, &current_ewk_view_h);

	int to_x = (current_ewk_view_w - snapshot_w) / 2 - snapshot_x;
	int to_y = (current_ewk_view_h - snapshot_h) / 2 - snapshot_y + current_ewk_view_y;

	BROWSER_LOGD("snapshot_x = %d, snapshot_y= %d, snapshot_w = %d, snapshot_h=%d, ", snapshot_x, snapshot_y, snapshot_w, snapshot_h);

	if (window_list[index]->m_ewk_view == NULL) {
		m_zoom_effect_image = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
		if (!m_zoom_effect_image) {
			BROWSER_LOGE("evas_object_rectangle_add failed");
			return EINA_FALSE;
		}
		evas_object_resize(m_zoom_effect_image, snapshot_w, snapshot_h);
		evas_object_color_set(m_zoom_effect_image, 255, 255, 255, 255);

		m_flip_effect_image = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
		if (!m_flip_effect_image) {
			BROWSER_LOGE("evas_object_rectangle_add failed");
			return EINA_FALSE;
		}
		evas_object_resize(m_flip_effect_image, current_ewk_view_w, current_ewk_view_h);
		evas_object_color_set(m_flip_effect_image, 255, 255, 255, 255);
	} else {
		m_zoom_effect_image = _capture_snapshot(window_list[index],
					(float)((float)snapshot_w / (float)current_ewk_view_w));
		if (!m_zoom_effect_image) {
			BROWSER_LOGE("_capture_snapshot failed");
			m_zoom_effect_image = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
			if (!m_zoom_effect_image) {
				BROWSER_LOGE("evas_object_rectangle_add failed");
				return EINA_FALSE;
			}
			evas_object_resize(m_zoom_effect_image, snapshot_w, snapshot_h);
			evas_object_color_set(m_zoom_effect_image, 255, 255, 255, 255);
		}

		m_flip_effect_image = _capture_snapshot(window_list[index], 1.0);
		if (!m_flip_effect_image) {
			BROWSER_LOGE("_capture_snapshot failed");
			m_flip_effect_image = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
			if (!m_flip_effect_image) {
				BROWSER_LOGE("evas_object_rectangle_add failed");
				return EINA_FALSE;
			}
			evas_object_resize(m_flip_effect_image, current_ewk_view_w, current_ewk_view_h);
			evas_object_color_set(m_flip_effect_image, 255, 255, 255, 255);
		}
	}
	evas_object_move(m_flip_effect_image, current_ewk_view_x, current_ewk_view_y);

	evas_object_move(m_zoom_effect_image, snapshot_x, snapshot_y);
	evas_object_show(m_zoom_effect_image);

	/* m_zoom_effect_image is only for zoom out effect for the first time. */
	m_zoom_transit = elm_transit_add();
	if (!m_zoom_transit) {
		BROWSER_LOGE("elm_transit_add failed");
		return EINA_FALSE;
	}
	/* Block event durring animation. */
	elm_transit_event_enabled_set(m_zoom_transit, EINA_TRUE);

	m_select_item_index = index;

#if defined(FEATURE_MOST_VISITED_SITES)
	/* Destroy speed dial little eariler because of blinking issue. */
	if (!(browser_view->get_url(window_list[m_select_item_index]).empty()))
		browser_view->_show_most_visited_sites(EINA_FALSE);
#endif

	elm_transit_object_add(m_zoom_transit, m_zoom_effect_image);
	elm_transit_object_add(m_zoom_transit, m_flip_effect_image);
	elm_transit_tween_mode_set(m_zoom_transit, ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL);
	elm_transit_objects_final_state_keep_set(m_zoom_transit, EINA_FALSE);
	elm_transit_effect_resizable_flip_add(m_zoom_transit, ELM_TRANSIT_EFFECT_FLIP_AXIS_Y, EINA_TRUE);
	elm_transit_duration_set(m_zoom_transit, 0.5);
	elm_transit_del_cb_set(m_zoom_transit, __zoom_in_finished_cb, this);
	elm_transit_go(m_zoom_transit);

	/* If the url bar in browser scroller is being displayed,
	 * the zoom animation should be move up because of the url bar gap.
	 * So add translation effect as well. */
	m_zoom_move_transit = elm_transit_add();
	if (!m_zoom_move_transit) {
		BROWSER_LOGE("elm_transit_add failed");
		return EINA_FALSE;
	}
	/* Block event durring animation. */
	elm_transit_event_enabled_set(m_zoom_move_transit, EINA_TRUE);

	elm_transit_object_add(m_zoom_move_transit, m_zoom_effect_image);
	elm_transit_object_add(m_zoom_move_transit, m_flip_effect_image);
	elm_transit_tween_mode_set(m_zoom_move_transit, ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL);
	elm_transit_objects_final_state_keep_set(m_zoom_move_transit, EINA_FALSE);

	BROWSER_LOGD("to_x = %d, to_y = %d", to_x, to_y);
	elm_transit_effect_translation_add(m_zoom_move_transit, 0, 0, to_x, to_y);

	elm_transit_duration_set(m_zoom_move_transit, 0.5);
	elm_transit_go(m_zoom_move_transit);

	return EINA_TRUE;
}

Eina_Bool Browser_Multi_Window_View::__delete_window_icon_grid_mode(int index)
{
	BROWSER_LOGD("[%s]", __func__);
	std::vector<Browser_Window *> window_list = m_browser->get_window_list();
	Elm_Object_Item *gengrid_item = elm_gengrid_first_item_get(m_gengrid);
	for (int i = 1 ; i < window_list.size() ; i++) {
		if (index + 1 == i)
			break;
		gengrid_item = elm_gengrid_item_next_get(gengrid_item);
	}
	elm_object_item_del(gengrid_item);

	m_current_position_index = index;

	_delete_window_scroll_finished();

	if (m_item_list.size() == 1) {
		/* In case of the muti window is 1, hide the delete icon in normal mode. */
		edje_object_signal_emit(elm_layout_edje_get(m_item_list[0]),
				"hide,delete_window_icon,signal", "");
	}

	if (!_create_gengrid()) {
		BROWSER_LOGE("_create_gengrid failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

void Browser_Multi_Window_View::__delete_window_icon_grid_mode_clicked_cb(void *data,
					Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	gengrid_callback_param *param = (gengrid_callback_param *)data;
	Browser_Multi_Window_View *multi_window_view = NULL;
	multi_window_view = (Browser_Multi_Window_View *)(param->multi_window_view);

	if (multi_window_view->m_is_reordering) {
		BROWSER_LOGD("reodering mode");
		multi_window_view->m_is_reordering = EINA_FALSE;
		return;
	}

	int index = param->index;
	multi_window_view->__delete_window_icon_grid_mode(index);
}

void Browser_Multi_Window_View::__snapshot_grid_mode_clicked_cb(void *data,
					Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	gengrid_callback_param *param = (gengrid_callback_param *)data;
	Browser_Multi_Window_View *multi_window_view = NULL;
	multi_window_view = (Browser_Multi_Window_View *)(param->multi_window_view);
	if (multi_window_view->m_is_reordering) {
		BROWSER_LOGD("reodering mode");
		multi_window_view->m_is_reordering = EINA_FALSE;
		return;
	}
	if (multi_window_view->m_zoom_transit) {
		BROWSER_LOGD("zoom transit is running");
		return;
	}

	int index = param->index;

	if (!multi_window_view->_show_grid_mode_zoom_in_effect(index))
		BROWSER_LOGE("_show_grid_mode_zoom_in_effect failed");
}

Evas_Object *Browser_Multi_Window_View::_get_gengrid_icon_cb(void *data,
						Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	gengrid_callback_param *param = (gengrid_callback_param *)data;
	Browser_Multi_Window_View *multi_window_view = NULL;
	multi_window_view = (Browser_Multi_Window_View *)(param->multi_window_view);
	int index = param->index;

	if (!strncmp(part, "elm.swallow.icon", strlen("elm.swallow.icon"))) {
		Evas_Object *snapshot_image = evas_object_image_add(evas_object_evas_get(obj));
		if (!snapshot_image) {
			BROWSER_LOGE("evas_object_image_add failed");
			return NULL;
		}
		std::vector<Browser_Window *> window_list = m_browser->get_window_list();
		int snapshot_w = 0;
		int snapshot_h = 0;
#if defined(HORIZONTAL_UI)
		if (multi_window_view->is_landscape()) {
			evas_object_image_source_set(snapshot_image, window_list[index]->m_landscape_snapshot_image);
			evas_object_geometry_get(window_list[index]->m_landscape_snapshot_image, NULL, NULL,
										&snapshot_w, &snapshot_h);
			BROWSER_LOGD("landscape, snapshot_w=%d, snapshot_h=%d", snapshot_w, snapshot_h);
		}
		else
#endif
		{
			evas_object_image_source_set(snapshot_image, window_list[index]->m_portrait_snapshot_image);
			evas_object_geometry_get(window_list[index]->m_portrait_snapshot_image, NULL, NULL,
										&snapshot_w, &snapshot_h);
			BROWSER_LOGD("portrait, snapshot_w=%d, snapshot_h=%d", snapshot_w, snapshot_h);
		}

		int item_w = 0;
		int item_h = 0;
		elm_gengrid_item_size_get(multi_window_view->m_gengrid, &item_w, &item_h);
		evas_object_image_fill_set(snapshot_image, 0, 0, item_w, item_h);

		Evas_Object *grid_item_layout = elm_layout_add(obj);
		if (!grid_item_layout) {
			BROWSER_LOGE("grid_item_layout failed");
			return NULL;
		}
		if (!elm_layout_file_set(grid_item_layout, BROWSER_EDJE_DIR"/browser-multi-window.edj",
									"browser/multi-window/grid_item"))
			BROWSER_LOGE("elm_layout_file_set failed");

		if (window_list.size() > 1)
			edje_object_signal_emit(elm_layout_edje_get(grid_item_layout),
							"show,delete_icon,signal", "");

		evas_object_size_hint_weight_set(grid_item_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(grid_item_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_part_content_set(grid_item_layout, "elm.swallow.snapshot", snapshot_image);
		evas_object_show(snapshot_image);
		evas_object_show(grid_item_layout);

		edje_object_signal_callback_add(elm_layout_edje_get(grid_item_layout), "mouse,clicked,1",
					"delete_icon_click_area", __delete_window_icon_grid_mode_clicked_cb, data);
		edje_object_signal_callback_add(elm_layout_edje_get(grid_item_layout), "mouse,up,1",
					"elm.swallow.snapshot", __snapshot_grid_mode_clicked_cb, data);

		return grid_item_layout;
	}

	return NULL;
}

void Browser_Multi_Window_View::__gengrid_item_longpress_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!data)
		return;

	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;
	multi_window_view->m_is_reordering = EINA_TRUE;
}

Eina_Bool Browser_Multi_Window_View::_reorder_windows(void)
{
	BROWSER_LOGD("[%s]", __func__);
	std::vector<Browser_Window *> window_list = m_browser->get_window_list();
	std::vector<Browser_Window *> reodered_list;
	std::vector<Evas_Object *> reodered_item = m_item_list;
	reodered_list.clear();
	reodered_item.clear();

	Elm_Object_Item *it = elm_gengrid_first_item_get(m_gengrid);
	for (int i = 0 ; i < window_list.size() ; i++) {
		if (!it)
			break;
		gengrid_callback_param *it_data = (gengrid_callback_param *)elm_object_item_data_get(it);
		BROWSER_LOGD("i=[%d], index=[%d]", i, it_data->index);

		reodered_list.push_back(window_list[it_data->index]);
		reodered_item.push_back(m_item_list[it_data->index]);

		it_data->index = i;

		it = elm_gengrid_item_next_get(it);
	}

	/* Reoder the browser class windows. */
	m_browser->change_order(reodered_list);

	/* Reoder the multi window normal mode items. */
	m_item_list.clear();
	m_item_list = reodered_item;

	/* Reoder the multi window normal mode items. */
	elm_box_unpack_all(m_item_box);
	elm_box_pack_end(m_item_box, m_dummy_front_item_snapshot);
	for (int i = 0 ; i < m_item_list.size() ; i++)
		elm_box_pack_end(m_item_box, m_item_list[i]);
	elm_box_pack_end(m_item_box, m_dummy_end_item_snapshot);

	evas_object_smart_callback_call(m_scroller, "scroll,anim,stop", NULL);

	return EINA_TRUE;
}

void Browser_Multi_Window_View::__gengrid_item_moved_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!data)
		return;

	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;
	if (!multi_window_view->_reorder_windows())
		BROWSER_LOGE("_reorder_windows failed");
}

Eina_Bool Browser_Multi_Window_View::_create_gengrid(void)
{
	if (m_gengrid)
		evas_object_del(m_gengrid);

	m_gengrid = elm_gengrid_add(m_navi_bar);
	if (!m_gengrid) {
		BROWSER_LOGE("elm_gengrid_add failed");
		return EINA_FALSE;
	}
	std::vector<Browser_Window *> window_list = m_browser->get_window_list();

	elm_object_part_content_set(m_main_layout, "elm.swallow.gengrid", m_gengrid);

	int window_w = 0;
	evas_object_geometry_get(m_win, NULL, NULL, &window_w, NULL);

	int gengrid_area_h = 0;
	edje_object_part_geometry_get(elm_layout_edje_get(m_main_layout), "elm.swallow.gengrid",
								NULL, NULL, NULL, &gengrid_area_h);

	elm_gengrid_item_size_set(m_gengrid, window_w / 3, gengrid_area_h / 3);
	elm_gengrid_horizontal_set(m_gengrid, EINA_FALSE);
	elm_gengrid_bounce_set(m_gengrid, EINA_FALSE, EINA_FALSE);
	elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
	elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_NONE);
	elm_gengrid_align_set(m_gengrid, 0.0, 0.0);
	elm_gengrid_reorder_mode_set(m_gengrid, EINA_TRUE);
	evas_object_show(m_gengrid);

	evas_object_smart_callback_add(m_gengrid, "moved", __gengrid_item_moved_cb, this);
	evas_object_smart_callback_add(m_gengrid, "longpressed", __gengrid_item_longpress_cb, this);

	m_gengrid_item_class.item_style = "default_grid";
	m_gengrid_item_class.func.text_get = NULL;
	m_gengrid_item_class.func.content_get = _get_gengrid_icon_cb;
	m_gengrid_item_class.func.state_get = NULL;
	m_gengrid_item_class.func.del = NULL;

	for (int i = 0 ; i < window_list.size() ; i++ ) {
		m_callback_param[i].multi_window_view = this;
		m_callback_param[i].index = i;
		elm_gengrid_item_append(m_gengrid, &m_gengrid_item_class, &m_callback_param[i], NULL, NULL);
	}

	return EINA_TRUE;
}

void Browser_Multi_Window_View::__view_change_button_clicked_cb(void *data,
								Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;

	if (multi_window_view->m_zoom_transit)
		return;

	if (!multi_window_view->_is_grid_mode()) {
		if (!multi_window_view->_create_gengrid()) {
			BROWSER_LOGE("_create_gengrid failed");
			return;
		}
		edje_object_signal_emit(elm_layout_edje_get(multi_window_view->m_main_layout),
									"show,grid_mode,signal", "");

		elm_toolbar_item_icon_set(multi_window_view->m_change_view_button,
					BROWSER_IMAGE_DIR"/I01_controlbar_icon_view_change.png");
	} else {
		/* To sync grid mode, if the window is deleted or reodered at grid mode. */
		evas_object_smart_callback_call(multi_window_view->m_scroller, "scroll,anim,stop", NULL);

		edje_object_signal_emit(elm_layout_edje_get(multi_window_view->m_main_layout),
									"show,normal_mode,signal", "");
		elm_toolbar_item_icon_set(multi_window_view->m_change_view_button,
					BROWSER_IMAGE_DIR"/I01_controlbar_icon_view_change_3x3.png");
	}
}

void Browser_Multi_Window_View::__delete_gengrid_cb(void *data,
					Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;
	if (multi_window_view->m_gengrid) {
		evas_object_del(multi_window_view->m_gengrid);
		multi_window_view->m_gengrid = NULL;
	}
}

Eina_Bool Browser_Multi_Window_View::__zoom_out_effect_idler_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;
	multi_window_view->m_zoom_out_effect_idler = NULL;

	if (multi_window_view->m_init_grid_mode) {
		multi_window_view->__zoom_out_finished();

		__view_change_button_clicked_cb(multi_window_view, NULL, NULL);
		edje_object_signal_emit(elm_layout_edje_get(multi_window_view->m_main_layout),
								"show,bg,signal", "");
		multi_window_view->m_init_grid_mode = EINA_FALSE;
		return ECORE_CALLBACK_CANCEL;
	}

	/*  zoom out effect for the first time when user excutes multi window. */
	if (!multi_window_view->_show_zoom_out_effect()) {
		BROWSER_LOGD("_show_zoom_out_effect failed");
		return ECORE_CALLBACK_CANCEL;
	}

	edje_object_signal_emit(elm_layout_edje_get(multi_window_view->m_main_layout),
						"show,bg,signal", "");

	return ECORE_CALLBACK_CANCEL;
}

void Browser_Multi_Window_View::__index_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Multi_Window_View *multi_window_view = (Browser_Multi_Window_View *)data;
	Elm_Object_Item *selected_it = (Elm_Object_Item *)event_info;
	int index = atoi(elm_index_item_letter_get(selected_it));
	BROWSER_LOGD("selected index = %d, multi_window_view->m_current_position_index = %d",
			index, multi_window_view->m_current_position_index);

	if (multi_window_view->m_current_position_index == index - 1 || index - 1 < 0)
		return;

	multi_window_view->m_current_position_index = index - 1;

	int scroll_size = multi_window_view->_get_scroll_page_size();
	int scroller_y = 0;
	int scroller_w = 0;
	int scroller_h = 0;
	BROWSER_LOGD("multi_window_view->m_current_position_index = %d, scroll_size=%d",
				multi_window_view->m_current_position_index, scroll_size);
	elm_scroller_region_get(multi_window_view->m_scroller, NULL, &scroller_y, &scroller_w, &scroller_h);
	elm_scroller_region_bring_in(multi_window_view->m_scroller, scroll_size * multi_window_view->m_current_position_index,
						scroller_y, scroller_w, scroller_h);
}

Evas_Object *Browser_Multi_Window_View::_create_control_bar(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *controlbar_layout;
	controlbar_layout = elm_layout_add(m_main_layout);
	if (!controlbar_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	if (!elm_layout_file_set(controlbar_layout, BROWSER_EDJE_DIR"/browser-view-control-bar.edj",
				"browser-view-controlbar")) {
		BROWSER_LOGE("Can not set layout browser-view-controlbar\n");
		return NULL;
	}
	evas_object_size_hint_weight_set(controlbar_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(controlbar_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(controlbar_layout);

	Evas_Object *control_bar;
	control_bar = elm_toolbar_add(controlbar_layout);
	if (control_bar) {
		elm_object_style_set(control_bar, "browser/browser-view");

		elm_toolbar_shrink_mode_set(control_bar, ELM_TOOLBAR_SHRINK_EXPAND);
		m_new_window_button =  elm_toolbar_item_append(control_bar, BROWSER_IMAGE_DIR"/I01_controlbar_icon_new_window.png",
										NULL, __new_window_button_clicked_cb, this);

		Elm_Object_Item *item = elm_toolbar_item_append(control_bar, NULL, NULL, NULL, NULL);
		elm_object_item_disabled_set(item, EINA_TRUE);

		m_change_view_button = elm_toolbar_item_append(control_bar,
					BROWSER_IMAGE_DIR"/I01_controlbar_icon_view_change_3x3.png", NULL,
					__view_change_button_clicked_cb, this);

		item = elm_toolbar_item_append(control_bar, NULL, NULL, NULL, NULL);
		elm_object_item_disabled_set(item, EINA_TRUE);

		elm_object_part_content_set(controlbar_layout, "elm.swallow.controlbar", control_bar);

		m_cancel_button = elm_button_add(control_bar);
		if (!m_cancel_button) {
			BROWSER_LOGE("elm_button_add failed");
			return NULL;
		}
		elm_object_style_set(m_cancel_button, "browser/multi_window_cancel_button");
		elm_object_text_set(m_cancel_button, BR_STRING_CANCEL);

		elm_object_part_content_set(controlbar_layout, "elm.swallow.back_button", m_cancel_button);
		evas_object_smart_callback_add(m_cancel_button, "clicked", __close_multi_window_button_clicked_cb, this);
		evas_object_show(m_cancel_button);

		evas_object_show(control_bar);
	}

	return controlbar_layout;
}

Eina_Bool Browser_Multi_Window_View::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_View *browser_view = m_data_manager->get_browser_view();

	/* m_zoom_effect_image is only for zoom out effect for the first time. */
	Browser_Window *focused_window = m_data_manager->get_browser_view()->m_focused_window;
	if (browser_view->get_url(focused_window).empty()) {
		m_zoom_effect_image = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
		if (!m_zoom_effect_image) {
			BROWSER_LOGE("evas_object_rectangle_add failed");
			return EINA_FALSE;
		}
		int focused_ewk_view_w = 0;
		int focused_ewk_view_h = 0;
		evas_object_geometry_get(browser_view->m_focused_window->m_ewk_view, NULL, NULL,
							&focused_ewk_view_w, &focused_ewk_view_h);

		evas_object_size_hint_min_set(m_zoom_effect_image, focused_ewk_view_w, focused_ewk_view_h);
		evas_object_resize(m_zoom_effect_image, focused_ewk_view_w, focused_ewk_view_h);
		evas_object_color_set(m_zoom_effect_image, 255, 255, 255, 255);

		m_flip_effect_image = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
		if (!m_flip_effect_image) {
			BROWSER_LOGE("evas_object_rectangle_add failed");
			return EINA_FALSE;
		}

		evas_object_size_hint_min_set(m_flip_effect_image,
			(int)(focused_ewk_view_w * BROWSER_MULTI_WINDOW_ITEM_RATIO),
			(int)(focused_ewk_view_h * BROWSER_MULTI_WINDOW_ITEM_RATIO));
		evas_object_resize(m_flip_effect_image,
			(int)(focused_ewk_view_w * BROWSER_MULTI_WINDOW_ITEM_RATIO),
			(int)(focused_ewk_view_h * BROWSER_MULTI_WINDOW_ITEM_RATIO));

		evas_object_color_set(m_flip_effect_image, 255, 255, 255, 255);
	} else {
		m_zoom_effect_image = _capture_snapshot(focused_window, 1.0);
		m_flip_effect_image = _capture_snapshot(focused_window, BROWSER_MULTI_WINDOW_ITEM_RATIO);
	}

	if (!m_zoom_effect_image || !m_flip_effect_image) {
		BROWSER_LOGD("_capture_snapshot failed");
		return EINA_FALSE;
	}

	Evas_Object *snapshot_image = NULL;
	if (browser_view->get_url(focused_window).empty()) {
		snapshot_image = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
		if (!snapshot_image) {
			BROWSER_LOGE("evas_object_rectangle_add failed");
			return EINA_FALSE;
		}
		int focused_ewk_view_w = 0;
		int focused_ewk_view_h = 0;
		evas_object_geometry_get(browser_view->m_focused_window->m_ewk_view, NULL, NULL,
						&focused_ewk_view_w, &focused_ewk_view_h);

		evas_object_size_hint_min_set(snapshot_image, (int)(focused_ewk_view_w * BROWSER_MULTI_WINDOW_ITEM_RATIO),
							(int)(focused_ewk_view_h * BROWSER_MULTI_WINDOW_ITEM_RATIO));
		evas_object_resize(snapshot_image, (int)(focused_ewk_view_w * BROWSER_MULTI_WINDOW_ITEM_RATIO),
						(int)(focused_ewk_view_h * BROWSER_MULTI_WINDOW_ITEM_RATIO));
		evas_object_color_set(snapshot_image, 255, 255, 255, 255);
	} else {
		snapshot_image = _capture_snapshot(focused_window, BROWSER_MULTI_WINDOW_ITEM_RATIO);
	}

	if (!snapshot_image) {
		BROWSER_LOGE("_capture_snapshot failed");
		return EINA_FALSE;
	}

	m_main_layout = elm_layout_add(browser_view->m_navi_bar);
	if (!m_main_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}
	if (!elm_layout_file_set(m_main_layout, BROWSER_EDJE_DIR"/browser-multi-window.edj",
							"browser/browser-multi-window")) {
		BROWSER_LOGE("elm_layout_file_set failed");
		return EINA_FALSE;
	}
	evas_object_show(m_main_layout);

	m_scroller = elm_scroller_add(m_navi_bar);
	if (!m_scroller) {
		BROWSER_LOGD("elm_scroller_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_bounce_set(m_scroller, EINA_TRUE, EINA_FALSE);

	elm_object_part_content_set(m_main_layout, "elm.swallow.scroller", m_scroller);
	elm_scroller_policy_set(m_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	evas_object_show(m_scroller);

	/* After change normal mode -> gengrid mode, delete the gengrid after view change effect in __delete_gengrid_cb. */
	edje_object_signal_callback_add(elm_layout_edje_get(m_main_layout), "delete,gengrid,signal",
				"*", __delete_gengrid_cb, this);

	evas_object_smart_callback_add(m_scroller, "scroll,anim,stop", __scroller_animation_stop_cb, this);
	evas_object_smart_callback_add(m_scroller, "edge,left", __scroller_animation_stop_cb, this);
	evas_object_smart_callback_add(m_scroller, "edge,right", __scroller_animation_stop_cb, this);

	m_item_box = elm_box_add(m_navi_bar);
	if (!m_item_box) {
		BROWSER_LOGD("elm_box_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_item_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_item_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_horizontal_set(m_item_box, EINA_TRUE);
	elm_object_content_set(m_scroller, m_item_box);
	evas_object_show(m_item_box);

	m_controlbar = _create_control_bar();
	if (!m_controlbar) {
		BROWSER_LOGE("_create_control_bar failed");
		return EINA_FALSE;
	}
	elm_object_part_content_unset(browser_view->m_main_layout, "elm.swallow.control_bar");
	evas_object_hide(browser_view->m_control_bar);

	elm_object_part_content_set(browser_view->m_main_layout, "elm.swallow.control_bar", m_controlbar);

	int index = 0;
	std::vector<Browser_Window *> window_list = m_browser->get_window_list();
	for (index = 0 ; index < window_list.size() ; index++) {
		if (window_list[index] == m_data_manager->get_browser_view()->m_focused_window)
			break;
	}
	m_current_position_index = index;

	m_page_control = elm_index_add(m_navi_bar);
	if (!m_page_control) {
		BROWSER_LOGD("elm_page_control_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_page_control, "pagecontrol");
	elm_index_horizontal_set(m_page_control, EINA_TRUE);
	elm_index_autohide_disabled_set(m_page_control, EINA_TRUE);

	memset(m_index_items, 0x00, sizeof(Elm_Object_Item *) * BROWSER_MULTI_WINDOW_MAX_COUNT);

	Elm_Object_Item *index_it = NULL;
	for (int i = 0 ; i < window_list.size() ; i++) {
		char number[5] = {0, };
		sprintf(number, "%d", i + 1);

		if (m_current_position_index == i)
			m_index_items[i] = index_it = elm_index_item_append(m_page_control, number, __index_selected_cb, this);
		else
			m_index_items[i] = elm_index_item_append(m_page_control, number, __index_selected_cb, this);
	}

	elm_index_level_go(m_page_control, m_current_position_index);
	elm_index_item_selected_set(index_it, EINA_TRUE);

	elm_object_part_content_set(m_main_layout, "elm.swallow.page_control", m_page_control);
	evas_object_show(m_page_control);

	if (BROWSER_MULTI_WINDOW_MAX_COUNT <= window_list.size())
		elm_object_item_disabled_set(m_new_window_button, EINA_TRUE);

	_set_multi_window_mode(EINA_TRUE);

#if defined(HORIZONTAL_UI)
	if (is_landscape()) {
		/* Landscape mode */
		if (browser_view->m_focused_window->m_landscape_snapshot_image)
			evas_object_del(browser_view->m_focused_window->m_landscape_snapshot_image);
		browser_view->m_focused_window->m_landscape_snapshot_image = snapshot_image;

		for (int i = 0 ; i < window_list.size() ; i++) {
			/* Focused window is already captured above. So skip it. */
			if ((window_list[i] != browser_view->m_focused_window)
			     && !(window_list[i]->m_landscape_snapshot_image)) {
				Evas_Object *snapshot_image = NULL;
				if (!window_list[i]->m_ewk_view || browser_view->get_url(window_list[i]).empty()) {
					snapshot_image = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
					if (!snapshot_image) {
						BROWSER_LOGE("evas_object_rectangle_add failed");
						return EINA_FALSE;
					}
					int focused_ewk_view_w = 0;
					int focused_ewk_view_h = 0;
					evas_object_geometry_get(browser_view->m_focused_window->m_ewk_view, NULL, NULL,
									&focused_ewk_view_w, &focused_ewk_view_h);

					evas_object_size_hint_min_set(snapshot_image, (int)(focused_ewk_view_w * BROWSER_MULTI_WINDOW_ITEM_RATIO),
										(int)(focused_ewk_view_h * BROWSER_MULTI_WINDOW_ITEM_RATIO));
					evas_object_resize(snapshot_image, (int)(focused_ewk_view_w * BROWSER_MULTI_WINDOW_ITEM_RATIO),
									(int)(focused_ewk_view_h * BROWSER_MULTI_WINDOW_ITEM_RATIO));
					evas_object_color_set(snapshot_image, 255, 255, 255, 255);
				} else {
					snapshot_image = _capture_snapshot(window_list[i],
										BROWSER_MULTI_WINDOW_ITEM_RATIO);
					if (!snapshot_image) {
						BROWSER_LOGD("_capture_snapshot failed");
						return EINA_FALSE;
					}
				}
				window_list[i]->m_landscape_snapshot_image = snapshot_image;
			}
		}
	}
	else
#endif
	{
		/* Portrait mode */
		if (browser_view->m_focused_window->m_portrait_snapshot_image)
			evas_object_del(browser_view->m_focused_window->m_portrait_snapshot_image);
		browser_view->m_focused_window->m_portrait_snapshot_image = snapshot_image;

		for (int i = 0 ; i < window_list.size() ; i++) {
			/* Focused window is already captured above. So skip it. */
			if ((window_list[i] != browser_view->m_focused_window)
			     && !(window_list[i]->m_portrait_snapshot_image)) {
				Evas_Object *snapshot_image = NULL;
				if (window_list[i]->m_ewk_view == NULL || browser_view->get_url(window_list[i]).empty()) {
					snapshot_image = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
					if (!snapshot_image) {
						BROWSER_LOGE("evas_object_rectangle_add failed");
						return EINA_FALSE;
					}
					int focused_ewk_view_w = 0;
					int focused_ewk_view_h = 0;
					evas_object_geometry_get(browser_view->m_focused_window->m_ewk_view, NULL, NULL,
									&focused_ewk_view_w, &focused_ewk_view_h);

					evas_object_size_hint_min_set(snapshot_image, (int)(focused_ewk_view_w * BROWSER_MULTI_WINDOW_ITEM_RATIO),
										(int)(focused_ewk_view_h * BROWSER_MULTI_WINDOW_ITEM_RATIO));
					evas_object_resize(snapshot_image, (int)(focused_ewk_view_w * BROWSER_MULTI_WINDOW_ITEM_RATIO),
									(int)(focused_ewk_view_h * BROWSER_MULTI_WINDOW_ITEM_RATIO));
					evas_object_color_set(snapshot_image, 255, 255, 255, 255);
				} else {
					snapshot_image = _capture_snapshot(window_list[i],
										BROWSER_MULTI_WINDOW_ITEM_RATIO);
					if (!snapshot_image) {
						BROWSER_LOGD("_capture_snapshot failed");
						return EINA_FALSE;
					}
				}
				window_list[i]->m_portrait_snapshot_image = snapshot_image;
			}
		}
	}

	if (!_fill_multi_window_items()) {
		BROWSER_LOGD("_fill_multi_window_items failed");
		return EINA_FALSE;
	}

	edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout),
						"show,multi_window,signal", "");

	/* Give zoom out effect in idler because the effect is not smooth. */
	m_zoom_out_effect_idler = ecore_idler_add(__zoom_out_effect_idler_cb, this);

	return EINA_TRUE;
}

