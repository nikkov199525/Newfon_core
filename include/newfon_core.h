/* newfon_core.h -- Public interface of the Newfon speech core
 *
 * The core is the Minsk "Phonemophone" synthesizer engine (as published in
 * the ru_tts project) with the voice set, the pitch handling and the speech
 * rate/acceleration model of Newfon by Sergey Shishmintsev restored from the
 * original newfon_nvda.dll.
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 * Copyright (C) 2021 Alexander Ivanov <ivalex01@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef NEWFON_CORE_H
#define NEWFON_CORE_H

#include <stdlib.h>


#if defined(_WIN32) || defined(_WIN64)
#define NEWFON_EXPORT __declspec(dllexport)
#else
#define NEWFON_EXPORT
#endif

/* TTS control flags */
#define DEC_SEP_POINT 1 /* Use point as a decimal separator */
#define DEC_SEP_COMMA 2 /* Use comma as a decimal separator */
#define USE_LEGACY_RATE_ALGO 8 /* Use the original sound transition algorithm of
                                  Newfon; without this flag the adaptive
                                  powf-based crossfade of ru_tts is used */

/* Voices. The tone range of every voice is wired into the core
   and cannot be altered by any configuration parameter. */
#define NEWFON_VOICE_COUNT 4
#define NEWFON_MALE_1 0
#define NEWFON_FEMALE_1 1
#define NEWFON_MALE_2 2
#define NEWFON_FEMALE_2 3

/* Speech rate range. As in Newfon, the value is a duration factor:
   0 gives the fastest speech, NEWFON_RATE_MAX the slowest one. */
#define NEWFON_RATE_MIN 0
#define NEWFON_RATE_MAX 150
#define NEWFON_RATE_DEFAULT 30

/* Acceleration range. 0 means no acceleration, 7 is the mildest
   and 1 the strongest one, exactly as in Newfon. */
#define NEWFON_ACCEL_OFF 0
#define NEWFON_ACCEL_MAX 7

/* Interclause pause range */
#define NEWFON_PAUSE_MAX 255

/* Relative pitch and inflection. 50 keeps the voice exactly as it is
   defined by the core, other values shift its tone range the same way
   as the original Newfon NVDA driver did. */
#define NEWFON_PITCH_DEFAULT 50
#define NEWFON_INFLECTION_DEFAULT 50


/* BEGIN_C_DECLS should be used at the beginning of C declarations,
   so that C++ compilers don't mangle their names.  Use END_C_DECLS at
   the end of C declarations. */
#undef BEGIN_C_DECLS
#undef END_C_DECLS
#ifdef __cplusplus
# define BEGIN_C_DECLS extern "C" {
# define END_C_DECLS }
#else
# define BEGIN_C_DECLS /* empty */
# define END_C_DECLS /* empty */
#endif

BEGIN_C_DECLS

/* Callback function to utilize generated sound */
typedef int (*newfon_callback)(void *buffer, size_t size, void *user_data);

/* Speech parameters */
typedef struct
{
  int voice; /* One of the NEWFON_VOICE_* values */

  int speech_rate; /* [0..150], greater values imply slower speech */
  int acceleration; /* [0..7], 0 means no acceleration */

  int pitch; /* [0..100], 50 keeps the native tone range of the voice */
  int inflection; /* [0..100], 50 keeps the native tone range of the voice */

  /* Interclause pause duration. Gaps at punctuation marks are those of
     Newfon and are not adjustable: their length is the length of the mark
     multiplied by this value. */
  int pause; /* [0..255] */

  /* Combination of TTS control flags */
  int flags;
} newfon_conf_t;

/*
 * Initialize configuration structure with the default values.
 */
extern NEWFON_EXPORT void newfon_config_init(newfon_conf_t *config);

/*
 * Return the name of the specified voice or NULL when the index
 * is out of range.
 */
extern NEWFON_EXPORT const char *newfon_voice_name(int voice);

/*
 * Perform TTS transformation for specified text.
 *
 * The first argument points to a configuration structure.
 * The second argument points to a zero-terminated string to transfer.
 * This string must contain a Russian text in koi8-r.
 * The next two arguments specify a buffer that will be used
 * by the core for delivering produced wave data
 * chunk by chunk to the consumer specified by the fourth argument.
 * Non-zero return value of the consumer causes immediate speech termination.
 * The last argument points to any additional user data passed to the consumer.
 */
extern NEWFON_EXPORT void newfon_transfer(const newfon_conf_t *config,
                                          const char *text, void *wave_buffer, size_t wave_buffer_size,
                                          newfon_callback wave_consumer, void *user_data);

END_C_DECLS

#endif
