#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dlfcn.h>

#include <math.h>
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

#define MIN(A,B) ({ __typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __a : __b; })
#define MAX(A,B) ({ __typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __b : __a; })

#define CLAMP(x, low, high) ({\
  __typeof__(x) __x = (x); \
  __typeof__(low) __low = (low);\
  __typeof__(high) __high = (high);\
  __x > __high ? __high : (__x < __low ? __low : __x);\
  })

void* malloc(size_t size)
{
	static void* (*real_malloc)(size_t) = NULL;
	if (!real_malloc)
		real_malloc = dlsym(RTLD_NEXT, "malloc");

	void *p = real_malloc(size);

	int ticks = clock();
	if(ticks > 0){
		gen_square_wave(44100, CLAMP(ticks, 20, 20000), 10, 0.7);
	}

	gen_square_wave(44100, CLAMP(size, 20, 10000), 20, 0.7);

	return p;
}

ssize_t read(int fd, void * data, size_t count)
{
	static ssize_t (*real_read)(int, void*, size_t) = NULL;
	if (!real_read)
		real_read= dlsym(RTLD_NEXT, "read");
	ssize_t p = real_read(fd, data, count);
	gen_square_wave(44100, CLAMP(count, 20, 20000), CLAMP(p, 100, 1700), 0.7);
	return p;
}

