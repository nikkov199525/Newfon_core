/* modulation.h -- Voice modulation control means
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 * Copyright (C) 2021 Alexander Ivanov <ivalex01@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef NEWFON_MODULATION_H
#define NEWFON_MODULATION_H

#include <stdint.h>

/* Lowest and highest tone the core accepts, Hz */
#define NEWFON_TONE_MIN 50
#define NEWFON_TONE_MAX 500

/* Pitch and intonation control data */
typedef struct
{
  uint16_t mintone;
  uint16_t maxtone;
} modulation_t;

/*
 * Setup modulation parameters for the specified voice.
 *
 * The tone range of a voice is defined by the core. The pitch and the
 * inflection, both given on the [0..100] scale used by NVDA, shift and
 * widen that range the same way the original Newfon add-on did, so with
 * their default value of 50 the voice sounds exactly as it is wired in.
 */
extern void modulation_setup(modulation_t *modulation, int voice, int pitch, int inflection);

#endif
