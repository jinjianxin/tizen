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
  *@file browser-utility.cpp
  *@brief browser 工具函数
  */

extern "C" {
#include <Elementary.h>
}

#include "browser-utility.h"
#include "app.h" /* for preference CAPI */

static void __changed_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *layout = (Evas_Object *)data;

    void *erase = evas_object_data_get(layout, "eraser");
    BROWSER_LOGD("erase = %d", erase);
    if (!erase)
        return;

    if (elm_object_focus_get(layout)) {
        if (elm_entry_is_empty(obj))
            elm_object_signal_emit(layout, "elm,state,eraser,hide", "elm");
        else {
            elm_object_signal_emit(layout, "elm,state,eraser,show", "elm");
        }
    }
    if (!elm_entry_is_empty(obj))
        elm_object_signal_emit(layout, "elm,state,guidetext,hide", "elm");
}

static void __focused_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *layout = (Evas_Object *)data;

    void *erase = evas_object_data_get(layout, "eraser");
    BROWSER_LOGD("erase = %d", erase);

    if (!elm_entry_is_empty(obj) && erase)
        elm_object_signal_emit(layout, "elm,state,eraser,show", "elm");

    elm_object_signal_emit(layout, "elm,state,guidetext,hide", "elm");
}

static void __unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *layout = (Evas_Object *)data;

    if (elm_entry_is_empty(obj))
        elm_object_signal_emit(layout, "elm,state,guidetext,show", "elm");

    elm_object_signal_emit(layout, "elm,state,eraser,hide", "elm");
}

static void __url_entry_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *layout = (Evas_Object *)data;

    void *erase = evas_object_data_get(layout, "eraser");
    BROWSER_LOGD("erase = %d", erase);

    if (!elm_entry_is_empty(obj) && erase)
        elm_object_signal_emit(layout, "elm,state,eraser,show", "elm");

    elm_object_signal_emit(layout, "elm,state,guidetext,hide", "elm");
    //	edje_object_signal_emit(elm_layout_edje_get(layout), "ellipsis_hide,signal", "");
    elm_object_signal_emit(layout, "ellipsis_hide,signal", "elm");

    int *cursor_position = (int *)evas_object_data_get(obj, "cursor_position");
    BROWSER_LOGD("cursor_position = %d", cursor_position);
    elm_entry_cursor_pos_set(obj, (int)cursor_position);
}

static void __url_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *layout = (Evas_Object *)data;

    if (elm_entry_is_empty(obj))
        elm_object_signal_emit(layout, "elm,state,guidetext,show", "elm");

    elm_object_signal_emit(layout, "elm,state,eraser,hide", "elm");

    //	edje_object_signal_emit(elm_layout_edje_get(layout), "ellipsis_show,signal", "");
    elm_object_signal_emit(layout, "ellipsis_show,signal", "elm");

    evas_object_data_set(obj, "cursor_position", (void *)elm_entry_cursor_pos_get(obj));

    BROWSER_LOGD("cursor_position = %d", elm_entry_cursor_pos_get(obj));
}

static void __eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    Evas_Object *entry = (Evas_Object *)data;
    elm_entry_entry_set(entry, "");
}

/**
 * @brief br_elm_url_editfield_add 返回一个url输入框 在browser-view.cpp +2741
 * @param parent 父类容器
 * @return 返回一个layout布局
 */
Evas_Object *br_elm_url_editfield_add(Evas_Object *parent)
{
    Evas_Object *layout = elm_layout_add(parent);

    elm_layout_theme_set(layout, "layout", "browser-editfield", "default");

    Evas_Object *entry = elm_entry_add(parent);

    elm_entry_single_line_set(entry, EINA_TRUE);
    elm_entry_scrollable_set(entry, EINA_TRUE);

    elm_entry_text_style_user_push(entry, "DEFAULT='font_size=35 color=#3C3632 ellipsis=1'");
    elm_object_part_content_set(layout, "elm.swallow.content", entry);

    evas_object_smart_callback_add(entry, "changed", __changed_cb, layout);
    evas_object_smart_callback_add(entry, "focused", __url_entry_focused_cb, layout);
    evas_object_smart_callback_add(entry, "unfocused", __url_entry_unfocused_cb, layout);
    elm_object_signal_callback_add(layout, "elm,eraser,clicked", "elm",
                                   __eraser_clicked_cb, entry);

    elm_object_signal_emit(layout, "ellipsis_show,signal", "elm");

    return layout;
}

Evas_Object *br_elm_find_word_editfield_add(Evas_Object *parent)
{
    Evas_Object *layout = elm_layout_add(parent);

    elm_layout_theme_set(layout, "layout", "browser-editfield", "default");

    Evas_Object *entry = elm_entry_add(parent);

    elm_entry_single_line_set(entry, EINA_TRUE);
    elm_entry_scrollable_set(entry, EINA_TRUE);

    elm_entry_text_style_user_push(entry, "color=#000000 ellipsis=1'");
    elm_object_part_content_set(layout, "elm.swallow.content", entry);

    evas_object_smart_callback_add(entry, "changed", __changed_cb, layout);
    evas_object_smart_callback_add(entry, "focused", __focused_cb, layout);
    evas_object_smart_callback_add(entry, "unfocused", __unfocused_cb, layout);
    elm_object_signal_callback_add(layout, "elm,eraser,clicked", "elm",
                                   __eraser_clicked_cb, entry);

    elm_object_signal_emit(layout, "ellipsis_hide,signal", "elm");

    return layout;
}

Evas_Object *br_elm_editfield_add(Evas_Object *parent, Eina_Bool title)
{
    Evas_Object *layout = elm_layout_add(parent);
    if (title)
        elm_layout_theme_set(layout, "layout", "editfield", "title");
    else
        elm_layout_theme_set(layout, "layout", "editfield", "default");

    Evas_Object *entry = elm_entry_add(parent);
    elm_object_part_content_set(layout, "elm.swallow.content", entry);

    elm_object_signal_emit(layout, "elm,state,guidetext,hide", "elm");
    evas_object_data_set(layout, "eraser", (void *)EINA_TRUE);

    evas_object_smart_callback_add(entry, "changed", __changed_cb, layout);
    evas_object_smart_callback_add(entry, "focused", __focused_cb, layout);
    evas_object_smart_callback_add(entry, "unfocused", __unfocused_cb, layout);
    elm_object_signal_callback_add(layout, "elm,eraser,clicked", "elm",
                                   __eraser_clicked_cb, entry);

    return layout;
}

void br_elm_editfield_label_set(Evas_Object *obj, const char *label)
{
    elm_object_part_text_set(obj, "elm.text", label);
}

Evas_Object *br_elm_editfield_entry_get(Evas_Object *obj)
{
    return elm_object_part_content_get(obj, "elm.swallow.content");
}

void br_elm_editfield_guide_text_set(Evas_Object *obj, const char *text)
{
    elm_object_part_text_set(obj, "elm.guidetext", text);
}

void br_elm_editfield_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line)
{
    Evas_Object *entry = br_elm_editfield_entry_get(obj);
    elm_entry_single_line_set(entry, EINA_TRUE);
    elm_entry_scrollable_set(entry, EINA_TRUE);
}

void br_elm_editfield_eraser_set(Evas_Object *obj, Eina_Bool visible)
{
    evas_object_data_set(obj, "eraser", (void *)visible);

    if (visible)
        elm_object_signal_emit(obj, "elm,state,eraser,show", "elm");
    else
        elm_object_signal_emit(obj, "elm,state,eraser,hide", "elm");
}


static void __searchbar_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *layout = (Evas_Object *)data;

    if (elm_object_focus_get(layout)) {
        if (elm_entry_is_empty(obj))
            elm_object_signal_emit(layout, "elm,state,eraser,hide", "elm");
        else {
            elm_object_signal_emit(layout, "elm,state,eraser,show", "elm");
        }
    }
    if (!elm_entry_is_empty(obj))
        elm_object_signal_emit(layout, "elm,state,guidetext,hide", "elm");
}

static void __searchbar_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *layout = (Evas_Object *)data;

    if (!elm_entry_is_empty(obj))
        elm_object_signal_emit(layout, "elm,state,eraser,show", "elm");

    elm_object_signal_emit(layout, "elm,state,guidetext,hide", "elm");
    elm_object_signal_emit(layout, "cancel,in", "");
}

static void __searchbar_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *layout = (Evas_Object *)data;

    if (elm_entry_is_empty(obj))
        elm_object_signal_emit(layout, "elm,state,guidetext,show", "elm");
    elm_object_signal_emit(layout, "elm,state,eraser,hide", "elm");
}

static void __searchbar_eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    Evas_Object *entry = (Evas_Object *)data;

    elm_entry_entry_set(entry, "");
}

static void __searchbar_bg_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    Evas_Object *entry = (Evas_Object *)data;

    elm_object_focus_set(entry, EINA_TRUE);
}

static void __searchbar_cancel_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *searchbar_layout = (Evas_Object *)data;
    Evas_Object *entry = elm_object_part_content_get(searchbar_layout, "elm.swallow.content");
    const char* text;
    evas_object_hide(obj);
    elm_object_signal_emit(searchbar_layout, "cancel,out", "");

    text = elm_entry_entry_get(entry);
    if (text != NULL && strlen(text) > 0)
        elm_entry_entry_set(entry, NULL);

    elm_object_focus_set(entry, EINA_FALSE);
}

/**
 * @brief __search_entry_clicked_cb entry 回调函数
 * @param data
 * @param obj
 * @param emission
 * @param source
 */
static void __search_entry_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    Evas_Object *entry = (Evas_Object *)data;
    if (!elm_object_focus_allow_get(entry)) {
        elm_object_focus_allow_set(entry, EINA_TRUE);
        elm_object_focus_set(entry, EINA_TRUE);
    }
}

/**
 * @brief br_elm_searchbar_add 创建搜索条
 * @param parent 父对象
 * @return  返回创建成功的父对象
 */
Evas_Object *br_elm_searchbar_add(Evas_Object *parent)
{
    Evas_Object *searchbar_layout = elm_layout_add(parent);
    elm_layout_theme_set(searchbar_layout, "layout", "searchbar", "cancel_button");
    Evas_Object *entry = elm_entry_add(searchbar_layout);
    elm_entry_scrollable_set(entry, EINA_TRUE);
    elm_entry_single_line_set(entry, EINA_TRUE);
    elm_object_part_content_set(searchbar_layout, "elm.swallow.content", entry);
    elm_object_part_text_set(searchbar_layout, "elm.guidetext", BR_STRING_SEARCH);

    /* Workaround
      * When tab history view, the search entry has focus automatically.
      * So the keypad is shown. So give focus manually. */
    edje_object_signal_callback_add(elm_layout_edje_get(searchbar_layout),
                                    "mouse,clicked,1", "elm.swallow.content", __search_entry_clicked_cb, entry);

    elm_object_focus_allow_set(entry, EINA_FALSE);

    Evas_Object *cancel_button = elm_button_add(searchbar_layout);
    elm_object_part_content_set(searchbar_layout, "button_cancel", cancel_button);
    elm_object_style_set(cancel_button, "searchbar/default");
    elm_object_text_set(cancel_button, BR_STRING_CANCEL);
    evas_object_smart_callback_add(cancel_button, "clicked", __searchbar_cancel_clicked_cb, searchbar_layout);

    evas_object_smart_callback_add(entry, "changed", __searchbar_changed_cb, searchbar_layout);
    evas_object_smart_callback_add(entry, "focused", __searchbar_focused_cb, searchbar_layout);
    evas_object_smart_callback_add(entry, "unfocused", __searchbar_unfocused_cb, searchbar_layout);
    elm_object_signal_callback_add(searchbar_layout, "elm,eraser,clicked", "elm",
                                   __searchbar_eraser_clicked_cb, entry);
    elm_object_signal_callback_add(searchbar_layout, "elm,bg,clicked", "elm", __searchbar_bg_clicked_cb, entry);

    return searchbar_layout;
}

/**
 * @brief br_elm_searchbar_text_set 设置entry文件
 * @param obj layout对象
 * @param text 要设置的文本
 */
void br_elm_searchbar_text_set(Evas_Object *obj, const char *text)
{
    Evas_Object *entry = elm_object_part_content_get(obj, "elm.swallow.content");
    elm_object_text_set(entry, text);
}

/**
 * @brief br_elm_searchbar_text_get 获得entry文件
 * @param obj layout对象
 * @return  返回取得字符串
 */
char *br_elm_searchbar_text_get(Evas_Object *obj)
{
    Evas_Object *entry = elm_object_part_content_get(obj, "elm.swallow.content");
    const char *text = elm_object_text_get(entry);
    BROWSER_LOGD("search entry text=[%s]", text);

    if (text && strlen(text))
        return strdup(text);
    else
        return NULL;
}

/**
 * @brief br_elm_searchbar_entry_get 获得entry对象
 * @param obj layout对象
 * @return 返回获得成功的layput对象
 */
Evas_Object *br_elm_searchbar_entry_get(Evas_Object *obj)
{
    return elm_object_part_content_get(obj, "elm.swallow.content");
}

#if defined(GENLIST_SWEEP)
void br_elm_genlist_sweep_item_recover(Evas_Object *obj)
{
    if (obj == NULL) {
        BROWSER_LOGD("obj is NULL");
        return;
    }

    Elm_Object_Item *it = (Elm_Object_Item *) elm_genlist_decorated_item_get(obj);

    /* Finish genlist sweep*/
    if (it) {
        elm_genlist_item_decorate_mode_set(it, "slide", EINA_FALSE);
        elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DEFAULT);
        elm_genlist_item_update(it);
    }
}
#endif

/**
 * @brief br_preference_set_bool 创建bool配置文件键值对
 * @param key 键名
 * @param value 要设置的值
 * @return 是否创建成功
 */
bool br_preference_set_bool(const char *key, bool value)
{
    int ret;
    ret = preference_set_boolean(key, value);
    switch (ret) {
    case PREFERENCE_ERROR_NONE:
        return true;
    case PREFERENCE_ERROR_INVALID_PARAMETER:
        BROWSER_LOGE("Can not get [%s] value. Invalid parameter\n", key);
        return false;
    case PREFERENCE_ERROR_IO_ERROR:
        BROWSER_LOGE("Can not get [%s] value. Internal IO error\n", key);
        return false;
    default:
        return false;
    }
    return true;
}

/**
 * @brief br_preference_get_bool 获得设置的键值对
 * @param key 键名
 * @param value 要获得的值
 * @return 是获取成功
 */
bool br_preference_get_bool(const char *key, bool *value)
{
    int ret = preference_get_boolean(key, value);
    switch (ret) {
    case PREFERENCE_ERROR_NONE:
        return true;
    case PREFERENCE_ERROR_INVALID_PARAMETER:
        BROWSER_LOGE("Can not get [%s] value. Invalid parameter\n", key);
        return false;
    case PREFERENCE_ERROR_NO_KEY:
        BROWSER_LOGE("Can not get [%s] value. Required key not available\n", key);
        return false;
    case PREFERENCE_ERROR_IO_ERROR:
        BROWSER_LOGE("Can not get [%s] value. Internal IO error\n", key);
        return false;
    default:
        return false;
    }
    return true;
}

/**
 * @brief br_preference_create_bool 创建一个bool类型的键值对
 * @param key 键名
 * @param value 要创建的值
 * @return 是否创建成功
 */
bool br_preference_create_bool(const char *key, bool value)
{
    BROWSER_LOGD("%s(%d)", key, value);
    bool existing = false;
    int ret = preference_is_existing(key, &existing);
    switch (ret) {
    case PREFERENCE_ERROR_NONE:
        if (!existing)
            preference_set_boolean(key, value);
        return true;
    case PREFERENCE_ERROR_INVALID_PARAMETER:
        BROWSER_LOGE("Can not initialize [%s] value. Invalid parameter\n", key);
        return false;
    case PREFERENCE_ERROR_IO_ERROR:
        BROWSER_LOGE("Can not initialize [%s] value. Internal IO error\n", key);
        return false;
    default:
        return false;
    }
    return true;
}

bool br_preference_set_int(const char *key, int value)
{
    int ret;
    ret = preference_set_int(key, value);
    switch (ret) {
    case PREFERENCE_ERROR_NONE:
        return true;
    case PREFERENCE_ERROR_INVALID_PARAMETER:
        BROWSER_LOGE("Can not get [%s] value. Invalid parameter\n", key);
        return false;
    case PREFERENCE_ERROR_IO_ERROR:
        BROWSER_LOGE("Can not get [%s] value. Internal IO error\n", key);
        return false;
    default:
        return false;
    }
    return true;
}

bool br_preference_get_int(const char *key, int *value)
{
    int ret = preference_get_int(key, value);
    switch (ret) {
    case PREFERENCE_ERROR_NONE:
        return true;
    case PREFERENCE_ERROR_INVALID_PARAMETER:
        BROWSER_LOGE("Can not get [%s] value. Invalid parameter\n", key);
        return false;
    case PREFERENCE_ERROR_NO_KEY:
        BROWSER_LOGE("Can not get [%s] value. Required key not available\n", key);
        return false;
    case PREFERENCE_ERROR_IO_ERROR:
        BROWSER_LOGE("Can not get [%s] value. Internal IO error\n", key);
        return false;
    default:
        return false;
    }
    return true;
}

bool br_preference_create_int(const char *key, int value)
{
    BROWSER_LOGD("%s(%d)", key, value);
    bool existing = false;
    int ret = preference_is_existing(key, &existing);
    switch (ret) {
    case PREFERENCE_ERROR_NONE:
        if (!existing)
            preference_set_int(key, value);
        return true;
    case PREFERENCE_ERROR_INVALID_PARAMETER:
        BROWSER_LOGE("Can not initialize [%s] value. Invalid parameter\n", key);
        return false;
    case PREFERENCE_ERROR_IO_ERROR:
        BROWSER_LOGE("Can not initialize [%s] value. Internal IO error\n", key);
        return false;
    default:
        return false;
    }
    return true;
}

bool br_preference_set_str(const char *key, const char *value)
{
    int ret;
    ret = preference_set_string(key, value);
    switch (ret) {
    case PREFERENCE_ERROR_NONE:
        return true;
    case PREFERENCE_ERROR_INVALID_PARAMETER:
        BROWSER_LOGE("Can not get [%s] value. Invalid parameter\n", key);
        return false;
    case PREFERENCE_ERROR_IO_ERROR:
        BROWSER_LOGE("Can not get [%s] value. Internal IO error\n", key);
        return false;
    default:
        return false;
    }
    return true;
}

bool br_preference_get_str(const char *key, char **value)
{
    int ret = preference_get_string(key, value);
    switch (ret) {
    case PREFERENCE_ERROR_NONE:
        return true;
    case PREFERENCE_ERROR_INVALID_PARAMETER:
        BROWSER_LOGE("Can not get [%s] value. Invalid parameter\n", key);
        return false;
    case PREFERENCE_ERROR_OUT_OF_MEMORY:
        BROWSER_LOGE("Can not get [%s] value. Out of Memory\n", key);
        return false;
    case PREFERENCE_ERROR_NO_KEY:
        BROWSER_LOGE("Can not get [%s] value. Required key not available\n", key);
        return false;
    case PREFERENCE_ERROR_IO_ERROR:
        BROWSER_LOGE("Can not get [%s] value. Internal IO error\n", key);
        return false;
    default:
        return false;
    }
    return true;
}

bool br_preference_create_str(const char *key, const char *value)
{
    BROWSER_LOGD("%s(%s)", key, value);
    bool existing = false;
    int ret = preference_is_existing(key, &existing);
    switch (ret) {
    case PREFERENCE_ERROR_NONE:
        if (!existing)
            preference_set_string(key, value);
        return true;
    case PREFERENCE_ERROR_INVALID_PARAMETER:
        BROWSER_LOGE("Can not initialize [%s] value. Invalid parameter\n", key);
        return false;
    case PREFERENCE_ERROR_IO_ERROR:
        BROWSER_LOGE("Can not initialize [%s] value. Internal IO error\n", key);
        return false;
    default:
        return false;
    }
    return true;
}

bool br_preference_set_changed_cb(const char *key, br_preference_changed_cb callback, void *user_data)
{
    BROWSER_LOGD("[%s]", key);
    int ret = preference_set_changed_cb(key, (preference_changed_cb)callback, user_data);
    switch (ret) {
    case PREFERENCE_ERROR_NONE:
        return true;
    case PREFERENCE_ERROR_INVALID_PARAMETER:
        BROWSER_LOGE("Can not set [%s] callback. Invalid parameter\n", key);
        return false;
    case PREFERENCE_ERROR_OUT_OF_MEMORY:
        BROWSER_LOGE("Can not set [%s] callback. Out of Memory\n", key);
        return false;
    case PREFERENCE_ERROR_NO_KEY:
        BROWSER_LOGE("Can not set [%s] callback. Required key not available\n", key);
        return false;
    case PREFERENCE_ERROR_IO_ERROR:
        BROWSER_LOGE("Can not set [%s] callback. Internal IO error\n", key);
        return false;
    default:
        return false;
    }
    return true;
}

bool br_preference_unset_changed_cb(const char *key)
{
    BROWSER_LOGD("[%s]", key);
    int ret = preference_unset_changed_cb(key);
    switch (ret) {
    case PREFERENCE_ERROR_NONE:
        return true;
    case PREFERENCE_ERROR_INVALID_PARAMETER:
        BROWSER_LOGE("Can not unset [%s] callback. Invalid parameter\n", key);
        return false;
    case PREFERENCE_ERROR_IO_ERROR:
        BROWSER_LOGE("Can not unset [%s] callback. Internal IO error\n", key);
        return false;
    default:
        return false;
    }
    return true;
}

