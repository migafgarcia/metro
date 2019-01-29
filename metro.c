#include <AL/al.h>
#include <AL/alc.h>
#include <math.h>
#include <stdio.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include <audio/wave.h>
#include <stdbool.h>


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

static inline ALenum to_al_format(short channels, short samples)
{
    bool stereo = (channels > 1);

    switch (samples) {
        case 16:
            if (stereo)
                return AL_FORMAT_STEREO16;
            else
                return AL_FORMAT_MONO16;
        case 8:
            if (stereo)
                return AL_FORMAT_STEREO8;
            else
                return AL_FORMAT_MONO8;
        default:
            return -1;
    }
}

void beep(int bpm) {

    float time_s = 0.1f;
    unsigned int sample_rate = 7500;
    float sound_hz = 700;
    unsigned int buffer_size = (unsigned int) (time_s * sample_rate);
    short initial_volume = (short) (0.8 * SHRT_MAX);


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

void wav(int bpm){
    int ret;
    WaveInfo *upWave, *downWave;
    char *bufferData;
    ALuint upBuffer, upSource;
    ALuint downBuffer, downSource;
    ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };



    /* set orientation */
    alListener3f(AL_POSITION, 0, 0, 1.0f);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    alListenerfv(AL_ORIENTATION, listenerOri);

    alGenSources((ALuint)1, &upSource);

    alSourcef(upSource, AL_PITCH, 1);
    alSourcef(upSource, AL_GAIN, 1);
    alSource3f(upSource, AL_POSITION, 0, 0, 0);
    alSource3f(upSource, AL_VELOCITY, 0, 0, 0);
    alSourcei(upSource, AL_LOOPING, AL_FALSE);

    alGenBuffers(1, &upBuffer);

    /* load data */
	upWave = WaveOpenFileForReading("up.wav");
	if (!upWave) {
		fprintf(stderr, "failed to read upWave file\n");
		return;
	}

	ret = WaveSeekFile(0, upWave);
	if (ret) {
		fprintf(stderr, "failed to seek upWave file\n");
		return;
	}

	bufferData = malloc(upWave->dataSize);
	if (!bufferData) {
		perror("malloc");
		return;
	}

	ret = WaveReadFile(bufferData, upWave->dataSize, upWave);
	if (ret != upWave->dataSize) {
		fprintf(stderr, "short read: %d, want: %d\n", ret, upWave->dataSize);
		return;
	}

	alBufferData(upBuffer, to_al_format(upWave->channels, upWave->bitsPerSample),
			bufferData, upWave->dataSize, upWave->sampleRate);


    alSourcei(upSource, AL_BUFFER, upBuffer);



    alGenSources((ALuint)1, &downSource);

    alSourcef(downSource, AL_PITCH, 1);
    alSourcef(downSource, AL_GAIN, 1);
    alSource3f(downSource, AL_POSITION, 0, 0, 0);
    alSource3f(downSource, AL_VELOCITY, 0, 0, 0);
    alSourcei(downSource, AL_LOOPING, AL_FALSE);

    alGenBuffers(1, &downBuffer);

    /* load data */
    downWave = WaveOpenFileForReading("down.wav");
    if (!downWave) {
        fprintf(stderr, "failed to read downWave file\n");
        return;
    }

    ret = WaveSeekFile(0, downWave);
    if (ret) {
        fprintf(stderr, "failed to seek downWave file\n");
        return;
    }

    bufferData = malloc(downWave->dataSize);
    if (!bufferData) {
        perror("malloc");
        return;
    }

    ret = WaveReadFile(bufferData, downWave->dataSize, downWave);
    if (ret != downWave->dataSize) {
        fprintf(stderr, "short read: %d, want: %d\n", ret, downWave->dataSize);
        return;
    }

    alBufferData(downBuffer, to_al_format(downWave->channels, downWave->bitsPerSample),
                 bufferData, downWave->dataSize, downWave->sampleRate);


    alSourcei(downSource, AL_BUFFER, downBuffer);

    bool up = 1;

    while (running) {
        if(up) {
            alSourcePlay(upSource);
            up = 0;
        }
        else {
            alSourcePlay(downSource);
            up = 1;
        }

        usleep((__useconds_t) (60000000 / bpm));
    }

    alDeleteSources(1, &upSource);
    alDeleteSources(1, &downSource);
    alDeleteBuffers(1, &upBuffer);
    alDeleteBuffers(1, &downBuffer);
}

int main(int argc, char** argv) {

    if(argc < 2 || argc > 2) {
        printf("Invalid arguments");
        return 0;
    }


    signal(SIGINT, stop);

    init_al();

    long bpm = strtol(argv[1], NULL, 10);

//    beep(bpm);
    wav(bpm);


    exit_al();

    return 0;
}

