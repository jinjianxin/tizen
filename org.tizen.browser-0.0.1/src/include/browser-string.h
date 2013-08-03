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

#ifndef BROWSER_STRING_H
#define BROWSER_STRING_H

#include "browser-dlog.h"

/* browser string define for translation */

/* System string */
/*************************************************************************************************************/
#define BR_STRING_OK            dgettext("sys_string", "IDS_COM_SK_OK")
#define BR_STRING_EDIT          dgettext("sys_string", "IDS_COM_SK_EDIT")
#define BR_STRING_YES           dgettext("sys_string", "IDS_COM_SK_YES")
#define BR_STRING_NO            dgettext("sys_string", "IDS_COM_SK_NO")
#define BR_STRING_BACK          dgettext("sys_string", "IDS_COM_BODY_BACK")
#define BR_STRING_CANCEL        dgettext("sys_string", "IDS_COM_SK_CANCEL")
#define BR_STRING_SAVE          dgettext("sys_string", "IDS_COM_SK_SAVE")
#define BR_STRING_DONE          dgettext("sys_string", "IDS_COM_SK_DONE")
#define BR_STRING_DELETE        dgettext("sys_string", "IDS_COM_SK_DELETE")
#define BR_STRING_DELETE_Q      dgettext("sys_string", "IDS_COM_POP_DELETE_Q")
#define BR_STRING_DELETE_ALL           dgettext("sys_string", "IDS_COM_BODY_DELETE_ALL")
#define BR_STRING_CLOSE         dgettext("sys_string", "IDS_COM_POP_CLOSE")
#define BR_STRING_ON            dgettext("sys_string", "IDS_COM_BODY_ON")
#define BR_STRING_OFF           dgettext("sys_string", "IDS_COM_BODY_OFF")
#define BR_STRING_WARNING       dgettext("sys_string", "IDS_COM_POP_WARNING")
#define BR_STRING_ERROR         dgettext("sys_string", "IDS_COM_POP_ERROR")
#define BR_STRING_PROCESSING    dgettext("sys_string", "IDS_COM_POP_PROCESSING")
#define BR_STRING_MESSAGES      dgettext("sys_string", "IDS_COM_BODY_MESSAGES")
#define BR_STRING_EMAIL         dgettext("sys_string", "IDS_COM_BODY_EMAIL")
#define BR_STRING_SHARE         dgettext("sys_string", "IDS_COM_BUTTON_SHARE")
#define BR_STRING_BRIGHTNESS    dgettext("sys_string", "IDS_COM_OPT_BRIGHTNESS")
#define BR_STRING_AUTOMATIC     dgettext("sys_string", "IDS_COM_BODY_AUTOMATIC")
#define BR_STRING_INTERNET      dgettext("sys_string", "IDS_COM_BODY_INTERNET")
#define BR_STRING_CALL          dgettext("sys_string", "IDS_COM_BODY_CALL")
#define BR_STRING_NFC           dgettext("sys_string", "IDS_COM_BODY_NFC")
#define BR_STRING_DEFAULT       dgettext("sys_string", "IDS_COM_BODY_DEFAULT")
#define BR_STRING_LOW           dgettext("sys_string", "IDS_COM_BODY_LOW")

/* Need to fix */
#define BR_STRING_WARNING_VIDEO_PLAYER _("Can not launch video-player while video-call is running.")//STMS common
/*************************************************************************************************************/


/* Browser main view */
/*************************************************************************************************************/
#define BR_STRING_VIEW_CHANGE           _("IDS_BR_SK3_CHANGE_VIEW")
#define BR_STRING_NEW_WINDOW            _("IDS_BR_SK1_NEW_WINDOW")
#define BR_STRING_CREATE_FOLDER         _("IDS_BR_SK3_CREATE_FOLDER")
#define BR_STRING_FOLDER                _("IDS_BR_HEADER_FOLDER")
#define BR_STRING_HISTORY               _("IDS_BR_TAB_HISTORY")
#define BR_STRING_LOADING               _("IDS_BR_HEADER_LOADING_ING")
#define BR_STRING_ADD_TO_BOOKMARKS      _("IDS_BR_SK3_ADD_TO_BOOKMARKS")
#define BR_STRING_EDIT_BOOKMARK         _("IDS_BR_BODY_EDIT_BOOKMARK")
#define BR_STRING_TITLE                 _("IDS_BR_BODY_TITLE")
#define BR_STRING_URL                   _("IDS_BR_BODY_URL")
#define BR_STRING_SELECT_ALL            _("IDS_BR_OPT_SELECT_ALL")
#define BR_STRING_SELECT_ITEMS          _("IDS_BR_HEADER_SELECT_ITEMS")
#define BR_STRING_SELECT_HISTORIES      _("Select histories")//STMS, new text
#define BR_STRING_NO_BOOKMARKS          _("IDS_BR_BODY_NO_BOOKMARKS")
#define BR_STRING_NO_HISTORY            _("IDS_BR_BODY_NO_HISTORIES")
#define BR_STRING_SHARE_VIA_MESSAGE     _("IDS_BR_OPT_SHARE_VIA_MESSAGES")
#define BR_STRING_SHARE_VIA_EMAIL       _("IDS_BR_OPT_SHARE_VIA_EMAIL")
#define BR_STRING_SHARE_VIA             _("IDS_BR_OPT_SHARE_VIA")
#define BR_STRING_DOWNLOAD_MANAGER      _("IDS_BR_OPT_DOWNLOAD_MANAGER")
#define BR_STRING_UNLOCK                _("IDS_BR_OPT_UNLOCK")
#define BR_STRING_LOCK                  _("IDS_BR_OPT_LOCK")
#define BR_STRING_MODIFY                _("IDS_BR_BODY_MODIFY")
#define BR_STRING_REQUEST_LOCATION      _("IDS_BR_BODY_PS_REQUESTS_YOUR_LOCATION")
#define BR_STRING_ALLOW                 _("IDS_BR_OPT_ALLOW")
#define BR_STRING_FORWARD               _("IDS_BR_OPT_NAVIGATE_GO_FORWARD")//Not use
#define BR_STRING_REMEMBER_PREFERENCE   _("IDS_BR_BODY_REMEMBER_PREFERENCE")
#define BR_STRING_ITEMS_SELECTED        _("IDS_BR_POP_PD_ITEMS_SELECTED")
#define BR_STRING_ONE_ITEM_SELECTED     _("IDS_BR_POP_1_ITEM_SELECTED")
#define BR_STRING_EMPTY                 _("IDS_BR_BODY_EMPTY")
#define BR_STRING_URL_GUIDE_TEXT        _("IDS_BR_BODY_SEARCH_OR_ENTER_URL")
#define BR_STRING_AUTO                  _("IDS_BR_BODY_AUTO")
#define BR_STRING_MANUAL                _("IDS_BR_OPT_MANUAL")
/*************************************************************************************************************/


/* More menu */
/*************************************************************************************************************/
#define BR_STRING_FIND_ON_PAGE          _("IDS_BR_OPT_FIND_ON_PAGE")
//#define BR_STRING_WEB_CLIPPING        _("IDS_BR_OPT_ADD_TO_WEB_CLIP")
//share menu
#define BR_STRING_BOOKMARKS             _("IDS_BR_BODY_BOOKMARKS")
#define BR_STRING_BOOKMARK              _("IDS_BR_OPT_BOOKMARK")
#define BR_STRING_SAVE_FOR_OFFLINE_READING  _("IDS_BR_BODY_SAVE_FOR_OFFLINE_READING_ABB")//stms, new text
#define BR_STRING_PRIVATE                   dgettext("sys_string", "IDS_COM_BODY_PRIVATE")//private
//brightness
#define BR_STRING_HIGH                  _("IDS_BR_BODY_HIGH_MPRIORITY")
#define BR_STRING_MEDIUM                _("IDS_BR_BODY_MEDIUM")
#define BR_STRING_SETTINGS              _("IDS_BR_BODY_SETTINGS")
/*************************************************************************************************************/


/* Settings */
/*************************************************************************************************************/
#define BR_STRING_HOMEPAGE              _("IDS_BR_BODY_HOMEPAGE")
#if defined(FEATURE_MOST_VISITED_SITES)
#define BR_STRING_MOST_VISITED_SITES    _("IDS_BR_BODY_MOST_VISITED_SITES")
#endif
#define BR_STRING_BLANK_PAGE            _("IDS_BR_BODY_BLANK_PAGE")
#define BR_STRING_RECENTLY_VISITED_SITE     _("Recently visited site")//IDS_BR_HEADER_RECENTLY_VISITED_PAGES
#define BR_STRING_CURRENT_PAGE          _("IDS_BR_BODY_CURRENT_PAGE")
#define BR_STRING_USER_HOMEPAGE             _("User homepage")//not visible text
#define BR_STRING_CONTENT_SETTINGS      _("IDS_BR_MBODY_PAGE_CONTENT_SETTINGS")
#define BR_STRING_DEFAULT_VIEW_LEVEL    _("IDS_BR_BODY_DEFAULT_VIEW")//stms, new text
#define BR_STRING_FIT_TO_WIDTH          _("IDS_BR_BODY_FIT_TO_WIDTH")
#define BR_STRING_READABLE              _("IDS_BR_BODY_READABLE")
#define BR_STRING_RUN_JAVASCRIPT        _("IDS_BR_BODY_RUN_JAVASCRIPT")
#define BR_STRING_SHOW_IMAGES           _("IDS_BR_BODY_SHOW_IMAGES")
#define BR_STRING_ALWAYS_ON             _("IDS_BR_BODY_ALWAYS_ON")
#define BR_STRING_BLOCK_POPUP           _("Block popup")//stms, new text
#define BR_STRING_PRIVACY_AND_SECURTY       _("IDS_BR_BODY_PRIVACY_AND_SECURITY_ABB")
#define BR_STRING_ALWAYS_ASK                _("IDS_BR_BODY_ALWAYS_ASK")
#define BR_STRING_DELETE_BROWSING_DATA      _("IDS_BR_BODY_DELETE_BROWSING_DATA")
#define BR_STRING_CLEAR_CACHE           _("IDS_BR_BODY_CLEAR_CACHE")
#define BR_STRING_CLEAR_ALL_CACHE_DATA_Q      _("Clear all cache data?")//stms, new text
#define BR_STRING_CLEAR_HISTORY         _("IDS_BR_BODY_CLEAR_HISTORY")
#define BR_STRING_CLEAR_ALL_HISTORY_DATA_Q  _("Clear all history?")//stms, new text
#define BR_STRING_SHOW_SECURITY_WARNINGS        _("IDS_BR_BODY_SHOW_SECURITY_WARNINGS")
#define BR_STRING_COOKIES               _("IDS_BR_BODY_COOKIES")
#define BR_STRING_ACCEPT_COOKIES        _("IDS_BR_BODY_ACCEPT_COOKIES")
#define BR_STRING_CLEAR_ALL_COOKIE_DATA _("IDS_BR_BODY_CLEAR_ALL_COOKIE_DATA")
#define BR_STRING_CLEAR_ALL_COOKIE_DATA_Q   _("Clear all cookie data?")//stms, new text
#define BR_STRING_REMEMBER_FROM_DATA    _("IDS_BR_BODY_REMEMBER_FORM_DATA")
#define BR_STRING_CLEAR_FROM_DATA       _("Clear from data")//stms, new text
#define BR_STRING_CLEAR_ALL_FROM_DATA_Q _("Clear all from data?")//stms, new text
#define BR_STRING_LOCATION                  dgettext("sys_string", "IDS_COM_BODY_LOCATION")//_("Location")//stms, new text
#define BR_STRING_ENABLE_LOCATION       _("IDS_BR_BODY_ENABLE_LOCATION")
#define BR_STRING_CLEAR_LOCATION_ACCESS _("IDS_BR_BODY_CLEAR_LOCATION_ACCESS")
#define BR_STRING_CLEAR_ALL_LOCATION_DATA_Q     _("Clear all location data?")//stms, new text
#define BR_STRING_FORMDATA                  _("IDS_BR_HEADER_FORM_DATA")
#define BR_STRING_REMEMBER_FORMDATA         _("IDS_BR_BODY_REMEMBER_FORM_DATA")
#define BR_STRING_CLEAR_FORMDATA            _("IDS_BR_BODY_CLEAR_FORM_DATA")
#define BR_STRING_CLEAR_ALL_FORMDATA_Q _("Clear all form data?")
#define BR_STRING_PASSWORDS             _("Passwords")//stms, new text
#define BR_STRING_REMEMBER_PASSWORDS    _("IDS_BR_BODY_REMEMBER_PASSWORDS")
#define BR_STRING_CLEAR_PASSWORDS       _("IDS_BR_BODY_CLEAR_PASSWORDS")
#define BR_STRING_CLEAR_ALL_SAVED_PASSWORDS_Q   _("IDS_BR_POP_DELETE_ALL_SAVED_PASSWORDS_Q")
#define BR_STRING_SEARCH                _("IDS_BR_BODY_SEARCH")
#define BR_STRING_WEBSITE_SETTINGS      _("IDS_BR_BODY_WEBSITE_SETTINGS")//stms, new text
#define BR_STRING_RESET_TO_DEFAULT      _("IDS_BR_BODY_RESET_TO_DEFAULT")
#define BR_STRING_RESET_TO_DEFAULT_Q    _("Reset all settings to default?")//stms, new text
#define BR_STRING_RESET_ALL_SETTINGS_TO_DEFAULT_Q	_("IDS_BR_POP_ALL_SETTINGS_TO_DEFAULT_CONTINUE_Q")
#define BR_STRING_USER_AGENT            _("IDS_BR_HEADER_USER_AGENT")

//website settings
#define BR_STRING_ABOUT_BROWSER         _("IDS_BR_BODY_ABOUT_BROWSER")
#define BR_STRING_SAVED_ID_PASSWORD     _("IDS_BR_BODY_AUTO_SAVE_ID_PASSWORD")
#define BR_STRING_CLEAR_STORED_DATA     _("Clear stored data")
#define BR_STRING_CLEAR_LOCATION_ACCESS_FOR_THIS_WEBSITE     _("Clear location access for this website?")
#define BR_STRING_DELETE_ALL_STORED_DATA_BY_THIS_WEBSITE     _("Delete all data stored by this website?")
#define BR_STRING_DELETE_ALL_WEBSITE_DATA_AND_LOCATION_PERMISSIONS     _("Delete all website data and location permissions?")

//developer mode
#define BR_STRING_DEVELOPER_MODE                _("IDS_BR_BODY_DEVELOPER_MODE")
/*************************************************************************************************************/


/* Popup, inform and warnings */
/*************************************************************************************************************/
#define BR_STRING_DELETED               _("IDS_BR_POP_DELETED")
#define BR_STRING_ENTER_URL             _("IDS_BR_POP_ENTER_URL")
#define BR_STRING_ALREADY_EXISTS        _("IDS_BR_POP_ALREADY_EXISTS")
#define BR_STRING_FAILED                _("IDS_BR_POP_FAIL")
#define BR_STRING_SAVED                 _("IDS_BR_POP_SAVED")
#define BR_STRING_ENTER_TITLE           _("IDS_BR_BODY_ENTER_TITLE")
#define BR_STRING_ENTER_FOLDER_NAME     _("IDS_BR_BODY_ENTER_FOLDER_NAME")
#define BR_STRING_ENTER_BOOKMARK_NAME   _("IDS_BR_POP_ENTER_BOOKMARK_NAME")
#define BR_STRING_NETWORK_ERROR         _("IDS_BR_POP_NETWORK_ERROR")
#define BR_STRING_DO_NOT_ASK_AGAIN      _("IDS_BR_OPT_DO_NOT_ASK_AGAIN")
#define BR_STRING_NOT_FOUND_URL         _("IDS_BR_POP_URL_NOT_FOUND")
#define BR_STRING_REFUSE                _("IDS_BR_POP_REFUSE")
#define BR_STRING_AUTH_REQUIRED         _("IDS_BR_BODY_DESTINATIONS_AUTHENTICATION_REQUIRED")
#define BR_STRING_USER_NAME	            _("IDS_BR_BODY_AUTHUSERNAME")
#define BR_STRING_PASSWORD              _("IDS_BR_BODY_AUTHPASSWORD")

#define BR_STRING_NO_SEARCH_RESULT      _("No search result")//STMS request design ID

#define BR_STRING_DISPLAY_POPUP_Q	            _("This site is attempting to open pop-up window. Show pop-up?")
#define BR_STRING_PERMISSION_CHECK              _("Permission check")
#define BR_STRING_WEB_NOTIFICATION_Q            _("Do you want to receive web notification?")
#define BR_STRING_WEBPROCESS_CRASH              _("WebProcess is crashed")

#define BR_STRING_MSG_MDM_POLICY                _("Security policy restricts use of Internet browser")
#define BR_STRING_TITLE_SELECT_AN_ACTION        _("Select an action")//STMS, new text
#define BR_STRING_WARNING_OVER_BOOKMARK_LIMIT   _("IDS_BR_POP_UNABLE_TO_ADD_BOOKMARK_MAXIMUM_NUMBER_OF_BOOKMARKS_REACHED")
#define BR_STRING_MOST_VISITED_GUIDE_TEXT       _("Offer 3 thumbnails of most visited sites to let you access to each site easily")//stms, new text
#define BR_STRING_ADD_TO_MOST_VISITED_SITES     _("Add to Most visited sites")//STMS, new text
#define BR_STRING_MY_SITES_GUIDE_TEXT           _("You can add, delete, pin or unpin each item of \"My sites\" by doing a long press. <br>You can also reorder items of \"My sites\" by drag and drop.")//STMS, 
#define BR_STRING_ADDED_TO_BOOKMARKS            _("IDS_BR_POP_ADDED_TO_BOOKMARKS")
#define BR_STRING_REMOVED_TO_BOOKMARKS          _("IDS_BR_POP_REMOVED_FROM_BOOKMARKS")
// new text
#define BR_STRING_MSG_DELETE_WEBSITE_LOCATION_ACCESS_INFORMATION_Q  _("IDS_BR_POP_DELETE_WEBSITE_LOCATION_ACCESS_INFORMATION_Q")
/*************************************************************************************************************/


/* Others */
/*************************************************************************************************************/
#define BR_STRING_HOUR                  dgettext("sys_string", "IDS_COM_BODY_1_HOUR")
#define BR_STRING_HOURS                 dgettext("sys_string", "IDS_COM_POP_PD_HOURS")
#define BR_STRING_NEVER_AUTO_UPDATE     _("Never auto update")
/*************************************************************************************************************/


/* browser context menu */
#define BR_STRING_CTXMENU_OPEN_LINK_IN_NEW_WINDOW       _("IDS_BR_BODY_OPEN_IN_NEW_WINDOW")
#define BR_STRING_CTXMENU_DOWNLOAD_LINKED_FILE          _("IDS_BR_BODY_DOWNLOAD")
#define BR_STRING_CTXMENU_COPY_LINK_LOCATION            _("IDS_BR_OPT_COPY_LINK_URL")
#define BR_STRING_CTXMENU_OPEN_IMAGE_IN_NEW_WINDOW      _("IDS_BR_BODY_VIEW_IMAGE")
#define BR_STRING_CTXMENU_OPEN_MEDIA_IN_NEW_WINDOW      _("IDS_BR_BODY_PLAY")
#define BR_STRING_CTXMENU_COPY_IMAGE_LOCATION           _("IDS_BR_OPT_COPY_LINK_URL")
#define BR_STRING_CTXMENU_SAVE_IMAGE_AS                 _("Save image as")
#define BR_STRING_CTXMENU_COPY_IMAGE                    _("IDS_BR_OPT_COPY_IMAGE")
#define BR_STRING_CTXMENU_SAVE_IMAGE                    _("IDS_BR_OPT_SAVE_IMAGE")
#define BR_STRING_CTXMENU_SHARE_IMAGE                   _("Share image")
#define BR_STRING_CTXMENU_COPY_MEDIA                    dgettext("sys_string", "IDS_COM_BODY_COPY")
#define BR_STRING_CTXMENU_SEND_IMAGE_VIA_EMAIL          _("IDS_BR_OPT_SEND_VIA_EMAIL")
#define BR_STRING_CTXMENU_SEND_IMAGE_VIA_MESSAGE        _("IDS_BR_OPT_SEND_VIA_MESSAGE")
#define BR_STRING_CTXMENU_OPEN_VIDEO_IN_NEW_WINDOW      _("Open video in new window")//stms, new text
#define BR_STRING_CTXMENU_OPEN_AUDIO_IN_NEW_WINDOW      _("Open audio in new window")//stms, new text
#define BR_STRING_CTXMENU_COPY_VIDEO_LINK_LOCATION      _("IDS_BR_BODY_COPY_URL")//"Copy video link location"
#define BR_STRING_CTXMENU_COPY_AUDIO_LINK_LOCATION      _("IDS_BR_BODY_COPY_URL")//"Copy audio link location"
#define BR_STRING_CTXMENU_TOGGLE_MEDIA_CONTOLS          _("Toggle media controls")
#define BR_STRING_CTXMENU_TOGGLE_MEDIA_LOOP_PLAYBACK    _("Toggle media loop playback")
#define BR_STRING_CTXMENU_SWITCH_VIDEO_TO_FUUL_SCREEN   _("Switch video to fullscreen")
#define BR_STRING_CTXMENU_PLAY                          _("IDS_BR_BODY_PLAY")
#define BR_STRING_CTXMENU_PAUSE                         dgettext("sys_string", "IDS_COM_BODY_PAUSE")
#define BR_STRING_CTXMENU_MUTE                          dgettext("sys_string", "IDS_COM_BODY_MUTE")
#define BR_STRING_CTXMENU_OPEN_FRAME_IN_NEW_WINDOW      _("Open frame in new window")
#define BR_STRING_CTXMENU_MORE                          _("IDS_BR_BUTTON_MORE")
#define BR_STRING_CTXMENU_COPY                          dgettext("sys_string", "IDS_COM_BODY_COPY")
#define BR_STRING_CTXMENU_SEARCH                        dgettext("sys_string", "IDS_COM_BODY_SEARCH")
#define BR_STRING_CTXMENU_SHARE                         dgettext("sys_string", "IDS_COM_BUTTON_SHARE")
#define BR_STRING_CTXMENU_DELETE                        dgettext("sys_string", "IDS_COM_BODY_DELETE")
#define BR_STRING_CTXMENU_FIND_ON_PAGE                  _("IDS_BR_OPT_FIND_ON_PAGE")
#define BR_STRING_CTXMENU_PASTE_AND_GO                  _("Paste & Go")
#define BR_STRING_CTXMENU_SHARE_IMAGE_URL               _("Share image URL")
#define BR_STRING_CTXMENU_SHARE_LINK                    _("IDS_BR_BODY_SHARE_LINK")
#define BR_STRING_CTXMENU_SAVE_LINK                     _("IDS_BR_BODY_SAVE_LINK")
#define BR_STRING_CTXMENU_SELECT_ALL                    _("IDS_BR_OPT_SELECT_ALL")
#define BR_STRING_CTXMENU_SELECT                        _("IDS_BR_OPT_SELECT")
#define BR_STRING_CTXMENU_INSERT_UNICODE_CON_CHAR       _("Insert unicode control character")
#define BR_STRING_CTXMENU_INPUT_METHOD                  _("Input methods")
#define BR_STRING_CTXMENU_GO_BACK                       _("IDS_BR_OPT_NAVIGATE_GO_BACKWARD")
#define BR_STRING_CTXMENU_GO_FORWARD                    _("IDS_BR_OPT_NAVIGATE_GO_FORWARD")
#define BR_STRING_CTXMENU_STOP                          _("IDS_BR_OPT_STOP")
#define BR_STRING_CTXMENU_RELOAD                        _("IDS_BR_OPT_RELOAD")
#define BR_STRING_CTXMENU_CUT                           dgettext("sys_string", "IDS_COM_BODY_CUT")
#define BR_STRING_CTXMENU_PASTE                         _("IDS_BR_OPT_PASTE")
#define BR_STRING_CTXMENU_SELECT_WORD                   _("IDS_BR_BODY_SELECT_WORD")
#define BR_STRING_CTXMENU_NO_GUESS_FOUND                _("No guesses found")
#define BR_STRING_CTXMENU_IGNORE_SPELLING               _("Ignore spelling")
#define BR_STRING_CTXMENU_LEARN_SPELLING                _("Learn spelling")
#define BR_STRING_CTXMENU_SEARCH_THE_WEB                _("Search the Web")
#define BR_STRING_CTXMENU_SEARCH_IN_SPOTLIGHT           _("Search in Spotlight")
#define BR_STRING_CTXMENU_LOCK_UP_IN_DIRECTORY          _("Look up in dictionary")
#define BR_STRING_CTXMENU_OPEN                          _("IDS_BR_OPT_OPEN")
#define BR_STRING_CTXMENU_IGNORE_GRAMMAR                _("Ignore grammar")
#define BR_STRING_CTXMENU_SPELLING_AND_GRAMMAR          _("Spelling and grammar")
#define BR_STRING_CTXMENU_SHOW_SPELLING_AND_GRAMMAR     _("Show spelling and grammar")
#define BR_STRING_CTXMENU_HIDE_SPELLING_AND_GRAMMAR     _("Hide spelling and grammar")
#define BR_STRING_CTXMENU_CHECK_DOCUMENT_NOW            _("Check document now")
#define BR_STRING_CTXMENU_CHECK_SPELLING_WHILE_TYPEING  _("Check spelling while _Typing")
#define BR_STRING_CTXMENU_CHECK_GRAMMAR_WITH_SPELLING   _("Check grammar with spelling")
#define BR_STRING_CTXMENU_SHOW_FONTS                    _("Show fonts")
#define BR_STRING_CTXMENU_FONT                          _("Font")
#define BR_STRING_CTXMENU_BOLD                          _("Bold")
#define BR_STRING_CTXMENU_ITALIC                        _("Italic")
#define BR_STRING_CTXMENU_OUTLINE                       _("Outline")
#define BR_STRING_CTXMENU_UNDERLINE                     _("Underline")
#define BR_STRING_CTXMENU_SHOW_COLORS                   _("Show colors")
#define BR_STRING_CTXMENU_OUTLINE                       _("Outline")
#define BR_STRING_CTXMENU_STYLE                         _("Style")
#define BR_STRING_CTXMENU_INPECT_ELEMENT                _("Inspect element")
#define BR_STRING_CTXMENU_MISSING_PLUGIN                _("missing plugin")
#define BR_STRING_CTXMENU_PLUGIN_CRASHED                _("plugin crashed")
#define BR_STRING_CTXMENU_SPEECH                        _("Speech")
#define BR_STRING_CTXMENU_START_SPEAKING                _("Start speaking")
#define BR_STRING_CTXMENU_STOP_SPEAKING                 _("Stop speaking")
#define BR_STRING_CTXMENU_SELECTION_MODE                _("Selection mode")

#endif /* BROWSER_STRING_H */
