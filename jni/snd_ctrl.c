// �m�[�g�̍Đ��Ƙ^���̊Ǘ��A�����I�ȍĐ��iAI)
// �e���܂����I�[�g��S�������ɒʂ��Ȃ��Ƃ����Ȃ�
//

#include <android/log.h>
#include <time.h>


// for threading
#include <unistd.h>  // sleep()���`
#include <pthread.h>

#include <stdbool.h>
#include <stdlib.h>

#include "snd_ctrl.h"
#include "snd_asst.h"
#include "snd_sles.h"
#include "snd_scal.h"
#include "hon_type.h"
#include "and_main.h"


//#define AMMO_INCREASE_RATE 50 // ��tics���߂����ƁAAMMO��1�ɑ��₷
//#define AMMO_MAX 5
//// ���̒l�͋L�^������̍Đ����𐔂���
//#define PART_TTL 10//15
//#define FADE_OUT_POINT 4
//#define FADE_OUT_FACTOR 0.9F
//
//#define SILENCE_BEFORE_AUTO_PLAY 150
//#define MINIMUM_CHORD_PLAY_TIME 300
//
//#define TOTAL_NOTES_PER_PART 32
//#define TOTAL_PARTS 4
//
//#define NS_IN_SEC 1000000000
//


#define AMMO_INCREASE_RATE 50 // ��tics���߂����ƁAAMMO��1�ɑ��₷
#define AMMO_MAX 5
// ���̒l�͋L�^������̍Đ����𐔂���
#define PART_TTL 9
#define FADE_OUT_POINT 4
#define FADE_OUT_FACTOR 0.9F

// �����I�ȍĐ�
#define SILENCE_BEFORE_AUTO_PLAY 150
#define MINIMUM_CHORD_PLAY_TIME 1000
#define ONESHOT_RANDOM 180 // ���̒l���ς��Ƃ�����
#define CHORD_CHANGE_RANDOM 2000



#define TOTAL_NOTES_PER_PART 32
#define TOTAL_PARTS 7

#define NS_IN_SEC 1000000000










typedef struct {
	float pos_x;
	float pos_y;
	int seg;
	float vel;
	int tic;


}note;


typedef struct {

	note note_info[TOTAL_NOTES_PER_PART];
	int current_note;
	int total_tics;
	int current_tic;
	int is_recording;


//	size_t ammo_remaining;

	size_t play_count;
	size_t plays_ttl;
	float fade_out_factor;

	int is_alive;


//	int been_used; // ���́`�p�[�g�͓����Ɏg�p���Ă�



}part;


struct {

	// ����͖�蔭���̌�������
	struct timeval start_time;
	struct timeval finish_time;
	struct timeval proc_time;

	struct timeval sleep_time;
	//sleep_time.tv_usec = 20000;
//	sleep_time.tv_usec = 500000;
	//sleep_time.tv_sec = 2;
	struct timeval adjusted_sleep_time;

	struct timeval curr_time;
	struct timezone tzp;

	struct timespec start_time_s;
	struct timespec finish_time_s;

	struct timespec diff_time_s;
}timing;




part parts[TOTAL_PARTS];

static int current_rec_part = 0;
static size_t tics_per_part = 1500; // 3000; // 5000;
static size_t tic_increment = 0;

size_t ammo_current = AMMO_MAX;
size_t ammo_max = AMMO_MAX;
size_t ammo_increase_counter;

// �����I�ȍĐ�
size_t not_active_count = 0;
int parts_active = FALSE;

size_t chord_change_count = 0;

extern size_t screen_width;
extern size_t screen_height_reduced;


void* timing_loop(void* args);
void tic_counter();
void init_part(part* p, int rec);
void reset_all_notes(part* part);
void play_all_parts();
void add_tic_increment(int inc);
void increase_ammo();
int cycle_rec_part();
void count_part_ttl(part* p);
void factor_part_vel(part* p, float factor);
void parts_are_active();
void auto_play();

void init_random_seed() {
    srand((unsigned)time( NULL ));
}

int obtain_random(int modulus) {
	int r;
	r = rand();
	__android_log_print(ANDROID_LOG_DEBUG, "obtain_random", "r %d", r);


    return (rand() % modulus);
};

void init_timing_loop() {

	pthread_t fade_in;
	pthread_create(&fade_in, NULL, timing_loop, (void*)NULL);

}



// �R�����̂ق������i�I�ɐ�����
void* timing_loop(void* args) {

	while (1) {
//		clock_gettime(CLOCK_MONOTONIC, &timing.start_time_s);
//		// ����
//
//	    clock_gettime(CLOCK_MONOTONIC, &timing.finish_time_s);
//
//	    // StackOverflow�����
////	    (timing.finish_time_s.tv_sec * NS_IN_SEC + timing.finish_time_s.tv_nsec) - (timing.start_time_s.tv_nsec * NS_IN_SEC + timing.start_time_s.tv_nsec)�G
//
//	    timing.diff_time_s.tv_nsec = (5000000 - (timing.finish_time_s.tv_nsec - timing.start_time_s.tv_nsec));
//		nanosleep(&timing.diff_time_s, NULL);


		// ���ꂾ���ŏ\�����܂萫�i�I�ȃ^�C�~���O���K�v�Ȃ�����
		usleep(100000); // 100�~���b

		vol_automation();
		increase_ammo();
		auto_play();

//		gettimeofday(&timing.curr_time, &timing.tzp);
//		__android_log_print(ANDROID_LOG_DEBUG, "sound_control_lroop", "gettimeofday: %d %d sleep_time: %d %d",
//				timing.curr_time.tv_sec, timing.curr_time.tv_usec, timing.adjusted_sleep_time.tv_sec, timing.adjusted_sleep_time.tv_usec);
//
//		gettimeofday(&timing.curr_time, &timing.tzp);
//		__android_log_print(ANDROID_LOG_DEBUG, "timing_loop", "gettimeofday: %d %d",
//				timing.curr_time.tv_sec, timing.curr_time.tv_usec);

	}
	return NULL;

}

void increase_ammo() {
	if (ammo_increase_counter < AMMO_INCREASE_RATE && ammo_current < ammo_max) {
		ammo_increase_counter++;
	}
	if (ammo_increase_counter == AMMO_INCREASE_RATE && ammo_current < ammo_max) {
		ammo_increase_counter = 0;
		ammo_current++;
		__android_log_print(ANDROID_LOG_DEBUG, "increase_ammo", "ammo_current %d", ammo_current);
	}
}

int decrease_ammo() { // �^�b�`����Ƃ��̏����EAMMO�����邽��
	if (ammo_current > 0) {
		ammo_current--;
		ammo_increase_counter = 0;
		return TRUE;
		__android_log_print(ANDROID_LOG_DEBUG, "increase_ammo", "ammo_current %d", ammo_current);
	}
	return FALSE;
}

//int decrease_ammo() { // �^�b�`����Ƃ��̏����EAMMO�����邽��
//
//	part* p = (parts + current_rec_part);
//
//	if (p->ammo_remaining > 0) {
//		p->ammo_remaining--;
//		__android_log_print(ANDROID_LOG_DEBUG, "decrease_ammo", "p->ammo_remaining %d", p->ammo_remaining);
//
//		return TRUE;
//	}
//	return FALSE;
//}


// main����Ă�
void record_note(float x, float y, int seg, float vel){



	part* p = (parts + current_rec_part);

//	__android_log_print(ANDROID_LOG_DEBUG, "record_note", "current_rec_part %d",
//			current_rec_part);


	int tic = p->current_tic;
	int n = p->current_note;

	p->note_info[n].pos_x = x;
	p->note_info[n].pos_y = y;

	p->note_info[n].seg = seg;
	p->note_info[n].vel = vel;

	p->note_info[n].tic = tic;

	__android_log_print(ANDROID_LOG_DEBUG, "record_note", "current_rec_part %d, current_tic %d, current_note %d",
			current_rec_part,p->current_tic, p->current_note);

	p->current_note++;

}



// ��TIC���s���Ȃ��� //
// tic��S���i�߂Ȃ��Ƃ����Ȃ�
void tic_counter() {

	int i;
	for (i = 0; i < TOTAL_PARTS; i++) {
		part* p = (parts + i);

//		if (p->is_alive) {

			if (p->current_tic >= p->total_tics && p->is_recording) {

								p->is_recording = FALSE;
								p->current_tic = 0;
	//							current_rec_part++;
	//							init_part(parts + current_rec_part, TRUE);
	//							init_part(parts + cycle_rec_part(), TRUE);

								//__android_log_print(ANDROID_LOG_DEBUG, "tic_counter", "p->current_tic:  %d, p->total_tics: %d i: %d", p->current_tic, p->total_tics, i);
								// init_part(parts + cycle_rec_part(), TRUE);

								init_part(parts + get_free_part(), TRUE);

			} else if (p->current_tic >= p->total_tics && !p->is_recording && p->is_alive) {


				__android_log_print(ANDROID_LOG_DEBUG, "tic_counter", "p->current_tic:  %d, p->total_tics: %d i: %d", p->current_tic, p->total_tics, i);
								count_part_ttl(p);
								p->current_tic = 0;

			} else if (p->current_tic < p->total_tics && p->is_recording && p->current_note > 0) {
								p->current_tic++;

			} else if (p->current_tic < p->total_tics && !p->is_recording && p->is_alive) {
								p->current_tic++;

			}

//		}
	}

}

//int cycle_rec_part() {
//
//	if (current_rec_part < TOTAL_PARTS) {
//
//		current_rec_part++;
//
//	}
//	if (current_rec_part == TOTAL_PARTS) {
//		current_rec_part = 0;
//	}
//
//	__android_log_print(ANDROID_LOG_DEBUG, "cycle_rec_part", "current_rec_part  %d", current_rec_part);
//	return current_rec_part;
//
//}

int get_free_part() { // ����������get_free_part()

	int i;
	for (i = 0; i < TOTAL_PARTS; i++) {

		if ((parts + i)->is_alive == FALSE) {
			current_rec_part = i;
			__android_log_print(ANDROID_LOG_DEBUG, "get_free_part", "current_rec_part %d", i);
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
	__android_log_print(ANDROID_LOG_DEBUG, "get_free_part", "get_oldest_part %d", current_rec_part);
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

	__android_log_print(ANDROID_LOG_DEBUG, "parts_are_active", "active %d", active);
	parts_active = active;
}

void set_parts_active() {
	parts_active = TRUE;
	not_active_count = 0;
	chord_change_count = 0;
}

void auto_play() {
	if (!parts_active && not_active_count < SILENCE_BEFORE_AUTO_PLAY) {
		not_active_count++;
	}
	if (not_active_count == SILENCE_BEFORE_AUTO_PLAY) {

		int r = obtain_random(ONESHOT_RANDOM);
		//__android_log_print(ANDROID_LOG_DEBUG, "auto_play", "(!obtain_random(50)) r %d", r);

		if (r == 0) {



			float x = (float) (obtain_random(screen_width));
			float y = (float)(obtain_random(screen_height_reduced));

			__android_log_print(ANDROID_LOG_DEBUG, "auto_play", "x %f y %f", x, y);
			play_rec_note(x, y);
		}

		if (chord_change_count < MINIMUM_CHORD_PLAY_TIME) {
			chord_change_count++;
		}

		if (chord_change_count == MINIMUM_CHORD_PLAY_TIME) {
			int s = obtain_random(CHORD_CHANGE_RANDOM);

			if (s == 0) {
				__android_log_print(ANDROID_LOG_DEBUG, "auto_play", "(obtain_random(2000)) s %d", s);
				int success = cycle_scale();
				chord_change_count = 0;
			}

		}
	}

	__android_log_print(ANDROID_LOG_DEBUG, "auto_play", "not_active_count %d", not_active_count);
	__android_log_print(ANDROID_LOG_DEBUG, "auto_play", "chord_change_count %d", chord_change_count);
}





// ���������邽�߂���
void init_all_parts() {

	total_tic_counter = 0;

	int i;
	int total_parts = TOTAL_PARTS;

	for (i = 0; i < total_parts; i++) {
		part* p = (parts + i);

		if (i == 0)
			init_part(p, TRUE);
		else
			init_part(p, FALSE);

//		p->total_tics = tics_per_part + tic_increment;
//		add_tic_increment(8);
	}
}

void init_part(part* p, int rec) {


	p->current_note = 0;
	p->current_tic = 0;
	p->is_recording = rec;
	p->play_count = 0;

	p->total_tics = tics_per_part + obtain_random(250);

	p->is_alive = rec;

	__android_log_print(ANDROID_LOG_DEBUG, "init_part", "p->total_tics  %d", p->total_tics);

	reset_all_notes(p);

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

// �p�[�g�̒������قȂ�
void add_tic_increment(int inc) {
	tic_increment += inc;
}


void play_all_parts() {

	int i;


	int total_parts = TOTAL_PARTS;

	for (i = 0; i < total_parts; i++) {
		part* p = (parts + i);

		if (!p->is_recording && p->is_alive) {

			int j;
			int total_notes = p->current_note; // ���̕�����ԑ���
			for (j = 0; j < total_notes; j++) {

				note* n = (p->note_info) + j;

				if (n->tic == p->current_tic) {

					play_note(n->seg, n->vel);
				 	__android_log_print(ANDROID_LOG_DEBUG, "play_all_parts", "total_tic_counter: %d: part: %d tic: %d current_tic: %d", total_tic_counter, i, n->tic, p->current_tic);
					//draw_note(note->pos_x, note->pos_y);

				 	//n->vel = n->vel*0.95F;
//				 	__android_log_print(ANDROID_LOG_DEBUG, "play_all_parts", "n->vel: %f", n->vel);

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
		//reset_all_notes(p); // �ĂԕK�v�Ȃ�����

		parts_are_active();

	}
	__android_log_print(ANDROID_LOG_DEBUG, "count_part_ttl", "p->play_count: %d", p->play_count);

}

// �L��p�[�g�̃m�[�g��vel�S�����₷�E����̂��߂̊֐�
void factor_part_vel(part* p, float factor) {
	int j;
	int total_notes = p->current_note;

	for (j = 0; j < total_notes; j++) {
					note* n = (p->note_info) + j;
				 	n->vel = n->vel*factor;
	}
}






//static int current_ammo;


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


