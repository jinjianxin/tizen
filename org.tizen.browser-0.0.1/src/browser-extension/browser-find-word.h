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


#ifndef BROWSER_FIND_WORD_H
#define BROWSER_FIND_WORD_H

#include "browser-config.h"
#include "browser-view.h"

#include <string>

using namespace std;

class Browser_Find_Word {
public:
	Browser_Find_Word(Browser_View *browser_view);
	~Browser_Find_Word(void);

	typedef enum _Find_Word_Direction {
		BROWSER_FIND_WORD_FORWARD,
		BROWSER_FIND_WORD_BACKWARD
	} Find_Word_Direction;

	int find_word(const char *word, Find_Word_Direction direction);
	void init_index();
	int get_match_max_value();

private:
	static void __did_find_string_cb(Evas_Object* o, const char* string, int match_count, void* user_data);

	int m_find_word_index;
	int m_find_word_max_count;
	Browser_View *m_browser_view;
};

#endif /* BROWSER_FIND_WORD_H */

