#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "portaudio.h"
#include "sndfile.h"
#include "lo/lo.h"

#define VL (32)

float   last_samp = 0.0;
long    start = 0;
long    block_count = 0; 
long    min_period = 100000;
long    prev_period = 100000;

void pedalIn(float *x)
{
    float samp;
    int i;
    char message[30];
    lo_blob messageObj = lo_blob_new(sizeof(message), message) ;
    lo_address address = lo_address_new(NULL, "7770");

    for (i = 0; i < VL; i++) {
        samp = x[i];
        //printf("%f\n", samp);
        if ((last_samp < 0) && (samp >= 0)) {
            long count = block_count * VL + i;
			long period = count - start;
			if (period < min_period) {
                min_period = period;
            }
			start = count;
        }
        last_samp = samp; 
    }
    if (!(block_count & 7)) {
        if ((prev_period > 20) && (min_period <= 20)) {
            
            sprintf(message, "pedal_trigger set on block %d", block_count);
            
            if (lo_send(address, "/foo/bar", "si", "pedal_triger set", block_count) == -1) {
	            printf("OSC error %d: %s\n", lo_address_errno(address), lo_address_errstr(address));
            }
            //pedal_trigger = true;
		}

		prev_period = min_period;
		min_period = 100000; 
    }
    block_count++; 
}

int main (int argc, char *argv[]) {
    
    if (argc != 2) {
       fprintf(stderr, "Expecting wav file as argument\n");
       return 1;
    }

    SF_INFO sndInfo;
    SNDFILE *sndFile = sf_open(argv[1], SFM_READ, &sndInfo);
    if (sndFile == NULL) {
        fprintf(stderr, "Error reading source file '%s': %s\n", argv[1], sf_strerror(sndFile));
        return 1;
    }

    float *wavArray = malloc(sndInfo.frames * sizeof(float) * sndInfo.channels);
    if (wavArray == NULL) {
        fprintf(stderr, "Could not allocate memory for data\n");
        sf_close(sndFile);
        return 1;
    }

    // Load data
    long numFrames = sf_read_float(sndFile, wavArray, sndInfo.frames);
    if (numFrames != sndInfo.frames) {
        fprintf(stderr, "Did not read enough frames for source\n");
        sf_close(sndFile);
        free(wavArray);
        return 1;
    }

    while (block_count * 32 <= sndInfo.frames) {
        //printf("input: %f\n", (wavArray + (block_count * 32 - 1))[0]);
        pedalIn(wavArray + (block_count * 32 - 1));
    }

    free(wavArray);
    return 0;
}
