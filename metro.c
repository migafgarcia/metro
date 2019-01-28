#include <stdio.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>


static volatile int keepRunning = 1;

void stop() {
    keepRunning = 0;
}

void init_al()
{
    const char *defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    ALCdevice* dev = alcOpenDevice(defname);
    ALCcontext *ctx = alcCreateContext(dev, NULL);
    alcMakeContextCurrent(ctx);
}

void exit_al()
{
    ALCcontext* ctx = alcGetCurrentContext();
    ALCdevice* dev = alcGetContextsDevice(ctx);
    alcMakeContextCurrent(0);
    alcDestroyContext(ctx);
    alcCloseDevice(dev);
}

void beep(int bpm) {


    float freq = 440;
    float seconds = 0.2;
    ALuint buf;
    alGenBuffers(1, &buf);

    unsigned sample_rate = 44100;
    size_t buf_size = (size_t) (seconds * sample_rate);
    short *samples = malloc(buf_size * sizeof(short));

    for (unsigned i = 0; i < buf_size; i++)
        samples[i] = (short) (10000 * sin(1.2 * M_PI * i * freq / sample_rate));

    alBufferData(buf, AL_FORMAT_STEREO16, samples, buf_size, sample_rate);


    ALuint src;
    alGenSources(1, &src);
    alSourcei(src, AL_BUFFER, buf);

    while (keepRunning) {
        alSourcePlay(src);
        usleep((__useconds_t) (60000000 / bpm));
    }

    free(samples);

    alSourceStopv(1,&src);
    alDeleteSources(1,&src);
    alDeleteBuffers(1,&buf);
}


int main(int argc, char **argv) {

    if(argc < 2 || argc > 2) {
        printf("Invalid arguments");
        return 0;
    }

    signal(SIGINT, stop);

    init_al();

    int bpm = atoi(argv[1]);

    beep(bpm);

    exit_al();
}