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


#include "browser-context-menu.h"
#include "browser-view.h"

Browser_Context_Menu::Browser_Context_Menu(Browser_View *browser_view)
:	m_browser_view(browser_view)
	,m_ewk_view(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Context_Menu::~Browser_Context_Menu(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

void Browser_Context_Menu::init(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	deinit();

	m_ewk_view = ewk_view;

	evas_object_smart_callback_add(ewk_view, "contextmenu,customize", __get_context_menu_from_proposed_context_menu_cb, this);
	evas_object_smart_callback_add(ewk_view, "contextmenu,selected", __custom_context_menu_item_selected_cb, this);
}

void Browser_Context_Menu::deinit(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_ewk_view) {
		evas_object_smart_callback_del(m_ewk_view, "contextmenu,customize",
						__get_context_menu_from_proposed_context_menu_cb);
		evas_object_smart_callback_del(m_ewk_view, "contextmenu,selected",
						__custom_context_menu_item_selected_cb);
	}
}

Eina_Bool Browser_Context_Menu::launch_context_menu(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!menu)
		return EINA_FALSE;

	switch (_check_context_menu_mode(menu)) {
	case CUSTOM_CONTEXT_MENU_TEXT_ONLY_MODE:
		_append_text_only_mode_context_menu(menu);
		break;

	case CUSTOM_CONTEXT_MENU_HYPERLINK_ONLY_MODE:
		_append_hyperlink_only_mode_context_menu(menu);
		break;

	case CUSTOM_CONTEXT_MENU_TEXT_WITH_HYPERLINK_MODE:
		_append_text_with_hyperlink_mode_context_menu(menu);
		break;

	case CUSTOM_CONTEXT_MENU_IMAGE_ONLY_MODE:
		_append_image_only_mode_context_menu(menu);
		break;

	case CUSTOM_CONTEXT_MENU_IMAGE_WITH_HYPERLINK_MODE:
		_append_image_with_hyperlink_mode_context_menu(menu);
		break;

	case CUSTOM_CONTEXT_MENU_TEXT_AND_IMAGE_SELECTION_MODE:
		/* To do */
		break;

	default:
		_append_normal_mode_context_menu(menu);
		break;
	}
}

void Browser_Context_Menu::__get_context_menu_from_proposed_context_menu_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!event_info)
		return;

	if (!data)
		return;

	Ewk_Context_Menu *menu = static_cast<Ewk_Context_Menu*>(event_info);
	Browser_Context_Menu *context_menu = static_cast<Browser_Context_Menu *>(data);
	context_menu->launch_context_menu(menu);
}

void Browser_Context_Menu::__custom_context_menu_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!event_info)
		return;

	Ewk_Context_Menu_Item* item = static_cast<Ewk_Context_Menu_Item*>(event_info);
	Browser_Context_Menu *context_menu = static_cast<Browser_Context_Menu *>(data);

	std::string link_url_string = ewk_context_menu_item_link_url_get(item);
	BROWSER_LOGD("link url=[%s]", link_url_string.c_str());
	std::string image_url_string = ewk_context_menu_item_image_url_get(item);
	BROWSER_LOGD("image url=[%s]", image_url_string.c_str());

	const char *selected_text = ewk_view_text_selection_text_get(context_menu->m_ewk_view);

	Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);
	switch (tag) {
	case CUSTOM_CONTEXT_MENU_ITEM_FIND_ON_TAG:
		BROWSER_LOGD("Find selected text[%s] on the page", selected_text);
		if (selected_text && strlen(selected_text) > 0)
			context_menu->find_word_with_text(selected_text);
		break;
	default:
		break;
	}
}

Eina_Bool Browser_Context_Menu::_check_context_menu_mode(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!menu)
		return EINA_FALSE;

	int count = ewk_context_menu_item_count(menu);

	Eina_Bool has_text = EINA_FALSE;
	Eina_Bool has_image = EINA_FALSE;
	Eina_Bool has_hyperlink = EINA_FALSE;

	Ewk_Context_Menu_Item *item;
	Ewk_Context_Menu_Item_Tag tag;

	for (int i = 0; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, i);
		tag = ewk_context_menu_item_tag_get(item);
		BROWSER_LOGD("tag[%d]", tag);

		if ((tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY)
		|| (tag == EWK_CONTEXT_MENU_ITEM_TAG_CUT)
		|| (tag == EWK_CONTEXT_MENU_ITEM_TAG_PASTE)
		|| (tag == EWK_CONTEXT_MENU_ITEM_TAG_SELECT_ALL)
		|| (tag == EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_WEB)
		|| (tag == EWK_CONTEXT_MENU_ITEM_TAG_SELECT_WORD))
			has_text = EINA_TRUE;

		if ((tag == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK)
		|| (tag == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW)
		|| (tag == EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK)
		|| (tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD))
			has_hyperlink = EINA_TRUE;

		if ((tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD)
		|| (tag == EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK)
		|| (tag == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_FRAME_IN_NEW_WINDOW)
		|| (tag == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW))
			has_image = EINA_TRUE;
	}

	if (has_text == EINA_TRUE && has_hyperlink == EINA_FALSE && has_image == EINA_FALSE) {
		BROWSER_LOGD("CUSTOM_CONTEXT_MENU_TEXT_ONLY_MODE");
		return CUSTOM_CONTEXT_MENU_TEXT_ONLY_MODE;
	} else if (has_text == EINA_TRUE && has_hyperlink == EINA_FALSE && has_image == EINA_TRUE) {
		BROWSER_LOGD("CUSTOM_CONTEXT_MENU_TEXT_AND_IMAGE_SELECTION_MODE");
		return CUSTOM_CONTEXT_MENU_TEXT_AND_IMAGE_SELECTION_MODE;
	} else if (has_text == EINA_TRUE && has_hyperlink == EINA_TRUE && has_image == EINA_TRUE) {
		BROWSER_LOGD("CUSTOM_CONTEXT_MENU_TEXT_WITH_HYPERLINK_MODE");
		return CUSTOM_CONTEXT_MENU_TEXT_WITH_HYPERLINK_MODE;
	} else if (has_text == EINA_FALSE && has_hyperlink == EINA_TRUE && has_image == EINA_FALSE) {
		BROWSER_LOGD("CUSTOM_CONTEXT_MENU_HYPERLINK_ONLY_MODE");
		return CUSTOM_CONTEXT_MENU_HYPERLINK_ONLY_MODE;
	} else if (has_text == EINA_FALSE && has_hyperlink == EINA_FALSE && has_image == EINA_TRUE){
		BROWSER_LOGD("CUSTOM_CONTEXT_MENU_IMAGE_ONLY_MODE");
		return CUSTOM_CONTEXT_MENU_IMAGE_ONLY_MODE;
	} else if (has_text == EINA_FALSE && has_hyperlink == EINA_TRUE && has_image == EINA_TRUE){
		BROWSER_LOGD("CUSTOM_CONTEXT_MENU_IMAGE_WITH_HYPERLINK_MODE");
		return CUSTOM_CONTEXT_MENU_IMAGE_WITH_HYPERLINK_MODE;
	} else{
		BROWSER_LOGD("CUSTOM_CONTEXT_MENU_MODE_INDEX_END");
		return CUSTOM_CONTEXT_MENU_MODE_INDEX_END;
	}
}

Eina_Bool Browser_Context_Menu::_append_text_only_mode_context_menu(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!menu)
		return EINA_FALSE;

	Ewk_Context_Menu_Item *item;
	Ewk_Context_Menu_Item_Tag tag;
	int index = 0;
	int count = ewk_context_menu_item_count(menu);
	int *custom_context_menu_array = NULL;

	if (count == 0)
		return EINA_FALSE;
	custom_context_menu_array = (int *)malloc(sizeof(int) * count);
	if (!custom_context_menu_array)
		return EINA_FALSE;

	memset(custom_context_menu_array, 0x00, sizeof(int) * count);

	/* To re-arrange menu order to follow UX guideline
		: copy menu would be best if it's at the first in context menu */
	for (int i = 0; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, i);
		tag = ewk_context_menu_item_tag_get(item);

		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY) {
			custom_context_menu_array[index++] = EWK_CONTEXT_MENU_ITEM_TAG_COPY;
			ewk_context_menu_item_remove(menu, item);
		}
	}
#if 0
	for (int i = 0; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, i);
		tag = ewk_context_menu_item_tag_get(item);

		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_CUT) {
			custom_context_menu_array[index++] = EWK_CONTEXT_MENU_ITEM_TAG_CUT;
			ewk_context_menu_item_remove(menu, item);
		}
	}
#endif

	for (int i = index; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, 0);
		tag = ewk_context_menu_item_tag_get(item);

		custom_context_menu_array[index++] = tag;
		ewk_context_menu_item_remove(menu, item);
	}

	for (int i = 0; i < count; i++) {
		tag = custom_context_menu_array[i];

		switch (tag) {
		case EWK_CONTEXT_MENU_ITEM_TAG_CUT:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_CUT, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_PASTE:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_PASTE, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_COPY, true);
			ewk_context_menu_item_append_as_action(menu, CUSTOM_CONTEXT_MENU_ITEM_FIND_ON_TAG, BR_STRING_CTXMENU_FIND_ON_PAGE, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_SELECT_ALL:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_SELECT_ALL, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_SELECT_WORD:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_SELECT_WORD, true);
			break;
		default:
			break;
		}
	}

	if (custom_context_menu_array)
		free(custom_context_menu_array);
	custom_context_menu_array = NULL;

	return EINA_TRUE;
}

Eina_Bool Browser_Context_Menu::_append_text_with_hyperlink_mode_context_menu(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!menu)
		return EINA_FALSE;

	return _append_hyperlink_only_mode_context_menu(menu);
}

Eina_Bool Browser_Context_Menu::_append_hyperlink_only_mode_context_menu(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!menu)
		return EINA_FALSE;

	Ewk_Context_Menu_Item *item;
	Ewk_Context_Menu_Item_Tag tag;
	int index = 0;
	int count = ewk_context_menu_item_count(menu);
	int *custom_context_menu_array = (int *)malloc(sizeof(int) * count);

	if (count == 0)
		return EINA_FALSE;

	if (!custom_context_menu_array)
		return EINA_FALSE;

	memset(custom_context_menu_array, 0x00, sizeof(int) * count);

	/* To re-arrange menu order to follow UX guideline
		: "Open in new window" menu would be best if it's at the first in context menu */
	for (int i = 0; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, i);
		tag = ewk_context_menu_item_tag_get(item);

		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK) {
			custom_context_menu_array[index++] = EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK;
			ewk_context_menu_item_remove(menu, item);
		}
	}

	for (int i = 0; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, i);
		tag = ewk_context_menu_item_tag_get(item);

		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW) {
			custom_context_menu_array[index++] = EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW;
			ewk_context_menu_item_remove(menu, item);
		}
	}

	for (int i = 0; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, i);
		tag = ewk_context_menu_item_tag_get(item);

		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD) {
			custom_context_menu_array[index++] = EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD;
			ewk_context_menu_item_remove(menu, item);
		}
	}

	for (int i = index; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, 0);
		tag = ewk_context_menu_item_tag_get(item);

		custom_context_menu_array[index++] = tag;
		ewk_context_menu_item_remove(menu, item);
	}

	for (int i = 0; i < count; i++) {
		tag = custom_context_menu_array[i];

		switch (tag) {

		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_OPEN, true);
			break;

		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_OPEN_LINK_IN_NEW_WINDOW, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_COPY_LINK_LOCATION, true);
			break;
#if 0
		case EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_SAVE_LINK, true);
			break;
#endif
		default:
			break;
		}
	}

	if (custom_context_menu_array)
		free(custom_context_menu_array);
	custom_context_menu_array = NULL;

	return EINA_TRUE;
}

Eina_Bool Browser_Context_Menu::_append_image_only_mode_context_menu(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!menu)
		return EINA_FALSE;

	Ewk_Context_Menu_Item *item;
	Ewk_Context_Menu_Item_Tag tag;
	int index = 0;
	int count = ewk_context_menu_item_count(menu);
	int *custom_context_menu_array = (int *)malloc(sizeof(int) * count);

	if (count == 0)
		return EINA_FALSE;

	if (!custom_context_menu_array)
		return EINA_FALSE;

	memset(custom_context_menu_array, 0x00, sizeof(int) * count);

	/* To re-arrange menu order to follow UX guideline
		: "Copy image" menu would be best if it's at the first in context menu */
	for (int i = 0; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, i);
		tag = ewk_context_menu_item_tag_get(item);

		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD) {
			custom_context_menu_array[index++] = EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD;
			ewk_context_menu_item_remove(menu, item);
		}
	}

	for (int i = index; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, 0);
		tag = ewk_context_menu_item_tag_get(item);

		custom_context_menu_array[index++] = tag;
		ewk_context_menu_item_remove(menu, item);
	}

	for (int i = 0; i < count; i++) {
		tag = custom_context_menu_array[i];

		switch (tag) {
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_COPY_IMAGE, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_OPEN_IMAGE_IN_NEW_WINDOW, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_SAVE_IMAGE, true);
			break;
		default:
			break;
		}
	}

	if (custom_context_menu_array)
		free(custom_context_menu_array);
	custom_context_menu_array = NULL;

	return EINA_TRUE;
}

Eina_Bool Browser_Context_Menu::_append_image_with_hyperlink_mode_context_menu(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!menu)
		return EINA_FALSE;

	Ewk_Context_Menu_Item *item;
	Ewk_Context_Menu_Item_Tag tag;
	int index = 0;
	int count = ewk_context_menu_item_count(menu);
	int *custom_context_menu_array = (int *)malloc(sizeof(int) * count);

	if (count == 0)
		return EINA_FALSE;

	if (!custom_context_menu_array)
		return EINA_FALSE;

	memset(custom_context_menu_array, 0x00, sizeof(int) * count);

	/* To re-arrange menu order to follow UX guideline
		: "Open in new window" menu would be best if it's at the first in context menu */
	for (int i = 0; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, i);
		tag = ewk_context_menu_item_tag_get(item);

		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW) {
			custom_context_menu_array[index++] = EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW;
			ewk_context_menu_item_remove(menu, item);
		}
	}

	for (int i = 0; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, i);
		tag = ewk_context_menu_item_tag_get(item);

		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD) {
			custom_context_menu_array[index++] = EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD;
			ewk_context_menu_item_remove(menu, item);
		}
	}

	for (int i = 0; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, i);
		tag = ewk_context_menu_item_tag_get(item);

		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD) {
			custom_context_menu_array[index++] = EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD;
			ewk_context_menu_item_remove(menu, item);
		}
	}

	for (int i = index; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, 0);
		tag = ewk_context_menu_item_tag_get(item);

		custom_context_menu_array[index++] = tag;
		ewk_context_menu_item_remove(menu, item);
	}

	for (int i = 0; i < count; i++) {
		tag = custom_context_menu_array[i];

		switch (tag) {
		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_OPEN_LINK_IN_NEW_WINDOW, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_COPY_LINK_LOCATION, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_COPY_IMAGE, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_OPEN_IMAGE_IN_NEW_WINDOW, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_SAVE_IMAGE, true);
			break;
		default:
			break;
		}
	}

	if (custom_context_menu_array)
		free(custom_context_menu_array);
	custom_context_menu_array = NULL;

	return EINA_TRUE;
}

Eina_Bool Browser_Context_Menu::_append_text_and_image_selection_mode_context_menu(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!menu)
		return EINA_FALSE;

	return EINA_FALSE;
}

Eina_Bool Browser_Context_Menu::_append_normal_mode_context_menu(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!menu)
		return EINA_FALSE;

	Ewk_Context_Menu_Item *item;
	Ewk_Context_Menu_Item_Tag tag;
	bool show_hyperlink_item_flag = false;
	bool show_imagemode_flag = false;
	int count = ewk_context_menu_item_count(menu);

	for (int i = 0; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, 0);
		tag = ewk_context_menu_item_tag_get(item);
		ewk_context_menu_item_remove(menu, item);

		switch (tag) {
		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_OPEN, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_OPEN_LINK_IN_NEW_WINDOW, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_SAVE_LINK, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_OPEN_IMAGE_IN_NEW_WINDOW, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_SAVE_IMAGE, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_COPY_IMAGE, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_COPY_IMAGE_LOCATION, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_CUT:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_CUT, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_PASTE:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_PASTE, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_COPY, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_SELECT_ALL:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_SELECT_ALL, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_SELECT_WORD:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_SELECT_WORD, true);
			break;
		default:
			break;
		}
	}

	return EINA_TRUE;
}


