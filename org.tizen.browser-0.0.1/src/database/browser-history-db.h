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


#ifndef BROWSER_HISTORY_DB_H
#define BROWSER_HISTORY_DB_H

#include "browser-config.h"

extern "C" {
#include "db-util.h"
}

#include <iostream>
#include <string>
#include <vector>

class Browser_History_DB {
public:
	struct history_item {
		int id;
		std::string url;
		std::string title;
		std::string date;
		Eina_Bool is_delete;

		void *user_data;
	};
	struct most_visited_item {
		std::string url;
		std::string title;
	};

	Browser_History_DB();
	~Browser_History_DB();

	Eina_Bool get_history_list(std::vector<history_item*> &list);
	Eina_Bool get_history_list_by_partial_url(const char *url, int count, std::vector<std::string> &list);
	Eina_Bool get_most_visited_list(std::vector<most_visited_item> &list);
	Eina_Bool save_history(const char *url, const char *title, Eina_Bool *is_full);
	Eina_Bool delete_history(int history_id);
	Eina_Bool delete_history(const char *url);
	Eina_Bool clear_history(void);
	Eina_Bool is_in_bookmark(const char* url, int *bookmark_id);
private:
	Eina_Bool _open_db(void);
	Eina_Bool _close_db(void);
	
	static sqlite3* m_db_descriptor;
};

#endif	/* BROWSER_HISTORY_DB_H */

