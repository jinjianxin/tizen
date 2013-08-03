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


#include <ail.h>
#include <url_download.h>

#include "browser-common-view.h"
#include "browser-policy-decision-maker.h"

#include "ewk_policy_decision.h"
Browser_Policy_Decision_Maker::Browser_Policy_Decision_Maker(Evas_Object *navi_bar, Browser_View *browser_view)
:
	m_ewk_view(NULL)
	,m_list_popup(NULL)
	,m_app_list(NULL)
	,m_navi_bar(navi_bar)
	,m_browser_view(browser_view)
	,m_found_matched_app(EINA_FALSE)
{
	BROWSER_LOGD("[%s]", __func__);

	Ewk_Context *ewk_context = ewk_context_default_get();
	ewk_context_did_start_download_callback_set(ewk_context, __download_did_start_cb, this);
}

Browser_Policy_Decision_Maker::~Browser_Policy_Decision_Maker(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_app_list) {
		evas_object_del(m_app_list);
		m_app_list = NULL;
	}
	if(m_list_popup) {
		evas_object_del(m_list_popup);
		m_list_popup = NULL;
	}
}

void Browser_Policy_Decision_Maker::init(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	deinit();
	m_ewk_view = ewk_view;

	evas_object_smart_callback_add(m_ewk_view, "policy,navigation,decide", __decide_policy_for_navigation_action, this);
	evas_object_smart_callback_add(m_ewk_view, "policy,response,decide", __decide_policy_for_response_cb, this);
}

void Browser_Policy_Decision_Maker::deinit(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_ewk_view) {
		evas_object_smart_callback_del(m_ewk_view, "policy,navigation,decide", __decide_policy_for_navigation_action);
		evas_object_smart_callback_del(m_ewk_view, "policy,response,decide", __decide_policy_for_response_cb);
	}
}

void Browser_Policy_Decision_Maker::pause(void)
{
	if (m_list_popup)
		__popup_response_cb(this, NULL, NULL);
}

void Browser_Policy_Decision_Maker::__download_did_start_cb(const char *download_url, void *user_data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!user_data)
		return;

	Browser_Policy_Decision_Maker *decision_maker = (Browser_Policy_Decision_Maker *)user_data;
	BROWSER_LOGD("download_url=[%s]", download_url);

	if (!decision_maker->_launch_download_app(download_url))
		BROWSER_LOGE("_launch_download_app failed");
}

Eina_Bool Browser_Policy_Decision_Maker::_handle_exscheme(void)
{
	BROWSER_LOGD("request_url = [%s]", m_url.c_str());
	if (m_url.empty()) {
		BROWSER_LOGE("url is null");
		return EINA_FALSE;
	}

	if (!m_url.compare(0, strlen(BROWSER_HTTP_SCHEME), BROWSER_HTTP_SCHEME)
	    || !m_url.compare(0, strlen(BROWSER_HTTPS_SCHEME), BROWSER_HTTPS_SCHEME)
	    || !m_url.compare(0, strlen(BROWSER_FILE_SCHEME), BROWSER_FILE_SCHEME))
		return EINA_FALSE;

	if (!m_url.compare(0, strlen(BROWSER_RTSP_SCHEME), BROWSER_RTSP_SCHEME)) {
		BROWSER_LOGD("rtsp scheme");
		if (!_launch_streaming_player(m_url.c_str()))
			BROWSER_LOGE("_launch_streaming_player failed");
		return EINA_TRUE;
	} else if (!m_url.compare(0, strlen(BROWSER_MAIL_TO_SCHEME), BROWSER_MAIL_TO_SCHEME)) {
		BROWSER_LOGD("mail to scheme");
		if (_send_via_email(m_url.c_str()) != EINA_TRUE)
			BROWSER_LOGE("_send_via_email failed");
		return EINA_TRUE;
	} else if (!m_url.compare(0, strlen(BROWSER_SMS_SCHEME), BROWSER_SMS_SCHEME)) {
		BROWSER_LOGD("sms scheme");
		std::string body_string;

		if (m_url.find("?body=") != string::npos) {
			body_string = m_url.substr(m_url.find("?body="));
			m_url = m_url.substr(0, m_url.length() - body_string.length());
		}

		if (!body_string.empty())
			body_string = std::string(body_string.c_str() + strlen("?body="));
		_send_via_message(body_string, std::string(m_url.c_str() + strlen(BROWSER_SMS_SCHEME)));
		return EINA_TRUE;
	} else if (!m_url.compare(0, strlen(BROWSER_SMS_TO_SCHEME), BROWSER_SMS_TO_SCHEME)) {
		BROWSER_LOGD("sms scheme");
		std::string body_string;

		if (m_url.find("?body=") != string::npos) {
			body_string = m_url.substr(m_url.find("?body="));
			m_url = m_url.substr(0, m_url.length() - body_string.length());
		}

		if (!body_string.empty())
			body_string = std::string(body_string.c_str() + strlen("?body="));
		_send_via_message(body_string, std::string(m_url.c_str() + strlen(BROWSER_SMS_TO_SCHEME)));
		return EINA_TRUE;
	} else if (!m_url.compare(0, strlen(BROWSER_MMS_SCHEME), BROWSER_MMS_SCHEME)) {
		BROWSER_LOGD("mms scheme");
		std::string body_string;

		if (m_url.find("?body=") != string::npos || m_url.find("&body=") != string::npos) {
			if (m_url.find("?body=") != string::npos)
				body_string = m_url.substr(m_url.find("?body="));
			else
				body_string = m_url.substr(m_url.find("&body="));
			m_url = m_url.substr(0, m_url.length() - body_string.length());
		}
		if (m_url.find("?subject=") != string::npos) {
			m_url = m_url.substr(0, m_url.length() - m_url.substr(m_url.find("?subject=")).length());
		}

		if (!body_string.empty())
			body_string = std::string(body_string.c_str() + strlen("?body="));
		_send_via_message(body_string, std::string(m_url.c_str() + strlen(BROWSER_MMS_SCHEME)));
		return EINA_TRUE;
	} else if (!m_url.compare(0, strlen(BROWSER_MMS_TO_SCHEME), BROWSER_MMS_TO_SCHEME)) {
		BROWSER_LOGD("mms scheme");
		std::string body_string;

		if (m_url.find("?body=") != string::npos || m_url.find("&body=") != string::npos) {
			if (m_url.find("?body=") != string::npos)
				body_string = m_url.substr(m_url.find("?body="));
			else
				body_string = m_url.substr(m_url.find("&body="));

			m_url = m_url.substr(0, m_url.length() - body_string.length());
		}
		if (m_url.find("?subject=") != string::npos) {
			m_url = m_url.substr(0, m_url.length() - m_url.substr(m_url.find("?subject=")).length());
		}

		if (!body_string.empty())
			body_string = std::string(body_string.c_str() + strlen("?body="));
		_send_via_message(body_string, std::string(m_url.c_str() + strlen(BROWSER_MMS_TO_SCHEME)));

		return EINA_TRUE;
	} else if (!m_url.compare(0, strlen(BROWSER_WTAI_WP_AP_SCHEME), BROWSER_WTAI_WP_AP_SCHEME)) {
		BROWSER_LOGD("wtai://wp/mc or wtai://wp/ap");
		m_url = std::string(m_url.c_str() + strlen(BROWSER_WTAI_WP_AP_SCHEME));

		if (m_url.find(";") != string::npos) {
			m_url = m_url.substr(0, m_url.length() - m_url.substr(m_url.find(";")).length());
		}

		BROWSER_LOGD("phone number = [%s]", m_url.c_str());

		_add_to_contact(m_url);

		return EINA_TRUE;
	}

	return EINA_FALSE;
}

void Browser_Policy_Decision_Maker::__decide_policy_for_navigation_action(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	BROWSER_LOGD("%s", __func__);
	Browser_Policy_Decision_Maker *decision_maker = (Browser_Policy_Decision_Maker *)data;
	Ewk_Policy_Decision *policy_decision = (Ewk_Policy_Decision *)event_info;

	const char *url = ewk_policy_decision_url_get(policy_decision);
	BROWSER_LOGD("<<< url = [%s]", url);
	if (url && strlen(url))
		decision_maker->m_url = std::string(url);

	decision_maker->m_cookies.clear();

	if (decision_maker->_handle_exscheme())
		ewk_policy_decision_ignore(policy_decision);
	else
		ewk_policy_decision_use(policy_decision);
}

void Browser_Policy_Decision_Maker::__decide_policy_for_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_Policy_Decision_Maker *decision_maker = (Browser_Policy_Decision_Maker *)data;
	Ewk_Policy_Decision *policy_decision = (Ewk_Policy_Decision *)event_info;

	Ewk_Policy_Decision_Type policy_type = ewk_policy_decision_type_get(policy_decision);

	switch (policy_type) {
	case EWK_POLICY_DECISION_USE:
		BROWSER_LOGD("policy_use");
		ewk_policy_decision_use(policy_decision);
		break;

	case EWK_POLICY_DECISION_DOWNLOAD:
		BROWSER_LOGD("policy_download");
		ewk_policy_decision_suspend(policy_decision);
		decision_maker->_request_download(policy_decision);
		ewk_policy_decision_ignore(policy_decision);
		break;

	case EWK_POLICY_DECISION_IGNORE:
	default:
		BROWSER_LOGD("policy_ignore");
		ewk_policy_decision_ignore(policy_decision);
		break;
	}
}

void Browser_Policy_Decision_Maker::_request_download(Ewk_Policy_Decision *policy_decision)
{

	string extension_name;
	string ambiguous_mime1 = "text/plain";
	string ambiguous_mime2 = "application/octet-stream";
	int ret = 0;

	BROWSER_LOGD("[%s]", __func__);

	m_url.clear();
	m_cookies.clear();
	m_default_player_pkg_name.clear();
	m_found_matched_app = EINA_FALSE;

	m_url = std::string(ewk_policy_decision_url_get(policy_decision));

	m_cookies = std::string(ewk_policy_decision_cookie_get(policy_decision));

	BROWSER_LOGD("url=[%s]", m_url.c_str());
	BROWSER_LOGD("cookie=[%s]", m_cookies.c_str());

	std::string content_type;
	content_type = std::string(ewk_policy_decision_response_mime_get(policy_decision));
	BROWSER_LOGD("content_type=[%s]", content_type.c_str());

	if (content_type.empty()) {
		BROWSER_LOGD("Download linked file from cotent menu");
		if (!_launch_download_app(m_url.c_str(), m_cookies.c_str()))
			BROWSER_LOGE("_launch_download_app failed");
		return;
	}
	BROWSER_LOGD("mime type [%s]", content_type.c_str());
	/* If mime is ambiguous mime or not. If it is, get extension name from url */
	if (content_type.compare(ambiguous_mime1) == 0
	    || content_type.compare(ambiguous_mime2) == 0) {
		extension_name = _get_extension_name_from_url(m_url);
	}
	/* If mime is ambiguous and extension name is existed,
	*   decide streaming player according to extension name from url
	* 1. mp4, 3gp : streaming video player case
	* 2. mp3 : streaming music player case
	* 3. otherewise : download app case
	*/
	if (!extension_name.empty()) {
		BROWSER_LOGD("extension name from url [%s]", extension_name.c_str());
#ifdef SUPPORT_HLS
		if (extension_name.compare("m3u") == 0 ||
				extension_name.compare("m3u8") == 0) {
			_launch_streaming_player(m_url.c_str(), m_cookies.c_str());
			return;
		} else
#endif
		if (extension_name.compare("mp4") == 0 || extension_name.compare("3gp") == 0) {
			m_default_player_pkg_name = SEC_VIDEO_PLAYER;
			if (!_show_app_list_popup())
				BROWSER_LOGE("_show_app_list_popup failed");
		} else if (extension_name.compare("mp3") == 0) {
			m_default_player_pkg_name = SEC_MUSIC_PLAYER;
			if (!_show_app_list_popup())
				BROWSER_LOGE("_show_app_list_popup failed");
		} else {
			if (!_launch_download_app(m_url.c_str(), m_cookies.c_str()))
				BROWSER_LOGE("_launch_download_app failed");
		}
		return;
	}

#ifdef SUPPORT_HLS
	if (content_type.compare("application/vnd.apple.mpegurl") == 0 ||
					content_type.compare("application/x-mpegurl") == 0 ||
					content_type.compare("application/m3u") == 0 ||
					content_type.compare("audio/x-mpegurl") == 0 ||
					content_type.compare("audio/m3u") == 0 ||
					content_type.compare("audio/x-m3u") == 0) {
		_launch_streaming_player(m_url.c_str(), m_cookies.c_str());
		return;
	}
#endif

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return;
	}

	if (!service_handle) {
		BROWSER_LOGE("service handle is NULL");
		return;
	}

	if (service_set_operation(service_handle, SERVICE_OPERATION_VIEW) < 0) {
		BROWSER_LOGE("Fail to set service operation");
		service_destroy(service_handle);
		return;
	}

	if (service_set_mime(service_handle, content_type.c_str()) < 0) {
		BROWSER_LOGE("Fail to set mime type");
		service_destroy(service_handle);
		return;
	}

	ret = service_foreach_app_matched(service_handle, __launch_matched_application_cb, this);

	if (ret < 0) {
		BROWSER_LOGE("Fail to get default application by mime type");
		service_destroy(service_handle);
	}

	if (m_found_matched_app == EINA_FALSE) {
		if (!_launch_download_app(m_url.c_str(), m_cookies.c_str()))
			BROWSER_LOGE("_launch_download_app failed");

		service_destroy(service_handle);
	}

	return;
}

Eina_Bool Browser_Policy_Decision_Maker::_launch_download_app(const char *url, const char* cookie)
{
	service_h service_handle = NULL;
	BROWSER_LOGD("%s", __func__);
	if (url && strlen(url)) {

		if (service_create(&service_handle) < 0) {
			BROWSER_LOGE("Fail to create service handle");
			return EINA_FALSE;
		}
		
		if (!service_handle) {
			BROWSER_LOGE("service handle is NULL");
			return EINA_FALSE;
		}

		if (service_set_operation(service_handle, SERVICE_OPERATION_DOWNLOAD) < 0) {
			BROWSER_LOGE("Fail to set service operation");
			service_destroy(service_handle);
			return EINA_FALSE;
		}

		if (service_set_uri(service_handle, url) < 0) {
			BROWSER_LOGE("Fail to set uri");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
		if (cookie && strlen(cookie)) {
			if (service_add_extra_data(service_handle, "cookie", cookie) < 0) {
				BROWSER_LOGE("Fail to set extra data");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		}

		if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
			BROWSER_LOGE("Fail to launch service operation");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
		service_destroy(service_handle);

		return EINA_TRUE;
	}

	return EINA_FALSE;
}

string Browser_Policy_Decision_Maker::_get_extension_name_from_url(string& url)
{
	string ext;
	size_t pos = 0;

	if(url.empty()) {
		BROWSER_LOGE("url is NULL");
		return string();
	}
	pos = url.find_last_of("/");

	if (pos != string::npos) {
		string tmp;
		tmp = url.substr(pos + 1);
		pos = tmp.find_last_of(".");
		if (pos != string::npos) {
			string tmp2;
			size_t end = 0;
			tmp2 = tmp.substr(pos+1);
			pos = tmp2.find_last_of("?");
			if (pos != string::npos)
				ext = tmp2.substr(0, pos - 1);
			else
				ext = tmp2;
		}
	}
	BROWSER_LOGD("extension name[%s]\n",ext.c_str());
	return ext;
}

const char *Browser_Policy_Decision_Maker::_get_app_name_from_pkg_name(string& pkg_name)
{
	ail_appinfo_h handle;
	ail_error_e ret;
	string app_name;
	char *str = NULL;

	ret = ail_package_get_appinfo(pkg_name.c_str(), &handle);
	if (ret != AIL_ERROR_OK) {
		return NULL;
	}

	ret = ail_appinfo_get_str(handle, AIL_PROP_NAME_STR, &str);
	if (ret != AIL_ERROR_OK) {
		return NULL;
	}
	app_name = (const char*) str;
	BROWSER_LOGD("pkg's name[%s]pkg[%s]\n",app_name.c_str(),pkg_name.c_str());

	ret = ail_package_destroy_appinfo(handle);
	if (ret != AIL_ERROR_OK)
		return NULL;

	return app_name.c_str();
}

void Browser_Policy_Decision_Maker::__player_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("%s", __func__);
	if (!data)
		return;

	Browser_Policy_Decision_Maker *decision_maker = (Browser_Policy_Decision_Maker *)data;

	if (!decision_maker->_launch_streaming_player(decision_maker->m_url.c_str(), decision_maker->m_cookies.c_str()))
		BROWSER_LOGE("_launch_streaming_player failed");

	__popup_response_cb(decision_maker, NULL, NULL);
}

void Browser_Policy_Decision_Maker::__internet_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("%s", __func__);
	if (!data)
		return;

	Browser_Policy_Decision_Maker *decision_maker = (Browser_Policy_Decision_Maker *)data;

	if (!decision_maker->_launch_download_app(decision_maker->m_url.c_str(), decision_maker->m_cookies.c_str()))
		BROWSER_LOGE("_launch_download_app failed");

	__popup_response_cb(decision_maker, NULL, NULL);
}

bool Browser_Policy_Decision_Maker::__launch_matched_application_cb(service_h service_handle, const char *package, void *data)
{
	BROWSER_LOGD("%s", __func__);

	if (!data) {
		BROWSER_LOGD("unable to set Browser_Policy_Decision_Maker pointer");
		service_destroy(service_handle);
		return false;
	}

	Browser_Policy_Decision_Maker *decision_maker = (Browser_Policy_Decision_Maker *)data;
	string pkg_name = package;
	decision_maker->m_default_player_pkg_name = pkg_name;


	if (!pkg_name.empty() && (pkg_name.compare(SEC_VIDEO_PLAYER) == 0 || pkg_name.compare(SEC_MUSIC_PLAYER) == 0)) {
		BROWSER_LOGD("default app [%s]", pkg_name.c_str());
		if (!decision_maker->_show_app_list_popup())
			BROWSER_LOGE("_show_app_list_popup failed");
		else
			decision_maker->m_found_matched_app = EINA_TRUE;
	} else {
		BROWSER_LOGE("Fail to get default app");
		if (!decision_maker->_launch_download_app(decision_maker->m_url.c_str(), decision_maker->m_cookies.c_str()))
			BROWSER_LOGE("_launch_download_app failed");
	}

	service_destroy(service_handle);

	return true;
}

Eina_Bool Browser_Policy_Decision_Maker::_show_app_list_popup(void)
{
	if (m_url.empty()) {
		BROWSER_LOGE("url is empty");
		return EINA_FALSE;
	}

	m_list_popup = elm_popup_add(m_navi_bar);
	if (!m_list_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_list_popup, "menustyle");
	elm_object_part_text_set(m_list_popup, "title,text", BR_STRING_TITLE_SELECT_AN_ACTION);
	evas_object_size_hint_weight_set(m_list_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_app_list = elm_list_add(m_list_popup);
	if (!m_app_list) {
		BROWSER_LOGE("elm_list_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_app_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_app_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

	const char *app_name = _get_app_name_from_pkg_name(m_default_player_pkg_name);
	if (app_name && strlen(app_name))
		elm_list_item_append(m_app_list, app_name, NULL, NULL, __player_cb, this);

	elm_list_item_append(m_app_list, BR_STRING_INTERNET, NULL, NULL, __internet_cb, this);
	evas_object_show(m_app_list);

	elm_object_content_set(m_list_popup, m_app_list);
	evas_object_show(m_list_popup);

	Evas_Object *cancel_button = elm_button_add(m_list_popup);
	if (!cancel_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(cancel_button, BR_STRING_CLOSE);
	elm_object_part_content_set(m_list_popup, "button1", cancel_button);
	elm_object_style_set(cancel_button, "popup_button/default");
	evas_object_smart_callback_add(cancel_button, "clicked", __popup_response_cb, this);

	return EINA_TRUE;
}

void Browser_Policy_Decision_Maker::__popup_response_cb(void* data, Evas_Object* obj,
		void* event_info)
{
	BROWSER_LOGD("%s", __func__);

	if (!data)
		return;

	Browser_Policy_Decision_Maker *decision_maker = (Browser_Policy_Decision_Maker *)data;
	if (decision_maker->m_app_list) {
		evas_object_del(decision_maker->m_app_list);
		decision_maker->m_app_list = NULL;
	}
	if(decision_maker->m_list_popup) {
		evas_object_del(decision_maker->m_list_popup);
		decision_maker->m_list_popup = NULL;
	}
}

