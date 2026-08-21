/* newfon_speak.c -- Command line front end of the Newfon core
 *
 * Renders a koi8-r text into a 10000 Hz 16 bit mono wave file.
 * It exists for testing the core outside of NVDA.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "newfon_core.h"

#define WAVE_SIZE 4096
#define SAMPLE_RATE 10000

typedef struct
{
  FILE *file;
  unsigned long samples;
} output_t;

static int consumer(void *buffer, size_t size, void *user_data)
{
  output_t *output = user_data;
  const signed char *samples = buffer;
  size_t i;

  for (i = 0; i < size; i++)
    {
      int value = ((int) samples[i]) * 256;
      fputc(value & 0xFF, output->file);
      fputc((value >> 8) & 0xFF, output->file);
    }
  output->samples += size;
  return 0;
}

static void put32(FILE *f, unsigned long value)
{
  fputc(value & 0xFF, f);
  fputc((value >> 8) & 0xFF, f);
  fputc((value >> 16) & 0xFF, f);
  fputc((value >> 24) & 0xFF, f);
}

static void put16(FILE *f, unsigned int value)
{
  fputc(value & 0xFF, f);
  fputc((value >> 8) & 0xFF, f);
}

static void write_header(FILE *f, unsigned long samples)
{
  fwrite("RIFF", 1, 4, f);
  put32(f, 36 + samples * 2);
  fwrite("WAVEfmt ", 1, 8, f);
  put32(f, 16);
  put16(f, 1);
  put16(f, 1);
  put32(f, SAMPLE_RATE);
  put32(f, SAMPLE_RATE * 2);
  put16(f, 2);
  put16(f, 16);
  fwrite("data", 1, 4, f);
  put32(f, samples * 2);
}

static void usage(const char *name)
{
  fprintf(stderr,
          "usage: %s [-v voice] [-r rate] [-a accel] [-p pause]\n"
          "          [-P pitch] [-i inflection] -o out.wav text\n"
          "  voice      0..3: male 1, female 1, male 2, female 2\n"
          "  rate       0..150, greater is slower (default %d)\n"
          "  accel      0..7, 0 is no acceleration\n"
          "  pause      0..255 (default 100)\n"
          "  pitch      0..100 (default 50)\n"
          "  inflection 0..100 (default 50)\n"
          "The text must be in koi8-r.\n",
          name, NEWFON_RATE_DEFAULT);
}

int main(int argc, char **argv)
{
  newfon_conf_t config;
  output_t output = { NULL, 0 };
  const char *out_name = NULL;
  const char *text_file = NULL;
  char *text = NULL;
  char buffer[WAVE_SIZE];
  int i;

  newfon_config_init(&config);
  for (i = 1; (i < argc) && (argv[i][0] == '-') && argv[i][1]; i += 2)
    {
      if ((i + 1) >= argc)
        {
          usage(argv[0]);
          return 2;
        }
      switch (argv[i][1])
        {
        case 'v': config.voice = atoi(argv[i + 1]); break;
        case 'r': config.speech_rate = atoi(argv[i + 1]); break;
        case 'a': config.acceleration = atoi(argv[i + 1]); break;
        case 'p': config.pause = atoi(argv[i + 1]); break;
        case 'P': config.pitch = atoi(argv[i + 1]); break;
        case 'i': config.inflection = atoi(argv[i + 1]); break;
        case 'o': out_name = argv[i + 1]; break;
        case 'f': text_file = argv[i + 1]; break;
        case 'L':
          if (atoi(argv[i + 1]))
            config.flags |= USE_LEGACY_RATE_ALGO;
          else config.flags &= ~USE_LEGACY_RATE_ALGO;
          break;
        default: usage(argv[0]); return 2;
        }
    }
  if ((out_name == NULL) || ((i >= argc) && (text_file == NULL)))
    {
      usage(argv[0]);
      return 2;
    }

  if (text_file)
    {
      long size;
      FILE *f = fopen(text_file, "rb");
      if (!f)
        {
          fprintf(stderr, "cannot read %s\n", text_file);
          return 1;
        }
      fseek(f, 0, SEEK_END);
      size = ftell(f);
      fseek(f, 0, SEEK_SET);
      text = malloc(size + 1);
      if (!text || (fread(text, 1, size, f) != (size_t) size))
        {
          fprintf(stderr, "cannot read %s\n", text_file);
          fclose(f);
          return 1;
        }
      text[size] = 0;
      fclose(f);
    }
  else text = argv[i];

  output.file = fopen(out_name, "wb");
  if (!output.file)
    {
      fprintf(stderr, "cannot write %s\n", out_name);
      return 1;
    }
  write_header(output.file, 0);
  newfon_transfer(&config, text, buffer, sizeof(buffer), consumer, &output);
  fseek(output.file, 0, SEEK_SET);
  write_header(output.file, output.samples);
  fclose(output.file);
  printf("voice=%s samples=%lu\n", newfon_voice_name(config.voice), output.samples);
  if (text_file)
    free(text);
  return 0;
}
