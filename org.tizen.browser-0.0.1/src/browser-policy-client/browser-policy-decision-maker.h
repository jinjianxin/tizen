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


#ifndef BROWSER_POLICY_DECISION_MAKER_H
#define BROWSER_POLICY_DECISION_MAKER_H

#include "browser-config.h"
#include "browser-view.h"

#include <string>

using namespace std;

class Browser_Common_View;

class Browser_Policy_Decision_Maker : public Browser_Common_View {
public:
	Browser_Policy_Decision_Maker(Evas_Object *navi_bar, Browser_View *browser_view);
	~Browser_Policy_Decision_Maker(void);

	void init(Evas_Object *ewk_view);
	void deinit(void);
	void pause(void);
private:
	enum {
		policy_use,
		policy_download,
		policy_ignore
	};

	/* ewk view event callback functions */
	static void __decide_policy_for_navigation_action(void *data, Evas_Object *obj, void *event_info);
	static void __decide_policy_for_response_cb(void *data, Evas_Object *obj, void *event_info);

	/* download client callback functions */
	static void __download_did_start_cb(const char *download_url, void *user_data);

	/* Elementary event callback functions */
	static void __popup_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __player_cb(void *data, Evas_Object *obj, void *event_info);
	static void __internet_cb(void *data, Evas_Object *obj, void *event_info);
	static bool __launch_matched_application_cb(service_h service, const char *package, void *data);

	/* Warning : MUST free() returned char* */
	void _request_download(Ewk_Policy_Decision *policy_decision);
	Eina_Bool _launch_download_app(const char *url, const char *cookies = NULL);
	string _get_extension_name_from_url(string &url);
	Eina_Bool _show_app_list_popup(void);
	const char *_get_app_name_from_pkg_name(string& pkg_name);
	Eina_Bool _handle_exscheme(void);

	Evas_Object *m_ewk_view;
	Evas_Object *m_navi_bar;
	Evas_Object *m_list_popup;
	Evas_Object *m_app_list;
	Browser_View *m_browser_view;
	Eina_Bool m_found_matched_app;

	string m_url;
	string m_cookies;
	string m_default_player_pkg_name;
};
#endif /* BROWSER_POLICY_DECISION_MAKER_H */

