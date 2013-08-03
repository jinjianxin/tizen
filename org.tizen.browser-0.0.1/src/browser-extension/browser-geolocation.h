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


#ifndef BROWSER_GEOLOCATION_H
#define BROWSER_GEOLOCATION_H

#include "browser-common-view.h"
#include "browser-config.h"

#include <location/locations.h>

class Browser_Class;

class Browser_Geolocation : public Browser_Common_View {
	friend class Browser_Class;
public:
	Browser_Geolocation(void);
	~Browser_Geolocation(void);

	Eina_Bool init(Evas_Object *ewk_view);
private:
	Eina_Bool _show_request_confirm_popup(std::string msg);
	static void __geolocation_permission_request_cb(void *data, Evas_Object *obj, void *event_info);

	/* Elementary event callback functions */
	static void __popup_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_cancel_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_popup;
	Evas_Object *m_check;
	Browser_Geolocation_DB *m_geolocation_db;
	string m_host_string;
	Evas_Object *m_ewk_view;

	Ewk_Geolocation_Permission_Request m_permission_request;
	location_manager_h m_location_manager;
	Eina_Bool m_is_location_manager_valid;
};

#endif /* BROWSER_GEOLOCATION_H */

