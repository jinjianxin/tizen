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


#ifndef BROWSER_UTILITY_H
#define BROWSER_UTILITY_H

#include "browser-config.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef void(*br_preference_changed_cb )(const char *key, void *user_data);

Evas_Object *br_elm_url_editfield_add(Evas_Object *parent);
Evas_Object *br_elm_editfield_add(Evas_Object *parent, Eina_Bool title = EINA_FALSE);
void br_elm_editfield_label_set(Evas_Object *obj, const char *label);
Evas_Object *br_elm_editfield_entry_get(Evas_Object *obj);
void br_elm_editfield_guide_text_set(Evas_Object *obj, const char *text);
void br_elm_editfield_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line);
void br_elm_editfield_eraser_set(Evas_Object *obj, Eina_Bool visible);
Evas_Object *br_elm_searchbar_add(Evas_Object *parent);
void br_elm_searchbar_text_set(Evas_Object *obj, const char *text);
char *br_elm_searchbar_text_get(Evas_Object *obj);
Evas_Object *br_elm_searchbar_entry_get(Evas_Object *obj);
Evas_Object *br_elm_find_word_editfield_add(Evas_Object *parent);

#if defined(GENLIST_SWEEP)
void br_elm_genlist_sweep_item_recover(Evas_Object *obj);
#endif

/* preference APIs */
bool br_preference_set_bool(const char *key, bool value);
bool br_preference_get_bool(const char *key, bool *value);
bool br_preference_create_bool(const char *key, bool value);
bool br_preference_set_int(const char *key, int value);
bool br_preference_get_int(const char *key, int *value);
bool br_preference_create_int(const char *key, int value);
bool br_preference_set_str(const char *key, const char *value);
bool br_preference_get_str(const char *key, char **value);
bool br_preference_create_str(const char *key, const char *value);
bool br_preference_set_changed_cb(const char *key, br_preference_changed_cb callback, void *user_data);
bool br_preference_unset_changed_cb(const char *key);

#ifdef __cplusplus
}
#endif

#endif /* BROWSER_UTILITY_H */

