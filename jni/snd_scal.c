/*
 * scales.c
 *
 *  Created on: 2013/06/02
 *      Author: Michael
 */

// �����̏���������邽��

#include "snd_asst.h"
#include "snd_sles.h"
#include <android/log.h>

#include "snd_ctrl.h"

//C2-B4
//48 - 83

#define TOTAL_NOTES 24
#define START_NOTE 48// MIDI�ԍ�

int selected_scale = 1;
extern sample_def oneshot_samples[];

typedef struct {

	//int id; //�o�b�t�@�[�̂���
	char* name;

	// ��ʂ̕�����MIDI�m�[�g��
	int midimap[24];
	int chord_loop; // ���̕��@������͂���

} scale_def;


// ���l����Έ�ԊȒP�ȕ��@�Ƃ����̂�
// �󔒂ȂƂ���͗~������΁A�����m�[�g�����Ă��\���܂���B

// ���̔z��̒��Ƀ��[�v���Ă��鉹�̏������ׂ��I
scale_def scales[] = {

		{ "test scale",
				{
						48, 48, 48, 48, 48, 48, 48, 48,
						49, 49, 49, 49, 49, 49, 49, 49,
						50, 50, 50, 50, 50, 50, 50, 50
				},
				1
		},

		{ "major scale",
				{
						48, 48, 50, 52, 53, 55, 57, 59,
						60, 62, 64, 65, 67, 69, 71, 72,
						74, 76, 77, 79, 81, 83, 83, 83
				},
				2
		},

		{ "minor scale",
				{
						49, 49, 51, 51, 54, 54, 56, 56,
						58, 58, 61, 61, 63, 66, 68, 70,
						73, 75, 78, 78, 80, 80, 82, 82
				},
				3
		}


};


// ���̊֐��͕K�v�Ȃ�����
// �����������邽�߂����̊֐�
void play_loop() {
	int success = enqueue_seamless_loop(looping_samples + selected_scale);
}

void play_note(int segment, float vel) {
	int sample = scales[selected_scale].midimap[segment];
	sample -= START_NOTE;

	int success = enqueue_one_shot(oneshot_samples + sample, vel);

	__android_log_print(ANDROID_LOG_DEBUG, "play_note", "vel: %f", vel);

}

int cycle_scale() {

	// �����Ńt�F�[�h�����ĂȂ������m�F���Ȃ���
	if(current_voice_fading()) {

		__android_log_write(ANDROID_LOG_DEBUG, "cycle_scale", "current_voice_fading()");
		return 0;
	}

	__android_log_write(ANDROID_LOG_DEBUG, "cycle_scale", "cycle_scale() called");
	int total_scales = sizeof scales / sizeof scales[0];

	if (selected_scale < total_scales)
		selected_scale += 1;

	if (selected_scale == total_scales)
		selected_scale = 0;

	int success = enqueue_seamless_loop(looping_samples + selected_scale);


	return 1;
}
