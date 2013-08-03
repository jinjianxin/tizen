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

#ifndef BROWSER_SETTINGS_CLASS_H
#define BROWSER_SETTINGS_CLASS_H

#include "browser-config.h"

class Browser_Settings_Main_View;

class Browser_Settings_Class {
public:
	Browser_Settings_Class(void);
	~Browser_Settings_Class(void);

	Eina_Bool init(void);
private:
	Browser_Settings_Main_View *m_settings_main_view;
};

#endif /* BROWSER_SETTINGS_CLASS_H */

