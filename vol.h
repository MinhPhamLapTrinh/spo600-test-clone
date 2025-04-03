/* This is the number of samples to be processed */
#define SAMPLES 50000000 

/* This is the volume scaling factor to be used */
#define VOLUME 50.0 // Percent of original volume

/* Function prototype to fill an array sample of 
 * length sample_count with random int16_t numbers
 * to simulate an audio buffer */
void vol_createsample(int16_t* sample, int32_t sample_count);

