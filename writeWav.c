#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dlfcn.h>

#include <math.h>
#include <sndfile.h>
#include <malloc.h>

int gen_square_wave(int sample_rate, int frequency, int duration, float amplitude)
{
	int samples       = sample_rate * duration / 1000;
	int tone_midpoint = sample_rate / frequency / 2;
	int sample        = -(1 << (13 - 1)) * amplitude;

	int i;
	for(i=0; i < samples; i++)
	{
		if(i % tone_midpoint == 0)
			sample = -sample;

		printf("%c%c", sample & 0xff, (sample >> 8) & 0xff);
	}

	return 0;
}

void* malloc(size_t size)
{
	static void* (*real_malloc)(size_t) = NULL;
	if (!real_malloc)
		real_malloc = dlsym(RTLD_NEXT, "malloc");

	void *p = real_malloc(size);
	gen_square_wave(44100, size, 100, 0.2);
	return p;
}
