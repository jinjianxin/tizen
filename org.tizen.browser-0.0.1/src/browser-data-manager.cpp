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
 *数据操作类
 */

/**
  *@file browser-data-manager.cpp
  *@brief 管理数据操作数据实现浏览器数据的管理
  */

#include "browser-add-to-bookmark-view.h"
#if defined(FEATURE_MOST_VISITED_SITES)
#include "add-to-most-visited-sites-view.h"
#endif
#include "browser-geolocation-db.h"
#include "browser-bookmark-db.h"
#include "browser-bookmark-view.h"
#include "browser-config.h"
#include "browser-data-manager.h"
#include "browser-history-db.h"
#include "browser-history-layout.h"
#include "browser-multi-window-view.h"
#include "browser-new-folder-view.h"
#include "browser-select-folder-view.h"
#if defined(FEATURE_MOST_VISITED_SITES)
#include "most-visited-sites.h"
#endif
#include "browser-view.h"

Browser_Data_Manager::Browser_Data_Manager(void)
:
	m_browser_view(NULL)
	,m_bookmark_view(NULL)
	,m_add_to_bookmark_view(NULL)
	,m_edit_bookmark_view(NULL)
#if defined(FEATURE_MOST_VISITED_SITES)
	,m_add_to_most_visited_sites_view(NULL)
#endif
	,m_new_folder_view(NULL)
	,m_select_folder_view(NULL)
	,m_history_layout(NULL)
	,m_multi_window_view(NULL)
	,m_bookmark_db(NULL)
	,m_history_db(NULL)
	,m_geolocation_db(NULL)
	,m_stack_status(0)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Data_Manager::~Browser_Data_Manager(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_bookmark_view) {
		delete m_bookmark_view;
		m_bookmark_view = NULL;
	}
	if (m_add_to_bookmark_view) {
		delete m_add_to_bookmark_view;
		m_add_to_bookmark_view = NULL;
	}
	if (m_edit_bookmark_view) {
		delete m_edit_bookmark_view;
		m_edit_bookmark_view = NULL;
	}
#if defined(FEATURE_MOST_VISITED_SITES)
	if (m_add_to_most_visited_sites_view) {
		delete m_add_to_most_visited_sites_view;
		m_add_to_most_visited_sites_view = NULL;
	}
#endif
	if (m_new_folder_view) {
		delete m_new_folder_view;
		m_new_folder_view = NULL;
	}
	if (m_select_folder_view) {
		delete m_select_folder_view;
		m_select_folder_view = NULL;
	}
	if (m_history_layout) {
		delete m_history_layout;
		m_history_layout = NULL;
	}
	if (m_bookmark_db) {
		delete m_bookmark_db;
		m_bookmark_db = NULL;
	}
	if (m_history_db) {
		delete m_history_db;
		m_history_db = NULL;
	}
	if (m_multi_window_view) {
		delete m_multi_window_view;
		m_multi_window_view = NULL;
	}
}


Eina_Bool Browser_Data_Manager::is_in_view_stack(view_stack_status view)
{
	BROWSER_LOGD("m_stack_status = %d, view = %d, m_stack_status & view = %d",
		m_stack_status, view, m_stack_status & view);

	if (m_stack_status & view)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

void Browser_Data_Manager::set_browser_view(Browser_View *browser_view)
{
	BROWSER_LOGD("[%s]", __func__);
	m_browser_view = browser_view;
	m_stack_status = m_stack_status | BR_BROWSER_VIEW;
}

Browser_Bookmark_View *Browser_Data_Manager::create_bookmark_view(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_bookmark_view)
		return NULL;

	m_bookmark_view = new(nothrow) Browser_Bookmark_View;
	if (!m_bookmark_view)
		BROWSER_LOGE("new Browser_Bookmark_View failed");
	else
		m_stack_status = m_stack_status | BR_BOOKMARK_VIEW;

	return m_bookmark_view;
}

void Browser_Data_Manager::destroy_bookmark_view(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_bookmark_view) {
		delete m_bookmark_view;
		m_bookmark_view = NULL;
		m_stack_status = m_stack_status - BR_BOOKMARK_VIEW;
	}
}

Browser_Bookmark_DB *Browser_Data_Manager::create_bookmark_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_bookmark_db)
		return NULL;

	m_bookmark_db = new(nothrow) Browser_Bookmark_DB;
	if (!m_bookmark_db)
		BROWSER_LOGE("new Browser_Bookmark_DB failed");

	return m_bookmark_db;
}

void Browser_Data_Manager::destroy_bookmark_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_bookmark_db) {
		delete m_bookmark_db;
		m_bookmark_db = NULL;
	}
}

Browser_Add_To_Bookmark_View *Browser_Data_Manager::create_add_to_bookmark_view(string title,
								string url, int current_folder_id)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_add_to_bookmark_view)
		return NULL;

	m_add_to_bookmark_view = new(nothrow) Browser_Add_To_Bookmark_View(title, url,
								EINA_FALSE, current_folder_id);
	if (!m_add_to_bookmark_view)
		BROWSER_LOGE("new Browser_Add_To_Bookmark_View failed");
	else
		m_stack_status = m_stack_status | BR_ADD_TO_BOOKMARK_VIEW;

	return m_add_to_bookmark_view;
}

void Browser_Data_Manager::destroy_add_to_bookmark_view(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_add_to_bookmark_view) {
		delete m_add_to_bookmark_view;
		m_add_to_bookmark_view = NULL;
		m_stack_status = m_stack_status - BR_ADD_TO_BOOKMARK_VIEW;
	}
}

Browser_Add_To_Bookmark_View *Browser_Data_Manager::create_edit_bookmark_view(string title,
								string url, int current_folder_id)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_edit_bookmark_view)
		return NULL;

	m_edit_bookmark_view = new(nothrow) Browser_Add_To_Bookmark_View(title, url,
								EINA_TRUE, current_folder_id);
	if (!m_edit_bookmark_view)
		BROWSER_LOGE("new Browser_Add_To_Bookmark_View failed");
	else
		m_stack_status = m_stack_status | BR_EDIT_BOOKMARK_VIEW;

	return m_edit_bookmark_view;
}

void Browser_Data_Manager::destroy_edit_bookmark_view(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_edit_bookmark_view) {
		delete m_edit_bookmark_view;
		m_edit_bookmark_view = NULL;
		m_stack_status = m_stack_status - BR_EDIT_BOOKMARK_VIEW;
	}
}

#if defined(FEATURE_MOST_VISITED_SITES)
Add_To_Most_Visited_Sites_View *Browser_Data_Manager::create_add_to_most_visited_sites_view(Most_Visited_Sites *most_visited_sites)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_add_to_most_visited_sites_view)
		return NULL;

	m_add_to_most_visited_sites_view = new(nothrow) Add_To_Most_Visited_Sites_View(most_visited_sites);
	if (!m_add_to_most_visited_sites_view)
		BROWSER_LOGE("new Add_To_Most_Visited_Sites_View failed");
	else
		m_stack_status = m_stack_status | ADD_TO_MOST_VISITED_SITES_VIEW;

	return m_add_to_most_visited_sites_view;
}
#endif

#if defined(FEATURE_MOST_VISITED_SITES)
void Browser_Data_Manager::destroy_add_to_most_visited_sites_view(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_add_to_most_visited_sites_view) {
		delete m_add_to_most_visited_sites_view;
		m_add_to_most_visited_sites_view = NULL;
		m_stack_status = m_stack_status - ADD_TO_MOST_VISITED_SITES_VIEW;
	}
}
#endif

Browser_New_Folder_View *Browser_Data_Manager::create_new_folder_view(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_new_folder_view)
		return NULL;

	m_new_folder_view = new(nothrow) Browser_New_Folder_View;
	if (!m_new_folder_view)
		BROWSER_LOGE("new Browser_New_Folder_View failed");
	else
		m_stack_status = m_stack_status | BR_NEW_FOLDER_VIEW;

	return m_new_folder_view;
}

void Browser_Data_Manager::destroy_new_folder_view(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_new_folder_view) {
		delete m_new_folder_view;
		m_new_folder_view = NULL;
		m_stack_status = m_stack_status - BR_NEW_FOLDER_VIEW;
	}
}

/* 'current_folder_id' is used to mark the current folder radio button in select folder view. */
Browser_Select_Folder_View *Browser_Data_Manager::create_select_folder_view(int current_folder_id)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_select_folder_view)
		return NULL;

	m_select_folder_view = new(nothrow) Browser_Select_Folder_View(current_folder_id);
	if (!m_select_folder_view)
		BROWSER_LOGE("new Browser_Select_Folder_View failed");
	else
		m_stack_status = m_stack_status | BR_SELECT_FOLDER_VIEW;

	return m_select_folder_view;
}

void Browser_Data_Manager::destroy_select_folder_view(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_select_folder_view) {
		delete m_select_folder_view;
		m_select_folder_view = NULL;
		m_stack_status = m_stack_status - BR_SELECT_FOLDER_VIEW;
	}
}

Browser_Multi_Window_View* Browser_Data_Manager::create_multi_window_view(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_multi_window_view)
		return NULL;

	m_multi_window_view = new(nothrow) Browser_Multi_Window_View;
	if (!m_multi_window_view) {
		BROWSER_LOGE("new Browser_Multi_Window_View failed");
		return NULL;
	} else
		m_stack_status = m_stack_status | BR_MULTI_WINDOW_VIEW;

	return m_multi_window_view;
}

void Browser_Data_Manager::destroy_multi_window_view(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_multi_window_view) {
		delete m_multi_window_view;
		m_multi_window_view = NULL;
		m_stack_status = m_stack_status - BR_MULTI_WINDOW_VIEW;
	}
}

Browser_History_Layout *Browser_Data_Manager::create_history_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_history_layout)
		return NULL;

	m_history_layout = new(nothrow) Browser_History_Layout;
	if (!m_history_layout)
		BROWSER_LOGE("new Browser_History_Layout failed");

	return m_history_layout;
}

void Browser_Data_Manager::destroy_history_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_history_layout) {
		delete m_history_layout;
		m_history_layout = NULL;
	}
}

Browser_History_DB *Browser_Data_Manager::create_history_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_history_db)
		return NULL;

	m_history_db = new(nothrow) Browser_History_DB;
	if (!m_history_db)
		BROWSER_LOGE("new Browser_History_DB failed");

	return m_history_db;
}

void Browser_Data_Manager::destroy_history_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_history_db) {
		delete m_history_db;
		m_history_db = NULL;
	}
}

Browser_Geolocation_DB *Browser_Data_Manager::create_geolocation_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_geolocation_db)
		return NULL;

	m_geolocation_db = new(nothrow) Browser_Geolocation_DB;
	if (!m_geolocation_db)
		BROWSER_LOGE("new Browser_Geolocation_DB failed");

	return m_geolocation_db;
}

void Browser_Data_Manager::destroy_geolocation_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_geolocation_db) {
		delete m_geolocation_db;
		m_geolocation_db = NULL;
	}
}
