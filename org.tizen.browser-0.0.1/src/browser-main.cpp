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
  *@file browser-main.cpp
  *@brief browser主函数
  */

extern "C" {
#include <ITapiSat.h>
#include <TapiUtility.h>
}

#include <app.h>
#include "browser-config.h"
#include "browser-class.h"

struct browser_data {
	Evas_Object *main_win;
	Evas_Object *bg;
	Evas_Object *main_layout;
	Evas_Object *navi_bar;
	Elm_Theme *browser_theme;

	Browser_Class *browser_instance;

	Eina_Bool is_paused;
};


/**
 * @brief __br_set_env 设置环境变量
 */
static void __br_set_env(void)
{
#if !defined(TIZEN_PUBLIC)
	/* manual enabling of CoreGL fastpath */	
	setenv("COREGL_FASTPATH", "1", 1);
#endif

	/* set image cache suze */
	if (setenv("ELM_IMAGE_CACHE", "0", 1))
		BROWSER_LOGD("ELM_IMAGE_CACHE is set to 1MB");

	/* This is because of showing navigation bar more quickly. */
	if (setenv("ELM_BRING_IN_SCROLL_FRICTION", "0.2", 1))
		BROWSER_LOGD("ELM_BRING_IN_SCROLL_FRICTION is set");

	if (setenv("ELM_PAGE_SCROLL_FRICTION", "0.4", 1))
		BROWSER_LOGD("ELM_PAGE_SCROLL_FRICTION is set");

	if (setenv("ELM_THUMBSCROLL_BOUNCE_FRICTION", "0.2", 1))
		BROWSER_LOGD("ELM_THUMBSCROLL_BOUNCE_FRICTION is set");
}


/**
 * @brief __br_destroy 退出函数
 * @param data
 */
static void __br_destroy(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	elm_exit();
}

static void __main_win_del_cb(void *data, Evas_Object *obj, void *event)
{
	BROWSER_LOGD("[Browser-Launching time measure]<< window delete callback >>");
}

/**
 * @brief __init_preference 初始化程序配置
 * @return 是否创建成功
 */
static bool __init_preference()
{
	BROWSER_LOGD("[%s]", __func__);

	if (!br_preference_create_bool(SHOW_MY_SITES_GUIDE, true))
		return false;
	if (!br_preference_create_str(LAST_VISITED_URL_KEY, ""))
		return false;
#if defined(FEATURE_MOST_VISITED_SITES)
	if (!br_preference_create_str(HOMEPAGE_KEY, MOST_VISITED_SITES))
		return false;
#else
	if (!br_preference_create_str(HOMEPAGE_KEY, USER_HOMEPAGE))
		return false;
#endif
	if (!br_preference_create_str(USER_HOMEPAGE_KEY, BROWSER_DEFAULT_USER_HOMEPAGE))
		return false;
	if (!br_preference_create_str(SEARCHURL_KEY, "http://search.yahoo.com/search?p="))
		return false;
	if (!br_preference_create_str(DEFAULT_VIEW_LEVEL_KEY, READABLE))
		return false;
	if (!br_preference_create_bool(RUN_JAVASCRIPT_KEY, true))
		return false;
	if (!br_preference_create_bool(DISPLAY_IMAGES_KEY, true))
		return false;
	if (!br_preference_create_bool(BLOCK_POPUP_KEY, true))
		return false;
	if (!br_preference_create_bool(SHOW_SECURITY_WARNINGS_KEY, true))
		return false;
	if (!br_preference_create_bool(ACCEPT_COOKIES_KEY, true))
		return false;
	if (!br_preference_create_bool(ENABLE_LOCATION_KEY, true))
		return false;
#ifdef ZOOM_BUTTON
	if (!br_preference_create_bool(ZOOM_BUTTON_KEY, true))
		return false;
#endif
	return true;
}

/**
 * @brief __create_main_win 创建住窗口
 * @param app_data 传递的程序结构体
 * @return 返回创建的对象
 */
static Evas_Object *__create_main_win(void *app_data)
{
	Evas_Object *window = elm_win_add(NULL, BROWSER_PACKAGE_NAME, ELM_WIN_BASIC);
	if (window) {
		int w;
		int h;
		elm_win_title_set(window, BROWSER_PACKAGE_NAME);
		elm_win_borderless_set(window, EINA_TRUE);
		elm_win_conformant_set(window, EINA_TRUE);
		evas_object_smart_callback_add(window, "delete,request",
						__main_win_del_cb, app_data);
		ecore_x_window_size_get(ecore_x_window_root_first_get(),
					&w, &h);
		evas_object_resize(window, w, h);
		elm_win_indicator_mode_set(window, ELM_WIN_INDICATOR_SHOW);
		evas_object_show(window);
	}

	return window;
}

/**
 * @brief __create_bg  创建背景
 * @param win  父对象
 * @return 返回创建成功的背景
 */

static Evas_Object *__create_bg(Evas_Object *win)
{
	Evas_Object *bg;
	bg = elm_bg_add(win);
	if (!bg)
		return NULL;

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, bg);
	evas_object_show(bg);

	return bg;
}

/**
 * @brief __create_main_layout  创建程序主体布局
 * @param win 父对象
 * @return 返回布局对象
 */

static Evas_Object *__create_main_layout(Evas_Object *win)
{
	Evas_Object *layout;
	layout = elm_layout_add(win);
	if (!layout)
		return NULL;

	if (!elm_layout_theme_set(layout, "layout", "application", "default"))
		BROWSER_LOGE("elm_layout_theme_set is failed.\n");

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, layout);
	edje_object_signal_emit(elm_layout_edje_get(layout), "elm,state,show,indicator", "elm");
	evas_object_show(layout);

	return layout;
}

/**
 * @brief __create_navi_bar  创建naviframe容器
 * @param app_data 应用程序数据
 * @return 返回创建的naviframe容器
 */
static Evas_Object *__create_navi_bar(void *app_data)
{
    BROWSER_LOGD("**********************[%s]", __func__);
	Evas_Object *navi_bar;
	struct browser_data *ad = (struct browser_data *)app_data;
	navi_bar = elm_naviframe_add(ad->main_layout);
	if (navi_bar) {
		elm_object_theme_set(navi_bar, ad->browser_theme);
		elm_object_style_set(navi_bar, "browser");
		elm_object_part_content_set(ad->main_layout, "elm.swallow.content", navi_bar);
		evas_object_show(navi_bar);
	}

	return navi_bar;
}

/**
 * @brief __process_app_service 处理程序服务？
 * @param service service_h对象 定义在三星应用程序框架中
 * @param app_data 应用程序数据
 * @return 是否成功处理
 */
static Eina_Bool __process_app_service(service_h service, void *app_data)
{
    BROWSER_LOGD("**********************[%s]", __func__);
	struct browser_data *ad = (struct browser_data *)app_data;

	char *operation = NULL;
	char *request_uri = NULL;
	char *request_mime_type = NULL;

    if (service_get_operation(service, &operation) != SERVICE_ERROR_NONE) {
        BROWSER_LOGD("get service operation failed");
        return EINA_FALSE;
    }

    if (service_get_uri(service, &request_uri) != SERVICE_ERROR_NONE) {
        BROWSER_LOGD("get service uri failed");
    }

    if (service_get_mime(service, &request_mime_type) != SERVICE_ERROR_NONE) {
        BROWSER_LOGD("get service mime failed");
    }

    if (!operation && !request_uri && !request_mime_type) {
        BROWSER_LOGD("Not app svc");
        return EINA_FALSE;
    }

    operation = "http://tizen.org/appcontrol/operation/default";

    BROWSER_LOGD("*****operation[%s] \n, request_uri[%s]\n, request_mime_type[%s]\n", operation, request_uri, request_mime_type);

    operation = "http://tizen.org/appcontrol/operation/default";

	std::string full_path;

	if (request_mime_type) {
		if (!strncmp(request_mime_type, "application/x-shockwave-flash", strlen("application/x-shockwave-flash"))
			|| !strncmp(request_mime_type, "image/svg+xml", strlen("image/svg+xml"))
			|| !strncmp(request_mime_type, "text/html", strlen("text/html"))
			|| !strncmp(request_mime_type, "application/xml", strlen("application/xml"))) {
			if (request_uri)
				full_path = std::string(request_uri);
		} else {
			BROWSER_LOGD("Not allowed mime type : [%s]", request_mime_type);
			return EINA_FALSE;
		}
	} else if (request_uri) {
		full_path = std::string(request_uri);
	}

	BROWSER_LOGD("url=[%s]", full_path.c_str());

	if (ad->main_win)
		elm_win_activate(ad->main_win);

	ad->browser_instance->launch(full_path.c_str(), EINA_TRUE);

	return EINA_TRUE;
}

/**
 * @brief __br_load_url 加载指定地址
 * @param service service_h对象
 * @param app_data 应用程序数据
 */
static void __br_load_url(service_h service, void *app_data)
{
    BROWSER_LOGD("**********************[%s]", __func__);
	struct browser_data *ad = (struct browser_data *)app_data;

    if (__process_app_service(NULL, app_data)) {
		BROWSER_LOGD("app service");
		return;
	}

	std::string full_path;
	char *mime_type = NULL;
	char *search_keyword = NULL;
	char *goto_url = NULL;
	char *url = NULL;

	if (service_get_uri(service, &url) != SERVICE_ERROR_NONE) {
		BROWSER_LOGD("get service uri failed");
	}

	if (service_get_mime(service, &mime_type) != SERVICE_ERROR_NONE) {
		BROWSER_LOGD("get service mime failed");
	}

	if (service_get_extra_data(service, "search_keyword", &search_keyword) != SERVICE_ERROR_NONE) {
		BROWSER_LOGD("get service extra data(search keyword) failed");
	}

	if (service_get_extra_data(service, "goto_url", &search_keyword) != SERVICE_ERROR_NONE) {
		BROWSER_LOGD("get service extra data(search keyword) failed");
	}

    printf("**********%s\t**********%s\t**********%s\t**********%s\n",
           mime_type,search_keyword,goto_url,url);

	if (mime_type && url) {
		BROWSER_LOGD("mime type=[%s], url=[%s]", mime_type, url);
		if (!strcmp(mime_type, "application/x-shockwave-flash")
			|| !strcmp(mime_type, "image/svg+xml")
			|| !strcmp(mime_type, "text/html")
			|| !strcmp(mime_type, "application/xml")) {
			full_path = std::string(url);
		}
	} else if (search_keyword) {
		BROWSER_LOGD("search_keyword=[%s]", search_keyword);
		if (search_keyword) {
			char *search_url = NULL;
			if (br_preference_get_str(SEARCHURL_KEY, &search_url) == false) {
				BROWSER_LOGE("failed to get %s preference\n", SEARCHURL_KEY);
				return;
			}
			if (search_url) {
				full_path = std::string(search_url) + std::string(search_keyword);
				free(search_url);
			}
		}
	} else if (goto_url || url) {
		BROWSER_LOGD("goto_url=[%s], url=[%s]", goto_url, url);
		if (goto_url)
			full_path = goto_url;
		else
			full_path = url;
	}
	BROWSER_LOGD("url=[%s]", full_path.c_str());

	if (ad->main_win)
		elm_win_activate(ad->main_win);

	ad->browser_instance->launch(full_path.c_str(), EINA_TRUE);
}

/**
 * @brief __br_keydown_event  处理键盘事件
 * @param data 传递给回调函数的参数
 * @param type 类型？
 * @param event Ecore_Event_Key结构体
 * @return 是否执行成功
 */
static Eina_Bool __br_keydown_event(void *data, int type, void *event)
{
	BROWSER_LOGD("[%s]", __func__);
	Ecore_Event_Key *ev = (Ecore_Event_Key *)event;

	if (!strcmp(ev->keyname, KEY_END)) {
		__br_destroy(data);
	}

	return EXIT_FAILURE;
}


#if defined(HORIZONTAL_UI)

/**
 * @brief __br_rotate_cb 处理屏幕旋转问题
 * @param mode 旋转方向
 * @param data 传递参数
 */
static void __br_rotate_cb(app_device_orientation_e mode, void *data)
{
    BROWSER_LOGD("**********************[%s] rotation mode = %d", __func__, mode);
	struct browser_data *ad = (struct browser_data *)data;
	int rotation_value;

	switch (mode) {
	case APP_DEVICE_ORIENTATION_0:
		rotation_value = 0;
		ug_send_event(UG_EVENT_ROTATE_PORTRAIT);
		break;
	case APP_DEVICE_ORIENTATION_90:
		rotation_value = 90;
		ug_send_event(UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN);
		break;
	case APP_DEVICE_ORIENTATION_180:
		rotation_value = 180;
		ug_send_event(UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN);
		break;
	case APP_DEVICE_ORIENTATION_270:
		rotation_value = 270;
		ug_send_event(UG_EVENT_ROTATE_LANDSCAPE);
		break;
	default:
		rotation_value = -1;
		break;
	}

	if (rotation_value >= 0 && ad->browser_instance) {
		if (ad->browser_instance->is_available_to_rotate()) {
			elm_win_rotation_with_resize_set(ad->main_win, rotation_value);
			ad->browser_instance->rotate(rotation_value);
		}
	}
}
#endif

/**
 * @brief __br_low_memory_cb 系统低电量时调用此函数
 * @param data 传递的应用程序数据
 */
static void __br_low_memory_cb(void* data)
{
	BROWSER_LOGD("[%s]", __func__);
	/* To do */
	struct browser_data *ad = (struct browser_data *)data;

	if (ad && ad->browser_instance)
		ad->browser_instance->clean_up_windows();
}

/**
 * @brief __br_low_battery_cb 低电量时调用此函数
 * @param data 传递的应用程序数据
 */
static void __br_low_battery_cb(void* data)
{
	BROWSER_LOGD("[%s]", __func__);
	/* To do */
	return;
}

static void __br_lang_changed_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	/* To do */
	return;
}

static void __br_region_changed_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	/* To do */
	return;
}

static void __br_register_system_event(void *app_data)
{
	/* To do */
	struct browser_data *ad = (struct browser_data *)app_data;
}

/**
 * @brief __browser_set_i18n 设置语言环境
 * @param domain 语言环境参数
 * @param dir 国家化文件存放路径
 * @return 是否创建成功
 */
static int __browser_set_i18n(const char *domain, const char *dir)
{
    char *r = NULL;

    if (domain == NULL) {
		BROWSER_LOGE("domain is NULL");
        return -1;
    }

    r = setlocale(LC_ALL, "");

    /* if locale is not set properly, try again to set as language base */
    if (r == NULL) {
        char *lang_set = vconf_get_str(VCONFKEY_LANGSET);
        if (!lang_set && strlen(lang_set) > 0)
            r = setlocale(LC_ALL, lang_set);

        BROWSER_LOGE("setlocale as [%s]", r);
        if (lang_set)
            free(lang_set);
    }

    if (r == NULL) {
		BROWSER_LOGE("setlocale failed");
        return -1;
    }

    r = bindtextdomain(domain, dir);
    if (r == NULL) {
		BROWSER_LOGE("bindtextdomain failed");
        return -1;
    }

    r = textdomain(domain);
    if (r == NULL) {
		BROWSER_LOGE("textdomain failed");
        return -1;
    }

    return 0;
}
/**
 * @brief __br_app_create 应用程序初始化
 * @param app_data 应用程序数据
 * @return 是否执行成功
 */
static bool __br_app_create(void *app_data)
{
    BROWSER_LOGD("********************[Browser-Launching time measure] << theme extenstion >>");
	struct browser_data *ad = (struct browser_data *)app_data;

	elm_config_preferred_engine_set("opengl_x11");


	/**
	 *加载配置文件
	 */
	if(!__init_preference())
		return false;
	/**
	*加载主题
	*/
	ad->browser_theme = elm_theme_new();
	elm_theme_ref_set(ad->browser_theme, NULL);
	elm_theme_extension_add(ad->browser_theme, BROWSER_NAVIFRAME_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_CONTROLBAR_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_BUTTON_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_PROGRESSBAR_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_URL_LAYOUT_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_PREDICTIVE_HISTORY_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_SETTINGS_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_BOOKMARK_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_FIND_WORD_LAYOUT_THEME);

	BROWSER_LOGD("[Browser-Launching time measure] << create main window >>");
	ad->main_win = __create_main_win(ad); //创建窗口
	if (!ad->main_win) {
		BROWSER_LOGE("fail to create window");
		return false;
	}

	BROWSER_LOGD("[Browser-Launching time measure] << create background >>");
    ad->bg = __create_bg(ad->main_win); //
	if (!ad->bg) {
		BROWSER_LOGE("fail to create bg");
		return false;
	}

	BROWSER_LOGD("[Browser-Launching time measure] << create layout main >>");
	ad->main_layout = __create_main_layout(ad->main_win);
	if (!ad->main_layout) {
		BROWSER_LOGE("fail to create main layout");
		return false;
	}

	ad->navi_bar = __create_navi_bar(ad);
	if (!ad->navi_bar) {
		BROWSER_LOGE("fail to create navi bar");
		return false;
	}

	/* create browser instance & init */
	ad->browser_instance = new(nothrow) Browser_Class(ad->main_win, ad->navi_bar, ad->bg);
	if (!ad->browser_instance) {
		BROWSER_LOGE("fail to Browser_Class");
		return false;
    }
    /**
      *初始化browser_view
      */
	if (ad->browser_instance->init() == EINA_FALSE) {
		BROWSER_LOGE("fail to browser init");
		return false;
	}

	/* init internationalization */
	int ret = __browser_set_i18n(BROWSER_PACKAGE_NAME, BROWSER_LOCALE_DIR);
	if (ret) {
		BROWSER_LOGE("fail to __browser_set_i18n");
		return false;
	}

#if defined(HORIZONTAL_UI)
	app_device_orientation_e rotation_value = app_get_device_orientation();

	if (rotation_value != APP_DEVICE_ORIENTATION_0) {
		elm_win_rotation_with_resize_set(ad->main_win, rotation_value);
		ad->browser_instance->rotate(rotation_value);
	}
#endif

	return true;
}

/* GCF test requirement */
static void __send_termination_event_to_tapi(void)
{
    BROWSER_LOGD("**********************[%s]", __func__);
	int ret = TAPI_API_SUCCESS;
	int request_id = -1;
	TelSatEventDownloadReqInfo_t event_data;
	event_data.eventDownloadType = TAPI_EVENT_SAT_DW_TYPE_BROWSER_TERMINATION;
	event_data.u.browserTerminationEventReqInfo.browserTerminationCause = TAPI_SAT_BROWSER_TERMINATED_BY_USER;

	TapiHandle *handle = NULL;
	handle = tel_init(NULL);

	ret = tel_download_sat_event(handle, &event_data, NULL, NULL);
	if(ret != TAPI_API_SUCCESS && ret != TAPI_API_SAT_EVENT_NOT_REQUIRED_BY_USIM)
		BROWSER_LOGE("failed to tel_download_sat_event");

	tel_deinit(handle);
}

static void __br_app_terminate(void *app_data)
{
    BROWSER_LOGD("**********************[%s]", __func__);
	struct browser_data *ad = (struct browser_data *)app_data;

	elm_theme_extension_del(ad->browser_theme, BROWSER_NAVIFRAME_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_CONTROLBAR_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_BUTTON_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_URL_LAYOUT_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_PROGRESSBAR_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_PREDICTIVE_HISTORY_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_SETTINGS_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_BOOKMARK_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_FIND_WORD_LAYOUT_THEME);
	elm_theme_free(ad->browser_theme);

	/* GCF test requirement */
	__send_termination_event_to_tapi();

	if (ad->main_win);
		evas_object_del(ad->main_win);

	if (ad->browser_instance)
		delete ad->browser_instance;

	BROWSER_LOGD("[Browser-Launching time measure] << __br_app_terminate ends >>");
}

static void __br_app_pause(void *app_data)
{
    BROWSER_LOGD("**********************[%s]", __func__);
	struct browser_data *ad = (struct browser_data *)app_data;

	if (!ad || !ad->browser_instance)
		return;

	ad->browser_instance->pause();

	ad->is_paused = EINA_TRUE;
}

static void __br_app_resume(void *app_data)
{
    BROWSER_LOGD("**********************[%s]", __func__);
	struct browser_data *ad = (struct browser_data *)app_data;

	if (!ad || !ad->browser_instance)
		return;

	ad->browser_instance->resume();

	ad->is_paused = EINA_FALSE;
}

/**
 * @brief __br_app_reset 重置应用程序
 * @param service service_h结构体
 * @param app_data 应用程序数据
 */
static void __br_app_reset(service_h service, void *app_data)
{
	struct browser_data *ad = (struct browser_data *)app_data;
    BROWSER_LOGD("**********************[%s]", __func__);

	ad->browser_instance->reset();

    __br_load_url(service, app_data);

	ad->is_paused = EINA_FALSE;
}

/*

int main(int argc, char *argv[])
{
    __br_set_env();

    app_event_callback_s ops;
    memset(&ops, 0x0, sizeof(app_event_callback_s));

    ops.create = __br_app_create; //初始化函数
    ops.terminate = __br_app_terminate; //析构函数
    ops.pause = __br_app_pause;
    ops.resume = __br_app_resume;
    ops.service = __br_app_reset;
    ops.low_memory = __br_low_memory_cb; //低内存回调函数
    ops.low_battery = __br_low_battery_cb; //低电量回调函数
#if defined(HORIZONTAL_UI)
    ops.device_orientation = __br_rotate_cb;
#endif
    ops.language_changed = __br_lang_changed_cb;//语言切换
    ops.region_format_changed = __br_region_changed_cb;

    struct browser_data ad;
    memset(&ad, 0x0, sizeof(struct browser_data));

    int ret = app_efl_main(&argc, &argv, &ops, &ad);

    elm_run();

    return ret;
}*/

static Eina_Bool __process_app_service( void *app_data)
{
    BROWSER_LOGD("**********************[%s]", __func__);
    struct browser_data *ad = (struct browser_data *)app_data;

    char *operation = NULL;
    char *request_uri = NULL;
    char *request_mime_type = NULL;

//    if (service_get_operation(service, &operation) != SERVICE_ERROR_NONE) {
//        BROWSER_LOGD("get service operation failed");
//        return EINA_FALSE;
//    }

//    if (service_get_uri(service, &request_uri) != SERVICE_ERROR_NONE) {
//        BROWSER_LOGD("get service uri failed");
//    }

//    if (service_get_mime(service, &request_mime_type) != SERVICE_ERROR_NONE) {
//        BROWSER_LOGD("get service mime failed");
//    }

//    if (!operation && !request_uri && !request_mime_type) {
//        BROWSER_LOGD("Not app svc");
//        return EINA_FALSE;
//    }

    operation = "http://tizen.org/appcontrol/operation/default";

    BROWSER_LOGD("*****operation[%s] \n, request_uri[%s]\n, request_mime_type[%s]\n", operation, request_uri, request_mime_type);

    operation = "http://tizen.org/appcontrol/operation/default";

    std::string full_path;

    if (request_mime_type) {
        if (!strncmp(request_mime_type, "application/x-shockwave-flash", strlen("application/x-shockwave-flash"))
            || !strncmp(request_mime_type, "image/svg+xml", strlen("image/svg+xml"))
            || !strncmp(request_mime_type, "text/html", strlen("text/html"))
            || !strncmp(request_mime_type, "application/xml", strlen("application/xml"))) {
            if (request_uri)
                full_path = std::string(request_uri);
        } else {
            BROWSER_LOGD("Not allowed mime type : [%s]", request_mime_type);
            return EINA_FALSE;
        }
    } else if (request_uri) {
        full_path = std::string(request_uri);
    }

    BROWSER_LOGD("url=[%s]", full_path.c_str());

    if (ad->main_win)
        elm_win_activate(ad->main_win);

    ad->browser_instance->launch(full_path.c_str(), EINA_TRUE);

    return EINA_TRUE;
}

static void __br_load_url(void *app_data)
{
    BROWSER_LOGD("**********************[%s]", __func__);
    struct browser_data *ad = (struct browser_data *)app_data;

    if (__process_app_service(app_data)) {
        BROWSER_LOGD("app service");
        return;
    }

    std::string full_path;
    char *mime_type = NULL;
    char *search_keyword = NULL;
    char *goto_url = NULL;
    char *url = NULL;

   /* if (service_get_uri(service, &url) != SERVICE_ERROR_NONE) {
        BROWSER_LOGD("get service uri failed");
    }

    if (service_get_mime(service, &mime_type) != SERVICE_ERROR_NONE) {
        BROWSER_LOGD("get service mime failed");
    }

    if (service_get_extra_data(service, "search_keyword", &search_keyword) != SERVICE_ERROR_NONE) {
        BROWSER_LOGD("get service extra data(search keyword) failed");
    }

    if (service_get_extra_data(service, "goto_url", &search_keyword) != SERVICE_ERROR_NONE) {
        BROWSER_LOGD("get service extra data(search keyword) failed");
    }
*/

    if (mime_type && url) {
        BROWSER_LOGD("mime type=[%s], url=[%s]", mime_type, url);
        if (!strcmp(mime_type, "application/x-shockwave-flash")
            || !strcmp(mime_type, "image/svg+xml")
            || !strcmp(mime_type, "text/html")
            || !strcmp(mime_type, "application/xml")) {
            full_path = std::string(url);
        }
    } else if (search_keyword) {
        BROWSER_LOGD("search_keyword=[%s]", search_keyword);
        if (search_keyword) {
            char *search_url = NULL;
            if (br_preference_get_str(SEARCHURL_KEY, &search_url) == false) {
                BROWSER_LOGE("failed to get %s preference\n", SEARCHURL_KEY);
                return;
            }
            if (search_url) {
                full_path = std::string(search_url) + std::string(search_keyword);
                free(search_url);
            }
        }
    } else if (goto_url || url) {
        BROWSER_LOGD("goto_url=[%s], url=[%s]", goto_url, url);
        if (goto_url)
            full_path = goto_url;
        else
            full_path = url;
    }
    BROWSER_LOGD("url=[%s]", full_path.c_str());

    if (ad->main_win)
        elm_win_activate(ad->main_win);

    ad->browser_instance->launch(full_path.c_str(), EINA_TRUE);
}


static void __br_app_reset(void *app_data)
{
    struct browser_data *ad = (struct browser_data *)app_data;
    BROWSER_LOGD("**********************[%s]", __func__);

    ad->browser_instance->reset();

    __br_load_url(NULL, app_data);

    ad->is_paused = EINA_FALSE;
}

int main(int argc,char *argv[])
{
    g_type_init();
    elm_init(argc, argv);

    __br_set_env();
    struct browser_data ad;
    memset(&ad, 0x0, sizeof(struct browser_data));


    __br_app_create(&ad);

    __br_app_reset(&ad);


    elm_run();

}
