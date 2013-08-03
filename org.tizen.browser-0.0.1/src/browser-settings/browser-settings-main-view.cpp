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


#include "browser-view.h"
#include "browser-data-manager.h"
#include "browser-history-db.h"
#include "browser-settings-clear-data-view.h"
#include "browser-settings-edit-homepage-view.h"
#include "browser-settings-main-view.h"
#include "browser-settings-website-setting.h"

#include "browser-geolocation-db.h"

#include "browser-settings-user-agent-view.h"

Browser_Settings_Main_View::Browser_Settings_Main_View(void)
:
	m_genlist(NULL)
	,m_back_button(NULL)
	,m_homepage_radio_group(NULL)
	,m_edit_homepage_view(NULL)
	,m_default_view_level_radio_group(NULL)
	,m_auto_save_id_pass_radio_group(NULL)
	,m_run_javascript_check(NULL)
	,m_display_images_check(NULL)
	,m_accept_cookies_check(NULL)
	,m_enable_location_check(NULL)
	,m_clear_location_confirm_popup(NULL)
	,m_clear_cache_confirm_popup(NULL)
	,m_clear_history_confirm_popup(NULL)
	,m_clear_data_view(NULL)
	,m_block_popup_check(NULL)
	,m_reset_confirm_popup(NULL)
	,m_navi_it(NULL)
	,m_show_security_warnings_check(NULL)
	,m_clear_all_cookies_data_confirm_popup(NULL)
	,m_website_setting(NULL)
	,m_user_agent_view(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Settings_Main_View::~Browser_Settings_Main_View(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_edit_homepage_view) {
		delete m_edit_homepage_view;
		m_edit_homepage_view = NULL;
	}
	if (m_clear_data_view) {
		delete m_clear_data_view;
		m_clear_data_view = NULL;
	}
	if (m_website_setting) {
		delete m_website_setting;
		m_website_setting = NULL;
	}

	if (m_user_agent_view) {
		delete m_user_agent_view;
		m_user_agent_view = NULL;
	}

	evas_object_smart_callback_del(m_navi_bar, "transition,finished", __naviframe_pop_finished_cb);
}

Eina_Bool Browser_Settings_Main_View::init(void)
{
	BROWSER_LOGD("[%s]", __func__);
	return _create_main_layout();
}

void Browser_Settings_Main_View::__back_button_clicked_cb(void *data, Evas_Object* obj,
									void* event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (elm_naviframe_bottom_item_get(m_navi_bar) != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);

	m_data_manager->get_browser_view()->return_to_browser_view();
}

void Browser_Settings_Main_View::__naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	if (main_view->m_navi_it != elm_naviframe_top_item_get(m_navi_bar))
		return;

	if (main_view->m_edit_homepage_view) {
		delete main_view->m_edit_homepage_view;
		main_view->m_edit_homepage_view = NULL;

		/* If back from edit user homepage, update the url string. */
		char* homepage = NULL;
		if (br_preference_get_str(HOMEPAGE_KEY, &homepage) == false) {
			BROWSER_LOGE("failed to get %s preferenct\n", HOMEPAGE_KEY);
			return;
		}
		if (homepage) {
			if (!strncmp(homepage, USER_HOMEPAGE,
						strlen(USER_HOMEPAGE)))
				elm_radio_value_set(main_view->m_homepage_radio_group, 2);

			free(homepage);

			elm_genlist_realized_items_update(main_view->m_genlist);
		}
	}

	if (main_view->m_edit_homepage_view) {
		delete main_view->m_edit_homepage_view;
		main_view->m_edit_homepage_view = NULL;
	}

	if (main_view->m_clear_data_view) {
		delete main_view->m_clear_data_view;
		main_view->m_clear_data_view = NULL;
	}
	if (main_view->m_user_agent_view) {
		delete main_view->m_user_agent_view;
		main_view->m_user_agent_view = NULL;
	}

	BROWSER_LOGD("main_view->m_website_setting = %d", main_view->m_website_setting);
	if (main_view->m_website_setting) {
		delete main_view->m_website_setting;
		main_view->m_website_setting = NULL;
	}
	/*Disable website setting menu when there is no such data */
	Browser_Geolocation_DB *geo_db = main_view->get_data_manager()->get_geolocation_db();
	if ((geo_db->get_geolocation_info_count()) == 0) {
		elm_object_item_disabled_set(main_view->m_website_setting_callback_data.it, EINA_TRUE);
		Ewk_Context *context = ewk_context_default_get();
		ewk_context_application_cache_origins_get(context, main_view->__application_cache_origin_get_cb, main_view);
		ewk_context_web_storage_origins_get(context, main_view->__web_storage_origin_get_cb, main_view);
		ewk_context_web_database_origins_get(context, main_view->__web_database_origin_get_cb, main_view);
	}
}

Eina_Bool Browser_Settings_Main_View::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_genlist = _create_content_genlist();
	if (!m_genlist) {
		BROWSER_LOGE("_create_content_genlist failed");
		return EINA_FALSE;
	}

	evas_object_show(m_genlist);

	m_back_button = elm_button_add(m_genlist);
	if (!m_back_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_back_button, "browser/bookmark_controlbar_back");
	evas_object_show(m_back_button);
	evas_object_smart_callback_add(m_back_button, "clicked", __back_button_clicked_cb, this);

	m_homepage_radio_group = elm_radio_add(m_genlist);
	if (!m_homepage_radio_group) {
		BROWSER_LOGE("elm_radio_add failed.");
		return EINA_FALSE;
	}
	elm_radio_state_value_set(m_homepage_radio_group, -1);

	m_default_view_level_radio_group = elm_radio_add(m_genlist);
	if (!m_default_view_level_radio_group) {
		BROWSER_LOGE("elm_radio_add failed.");
		return EINA_FALSE;
	}
	elm_radio_state_value_set(m_default_view_level_radio_group, -1);

	m_auto_save_id_pass_radio_group = elm_radio_add(m_genlist);
	if (!m_auto_save_id_pass_radio_group) {
		BROWSER_LOGE("elm_radio_add failed.");
		return EINA_FALSE;
	}
	elm_radio_state_value_set(m_auto_save_id_pass_radio_group, -1);
	m_navi_it = elm_naviframe_item_push(m_navi_bar, BR_STRING_INTERNET,
							m_back_button, NULL, m_genlist, "browser_titlebar");

	evas_object_smart_callback_add(m_navi_bar, "transition,finished", __naviframe_pop_finished_cb, this);

	return EINA_TRUE;
}

Evas_Object *Browser_Settings_Main_View::__genlist_icon_get(void *data, Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);

	if (type == BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE
#if defined(FEATURE_MOST_VISITED_SITES)
	    || type == BR_HOMEPAGE_SUBMENU_MOST_VISITED_SITES
#endif
	    || type == BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE
	    || type == BR_HOMEPAGE_SUBMENU_CURRENT_PAGE
	    || type == BR_HOMEPAGE_SUBMENU_EMPTY_PAGE) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			Evas_Object *radio_button = elm_radio_add(obj);
			if (radio_button) {
#if defined(FEATURE_MOST_VISITED_SITES)
				if (type == BR_HOMEPAGE_SUBMENU_MOST_VISITED_SITES)
					elm_radio_state_value_set(radio_button, 0);
				else
#endif
				if (type == BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE)
					elm_radio_state_value_set(radio_button, 1);
				else if (type == BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE)
					elm_radio_state_value_set(radio_button, 2);
				else if (type == BR_HOMEPAGE_SUBMENU_CURRENT_PAGE)
					elm_radio_state_value_set(radio_button, 3);
				else if (type == BR_HOMEPAGE_SUBMENU_EMPTY_PAGE)
					elm_radio_state_value_set(radio_button, 4);

				elm_radio_group_add(radio_button, main_view->m_homepage_radio_group);
				evas_object_propagate_events_set(radio_button, EINA_FALSE);

				evas_object_smart_callback_add(radio_button, "changed",
							__homepage_sub_item_clicked_cb, data);

				char* homepage = NULL;
				if (br_preference_get_str(HOMEPAGE_KEY, &homepage) == false) {
					BROWSER_LOGE("failed to get %s preference\n", HOMEPAGE_KEY);
					return NULL;
				}

				if (!homepage) {
#if defined(FEATURE_MOST_VISITED_SITES)
					br_preference_set_str(HOMEPAGE_KEY, MOST_VISITED_SITES);
					homepage = strdup(MOST_VISITED_SITES);
#else
					br_preference_set_str(HOMEPAGE_KEY, USER_HOMEPAGE);
					homepage = strdup(USER_HOMEPAGE);
#endif
				}

				if (!homepage) {
					BROWSER_LOGE("strdup failed");
					return NULL;
				}

#if defined(FEATURE_MOST_VISITED_SITES)
				if (!strncmp(homepage, MOST_VISITED_SITES,
							strlen(MOST_VISITED_SITES)))
					elm_radio_value_set(main_view->m_homepage_radio_group, 0);
				else
#endif
				if (!strncmp(homepage, RECENTLY_VISITED_SITE,
							strlen(RECENTLY_VISITED_SITE)))
					elm_radio_value_set(main_view->m_homepage_radio_group, 1);
				else if (!strncmp(homepage, USER_HOMEPAGE,
							strlen(USER_HOMEPAGE)))
					elm_radio_value_set(main_view->m_homepage_radio_group, 2);
				else if (!strncmp(homepage, EMPTY_PAGE,
							strlen(EMPTY_PAGE)))
					elm_radio_value_set(main_view->m_homepage_radio_group, 4);
				else
					elm_radio_value_set(main_view->m_homepage_radio_group, 3);

				free(homepage);
			}

			return radio_button;
		}
	} else if (type == BR_CONTENT_SUBMENU_FIT_TO_WIDTH
	     || type == BR_CONTENT_SUBMENU_READABLE) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			Evas_Object *radio_button = elm_radio_add(obj);
			if (radio_button) {
				if (type == BR_CONTENT_SUBMENU_FIT_TO_WIDTH)
					elm_radio_state_value_set(radio_button, 0);
				else
					elm_radio_state_value_set(radio_button, 1);

				elm_radio_group_add(radio_button, main_view->m_default_view_level_radio_group);

				char *view_level = NULL;
				if (br_preference_get_str(DEFAULT_VIEW_LEVEL_KEY, &view_level) == false) {
					BROWSER_LOGE("failed to get %s prefernece", DEFAULT_VIEW_LEVEL_KEY);
					return NULL;
				}
				if (!view_level) {
					br_preference_set_str(DEFAULT_VIEW_LEVEL_KEY, READABLE);
					view_level = strdup(BR_STRING_READABLE);
				}
				if (!view_level) {
					BROWSER_LOGE("strdup failed");
					return NULL;
				}

				if (!strncmp(view_level, FIT_TO_WIDTH, strlen(FIT_TO_WIDTH)))
					elm_radio_value_set(main_view->m_default_view_level_radio_group, 0);
				else
					elm_radio_value_set(main_view->m_default_view_level_radio_group, 1);

				free(view_level);
			}

			return radio_button;
		}
	} else if (type == BR_CONTENT_MENU_RUN_JAVASCRIPT) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			main_view->m_run_javascript_check = elm_check_add(obj);
			if (main_view->m_run_javascript_check) {
				elm_object_style_set(main_view->m_run_javascript_check, "on&off");
				evas_object_smart_callback_add(main_view->m_run_javascript_check, "changed",
						__run_javascript_check_changed_cb, main_view->m_run_javascript_check);

				bool run_javascript = 1;
				br_preference_get_bool(RUN_JAVASCRIPT_KEY, &run_javascript);
				elm_check_state_set(main_view->m_run_javascript_check, run_javascript);
				evas_object_propagate_events_set(main_view->m_run_javascript_check, EINA_FALSE);
			}
			return main_view->m_run_javascript_check;
		}
	} else if (type == BR_CONTENT_MENU_DISPLAY_IMAGES) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			main_view->m_display_images_check = elm_check_add(obj);
			if (main_view->m_display_images_check) {
				elm_object_style_set(main_view->m_display_images_check, "on&off");
				evas_object_smart_callback_add(main_view->m_display_images_check, "changed",
						__display_images_check_changed_cb, main_view->m_display_images_check);

				bool display_images = 1;
				br_preference_get_bool(DISPLAY_IMAGES_KEY, &display_images);
				elm_check_state_set(main_view->m_display_images_check, display_images);
				evas_object_propagate_events_set(main_view->m_display_images_check, EINA_FALSE);
			}
			return main_view->m_display_images_check;
		}
	} else if (type == BR_CONTENT_MENU_BLOCK_POPUP) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			main_view->m_block_popup_check = elm_check_add(obj);
			if (main_view->m_block_popup_check) {
				elm_object_style_set(main_view->m_block_popup_check, "on&off");
				evas_object_smart_callback_add(main_view->m_block_popup_check, "changed",
						__block_popup_check_changed_cb, main_view->m_block_popup_check);

				bool block_popup = 1;
				br_preference_get_bool(BLOCK_POPUP_KEY, &block_popup);
				elm_check_state_set(main_view->m_block_popup_check, block_popup);
				evas_object_propagate_events_set(main_view->m_block_popup_check, EINA_FALSE);
			}
			return main_view->m_block_popup_check;
		}
	} else if (type == BR_PRIVATE_MENU_SHOW_SECURITY_WARNINGS) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			main_view->m_show_security_warnings_check = elm_check_add(obj);
			if (main_view->m_show_security_warnings_check) {
				elm_object_style_set(main_view->m_show_security_warnings_check, "on&off");
				evas_object_smart_callback_add(main_view->m_show_security_warnings_check, "changed",
						__show_security_warnings_check_changed_cb, main_view->m_show_security_warnings_check);

				bool show_security_warnings = 1;
				br_preference_get_bool(SHOW_SECURITY_WARNINGS_KEY, &show_security_warnings);
				elm_check_state_set(main_view->m_show_security_warnings_check, show_security_warnings);
				evas_object_propagate_events_set(main_view->m_show_security_warnings_check, EINA_FALSE);
			}
			return main_view->m_show_security_warnings_check;
		}
	} else if (type == BR_PRIVACY_MENU_ACCEPT_COOKIES) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			main_view->m_accept_cookies_check = elm_check_add(obj);
			if (main_view->m_accept_cookies_check) {
				elm_object_style_set(main_view->m_accept_cookies_check, "on&off");
				evas_object_smart_callback_add(main_view->m_accept_cookies_check, "changed",
						__accept_cookies_check_changed_cb, main_view->m_accept_cookies_check);

				bool accept_cookies = 1;
				br_preference_get_bool(ACCEPT_COOKIES_KEY, &accept_cookies);
				elm_check_state_set(main_view->m_accept_cookies_check, accept_cookies);
				evas_object_propagate_events_set(main_view->m_accept_cookies_check, EINA_FALSE);
			}
			return main_view->m_accept_cookies_check;
		}
	} else if (type == BR_PRIVACY_MENU_ENABLE_LOCATION) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			main_view->m_enable_location_check = elm_check_add(obj);
			if (main_view->m_enable_location_check) {
				elm_object_style_set(main_view->m_enable_location_check, "on&off");
				evas_object_smart_callback_add(main_view->m_enable_location_check, "changed",
						__enable_location_check_changed_cb, main_view);

				bool enable_location = 1;
				br_preference_get_bool(ENABLE_LOCATION_KEY, &enable_location);
				elm_check_state_set(main_view->m_enable_location_check, enable_location);
				evas_object_propagate_events_set(main_view->m_enable_location_check, EINA_FALSE);
			}
			return main_view->m_enable_location_check;
		}
	}
	return NULL;
}

void Browser_Settings_Main_View::__run_javascript_check_changed_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	const char *key = RUN_JAVASCRIPT_KEY;
	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	br_preference_set_bool(key, state);
}

void Browser_Settings_Main_View::__display_images_check_changed_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	const char *key = DISPLAY_IMAGES_KEY;
	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	br_preference_set_bool(key, state);
}

void Browser_Settings_Main_View::__block_popup_check_changed_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	const char *key = BLOCK_POPUP_KEY;
	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	br_preference_set_bool(key, state);
}

void Browser_Settings_Main_View::__show_security_warnings_check_changed_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	const char *key = SHOW_SECURITY_WARNINGS_KEY;
	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	br_preference_set_bool(key, state);
}

void Browser_Settings_Main_View::__accept_cookies_check_changed_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	const char *key = ACCEPT_COOKIES_KEY;
	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	br_preference_set_bool(key, state);
}

void Browser_Settings_Main_View::__enable_location_check_changed_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!data) {
		BROWSER_LOGE("data is null");
		return;
	}

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	const char *key = ENABLE_LOCATION_KEY;

	Eina_Bool state = elm_check_state_get(obj);
	br_preference_set_bool(key, state);

	elm_object_item_disabled_set(main_view->m_clear_location_access_callback_data.it, !state);
}

char *Browser_Settings_Main_View::__genlist_label_get(void *data, Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);

	if (type == BR_HOMEPAGE_TITLE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_HOMEPAGE);
	} else if (type == BR_HOMEPAGE_MENU) {
		if (!strncmp(part, "elm.text.1", strlen("elm.text.1")))
			return strdup(BR_STRING_HOMEPAGE);
		else if (!strncmp(part, "elm.text.2", strlen("elm.text.2"))) {
			char *homepage = NULL;
			if (br_preference_get_str(HOMEPAGE_KEY, &homepage) == false) {
				BROWSER_LOGE("failed to get %s preference\n", HOMEPAGE_KEY);
				return NULL;
			}
			BROWSER_LOGD("homepage = %s", homepage);
			if (homepage) {
#if defined(FEATURE_MOST_VISITED_SITES)
				if (!strncmp(homepage, MOST_VISITED_SITES, strlen(MOST_VISITED_SITES))) {
					free(homepage);
					return strdup(BR_STRING_MOST_VISITED_SITES);
				} else
#endif
				if (!strncmp(homepage, RECENTLY_VISITED_SITE,
							strlen(RECENTLY_VISITED_SITE))) {
					free(homepage);
					return strdup(BR_STRING_RECENTLY_VISITED_SITE);
				} else if (!strncmp(homepage, EMPTY_PAGE,
					strlen(EMPTY_PAGE))) {
					free(homepage);
					return strdup(BR_STRING_BLANK_PAGE);
				} else {
					free(homepage);
					return strdup(BR_STRING_USER_HOMEPAGE);
				}
			}
			return NULL;
		}
	}
#if defined(FEATURE_MOST_VISITED_SITES)
	else if (type == BR_HOMEPAGE_SUBMENU_MOST_VISITED_SITES) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_MOST_VISITED_SITES);
	}
#endif
	else if (type == BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_RECENTLY_VISITED_SITE);
	} else if (type == BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE) {
		if (!strncmp(part, "elm.text", strlen("elm.text"))) {
			char *homepage = NULL;
			if (br_preference_get_str(USER_HOMEPAGE_KEY, &homepage) == false) {
				BROWSER_LOGE("failed to get %s preference\n", USER_HOMEPAGE_KEY);
				return NULL;
			}
			return homepage;
		}
	} else if (type == BR_HOMEPAGE_SUBMENU_CURRENT_PAGE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_CURRENT_PAGE);
	} else if (type == BR_HOMEPAGE_SUBMENU_EMPTY_PAGE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_BLANK_PAGE);
	} else if (type == BR_CONTENT_MENU_DEFAULT_VIEW_LEVEL) {
		if (!strncmp(part, "elm.text.1", strlen("elm.text.1")))
			return strdup(BR_STRING_DEFAULT_VIEW_LEVEL);
		else if (!strncmp(part, "elm.text.2", strlen("elm.text.2"))) {
			char *view_level = NULL;
			if (br_preference_get_str(DEFAULT_VIEW_LEVEL_KEY, &view_level) == false) {
				BROWSER_LOGE("failed to get %s preference\n", DEFAULT_VIEW_LEVEL_KEY);
				return NULL;
			}
			if (view_level) {
				if (!strncmp(view_level, FIT_TO_WIDTH, strlen(FIT_TO_WIDTH))) {
					free(view_level);
					return strdup(BR_STRING_FIT_TO_WIDTH);
				} else {
					free(view_level);
					return strdup(BR_STRING_READABLE);
				}
			}
			return NULL;
		}
	} else if (type == BR_CONTENT_SUBMENU_FIT_TO_WIDTH) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_FIT_TO_WIDTH);
	} else if (type == BR_CONTENT_SUBMENU_READABLE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_READABLE);
	} else if (type == BR_CONTENT_TITLE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_CONTENT_SETTINGS);
	} else if (type == BR_CONTENT_MENU_RUN_JAVASCRIPT) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_RUN_JAVASCRIPT);
	} else if (type == BR_CONTENT_MENU_DISPLAY_IMAGES) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_SHOW_IMAGES);
	} else if (type == BR_CONTENT_MENU_BLOCK_POPUP) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_BLOCK_POPUP);
	} else if (type == BR_PRIVACY_TITLE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_PRIVACY_AND_SECURTY);
	} else if (type == BR_PRIVATE_MENU_CLEAR_CACHE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_CLEAR_CACHE);
	} else if (type == BR_PRIVATE_MENU_CLEAR_HISTORY) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_CLEAR_HISTORY);
	} else if (type == BR_PRIVATE_MENU_SHOW_SECURITY_WARNINGS) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_SHOW_SECURITY_WARNINGS);
	} else if (type == BR_PRIVACY_MENU_ACCEPT_COOKIES) {
		if (!strncmp(part, "elm.text.2", strlen("elm.text.2")))
			return strdup(BR_STRING_COOKIES);
		else if (!strncmp(part, "elm.text.1", strlen("elm.text.1")))
			return strdup(BR_STRING_ACCEPT_COOKIES);
	} else if (type == BR_PRIVATE_MENU_CLEAR_ALL_COOKIE_DATA) {
		if (!strncmp(part, "elm.text.2", strlen("elm.text.2")))
			return strdup(BR_STRING_COOKIES);
		else if (!strncmp(part, "elm.text.1", strlen("elm.text.1")))
			return strdup(BR_STRING_CLEAR_ALL_COOKIE_DATA);
	} else if (type == BR_PRIVACY_SUBMENU_ALWAYS_ASK) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_ALWAYS_ASK);
	} else if (type == BR_PRIVACY_SUBMENU_ALWAYS_ON) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_ON);
	} else if (type == BR_PRIVACY_SUBMENU_ALWAYS_OFF) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_OFF);
	} else if (type == BR_PRIVACY_MENU_CLEAR_PRIVATE_DATA) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_DELETE_BROWSING_DATA);
	} else if (type == BR_PRIVACY_MENU_ENABLE_LOCATION) {
		if (!strncmp(part, "elm.text.2", strlen("elm.text.2")))
			return strdup(BR_STRING_LOCATION);
		else if (!strncmp(part, "elm.text.1", strlen("elm.text.1")))
			return strdup(BR_STRING_ENABLE_LOCATION);
	} else if (type == BR_PRIVACY_MENU_CLEAR_LOCATION_ACCESS) {
		if (!strncmp(part, "elm.text.2", strlen("elm.text.2")))
			return strdup(BR_STRING_LOCATION);
		else if (!strncmp(part, "elm.text.1", strlen("elm.text.1")))
			return strdup(BR_STRING_CLEAR_LOCATION_ACCESS);
	} else if (type == BR_PRIVACY_WEBSITE_SETTING) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_WEBSITE_SETTINGS);
	} else if (type == BR_DEBUG_TITLE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_DEVELOPER_MODE);
	} else if (type == BR_MENU_USER_AGENT) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_USER_AGENT);
	} else if (type == BR_MENU_RESET_TO_DEFAULT) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_RESET_TO_DEFAULT);
	}

	return NULL;
}

void Browser_Settings_Main_View::__homepage_sub_item_clicked_cb(void *data, Evas_Object *obj,
											void *event_info)
{
	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);

	int radio_value = 0;
#if defined(FEATURE_MOST_VISITED_SITES)
	if (type == BR_HOMEPAGE_SUBMENU_MOST_VISITED_SITES)
		radio_value = 0;
	else
#endif
	if (type == BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE)
		radio_value = 1;
	else if (type == BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE)
		radio_value = 2;
	else if (type == BR_HOMEPAGE_SUBMENU_CURRENT_PAGE)
		radio_value = 3;
	else if (type == BR_HOMEPAGE_SUBMENU_EMPTY_PAGE)
		radio_value = 4;

	elm_radio_value_set(main_view->m_homepage_radio_group, radio_value);

	if (radio_value == 3)
		elm_radio_value_set(main_view->m_homepage_radio_group, 2);

#if defined(FEATURE_MOST_VISITED_SITES)
	if (radio_value == 0)
		br_preference_set_str(HOMEPAGE_KEY, MOST_VISITED_SITES);
	else
#endif
	if (radio_value == 1)
		br_preference_set_str(HOMEPAGE_KEY, RECENTLY_VISITED_SITE);
	else if (radio_value == 2) {
		main_view->m_edit_homepage_view = new(nothrow) Browser_Settings_Edit_Homepage_View(main_view);
		if (!main_view->m_edit_homepage_view) {
			BROWSER_LOGE("new Browser_Settings_Edit_Homepage_View failed");
			return;
		}
		if (!main_view->m_edit_homepage_view->init()) {
			BROWSER_LOGE("m_edit_homepage_view failed");
			delete main_view->m_edit_homepage_view;
			main_view->m_edit_homepage_view = NULL;
		}
	} else if (radio_value == 3) {
		br_preference_set_str(USER_HOMEPAGE_KEY, m_data_manager->get_browser_view()->get_url().c_str());
		br_preference_set_str(HOMEPAGE_KEY, USER_HOMEPAGE);

		elm_genlist_item_update(main_view->m_user_homepage_item_callback_data.it);
	} else if (radio_value == 4) {
		br_preference_set_str(HOMEPAGE_KEY, EMPTY_PAGE);
	}

	elm_genlist_item_update(main_view->m_homepage_item_callback_data.it);

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

void Browser_Settings_Main_View::__default_view_level_sub_item_clicked_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);

	int radio_value = 0;
	if (type == BR_CONTENT_SUBMENU_FIT_TO_WIDTH)
		radio_value = 0;
	else if (type == BR_CONTENT_SUBMENU_READABLE)
		radio_value = 1;

	if (elm_radio_value_get(main_view->m_default_view_level_radio_group) != radio_value) {
		elm_radio_value_set(main_view->m_default_view_level_radio_group, radio_value);
		if (radio_value == 0)
			br_preference_set_str(DEFAULT_VIEW_LEVEL_KEY, FIT_TO_WIDTH);
		else
			br_preference_set_str(DEFAULT_VIEW_LEVEL_KEY, READABLE);

		elm_genlist_item_update(main_view->m_defailt_view_level_item_callback_data.it);
	}

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

void Browser_Settings_Main_View::__expandable_icon_clicked_cb(void *data, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);
	Elm_Object_Item *it = callback_data->it;

	Evas_Object *genlist = elm_object_item_widget_get(it);
	Eina_Bool is_expanded = elm_genlist_item_expanded_get(it);
	if (is_expanded) {
		elm_genlist_item_expanded_set(it, EINA_FALSE);
		elm_genlist_item_subitems_clear(it);

		main_view->m_user_homepage_item_callback_data.it = NULL;
	} else {
		elm_genlist_item_expanded_set(it, EINA_TRUE);
		if (it == main_view->m_homepage_item_callback_data.it) {
			BROWSER_LOGD("__expandable_icon_clicked_cb - homepage");
			/* If homepage menu. */
			main_view->m_current_page_item_callback_data.type = BR_HOMEPAGE_SUBMENU_CURRENT_PAGE;
			main_view->m_current_page_item_callback_data.user_data = main_view;
			main_view->m_current_page_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_current_page_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __homepage_sub_item_clicked_cb,
										&(main_view->m_current_page_item_callback_data));

			main_view->m_empty_page_item_callback_data.type = BR_HOMEPAGE_SUBMENU_EMPTY_PAGE;
			main_view->m_empty_page_item_callback_data.user_data = main_view;
			main_view->m_empty_page_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_empty_page_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __homepage_sub_item_clicked_cb,
										&(main_view->m_empty_page_item_callback_data));

			main_view->m_recently_visited_item_callback_data.type = BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE;
			main_view->m_recently_visited_item_callback_data.user_data = main_view;
			main_view->m_recently_visited_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_recently_visited_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __homepage_sub_item_clicked_cb,
										&(main_view->m_recently_visited_item_callback_data));

#if defined(FEATURE_MOST_VISITED_SITES)
			main_view->m_most_visited_item_callback_data.type = BR_HOMEPAGE_SUBMENU_MOST_VISITED_SITES;
			main_view->m_most_visited_item_callback_data.user_data = main_view;
			main_view->m_most_visited_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_most_visited_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __homepage_sub_item_clicked_cb,
										&(main_view->m_most_visited_item_callback_data));
#endif

			main_view->m_user_homepage_item_callback_data.type = BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE;
			main_view->m_user_homepage_item_callback_data.user_data = main_view;
			main_view->m_user_homepage_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_user_homepage_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __homepage_sub_item_clicked_cb,
										&(main_view->m_user_homepage_item_callback_data));

			if (m_data_manager->get_browser_view()->get_url().empty())
				elm_object_item_disabled_set(main_view->m_current_page_item_callback_data.it, EINA_TRUE);
		} else if (it == main_view->m_defailt_view_level_item_callback_data.it) {
			BROWSER_LOGD("__expandable_icon_clicked_cb - default view");
			main_view->m_fit_to_width_item_callback_data.type = BR_CONTENT_SUBMENU_FIT_TO_WIDTH;
			main_view->m_fit_to_width_item_callback_data.user_data = main_view;
			main_view->m_fit_to_width_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_fit_to_width_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __default_view_level_sub_item_clicked_cb,
										&(main_view->m_fit_to_width_item_callback_data));

			main_view->m_readable_item_callback_data.type = BR_CONTENT_SUBMENU_READABLE;
			main_view->m_readable_item_callback_data.user_data = main_view;
			main_view->m_readable_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_readable_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __default_view_level_sub_item_clicked_cb,
										&(main_view->m_readable_item_callback_data));
		}
	}

	elm_genlist_item_selected_set(it, EINA_FALSE);
}

void Browser_Settings_Main_View::__on_off_check_clicked_cb(void *data, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);

	Eina_Bool state = EINA_TRUE;
	if (type == BR_CONTENT_MENU_RUN_JAVASCRIPT) {
		state = elm_check_state_get(main_view->m_run_javascript_check);
		elm_check_state_set(main_view->m_run_javascript_check, !state);
		__run_javascript_check_changed_cb(main_view->m_run_javascript_check, NULL, NULL);
	} else if (type == BR_CONTENT_MENU_DISPLAY_IMAGES) {
		state = elm_check_state_get(main_view->m_display_images_check);
		elm_check_state_set(main_view->m_display_images_check, !state);
		__display_images_check_changed_cb(main_view->m_display_images_check, NULL, NULL);
	} else if (type == BR_CONTENT_MENU_BLOCK_POPUP) {
		state = elm_check_state_get(main_view->m_block_popup_check);
		elm_check_state_set(main_view->m_block_popup_check, !state);
		__block_popup_check_changed_cb(main_view->m_block_popup_check, NULL, NULL);
	} else if (type == BR_PRIVATE_MENU_SHOW_SECURITY_WARNINGS) {
		state = elm_check_state_get(main_view->m_show_security_warnings_check);
		elm_check_state_set(main_view->m_show_security_warnings_check, !state);
		__show_security_warnings_check_changed_cb(main_view->m_show_security_warnings_check, NULL, NULL);
	} else if (type == BR_PRIVACY_MENU_ACCEPT_COOKIES) {
		state = elm_check_state_get(main_view->m_accept_cookies_check);
		elm_check_state_set(main_view->m_accept_cookies_check, !state);
		__accept_cookies_check_changed_cb(main_view->m_accept_cookies_check, NULL, NULL);
	} else if (type == BR_PRIVACY_MENU_ENABLE_LOCATION) {
		state = elm_check_state_get(main_view->m_enable_location_check);
		elm_check_state_set(main_view->m_enable_location_check, !state);
		__enable_location_check_changed_cb(main_view, main_view->m_enable_location_check, NULL);
	}
	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

Eina_Bool Browser_Settings_Main_View::_call_user_agent(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_user_agent_view)
		delete m_user_agent_view;
	m_user_agent_view = new(nothrow) Browser_Settings_User_Agent_View(this);
	if (!m_user_agent_view) {
		BROWSER_LOGE("new Browser_Settings_User_Agent_View failed");
		return EINA_FALSE;
	}
	if (!m_user_agent_view->init()) {
		BROWSER_LOGE("m_user_agent_view->init failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

void Browser_Settings_Main_View::__genlist_item_clicked_cb(void *data, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);

	if (type == BR_PRIVACY_MENU_CLEAR_PRIVATE_DATA) {
		main_view->m_clear_data_view = new(nothrow) Browser_Settings_Clear_Data_View(main_view);
		if (!main_view->m_clear_data_view) {
			BROWSER_LOGE("new Browser_Settings_Clear_Data_View failed");
			return;
		}
		if (!main_view->m_clear_data_view->init()) {
			BROWSER_LOGE("m_clear_data_view->init failed");
			delete main_view->m_clear_data_view;
			main_view->m_clear_data_view = NULL;
			return;
		}
	} else if (type == BR_PRIVATE_MENU_CLEAR_CACHE) {
		main_view->_show_clear_cache_confirm_popup();
	} else if (type == BR_PRIVATE_MENU_CLEAR_HISTORY) {
		main_view->_show_clear_history_confirm_popup();
	} else if (type == BR_PRIVATE_MENU_CLEAR_ALL_COOKIE_DATA) {
		main_view->_show_clear_all_cookie_data_confirm_popup();
	} else if (type == BR_PRIVACY_WEBSITE_SETTING) {
		BROWSER_LOGD("Web sites setting");
		if (main_view->m_website_setting)
			delete main_view->m_website_setting;
		main_view->m_website_setting = new(nothrow) Browser_Settings_Website_Setting(main_view);
		if (!main_view->m_website_setting) {
			BROWSER_LOGE("new Browser_Settings_Website_Setting failed");
			return;
		}
		if (!main_view->m_website_setting->init()) {
			BROWSER_LOGE("m_website_setting->init failed");
			delete main_view->m_website_setting;
			main_view->m_website_setting = NULL;
			return;
		}
	} else if (type == BR_MENU_USER_AGENT) {
		if (!main_view->_call_user_agent())
			BROWSER_LOGE("_call_user_agent failed");
	} else if (type == BR_MENU_RESET_TO_DEFAULT) {
		main_view->_show_reset_confirm_popup();
	} else if (type == BR_PRIVACY_MENU_CLEAR_LOCATION_ACCESS) {
		main_view->_show_clear_location_confirm_popup();
	}

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

void Browser_Settings_Main_View::_reset_settings(void)
{
	BROWSER_LOGD("[%s]", __func__);
#if defined(FEATURE_MOST_VISITED_SITES)
	br_preference_set_str(HOMEPAGE_KEY, MOST_VISITED_SITES);
#else
	br_preference_set_str(HOMEPAGE_KEY, USER_HOMEPAGE);
#endif
	br_preference_set_str(USER_HOMEPAGE_KEY, BROWSER_DEFAULT_USER_HOMEPAGE);
	br_preference_set_str(DEFAULT_VIEW_LEVEL_KEY, READABLE);
	br_preference_set_bool(RUN_JAVASCRIPT_KEY, true);
	br_preference_set_bool(DISPLAY_IMAGES_KEY, true);
	br_preference_set_bool(BLOCK_POPUP_KEY, true);
	br_preference_set_bool(SHOW_SECURITY_WARNINGS_KEY, true);
	br_preference_set_bool(ACCEPT_COOKIES_KEY, true);
	if (vconf_set_int(VCONFKEY_SETAPPL_DEFAULT_MEM_WAP_INT, SETTING_DEF_MEMORY_PHONE) < 0)
		BROWSER_LOGE("vconf_set_int(VCONFKEY_SETAPPL_DEFAULT_MEM_WAP_INT, SETTING_DEF_MEMORY_PHONE) failed");
	if (vconf_set_str(USERAGENT_KEY, DEFAULT_USER_AGENT_STRING) < 0)
		BROWSER_LOGE("vconf_set_str(USERAGENT_KEY, DEFAULT_USER_AGENT_STRING) failed");

	elm_genlist_realized_items_update(m_genlist);
}

void Browser_Settings_Main_View::__reset_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	if (main_view->m_reset_confirm_popup) {
		evas_object_del(main_view->m_reset_confirm_popup);
		main_view->m_reset_confirm_popup = NULL;
	}

	main_view->_reset_settings();
}

void Browser_Settings_Main_View::__cancel_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	if (main_view->m_reset_confirm_popup) {
		evas_object_del(main_view->m_reset_confirm_popup);
		main_view->m_reset_confirm_popup = NULL;
	}
}

void Browser_Settings_Main_View::__clear_cache_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	if (main_view->m_clear_cache_confirm_popup) {
		evas_object_del(main_view->m_clear_cache_confirm_popup);
		main_view->m_clear_cache_confirm_popup = NULL;
	}

	/* Clear cache */
	Ewk_Context *ewk_context = ewk_context_default_get();
	ewk_context_cache_clear(ewk_context);
	ewk_context_web_indexed_database_delete_all(ewk_context);
	ewk_context_application_cache_delete_all(ewk_context);
	ewk_context_web_storage_delete_all(ewk_context);
	ewk_context_web_database_delete_all(ewk_context);

	main_view->show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);
}

void Browser_Settings_Main_View::__cancel_clear_cache_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	if (main_view->m_clear_cache_confirm_popup) {
		evas_object_del(main_view->m_clear_cache_confirm_popup);
		main_view->m_clear_cache_confirm_popup = NULL;
	}
}

void Browser_Settings_Main_View::__clear_history_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	if (main_view->m_clear_history_confirm_popup) {
		evas_object_del(main_view->m_clear_history_confirm_popup);
		main_view->m_clear_history_confirm_popup = NULL;
	}
	/* Clear history */
	if (!main_view->m_data_manager->get_history_db()->clear_history())
		BROWSER_LOGE("get_history_db()->clear_history() failed");

	main_view->show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);
}

void Browser_Settings_Main_View::__cancel_clear_history_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	if (main_view->m_clear_history_confirm_popup) {
		evas_object_del(main_view->m_clear_history_confirm_popup);
		main_view->m_clear_history_confirm_popup = NULL;
	}
}

void Browser_Settings_Main_View::__clear_all_cookie_data_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	if (main_view->m_clear_all_cookies_data_confirm_popup) {
		evas_object_del(main_view->m_clear_all_cookies_data_confirm_popup);
		main_view->m_clear_all_cookies_data_confirm_popup = NULL;
	}

	Ewk_Context *ewk_context = ewk_context_default_get();
	ewk_context_cookies_clear(ewk_context);

	main_view->show_notify_popup(BR_STRING_DELETED, 3, EINA_TRUE);
}

void Browser_Settings_Main_View::__cancel_clear_all_cookie_data_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	if (main_view->m_clear_all_cookies_data_confirm_popup) {
		evas_object_del(main_view->m_clear_all_cookies_data_confirm_popup);
		main_view->m_clear_all_cookies_data_confirm_popup = NULL;
	}
}

Eina_Bool Browser_Settings_Main_View::_show_reset_confirm_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_reset_confirm_popup)
		evas_object_del(m_reset_confirm_popup);

	m_reset_confirm_popup = elm_popup_add(m_genlist);
	if (!m_reset_confirm_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}

	evas_object_size_hint_weight_set(m_reset_confirm_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	std::string confirm_msg = std::string(BR_STRING_RESET_ALL_SETTINGS_TO_DEFAULT_Q);
	elm_object_text_set(m_reset_confirm_popup, confirm_msg.c_str());

	Evas_Object *ok_button = elm_button_add(m_reset_confirm_popup);
	if (!ok_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(ok_button, BR_STRING_YES);
	elm_object_part_content_set(m_reset_confirm_popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __reset_confirm_response_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_reset_confirm_popup);
	elm_object_text_set(cancel_button, BR_STRING_NO);
	elm_object_part_content_set(m_reset_confirm_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __cancel_confirm_response_cb, this);

	evas_object_show(m_reset_confirm_popup);

	return EINA_TRUE;
}

Eina_Bool Browser_Settings_Main_View::_show_clear_cache_confirm_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_clear_cache_confirm_popup)
		evas_object_del(m_clear_cache_confirm_popup);

	m_clear_cache_confirm_popup = elm_popup_add(m_genlist);
	if (!m_clear_cache_confirm_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}

	evas_object_size_hint_weight_set(m_clear_cache_confirm_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	std::string confirm_msg = std::string(BR_STRING_CLEAR_ALL_CACHE_DATA_Q);
	elm_object_text_set(m_clear_cache_confirm_popup, confirm_msg.c_str());

	Evas_Object *ok_button = elm_button_add(m_clear_cache_confirm_popup);
	if (!ok_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(ok_button, BR_STRING_YES);
	elm_object_part_content_set(m_clear_cache_confirm_popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __clear_cache_confirm_response_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_clear_cache_confirm_popup);
	elm_object_text_set(cancel_button, BR_STRING_NO);
	elm_object_part_content_set(m_clear_cache_confirm_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __cancel_clear_cache_confirm_response_cb, this);

	evas_object_show(m_clear_cache_confirm_popup);

	return EINA_TRUE;
}

Eina_Bool Browser_Settings_Main_View::_show_clear_history_confirm_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_clear_history_confirm_popup)
		evas_object_del(m_clear_history_confirm_popup);

	m_clear_history_confirm_popup = elm_popup_add(m_genlist);
	if (!m_clear_history_confirm_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}

	evas_object_size_hint_weight_set(m_clear_history_confirm_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	std::string confirm_msg = std::string(BR_STRING_CLEAR_ALL_HISTORY_DATA_Q);
	elm_object_text_set(m_clear_history_confirm_popup, confirm_msg.c_str());

	Evas_Object *ok_button = elm_button_add(m_clear_history_confirm_popup);
	if (!ok_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(ok_button, BR_STRING_YES);
	elm_object_part_content_set(m_clear_history_confirm_popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __clear_history_confirm_response_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_clear_history_confirm_popup);
	elm_object_text_set(cancel_button, BR_STRING_NO);
	elm_object_part_content_set(m_clear_history_confirm_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __cancel_clear_history_confirm_response_cb, this);

	evas_object_show(m_clear_history_confirm_popup);

	return EINA_TRUE;
}

Eina_Bool Browser_Settings_Main_View::_show_clear_all_cookie_data_confirm_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_clear_all_cookies_data_confirm_popup)
		evas_object_del(m_clear_all_cookies_data_confirm_popup);

	m_clear_all_cookies_data_confirm_popup = elm_popup_add(m_genlist);
	if (!m_clear_all_cookies_data_confirm_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}

	evas_object_size_hint_weight_set(m_clear_all_cookies_data_confirm_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	std::string confirm_msg = std::string(BR_STRING_CLEAR_ALL_COOKIE_DATA_Q);
	elm_object_text_set(m_clear_all_cookies_data_confirm_popup, confirm_msg.c_str());

	Evas_Object *ok_button = elm_button_add(m_clear_all_cookies_data_confirm_popup);
	if (!ok_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(ok_button, BR_STRING_YES);
	elm_object_part_content_set(m_clear_all_cookies_data_confirm_popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __clear_all_cookie_data_confirm_response_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_clear_all_cookies_data_confirm_popup);
	elm_object_text_set(cancel_button, BR_STRING_NO);
	elm_object_part_content_set(m_clear_all_cookies_data_confirm_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __cancel_clear_all_cookie_data_confirm_response_cb, this);

	evas_object_show(m_clear_all_cookies_data_confirm_popup);

	return EINA_TRUE;
}

void Browser_Settings_Main_View::__clear_location_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	if (main_view->m_clear_location_confirm_popup) {
		evas_object_del(main_view->m_clear_location_confirm_popup);
		main_view->m_clear_location_confirm_popup = NULL;
	}

	//main_view->_clear_location_settings();
	Browser_Geolocation_DB *geo_db = main_view->get_data_manager()->get_geolocation_db();
	geo_db->remove_all_geolocation_data();
}

void Browser_Settings_Main_View::__clear_location_cancel_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	if (main_view->m_clear_location_confirm_popup) {
		evas_object_del(main_view->m_clear_location_confirm_popup);
		main_view->m_clear_location_confirm_popup = NULL;
	}
}

Eina_Bool Browser_Settings_Main_View::_show_clear_location_confirm_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_clear_location_confirm_popup)
		evas_object_del(m_clear_location_confirm_popup);

	m_clear_location_confirm_popup = elm_popup_add(m_genlist);
	if (!m_clear_location_confirm_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}

	evas_object_size_hint_weight_set(m_clear_location_confirm_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	std::string confirm_msg = std::string(BR_STRING_MSG_DELETE_WEBSITE_LOCATION_ACCESS_INFORMATION_Q);
	elm_object_text_set(m_clear_location_confirm_popup, confirm_msg.c_str());

	Evas_Object *ok_button = elm_button_add(m_clear_location_confirm_popup);
	if (!ok_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(ok_button, BR_STRING_OK);
	elm_object_part_content_set(m_clear_location_confirm_popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __clear_location_confirm_response_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_clear_location_confirm_popup);
	if (!cancel_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(cancel_button, BR_STRING_CANCEL);
	elm_object_part_content_set(m_clear_location_confirm_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __clear_location_cancel_confirm_response_cb, this);

	evas_object_show(m_clear_location_confirm_popup);

	return EINA_TRUE;
}

Evas_Object *Browser_Settings_Main_View::_create_content_genlist(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *genlist = NULL;

	genlist = elm_genlist_add(m_navi_bar);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	elm_object_style_set(genlist, "dialogue");

	m_radio_text_item_class.item_style = "dialogue/1text.1icon/expandable2";
	m_radio_text_item_class.func.text_get = __genlist_label_get;
	m_radio_text_item_class.func.content_get = __genlist_icon_get;
	m_radio_text_item_class.func.state_get = NULL;
	m_radio_text_item_class.func.del = NULL;

	m_category_title_item_class.item_style = "dialogue/title";
	m_category_title_item_class.func.text_get = __genlist_label_get;
	m_category_title_item_class.func.content_get = NULL;
	m_category_title_item_class.func.del = NULL;
	m_category_title_item_class.func.state_get = NULL;

	/* Homepage */
	m_homepage_title_callback_data.type = BR_HOMEPAGE_TITLE;
	m_homepage_title_callback_data.user_data = this;
	m_homepage_title_callback_data.it = elm_genlist_item_append(genlist, &m_category_title_item_class,
						&m_homepage_title_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(m_homepage_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_2_text_item_class.item_style = "dialogue/2text.3/expandable";
	m_2_text_item_class.func.text_get = __genlist_label_get;
	m_2_text_item_class.func.content_get = NULL;
	m_2_text_item_class.func.del = NULL;
	m_2_text_item_class.func.state_get = NULL;

	m_homepage_item_callback_data.type = BR_HOMEPAGE_MENU;
	m_homepage_item_callback_data.user_data = this;
	m_homepage_item_callback_data.it = elm_genlist_item_append(genlist, &m_2_text_item_class,
							&m_homepage_item_callback_data, NULL, ELM_GENLIST_ITEM_TREE,
							__expandable_icon_clicked_cb, &m_homepage_item_callback_data);

	/* Content - Run JavaScript / Display Images */
	m_content_title_callback_data.type = BR_CONTENT_TITLE;
	m_content_title_callback_data.user_data = this;
	m_content_title_callback_data.it = elm_genlist_item_append(genlist, &m_category_title_item_class,
						&m_content_title_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(m_content_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);


	m_defailt_view_level_item_callback_data.type = BR_CONTENT_MENU_DEFAULT_VIEW_LEVEL;
	m_defailt_view_level_item_callback_data.user_data = this;
	m_defailt_view_level_item_callback_data.it = elm_genlist_item_append(genlist, &m_2_text_item_class,
						&m_defailt_view_level_item_callback_data, NULL, ELM_GENLIST_ITEM_TREE,
						__expandable_icon_clicked_cb, &m_defailt_view_level_item_callback_data);

	m_1_text_1_icon_item_class.item_style = "dialogue/1text.1icon";
	m_1_text_1_icon_item_class.func.text_get = __genlist_label_get;
	m_1_text_1_icon_item_class.func.content_get = __genlist_icon_get;
	m_1_text_1_icon_item_class.func.state_get = NULL;
	m_1_text_1_icon_item_class.func.del = NULL;

	m_run_javascript_item_callback_data.type = BR_CONTENT_MENU_RUN_JAVASCRIPT;
	m_run_javascript_item_callback_data.user_data = this;
	m_run_javascript_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_1_icon_item_class,
						&m_run_javascript_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_run_javascript_item_callback_data);

	m_display_images_item_callback_data.type = BR_CONTENT_MENU_DISPLAY_IMAGES;
	m_display_images_item_callback_data.user_data = this;
	m_display_images_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_1_icon_item_class,
						&m_display_images_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_display_images_item_callback_data);

	m_block_popup_item_callback_data.type = BR_CONTENT_MENU_BLOCK_POPUP;
	m_block_popup_item_callback_data.user_data = this;
	m_block_popup_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_1_icon_item_class,
						&m_block_popup_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_block_popup_item_callback_data);

	/* Privacy - Accept cookies / Auto save ID, password / Clear private data */
	m_privacy_title_callback_data.type = BR_PRIVACY_TITLE;
	m_privacy_title_callback_data.user_data = this;
	m_privacy_title_callback_data.it = elm_genlist_item_append(genlist, &m_category_title_item_class,
						&m_privacy_title_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(m_privacy_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_clear_cache_item_callback_data.type = BR_PRIVATE_MENU_CLEAR_CACHE;
	m_clear_cache_item_callback_data.user_data = this;
	m_clear_cache_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_1_icon_item_class,
						&m_clear_cache_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__genlist_item_clicked_cb, &m_clear_cache_item_callback_data);

	m_clear_history_item_callback_data.type = BR_PRIVATE_MENU_CLEAR_HISTORY;
	m_clear_history_item_callback_data.user_data = this;
	m_clear_history_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_1_icon_item_class,
						&m_clear_history_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__genlist_item_clicked_cb, &m_clear_history_item_callback_data);

	m_show_security_warnings_item_callback_data.type = BR_PRIVATE_MENU_SHOW_SECURITY_WARNINGS;
	m_show_security_warnings_item_callback_data.user_data = this;
	m_show_security_warnings_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_1_icon_item_class,
						&m_show_security_warnings_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_show_security_warnings_item_callback_data);

	m_2_text_1_icon_item_class.item_style = "dialogue/2text.1icon.5";
	m_2_text_1_icon_item_class.func.text_get = __genlist_label_get;
	m_2_text_1_icon_item_class.func.content_get = __genlist_icon_get;
	m_2_text_1_icon_item_class.func.state_get = NULL;
	m_2_text_1_icon_item_class.func.del = NULL;

	m_accept_cookies_item_callback_data.type = BR_PRIVACY_MENU_ACCEPT_COOKIES;
	m_accept_cookies_item_callback_data.user_data = this;
	m_accept_cookies_item_callback_data.it = elm_genlist_item_append(genlist, &m_2_text_1_icon_item_class,
						&m_accept_cookies_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_accept_cookies_item_callback_data);

	m_2_text_3_item_class.item_style = "dialogue/2text.2";
	m_2_text_3_item_class.func.text_get = __genlist_label_get;
	m_2_text_3_item_class.func.content_get = __genlist_icon_get;
	m_2_text_3_item_class.func.state_get = NULL;
	m_2_text_3_item_class.func.del = NULL;

	m_clear_all_cookies_data_item_callback_data.type = BR_PRIVATE_MENU_CLEAR_ALL_COOKIE_DATA;
	m_clear_all_cookies_data_item_callback_data.user_data = this;
	m_clear_all_cookies_data_item_callback_data.it = elm_genlist_item_append(genlist, &m_2_text_3_item_class,
						&m_clear_all_cookies_data_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__genlist_item_clicked_cb, &m_clear_all_cookies_data_item_callback_data);

	m_1_text_item_class.item_style = "dialogue/1text";
	m_1_text_item_class.func.text_get = __genlist_label_get;
	m_1_text_item_class.func.content_get = NULL;
	m_1_text_item_class.func.state_get = NULL;
	m_1_text_item_class.func.del = NULL;

	m_enable_location_callback_data.type = BR_PRIVACY_MENU_ENABLE_LOCATION;
	m_enable_location_callback_data.user_data = this;
	m_enable_location_callback_data.it = elm_genlist_item_append(genlist, &m_2_text_1_icon_item_class,
						&m_enable_location_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_enable_location_callback_data);

	m_clear_location_access_callback_data.type = BR_PRIVACY_MENU_CLEAR_LOCATION_ACCESS;
	m_clear_location_access_callback_data.user_data = this;
	m_clear_location_access_callback_data.it = elm_genlist_item_append(genlist, &m_2_text_3_item_class,
							&m_clear_location_access_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_clear_location_access_callback_data);
	bool enable_location = 1;
	br_preference_get_bool(ENABLE_LOCATION_KEY, &enable_location);
	if (!enable_location)
		elm_object_item_disabled_set(m_clear_location_access_callback_data.it, EINA_TRUE);

	/* Others */
	m_seperator_item_class.item_style = "grouptitle.dialogue.seperator";
	m_seperator_item_class.func.text_get = NULL;
	m_seperator_item_class.func.content_get = NULL;
	m_seperator_item_class.func.state_get = NULL;
	m_seperator_item_class.func.del = NULL;
	Elm_Object_Item *it = elm_genlist_item_append(genlist, &m_seperator_item_class,
						NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_website_setting_callback_data.type = BR_PRIVACY_WEBSITE_SETTING;
	m_website_setting_callback_data.user_data = this;
	m_website_setting_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_1_icon_item_class,
							&m_website_setting_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_website_setting_callback_data);

	m_reset_item_callback_data.type = BR_MENU_RESET_TO_DEFAULT;
	m_reset_item_callback_data.user_data = this;
	m_reset_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_item_class,
							&m_reset_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_reset_item_callback_data);

	/* Debug */
	m_debug_title_callback_data.type = BR_DEBUG_TITLE;
	m_debug_title_callback_data.user_data = this;
	m_debug_title_callback_data.it = elm_genlist_item_append(genlist, &m_category_title_item_class,
						&m_debug_title_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(m_debug_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_user_agent_item_callback_data.type = BR_MENU_USER_AGENT;
	m_user_agent_item_callback_data.user_data = this;
	m_user_agent_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_item_class,
							&m_user_agent_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_user_agent_item_callback_data);

	return genlist;
}

void Browser_Settings_Main_View::__application_cache_origin_get_cb(Eina_List* origins, void* user_data)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)user_data;
	int count = eina_list_count(origins);

	if (count > 0) {
		if (main_view->m_website_setting_callback_data.it != NULL)
			elm_object_item_disabled_set(main_view->m_website_setting_callback_data.it, EINA_FALSE);
	}
}

void Browser_Settings_Main_View::__web_storage_origin_get_cb(Eina_List* origins, void* user_data)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)user_data;
	int count = eina_list_count(origins);

	if (count > 0) {
		if (main_view->m_website_setting_callback_data.it != NULL)
			elm_object_item_disabled_set(main_view->m_website_setting_callback_data.it, EINA_FALSE);
	}
}

void Browser_Settings_Main_View::__web_database_origin_get_cb(Eina_List* origins, void* user_data)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)user_data;
	int count = eina_list_count(origins);

	if (count > 0) {
		if (main_view->m_website_setting_callback_data.it != NULL)
			elm_object_item_disabled_set(main_view->m_website_setting_callback_data.it, EINA_FALSE);
	}
}

