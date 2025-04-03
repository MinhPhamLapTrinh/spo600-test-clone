#include <stdlib.h>
#include <stdint.h>
#include "vol.h"

void vol_createsample(int16_t* sample, int32_t sample_count) {
	int i;
	for (i=0; i<sample_count; i++) {
		sample[i] = (rand()%65536)-32768;
	}
	return;
}

