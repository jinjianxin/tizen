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

/***
 *单个标签页的定义
 */

/**
  *@file browser-window.cpp
  *@brief 单个标签页的定义
  */


#include "browser-window.h"

Browser_Window::Browser_Window(void)
:
	m_ewk_view(NULL)
	,m_portrait_snapshot_image(NULL)
#if defined(HORIZONTAL_UI)
	,m_landscape_snapshot_image(NULL)
#endif
	,m_ewk_view_layout(NULL)
	,m_parent(NULL)
	,m_favicon(NULL)
	,m_option_header_favicon(NULL)
	,m_secure_icon(NULL)
	,m_option_header_secure_icon(NULL)
	,m_created_by_user(EINA_FALSE)
	,m_is_reader(EINA_FALSE)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Window::~Browser_Window(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_ewk_view)
		evas_object_del(m_ewk_view);
	if (m_portrait_snapshot_image)
		evas_object_del(m_portrait_snapshot_image);
#if defined(HORIZONTAL_UI)
	if (m_landscape_snapshot_image)
		evas_object_del(m_landscape_snapshot_image);
#endif
	if (m_ewk_view_layout)
		evas_object_del(m_ewk_view_layout);
	if (m_favicon)
		evas_object_del(m_favicon);
	if (m_option_header_favicon)
		evas_object_del(m_option_header_favicon);
}

