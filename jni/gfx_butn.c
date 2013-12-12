/*
 * gfx_butn.c
 *
 *  Created on: 2013/11/29
 *      Author: Michael
 */

#include <stddef.h>
#include <android/log.h>
#include <time.h>

//#include <GLES/gl.h>
#include <EGL/egl.h>
#include <GLES/gl.h>


#include "gfx_gles.h"
#include "gfx/vertex.h"
#include "hon_type.h"
#include "gfx_asst.h"
#include "gfx_butn.h"


#include "and_main.h"

struct button buttons[] = {
		{textures + 2, textures + 5,
				-1.0F, -1.0F, 1.0F, 0.0F, 0.0F, FALSE,
				0.0F, 0.0F, 0.0F, 0.0F,
				TOUCH_EVENT_BUTTON_0, FALSE, TRUE, FALSE, BTN_FADE_IN_RATE, BTN_FADE_OUT_RATE,
				buttons+1, NULL},
		{textures + 3, textures + 6,
				-1.0F, -1.0F, 1.0F, 0.0F, 0.0F, FALSE,
				0.0F, 0.0F, 0.0F, 0.0F,
				TOUCH_EVENT_BUTTON_1, FALSE, FALSE, FALSE, BTN_FADE_IN_RATE, BTN_FADE_OUT_RATE,
				buttons+2, buttons+0},
		{textures + 4, textures + 7,
				-1.0F, -1.0F, 1.0F, 0.0F, 0.0F, FALSE,
				0.0F, 0.0F, 0.0F, 0.0F,
				TOUCH_EVENT_BUTTON_2, FALSE, FALSE, FALSE, BTN_FADE_IN_RATE, BTN_FADE_OUT_RATE,
				NULL, buttons+1},
};

struct vertex btn_quad[] = {
	{0.0f, 		0.0f, 		0.0f, 		0.0f, 		0.0f, 		0.0f}, //0, 0
	{0.25f, 	0.0f, 		0.0f, 		1.0f, 		0.0f, 		0.0f}, //0,1
	{0.25f, 	0.25f, 	0.0f, 		1.0f,		1.0f, 		0.0f},
	{0.0f, 		0.25f, 	0.0f, 		0.0f,		1.0f, 		0.0f},
};

unsigned short btn_quad_index[] = {
  0, 1, 3, 2
};

int sizeof_button_array = sizeof buttons/sizeof buttons[0];
int sizeof_buttons = sizeof buttons;
int sizeof_btn_quad = sizeof btn_quad;
int sizeof_btn_quad_index = sizeof btn_quad_index;

int button_fade_in_called = FALSE;
int button_fade_out_called = FALSE;



void set_index_fading(int i);
void set_btn_fading(struct button* b, int in);



void calc_btn_quad_verts(int bm_w, int bm_h) {

	float gl_w = ((float)bm_w/(float)g_sc.width)*2.0F;
	float gl_h = ((float)bm_h/(float)g_sc.height)*2.0F;

	LOGD("calc_btn_quad_verts", "gl_w: %f", gl_w);

	btn_quad[1].x = gl_w;
	btn_quad[2].x = gl_w; 	btn_quad[2].y = gl_h;
									btn_quad[3].y = gl_h;
	int i;

	for (i = 0; i < sizeof_button_array; i++) {
		struct button* b = buttons + i;

		b->gl_x = -1.0F + (i * gl_w);

		b->touch_bl.x = gl_to_scr(-1.0F+(i * gl_w), 1);
		b->touch_bl.y = gl_to_scr(-1.0F, 0);
		b->touch_tr.x = gl_to_scr(-1.0F+((i+1)*gl_w),1);
		b->touch_tr.y = gl_to_scr(-1.0+gl_h, 0);

		LOGD("calc_btn_quad_verts", "b->touch_bl.x: %f", b->touch_bl.x);
		LOGD("calc_btn_quad_verts", "b->touch_bl.y: %f", b->touch_bl.y);
		LOGD("calc_btn_quad_verts", "b->touch_tr.x: %f", b->touch_tr.x);
		LOGD("calc_btn_quad_verts", "b->touch_tr.y: %f", b->touch_tr.y);
		LOGD("calc_btn_quad_verts", "b->gl_x: %f", b->gl_x);

	}
}

//int get_touch_response(float x, float y) {
//
//	LOGD("get_touch_response", "x: %f, y: %f", x, y);
//
//
//	int response = TOUCH_EVENT_GAME;
//
//	if (!interactive_mode) {
//		response = TOUCH_EVENT_INTERACTIVE_ON;
//	}
//	else {
//		int i;
//		for (i = 0; i < sizeof_button_array; i++) {
//			struct button* b = buttons + i;
//
//			LOGD("get_touch_response", "b->touch_bl.x: %f, b->touch_bl.y: %f", b->touch_bl.x, b->touch_bl.y);
//			LOGD("get_touch_response", "b->touch_tr.x: %f, b->touch_tr.y: %f", b->touch_tr.x, b->touch_tr.y);
//	//		11-29 12:26:31.113: D/get_touch_response(30149): x: 67.187500, y: 432.656250
//	//		11-29 12:26:31.113: D/get_touch_response(30149): b->touch_bl.x: 0.000000, b->touch_bl.y: 480.000000
//	//		11-29 12:26:31.117: D/get_touch_response(30149): b->touch_tr.x: 128.000000, b->touch_tr.y: 352.000000
//
//			if (!b->fading_in && !b->fading_out && !b->is_touch_anim) {
//				if (x>b->touch_bl.x && x<b->touch_tr.x) {
//					if (y<b->touch_bl.y && y>b->touch_tr.y) {
//
//						b->is_touch_anim = TRUE;
//
//						response = b->event_enum;
//					}
//				}
//			}
//		}
//	}
//	return response;
//}




int get_touch_response(float x, float y) {

	LOGD("get_touch_response", "x: %f, y: %f", x, y);


	int res = TOUCH_EVENT_GAME;

	int i;
	for (i = 0; i < sizeof_button_array; i++) {
		struct button* b = buttons + i;

		LOGD("get_touch_response", "b->touch_bl.x: %f, b->touch_bl.y: %f", b->touch_bl.x, b->touch_bl.y);
		LOGD("get_touch_response", "b->touch_tr.x: %f, b->touch_tr.y: %f", b->touch_tr.x, b->touch_tr.y);


		if (x > b->touch_bl.x && x < b->touch_tr.x) {
			if (y < b->touch_bl.y && y > b->touch_tr.y) {

				if (!button_fade_in_called && !button_fade_out_called) {
					if (!b->is_touch_anim) {
						if (!interactive_mode) {
							res = TOUCH_EVENT_INTERACTIVE_ON;
							return res;
						}
						else {

							b->is_touch_anim = TRUE;
							res = b->event_enum;
							return res;
						}
					}
				}

			}
		}


	}



	return res;
}



















void btn_anim(struct button* b) {

	if (b->fading_in) {
//		LOGD("btn_anim", "b->fading_in");
		if (b->alpha < BTN_ALPHA_MAX) {
			b->alpha += (float)frame_delta * b->fade_in_rate;

			if (b->alpha > 0.2F) set_btn_fading(b->fade_in_next, TRUE);

//			LOGD("btn_anim", "b->alpha : %f", b->alpha);
		}
		else if (b->alpha >= BTN_ALPHA_MAX) {
			LOGD("btn_anim", "else if (b->alpha >= BTN_ALPHA_MAX)");
			b->alpha = BTN_ALPHA_MAX;
			b->fading_in = FALSE;
			LOGD("btn_anim", "b->fading_out: %d", b->fading_out);
			LOGD("btn_anim", "b->fading_in: %d", b->fading_in);
		}
	}


	if (b->fading_out) {
//		LOGD("btn_anim", "b->fading_out");
		if (b->alpha > BTN_ALPHA_MIN) {

			b->alpha -= (float)frame_delta * b->fade_out_rate;

			if (b->alpha < 0.34F) set_btn_fading(b->fade_out_next, FALSE);

//			LOGD("btn_anim", "b->alpha : %f", b->alpha);
		}
		else if (b->alpha <= BTN_ALPHA_MIN) {
			LOGD("btn_anim", "else if (b->alpha <= BTN_ALPHA_MIN)");
			b->alpha = BTN_ALPHA_MIN;
			b->fading_out = FALSE;
			LOGD("btn_anim", "b->fading_out: %d", b->fading_out);
			LOGD("btn_anim", "b->fading_in: %d", b->fading_in);
		}
	}




//	if (b->fading_out) {
//		if (b->alpha > BTN_ALPHA_MIN) {
//			b->alpha -= BTN_FADE_RATE;
//		} else if (b->alpha <= BTN_ALPHA_MIN) {
//			b->alpha = BTN_ALPHA_MIN;
//			b->fading_out = FALSE;
//		}
//	}

	if (b->is_touch_anim) {

		if (!b->pressed_peak) {
			b->alpha_pt += (float)frame_delta * BTN_PRESS_FADE_RATE;
		}
		if (b->alpha_pt >= 1.0F && !b->pressed_peak) {
			b->pressed_peak = TRUE;
			b->alpha_pt = 1.0F;
		}

		if (b->pressed_peak) {
			b->alpha_pt -= (float)frame_delta * BTN_PRESS_FADE_RATE_OUT;
		}

		if (b->alpha_pt <= 0.0F && b->pressed_peak) {
			b->pressed_peak = FALSE;
			b->is_touch_anim = FALSE;
			b->alpha_pt = 0.0F;
		}
	}
}

//void set_btn_fading(int b) {
//	if (b < sizeof_button_array)
//		buttons[b].fading_in = TRUE;
//}
void set_index_fading(int i) {
	if (i < sizeof_button_array) {
		struct button* b = buttons + i;
		if (!b->fading_in)
			b->fading_in = TRUE;
	}
}


void set_btn_fading(struct button* b, int in) {

	if (b != NULL) {
		if (in && !b->fading_out && !b->fading_in)
			b->fading_in = TRUE;
		if (!in && !b->fading_in && !b->fading_out)
			b->fading_out = TRUE;
	}
}

//int buttons_fading() {
//	int r = FALSE;
//	int i;
//	for (i = 0; i < sizeof_button_array; i++) {
//		button* b = buttons + i;
//		if (b->fading_in || b->fading_out) r = TRUE;
//	}
//	return r;
//}











