#include <stdio.h>
#include <unistd.h>
#include "support.h"


/*
 * General wave generation parameters.
 *
 * parameter 1 - wave frequency in Hertz; should be clamped to a sane range
 *   (e. g. 20-20000 Hz) with CLAMP_u(freq, min, max)
 * parameter 2 - wave duration in milliseconds
 * parameter 3 - wave amplitude (0.0-1.0)
 */

/*
 * Size-based wave generatrion for malloc() calls.
 *
 * size - allocated memory size in bytes
 */
#define MALLOC_WAVE_PARAMS(size) \
  CLAMP_u(size, 20u, 10000u), 20, 0.7f

/*
 * Size and runtime based wave generation for malloc() calls.
 *
 * size - see above
 * ticks - number of consumed CPU clock ticks since program start
 */
#define MALLOC_WAVE_TICKS_PARAMS(size, ticks) \
  CLAMP_u(ticks, 20u, 20000u), 10, 0.7f

/*
 * Size based wave generation for read() calls.
 *
 * requested - requested amount of bytes to read
 * returned - actual amounts of bytes read
 */
#define READ_WAVE_PARAMS(requested, returned) \
  CLAMP_u(requested, 20u, 20000u), CLAMP_u(returned, 100, 1700), 0.7f


// foward declarations ================================================

void* malloc(size_t size);

ssize_t read(int fd, void * data, size_t count);

static void gen_square_wave(unsigned frequency, unsigned duration, float amplitude);

static void __gen_square_wave_impl(unsigned samples, unsigned half_period_length, sample_t sample);


// implementations ====================================================

void* malloc(size_t size)
{
  static __thread volatile bool inside_malloc = false;

  void *const p = __malloc_data.real_malloc(size);

  if (!__malloc_data.nomalloc && !inside_malloc)
  {
    inside_malloc = true;

    flockfile(__wave_out);
    gen_square_wave(MALLOC_WAVE_TICKS_PARAMS(size,
      clock() - __malloc_data.ticks_start));
    gen_square_wave(MALLOC_WAVE_PARAMS(size));
    fflush_unlocked(__wave_out);
    funlockfile(__wave_out);

    inside_malloc = false;
  }

  return p;
}


ssize_t read(int fd, void * data, size_t count)
{
  const ssize_t p = __read_data.real_read(fd, data, count);

  if (!__read_data.noread)
  {
    flockfile(__wave_out);
    gen_square_wave(READ_WAVE_PARAMS(count, p));
    fflush_unlocked(__wave_out);
    funlockfile(__wave_out);
  }

  return p;
}


static inline
void gen_square_wave(unsigned frequency, unsigned duration, float amplitude)
{
  assert(amplitude >= 0.f && amplitude <= 1.f);

  __gen_square_wave_impl(
    DIV_ROUND(SAMPLING_RATE * duration, 1000),
    MAX(DIV_ROUND(SAMPLING_RATE / 2, frequency), 1u),
    #ifndef SAMPLING_FORMAT_FLOAT
      (sample_t)((float) INTTYPE_MAX(sample_t) * amplitude)
    #else
      amplitude
    #endif
    );
}


static void __gen_square_wave_impl(unsigned samples, unsigned half_period_length, sample_t sample)
{
  assert(__wave_out != NULL);
  assert(samples == 0 || half_period_length != 0);

  for (unsigned half_period_start = 0;
       half_period_start < samples;
       half_period_start += half_period_length)
  {
    const unsigned half_period_end =
        MIN(half_period_start + half_period_length, samples);
    for (unsigned i = half_period_start; i != half_period_end; i++)
      fwrite_unlocked(&sample, sizeof(sample), 1, __wave_out);

    #ifndef SAMPLING_FORMAT_FLOAT
      sample = (sample_t)(IS_SIGNED(sample_t) ? -sample : ~sample);
    #else
      sample = -sample;
    #endif
  }
}
