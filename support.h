#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <assert.h>


#if CHAR_BIT != 8
#error "Unsupported byte length"
#endif


#ifndef SAMPLING_FORMAT
#define SAMPLING_FORMAT int16_t
#endif

#ifndef SAMPLING_RATE
#define SAMPLING_RATE 44100
#endif

typedef SAMPLING_FORMAT sample_t;


#define MIN(A,B) ({ __typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __a : __b; })
#define MAX(A,B) ({ __typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __b : __a; })
#define CLAMP(x, low, high) MIN(MAX(x, low), high)
#define CLAMP_u(x, min, max) ((unsigned) CLAMP(x, min, max))

#define __getbyte(x, n) ((x >> (n * CHAR_BIT)) & 0xff)
#define __byteswap(x, n, c) (__getbyte(x, n) << ((c - n - 1) * CHAR_BIT))
#define byteswap32(x) ( \
  __byteswap((x), 0, 4) | __byteswap((x), 1, 4) | \
  __byteswap((x), 2, 4) | __byteswap((x), 3, 4))


struct  __malloc_data {
  bool nomalloc;
  void* (*real_malloc)(size_t);
  clock_t ticks_start;
};

struct __read_data {
  bool noread;
  ssize_t (*real_read)(int, void*, size_t);
};


extern __attribute__ ((visibility ("hidden")))
FILE *__wave_out;

extern __attribute__ ((visibility ("hidden")))
struct __malloc_data __malloc_data;

extern __attribute__ ((visibility ("hidden")))
struct __read_data __read_data;


#ifdef NDEBUG
  #define dbgprintf(format, ...) ((void) 0)
#else
  #define dbgprintf(format, ...) ((void) fprintf(stderr, format, __VA_ARGS__))
#endif


// macro tests ========================================================

#if byteswap32(0x78563412) != 0x12345678
#error
#endif
