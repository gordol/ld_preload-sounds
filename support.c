#define _GNU_SOURCE
#include "support.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>


FILE *__wave_out;

struct __malloc_data __malloc_data;

struct __read_data __read_data;

static int (*real_fclose)(FILE*);


static void init_all() __attribute__((constructor));

static void init_wave_out();
static void finish_wave_out();

static void init_malloc_data();

static void init_read_data();

static bool getenv_bool(const char *name, bool default_value);

int fclose(FILE*);


static void init_all()
{
  init_malloc_data();
  init_read_data();
  init_wave_out();

  real_fclose = dlsym(RTLD_NEXT, "fclose");
}


static void init_malloc_data()
{
  dbgprintf("entering %s\n", __func__);
  __malloc_data.real_malloc = dlsym(RTLD_NEXT, "malloc");
  dbgprintf("real malloc is at %p\n", __malloc_data.real_malloc);

  __malloc_data.nomalloc = getenv_bool("writeWav_nomalloc", false);
  __malloc_data.ticks_start = clock();
}


static void init_read_data()
{
  dbgprintf("entering %s\n", __func__);
  __read_data.real_read = dlsym(RTLD_NEXT, "read");
  dbgprintf("real read is at %p\n", __read_data.real_read);

  __read_data.noread = getenv_bool("writeWav_noread", false);
}


static bool getenv_bool(const char *name, bool default_value)
{
  const char *strvalue = getenv(name);
  return (strvalue && *strvalue) ? !!atoi(strvalue) : default_value;
}


// WAVE writing =======================================================

struct RIFF_header {
    uint32_t ID;
    uint32_t Size;
    uint32_t Format;

    uint32_t SubID_1;
    uint32_t SubSize_1;
    uint16_t AudioFmt;
    uint16_t Channel;
    uint32_t Frequency;
    uint32_t bytesRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;

    uint32_t SubID_2;
    uint32_t SubSize_2;
} __attribute__((packed));

struct wave_header_info {
  off_t offset;
  struct RIFF_header data;
};


#define CHANNELS 1

#define floor2multipleof(n, m) ((n) - (n) % (m))
#define WAVEDATA_MAXLENGTH \
  floor2multipleof(~(uint32_t) 0 - sizeof(struct RIFF_header), sizeof(sample_t) * CHANNELS * CHAR_BIT)
#define WAVEFILE_SIZE(datalen) \
  ((datalen) + sizeof(struct RIFF_header) - 2 * sizeof(uint32_t))

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultichar"

static struct wave_header_info wave_header = { -1, {
  byteswap32('RIFF'),
  WAVEFILE_SIZE(WAVEDATA_MAXLENGTH), // file size - 8 (>= 36, to be filled out later)
  byteswap32('WAVE'), // RIFF type:

  byteswap32('fmt '), // format chunk ID
  16, // format chunk size = 16
  1, // format type (PCM = 1)
  CHANNELS, // channel count (mono = 1)
  SAMPLING_RATE, // sampling rate
  SAMPLING_RATE * CHANNELS * sizeof(sample_t), // byte rate
  sizeof(sample_t) * CHANNELS, // block align
  sizeof(sample_t) * CHAR_BIT, // bits per sample

  byteswap32('data'), // data chunk ID
  WAVEDATA_MAXLENGTH // data size (to be filled out later)
}};

#pragma GCC diagnostic pop


static void init_wave_out()
{
  dbgprintf("entering %s\n", __func__);
  const char *dst = getenv("writeWav_out");

  if (dst && *dst)
  {
    bool nomalloc = __malloc_data.nomalloc;
    __malloc_data.nomalloc = true;

    if (*dst == '&') {
      char end;
      int fd;
      int n = sscanf(dst + 1, "%d%c", &fd, &end);
      assert(n <= 2);
      if (n > 0 && (n == 1 || end == '\0') && fd != -1) {
        __wave_out = fdopen(fd, "w");
      } else {
        __wave_out = NULL;
        errno = EINVAL;
      }
    } else {
      __wave_out = fopen(dst, "w");
    }

    __malloc_data.nomalloc = nomalloc;
  }
  else
  {
    dst = "<stdout>";
    __wave_out = stdout;
    errno = EBADF;
  }

  if (__wave_out)
  {
    //if (atexit(finish_all) == 0)
    {
      wave_header.offset = ftello(__wave_out);
      if (wave_header.offset > 0 ||
        fwrite(&wave_header.data, sizeof(wave_header.data), 1, __wave_out) == 1)
      {
        dbgprintf("wave_out file descriptor is %d\n", fileno(__wave_out));
        return;
      }
    }
    /*
    else
    {
      Dl_info dl_info;
      if (!dladdr(finish_wave_out, &dl_info))
        dl_info.dli_fname = "<unknown>";
      fprintf(stderr,
              "Could not register exit handler for shared library `%s'.\n",
              dl_info.dli_fname);
      goto fail;
    }
    */
  }

  fprintf(stderr, "Error: Cannot use `%s' to write wave data: %s\n"
    "Aborting!\n", dst, strerror(errno));
//fail:
  exit(127);
}


static void finish_wave_out()
{
  if (!__wave_out)
    return;

  dbgprintf("entering %s, wave_out is %p\n", __func__, __wave_out);
  if (wave_header.offset != 0)
    goto end;

  off_t wave_end = ftello(__wave_out);
  dbgprintf("%zd bytes of wave data written.\n", wave_end);

  if (wave_end >= wave_header.offset + (off_t) sizeof(wave_header.data))
  {
    if (wave_end - wave_header.offset <= ~(uint32_t) 0)
    {
      if (fseeko(__wave_out, wave_header.offset, SEEK_SET) == 0)
      {
        wave_header.data.SubSize_2 = (uint32_t)
            (wave_end - wave_header.offset - (off_t) sizeof(wave_header.data));
        assert(wave_header.data.SubSize_2 % wave_header.data.BlockAlign == 0);
        wave_header.data.Size = (uint32_t)
            WAVEFILE_SIZE(wave_header.data.SubSize_2);

        if (fwrite(&wave_header.data, sizeof(wave_header.data), 1, __wave_out) == 1
          && fseeko(__wave_out, wave_end, SEEK_SET) == 0)
        {
          dbgprintf("RIFF file size set to %u, wave data size set to %u.\n",
            wave_header.data.Size, wave_header.data.SubSize_2);
          goto end;
        }
      }
    }
    else
      errno = E2BIG;
  }
  else
    errno = 0;

  perror("Cannot set correct file size in WAVE header");

end:
  __wave_out = NULL;
}


/*
 * A terrible hack to call our "destructor" before the necessary streams are
 * closed.
 */
int fclose(FILE *f)
{
  if (f && (f == __wave_out || fileno(f) <= 2))
    finish_wave_out();

  return real_fclose(f);
}
