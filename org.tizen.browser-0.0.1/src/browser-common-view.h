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


#ifndef BROWSER_COMMON_VIEW_H
#define BROWSER_COMMON_VIEW_H

#include "browser-config.h"
#include "browser-data-manager.h"
#include "browser-utility.h"

class Browser_Data_Manager;
class Browser_Class;
class Browser_Window;

class Browser_Common_View {
public:
	typedef enum _call_type {
		VOICE_CALL,
		VIDEO_CALL,
		CALL_UNKNOWN
	} call_type;

	Browser_Common_View(void);
	~Browser_Common_View(void);

	Eina_Bool init(void);

#if defined(HORIZONTAL_UI)
	Eina_Bool is_landscape(void);
#endif
	Browser_Data_Manager *get_data_manager(void) { return m_data_manager; }
	Browser_Class *get_browser(void) { return m_browser; }
	void show_msg_popup(const char *msg, int timeout = 3);
	void show_msg_popup(const char *title, const char *msg, int timeout);
	void show_notify_popup(const char *msg, int timeout = 0, Eina_Bool has_control_bar = EINA_FALSE);
	void hide_notify_popup(void);
	void show_notify_popup_layout(const char *msg, int timeout, Evas_Object *parent);
	void hide_notify_popup_layout(Evas_Object *parent);
	Eina_Bool find_word_with_text(const char *text_to_find);
protected:
	Evas_Object *_capture_snapshot(Browser_Window *window, float scale);
	void _set_navigationbar_title(const char *title);
	Eina_Bool _has_url_sheme(const char *url);
	Eina_Bool _show_share_popup(const char *url);
	Eina_Bool _launch_streaming_player(const char *url, const char *cookie = NULL);
	Eina_Bool _send_via_message(std::string url, std::string to, Eina_Bool attach_file = EINA_FALSE);
	Eina_Bool _send_via_email(std::string url, Eina_Bool attach_file = EINA_FALSE);
	Eina_Bool _share_via_nfc(std::string url);
	Eina_Bool _add_to_contact(std::string number);
	Eina_Bool _check_available_sns_account(void);
	Eina_Bool _post_to_sns(std::string sns_name, std::string url);
	Eina_Bool _get_available_sns_list(void);
	static bool __get_sns_list(account_h account, void *data);

	/* Elementary event callback functions */
	static void __popup_response_cb(void* data, Evas_Object* obj, void* event_info);
	static void __send_via_email_cb(void *data, Evas_Object *obj, void *event_info);
	static void __send_via_message_cb(void *data, Evas_Object *obj, void *event_info);
	static void __share_via_nfc_cb(void *data, Evas_Object *obj, void *event_info);
	static bool __check_available_sns_account_cb(account_h handle, void *data);
	static void __post_to_sns_cb(void *data, Evas_Object *obj, void *event_info);
	static bool __get_post_capability_cb(account_capability_type_e type,
					account_capability_state_e state, void *data);

	/* set focus to edit field idler callback to show ime. */
	static Eina_Bool __set_focus_editfield_idler_cb(void *edit_field);

	char *_trim(char *str);
	/* For ug effect. */
	static void __ug_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv);
	static void __ug_result_cb(ui_gadget_h ug, bundle *result, void *priv);
	static void __ug_destroy_cb(ui_gadget_h ug, void *priv);

	static Browser_Data_Manager *m_data_manager;
	static Evas_Object *m_win;
	static Evas_Object *m_navi_bar;
	static Evas_Object *m_bg;
	static Browser_Class *m_browser;

	std::vector<std::string> m_sns_name_list;
	std::vector<std::string> m_sns_path_list;
	std::vector<Evas_Object *> m_sns_icon_list;
private:
	Evas_Object *m_popup;
	Evas_Object *m_selection_info;
	Evas_Object *m_selection_info_layout;
	Evas_Object *m_selinfo_layout;
	Evas_Object *m_share_popup;
	Evas_Object *m_share_list;
	Evas_Object *m_call_confirm_popup;
	std::string m_share_url;
	std::string m_tel_number;
	call_type m_call_type;
	ui_gadget_h m_ug;
};
#endif /* BROWSER_COMMON_VIEW_H */

