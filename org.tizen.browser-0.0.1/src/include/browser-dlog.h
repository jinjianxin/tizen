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


#ifndef BROWSER_DLOG_H
#define BROWSER_DLOG_H

#include <dlog.h>
/*
#define BROWSER_LOGD(fmt, args...) LOGD("[%s: %s: %d] "fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args)
#define BROWSER_LOGI(fmt, args...) LOGI("[%s: %s: %d] "fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args)
#define BROWSER_LOGW(fmt, args...) LOGW("[%s: %s: %d] "fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args)
#define BROWSER_LOGE(fmt, args...) LOGE("[%s: %s: %d] "fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args)
#define BROWSER_LOGE_IF(cond, fmt, args...) LOGE_IF(cond, "[%s: %s: %d] "fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args)
*/

#define BROWSER_LOGD(fmt, args...) printf("[%s: %s: %d ] \n "fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args,"\n")
#define BROWSER_LOGI(fmt, args...) printf("[%s: %s: %d ] \n "fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args,"\n")
#define BROWSER_LOGW(fmt, args...) printf("[%s: %s: %d ] \n"fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args,"\n")
#define BROWSER_LOGE(fmt, args...) printf("[%s: %s: %d ] \n"fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args)
#define BROWSER_LOGE_IF(cond, fmt, args...) printf(cond, "[%s: %s: %d:] \n"fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args)


#if defined(BROWSER_MEMALLOC_ABORT_ON)
#define new new(nothrow)
#define RET_NEWALLOC_FAILED(expr) do {\
	if ((expr) == NULL) {\
		BROWSER_LOGE("[%s: %s: %d] [Error] NULL returned!- blue screen is launched!",\
			(rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__),  __FUNCTION__,  __LINE__);\
		abort();\
		return;\
	}\
} while (0)
#else
#define RET_NEWALLOC_FAILED(expr) do {\
	if ((expr) == NULL) {\
		BROWSER_LOGE("[%s: %s: %d] [Error] NULL returned!- blue screen is launched!",\
			(rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__);\
		return;\
	}\
} while (0)
#endif

#if defined(BROWSER_MEMALLOC_ABORT_ON)
#define RETV_NEWALLOC_FAILED(expr,val) do {\
	if ((expr) == NULL) {\
		BROWSER_LOGE("[%s: %s: %d] [Error] NULL returned!- blue screen is launched!",\
			(rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__),  __FUNCTION__,  __LINE__);\
		abort();\
		return (val);\
	}\
} while (0)
#else
#define RETV_NEWALLOC_FAILED(expr,val) do {\
	if ((expr) == NULL) {\
		BROWSER_LOGE("[%s: %s: %d] [Error] NULL returned!- blue screen is launched!",\
			(rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__);\
		return (val);\
	}\
} while (0)
#endif

#define RET_PARAM_INVALID(expr) do {\
	if ((expr) == NULL) {\
		BROWSER_LOGE("[%s: %s: %d]invalid parameter!", (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__,\
			__LINE__);\
		return;\
	}\
} while (0)

#endif /* BROWSER_DLOG_H */

