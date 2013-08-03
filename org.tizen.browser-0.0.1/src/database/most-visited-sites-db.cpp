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
  *@file most-visited-sites-db.cpp
  *@brief 最常访问数据库
  */


using namespace std;

#include "most-visited-sites-db.h"

sqlite3* Most_Visited_Sites_DB::m_db_descriptor = NULL;

Most_Visited_Sites_DB::Most_Visited_Sites_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Most_Visited_Sites_DB::~Most_Visited_Sites_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Eina_Bool Most_Visited_Sites_DB::_open_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int error = db_util_open(BROWSER_MOST_VISITED_SITES_DB_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool Most_Visited_Sites_DB::_close_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_db_descriptor)
	{
		int error = db_util_close(m_db_descriptor);
		if (error != SQLITE_OK) {
			BROWSER_LOGE("db_util_close error");
			m_db_descriptor = NULL;
			return EINA_FALSE;
		}
		m_db_descriptor = NULL;
	}

	return EINA_TRUE;
}

Eina_Bool Most_Visited_Sites_DB::is_in_bookmark(const char* url, int *bookmark_id)
{
	int error = db_util_open(BROWSER_BOOKMARK_DB_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(m_db_descriptor, "select id from bookmarks where address=?",
									-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		return EINA_FALSE;
	}

	if (sqlite3_bind_text(sqlite3_stmt, 1, url, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");

	error = sqlite3_step(sqlite3_stmt);

	if (bookmark_id)
		*bookmark_id = sqlite3_column_int(sqlite3_stmt, 0);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	db_util_close(m_db_descriptor);

	return (error == SQLITE_ROW);
}

Eina_Bool Most_Visited_Sites_DB::delete_most_visited_sites_item(const char * url)
{
	int error = db_util_open(BROWSER_MOST_VISITED_SITES_DB_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		BROWSER_LOGD("check here");
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(m_db_descriptor, "delete from mostvisited where address=?",
								-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		return EINA_FALSE;
	}

	if (sqlite3_bind_text(sqlite3_stmt, 1, url, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.");

	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.");

	db_util_close(m_db_descriptor);

	return (error == SQLITE_OK || error == SQLITE_DONE);
}

Eina_Bool Most_Visited_Sites_DB::delete_most_visited_sites_item(int index)
{
	int error = db_util_open(BROWSER_MOST_VISITED_SITES_DB_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		BROWSER_LOGD("check here");
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(m_db_descriptor, "delete from mostvisited where id=?",
								-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		return EINA_FALSE;
	}

	if (sqlite3_bind_int(sqlite3_stmt, 1, index) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.");
	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.");

	db_util_close(m_db_descriptor);

	return (error == SQLITE_OK || error == SQLITE_DONE);
}

Eina_Bool Most_Visited_Sites_DB::delete_most_visited_sites_history_item(int history_id)
{
	int error = db_util_open(BROWSER_HISTORY_DB_PATH, &m_db_descriptor,
						DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		BROWSER_LOGE("db_util_open failed");
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(m_db_descriptor, "delete from history where id=?",
							-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		return EINA_FALSE;
	}

	if (sqlite3_bind_int(sqlite3_stmt, 1, history_id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	db_util_close(m_db_descriptor);

	return (error == SQLITE_OK || error == SQLITE_DONE);
}

Eina_Bool Most_Visited_Sites_DB::save_most_visited_sites_item(int index, const char *url,
						const char *title, const char *screen_shot)
{
	if(index > BROWSER_MOST_VISITED_SITES_ITEM_MAX) {
		BROWSER_LOGE("index is over BROWSER_MOST_VISITED_SITES_ITEM_MAX");
		return EINA_FALSE;
	}

	if (url == NULL || title == NULL) {
		BROWSER_LOGE("empty page");
		return EINA_FALSE;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select * from mostvisited where id=?",
								-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		return EINA_FALSE;
	}
	error = sqlite3_bind_int(sqlite3_stmt, 1, index);
	if (error != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(m_db_descriptor);
			return EINA_FALSE;
		}
	}

	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		return EINA_FALSE;
	}
	if (error == SQLITE_ROW) {  // already charged slot
		error = sqlite3_prepare_v2(m_db_descriptor, "update mostvisited set address=?, title=?, image=? where id=?",
											-1, &sqlite3_stmt, NULL);
		if (error != SQLITE_OK) {
			db_util_close(m_db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 1, url, -1, NULL) != SQLITE_OK) {
			db_util_close(m_db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 2, title, -1, NULL) != SQLITE_OK){
			db_util_close(m_db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 3, screen_shot, -1, NULL) != SQLITE_OK){
			db_util_close(m_db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_int(sqlite3_stmt, 4, index) != SQLITE_OK){
			db_util_close(m_db_descriptor);
			return EINA_FALSE;
		}
		error = sqlite3_step(sqlite3_stmt);
	} else {
		error = sqlite3_prepare_v2(m_db_descriptor, "insert into mostvisited (id, address, title, image) values(?,?,?,?)",
									-1, &sqlite3_stmt, NULL);
		if (error != SQLITE_OK){
			db_util_close(m_db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_int(sqlite3_stmt, 1, index) != SQLITE_OK){
			db_util_close(m_db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 2, url, -1, NULL) != SQLITE_OK){
			db_util_close(m_db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 3, title, -1, NULL) != SQLITE_OK){
			db_util_close(m_db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 4, screen_shot, -1, NULL) != SQLITE_OK){
			db_util_close(m_db_descriptor);
			return EINA_FALSE;
		}
		error = sqlite3_step(sqlite3_stmt);
	}
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		return EINA_FALSE;
	}

	db_util_close(m_db_descriptor);

	return (error == SQLITE_DONE);
}

std::string Most_Visited_Sites_DB::get_history_title_by_id(const char* id)
{
	std::string title;
	int error = db_util_open(BROWSER_HISTORY_DB_PATH, &m_db_descriptor,
						DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return title;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(m_db_descriptor, "select title from history where id=?",
								-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		return title;
	}

	if (sqlite3_bind_text(sqlite3_stmt, 1, id, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");

	error = sqlite3_step(sqlite3_stmt);
	if (error == SQLITE_ROW)
		title = std::string(reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0)));

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	db_util_close(m_db_descriptor);

	return title;
}

std::string Most_Visited_Sites_DB::get_history_id_by_url(const char* url)
{
	std::string history_id;
	history_id.clear();

	int error = db_util_open(BROWSER_HISTORY_DB_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return history_id;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(m_db_descriptor, "select id from history where address=?",
								-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		return history_id;
	}

	if (sqlite3_bind_text(sqlite3_stmt, 1, url, -1, NULL ) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");

	error = sqlite3_step(sqlite3_stmt);
	if (error == SQLITE_ROW)
		history_id = std::string(reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0)));

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	db_util_close(m_db_descriptor);
	return history_id;
}

Eina_Bool Most_Visited_Sites_DB::get_most_visited_list(std::vector<most_visited_sites_entry> &list,
							std::vector<most_visited_sites_entry> &duplicate_list, int count)
{
	int error = db_util_open(BROWSER_HISTORY_DB_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	if (duplicate_list.size() == 0)
		error = sqlite3_prepare_v2(m_db_descriptor, "select address, title, id from history order by counter desc limit ?",
						-1, &sqlite3_stmt, NULL);
	else {
		string query = "select address, title, id from history";
		Eina_Bool is_attach_where = EINA_FALSE;
		int default_prefix_length = strlen(DEFAULT_ICON_PREFIX);
		for (int i = 0; i < duplicate_list.size(); i++) {
			int size = (duplicate_list[i].id.length() < default_prefix_length)
					? duplicate_list[i].url.length() : default_prefix_length;
			if (memcmp(duplicate_list[i].id.c_str(), DEFAULT_ICON_PREFIX, size) == 0)
				continue;
			if (is_attach_where) {
				query.append(" and ");
			}
			if (!is_attach_where) {
				query.append(" where ");
				is_attach_where = true;
			}
			query.append("id != ");
			query.append(duplicate_list[i].id);
		}
		query.append(" order by counter desc limit ?");
		error = sqlite3_prepare_v2(m_db_descriptor, query.c_str(), -1, &sqlite3_stmt, NULL);
	}
	if (error != SQLITE_OK) {
		_close_db();
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 1, count) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_bind_int is failed.\n");

	most_visited_sites_entry item;
	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		item.url = reinterpret_cast<const char *> (sqlite3_column_text(sqlite3_stmt, 0));
		item.title = reinterpret_cast<const char *> (sqlite3_column_text(sqlite3_stmt, 1));
		item.id = reinterpret_cast<const char *> (sqlite3_column_text(sqlite3_stmt, 2));

		Eina_Bool is_duplicated = EINA_FALSE;
		for (int i = 0 ; i < duplicate_list.size() ; i++) {
			if (!strcmp(item.url.c_str(), duplicate_list[i].url.c_str())) {
				is_duplicated = EINA_TRUE;
				break;
			}
		}

		if (!is_duplicated)
			list.push_back(item);
	}
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	return (error == SQLITE_DONE);
}

Eina_Bool Most_Visited_Sites_DB::get_most_visited_sites_list(std::vector<most_visited_sites_entry> &list)
{
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select id, address, title, image from mostvisited order by id asc",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	most_visited_sites_entry item;
	int count = 0;
	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		item.index = sqlite3_column_int(sqlite3_stmt, 0);
		item.url = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 1));
		item.title = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 2));
		item.id = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt,3));

		Eina_Bool is_duplicated = EINA_FALSE;
		for (int i = 0 ; i < list.size() ; i++) {
			if (!strcmp(item.url.c_str(), list[i].url.c_str())) {
				is_duplicated = EINA_TRUE;
				break;
			}
		}

		if (!is_duplicated) {
			list.push_back(item);
			count++;
			if (count >= BROWSER_MOST_VISITED_SITES_ITEM_MAX)
				break;
		}
	}
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	return (error == SQLITE_DONE || error == SQLITE_ROW);
}

