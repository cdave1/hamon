/*
 * gfx_pics.h
 *
 *  Created on: 2013/12/02
 *      Author: Michael
 */

#ifndef GFX_FULS_H_
#define GFX_FULS_H_

#include "gfx_asst.h"


#define SPLASH_FADE_RATE (0.4F/(float)SEC_IN_US)
#define HELP_FADE_RATE (0.4F/(float)SEC_IN_US)




// �S��ʂ̊G���`���邽�߁i��F�X�v���b�V���j
typedef struct full_screen {

	char* title;
	texture_file* main_texture;
	float alpha;
	float fade_rate;

	int fading_in;
	int fading_out;
	int is_showing;

}full_screen;







extern full_screen screens[];
extern int sizeof_screens_array;
extern int sizeof_screens;

void screen_anim();


#endif /* GFX_PICS_H_ */
