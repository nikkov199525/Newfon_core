/* speechrate_control.c -- Apply speechrate parameters to a sound script
 *
 * The duration model is the one of Newfon, restored from newfon_nvda.dll:
 * only the variable part of a sound duration is scaled by the speech rate,
 * the bottom threshold is never touched, and the whole duration is finally
 * multiplied by the acceleration factor. See the comment in front of
 * apply_speechrate() below.
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 * Copyright (C) 2021 Alexander Ivanov <ivalex01@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "newfon_core.h"
#include "soundscript.h"
#include "timing.h"
#include "transcription.h"


/* Local static data */

/* Bottom duration threshold for all voice sounds */
static const uint8_t bottom[] =
  {
    30, 31, 32, 31, 30, 30,
    31, 32, 31, 30, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 30, 30, 30, 30,
    30, 15, 15, 15, 15, 15,
    30, 30, 30, 30, 30, 15,
    15, 15, 15, 15, 30, 30,
    30, 30, 30, 15, 15, 15,
    15, 15, 30, 30, 30, 30,
    30, 15, 15, 15, 15, 15,
    30, 30, 30, 30, 30, 10,
    11, 12, 11, 10, 10, 11,
    12, 11, 10, 15, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15,
    15, 15, 20, 30, 35, 20,
    40, 20, 35, 49, 30, 45,
    20, 40, 50, 40, 50, 60,
    50, 0, 20, 25, 10, 35,
    35, 20, 20, 30, 20, 35,
    40, 0, 20, 25, 0, 20,
    25, 20, 20, 30, 20, 20,
    30, 50, 50, 40, 40, 35,
    35, 50, 40, 40, 50, 30,
    30, 30, 30, 40, 50, 60,
    60, 80, 70, 70, 90, 50,
    60, 50, 50, 5, 0, 255,
    255, 0, 255, 255, 255, 255,
    255, 255, 255
  };

/* Top duration threshold for all voice sounds.
   The value for the intonational separator (index 197) is 55 as in
   Newfon; ru_tts has 15 there. */
static const uint8_t top[] =
  {
    75, 78, 80, 78, 75, 55,
    58, 60, 58, 55, 30, 30,
    30, 30, 30, 25, 25, 25,
    25, 25, 30, 30, 30, 30,
    30, 25, 25, 25, 25, 25,
    30, 30, 30, 30, 30, 25,
    25, 25, 25, 25, 30, 30,
    30, 30, 30, 25, 25, 25,
    25, 25, 60, 60, 60, 60,
    60, 25, 25, 25, 25, 25,
    50, 50, 50, 50, 50, 25,
    25, 25, 25, 25, 50, 50,
    50, 50, 50, 25, 25, 25,
    25, 25, 50, 50, 50, 50,
    50, 25, 25, 25, 25, 25,
    60, 60, 60, 60, 60, 50,
    50, 50, 50, 50, 50, 50,
    50, 50, 50, 30, 30, 35,
    30, 35, 30, 30, 30, 30,
    30, 35, 30, 30, 30, 30,
    35, 30, 40, 80, 90, 45,
    110, 50, 90, 100, 50, 110,
    40, 100, 110, 100, 100,
    110, 100, 20, 30, 30, 23,
    80, 40, 30, 30, 50, 35,
    50, 60, 19, 30, 30, 19,
    30, 30, 30, 30, 50, 30,
    30, 50, 110, 100, 80, 100,
    80, 70, 130, 120, 110, 130,
    90, 90, 80, 80, 90, 100,
    150, 150, 160, 170, 170,
    210, 130, 150, 130, 130,
    50, 0, 20, 30, 25, 25,
    40, 50, 55, 255, 55, 255
  };

/*
 * Sound duration forming elements.
 *
 * Newfon keeps the same elements as the original engine, but in a
 * normalized form: every row is expressed as a percentage of its own
 * maximum and the maximum itself, halved, is stored separately as the
 * weight of the row. Summing up round(element * weight / 100) therefore
 * yields exactly one half of the classic element sum.
 */
static const uint8_t elements[TIME_PLAN_ROWS][6] =
  {
    { 100, 100, 0, 0, 0, 0 },
    { 100, 50, 50, 0, 0, 0 },
    { 100, 20, 10, 0, 0, 0 },
    { 100, 50, 0, 0, 0, 0 },
    { 100, 50, 0, 0, 0, 0 },
    { 100, 50, 0, 0, 0, 0 },
    { 100, 80, 40, 0, 0, 0 },
    { 100, 60, 30, 0, 0, 0 },
    { 0, 20, 30, 100, 30, 40 }
  };

/* Weight of every row of the elements table, per cent */
static const uint8_t element_weights[TIME_PLAN_ROWS] =
  { 10, 25, 5, 10, 30, 15, 25, 10, 20 };

/* Round a hundredth part of the value the way the original engine does */
static uint16_t percentage(uint32_t value)
{
  return (uint16_t) (value / 100 + (((value % 100) < 50) ? 0 : 1));
}


/* Global functions */

/*
 * Initial timing setup for the specified speech rate, acceleration
 * level and interclause pause.
 */
void timing_setup(timing_t *timing, int speech_rate, int acceleration, int pause)
{
  int i;

  if (speech_rate < NEWFON_RATE_MIN)
    timing->rate = NEWFON_RATE_MIN;
  else if (speech_rate > NEWFON_RATE_MAX)
    timing->rate = NEWFON_RATE_MAX;
  else timing->rate = (uint16_t) speech_rate;

  /* Newfon rejects an acceleration level out of the [1..15] range
     and keeps the neutral one in that case. */
  if ((acceleration < 1) || (acceleration > 15))
    timing->accel = NEWFON_ACCEL_NEUTRAL;
  else timing->accel = (uint8_t) acceleration;

  if (pause < 0)
    timing->pause = 0;
  else if (pause > NEWFON_PAUSE_MAX)
    timing->pause = NEWFON_PAUSE_MAX;
  else timing->pause = (uint8_t) pause;

  for (i = 0; i < CLAUSE_SEPARATORS; i++)
    timing->gaplen[i] = top[i + 191];
}

/*
 * Apply speechrate parameters to the soundscript
 * according to specified timing data.
 *
 * Newfon evaluates a sound duration as
 *
 *   duration = (bottom[j] + (top[j] - bottom[j]) * factor / 100) * accel
 *
 * where the factor is a byte holding rate * weighted_element_sum / 70.
 * Truncation of that factor to a single byte is a property of the
 * original engine and is reproduced here: it is the reason why Newfon
 * speaks slightly faster at the very lowest rate than a bit above it.
 *
 * Interclause gaps are not affected by the speech rate at all, their
 * duration is simply the gap length multiplied by the pause factor.
 */
void apply_speechrate(soundscript_t *script, timing_t *timing, time_plan_ptr_t draft)
{
  uint16_t i;
  uint8_t n = 1;

  for (i = 0; i < script->length; i++)
    {
      uint8_t j = script->sounds[i].id;
      if (j < 189)
        {
          if ((draft[1][n] != 4) || (script->sounds[i].stage != 3))
            {
              uint32_t s = 0;
              uint8_t factor;
              uint16_t duration;
              uint8_t k;
              for (k = 0; k < TIME_PLAN_ROWS; k++)
                s += percentage(((uint32_t) elements[k][draft[k][n]]) * element_weights[k]);
              factor = (uint8_t) ((((uint32_t) timing->rate) * s / 70) & 0xFF);
              duration = bottom[j] + percentage(((uint32_t) (top[j] - bottom[j])) * factor);
              duration *= timing->accel;
              if ((draft[1][n] == 5) && (script->sounds[i].stage == 2))
                duration += duration >> 1;
              script->sounds[i].duration = duration;
            }
          else script->sounds[i].duration = 0;
          if (script->sounds[i].stage >= script->sounds[i + 1].stage)
            if (draft[1][n++] == 5)
              {
                while ((++i) < script->length)
                  {
                    script->sounds[i].duration = 0;
                    if (script->sounds[i].stage >= script->sounds[i + 1].stage)
                      break;
                  }
                n++;
              }
        }
      else
        {
          int k = j - 191;
          uint8_t gaplen = ((k >= 0) && (k < CLAUSE_SEPARATORS)) ? timing->gaplen[k] : top[j];
          script->sounds[i].duration = (uint16_t) (((unsigned int) gaplen) * timing->pause);
        }
    }
}
