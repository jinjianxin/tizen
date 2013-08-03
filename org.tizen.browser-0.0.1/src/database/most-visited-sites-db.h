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

#ifndef BROWSER_MOST_VISITED_SITES_DB_H
#define BROWSER_MOST_VISITED_SITES_DB_H

#include "browser-config.h"

extern "C" {
#include "db-util.h"
}

#include <iostream>
#include <string>
#include <vector>

class Most_Visited_Sites_DB {
public:
	struct most_visited_sites_entry {
		int index;
		std::string title;
		std::string url;
		std::string id;
	};

	Most_Visited_Sites_DB(void);
	~Most_Visited_Sites_DB(void);

	Eina_Bool get_most_visited_sites_list(std::vector<most_visited_sites_entry> &list);
	Eina_Bool get_most_visited_list(std::vector<most_visited_sites_entry> &list,
				std::vector<most_visited_sites_entry> &duplicate_list, int count);
	Eina_Bool save_most_visited_sites_item(int index, const char *url,
						const char *title, const char *screen_shot);
	Eina_Bool delete_most_visited_sites_item(int index);
	Eina_Bool delete_most_visited_sites_item(const char *url);
	Eina_Bool delete_most_visited_sites_history_item(int history_id);
	std::string get_history_id_by_url(const char* url);
	std::string get_history_title_by_id(const char* id);
	Eina_Bool is_in_bookmark(const char* url, int *bookmark_id);
private:
	Eina_Bool _open_db(void);
	Eina_Bool _close_db(void);

	static sqlite3* m_db_descriptor;
};

#endif	/* BROWSER_SPEED_DIAL_DB_H */

