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

#ifndef BROWSER_DATA_MANAGER_H
#define BROWSER_DATA_MANAGER_H

using namespace std;

#include "browser-config.h"

typedef enum _view_stack_status {
	BR_BROWSER_VIEW = 1,
	BR_BOOKMARK_VIEW = 1 << 1,
	BR_ADD_TO_BOOKMARK_VIEW = 1 << 2,
	BR_EDIT_BOOKMARK_VIEW = 1 << 3,
	BR_NEW_FOLDER_VIEW = 1 << 4,
	BR_SELECT_FOLDER_VIEW = 1 << 5,
	BR_MULTI_WINDOW_VIEW = 1 << 6,
	ADD_TO_MOST_VISITED_SITES_VIEW = 1 << 7
} view_stack_status;

class Browser_Add_To_Bookmark_View;
class Add_To_Most_Visited_Sites_View;
class Browser_Bookmark_DB;
class Browser_Bookmark_View;
class Browser_History_DB;
class Browser_History_Layout;
class Browser_Multi_Window_View;
class Browser_New_Folder_View;
class Browser_Select_Folder_View;
class Most_Visited_Sites;
class Browser_View;
class Browser_Geolocation_DB;

class Browser_Data_Manager {
public:
	Browser_Data_Manager(void);
	~Browser_Data_Manager(void);

	/* Must be.
	  * When create some view, it should be created by create_xxx method of Browser_Data_Manager.
	  * When leave that view (using elm_navigationbar_pop), the destroy_xxx method also called.
	  * The create_xxx, destroy_xxx methods control the view stack.
	  */
	Eina_Bool is_in_view_stack(view_stack_status view);

	Browser_View *get_browser_view(void) { return m_browser_view; }
	void set_browser_view(Browser_View *browser_view);

	Browser_Bookmark_View *get_bookmark_view(void) { return m_bookmark_view; }
	Browser_Bookmark_View *create_bookmark_view(void);
	void destroy_bookmark_view(void);

	Browser_Bookmark_DB *get_bookmark_db(void) { return m_bookmark_db; }
	Browser_Bookmark_DB *create_bookmark_db(void);
	void destroy_bookmark_db(void);

	Browser_Add_To_Bookmark_View *get_add_to_bookmark_view(void) { return m_add_to_bookmark_view; }
	Browser_Add_To_Bookmark_View *create_add_to_bookmark_view(string title,
								string url, int current_folder_id = BROWSER_BOOKMARK_MAIN_FOLDER_ID);
	void destroy_add_to_bookmark_view(void);

	Browser_Add_To_Bookmark_View *get_edit_bookmark_view(void) { return m_edit_bookmark_view; }
	Browser_Add_To_Bookmark_View *create_edit_bookmark_view(string title,
								string url, int current_folder_id = BROWSER_BOOKMARK_MAIN_FOLDER_ID);
	void destroy_edit_bookmark_view(void);

#if defined(FEATURE_MOST_VISITED_SITES)
	Add_To_Most_Visited_Sites_View *get_add_to_most_visited_sites_view(void) { return m_add_to_most_visited_sites_view; }
	Add_To_Most_Visited_Sites_View *create_add_to_most_visited_sites_view(Most_Visited_Sites *most_visited_sites);
	void destroy_add_to_most_visited_sites_view(void);
#endif

	Browser_New_Folder_View *get_new_folder_view(void) { return m_new_folder_view; }
	Browser_New_Folder_View *create_new_folder_view(void);
	void destroy_new_folder_view(void);

	Browser_Select_Folder_View *get_select_folder_view(void) { return m_select_folder_view; }
	Browser_Select_Folder_View *create_select_folder_view(int current_folder_id);
	void destroy_select_folder_view(void);

	Browser_Multi_Window_View *get_multi_window_view(void) { return m_multi_window_view; }
	Browser_Multi_Window_View *create_multi_window_view(void);
	void destroy_multi_window_view(void);

	Browser_History_Layout *get_history_layout(void) { return m_history_layout; }
	Browser_History_Layout *create_history_layout(void);
	void destroy_history_layout(void);

	Browser_History_DB *get_history_db(void) { return m_history_db; }
	Browser_History_DB *create_history_db(void);
	void destroy_history_db(void);

	Browser_Geolocation_DB *get_geolocation_db(void) { return m_geolocation_db; }
	Browser_Geolocation_DB *create_geolocation_db(void);
	void destroy_geolocation_db(void);

private:
	Browser_View *m_browser_view;
	Browser_Bookmark_View *m_bookmark_view;
	Browser_Add_To_Bookmark_View *m_add_to_bookmark_view;
#if defined(FEATURE_MOST_VISITED_SITES)
	Add_To_Most_Visited_Sites_View *m_add_to_most_visited_sites_view;
#endif
	Browser_Add_To_Bookmark_View *m_edit_bookmark_view;
	Browser_New_Folder_View *m_new_folder_view;
	Browser_Select_Folder_View *m_select_folder_view;
	Browser_History_Layout *m_history_layout;
	Browser_Multi_Window_View *m_multi_window_view;

	Browser_Bookmark_DB *m_bookmark_db;
	Browser_History_DB *m_history_db;
	Browser_Geolocation_DB *m_geolocation_db;

	unsigned int m_stack_status;
};

#endif /* BROWSER_DATA_MANAGER_H */

