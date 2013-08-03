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


#ifndef BROWSER_WINODW_H
#define BROWSER_WINODW_H

#include "browser-config.h"
#include <cairo.h>

class Browser_Window {
public:
	Browser_Window(void);
	~Browser_Window(void);

	/* Caution : m_ewk_view can be null even though the Browser_Window is not null.
	  * Because the m_ewk_view is destroyed and assigned to null after Browser_Class::clean_up_windows.
	  * So The null check is necessary at every usage. */
	Evas_Object *m_ewk_view;

	Evas_Object *m_ewk_view_layout;
	Evas_Object *m_portrait_snapshot_image;
#if defined(HORIZONTAL_UI)
	Evas_Object *m_landscape_snapshot_image;
#endif

	/* m_parent is a Browser_Window which invoke itself by javascript etc. */
	Browser_Window *m_parent;
	Evas_Object *m_favicon;
	Evas_Object *m_option_header_favicon;
	Evas_Object *m_secure_icon;
	Evas_Object *m_option_header_secure_icon;
	Eina_Bool m_created_by_user;
	Eina_Bool m_is_reader;
	std::string m_reader_html;
	std::string m_reader_base_url;
	std::string m_url;
	std::string m_title;
};
#endif /* BROWSER_WINODW_H */

