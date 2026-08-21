/* text2speech.c -- Full TTS transfer
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
#include "sink.h"
#include "timing.h"
#include "modulation.h"
#include "transcription.h"
#include "synth.h"
#include "voice.h"


/* Common entry points */

/*
 * Initialize configuration structure with the default values.
 */
NEWFON_EXPORT void newfon_config_init(newfon_conf_t *config)
{
  config->voice = NEWFON_MALE_1;
  config->speech_rate = NEWFON_RATE_DEFAULT;
  config->acceleration = NEWFON_ACCEL_OFF;
  config->pitch = NEWFON_PITCH_DEFAULT;
  config->inflection = NEWFON_INFLECTION_DEFAULT;
  config->pause = 100;
  config->flags = DEC_SEP_POINT | DEC_SEP_COMMA | USE_LEGACY_RATE_ALGO;
}

/*
 * Return the name of the specified voice.
 */
NEWFON_EXPORT const char *newfon_voice_name(int voice)
{
  if ((voice < 0) || (voice >= NEWFON_VOICE_COUNT))
    return NULL;
  return newfon_voices[voice].name;
}

/*
 * Perform TTS transformation for specified text.
 *
 * The first argument points to a configuration structure.
 * The second argument points to a zero-terminated string to transfer.
 * This string must contain a Russian text in koi8-r.
 * The next two arguments specify a buffer that will be used
 * by the core for delivering produced wave data
 * chunk by chunk to the consumer specified by the fourth argument.
 * The next argument points to any additional user data passed to the consumer.
 */
NEWFON_EXPORT void newfon_transfer(const newfon_conf_t *config,
                                   const char *text, void *wave_buffer, size_t wave_buffer_size,
                                   newfon_callback consumer, void *user_data)
{
  uint8_t *transcription_buffer = malloc(TRANSCRIPTION_BUFFER_SIZE);

  if (transcription_buffer)
    {
      ttscb_t ttscb = {0};
      sink_t transcription_consumer;

      /* Initialize data structures */
      sink_setup(&(ttscb.wave_consumer), wave_buffer, wave_buffer_size, consumer, user_data);
      sink_setup(&transcription_consumer, transcription_buffer, TRANSCRIPTION_MAXLEN, synth_function, &ttscb);
      ttscb.flags = config->flags;
      ttscb.voice = config->voice;

      /* Adjust speech rate, acceleration and pauses */
      timing_setup(&(ttscb.timing), config->speech_rate, config->acceleration, config->pause);

      /* Take the tone range of the voice */
      modulation_setup(&(ttscb.modulation), config->voice, config->pitch, config->inflection);

      /* Process text */
      process_text(text, &transcription_consumer);
      free(transcription_buffer);
    }
}
