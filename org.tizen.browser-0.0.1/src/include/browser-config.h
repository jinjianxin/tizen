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

#ifndef BROWSER_CONFIG_H
#define BROWSER_CONFIG_H

#include <account.h>
#include <appsvc.h>
#include <app_service.h>
#include <app_manager.h>
#include <aul.h>
#include <bundle.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <haptic.h>
#include <dirent.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>
#include <libsoup/soup.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <ui-gadget.h>
#include <unistd.h>
#include <url_download.h>
#include <utilX.h>
#include <vconf.h>
#include <Elementary.h>
#include <Ecore.h>
#include <Ecore_IMF.h>
#include <Ecore_X.h>
#include <Evas.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <syspopup_caller.h>
#include <vector>
#include <vconf-internal-keys.h>

#include "browser-dlog.h"
#include "browser-string.h"

#define BUILDING_EFL__
#include <EWebKit2.h>

#define BROWSER_PACKAGE_NAME "browser"
#define BROWSER_EDJE_DIR "/opt/apps/org.tizen.browser/res/edje"
#define BROWSER_LOCALE_DIR "/opt/apps/org.tizen.browser/res/locale"
#define BROWSER_IMAGE_DIR "/opt/apps/org.tizen.browser/res/images"

#define BROWSER_URL_SCHEME_CHECK "://"
#define BROWSER_HTTP_SCHEME "http://"
#define BROWSER_HTTPS_SCHEME "https://"
#define BROWSER_FILE_SCHEME "file://"
#define BROWSER_RTSP_SCHEME "rtsp://"
#define BROWSER_MAIL_TO_SCHEME "mailto:"
#define BROWSER_SMS_SCHEME "sms:"
#define BROWSER_SMS_TO_SCHEME "smsto:"
#define BROWSER_MMS_SCHEME "mms:"
#define BROWSER_MMS_TO_SCHEME "mmsto:"
#define BROWSER_WTAI_SCHEME "wtai://"
#define BROWSER_WTAI_WP_AP_SCHEME "wtai://wp/ap;"

#define BROWSER_DEFAULT_USER_HOMEPAGE	"www.tizen.org"

/* browser extension theme. */
#define BROWSER_BUTTON_THEME BROWSER_EDJE_DIR"/browser-define.edj"
#define BROWSER_NAVIFRAME_THEME BROWSER_EDJE_DIR"/browser-naviframe.edj"
#define BROWSER_CONTROLBAR_THEME BROWSER_EDJE_DIR"/browser-view-control-bar.edj"
#define BROWSER_URL_LAYOUT_THEME BROWSER_EDJE_DIR"/browser-view-url-layout.edj"
#define BROWSER_PROGRESSBAR_THEME BROWSER_EDJE_DIR"/browser-view-progressbar.edj"
#define BROWSER_PREDICTIVE_HISTORY_THEME BROWSER_EDJE_DIR"/browser-predictive-history.edj"
#define BROWSER_SETTINGS_THEME BROWSER_EDJE_DIR"/browser-settings.edj"
#define BROWSER_BOOKMARK_THEME BROWSER_EDJE_DIR"/browser-bookmark-view.edj"
#if defined(FEATURE_MOST_VISITED_SITES)
#define BROWSER_MOST_VISITED_SITES_THEME BROWSER_EDJE_DIR"/most-visited-sites.edj"
#define BROWSER_MOST_VISITED_THEME BROWSER_EDJE_DIR"/browser-most-visited.edj"
#endif
#define BROWSER_FIND_WORD_LAYOUT_THEME BROWSER_EDJE_DIR"/browser-view-find-word-layout.edj"

/* browser vconf path */
#define BROWSER_VCONF_PREFIX	"db/browser/"
#define BROWSER_SETTING_VCONF_PREFIX "db/browsersetting/"

#define SHOW_MY_SITES_GUIDE	"ShowMySitesGuide"
#define LAST_VISITED_URL_KEY	"LastVisitedUrl"
#define DEFAULT_USER_AGENT_STRING "System user agent"
#define HOMEPAGE_KEY	"HomepageMode"
#define USER_HOMEPAGE_KEY	"UserHomepage"
#define SEARCHURL_KEY	"SearchUrl"
#define DEFAULT_VIEW_LEVEL_KEY	"DefaultViewLevel"
#define RUN_JAVASCRIPT_KEY	"RunJavaScript"
#define DISPLAY_IMAGES_KEY	"DisplayImages"
#define BLOCK_POPUP_KEY	"BlockPopup"
#define SHOW_SECURITY_WARNINGS_KEY	"ShowSecurityWarnings"
#define ACCEPT_COOKIES_KEY	"AcceptCookies"
#define ENABLE_LOCATION_KEY	"EnableLocation"
#ifdef ZOOM_BUTTON
#define ZOOM_BUTTON_KEY	"Zoombutton"
#endif

#define USERAGENT_KEY	VCONFKEY_BROWSER_BROWSER_USER_AGENT
#define CUSTOM_USERAGENT_KEY	VCONFKEY_BROWSER_CUSTOM_USER_AGENT

#define BROWSER_DEFAULT_BRIGHTNESS_LEVEL	7

/* ID to save to vconf for browser settings */
#if defined(FEATURE_MOST_VISITED_SITES)
#define MOST_VISITED_SITES "MOST_VISITED_SITES"
#endif
#define RECENTLY_VISITED_SITE "RECENTLY_VISITED_SITE"
#define USER_HOMEPAGE "USER_HOMEPAGE"
#define EMPTY_PAGE	"EMPTY_PAGE"
#define FIT_TO_WIDTH	"FIT_TO_WIDTH"
#define READABLE	"READABLE"
#define ALWAYS_ASK	"ALWAYS_ASK"
#define ALWAYS_ON	"ON"
#define ALWAYS_OFF	"OFF"

/* bookmark definition */
#define BROWSER_BOOKMARK_DB_PATH	"/opt/dbspace/.internet_bookmark.db"
#define BROWSER_BOOKMARK_GENLIST_BLOCK_COUNT	50
#define BROWSER_BOOKMARK_MAIN_FOLDER_ID	1
#define BROWSER_BOOKMARK_COUNT_LIMIT	1000
#define BROWSER_MAX_TITLE_LEN	1024
#define BROWSER_MAX_URL_LEN	2048
#define BROWSER_BOOKMARK_PROCESS_BLOCK_COUNT	100

/* history definition */
#define BROWSER_HISTORY_DB_PATH	"/opt/dbspace/.browser-history.db"
#define BROWSER_HISTORY_COUNT_LIMIT	1000
#define BROWSER_MAX_DATE_LEN	40
#define BROWSER_PREDICTIVE_HISTORY_COUNT	2

/* multi window definition */
#define BROWSER_MULTI_WINDOW_MAX_COUNT	9
#define BROWSER_MULTI_WINDOW_ITEM_RATIO	0.60f

#define BROWSER_USER_AGENT_DB_PATH	"/opt/apps/org.tizen.browser/data/db/.browser.db"
#define BROWSER_DEFAULT_USER_AGENT_TITLE	"Tizen"

/* Most visited definition */
#define BROWSER_MOST_VISITED_COUNT_TEXT "3"

#if defined(FEATURE_MOST_VISITED_SITES)
/* Speed dial definition */
#define BROWSER_MOST_VISITED_SITES_DB_PATH	"/opt/apps/org.tizen.browser/data/db/.browser-mostvisited.db"
#define BROWSER_MOST_VISITED_SITES_ITEM_MAX	9
#define BROWSER_MOST_VISITED_SITES_URL	""
#define DEFAULT_ICON_PREFIX "default_"
#endif
#define BROWSER_BLANK_PAGE_URL	"about:blank"

/* Geolocation definition */
#define BROWSER_GEOLOCATION_DB_PATH	"/opt/apps/org.tizen.browser/data/db/.browser-geolocation.db"

/* Screen shot path definition */
#define BROWSER_SCREEN_SHOT_DIR	"/opt/apps/org.tizen.browser/data/screenshots/"
#define BROWSER_FAVICON_DB_PATH	"/opt/apps/org.tizen.browser/data/db/WebpageIcons.db"

#define BROWSER_CLEAN_UP_WINDOWS_TIMEOUT	(60 * 30) // 30 min

/* Haptic device definition */
#define BROWSER_HAPTIC_DEVICE_HANDLE    0

#define BROWSER_GOOGLE	"Google"
#define BROWSER_YAHOO	"Yahoo"
#define BROWSER_BING	"Bing"

#define SEC_DOWNLOAD_APP "org.tizen.download-provider"
#define SEC_STREAMING_PLAYER "org.tizen.video-player"
#define SEC_VIDEO_PLAYER SEC_STREAMING_PLAYER
#define SEC_MUSIC_PLAYER "org.tizen.sound-player"
#define SEC_VT_CALL "org.tizen.vtmain"
#define SEC_MESSAGE "org.tizen.message"
#define SEC_EMAIL "org.tizen.email"
#define SEC_SAMSUNG_APPS "org.tizen.samsungapps"

#define ELM_NAVIFRAME_ITEM_CONTENT "default"
#define ELM_NAVIFRAME_ITEM_CONTROLBAR "controlbar"
#define ELM_NAVIFRAME_ITEM_ICON "icon"
#define ELM_NAVIFRAME_ITEM_OPTIONHEADER "optionheader"
#define ELM_NAVIFRAME_ITEM_OPTIONHEADER2 "optionheader2"

#define ELM_NAVIFRAME_ITEM_SIGNAL_CONTROLBAR_CLOSE "elm,state,controlbar,close", ""
#define ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_CLOSE "elm,state,optionheader,close", ""
#define ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_INSTANT_CLOSE "elm,state,optionheader,instant_close", ""
#define ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_INSTANT_OPEN "elm,state,optionheader,instant_open", ""
#define ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_OPEN "elm,state,optionheader,open", ""
#define ELM_NAVIFRAME_ITEM_TITLE_LABEL "title"
#define ELM_NAVIFRAME_ITEM_TITLE_LEFT_BTN "title_left_btn"
#define ELM_NAVIFRAME_ITEM_TITLE_MORE_BTN "title_more_btn"
#define ELM_NAVIFRAME_ITEM_TITLE_RIGHT_BTN "title_right_btn"
#define ELM_NAVIFRAME_ITEM_PREV_BTN "prev_btn"

#endif /* BROWSER_CONFIG_H */

