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

#ifndef BROWSER_SCISSORBOX_VIEW_H
#define BROWSER_SCISSORBOX_VIEW_H

#include "browser-config.h"

class Browser_View;
class Browser_Scissorbox_View {
public:
	Browser_Scissorbox_View(Browser_View *browser_view);
	~Browser_Scissorbox_View(void);

	Eina_Bool init(void);
	Evas_Object *get_layout(void) { return m_scissorbox; }
private:
	Eina_Bool _create_main_layout(void);

	static void __scissorbox_changed_cb(void *data,
						Evas_Object *obj, void *event_info);

	Evas_Object *m_scissorbox;
	Browser_View *m_browser_view;
};

#endif /* BROWSER_SCISSORBOX_VIEW_H */

