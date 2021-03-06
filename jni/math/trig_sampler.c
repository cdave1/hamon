// trig_sampler.c

#include "common.h"
#include "hon_type.h"
#include "math/trig_sampler.h"
#include "gfx/frame_delta.h"

double deg_per_sample = (double)360.0/(double)SAMPLES_PER_CYCLE;

struct modulator modulators[] = {
		{NULL, MOD_SINE, 22050, 3.45f, 0.2f, 0.0f}
};

int sizeof_modulators_elements = sizeof modulators/sizeof modulators[0];

double deg_to_rad(double deg);
void log_all_sine_vals();
void init_modulators(struct modulator* m);
void calc_samples_per_us(struct modulator* m);

//ラジアンから角度への変換計算式 :
//角度 = ラジアン x 180 / 円周率

//角度からラジアンへの変換計算式 :
//ラジアン = 角度 x 円周率 / 180
double deg_to_rad(double deg) {
	double rad = deg * (double)PI /(double)180;
	return rad;
}

void init_all_trig_samples() {
	int i;
	for (i = 0; i < sizeof_modulators_elements; i++) {
		init_modulators(modulators + i);
		calc_samples_per_us(modulators + i);
	}
}

void init_modulators(struct modulator* m) {

	m->mod_cycle = (double*) malloc(sizeof(double)*SAMPLES_PER_CYCLE);
	double val;
	int i;
	for (i=0; i < SAMPLES_PER_CYCLE; i++) {
		val = sin(deg_to_rad(((double)i * deg_per_sample)));
		val =  ((val + 1.0F)/2.0F) * m->amp + (1.0F - m->amp); //   ((val/2.0F) + 0.5F) * m->amp +;
		m->mod_cycle[i] = val;
	}
}

void log_all_sine_vals(struct modulator* m) {
	int i;
	for (i = 0; i < SAMPLES_PER_CYCLE; i++) {
		LOGD("cycle_modulator", "i: %d, test: %f\n", i,  m->mod_cycle + i);
	}
}

void calc_samples_per_us(struct modulator* m) {
	m->samples_per_us= (double)SAMPLES_PER_CYCLE/(double)(SEC_IN_US* m->period);
	LOGD("calc_samples_per_us", "m->samples_per_us: %f\n", m->samples_per_us);
}

void cycle_modulator(struct modulator* m) {
	m->curr_samp = m->curr_samp + floor(m->samples_per_us * frame_delta); //always floor to stop???
	if (m->curr_samp >= SAMPLES_PER_CYCLE) {
		m->curr_samp = m->curr_samp - SAMPLES_PER_CYCLE;
	}

//	LOGD("cycle_modulator", "m->curr_samp: %d, hi_res_sin[m->curr_samp]: %f\n",
//			m->curr_samp, hi_res_sin[m->curr_samp]);
}
