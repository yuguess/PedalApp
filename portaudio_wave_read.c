#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"
#include "sndfile.h"

#define FRAMES_PER_BUFFER (1024)
#define	BUFFER_LEN	(1024)
#define PA_SAMPLE_TYPE  paFloat32

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
/*
    float *wavArray = malloc(sndInfo.frames * sizeof(float) * sndInfo.channels);
    if (wavArray == NULL) {
        fprintf(stderr, "Could not allocate memory for data\n");
        sf_close(sndFile);
        return 1;
    }
*/
    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;

    err = Pa_Initialize();
    if( err != paNoError ) goto done;

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default output device.\n");
        goto done;
    }

	outputParameters.channelCount = sndInfo.channels;
	outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
		&stream,
		NULL,
		&outputParameters,
		sndInfo.samplerate,
		FRAMES_PER_BUFFER,
		paClipOff,
		NULL,
		NULL );
    if (err != paNoError) goto done;

    float wavPtr[BUFFER_LEN];
    int readcount;

	if (stream) {

		err = Pa_StartStream( stream );
        
		while ((readcount = sf_read_float(sndFile, wavPtr, BUFFER_LEN))) {  
			err = Pa_WriteStream( stream, wavPtr, BUFFER_LEN );
		}
		err = Pa_CloseStream( stream );
		printf("Done.\n"); fflush(stdout);

	}
    sf_close(sndFile);
    //free(wavArray);
done:
    Pa_Terminate();
    if (err != paNoError)
    {
        fprintf( stderr, "An error occured while using the portaudio stream\n" );
        fprintf( stderr, "Error number: %d\n", err );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
        err = 1;          // Always return 0 or 1, but no other return codes. 
    }
    return err; 
}
