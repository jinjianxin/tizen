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
  *@file browser-bookmark-db.cpp
  *@brief 浏览器bookmark数据库
  */

using namespace std;

#include "browser-bookmark-db.h"

#define BOOKMARK_NORMAL	0
#define BOOKMARK_FOLDER 1

sqlite3* Browser_Bookmark_DB::m_db_descriptor = NULL;

Browser_Bookmark_DB::Browser_Bookmark_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Bookmark_DB::~Browser_Bookmark_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Eina_Bool Browser_Bookmark_DB::_open_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int error = db_util_open(BROWSER_BOOKMARK_DB_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool Browser_Bookmark_DB::_close_db(void)
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

Eina_Bool Browser_Bookmark_DB::_get_last_index(int folder_id, int *last_index)
{
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	std::string query = "select sequence from bookmarks where parent=? order by sequence desc";

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, query.c_str(), -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_int(sqlite3_stmt, 1, folder_id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");

	if ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW || error == SQLITE_DONE)
		*last_index = sqlite3_column_int(sqlite3_stmt, 0);
	else
		BROWSER_LOGE("sqlite3_step is failed. error=%d\n", error);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	if (_close_db() == EINA_FALSE)
		BROWSER_LOGE("_close_db is failed.");

	return (error == SQLITE_DONE || error == SQLITE_ROW);
}

Eina_Bool Browser_Bookmark_DB::delete_bookmark(int bookmark_id)
{
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "delete from bookmarks where id=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_int(sqlite3_stmt, 1, bookmark_id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");

	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	if (error != SQLITE_OK && error != SQLITE_DONE) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (_close_db() == EINA_FALSE)
		BROWSER_LOGE("_close_db is failed.");

	return (error == SQLITE_DONE);
}

Eina_Bool Browser_Bookmark_DB::delete_folder(int folder_id)
{
	delete_bookmark(folder_id);

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select id, type from bookmarks where parent=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_int(sqlite3_stmt, 1, folder_id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");

	std::vector<bookmark_item_type> item_type;
	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		bookmark_item_type type;
		type.id = sqlite3_column_int(sqlite3_stmt, 0);
		type.is_folder = (sqlite3_column_int(sqlite3_stmt, 1) == BOOKMARK_FOLDER);
		item_type.push_back(type);
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	for (std::vector<bookmark_item_type>::iterator iter = item_type.begin()
		; iter != item_type.end() ; iter++) {
		if(iter->is_folder)
			delete_folder(iter->id);
		else
			delete_bookmark(iter->id);
	}

	return EINA_TRUE;
}

Eina_Bool Browser_Bookmark_DB::get_bookmark_id_by_title_url(int folder_id, const char *title,
								const char *url, int *bookmark_id)
{
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select id from bookmarks where parent=? and title=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}
 
	if (sqlite3_bind_int(sqlite3_stmt, 1, folder_id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	if (sqlite3_bind_text(sqlite3_stmt, 2, title, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");

	error = sqlite3_step(sqlite3_stmt);
	if (error != SQLITE_ROW) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	*bookmark_id = sqlite3_column_int(sqlite3_stmt, 0);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

   	_close_db();

	return (error == SQLITE_ROW);
}

Eina_Bool Browser_Bookmark_DB::save_folder(const char* folder_name)
{
	int last_index = 0;
	Eina_Bool get_last_index_result = EINA_TRUE;
	if (!_get_last_index(BROWSER_BOOKMARK_MAIN_FOLDER_ID, &last_index)) {
		BROWSER_LOGE("_get_last_index failed.\n");
		get_last_index_result = EINA_FALSE;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "insert into bookmarks (type, parent, title, creationdate, sequence, updatedate, editable) values (?, ?, ?, DATETIME('now'), ?, DATETIME('now'), 1)",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_int(sqlite3_stmt, 1, 1) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	if (sqlite3_bind_int(sqlite3_stmt, 2, BROWSER_BOOKMARK_MAIN_FOLDER_ID) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	if (sqlite3_bind_text(sqlite3_stmt, 3, folder_name, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");

	if (get_last_index_result) {
		if (sqlite3_bind_int(sqlite3_stmt, 4, last_index + 1) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	} else {
		if (sqlite3_bind_int(sqlite3_stmt, 4, 1) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	}

	error = sqlite3_step(sqlite3_stmt);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	return (error == SQLITE_DONE);
}

Eina_Bool Browser_Bookmark_DB::save_bookmark(int folder_id, const char* title, const char* url)
{
	if (!title || !url)
		return EINA_FALSE;

	int last_index = 0;
	Eina_Bool get_last_index_result = EINA_TRUE;
	if (!_get_last_index(folder_id, &last_index)) {
		BROWSER_LOGE("_get_last_index failed.\n");
		get_last_index_result = EINA_FALSE;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "insert into bookmarks (type, parent, address, title, creationdate, sequence, updatedate, editable) values (?, ?, ?, ?, DATETIME('now'), ?, DATETIME('now'), 1)",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	std::string title_to_save = "";
	std::string url_to_save = "";

	title_to_save = title;
	if (title_to_save.size() >= BROWSER_MAX_TITLE_LEN)
		title_to_save.resize(BROWSER_MAX_TITLE_LEN - 1);
	url_to_save = url;
	if (url_to_save.size() >= BROWSER_MAX_URL_LEN)
		url_to_save.resize(BROWSER_MAX_URL_LEN - 1);

	if (sqlite3_bind_int(sqlite3_stmt, 1, 0) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	if (sqlite3_bind_int(sqlite3_stmt, 2, folder_id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	if (sqlite3_bind_text(sqlite3_stmt, 3, url_to_save.c_str(), -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");
	if (sqlite3_bind_text(sqlite3_stmt, 4, title_to_save.c_str(), -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");

	if (get_last_index_result) {
		if (sqlite3_bind_int(sqlite3_stmt, 5, last_index + 1) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	}
	else {
		if (sqlite3_bind_int(sqlite3_stmt, 5, 1) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	}

  	error = sqlite3_step(sqlite3_stmt);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	if (_close_db() == EINA_FALSE)
		BROWSER_LOGE("_close_db is failed.");

	return (error == SQLITE_DONE);
}

Eina_Bool Browser_Bookmark_DB::is_full(void)
{
	unsigned count = 0;

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select count(*) from bookmarks", -1, &sqlite3_stmt, NULL);
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

	count = sqlite3_column_int(sqlite3_stmt, 0);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (count >= BROWSER_BOOKMARK_COUNT_LIMIT)
		return EINA_TRUE;

	return EINA_FALSE;
}

Eina_Bool Browser_Bookmark_DB::is_duplicated(int folder_id, const char* title, const char* url)
{
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select id from bookmarks where parent=? and title=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_int(sqlite3_stmt, 1, folder_id) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");
		return EINA_FALSE;
	}

	if (sqlite3_bind_text(sqlite3_stmt, 2, title, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");
		return EINA_FALSE;
	}

	error = sqlite3_step(sqlite3_stmt);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	if (_close_db() == EINA_FALSE)
		BROWSER_LOGE("_close_db is failed.");
		
	if (error == SQLITE_ROW)
     		return EINA_TRUE;
   
   	return EINA_FALSE;
}

Eina_Bool Browser_Bookmark_DB::is_duplicated(const char *folder_name)
{
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2( m_db_descriptor, "select id from bookmarks where type=1 and parent=1 and title=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}


	if (sqlite3_bind_text(sqlite3_stmt, 1, folder_name, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");

	error = sqlite3_step(sqlite3_stmt);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	return (error == SQLITE_ROW);
}

std::string Browser_Bookmark_DB::get_folder_name_by_id(const int folder_id)
{
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	std::string folder_name = "";

	int error = sqlite3_prepare_v2(m_db_descriptor, "select title from bookmarks where type=1 and id=?",
					-1, &sqlite3_stmt, NULL);

	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return folder_name;
	}
 
	if (sqlite3_bind_int(sqlite3_stmt, 1, folder_id) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");
		return folder_name;
	}

	error = sqlite3_step(sqlite3_stmt);

	if (error == SQLITE_ROW)
		folder_name = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt,0));
   
   	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	if (_close_db() == EINA_FALSE)
		BROWSER_LOGE("_close_db is failed.");

	return folder_name;
}

Eina_Bool Browser_Bookmark_DB::get_folder_id(int current_folder_id, const char *folder_name, int *folder_id)
{
	if (folder_name == NULL || strlen(folder_name) <= 0) {
		BROWSER_LOGE("folder name is invalid.\n");
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select id from bookmarks where type=1 and parent=? and title=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}


	// binding values
	if (sqlite3_bind_int(sqlite3_stmt, 1, current_folder_id) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 2, folder_name, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");
		return EINA_FALSE;
	}

	// checking if a bookmark exists
	error = sqlite3_step(sqlite3_stmt);

	*folder_id = BROWSER_BOOKMARK_MAIN_FOLDER_ID; //just in case

	if (error == SQLITE_ROW || error == SQLITE_DONE)
		*folder_id = sqlite3_column_int(sqlite3_stmt, 0);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	if (_close_db() == EINA_FALSE) {
		BROWSER_LOGE("_close_db is failed.");
		return EINA_FALSE;
	}

	return (error == SQLITE_DONE || error == SQLITE_ROW);
}

Eina_Bool Browser_Bookmark_DB::modify_bookmark(int bookmark_id, int folder_id, const char *title, const char *url)
{
	BROWSER_LOGD("[%s]", __func__);

	int last_index = 0;
	Eina_Bool get_last_index_result = EINA_TRUE;
	if (!_get_last_index(folder_id, &last_index)) {
		BROWSER_LOGE("_get_last_index failed.\n");
		get_last_index_result = EINA_FALSE;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "update bookmarks set title=?, address=?, parent=?, updatedate=DATETIME('now'), sequence=? where id=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

   	if (sqlite3_bind_text(sqlite3_stmt, 1, title, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");
	if (sqlite3_bind_text(sqlite3_stmt, 2, url, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");
   	if (sqlite3_bind_int(sqlite3_stmt, 3, folder_id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");

	if (get_last_index_result) {
		if (sqlite3_bind_int(sqlite3_stmt, 4, last_index + 1) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	} else {
		if (sqlite3_bind_int(sqlite3_stmt, 4, 1) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	}

	if (sqlite3_bind_int(sqlite3_stmt, 5, bookmark_id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");

	error = sqlite3_step(sqlite3_stmt);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	return (error == SQLITE_DONE);
}

Eina_Bool Browser_Bookmark_DB::modify_bookmark_title(int bookmark_id, const char *title)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "update bookmarks set title=?, updatedate=DATETIME('now') where id=?",
				-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 1, title, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");

	if (sqlite3_bind_int(sqlite3_stmt, 2, bookmark_id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");

	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	if (_close_db() == EINA_FALSE) {
		BROWSER_LOGE("_close_db is failed.");
		return EINA_FALSE;
	}

	return (error == SQLITE_DONE);
}

Eina_Bool Browser_Bookmark_DB::modify_bookmark_order_index(int bookmark_id, int folder_id, int order_index)
{
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "update bookmarks set sequence=? where parent=? and id=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_int(sqlite3_stmt, 1, order_index) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");

	if (sqlite3_bind_int(sqlite3_stmt, 2, folder_id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");

	if (sqlite3_bind_int(sqlite3_stmt, 3, bookmark_id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");

	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	return (error == SQLITE_DONE);
}

Eina_Bool Browser_Bookmark_DB::get_untitled_folder_count(int *count)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	std::string statement = "select title from bookmarks where type=1 and length(title)>8 and title like '%Folder_%' order by length(title), title";
	int error = sqlite3_prepare_v2(m_db_descriptor, statement.c_str(), -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	std::string title;
	std::string title_from_db;
	char buf[32] = {0, };
	for (int i = 1; ; i++) {
		*count = i;
		error = sqlite3_step(sqlite3_stmt);
		if (error == SQLITE_ROW) {
			snprintf(buf, sizeof(buf)-1, "Folder_%02d", i);
			title = buf;
			title_from_db = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt,0));
			if (title_from_db != title)
				break;
		} else
			break;
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	return (error == SQLITE_DONE || error == SQLITE_ROW);
}

Browser_Bookmark_DB::bookmark_item *Browser_Bookmark_DB::get_bookmark_item_by_id(int bookmark_id)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE) {
		BROWSER_LOGE("open failed");
		return NULL;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select type,parent,address,title,sequence,editable from bookmarks where id=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return NULL;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 1, bookmark_id) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");
		return NULL;
	}

	bookmark_item *item = NULL;
	if ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		item = new(nothrow) bookmark_item;
		if (!item) {
			BROWSER_LOGE("bookmark_item new is failed.\n");
			return NULL;
		}

		item->id = bookmark_id;
		if (sqlite3_column_int(sqlite3_stmt, 0) == BOOKMARK_NORMAL)
			item->is_folder = EINA_FALSE;
		else
			item->is_folder = EINA_TRUE;

		if (!item->is_folder) {
			const char* url = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 2));
			if (url && strlen(url) > 0)
				item->url = url;
			else
				item->url = "";
		}

		item->parent = sqlite3_column_int(sqlite3_stmt, 1);

		const char* title = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 3));
		if (title && strlen(title) > 0)
			item->title = title;
		else
			item->title = "";

		item->order_index = sqlite3_column_int(sqlite3_stmt, 4);
		item->is_editable = EINA_TRUE;//sqlite3_column_int(sqlite3_stmt, 5);

		item->is_delete = EINA_FALSE;
		item->user_data_1 = NULL;
		item->user_data_2 = NULL;
		item->user_data_3 = 0;
	}

	error = sqlite3_finalize(sqlite3_stmt);
	if (error != SQLITE_OK)
	{
		BROWSER_LOGE("sqlite3_finalize is failed. (%d)", error);
		return NULL;
	}

	if (_close_db() == EINA_FALSE) {
		BROWSER_LOGE("_close_db is failed.");
		return NULL;
	}

	return item;
}

Eina_Bool Browser_Bookmark_DB::get_bookmark_list(std::vector<bookmark_item *> &list, int folder_id)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE) {
		BROWSER_LOGE("open failed");
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select id,type,parent,address,title,sequence,editable from bookmarks where parent=? order by sequence",
					-1,&sqlite3_stmt,NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 1, folder_id) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");
		return EINA_FALSE;
	}

	bookmark_item *item;
	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		item = new(nothrow) bookmark_item;
		if (!item) {
			BROWSER_LOGE("bookmark_item new is failed.\n");
			return EINA_FALSE;
		}

		item->id = sqlite3_column_int(sqlite3_stmt, 0);
		if (sqlite3_column_int(sqlite3_stmt, 1) == BOOKMARK_NORMAL)
			item->is_folder = EINA_FALSE;
		else
			item->is_folder = EINA_TRUE;

		if (!item->is_folder) {
			const char* url = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 3));
			if (url && strlen(url) > 0)
				item->url = url;
			else
				item->url = "";
		}

		item->parent = sqlite3_column_int(sqlite3_stmt, 2);

		const char* title = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 4));
		if (title && strlen(title) > 0)
			item->title = title;
		else
			item->title = "";

		item->order_index = sqlite3_column_int(sqlite3_stmt, 5);
		item->is_editable = EINA_TRUE;//sqlite3_column_int(sqlite3_stmt, 6);

		item->is_delete = EINA_FALSE;
		item->user_data_1 = NULL;
		item->user_data_2 = NULL;
		item->user_data_3 = 0;
		list.push_back(item);
	}

	error = sqlite3_finalize(sqlite3_stmt);
	if (error != SQLITE_OK)
	{
		BROWSER_LOGE("sqlite3_finalize is failed. (%d)", error);
		return EINA_FALSE;
	}

	if (_close_db() == EINA_FALSE) {
		BROWSER_LOGE("_close_db is failed.");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool Browser_Bookmark_DB::get_folder_list(std::vector<bookmark_item *> &list)
{
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select id,type,parent,address,title,sequence from bookmarks where type=1 and parent=? order by sequence",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_int(sqlite3_stmt, 1, BROWSER_BOOKMARK_MAIN_FOLDER_ID) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");

	bookmark_item *item;
	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW)
	{
		item = new(nothrow) bookmark_item;
		if (!item) {
			BROWSER_LOGE("bookmark_item new is failed.\n");
			return EINA_FALSE;
		}

		item->id = sqlite3_column_int(sqlite3_stmt, 0);
		item->is_folder = sqlite3_column_int(sqlite3_stmt, 1) == BOOKMARK_NORMAL ? EINA_FALSE : EINA_TRUE;
		item->parent = sqlite3_column_int(sqlite3_stmt, 2);
		item->title = "";

		const char* title = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 4));
		if (title && strlen(title)>0)
			item->title = title;
		else
			item->title = "";
		item->order_index = sqlite3_column_int(sqlite3_stmt, 5);
		item->is_editable = EINA_TRUE;
		item->is_delete = EINA_FALSE;

		list.push_back(item);
	}

	error = sqlite3_finalize(sqlite3_stmt);
	if (error != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed. (%d)\n", error);

	_close_db();

	return (error == SQLITE_DONE || error == SQLITE_OK);
}


