/*
 * scales.h
 *
 *  Created on: 2013/06/02
 *      Author: Michael
 *
 *      ���K�Ǘ�
 *
 *      Scale Management
 */

#ifndef SCALES_H_
#define SCALES_H_

#define TOTAL_NOTES 24

//#define TOTAL_SCALES 5
#define START_NOTE 48// MIDI�ԍ�

//#include "snd_asst.h"

struct sample_def;

struct scale {

	int midimap[TOTAL_NOTES];


	struct sample_def* looping_sample;

};








//typedef struct scale {
//
//	//int id; //�o�b�t�@�[�̂���
////	char* name;
//
//	// ��ʂ̕�����MIDI�m�[�g��
//	int midimap[TOTAL_NOTES];
////	int chord_loop; // ���̕��@������͂���
//
//	sample_def* looping_sample;
//
//} scale;
//






//void play_note(int segment, float vel);

//sample_def* get_scale_sample(int seg);



void start_loop();
//int cycle_scale();

#endif /* SCALES_H_ */
