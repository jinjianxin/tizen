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
  *@file browser-common-view.cpp
  *@brief 浏览器常用界面的实现
  */


#include "browser-class.h"
#include "browser-common-view.h"
#include "browser-view.h"
#include "browser-window.h"

Browser_Data_Manager *Browser_Common_View::m_data_manager;
Evas_Object *Browser_Common_View::m_win;
Evas_Object *Browser_Common_View::m_navi_bar;
Evas_Object *Browser_Common_View::m_bg;
Browser_Class *Browser_Common_View::m_browser;

Browser_Common_View::Browser_Common_View(void)
:
	m_selection_info(NULL)
	,m_selection_info_layout(NULL)
	,m_selinfo_layout(NULL)
	,m_popup(NULL)
	,m_ug(NULL)
	,m_share_popup(NULL)
	,m_share_list(NULL)
	,m_call_confirm_popup(NULL)
	,m_call_type(CALL_UNKNOWN)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Common_View::~Browser_Common_View(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_selection_info_layout) {
		evas_object_del(m_selection_info_layout);
		m_selection_info_layout = NULL;
	}
	if (m_selection_info) {
		evas_object_del(m_selection_info);
		m_selection_info = NULL;
	}
	if (m_popup) {
		evas_object_del(m_popup);
		m_popup = NULL;
	}
	if (m_share_popup) {
		evas_object_del(m_share_popup);
		m_share_popup = NULL;
	}
	if (m_share_list) {
		evas_object_del(m_share_list);
		m_share_list = NULL;
	}
	if (m_ug) {
		ug_destroy(m_ug);
		m_ug = NULL;
	}
	if (m_call_confirm_popup) {
		evas_object_del(m_call_confirm_popup);
		m_call_confirm_popup = NULL;
	}

	m_sns_path_list.clear();
	m_sns_name_list.clear();
	m_sns_icon_list.clear();
}

/**
 * @brief Browser_Common_View::show_msg_popup 显示msg提示
 * @param msg  要显示的消息
 * @param timeout 超时时间
 */
void Browser_Common_View::show_msg_popup(const char *msg, int timeout)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_popup) {
		evas_object_del(m_popup);
		m_popup = NULL;
	}

	m_popup = elm_popup_add(m_navi_bar);
	evas_object_size_hint_weight_set(m_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(m_popup, msg);
	elm_popup_timeout_set(m_popup, timeout);
	evas_object_show(m_popup);
}

void Browser_Common_View::show_msg_popup(const char *title, const char *msg, int timeout)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_popup) {
		evas_object_del(m_popup);
		m_popup = NULL;
	}

	m_popup = elm_popup_add(m_navi_bar);
	evas_object_size_hint_weight_set(m_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(m_popup, "title,text", title);
	elm_object_text_set(m_popup, msg);
	elm_popup_timeout_set(m_popup, timeout);
	evas_object_show(m_popup);
}

/**
 * @brief Browser_Common_View::hide_notify_popup 隐藏提示窗口
 */

void Browser_Common_View::hide_notify_popup(void)
{
	if (m_selection_info_layout) {
		evas_object_del(m_selection_info_layout);
		m_selection_info_layout = NULL;
	}

	if (m_selection_info) {
		evas_object_del(m_selection_info);
		m_selection_info = NULL;
	}
}

/**
 * @brief Browser_Common_View::show_notify_popup 显示提示窗口
 * @param msg 要显示的msg
 * @param timeout 超时时间
 * @param has_control_bar 是哦否显示controlbar
 */

void Browser_Common_View::show_notify_popup(const char *msg, int timeout, Eina_Bool has_control_bar)
{
	if (m_selection_info_layout) {
		evas_object_del(m_selection_info_layout);
		m_selection_info_layout = NULL;
	}

	if (m_selection_info) {
		evas_object_del(m_selection_info);
		m_selection_info = NULL;
	}

	int angle = 0;
	angle = elm_win_rotation_get(m_win);
	m_selection_info = elm_notify_add(m_navi_bar);
	if (!m_selection_info) {
		BROWSER_LOGD("elm_notify_add failed");
		return;
	}
	elm_notify_orient_set(m_selection_info, ELM_NOTIFY_ORIENT_BOTTOM);
	evas_object_size_hint_weight_set(m_selection_info, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_selection_info, EVAS_HINT_FILL, EVAS_HINT_FILL);
	m_selection_info_layout = elm_layout_add(m_selection_info);
	if (!m_selection_info_layout) {
		BROWSER_LOGD("elm_layout_add failed");
		return;
	}
	evas_object_size_hint_weight_set(m_selection_info_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_selection_info_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_content_set(m_selection_info, m_selection_info_layout);

	if (has_control_bar) {
		if(angle == 0 || angle == 180)
		   elm_layout_theme_set(m_selection_info_layout, "standard", "selectioninfo",
		   						"vertical/bottom_64");
		else
		   elm_layout_theme_set(m_selection_info_layout, "standard", "selectioninfo",
		   						"horizontal/bottom_64");
	} else {
		if(angle == 0 || angle == 180)
		   elm_layout_theme_set(m_selection_info_layout, "standard", "selectioninfo",
		   						"vertical/bottom_12");
		else
		   elm_layout_theme_set(m_selection_info_layout, "standard", "selectioninfo",
		   						"horizontal/bottom_12");
	}
	edje_object_part_text_set(elm_layout_edje_get(m_selection_info_layout), "elm.text", msg);

	if (timeout)
		elm_notify_timeout_set(m_selection_info, timeout);

	evas_object_show(m_selection_info);
}

/**
 * @brief Browser_Common_View::show_notify_popup_layout 显示layput布局的提示
 * @param msg 要显示的msg
 * @param timeout 超时时间
 * @param parent 父对象
 */
void Browser_Common_View::show_notify_popup_layout(const char *msg, int timeout, Evas_Object *parent)
{
	if (m_selinfo_layout) {
		evas_object_del(m_selinfo_layout);
		m_selinfo_layout = NULL;
	}

	m_selinfo_layout = elm_layout_add(parent);
	if (!m_selinfo_layout) {
		BROWSER_LOGD("elm_layout_add failed");
		return;
	}
	elm_object_part_content_set(parent,
			"selinfo.swallow.contents",
			m_selinfo_layout);
	evas_object_size_hint_weight_set(m_selinfo_layout,
			EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_selinfo_layout,
			EVAS_HINT_FILL,
			EVAS_HINT_FILL);

	elm_object_content_set(m_selection_info, m_selection_info_layout);

	/* Set the layout theme */
	elm_layout_theme_set(m_selinfo_layout, "standard", "selectioninfo", "default");
	/* Set the text */
	elm_object_part_text_set(m_selinfo_layout, "elm.text", msg);
	elm_object_signal_emit(parent, "show,selection,info", "elm");
}

/**
 * @brief Browser_Common_View::hide_notify_popup_layout 隐藏layout显示的布局
 * @param parent
 */
void Browser_Common_View::hide_notify_popup_layout(Evas_Object *parent)
{
	if (m_selinfo_layout) {
		evas_object_del(m_selinfo_layout);
		m_selinfo_layout = NULL;
	}

	elm_object_signal_emit(parent, "hide,selection,info", "elm");
}

Eina_Bool Browser_Common_View::find_word_with_text(const char *text_to_find)
{
	BROWSER_LOGD("[%s], text_to_find[%s]", __func__, text_to_find);

	if (!text_to_find)
		return EINA_FALSE;

	Browser_View *browser_view = m_data_manager->get_browser_view();
	return browser_view->launch_find_word_with_text(text_to_find);;
}

/**
 * @brief Browser_Common_View::_capture_snapshot 获得当前focus ewkview的快照
 * @param window Browser_Window对象
 * @param scale 缩放比例
 * @return  返回快照对象
 */
Evas_Object *Browser_Common_View::_capture_snapshot(Browser_Window *window, float scale)
{
	BROWSER_LOGD("[%s]", __func__);

	int focused_ewk_view_w = 0;
	int focused_ewk_view_h = 0;
	evas_object_geometry_get(window->m_ewk_view, NULL, NULL,
						&focused_ewk_view_w, &focused_ewk_view_h);

	Evas_Object *rectangle = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
	evas_object_size_hint_min_set(rectangle, focused_ewk_view_w * scale, focused_ewk_view_h * scale);
	evas_object_resize(rectangle, focused_ewk_view_w * scale, focused_ewk_view_h * scale);
	evas_object_color_set(rectangle, 255, 255, 255, 255);
	return rectangle;
}

void Browser_Common_View::__post_to_sns_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_Common_View *common_view = (Browser_Common_View *)data;

	Elm_Object_Item *selected_item = elm_list_selected_item_get(common_view->m_share_list);
	const char *sns_name = elm_object_item_text_get(selected_item);
	BROWSER_LOGD("sns_name=[%s]", sns_name);

	if (!common_view->_post_to_sns(std::string(sns_name), common_view->m_share_url))
		BROWSER_LOGE("_post_to_sns failed");

	__popup_response_cb(common_view, NULL, NULL);
}

Eina_Bool Browser_Common_View::_post_to_sns(std::string sns_name, std::string url)
{
	BROWSER_LOGD("sns_name=[%s],  url=[%s]", sns_name.c_str(), url.c_str());
	if (url.empty() || sns_name.empty()) {
		show_msg_popup(BR_STRING_EMPTY);
		return EINA_FALSE;
	}

	int index = 0;
	for (index = 0 ; index < m_sns_name_list.size() ; index++) {
		if (m_sns_name_list[index].find(sns_name) != string::npos)
			break;
	}

	if (m_sns_path_list[index].find("twitter") != string::npos
	    || m_sns_path_list[index].find("facebook") != string::npos) {

		int ret = 0;
		service_h service_handle = NULL;
		if (service_create(&service_handle) < 0) {
			BROWSER_LOGE("Fail to create service handle");
			return EINA_FALSE;
		}
		if (!service_handle) {
			BROWSER_LOGE("service handle is NULL");
			return EINA_FALSE;
		}
		if (service_set_operation(service_handle, SERVICE_OPERATION_SEND_TEXT) < 0) {
			BROWSER_LOGE("Fail to set service operation");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
		if (service_add_extra_data(service_handle, SERVICE_DATA_TEXT, (char *)url.c_str()) < 0) {
			BROWSER_LOGE("Fail to set post data");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
		if (service_set_package(service_handle, m_sns_path_list[index].c_str()) < 0) {
			BROWSER_LOGE("Fail to set SNS");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
		if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
			BROWSER_LOGE("Fail to launch service operation");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
		service_destroy(service_handle);
	}

	return EINA_TRUE;
}

void Browser_Common_View::__send_via_message_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_Common_View *common_view = (Browser_Common_View *)data;
	if (!common_view->_send_via_message(common_view->m_share_url, std::string()))
		BROWSER_LOGE("_send_via_message failed");

	__popup_response_cb(common_view, NULL, NULL);
}

Eina_Bool Browser_Common_View::_send_via_message(std::string url, std::string to, Eina_Bool attach_file)
{
	BROWSER_LOGD("[%s], url[%s], to[%s]", __func__, url.c_str(), to.c_str());
	if (url.empty() && to.empty()) {
		show_msg_popup(BR_STRING_EMPTY);
		return EINA_FALSE;
	}

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!service_handle) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!url.empty()) {
		if (attach_file) {
			if (service_set_operation(service_handle, SERVICE_OPERATION_SEND) < 0) {
				BROWSER_LOGE("Fail to set service operation");
				service_destroy(service_handle);
				return EINA_FALSE;
			}

			if (service_add_extra_data(service_handle, "ATTACHFILE", url.c_str())) {
				BROWSER_LOGE("Fail to set extra data");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		} else {
			if (service_set_operation(service_handle, SERVICE_OPERATION_SEND_TEXT) < 0) {
				BROWSER_LOGE("Fail to set service operation");
				service_destroy(service_handle);
				return EINA_FALSE;
			}

			if (service_add_extra_data(service_handle, SERVICE_DATA_TEXT, url.c_str()) < 0) {
				BROWSER_LOGE("Fail to set extra data");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		}
	}

	if (!to.empty()) {
		if (url.empty()) {
			if (service_set_operation(service_handle, SERVICE_OPERATION_SEND_TEXT) < 0) {
				BROWSER_LOGE("Fail to set service operation");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		}

		if (service_add_extra_data(service_handle, SERVICE_DATA_TO , to.c_str()) < 0) {
			BROWSER_LOGE("Fail to set extra data");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	}

	if (service_set_package(service_handle, SEC_MESSAGE) < 0) {//SEC_EMAIL
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}
	service_destroy(service_handle);

	return EINA_TRUE;
}

void Browser_Common_View::__send_via_email_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("%s", __func__);
	if (!data)
		return;

	Browser_Common_View *common_view = (Browser_Common_View *)data;
	if (!common_view->_send_via_email(common_view->m_share_url))
		BROWSER_LOGE("_send_via_email failed");

	__popup_response_cb(common_view, NULL, NULL);
}

void Browser_Common_View::__share_via_nfc_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("%s", __func__);
	if (!data)
		return;

	Browser_Common_View *common_view = (Browser_Common_View *)data;
	if (!common_view->_share_via_nfc(common_view->m_share_url))
		BROWSER_LOGE("_share_via_nfc failed");

	__popup_response_cb(common_view, NULL, NULL);
}

Eina_Bool Browser_Common_View::_launch_streaming_player(const char *url, const char *cookie)
{
	BROWSER_LOGD("%s", __func__);
	if (!url || strlen(url) == 0) {
		BROWSER_LOGE("url is empty");
		return EINA_FALSE;
	}

	bool is_running = false;
	if (app_manager_is_running(SEC_VT_CALL, &is_running) < 0) {
		BROWSER_LOGE("Fail to get app running information\n");
		return EINA_FALSE;
	}
	if (is_running) {
		BROWSER_LOGE("video-call is running......\n");
		show_msg_popup(BR_STRING_WARNING_VIDEO_PLAYER);
		return EINA_FALSE;
	}

	service_h service_handle = NULL;

	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!service_handle) {
		BROWSER_LOGE("service handle is NULL");
		return EINA_FALSE;
	}

	BROWSER_LOGD("url=[%s]", url);
	if (service_add_extra_data(service_handle, "path", url) < 0) {
		BROWSER_LOGE("Fail to set extra data");
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

	if (service_set_package(service_handle,SEC_STREAMING_PLAYER) < 0) {
		BROWSER_LOGE("Fail to set package");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool Browser_Common_View::_check_available_sns_account(void)
{
	BROWSER_LOGD("%s", __func__);

	int error_code = account_connect();
	bool result = EINA_FALSE;

	if(error_code != ACCOUNT_ERROR_NONE) {
		BROWSER_LOGD("account_connect failed with error_code[%d].\n", error_code);
		return EINA_FALSE;
	}

	if (account_query_account_by_package_name(__check_available_sns_account_cb,
				"org.tizen.facebook", NULL) == ACCOUNT_ERROR_NONE) {
		BROWSER_LOGD("Account for Facebook was set\n");
	} else if (account_query_account_by_package_name(__check_available_sns_account_cb,
				"org.tizen.twitter", NULL) == ACCOUNT_ERROR_NONE) {
		BROWSER_LOGD("Account for Twitter was set\n");
	} else {
		BROWSER_LOGD("Account Queried failed \n");
		error_code = account_disconnect();
		if(error_code != ACCOUNT_ERROR_NONE) {
			BROWSER_LOGD("(%d)-[Account] ret = %d, \n", __LINE__, error_code);
			return EINA_FALSE;
		}
	}

	error_code = account_disconnect();
	if(error_code != ACCOUNT_ERROR_NONE) {
		BROWSER_LOGD("(%d)-[Account] ret = %d, \n", __LINE__, error_code);
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

bool Browser_Common_View::__check_available_sns_account_cb(account_h handle, void *data)
{
	BROWSER_LOGD("%s", __func__);

	char *pkg_name = NULL;
	account_get_package_name(handle, &pkg_name);
	BROWSER_LOGD("pkg_name [%s]", pkg_name);

	if (pkg_name)
		free(pkg_name);
	pkg_name = NULL;

	return true;
}

Eina_Bool Browser_Common_View::_get_available_sns_list(void)
{
	BROWSER_LOGD("[%s]\n", __func__);

	int  error_code = 0;
	error_code = account_connect();

	if (error_code == ACCOUNT_ERROR_RECORD_NOT_FOUND) {
		show_msg_popup(BR_STRING_ERROR, BR_STRING_NOT_FOUND_URL, 2);
		return EINA_FALSE;
	} else if (error_code == ACCOUNT_ERROR_NONE) {
		error_code = account_foreach_account_from_db(__get_sns_list, this);
		error_code = account_disconnect();

		if (error_code !=ACCOUNT_ERROR_NONE) {
			BROWSER_LOGD("account_svc_disconnect failed with error code [%d]\n", error_code);
			return EINA_FALSE;
		}
	} else {
		BROWSER_LOGD("account_connect failed with error code [%d]\n", error_code);
		return EINA_FALSE;
	}

	return EINA_TRUE;
}


bool Browser_Common_View::__get_sns_list(account_h account, void *data)
{
	BROWSER_LOGD("[%s]\n", __func__);

	if (!data)
		return false;

	Browser_Common_View *common_view = (Browser_Common_View *)data;

	int account_id = 0;
	char *service_name = NULL;
	char *lib_path = NULL;
	char *icon_path = NULL;

	Eina_Bool can_post = EINA_FALSE;

	account_get_capability(account, __get_post_capability_cb, &can_post);
	account_get_account_id(account, &account_id);
	BROWSER_LOGD("account_id: %d\n", account_id);

	account_get_domain_name(account, &service_name);
	BROWSER_LOGD("service_name: %s\n", service_name);

	if (!can_post) {
		BROWSER_LOGD("cannot post to : [%s] in browser\n", service_name);
		if (service_name)
			free(service_name);
		service_name = NULL;
		return false;
	} else {
		account_get_package_name(account, &lib_path);
		BROWSER_LOGD("lib_path: %s\n", lib_path);

		account_get_icon_path(account, &icon_path);
		BROWSER_LOGD("icon_path: %s\n", icon_path);

		if (icon_path && strlen(icon_path) > 0) {
			BROWSER_LOGD("icon_path: %s\n", icon_path);
			Evas_Object *icon = elm_icon_add(common_view->m_share_popup);
			if (icon) {
				elm_icon_file_set(icon, icon_path, NULL);
				BROWSER_LOGD("icon_path: %s\n", icon_path);
				common_view->m_sns_icon_list.push_back(icon);
			}
		}

		if (service_name && strlen(service_name) && lib_path && strlen(lib_path)) {
			common_view->m_sns_path_list.push_back(std::string(lib_path));
			common_view->m_sns_name_list.push_back(std::string(service_name));
		}
	}
	if (service_name)
		free(service_name);
	service_name = NULL;

	if (icon_path)
		free(icon_path);
	icon_path = NULL;

	if (lib_path)
		free(lib_path);
	lib_path= NULL;

	return true;
}

bool Browser_Common_View::__get_post_capability_cb(account_capability_type_e type,
						account_capability_state_e state, void *data)
{
	Eina_Bool *can_post = (Eina_Bool *)data;
	if (!can_post) {
		BROWSER_LOGD("unable to post");
		return false;
	}

	if (ACCOUNT_CAPABILITY_STATUS_POST != type)
		return true;

	if (ACCOUNT_CAPABILITY_DISABLED == state)
		return true;

	*can_post = EINA_TRUE;

	return true;
}

Eina_Bool Browser_Common_View::_send_via_email(std::string url, Eina_Bool attach_file)
{
	BROWSER_LOGD("[%s], url[%s]", __func__, url.c_str());
	if (url.empty()) {
		show_msg_popup(BR_STRING_EMPTY);
		return EINA_FALSE;
	}

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!service_handle) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (attach_file) {
		if (service_set_operation(service_handle, SERVICE_OPERATION_SEND) < 0) {
			BROWSER_LOGE("Fail to set service operation");
			service_destroy(service_handle);
		return EINA_FALSE;
	}

		if (service_set_uri(service_handle, url.c_str()) < 0) {
			BROWSER_LOGE("Fail to set uri");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	} else {
		if (service_set_operation(service_handle, SERVICE_OPERATION_SEND_TEXT) < 0) {
			BROWSER_LOGE("Fail to set service operation");
			service_destroy(service_handle);
			return EINA_FALSE;
		}

		if (strstr(url.c_str(), BROWSER_MAIL_TO_SCHEME)) {
			if (service_add_extra_data(service_handle, SERVICE_DATA_TO, url.c_str() + strlen(BROWSER_MAIL_TO_SCHEME)) < 0) {
				BROWSER_LOGE("Fail to set mailto data");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		} else {
			if (service_add_extra_data(service_handle, SERVICE_DATA_TEXT, url.c_str()) < 0) {
				BROWSER_LOGE("Fail to set extra data");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		}
	}

	if (service_set_package(service_handle, SEC_EMAIL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}
	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool Browser_Common_View::_add_to_contact(std::string number)
{
	if (number.empty()) {
		BROWSER_LOGE("number is null");
		return EINA_FALSE;
	}
	struct ug_cbs cbs = {0, };
	cbs.layout_cb = __ug_layout_cb;
	cbs.result_cb = NULL;//__ug_result_cb;
	cbs.destroy_cb = __ug_destroy_cb;
	cbs.priv = (void *)this;

	char *phone_number = (char *)strdup(number.c_str());
	if (!phone_number) {
		BROWSER_LOGE("strdup failed");
		return EINA_FALSE;
	}

	service_h data = NULL;
	service_create(&data);
	if (data == NULL) {
		BROWSER_LOGE("fail to service_create.");
		return EINA_FALSE;
	}
/*
type.
CT_UG_REQUEST_ADD = 21,
CT_UG_REQUEST_ADD_WITH_NUM = 22,
CT_UG_REQUEST_ADD_WITH_EMAIL = 23,
CT_UG_REQUEST_ADD_WITH_WEB = 24,
*/
	if (service_add_extra_data(data, "type", "22")) {
		BROWSER_LOGE("service_add_extra_data is failed.");
		service_destroy(data);
		return EINA_FALSE;
	}
	if (service_add_extra_data(data, "ct_num", number.c_str())) {
		BROWSER_LOGE("service_add_extra_data is failed.");
		service_destroy(data);
		return EINA_FALSE;
	}

	if (!ug_create(NULL, "contacts-details-efl", UG_MODE_FULLVIEW, data, &cbs))
		BROWSER_LOGE("ug_create is failed.");

	if (service_destroy(data))
		BROWSER_LOGE("service_destroy is failed.");

	free(phone_number);
}

Eina_Bool Browser_Common_View::_share_via_nfc(std::string url)
{
	BROWSER_LOGD("[%s]", __func__);
	if (url.empty()) {
		show_msg_popup(BR_STRING_EMPTY);
		return EINA_FALSE;
	}

	struct ug_cbs cbs = {0, };
	cbs.layout_cb = __ug_layout_cb;
	cbs.result_cb = NULL;//__ug_result_cb;
	cbs.destroy_cb = __ug_destroy_cb;
	cbs.priv = (void *)this;

	char *share_url = (char *)strdup(url.c_str());
	if (!share_url) {
		BROWSER_LOGE("strdup failed");
		return EINA_FALSE;
	}

	service_h data = NULL;
	service_create(&data);
	if (data == NULL) {
		BROWSER_LOGE("fail to service_create.");
		return EINA_FALSE;
	}
	if (service_add_extra_data(data, "count", "1")) {
		BROWSER_LOGE("service_add_extra_data is failed.");
		service_destroy(data);
		return EINA_FALSE;
	}
	if (service_add_extra_data(data, "request_type", "data_buffer")) {
		BROWSER_LOGE("service_add_extra_data is failed.");
		service_destroy(data);
		return EINA_FALSE;
	}
	if (service_add_extra_data(data, "request_data", share_url)) {
		BROWSER_LOGE("service_add_extra_data is failed.");
		service_destroy(data);

		free(share_url);
		return EINA_FALSE;
	}

	if(!ug_create(NULL, "share-nfc-efl", UG_MODE_FULLVIEW, data, &cbs))
		BROWSER_LOGE("ug_create is failed.");

	if (service_destroy(data))
		BROWSER_LOGE("service_destroy is failed.");

	free(share_url);

	return EINA_TRUE;
}

Eina_Bool Browser_Common_View::_show_share_popup(const char *url)
{
	BROWSER_LOGE("url=[%s]", url);
	if (!url || strlen(url) == 0) {
		BROWSER_LOGE("url is empty");
		return EINA_FALSE;
	}

	m_share_url = std::string(url);

	m_sns_path_list.clear();
	m_sns_name_list.clear();
	m_sns_icon_list.clear();

	m_share_popup = elm_popup_add(m_navi_bar);
	if (!m_share_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_share_popup, "menustyle");
	elm_object_part_text_set(m_share_popup, "title,text", BR_STRING_SHARE);
	evas_object_size_hint_weight_set(m_share_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_share_list = elm_list_add(m_share_popup);
	if (!m_share_list) {
		BROWSER_LOGE("elm_list_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_share_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_share_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_list_item_append(m_share_list, BR_STRING_MESSAGES, NULL, NULL, __send_via_message_cb, this);
	elm_list_item_append(m_share_list, BR_STRING_EMAIL, NULL, NULL, __send_via_email_cb, this);

	evas_object_show(m_share_list);

	Evas_Object *cancel_button = elm_button_add(m_share_popup);
	elm_object_text_set(cancel_button, BR_STRING_CANCEL);
	elm_object_part_content_set(m_share_popup, "button1", cancel_button);
	elm_object_style_set(cancel_button, "popup_button/default");
	evas_object_smart_callback_add(cancel_button, "clicked", __popup_response_cb, this);

	elm_object_content_set(m_share_popup, m_share_list);

	evas_object_show(m_share_popup);

	return EINA_TRUE;
}

void Browser_Common_View::__popup_response_cb(void* data, Evas_Object* obj, void* event_info)
{
	BROWSER_LOGD("%s, event_info=%d", __func__, (int)event_info);

	if (!data)
		return;

	Browser_Common_View *common_view = (Browser_Common_View *)data;
	if (common_view->m_share_popup) {
		evas_object_del(common_view->m_share_popup);
		common_view->m_share_popup = NULL;
	}
	if (common_view->m_share_list) {
		evas_object_del(common_view->m_share_list);
		common_view->m_share_list = NULL;
	}

	common_view->m_sns_name_list.clear();
	common_view->m_sns_path_list.clear();

}

char *Browser_Common_View::_trim(char *str)
{
	char *pos_bos = str;

	while(*pos_bos == ' ')
		pos_bos++;

	char *pos_eos = pos_bos + strlen(pos_bos) - 1;

	while((pos_eos >= str) && (*pos_eos == ' ')) {
		*pos_eos = '\0';
		pos_eos--;
	}

	return pos_bos;
}

#if defined(HORIZONTAL_UI)
Eina_Bool Browser_Common_View::is_landscape(void)
{
	app_device_orientation_e rotation_value = app_get_device_orientation();
	if (rotation_value == APP_DEVICE_ORIENTATION_0 || rotation_value == APP_DEVICE_ORIENTATION_180)
		return EINA_FALSE;
	else
		return EINA_TRUE;
}
#endif

/* set focus to edit field idler callback to show ime. */
/**
 * @brief Browser_Common_View::__set_focus_editfield_idler_cb 处理输入法回调
 * @param edit_field 要使用输入法的控件
 * @return 是否设置成功
 */
Eina_Bool Browser_Common_View::__set_focus_editfield_idler_cb(void *edit_field)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!edit_field)
		return ECORE_CALLBACK_CANCEL;
	elm_object_focus_set((Evas_Object *)edit_field, EINA_TRUE);
	elm_object_signal_emit((Evas_Object *)edit_field, "clicked", "elm");

	Evas_Object *entry = br_elm_editfield_entry_get((Evas_Object *)edit_field);
	elm_entry_cursor_end_set(entry);

	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool Browser_Common_View::_has_url_sheme(const char *url)
{
	if (url && strlen(url)
	    && (strstr(url, BROWSER_URL_SCHEME_CHECK) || strstr(url, BROWSER_MAIL_TO_SCHEME)))
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

void Browser_Common_View::__ug_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!priv || !ug)
		return;

	Browser_Common_View *common_view = (Browser_Common_View *)priv;
	Evas_Object *base = (Evas_Object*)ug_get_layout(ug);
	if (!base)
		return;

	common_view->m_ug = ug;

	Evas_Object *win = (Evas_Object *)ug_get_window();

	switch (mode) {
	case UG_MODE_FULLVIEW:
		evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);		
		elm_win_resize_object_add(win, base);
		evas_object_show(base);
		break;
	default:
		break;
	}
}

void Browser_Common_View::__ug_result_cb(ui_gadget_h ug, bundle *result, void *priv)
{
	if (!priv || !ug)
		return;
}

void Browser_Common_View::__ug_destroy_cb(ui_gadget_h ug, void *priv)
{
	if (!priv || !ug)
		return;

	if (ug_destroy(ug))
		BROWSER_LOGD("ug_destroy is failed.\n");
}

