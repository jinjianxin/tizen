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
  *@file browser-user-agent-db.cpp
  *@brief 浏览器标识数据库
  */

using namespace std;

#include "browser-user-agent-db.h"

sqlite3* Browser_User_Agent_DB::m_db_descriptor = NULL;

Browser_User_Agent_DB::Browser_User_Agent_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_User_Agent_DB::~Browser_User_Agent_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Eina_Bool Browser_User_Agent_DB::_open_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int error = db_util_open(BROWSER_USER_AGENT_DB_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool Browser_User_Agent_DB::_close_db(void)
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

Eina_Bool Browser_User_Agent_DB::get_user_agent(const char *title, char *&user_agent)
{
	BROWSER_LOGD("[%s]", __func__);

	if(!strcmp(title, "System user agent")){
		user_agent = vconf_get_str(VCONFKEY_ADMIN_UAGENT);
		if (!user_agent) {
			BROWSER_LOGE("vconf_get_str(VCONFKEY_ADMIN_UAGENT) failed.");
			return EINA_FALSE;
		}
		return EINA_TRUE;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select value from user_agents where name=?",
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

	error = sqlite3_step(sqlite3_stmt);

	string value;
	if (error == SQLITE_ROW)
		value = reinterpret_cast<const char*>(sqlite3_column_text(sqlite3_stmt, 0));

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	if (!value.empty())
		user_agent = strdup(value.c_str());
	else
		user_agent = NULL;

	_close_db();

	return (error == SQLITE_DONE || error == SQLITE_ROW);
}

