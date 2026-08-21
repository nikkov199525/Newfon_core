/* timing.h -- Time planner and speech rate control interface
 *
 * Copyright (C) 1990, 1991 Speech Research Laboratory, Minsk
 * Copyright (C) 2005 Igor Poretsky <poretsky@mlbox.ru>
 * Copyright (C) 2021 Boris Lobanov <lobbormef@gmail.com>
 * Copyright (C) 2021 Alexander Ivanov <ivalex01@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef NEWFON_TIMING_H
#define NEWFON_TIMING_H

#include <stdint.h>


/* Number of punctuations treated as clause separators */
#define CLAUSE_SEPARATORS 7

/*
 * Acceleration is applied as an integer duration multiplier.
 * The neutral value, that is Newfon speaking with the acceleration
 * switched off, is 10. The acceleration levels 1..7 of the user
 * interface are used as the multiplier as is, so the level n speeds
 * the speech up by the factor of 10/n.
 */
#define NEWFON_ACCEL_NEUTRAL 10


typedef struct
{
  uint16_t rate; /* Duration factor, [0..150], greater is slower */
  uint8_t accel; /* Duration multiplier, [1..15], 10 is no acceleration */
  uint8_t pause; /* Interclause pause factor, [0..255] */
  uint8_t gaplen[CLAUSE_SEPARATORS];
} timing_t;


/*
 * Initial timing setup for the specified speech rate, acceleration
 * level and interclause pause.
 */
extern void timing_setup(timing_t *timing, int speech_rate, int acceleration, int pause);

#endif
