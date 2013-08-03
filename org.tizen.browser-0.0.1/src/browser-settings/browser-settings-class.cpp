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

#include "browser-settings-class.h"
#include "browser-settings-main-view.h"

Browser_Settings_Class::Browser_Settings_Class(void)
:
	m_settings_main_view(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Settings_Class::~Browser_Settings_Class(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_settings_main_view)
		delete m_settings_main_view;
}

Eina_Bool Browser_Settings_Class::init(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_settings_main_view = new(nothrow) Browser_Settings_Main_View;
	if (!m_settings_main_view) {
		BROWSER_LOGE("new Browser_Settings_Main_View failed");
		return EINA_FALSE;
	}

	if (!m_settings_main_view->init()) {
		BROWSER_LOGE("m_settings_main_view->init() failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

