// ノートの再生と録音の管理、自動的な再生（AI)
// 弾いましたオートを全部ここに通さないといけない
//

#include <android/log.h>
#include <time.h>


// for threading
#include <unistd.h>  // sleep()を定義
#include <pthread.h>

#include <stdbool.h>
#include <stdlib.h>

#include "snd_ctrl.h"



#include "hon_type.h"



#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "snd_asst.h"
#include "snd_scal.h"
#include "snd_sles.h"


#include "gfx/vertex.h"
#include "game/moods.h"
#include "and_main.h"

#include <android/log.h>
#include <android_native_app_glue.h>
#include "gfx_gles.h"

#include <SLES/OpenSLES.h>
#include "gfx/touch_circle.h"
#include "gfx/tex_circle.h"





#define AMMO_INCREASE_RATE 40//50 // 個のticsを過ごすと、AMMOが1に増やす
// この値は記録した後の再生数を数える
#define PART_TTL 9
#define FADE_OUT_POINT 4
#define FADE_OUT_FACTOR 0.9F

// 自動的な再生
#define SILENCE_BEFORE_AUTO_PLAY 150
#define SILENCE_BEFORE_AUTO_PLAY_INIT 115
#define ONE_SHOT_RND 200//90//230 // この値が変わるといいな
#define ONE_SHOT_RND_INIT 10 // この値が変わるといいな
#define TOTAL_START_SHOTS 2
#define MIN_CHORD_TIME 1800 // 2000のほうがいいのかも
#define CHORD_CHANGE_RND 4000 // 3500の方へ （2000だった）
#define MIN_REST_TIME 4000
#define AUTO_PLAY_REST_RND 4000
//#define TOTAL_NOTES_PER_PART 32
#define TOTAL_PARTS 7
#define TOTAL_PART_COLORS 8

#define RND_COL_LIMITER 0.2f


part parts[TOTAL_PARTS];

pthread_t control_loop;
pthread_attr_t thread_attr;

size_t one_shot_rnd[] = {110, 160, 230, 300, 400};

static int control_loop_running = TRUE;

int current_rec_part = 0;
static size_t tics_per_part = 1500; // 3000; // 5000;
//static size_t tic_increment = 0;

int playback_paused = FALSE;

static int start_shots = 0;

size_t ammo_current = AMMO_MAX;
//size_t ammo_max = AMMO_MAX;
size_t ammo_increase_counter;

// 自動的な再生
size_t not_active_count = SILENCE_BEFORE_AUTO_PLAY_INIT;
int parts_active = FALSE;

size_t chord_change_count = 0;



size_t one_shot_count = 0;
size_t rest_count = 0;
size_t chord_count = 0;
size_t one_shot_interval = 0;
size_t rest_interval = 0;
size_t chord_interval = 0;
size_t current_part_col = 0;


void* timing_loop(void* args);
void part_tic_counter();
void init_part(part* p, int rec);
void reset_all_notes(part* part);
void play_all_parts();
//void add_tic_increment(int inc);
void increase_ammo();
//int cycle_rec_part();
void count_part_ttl(part* p);
void factor_part_vel(part* p, float factor);
void parts_are_active();
void auto_play();

void init_part_color(part* p, int factor);



void init_random_seed() {
    srand((unsigned)time( NULL ));
}

int obtain_random(int modulus) {
	int r;
	r = rand();
	LOGD("obtain_random", "r %d", r);


    return (rand() % modulus);
}




void init_control_loop() {


//	  pthread_attr_init(&thread_attr);
//	  pthread_attr_setdetachstate(&thread_attr , PTHREAD_CREATE_DETACHED);

//	pthread_create(&control_loop, &thread_attr, timing_loop, (void*)NULL);



	pthread_create(&control_loop, NULL, timing_loop, (void*)NULL);



	//init_auto_vals(); // FIXME
}

void join_control_loop() {

	control_loop_running = FALSE;
	pthread_join(control_loop, NULL);
	pthread_exit(NULL);

}

void init_auto_vals() {
	one_shot_interval = obtain_random(ONE_SHOT_RND_INIT);
	rest_interval = MIN_REST_TIME + obtain_random(AUTO_PLAY_REST_RND);
	chord_interval = MIN_CHORD_TIME + obtain_random(CHORD_CHANGE_RND);

	LOGD("init_auto_vals", "one_shot_interval %d", one_shot_interval);
	LOGD("init_auto_vals", "rest_interval %d", rest_interval);
	LOGD("init_auto_vals", "chord_interval %d", chord_interval);

}

// コリラのほうが性格的に正しい
void* timing_loop(void* args) {

	while (control_loop_running) {
//		__android_log_write(ANDROID_LOG_DEBUG, "timing_loop", "while (1)");
//		clock_gettime(CLOCK_MONOTONIC, &timing.start_time_s);
//		// 処理
//
//	    clock_gettime(CLOCK_MONOTONIC, &timing.finish_time_s);
//
//	    // StackOverflowからの
////	    (timing.finish_time_s.tv_sec * NS_IN_SEC + timing.finish_time_s.tv_nsec) - (timing.start_time_s.tv_nsec * NS_IN_SEC + timing.start_time_s.tv_nsec)；
//
//	    timing.diff_time_s.tv_nsec = (5000000 - (timing.finish_time_s.tv_nsec - timing.start_time_s.tv_nsec));
//		nanosleep(&timing.diff_time_s, NULL);


		// これだけで十分あまり性格的なタイミングが必要ないかも
		usleep(100000); // 100ミリ秒
//		general_tic_counter();
		vol_automation();
		increase_ammo();
		if (show_gameplay && !playback_paused) auto_play();

//		shutdown_audio_delay();
//		gettimeofday(&timing.curr_time, &timing.tzp);
//		__android_log_print(ANDROID_LOG_DEBUG, "sound_control_lroop", "gettimeofday: %d %d sleep_time: %d %d",
//				timing.curr_time.tv_sec, timing.curr_time.tv_usec, timing.adjusted_sleep_time.tv_sec, timing.adjusted_sleep_time.tv_usec);
//
//		gettimeofday(&timing.curr_time, &timing.tzp);
//		__android_log_print(ANDROID_LOG_DEBUG, "timing_loop", "gettimeofday: %d %d",
//				timing.curr_time.tv_sec, timing.curr_time.tv_usec);
		//if (!control_loop_running) break;


	}
	return NULL;

}

//void bump_ammo() {
//
//}

void increase_ammo() {
	if (ammo_increase_counter < AMMO_INCREASE_RATE && ammo_current < AMMO_MAX) {
		ammo_increase_counter++;
	}
	if (ammo_increase_counter == AMMO_INCREASE_RATE && ammo_current < AMMO_MAX) {
		ammo_increase_counter = 0;
		ammo_current++;
		LOGD("increase_ammo", "ammo_current %d", ammo_current);
	}
}

int decrease_ammo() { // タッチするときの処理・AMMOを減るため
	if (ammo_current > 0) {
		ammo_current--;
		ammo_increase_counter = 0;
		return TRUE;
		LOGD("increase_ammo", "ammo_current %d", ammo_current);
	}
	return FALSE;
}



// mainから呼ぶ
void record_note(float x, float y, int seg, float vel){
//	LOGI("record_note", "x: %f", x);
//	LOGI("record_note", "y: %f", y);



	part* p = (parts + current_rec_part);

//	__android_log_print(ANDROID_LOG_DEBUG, "record_note", "current_rec_part %d",
//			current_rec_part);


	int tic = p->current_tic;
	int n = p->current_note;

	p->note_info[n].pos_x = x;
	p->note_info[n].pos_y = y;
//	LOGI("record_note", "p->note_info[n].pos_x: %f", p->note_info[n].pos_x );
//	LOGI("record_note", "p->note_info[n].pos_y: %f", p->note_info[n].pos_y);

	p->note_info[n].seg = seg;
	p->note_info[n].vel = vel;

	p->note_info[n].tic = tic;

//	LOGD("record_note", "current_part_color %d", current_part_color());
//	LOGD("record_note", "current_rec_part %d, current_tic %d, current_note %d, color %d",
//			current_rec_part,p->current_tic, p->current_note, p->color);

	p->current_note++;

}



// 毎TIC実行しなきゃ //
// ticを全部進めないといけない
void part_tic_counter() {

	if (!playback_paused) {

		int i;
		for (i = 0; i < TOTAL_PARTS; i++) {
			part* p = (parts + i);
				if (p->current_tic >= p->total_tics && p->is_recording) {
									p->is_recording = FALSE;
									p->current_tic = 0;
									LOGI("part_tic_counter", "(p->current_tic >= p->total_tics && p->is_recording)");
									init_part(parts + get_free_part(), TRUE);

				} else if (p->current_tic >= p->total_tics && !p->is_recording && p->is_alive) {
									count_part_ttl(p);
									p->current_tic = 0;
				} else if (p->current_tic < p->total_tics && p->is_recording && p->current_note > 0) {
									p->current_tic++;
				} else if (p->current_tic < p->total_tics && !p->is_recording && p->is_alive) {
									p->current_tic++;
				}
		}
	}
}




int get_free_part() { // もしかしてget_free_part()

	int i;
	for (i = 0; i < TOTAL_PARTS; i++) {

		if ((parts + i)->is_alive == FALSE) {
			current_rec_part = i;
			LOGI("get_free_part", "current_rec_part %d", i);
			return current_rec_part;
		}
	}
	return get_oldest_part();

}

int get_oldest_part() {

	int most_plays = 0;
	int part = 0;

	int i;
	for (i = 0; i < TOTAL_PARTS; i++) {
		int pc = (parts + i)->play_count;
		if(pc >= most_plays) {
			most_plays = pc;
			part = i;
		}
	}

	current_rec_part = part;
	LOGI("get_free_part", "get_oldest_part %d", current_rec_part);
	return current_rec_part;

}


void parts_are_active() {

	int active = FALSE;
	int i;
	for (i = 0; i < TOTAL_PARTS; i++) {
		if ((parts + i)->is_alive && !(parts + i)->is_recording) {
			active = TRUE;
		}
	}

	LOGI("parts_are_active", "active %d", active);
	parts_active = active;
}

void set_parts_active() {
	LOGD("set_parts_active", "set_parts_active()");

	parts_active = TRUE;
	not_active_count = 0;
	chord_change_count = 0;
}



void auto_play() {
//	LOGD("auto_play", "auto_play called()");

	if (!parts_active && not_active_count < SILENCE_BEFORE_AUTO_PLAY) {
		not_active_count++;
		LOGD("auto_play", "not_active_count %d", not_active_count);
	}
	if (not_active_count == SILENCE_BEFORE_AUTO_PLAY) {

		if (one_shot_count == one_shot_interval) {
//
//			float x = (float) (obtain_random(screen_width));
//			float y = (float) (obtain_random(screen_height_reduced));

//			float x = (float) (obtain_random(screen_width));
//			float y = (float) ((obtain_random(screen_height - (screen_margin*2.0F))) + screen_margin);

//			float x = (float) ((obtain_random(screen_width - (screen_margin_x * 2.0F))) + screen_margin_x);
//			float y = (float) ((obtain_random(screen_height - (screen_margin_y * 2.0F))) + screen_margin_y);


//			size_t x_range = (screen_width - (screen_margin_x_l + screen_margin_x_r));
//			size_t y_range = (screen_height - (screen_margin_y_t + screen_margin_y_b));
//
//			LOGD("auto_play", "x_range: %d", x_range);
//			LOGD("auto_play", "y_range: %d", y_range);


			float x = (float) ((obtain_random(screen_width - (screen_margin_x_l + screen_margin_x_r))) + screen_margin_x_l);
			float y = (float) ((obtain_random(screen_height - (screen_margin_y_t + screen_margin_y_b))) + screen_margin_y_t);




			LOGD("auto_play", "x %f y %f", x, y);
			trigger_note(x, y);
			ammo_current++; //


			one_shot_count = 0;
//			one_shot_interval = 5+obtain_random(500);

			if (start_shots < TOTAL_START_SHOTS) {
				one_shot_interval = 5+obtain_random(50);
				start_shots++;
			} else {
//				one_shot_interval = 5+obtain_random(390);
				one_shot_interval = 5+obtain_random(ONE_SHOT_RND);
			}

			LOGD("auto_play", "one_shot_interval %d", one_shot_interval);
		}


		if (rest_count == rest_interval) { // FIXME

			parts_active = TRUE; // FIXME これ問題の原因かも>>>???
			not_active_count = 0;

			rest_count =0;
			rest_interval = MIN_REST_TIME + obtain_random(AUTO_PLAY_REST_RND);
			LOGD("auto_play", "rest_interval %d", rest_interval);
		}


		if (chord_count == chord_interval) {
			int success = cycle_mood();
			chord_count = 0;
//			chord_interval = MIN_CHORD_TIME + obtain_random(3000);
			chord_interval = MIN_CHORD_TIME + obtain_random(CHORD_CHANGE_RND);
			LOGD("auto_play", "chord_interval %d", chord_interval);
		}

		one_shot_count++;
		rest_count++;
		chord_count++;
	}

}



// 初期化するためだけ
void init_all_parts() {

	// 録音したノートをレセットするため

	parts_active = FALSE;
	if (touch_enabled) not_active_count = 0;
	else not_active_count = SILENCE_BEFORE_AUTO_PLAY_INIT;
	current_rec_part = 0;


//	int total_tic_counter = 0;

	int i;
	int total_parts = TOTAL_PARTS;

	for (i = 0; i < total_parts; i++) {
		part* p = (parts + i);

		if (i == 0) init_part(p, TRUE);
		else init_part(p, FALSE);

		init_part_color(p, i);
//		init_part_colors(p);



		LOGD("init_part", "p->rgb->r: %f, g: %f, b: %f", p->rgb->r, p->rgb->g, p->rgb->b);
	}
}

void init_part(part* p, int rec) {


	p->current_note = 0;
	p->current_tic = 0;
	p->is_recording = rec;
	p->play_count = 0;

	p->total_tics = tics_per_part + obtain_random(250);

	p->is_alive = rec;

	LOGD("init_part", "p->total_tics  %d", p->total_tics);


	reset_all_notes(p);

}

void init_part_color(part* p, int factor) {

	p->rgb = (struct vertex_rgb*) malloc(sizeof(struct vertex_rgb));

	p->rgb->r = 1.0f;
	p->rgb->g = 1.0f;
	p->rgb->b = 1.0f;

//	float m = 0.2f;

	while (p->rgb->r >= RND_COL_LIMITER &&
				p->rgb->g >= RND_COL_LIMITER &&
				p->rgb->b >= RND_COL_LIMITER) 	{ // makes sure at least one rand val is under m
		p->rgb->r = (float) obtain_random(75) / 100.0f;
		p->rgb->g = (float) obtain_random(75) / 100.0f;
		p->rgb->b = (float) obtain_random(75) / 100.0f;
	}

	LOGD("init_part", "init_part_color(): p->rgb->r: %f, g: %f, b: %f", p->rgb->r, p->rgb->g, p->rgb->b);

}



void reset_all_notes(part* p) {

	int n;

	int total_notes = sizeof p->note_info / sizeof p->note_info[0];

	for (n = 0; n < total_notes; n++) {

		note* note = (p->note_info) + n;

		note->pos_x = 0.0F;
		note->pos_y = 0.0F;
		note->vel = 0.0F;
		note->tic = 0;
		note->vel = 0;
	}
}

// パートの長さを異なる
//void add_tic_increment(int inc) {
//	tic_increment += inc;
//}


void play_all_parts() {

	int i;


	int total_parts = TOTAL_PARTS;

	for (i = 0; i < total_parts; i++) {
		part* p = (parts + i);

		if (!p->is_recording && p->is_alive) {

			int j;
			int total_notes = p->current_note; // この方が一番早い
			for (j = 0; j < total_notes; j++) {

				note* n = (p->note_info) + j;

				//取り敢えず、最初の位置によってベロシティが減る程度が変えるってのは一番いい
				if (n->tic == p->current_tic) {
//				if (n->tic == p->current_tic && n->vel > 0.45F) {



					enqueue_one_shot(get_scale_sample(n->seg), float_to_slmillibel(n->vel, 1.0F), get_seg_permille(n->seg));

//				 	LOGI("play_all_parts", "total_tic_counter: %d: part: %d tic: %d current_tic: %d", total_tic_counter, i, n->tic, p->current_tic);

					activate_tex_circle(n->pos_x, n->pos_y, p->rgb, &n->vel);
//					activate_touch_circle(n->pos_x, n->pos_y, p->color, &n->vel);
//					LOGI("play_all_parts", "n->pos_x %f, n->pos_y %f", n->pos_x, n->pos_y);
					LOGI("play_all_parts", "part: %d, note: %d, n->vel %f", i, j, n->vel);
//					LOGI("play_all_parts", "part (i): %d", i);
//					LOGI("play_all_parts", "note, (j): %d", i);
				}
			}
		}
	}
}


void count_part_ttl(part* p) {

	if (p->play_count < PART_TTL && p->play_count < FADE_OUT_POINT) {
		p->play_count++;
	} else if (p->play_count < PART_TTL && p->play_count >= FADE_OUT_POINT) {
		p->play_count++;
		factor_part_vel(p, FADE_OUT_FACTOR);
	} else if (p->play_count == PART_TTL) {
		p->play_count = 0;
		//p->current_note = 0;
		p->is_alive = FALSE;
		//reset_all_notes(p); // 呼ぶ必要ないかも

		parts_are_active();

	}
	LOGD("count_part_ttl", "p->play_count: %d", p->play_count);

}

//// 有るパートのノートのvel全部増やす・減るのための関数
//void factor_part_vel(part* p, float factor) {
//	int j;
//	int total_notes = p->current_note;
//
//	for (j = 0; j < total_notes; j++) {
//					note* n = (p->note_info) + j;
//				 	n->vel = n->vel*factor;
//	}
//}


void factor_part_vel(part* p, float factor){
	int j;
	int total_notes = p->current_note;

	for (j = 0; j < total_notes; j++) {
					note* n = (p->note_info) + j;
				 	n->vel = n->vel*factor;
	}
}




//void shutdown_audio_delay() {
//	if (fading_out_exit) {
//		exit_fade_counter++;
//		if (exit_fade_counter == 12) {
//			shutdown_audio();
//		}
//	}
//}





//// I_GetTime
//// returns time in 1/70th second tics
////
//int  I_GetTime (void)
//{
//    struct timeval	tp;
//    struct timezone	tzp;
//    int			newtics;
//    static int		basetime=0;
//
//    gettimeofday(&tp, &tzp);
//    if (!basetime)
//	basetime = tp.tv_sec;
//    newtics = (tp.tv_sec-basetime)*TICRATE + tp.tv_usec*TICRATE/1000000;
//    return newtics;
//}

//
//#include <time.h>
//#include <stdio.h>
//
//main()
//{
//        struct timespec tp ;
//        clock_gettime(CLOCK_MONOTONIC, &tp) ;
//        printf("%llu %llu\n", tp.tv_sec, tp.tv_nsec) ;
//}


//
////
//// D-DoomLoop()
//// Not a globally visible function,
////  just included for source reference,
////  called by D_DoomMain, never exits.
//// Manages timing and IO,
////  calls all ?_Responder, ?_Ticker, and ?_Drawer,
////  calls I_GetTime, I_StartFrame, and I_StartTic
////
//void D_DoomLoop (void);
//
//
////
////  D_DoomLoop
////
//extern  boolean         demorecording;
//
//void D_DoomLoop (void)
//{
//    if (demorecording)
//	G_BeginRecording ();
//
//    if (M_CheckParm ("-debugfile"))
//    {
//	char    filename[20];
//	sprintf (filename,"debug%i.txt",consoleplayer);
//	printf ("debug output to: %s\n",filename);
//	debugfile = fopen (filename,"w");
//    }
//
//    I_InitGraphics ();
//
//    while (1)
//    {
//	// frame syncronous IO operations
//	I_StartFrame ();
//
//	// process one or more tics
//	if (singletics)
//	{
//	    I_StartTic ();
//	    D_ProcessEvents ();
//	    G_BuildTiccmd (&netcmds[consoleplayer][maketic%BACKUPTICS]);
//	    if (advancedemo)
//		D_DoAdvanceDemo ();
//	    M_Ticker ();
//	    G_Ticker ();
//	    gametic++;
//	    maketic++;
//	}
//	else
//	{
//	    TryRunTics (); // will run at least one tic
//	}
//
//	S_UpdateSounds (players[consoleplayer].mo);// move positional sounds
//
//	// Update display, next frame, with current state.
//	D_Display ();
//
//#ifndef SNDSERV
//	// Sound mixing for the buffer is snychronous.
//	I_UpdateSound();
//#endif
//	// Synchronous sound output is explicitly called.
//#ifndef SNDINTR
//	// Update sound output.
//	I_SubmitSound();
//#endif
//    }
//}
//
//



//int  I_GetTime (void)
//{
//    struct timeval	tp;
//    struct timezone	tzp;
//    int			newtics;
//    static int		basetime=0;
//
//    gettimeofday(&tp, &tzp);
//    if (!basetime)
//	basetime = tp.tv_sec;
//    newtics = (tp.tv_sec-basetime)*TICRATE + tp.tv_usec*TICRATE/1000000;
//    return newtics;
//}
//
//


