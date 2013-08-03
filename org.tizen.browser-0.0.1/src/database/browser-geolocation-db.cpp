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
  *@file browser-geolocation-db.cpp
  *@brief 浏览器定位数据库
  */


using namespace std;

#include "browser-geolocation-db.h"

sqlite3* Browser_Geolocation_DB::m_db_descriptor = NULL;

Browser_Geolocation_DB::Browser_Geolocation_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Geolocation_DB::~Browser_Geolocation_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Eina_Bool Browser_Geolocation_DB::_open_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int error = db_util_open(BROWSER_GEOLOCATION_DB_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool Browser_Geolocation_DB::_close_db(void)
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

Eina_Bool Browser_Geolocation_DB::save_geolocation_host(const char *address, bool accept)
{
	BROWSER_LOGD("[%s]", address);

	if (!address || strlen(address) == 0) {
		BROWSER_LOGE("address is empty");
		return EINA_FALSE;
	}

	if (get_geolocation_host_count(address) > 0) {
		BROWSER_LOGE("already exist\n");
		return EINA_FALSE;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "INSERT INTO geolocation (address, accept) values (?, ?)",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_text(sqlite3_stmt, 1, address, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");

	if (sqlite3_bind_int(sqlite3_stmt, 2, accept) != SQLITE_OK)
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

	return (error == SQLITE_DONE || error == SQLITE_ROW);
}

Eina_Bool Browser_Geolocation_DB::get_geolocation_host(const char *address, bool &accept)
{
	if (!address || strlen(address) == 0) {
		BROWSER_LOGE("address is empty");
		return EINA_FALSE;
	}

	if (get_geolocation_host_count(address) <= 0) {
		BROWSER_LOGE("No data or database error\n");
		return EINA_FALSE;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	std::string query = "select accept from geolocation where address like '%" + std::string(address) + "%'";

	int error = sqlite3_prepare_v2(m_db_descriptor, query.c_str(), -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	accept = 0;
	error = sqlite3_step(sqlite3_stmt);
	if (error == SQLITE_ROW) {
		accept = sqlite3_column_int(sqlite3_stmt, 0);
		BROWSER_LOGD("address: %s accept %d", address, accept);
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	_close_db();
	if (error == SQLITE_DONE || error == SQLITE_ROW)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Eina_Bool Browser_Geolocation_DB::remove_geolocation_data(const char *address)
{

	if (!address || strlen(address) == 0) {
		BROWSER_LOGE("address is empty");
		return EINA_FALSE;
	}

	if (get_geolocation_host_count(address) <= 0) {
		BROWSER_LOGE("No data\n");
		return EINA_FALSE;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	std::string query = "DELETE FROM geolocation WHERE address LIKE '%" + std::string(address) + "%'";

	int error = sqlite3_prepare_v2(m_db_descriptor, query.c_str(), -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

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

	return (error == SQLITE_DONE || error == SQLITE_ROW);
}

Eina_Bool Browser_Geolocation_DB::remove_all_geolocation_data(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	std::string query = "DELETE FROM geolocation";

	int error = sqlite3_prepare_v2(m_db_descriptor, query.c_str(), -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

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

	return (error == SQLITE_DONE || error == SQLITE_ROW);
}

int Browser_Geolocation_DB::get_geolocation_host_count(const char *address)
{
	unsigned count = 0;

	if (!address || strlen(address) == 0) {
		BROWSER_LOGE("address is empty");
		return -1;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	std::string query = "select count(*) from geolocation where address like '%" + std::string(address) + "%'";

	int error = sqlite3_prepare_v2(m_db_descriptor, query.c_str(), -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return -1;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (error != SQLITE_ROW) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return -1;
	}

	count = sqlite3_column_int(sqlite3_stmt, 0);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return -1;
	}

	_close_db();
	BROWSER_LOGD("count: %d\n", count);
	return count;

}

Eina_Bool Browser_Geolocation_DB::get_geolocation_info_list(std::vector<geolocation_info *> &list)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE) {
		BROWSER_LOGE("open failed");
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select address,accept from geolocation",
					-1,&sqlite3_stmt,NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	geolocation_info *item;
	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		item = new(nothrow) geolocation_info;
		if (!item) {
			BROWSER_LOGE("bookmark_item new is failed.\n");
			return EINA_FALSE;
		}

		item->url = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0));
		BROWSER_LOGD("<<< url = [%s] >>>", item->url.c_str());

		item->allow = sqlite3_column_int(sqlite3_stmt, 1);
		BROWSER_LOGD("<<< accept = [%d] >>>", item->allow);

		item->geolocation = EINA_TRUE;
		item->storage = EINA_FALSE;
		item->user_data = NULL;

		list.push_back(item);
	}

	error = sqlite3_finalize(sqlite3_stmt);
	if (error != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_finalize is failed. (%d)", error);
		return EINA_FALSE;
	}

	if (_close_db() == EINA_FALSE) {
		BROWSER_LOGE("_close_db is failed.");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

int Browser_Geolocation_DB::get_geolocation_info_count(void)
{
	BROWSER_LOGD("[%s]", __func__);
	unsigned count = 0;

	if (_open_db() == EINA_FALSE)
		return -1;

	sqlite3_stmt *sqlite3_stmt = NULL;
	std::string query = "select count(*) from geolocation";

	int error = sqlite3_prepare_v2(m_db_descriptor, query.c_str(), -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return -1;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (error != SQLITE_ROW) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return -1;
	}

	count = sqlite3_column_int(sqlite3_stmt, 0);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return -1;
	}

	_close_db();
	BROWSER_LOGD("count: %d\n", count);
	return count;
}
