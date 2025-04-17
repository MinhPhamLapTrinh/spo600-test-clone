// clone-test-core.c
// Chris Tyler 2017.11.29‑2024.11.19 - Licensed under GPLv3.
// For the Seneca College SPO600 Course
//
// Now with *two* cloned functions: sum_sample and scale_samples.
// We’ll generate PRUNE for sum_sample (identical clones) and
// NOPRUNE for scale_samples when FORCE_SCALE_DIFF is defined.

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "vol.h"

/* Annotate sum_sample so that GCC generates two clones */
CLONE_ATTRIBUTE
int sum_sample (int16_t *buff, size_t samples) {
    int t = 0;
    for (size_t x = 0; x < samples; x++) {
        t += buff[x];
    }
    return t;
}

/* Annotate scale_samples likewise */
CLONE_ATTRIBUTE
void scale_samples(int16_t *in, int16_t *out, int cnt, int volume) {
    for (int x = 0; x < cnt; x++) {
        out[x] = ((((int32_t) in[x]) * ((int32_t) (32767 * volume / 100) << 1)) >> 16);
    }

    /* This extra statement will only be compiled in when
       we pass -DFORCE_SCALE_DIFF → it makes the IR differ. */
#ifdef FORCE_SCALE_DIFF
    /* introduce one extra GIMPLE stmt so clones differ */
    int tmp = cnt;
    (void) tmp;
#endif
}

int main() {
    int     ttl = 0;

    // ---- Allocate and fill in[]
    int16_t *in  = calloc(SAMPLES, sizeof(int16_t));
    int16_t *out = calloc(SAMPLES, sizeof(int16_t));
    vol_createsample(in, SAMPLES);

    // ---- Call our two cloned functions
    scale_samples(in, out, SAMPLES, VOLUME);
    ttl = sum_sample(out, SAMPLES);

    // ---- Print result
    printf("Result: %d\n", ttl);
    return 0;
}