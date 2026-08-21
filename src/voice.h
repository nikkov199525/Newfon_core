/* voice.h -- Voice data definition
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 * Copyright (C) 2021 Alexander Ivanov <ivalex01@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef NEWFON_VOICE_H
#define NEWFON_VOICE_H

#include <stdint.h>

#include "newfon_core.h"


/* Number of voice samples */
#define VOICE_DIMENSION 201


/* Voice data structure */
typedef struct
{
  unsigned int pitch_factor;
  uint16_t sound_offsets[VOICE_DIMENSION];
  uint16_t sound_lengths[VOICE_DIMENSION];
  int8_t samples[];
} voice_t;

/*
 * Newfon voice definition.
 *
 * A voice is a sound bank plus a tone range and a sound length
 * threshold, exactly as the voice records of newfon_nvda.dll are built.
 * Both Newfon male voices share the male sound bank and both female
 * ones share the female bank; they differ by the tone range only.
 */
typedef struct
{
  const voice_t *bank;
  const char *name;
  uint16_t mintone; /* Hz */
  uint16_t maxtone; /* Hz */
  uint8_t threshold; /* Voice sample length threshold */
} newfon_voice_t;


/* Available sound banks */
extern const voice_t male;
extern const voice_t female;

/* The wired-in Newfon voice set */
extern const newfon_voice_t newfon_voices[NEWFON_VOICE_COUNT];

#endif
