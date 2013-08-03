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


extern "C" {
#include <Elementary.h>
#include <elm_widget.h>
}

#include "scissorbox.h"

#define CROP_POT_PATH 			BROWSER_IMAGE_DIR"/T01_CallerID_normal.png"
#define CROP_POT_HOLD_PATH 		BROWSER_IMAGE_DIR"/T01_CallerID_hold.png"
#define CROP_POT_PRESS_PATH 	BROWSER_IMAGE_DIR"/T01_CallerID_press.png"

#define CROP_LINEW_PATH 		BROWSER_IMAGE_DIR"/T01_CallerID_line_W.png"
#define CROP_LINEH_PATH 		BROWSER_IMAGE_DIR"/T01_CallerID_line_h.png"
#define CROP_LINEW_PRESS_PATH 	BROWSER_IMAGE_DIR"/T01_CallerID_line_W_press.png"
#define CROP_LINEH_PRESS_PATH 	BROWSER_IMAGE_DIR"/T01_CallerID_line_h_press.png"

#define DEFAULT_LINE_SIZE	(2 * elm_scale_get())

#define DEFAULT_MIN_RECT_SIZE		(10 * elm_scale_get())
#define DEFAULT_MIN_EVENT_RECT_SIZE	(45 * elm_scale_get())

#define DEFAULT_DIM_BG_ALPHA	102

#define DEFAULT_POT_NUM		4 /* Be careful: Don't change this number */

typedef enum {
	CENTER_POT,
	LEFT_TOP_POT,
	RIGHT_TOP_POT,
	RIGHT_BOTTOM_POT,
	LEFT_BOTTOM_POT
}pot_type;


typedef struct _Widget_Data Widget_Data;

struct _Widget_Data {
	Evas *evas;

	Evas_Object *parent;
	Evas_Object *obj;

	Evas_Object *bg;
	Evas_Object *selector_line[DEFAULT_POT_NUM];	/*  Line object*/
	Evas_Object *selector_pot[DEFAULT_POT_NUM];	/*  Gripper object*/

	Evas_Object *bg_grid[DEFAULT_POT_NUM];	/*  Rect object*/
	Evas_Object *event_pot[DEFAULT_POT_NUM];

	Evas_Coord_Rectangle rect;
	Evas_Coord_Rectangle bound;

	int pot_w, pot_h;
	int event_pot_w, event_pot_h; //Event object on image icon

	Evas_Coord touch_prev_x, touch_prev_y;
	Eina_Bool bResizing;
	pot_type pressed_pot;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void _del_hook(Evas_Object *obj)
{
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get(obj);
	if (!wd)
		return;

	evas_object_del(wd->bg);
	wd->bg = NULL;

	int i;
	for (i = 0; i < DEFAULT_POT_NUM; i++) {
		evas_object_del(wd->selector_line[i]);
		evas_object_del(wd->selector_pot[i]);
		evas_object_del(wd->bg_grid[i]);
		evas_object_del(wd->event_pot[i]);
	}

	free(wd);

}

static void _theme_hook(Evas_Object *obj)
{
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get(obj);
	if (!wd)
		return;
}


static void _sub_del(void *data, Evas_Object *obj, void *event_info)
{
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get(obj);

	if (!wd)
		return;
}


static void _sizing_eval(Evas_Object *obj)
{
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get(obj);
	if (!wd)
		return;
	if (!wd->rect.w || !wd->rect.h)
		return;

	BROWSER_LOGD("Resizing. XYWH(%d,%d,%d,%d)", wd->rect.x, wd->rect.y,
		    wd->rect.w, wd->rect.h);

	evas_object_move(wd->bg, wd->rect.x, wd->rect.y);
	evas_object_resize(wd->bg, wd->rect.w, wd->rect.h);

	/* Adjust dim bg object*/
	int px = 0;
	int py = 0;
	int pw = 0;
	int ph = 0;
	evas_object_geometry_get(wd->parent, &px, &py, &pw, &ph);
	BROWSER_LOGD("parent. XYWH(%d,%d,%d,%d)", px, py, pw, ph);

	evas_object_move(wd->bg_grid[0], px, py);
	evas_object_resize(wd->bg_grid[0], pw, wd->rect.y - py);

	evas_object_move(wd->bg_grid[1], wd->rect.x + wd->rect.w, wd->rect.y);
	evas_object_resize(wd->bg_grid[1], pw - (wd->rect.x + wd->rect.w), wd->rect.h);

	evas_object_move(wd->bg_grid[2], px, wd->rect.y + wd->rect.h);
	evas_object_resize(wd->bg_grid[2], pw, (ph + py) - (wd->rect.y + wd->rect.h));

	evas_object_move(wd->bg_grid[3], px, wd->rect.y);
	evas_object_resize(wd->bg_grid[3], wd->rect.x, wd->rect.h);

	/* Adjust line object */
	evas_object_move(wd->selector_line[0], wd->rect.x, wd->rect.y);
	evas_object_resize(wd->selector_line[0], wd->rect.w, DEFAULT_LINE_SIZE);

	evas_object_move(wd->selector_line[1], wd->rect.x + wd->rect.w, wd->rect.y);
	evas_object_resize(wd->selector_line[1], DEFAULT_LINE_SIZE, wd->rect.h);

	evas_object_move(wd->selector_line[2], wd->rect.x, wd->rect.y + wd->rect.h);
	evas_object_resize(wd->selector_line[2], wd->rect.w, DEFAULT_LINE_SIZE);

	evas_object_move(wd->selector_line[3], wd->rect.x, wd->rect.y);
	evas_object_resize(wd->selector_line[3], DEFAULT_LINE_SIZE, wd->rect.h);

	int x, y;

	/* Adjust image icon (pot) object */
	x = wd->rect.x - (wd->pot_w / 2);
	y = wd->rect.y - (wd->pot_h / 2);

	evas_object_move(wd->selector_pot[0], x, y);
	evas_object_move(wd->selector_pot[1], x + wd->rect.w, y);
	evas_object_move(wd->selector_pot[2], x + wd->rect.w, y + wd->rect.h);
	evas_object_move(wd->selector_pot[3], x, y + wd->rect.h);

	/* Adjust event  object */
	x = wd->rect.x - (wd->event_pot_w / 2);
	y = wd->rect.y - (wd->event_pot_h / 2);
	evas_object_move(wd->event_pot[0], x, y);
	evas_object_move(wd->event_pot[1], x + wd->rect.w, y);
	evas_object_move(wd->event_pot[2], x + wd->rect.w, y + wd->rect.h);
	evas_object_move(wd->event_pot[3], x, y + wd->rect.h);
}

static void
_selector_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get((Evas_Object *)data);
	if (!wd)
		return;

	BROWSER_LOGD(" inside ...");
	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *) event_info;

	wd->touch_prev_x = ev->output.x;
	wd->touch_prev_y = ev->output.y;
}

static Eina_Bool
_is_selector_movable(const Evas_Object *obj, Evas_Coord move_x,
		     Evas_Coord move_y, Evas_Coord *result_x, Evas_Coord *result_y)
{
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get(obj);
	if (!wd)
		return EINA_FALSE;

	if(!result_x || !result_y)
		return EINA_FALSE;

	Evas_Coord dst_x, dst_y;

	dst_x = wd->rect.x + move_x;
	dst_y = wd->rect.y + move_y;

	*result_x = move_x;
	*result_y = move_y;

	if (dst_x < wd->bound.x && move_x < 0) {
		/*  left bound*/
		BROWSER_LOGD("left bounded: wd->bound.x is %d", wd->bound.x);
		*result_x = wd->bound.x - wd->rect.x;
	}

	if (dst_y < wd->bound.y && move_y < 0) {
		/*  top bound*/
		BROWSER_LOGD("top bounded: wd->bound.y is %d", wd->bound.y);
		*result_y = wd->bound.y - wd->rect.y;
	}

	if ((dst_x + wd->rect.w >= wd->bound.x + wd->bound.w) && move_x > 0) {
		/*  right bound*/
		BROWSER_LOGD("right bound: [wd->bound.x,wd->bound.w] is [%d,%d]",
			     wd->bound.x, wd->bound.w);
		*result_x = wd->bound.x + wd->bound.w - wd->rect.x - wd->rect.w;
	}

	if ((dst_y + wd->rect.h >= wd->bound.y + wd->bound.h) && move_y > 0) {
		/*  bottom bound*/
		BROWSER_LOGD("bottom bound: [wd->bound.y,wd->bound.h] is [%d,%d]",
			     wd->bound.y, wd->bound.h);
		*result_y =  wd->bound.y + wd->bound.h - wd->rect.y - wd->rect.h;
	}

	if (*result_x || *result_y) {
		BROWSER_LOGD("[result_x, result_y] is [%d, %d]", *result_x, *result_y);
		return EINA_TRUE;
	} else {
		BROWSER_LOGD("result_x & result_y are all 0");
		return EINA_FALSE;
	}
}


static void
_selector_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *myobj = (Evas_Object *)data;

	Widget_Data *wd = (Widget_Data *)elm_widget_data_get(myobj);

	if (!wd)
		return;

	if (wd->bResizing)
		return;

	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *) event_info;
	if (!wd->touch_prev_x || !wd->touch_prev_y) {
		wd->touch_prev_x = ev->cur.output.x;
		wd->touch_prev_y = ev->cur.output.y;
		return;
	}

	Evas_Coord mov_x = 0, mov_y = 0;
	mov_x = ev->cur.output.x - wd->touch_prev_x;
	mov_y = ev->cur.output.y - wd->touch_prev_y;

	BROWSER_LOGD("[cur_x, cur_y: %d, %d] [mov_x, mov_y: %d, %d]",
			ev->cur.output.x, ev->cur.output.y, mov_x, mov_y);

	wd->touch_prev_x = ev->cur.output.x;
	wd->touch_prev_y = ev->cur.output.y;

	Evas_Coord result_x = 0;
	Evas_Coord result_y = 0;
	if (_is_selector_movable(myobj, mov_x, mov_y, &result_x, &result_y) == TRUE) {
		wd->rect.x += result_x;
		wd->rect.y += result_y;

		_sizing_eval(wd->obj);
	}
}

static void
_selector_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *myobj = (Evas_Object *)data;

	Widget_Data *wd = (Widget_Data *)elm_widget_data_get(myobj);
	if (!wd)
		return;

/*       ELM_MSG_HIGH( " ...");*/

	wd->touch_prev_x = 0;
	wd->touch_prev_y = 0;

	evas_object_smart_callback_call(wd->obj, "changed", NULL);
}

static void
_pot_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get((Evas_Object *)data);
	if (!wd)
		return;

	wd->bResizing = TRUE;

	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *) event_info;

	int i;

	if(ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) {
		for (i = 0; i < DEFAULT_POT_NUM; i++){
			evas_object_image_file_set(wd->selector_pot[i], CROP_POT_HOLD_PATH, NULL);
		}
	} else {
		for (i = 0; i < DEFAULT_POT_NUM; i++){
			evas_object_image_file_set(wd->selector_pot[i], CROP_POT_PRESS_PATH, NULL);
		}
	}

	wd->touch_prev_x = ev->output.x;
	wd->touch_prev_y = ev->output.y;

	Evas_Coord center_x, center_y;

	center_x = wd->rect.x + wd->rect.w / 2;
	center_y = wd->rect.y + wd->rect.h / 2;

	if(wd->touch_prev_x < center_x && wd->touch_prev_y < center_y) {
		wd->pressed_pot = LEFT_TOP_POT;
	}else if(wd->touch_prev_x > center_x && wd->touch_prev_y > center_y) {
		wd->pressed_pot = RIGHT_BOTTOM_POT;
	}else if(wd->touch_prev_x > center_x && wd->touch_prev_y < center_y) {
		wd->pressed_pot = RIGHT_TOP_POT;
	}else if(wd->touch_prev_x < center_x && wd->touch_prev_y > center_y) {
		wd->pressed_pot = LEFT_BOTTOM_POT;
	}else {
		wd->pressed_pot = CENTER_POT;
	}

}


typedef enum
{
	Direction_X,
	Direction_Y
}dominant_direction_type;

static void
_pot_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get((Evas_Object *)data);
	if (!wd)
		return;

	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *) event_info;

	Evas_Coord x, y, w, h;
	evas_object_geometry_get(wd->bg, &x, &y, &w, &h);

	Evas_Coord mov_x = 0;
	Evas_Coord mov_y = 0;
	Evas_Coord mov_dist = 0;

	mov_x = ev->cur.output.x - wd->touch_prev_x;
	mov_y = ev->cur.output.y - wd->touch_prev_y;

	dominant_direction_type dd = Direction_X;
	if(abs(mov_x) < abs(mov_y)) {
		dd = Direction_Y;
		mov_dist = mov_y;
	} else {
		mov_dist = mov_x;
	}

        BROWSER_LOGD(" *** [mov_x, mov_y] = [%d, %d] \n", mov_x, mov_y);

	if(abs(mov_dist) < 2)
		return;

	Evas_Coord_Rectangle rect;

	switch(wd->pressed_pot)
	{
		case LEFT_TOP_POT:
			rect.x = x + mov_x;
			rect.y = y + mov_y;
			rect.w = w - mov_x;
			rect.h = h - mov_y;
			break;
		case RIGHT_BOTTOM_POT:
			rect.x = x;
			rect.y = y;
			rect.w = w + mov_x;
			rect.h = h + mov_y;
			break;
		case RIGHT_TOP_POT:
			rect.x = x;
			rect.y = y + mov_y;
			rect.w = w + mov_x;
			rect.h = h - mov_y;
			break;
		case LEFT_BOTTOM_POT:
			rect.x = x + mov_x;
			rect.y = y;
			rect.w = w - mov_x;
			rect.h = h + mov_y;
			break;
		default:
			return;
	}

	if (rect.w <= DEFAULT_MIN_EVENT_RECT_SIZE ||
	    rect.h <= DEFAULT_MIN_EVENT_RECT_SIZE)
		return;

	if (rect.x < wd->bound.x ||
	    rect.y < wd->bound.y ||
	    rect.x + rect.w > wd->bound.x + wd->bound.w ||
	    rect.y + rect.h > wd->bound.y + wd->bound.h )
		return;

	wd->touch_prev_x = ev->cur.output.x;
	wd->touch_prev_y = ev->cur.output.y;

	memcpy(&wd->rect, &rect, sizeof(Evas_Coord_Rectangle));

	_sizing_eval(wd->obj);
}


static void
_pot_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get((Evas_Object *)data);
	if (!wd)
		return;

	BROWSER_LOGD(" ***inside ...");
	int i;
	for (i = 0; i < DEFAULT_POT_NUM; i++){
		evas_object_image_file_set(wd->selector_pot[i], CROP_POT_PATH, NULL);
	}

	wd->touch_prev_x = 0;
	wd->touch_prev_y = 0;

	wd->bResizing = FALSE;

	evas_object_smart_callback_call(wd->obj, "changed", NULL);
}

static void
_line_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get((Evas_Object *)data);
	if (!wd)
		return;

	int i;
	for (i = 0; i < DEFAULT_POT_NUM; i++){
		if(i % 2)
			evas_object_image_file_set(wd->bg_grid[i], CROP_LINEW_PRESS_PATH, NULL);
		else
			evas_object_image_file_set(wd->bg_grid[i], CROP_LINEH_PRESS_PATH, NULL);
	}
}

static void
_line_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get((Evas_Object *)data);
	if (!wd)
		return;
	int i;
	for (i = 0; i < DEFAULT_POT_NUM; i++){
		if(i % 2)
			evas_object_image_file_set(wd->bg_grid[i], CROP_LINEW_PATH, NULL);
		else
			evas_object_image_file_set(wd->bg_grid[i], CROP_LINEH_PATH, NULL);
	}
}

static void
_scissorbox_hide(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Widget_Data *wd;

	if (!data)
		return;

	wd = (Widget_Data *)elm_widget_data_get((Evas_Object *)data);
	if (!wd)
		return;

	int i = 0;
	for (i = 0; i < DEFAULT_POT_NUM; i++) {
		evas_object_hide(wd->selector_line[i]);

		evas_object_hide(wd->selector_pot[i]);

		evas_object_hide(wd->bg_grid[i]);

		evas_object_hide(wd->event_pot[i]);
	}

	evas_object_hide(wd->bg);
	BROWSER_LOGD("ScissorBox Hide");
}


static void
_scissorbox_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Widget_Data *wd;

	if (!data)
		return;

	wd = (Widget_Data *)elm_widget_data_get((Evas_Object *)data);
	if (!wd)
		return;

/*  TODO : Implement Clipper*/

	_sizing_eval(wd->obj);

	int i;
	for (i = 0; i < DEFAULT_POT_NUM; i++) {
		evas_object_show(wd->selector_line[i]);

		evas_object_show(wd->selector_pot[i]);

		evas_object_show(wd->bg_grid[i]);

		evas_object_show(wd->event_pot[i]);
	}

	evas_object_show(wd->bg);

	BROWSER_LOGD("ScissorBox Show");
}


Evas_Object *br_scissorbox_add(Evas_Object *parent)
{
	Evas_Object *obj;
	Widget_Data *wd;

	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	wd = (Widget_Data *)ELM_NEW(Widget_Data);

	obj = elm_widget_add(NULL, parent);
	elm_widget_type_set(obj, "scissorbox");
	elm_widget_sub_object_add(parent, obj);
	elm_widget_data_set(obj, wd);
	elm_widget_del_hook_set(obj, _del_hook);
	elm_widget_theme_hook_set(obj, _theme_hook);
	elm_widget_can_focus_set(obj, EINA_FALSE);

	wd->parent = parent;
	wd->obj = obj;
	wd->evas = evas_object_evas_get(parent);
	wd->pot_w = DEFAULT_MIN_RECT_SIZE;
	wd->pot_h = DEFAULT_MIN_RECT_SIZE;
	wd->event_pot_w = DEFAULT_MIN_EVENT_RECT_SIZE;
	wd->event_pot_h = DEFAULT_MIN_EVENT_RECT_SIZE;

	evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

	Evas_Object *bg = evas_object_rectangle_add(wd->evas);
	evas_object_smart_member_add(bg, obj);
	evas_object_color_set(bg, 0, 0, 0, 0);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_widget_sub_object_add(obj, bg);
	wd->bg = bg;

/*       elm_widget_resize_object_set(obj, wd->bg);*/

	evas_object_event_callback_add(wd->bg, EVAS_CALLBACK_MOUSE_DOWN,
				       _selector_mouse_down, obj);
	evas_object_event_callback_add(wd->bg, EVAS_CALLBACK_MOUSE_UP,
				       _selector_mouse_up, obj);
	evas_object_event_callback_add(wd->bg, EVAS_CALLBACK_MOUSE_MOVE,
				       _selector_mouse_move, obj);

	int i;
	for (i = 0; i < DEFAULT_POT_NUM; i++) {
		wd->bg_grid[i] = evas_object_rectangle_add(wd->evas);
		evas_object_smart_member_add(wd->bg_grid[i], obj);
		evas_object_color_set(wd->bg_grid[i], 0, 0, 0, DEFAULT_DIM_BG_ALPHA);
		evas_object_size_hint_weight_set(wd->bg_grid[i], EVAS_HINT_EXPAND,
						 		EVAS_HINT_EXPAND);

		evas_object_repeat_events_set(wd->bg_grid[i], EINA_FALSE);
		elm_widget_sub_object_add(obj, wd->bg_grid[i]);
	}

	int line_w = 0;
	int line_h = 0;
	for (i = 0; i < DEFAULT_POT_NUM; i++) {
		wd->selector_line[i] = evas_object_image_filled_add(wd->evas);
		evas_object_smart_member_add(wd->selector_line[i], obj);

		if(i % 2)
			evas_object_image_file_set(wd->selector_line[i],
						   CROP_LINEW_PATH, NULL);
		else
			evas_object_image_file_set(wd->selector_line[i],
			   			   CROP_LINEH_PATH, NULL);

		evas_object_image_size_get(wd->selector_line[i], &line_w, &line_h);
		evas_object_resize(wd->selector_line[i], line_w, line_h);

		evas_object_event_callback_add(wd->selector_line[i],
					       EVAS_CALLBACK_MOUSE_DOWN,
					       _line_mouse_down, obj);
		evas_object_event_callback_add(wd->selector_line[i],
					       EVAS_CALLBACK_MOUSE_UP,
					       _line_mouse_up, obj);
	}

	for (i = 0; i < DEFAULT_POT_NUM; i++) {
		/* Create image icon (pot) */
		wd->selector_pot[i] = evas_object_image_filled_add(wd->evas);
		evas_object_smart_member_add(wd->selector_pot[i], obj);

		evas_object_image_file_set(wd->selector_pot[i], CROP_POT_PATH,
					   NULL);
		evas_object_image_fill_set(wd->selector_pot[i], 0, 0, wd->pot_w,
					   wd->pot_h);

		evas_object_resize(wd->selector_pot[i], wd->pot_w, wd->pot_h);

		/* Create event object above image icon (pot) */
		wd->event_pot[i] = evas_object_rectangle_add(wd->evas);
		evas_object_smart_member_add(wd->event_pot[i], obj);
		evas_object_color_set(wd->event_pot[i], 0, 0, 0, 0);
		evas_object_resize(wd->event_pot[i], wd->event_pot_w, wd->event_pot_h);

		evas_object_event_callback_add(wd->event_pot[i],
					       EVAS_CALLBACK_MOUSE_DOWN,
					       _pot_mouse_down, obj);
		evas_object_event_callback_add(wd->event_pot[i],
					       EVAS_CALLBACK_MOUSE_UP,
					       _pot_mouse_up, obj);
		evas_object_event_callback_add(wd->event_pot[i],
					       EVAS_CALLBACK_MOUSE_MOVE,
					       _pot_mouse_move, obj);
	}

	int w, h;
	evas_object_image_size_get(wd->selector_pot[0], &w, &h);
	BROWSER_LOGD(" file %s, [%d, %d]", CROP_POT_PATH, w, h);

	evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW,
				       _scissorbox_show, obj);
	evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE,
				       _scissorbox_hide, obj);

	_sizing_eval(obj);

	return obj;
}

void br_scissorbox_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h)
{
	Widget_Data *wd;

	wd = (Widget_Data *)elm_widget_data_get(obj);
	if (!wd)
		return;

	if (x)
		*x = wd->rect.x;
	if (y)
		*y = wd->rect.y;
	if (w)
		*w = wd->rect.w;
	if (h)
		*h = wd->rect.h;

/*       IVUG_DEBUG_MSG( "Region: [%d, %d, %d, %d]", *x, *y, *w, *h);*/
}

void br_scissorbox_region_set(const Evas_Object *obj, int x, int y, int w, int h)
{
	Widget_Data *wd;

	wd = (Widget_Data *)elm_widget_data_get(obj);
	if (!wd)
		return;

	wd->rect.x = x;
	wd->rect.y = y;
	wd->rect.w = w;
	wd->rect.h = h;

	evas_object_resize(wd->obj, wd->rect.w, wd->rect.h);

	BROWSER_LOGD("Region Set: [%d, %d, %d, %d]", x, y, w, h);

	_sizing_eval(wd->obj);

}

void br_scissorbox_bound_set(const Evas_Object *obj, int x, int y, int w, int h)
{
	Widget_Data *wd;

	wd = (Widget_Data *)elm_widget_data_get(obj);
	if (!wd)
		return;

	wd->bound.x = x;
	wd->bound.y = y;
	wd->bound.w = w;
	wd->bound.h = h;

	BROWSER_LOGD("Boundary Set: [%d, %d, %d, %d]", x, y, w, h);

	_sizing_eval(wd->obj);
}
