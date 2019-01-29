#include <AL/al.h>
#include <AL/alc.h>
#include <math.h>
#include <stdio.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>


static volatile int running = 1;

void stop() {
    running = 0;
}

int sign(double value) {
    if(value < 0)
        return -1;
    else if(value == 0)
        return 0;
    else
        return 1;
}

double cot(double x) {
    return cos(x) / sin(x);
}

short sine_wave(int time, float sound_hz, unsigned int buffer_size, short volume, int multiplier) {
    return (short) (multiplier * volume *  sin(M_2_PI * sound_hz * time / buffer_size));
}

short square_wave(int time, float sound_hz, unsigned int buffer_size, short volume, int multiplier) {
    return (short) (multiplier * volume * sign(sin( sound_hz * time / buffer_size)));
}

short sawtooth_wave(int time, float sound_hz, unsigned int buffer_size, short volume, int multiplier) {
    return (short) -(multiplier * M_PI_2 * volume * atan(cot( M_PI * sound_hz * time / buffer_size)));
}

void init_al() {
    const char *defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    ALCdevice* dev = alcOpenDevice(defname);
    ALCcontext *ctx = alcCreateContext(dev, NULL);
    alcMakeContextCurrent(ctx);
}

void exit_al() {
    ALCcontext* ctx = alcGetCurrentContext();
    ALCdevice* dev = alcGetContextsDevice(ctx);
    alcMakeContextCurrent(0);
    alcDestroyContext(ctx);
    alcCloseDevice(dev);
}

void beep(int bpm) {

    float time_s = 0.1f;
    unsigned int sample_rate = 7500;
    float sound_hz = 700;
    unsigned int buffer_size = (unsigned int) (time_s * sample_rate);
    short initial_volume = (short) (0.8 * SHRT_MAX);


    init_al();

    ALshort data[buffer_size * 2];
    ALuint buffer, source;
    int i;

    alGenBuffers(1, &buffer);

    for (i = 0; i < buffer_size; i++) {
        data[i * 2] = sine_wave(i, sound_hz, buffer_size, (short) (initial_volume - (i * initial_volume / buffer_size)), 1);
        data[i * 2 + 1] = sine_wave(i, sound_hz, buffer_size, (short) (initial_volume - (i * initial_volume / buffer_size)), -1);
    }

    alBufferData(buffer, AL_FORMAT_STEREO16, data, sizeof(data), sample_rate);
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);

    while (running) {
        alSourcePlay(source);
        usleep((__useconds_t) (60000000 / bpm));
    }

    alSourceStop(source);
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
}

int main(int argc, char** argv) {

    if(argc < 2 || argc > 2) {
        printf("Invalid arguments");
        return 0;
    }


    signal(SIGINT, stop);

    long bpm = strtol(argv[1], NULL, 10);

    beep(bpm);

    exit_al();

    return 0;
}

