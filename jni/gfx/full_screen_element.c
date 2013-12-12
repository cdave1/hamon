/*
 * full_screen_element.c
 *
 *  Created on: 2013/12/05
 *      Author: Michael
 */
#include <android/log.h>
#include <time.h>

#include "gfx/full_screen_element.h"
#include "hon_type.h"
#include "gfx_asst.h"
#include "gfx_gles.h"

#include "gfx/vertex.h"
#include "game/moods.h"

#include "math/trig_sampler.h"


struct full_scr_el screens[] = {
		 {"splash", textures + 0, 0.0F, 0.0F, SPLASH_FADE_RATE, TRUE, FALSE, TRUE, NULL, 0.0, 1.0, 1.0 },
		 {"splash_bg", textures + 1, 0.0F, 0.0F, SPLASH_BG_FADE_RATE, TRUE, FALSE, TRUE, modulators + 0, 0.0, 1.0, 1.0 },
		 {"help", textures + 0, 0.0F, 0.0F, HELP_FADE_RATE, FALSE, FALSE, FALSE, NULL, 0.0, 1.0, 1.0 },
};

struct full_scr_el backgrounds[] = {
		 {"new_bg_1", NULL, 0.0F, 0.0F, BG_PULSE_FADE_RATE, TRUE, FALSE, TRUE, NULL, 0.0, 1.0, 1.0 },
		 {"new_bg_2", NULL, 0.0F, 0.0F, BG_PULSE_FADE_RATE, FALSE, FALSE, FALSE, NULL, 0.0, 1.0, 1.0 }
};



int sizeof_screens_elements = sizeof screens/sizeof screens[0];
int sizeof_screens = sizeof screens;
int sizeof_backgrounds_elements = sizeof backgrounds/sizeof backgrounds[0];
int sizeof_backgrounds = sizeof backgrounds;


int selected_background = 0;

//int splash_delay = 0;


void full_scr_anim(struct full_scr_el* fs) {

	full_scr_alpha_anim(fs);
//	draw_full_screen_image(fs);
	fs->alpha_mod = fs->alpha;
}


void full_scr_alpha_anim(struct full_scr_el* fs) {

		if (fs->fading_in && fs->alpha < 1.0) {
			fs->alpha += (float)frame_delta *  fs->fade_rate;
		} else if (fs->fading_in && fs->alpha >= 1.0) {
			fs->fading_in = FALSE;
			fs->alpha = 1.0;
		}

		if (fs->fading_out && fs->alpha > 0.0) {


			fs->alpha -= (float)frame_delta * fs->fade_rate;
		} else if (fs->fading_out && fs->alpha <= 0.0) {

			fs->fading_out = FALSE;
			fs->alpha = 0.0;
			fs->is_showing = FALSE;
		}
}



void full_scr_alpha_mod (struct full_scr_el* fs) {

	struct modulator* m = fs->mod_a;

	cycle_modulator(m);
	fs->alpha_mod = fs->alpha * (m->mod_cycle[m->curr_samp]);

}


float sum_alpha(struct full_scr_el* fs) {

	float a;
	if (fs->mod_a != NULL)
		a =  fs->alpha * (fs->mod_a->mod_cycle[fs->mod_a->curr_samp]);
	else
		a =  fs->alpha;

	return a;
}

void full_scr_mod(struct full_scr_el* fs) {
	fs->pulse += (float)frame_delta * BG_PULSE_FADE_RATE * fs->pulse_dir;
	if (fs->pulse_dir == 1.0F &&  fs->pulse > 1.0) fs->pulse_dir = -1.0F;
	if (fs->pulse_dir == -1.0F &&  fs->pulse < 0.2) fs->pulse_dir = 1.0F;
	//	LOGD("bg_pulse", "bg->pulse: %f", bg->pulse);
}


void full_scr_xfade() { //一発な関数

	LOGD("bg_xfade", "selected_background: %d", selected_background);
	struct full_scr_el* fs = backgrounds + selected_background;
	LOGD("bg_xfade", "fs->title: %s", fs->title);

	fs->fading_out = TRUE;
	if (selected_background<(sizeof_backgrounds_elements)) selected_background++;
	if (selected_background==(sizeof_backgrounds_elements)) selected_background = 0;
	LOGD("bg_xfade", "selected_background: %d", selected_background);
	fs = backgrounds + selected_background;

	LOGD("bg_xfade", "fs->title: %s", fs->title);

	fs->is_showing = TRUE;
	fs->fading_in = TRUE;

		int i;
		for (i=0;i<sizeof_backgrounds_elements;i++) {
			struct full_scr_el* bgfs = backgrounds+i;
			LOGD("bg_xfade", "fs->is_showing: %i", bgfs->is_showing);
			LOGD("bg_xfade", "fs->fading_in: %i", bgfs->fading_in);
			LOGD("bg_xfade", "fs->fading_out: %i", bgfs->fading_out);
		}
}


int full_scr_fading(struct full_scr_el* fs) {
	int r = FALSE;
	if (fs->is_showing) {
		if (fs->fading_in || fs->fading_out) r = TRUE;
		if (!fs->fading_in && !fs->fading_out) r = FALSE;
	}
	return r;
}



int all_bgs_fading() {

	int r = FALSE;
	int i;
	for (i=0; i<sizeof_backgrounds_elements; i++) {

		r = full_scr_fading(backgrounds+i);
	}

	return r;
}


