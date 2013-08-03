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


#include "browser-scissorbox-view.h"
#include "browser-view.h"
#include "browser-window.h"
#include "scissorbox.h"

//#define DEFAULT_SCISSORBOX_SIZE	(200 * elm_scale_get())
#define DEFAULT_SCISSORBOX_SIZE_1	172
#define DEFAULT_SCISSORBOX_SIZE_2	348
#define DEFAULT_SCISSORBOX_SIZE_3	700
#define SCISSORBOX_SNAPSHOT_PATH	BROWSER_SCREEN_SHOT_DIR"add_to_home.png"

Browser_Scissorbox_View::Browser_Scissorbox_View(Browser_View *browser_view)
:	m_browser_view(browser_view)
	,m_scissorbox(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Scissorbox_View::~Browser_Scissorbox_View(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_scissorbox) {
		evas_object_smart_callback_del(m_scissorbox, "changed", __scissorbox_changed_cb);
		evas_object_del(m_scissorbox);
	}
}

Eina_Bool Browser_Scissorbox_View::init(void)
{
	BROWSER_LOGD("[%s]", __func__);

	return _create_main_layout();
}

Eina_Bool Browser_Scissorbox_View::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *webview = m_browser_view->m_focused_window->m_ewk_view;
	m_scissorbox = br_scissorbox_add(webview);
	if (!m_scissorbox) {
		BROWSER_LOGE("br_scissorbox_add failed");
		return EINA_FALSE;
	}

	int webview_x = 0;
	int webview_y = 0;
	int webview_w = 0;
	int webview_h = 0;
	evas_object_geometry_get(webview, &webview_x, &webview_y, &webview_w, &webview_h);

	int scroller_y = 0;
	evas_object_geometry_get(m_browser_view->m_scroller, NULL, &scroller_y, NULL, NULL);

	br_scissorbox_region_set(m_scissorbox, webview_w / 4, webview_y + webview_h / 4,
						DEFAULT_SCISSORBOX_SIZE_1, DEFAULT_SCISSORBOX_SIZE_1);
	br_scissorbox_bound_set(m_scissorbox, webview_x, webview_y, webview_w, webview_h - webview_y + scroller_y);

	evas_object_smart_callback_add(m_scissorbox, "changed", __scissorbox_changed_cb, this);

	evas_object_show(m_scissorbox);

	return EINA_TRUE;
}

void Browser_Scissorbox_View::__scissorbox_changed_cb(void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
}

