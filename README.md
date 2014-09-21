To build, run ```make```, you know the drill. ;)

To try it out, run the ```writeWav``` script with the program and parameters you with to experiment with.
A wav file will be created in the current directory with a name of the current timestamp if you run with writeWav.

Otherwise, if you wish, you may LD_PRELOAD writeWav.so manually. If you preload manually, however, there are some environment vars that you should be aware of:

__Required__:
*    writeWav_out (default: none)

__Optional__:
*    writeWav_noread (default: false) If true, we will not use read() calls to generate data.
*    writeWav_nomalloc (default: false) If true, we will not use malloc() calls to generate data.


If you want to tweak the output, you may do so in ```writeWav.c``` near the top:


```c
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
```



You can listen to some examples of the output here:

* https://soundcloud.com/glowdon/jingy-compiler-1
* https://soundcloud.com/glowdon/the-sounds-of-malloc
* https://soundcloud.com/glowdon/malloc-read-hook-self-compile



Many thanks to [@davidfoerster](https://github.com/davidfoerster) and others for the forks and improvements!
