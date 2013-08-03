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


#ifndef BROWSER_CONTEXT_MENU_H
#define BROWSER_CONTEXT_MENU_H

#include "browser-config.h"
#include "browser-common-view.h"
#include "browser-utility.h"

class Browser_View;

class Browser_Context_Menu : public Browser_Common_View {
public:
	Browser_Context_Menu(Browser_View *browser_view);
	~Browser_Context_Menu(void);

	Eina_Bool init(void) { return EINA_TRUE; }
	void init(Evas_Object *ewk_view);
	void deinit(void);
	Eina_Bool launch_context_menu(Ewk_Context_Menu *menu);
	int get_pressed_position_x(void) { return m_pressed_x; }
	int get_pressed_position_y(void) { return m_pressed_y; }
	void set_pressed_position_x(int pressed_x) { m_pressed_x = pressed_x; }
	void set_pressed_position_y(int pressed_y) { m_pressed_y = pressed_y; }

	typedef enum _custom_context_menu_mode {
		CUSTOM_CONTEXT_MENU_MODE_INDEX_START = 0,
		CUSTOM_CONTEXT_MENU_TEXT_ONLY_MODE,
		CUSTOM_CONTEXT_MENU_TEXT_WITH_HYPERLINK_MODE,
		CUSTOM_CONTEXT_MENU_IMAGE_ONLY_MODE,
		CUSTOM_CONTEXT_MENU_IMAGE_WITH_HYPERLINK_MODE,
		CUSTOM_CONTEXT_MENU_HYPERLINK_ONLY_MODE,
		CUSTOM_CONTEXT_MENU_TEXT_AND_IMAGE_SELECTION_MODE,

		CUSTOM_CONTEXT_MENU_MODE_INDEX_END

	} custom_context_menu_mode;

	typedef enum _custom_context_menu_item_tag {
		CUSTOM_CONTEXT_MENU_ITEM_BASE_TAG = EWK_CONTEXT_MENU_ITEM_BASE_APPLICATION_TAG,
		CUSTOM_CONTEXT_MENU_ITEM_FIND_ON_TAG,
		CUSTOM_CONTEXT_MENU_ITEM_SELECTION_MODE_FROM_TEXT_TAG,
		CUSTOM_CONTEXT_MENU_ITEM_SELECTION_MODE_FROM_IMAGE_TAG,
		CUSTOM_CONTEXT_MENU_ITEM_SELECTION_MODE_FROM_TEXT_WITH_HYPERLINK_TAG,
		CUSTOM_CONTEXT_MENU_ITEM_SELECTION_MODE_FROM_IMAGE_WITH_HYPERLINK_TAG,
#if defined(FEATURE_SEARCH_SELECTED_WORD_IN_WEB) && defined(FEATURE_TEXT_READER) && !defined(TIZEN_PUBLIC)
		CUSTOM_CONTEXT_MENU_ITEM_SEARCH_THE_WEB_IN_READER_VIEW_TAG,
#endif
		CUSTOM_CONTEXT_MENU_ITEM_TAG_INDEX_END

	} custom_context_menu_item_tag;

private:
	static void __get_context_menu_from_proposed_context_menu_cb(void *data, Evas_Object *obj, void *event_info);
	static void __custom_context_menu_item_selected_cb(void *data, Evas_Object *obj, void *event_info);

	Eina_Bool _check_context_menu_mode(Ewk_Context_Menu *menu);
	Eina_Bool _append_text_only_mode_context_menu(Ewk_Context_Menu *menu);
	Eina_Bool _append_text_with_hyperlink_mode_context_menu(Ewk_Context_Menu *menu);
	Eina_Bool _append_hyperlink_only_mode_context_menu(Ewk_Context_Menu *menu);
	Eina_Bool _append_image_only_mode_context_menu(Ewk_Context_Menu *menu);
	Eina_Bool _append_image_with_hyperlink_mode_context_menu(Ewk_Context_Menu *menu);
	Eina_Bool _append_text_and_image_selection_mode_context_menu(Ewk_Context_Menu *menu);
	Eina_Bool _append_normal_mode_context_menu(Ewk_Context_Menu *menu);

	Browser_View *m_browser_view;
	Evas_Object *m_ewk_view;
	int m_pressed_x;
	int m_pressed_y;
};
#endif /* BROWSER_CONTEXT_MENU_H */

