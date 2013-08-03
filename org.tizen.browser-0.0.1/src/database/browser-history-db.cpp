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
  *@file browser-history-db.cpp
  *@brief 浏览器浏览历史数据库
  */

using namespace std;

#include "browser-history-db.h"

sqlite3* Browser_History_DB::m_db_descriptor = NULL;

Browser_History_DB::Browser_History_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_History_DB::~Browser_History_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Eina_Bool Browser_History_DB::_open_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int error = db_util_open(BROWSER_HISTORY_DB_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool Browser_History_DB::_close_db(void)
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

Eina_Bool Browser_History_DB::save_history(const char *url, const char *title, Eina_Bool *is_full)
{
	if (url == NULL || title == NULL || strlen(url) == 0 || strlen(title) == 0)
		return EINA_FALSE;

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select counter from history where address=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	error = sqlite3_bind_text(sqlite3_stmt, 1, url, -1, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (error == SQLITE_ROW) {
		int count = sqlite3_column_int(sqlite3_stmt, 0);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			_close_db();
			return EINA_FALSE;
		}

		error = sqlite3_prepare_v2(m_db_descriptor, "update history set counter=?, title=?, visitdate=DATETIME('now') where address=?",
						-1, &sqlite3_stmt, NULL);
		if (error != SQLITE_OK) {
			BROWSER_LOGD("SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		if (sqlite3_bind_int(sqlite3_stmt, 1, count + 1) != SQLITE_OK) {
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		if (sqlite3_bind_text(sqlite3_stmt, 2, title, -1, NULL) != SQLITE_OK) {
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		if (sqlite3_bind_text(sqlite3_stmt, 3, url, -1, NULL) != SQLITE_OK) {
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		error = sqlite3_step(sqlite3_stmt);
	} else {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			_close_db();
			return EINA_FALSE;
		}

		error = sqlite3_prepare_v2(m_db_descriptor, "select count(*) from history", -1, &sqlite3_stmt, NULL);
		if (error != SQLITE_OK) {
			BROWSER_LOGD("SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		error = sqlite3_step(sqlite3_stmt);
		if (error != SQLITE_ROW) {
			BROWSER_LOGD("SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		unsigned history_count = sqlite3_column_int(sqlite3_stmt, 0);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			_close_db();
			return EINA_FALSE;
		}

		if (history_count >= BROWSER_HISTORY_COUNT_LIMIT) {
			string delete_query = "delete from history where id = (select min(id) from history)";
			if (history_count > BROWSER_HISTORY_COUNT_LIMIT) {
				unsigned delete_count = history_count - BROWSER_HISTORY_COUNT_LIMIT + 1;
				char text[128] = {0, };
				sprintf(text, "delete from history where id in (select id from history limit %d)", delete_count);
				delete_query = text;
			}

			error = sqlite3_prepare_v2(m_db_descriptor, delete_query.c_str(), -1, &sqlite3_stmt, NULL);
			if (error != SQLITE_OK) {
				BROWSER_LOGD("SQL error=%d", error);
				if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
					BROWSER_LOGE("sqlite3_finalize is failed.\n");
				_close_db();
				return EINA_FALSE;
			}

			error = sqlite3_step(sqlite3_stmt);
			if (error != SQLITE_DONE) {
				BROWSER_LOGD("SQL error=%d", error);
				if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
					BROWSER_LOGE("sqlite3_finalize is failed.\n");
				_close_db();
				return EINA_FALSE;
			}
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
				_close_db();
				return EINA_FALSE;
			}
		}

		const string insert_url(url);
		const string insert_title(title);

		const string statement = "insert into history (address, title, counter, visitdate) values('" + insert_url 
						+ "','" + insert_title + "', 0, DATETIME('now'))";
		error = sqlite3_prepare_v2(m_db_descriptor, statement.c_str(), -1, &sqlite3_stmt, NULL);
		if (error != SQLITE_OK) {
			BROWSER_LOGD("SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		error = sqlite3_step(sqlite3_stmt);

		if (error == SQLITE_FULL)
			*is_full = EINA_TRUE;
		else
			*is_full = EINA_FALSE;
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		_close_db();
		return EINA_FALSE;
	}

	_close_db();

	return (error == SQLITE_DONE);
}

Eina_Bool Browser_History_DB::delete_history(const char *url)
{
	if (!url || strlen(url) == 0) {
		BROWSER_LOGE("url is empty");
		return EINA_FALSE;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "delete from history where address=?",
								-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_text(sqlite3_stmt, 1, url, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.");

	error = sqlite3_step(sqlite3_stmt);
	if (error != SQLITE_OK && error != SQLITE_DONE) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	return EINA_TRUE;
}

Eina_Bool Browser_History_DB::delete_history(int history_id)
{
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "delete from history where id=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_int(sqlite3_stmt, 1, history_id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	error = sqlite3_step(sqlite3_stmt);
	if (error != SQLITE_OK && error != SQLITE_DONE) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	return EINA_TRUE;
}

Eina_Bool Browser_History_DB::get_most_visited_list(vector<most_visited_item> &list)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;

	int error = sqlite3_prepare_v2(m_db_descriptor, "select address, title from history order by counter desc limit "BROWSER_MOST_VISITED_COUNT_TEXT,
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}


	most_visited_item item;
	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		item.url = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0));
		item.title = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 1));

		list.push_back(item);
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	return (error == SQLITE_DONE || error == SQLITE_ROW);
}

Eina_Bool Browser_History_DB::get_history_list_by_partial_url(const char *url, int count, std::vector<std::string> &list)
{
	if (!url || strlen(url) == 0) {
		BROWSER_LOGE("partial_url is empty");
		return EINA_FALSE;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	std::string query = "select address, counter from history where address like '%" + std::string(url) + "%'";

	int error = sqlite3_prepare_v2(m_db_descriptor, query.c_str(), -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	int i;
	for (i = 0; i < count; i++) {
		error = sqlite3_step(sqlite3_stmt);
		if (error == SQLITE_ROW) {
			std::string mark_up = std::string(reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0)));
			size_t index = mark_up.find(BROWSER_HTTP_SCHEME);
			if(index == 0) {
				index = mark_up.find(url, strlen(BROWSER_HTTP_SCHEME));
				if(index == -1)
					continue;
			}

			index = mark_up.find(BROWSER_HTTPS_SCHEME);
			if(index == 0) {
				index = mark_up.find(url, strlen(BROWSER_HTTPS_SCHEME));
				if(index == -1)
					continue;
			}

			list.push_back(std::string(reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0))));
		} else
			break;
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	if (error == SQLITE_DONE || error == SQLITE_ROW)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Eina_Bool Browser_History_DB::get_history_list(vector<history_item*> &list)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select id, address, title, visitdate from history order by visitdate desc",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	history_item *item;
	while(1) {
		error = sqlite3_step(sqlite3_stmt);
		if (error == SQLITE_ROW) {
			item = new(nothrow) history_item;
			if (!item) {
				BROWSER_LOGD("new history_item is failed");
				return EINA_FALSE;
			}
			const char* id = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0));
			if (id && strlen(id))
				item->id = atoi(id);
			else
				item->id = 0;

			const char* url = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 1));
			if (url && strlen(url))
				item->url = url;
			else
				item->url = "";
			
			const char* title = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 2));
			if (title && strlen(title))
				item->title = title;
			else
				item->title = "";

			const char* date = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 3));
			if (date && strlen(date))
				item->date = date;
			else
				item->date = "";

			item->user_data = NULL;
			item->is_delete = EINA_FALSE;

			list.push_back(item);
		} else
			break;
	}

	BROWSER_LOGD("SQL error: %d", error);\
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	_close_db();
	if (error == SQLITE_DONE || error == SQLITE_ROW)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Eina_Bool Browser_History_DB::clear_history(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "delete from history", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
	}

	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	_close_db();
	if (error == SQLITE_DONE || error == SQLITE_ROW)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Eina_Bool Browser_History_DB::is_in_bookmark(const char* url, int *bookmark_id)
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

