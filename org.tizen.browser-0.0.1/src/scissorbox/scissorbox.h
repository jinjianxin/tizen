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


#ifndef SCISSORBOX_H
#define SCISSORBOX_H

#include "browser-config.h"

#ifdef __cplusplus
extern "C" {
#endif

Evas_Object *br_scissorbox_add(Evas_Object *parent);

void br_scissorbox_set_object(Evas_Object *obj, int x, int y, int width, int height);
void br_scissorbox_bound_set(const Evas_Object *obj, int x, int y, int w, int h);
void br_scissorbox_region_set(const Evas_Object *obj, int x, int y, int w, int h);
void br_scissorbox_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h);

#ifdef __cplusplus
}
#endif

#endif /* SCISSORBOX_H*/

