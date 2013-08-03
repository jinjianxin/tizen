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


#ifndef BROWSER_GEOLOCATION_DB_H
#define BROWSER_GEOLOCATION_DB_H

#include "browser-config.h"

extern "C" {
#include "db-util.h"
}

#include <iostream>
#include <string>
#include <vector>

class Browser_Geolocation_DB {
public:
	struct geolocation_info {
		std::string url;
		Eina_Bool geolocation;
		Eina_Bool allow;
		Eina_Bool storage;
		void *origin;
		void *user_data;
	};

	Browser_Geolocation_DB();
	~Browser_Geolocation_DB();

	Eina_Bool save_geolocation_host(const char *address, bool accept);
	Eina_Bool get_geolocation_host(const char *address, bool &accept);
	Eina_Bool remove_geolocation_data(const char *address);
	Eina_Bool remove_all_geolocation_data(void);

	int get_geolocation_host_count(const char *address);
	Eina_Bool get_geolocation_info_list(std::vector<geolocation_info *> &list);
	int get_geolocation_info_count(void);
private:
	Eina_Bool _open_db(void);
	Eina_Bool _close_db(void);

	static sqlite3* m_db_descriptor;
};

#endif	/* BROWSER_GEOLOCATION_DB_H */

