/* numerics.c -- Numbers phonetic transcription
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
#include <stdio.h>

#include "phonemes.h"
#include "numerics.h"
#include "transcription.h"
#include "sink.h"
#include "synth.h"
#include "newfon_core.h"


/* Local constants */
#define NUMBER_FRACTION 1
#define NON_ZERO 2


/* Local data */

/* Predefined transcriptions */
static const uint8_t primary[] = /* 0..9 */
  {
    4, PH_N, PH_O, PH_PRIMARY_STRESS, PH_L_,
    5, PH_A, PH_D_, PH_I, PH_PRIMARY_STRESS, PH_N,
    4, PH_D, PH_V, PH_A, PH_PRIMARY_STRESS,
    4, PH_T, PH_R_, PH_I, PH_PRIMARY_STRESS,
    7, PH_CH, PH_E, PH_T, PH_Y, PH_PRIMARY_STRESS, PH_R_, PH_E,
    4, PH_P_, PH_A, PH_PRIMARY_STRESS, PH_T_,
    5, PH_SH, PH_E, PH_PRIMARY_STRESS, PH_S_, PH_T_,
    4, PH_S_, PH_E, PH_PRIMARY_STRESS, PH_M,
    6, PH_V, PH_O, PH_PRIMARY_STRESS, PH_S_, PH_E, PH_M,
    6, PH_D_, PH_E, PH_PRIMARY_STRESS, PH_V_, PH_A, PH_T_
  };
static const uint8_t secondary[] = /* 10..19 */
  {
    6, PH_D_, PH_E, PH_PRIMARY_STRESS, PH_S_, PH_A, PH_T_,
    9, PH_A, PH_D_, PH_I, PH_PRIMARY_STRESS, PH_N, PH_A, PH_C, PH_A, PH_T_,
    9, PH_D, PH_V_, PH_E, PH_N, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    9, PH_T, PH_R_, PH_I, PH_N, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    11, PH_CH, PH_E, PH_T, PH_Y, PH_PRIMARY_STRESS, PH_R, PH_N, PH_A, PH_C, PH_A, PH_T_,
    9, PH_P_, PH_A, PH_T, PH_N, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    9, PH_SH, PH_E, PH_S, PH_N, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    9, PH_S_, PH_E, PH_M, PH_N, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    11, PH_V, PH_A, PH_S_, PH_E, PH_M, PH_N, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    11, PH_D_, PH_E, PH_V_, PH_A, PH_T, PH_N, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_
  };
static const uint8_t tens[] = /* 20..90 */
  {
    7, PH_D, PH_V, PH_A, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    7, PH_T, PH_R_ , PH_I, PH_PRIMARY_STRESS, PH_C, PH_A, PH_T_,
    6, PH_S, PH_O, PH_PRIMARY_STRESS, PH_R, PH_A, PH_K,
    8, PH_P_, PH_A, PH_D_, PH_E, PH_S_, PH_A, PH_PRIMARY_STRESS, PH_T,
    9, PH_SH, PH_E, PH_Z_, PH_D_, PH_E, PH_S_, PH_A, PH_PRIMARY_STRESS, PH_T,
    9, PH_S_, PH_E, PH_PRIMARY_STRESS, PH_M, PH_D_, PH_E, PH_S_, PH_A, PH_T,
    11, PH_V, PH_O, PH_PRIMARY_STRESS, PH_S_, PH_E, PH_M, PH_D_, PH_E, PH_S_, PH_A, PH_T,
    10, PH_D_, PH_E, PH_V_, PH_A, PH_N, PH_O, PH_PRIMARY_STRESS, PH_S, PH_T, PH_A
  };
static const uint8_t hundreds[] = /* 100..900 */
  {
    4, PH_S, PH_T, PH_O, PH_PRIMARY_STRESS,
    7, PH_D, PH_V_, PH_E, PH_PRIMARY_STRESS, PH_S_, PH_T_, PH_I,
    7, PH_T, PH_R_, PH_I, PH_PRIMARY_STRESS, PH_S, PH_T, PH_A,
    10, PH_CH, PH_E, PH_T, PH_Y, PH_PRIMARY_STRESS, PH_R_, PH_E, PH_S, PH_T, PH_A,
    7, PH_P_, PH_A, PH_T, PH_S, PH_O, PH_PRIMARY_STRESS, PH_T,
    7, PH_SH, PH_E, PH_S, PH_S, PH_O, PH_PRIMARY_STRESS, PH_T,
    7, PH_S_, PH_E, PH_M, PH_S, PH_O, PH_PRIMARY_STRESS, PH_T,
    9, PH_V, PH_A, PH_S_, PH_E, PH_M, PH_S, PH_O, PH_PRIMARY_STRESS, PH_T,
    9, PH_D_, PH_E, PH_V_, PH_A, PH_T, PH_S, PH_O, PH_PRIMARY_STRESS, PH_T
  };
/* Short-scale names: 10^3 through 10^150 (up to 153 digits). */
static const uint8_t periods[][32] =
  {
    { 6, PH_T, PH_Y, PH_PRIMARY_STRESS, PH_S_, PH_A, PH_CH }, /* тысяч */
    { 7, PH_M_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* миллион */
    { 8, PH_M_, PH_I, PH_L_, PH_I, PH_A, PH_PRIMARY_STRESS, PH_R, PH_T }, /* миллиард */
    { 8, PH_T, PH_R_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* триллион */
    { 11, PH_K, PH_V, PH_A, PH_D, PH_R_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* квадриллион */
    { 11, PH_K, PH_V_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* квинтиллион */
    { 11, PH_S_, PH_E, PH_K, PH_S_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* секстиллион */
    { 10, PH_S_, PH_E, PH_P, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* септиллион */
    { 9, PH_A, PH_K, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* октиллион */
    { 9, PH_N, PH_A, PH_N_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* нониллион */
    { 9, PH_D_, PH_E, PH_C, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* дециллион */
    { 11, PH_U, PH_N_, PH_D_, PH_E, PH_C, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* ундециллион */
    { 12, PH_D, PH_U, PH_A, PH_D_, PH_E, PH_C, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* дуодециллион */
    { 12, PH_T, PH_R_, PH_E, PH_D_, PH_E, PH_C, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* тредециллион */
    { 17, PH_K, PH_V, PH_A, PH_T, PH_T, PH_U, PH_A, PH_R, PH_D_, PH_E, PH_C, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* кваттуордециллион */
    { 13, PH_K, PH_V_, PH_I, PH_N_, PH_D_, PH_E, PH_C, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* квиндециллион */
    { 13, PH_S_, PH_E, PH_K, PH_S_, PH_D_, PH_E, PH_C, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* сексдециллион */
    { 15, PH_S_, PH_E, PH_P, PH_T_, PH_E, PH_N_, PH_D_, PH_E, PH_C, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* септендециллион */
    { 13, PH_A, PH_K, PH_T, PH_A, PH_D_, PH_E, PH_C, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* октодециллион */
    { 14, PH_N, PH_A, PH_V_, PH_E, PH_M, PH_D_, PH_E, PH_C, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* новемдециллион */
    { 12, PH_V_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* вигинтиллион */
    { 14, PH_U, PH_N, PH_V_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* унвигинтиллион */
    { 15, PH_D, PH_U, PH_A, PH_V_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* дуовигинтиллион */
    { 15, PH_T, PH_R_, PH_E, PH_V_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* тревигинтиллион */
    { 20, PH_K, PH_V, PH_A, PH_T, PH_T, PH_U, PH_A, PH_R, PH_V_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* кваттуорвигинтиллион */
    { 16, PH_K, PH_V_, PH_I, PH_N, PH_V_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* квинвигинтиллион */
    { 16, PH_S_, PH_E, PH_K, PH_S, PH_V_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* сексвигинтиллион */
    { 18, PH_S_, PH_E, PH_P, PH_T_, PH_E, PH_N, PH_V_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* септенвигинтиллион */
    { 16, PH_A, PH_K, PH_T, PH_A, PH_V_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* октовигинтиллион */
    { 17, PH_N, PH_A, PH_V_, PH_E, PH_M, PH_V_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* новемвигинтиллион */
    { 13, PH_T, PH_R_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* тригинтиллион */
    { 15, PH_U, PH_N, PH_T, PH_R_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* унтригинтиллион */
    { 16, PH_D, PH_U, PH_A, PH_T, PH_R_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* дуотригинтиллион */
    { 16, PH_T, PH_R_, PH_E, PH_T, PH_R_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* третригинтиллион */
    { 21, PH_K, PH_V, PH_A, PH_T, PH_T, PH_U, PH_A, PH_R, PH_T, PH_R_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* кваттуортригинтиллион */
    { 17, PH_K, PH_V_, PH_I, PH_N, PH_T, PH_R_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* квинтригинтиллион */
    { 17, PH_S_, PH_E, PH_K, PH_S, PH_T, PH_R_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* секстригинтиллион */
    { 19, PH_S_, PH_E, PH_P, PH_T_, PH_E, PH_N, PH_T, PH_R_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* септентригинтиллион */
    { 17, PH_A, PH_K, PH_T, PH_A, PH_T, PH_R_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* октотригинтиллион */
    { 18, PH_N, PH_A, PH_V_, PH_E, PH_M, PH_T, PH_R_, PH_I, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* новемтригинтиллион */
    { 16, PH_K, PH_V, PH_A, PH_D, PH_R, PH_A, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* квадрагинтиллион */
    { 18, PH_U, PH_N, PH_K, PH_V, PH_A, PH_D, PH_R, PH_A, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* унквадрагинтиллион */
    { 19, PH_D, PH_U, PH_A, PH_K, PH_V, PH_A, PH_D, PH_R, PH_A, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* дуоквадрагинтиллион */
    { 19, PH_T, PH_R_, PH_E, PH_K, PH_V, PH_A, PH_D, PH_R, PH_A, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* треквадрагинтиллион */
    { 24, PH_K, PH_V, PH_A, PH_T, PH_T, PH_U, PH_A, PH_R, PH_K, PH_V, PH_A, PH_D, PH_R, PH_A, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* кваттуорквадрагинтиллион */
    { 20, PH_K, PH_V_, PH_I, PH_N, PH_K, PH_V, PH_A, PH_D, PH_R, PH_A, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* квинквадрагинтиллион */
    { 20, PH_S_, PH_E, PH_K, PH_S, PH_K, PH_V, PH_A, PH_D, PH_R, PH_A, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* сексквадрагинтиллион */
    { 22, PH_S_, PH_E, PH_P, PH_T_, PH_E, PH_N, PH_K, PH_V, PH_A, PH_D, PH_R, PH_A, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* септенквадрагинтиллион */
    { 20, PH_A, PH_K, PH_T, PH_A, PH_K, PH_V, PH_A, PH_D, PH_R, PH_A, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* октоквадрагинтиллион */
    { 21, PH_N, PH_A, PH_V_, PH_E, PH_M, PH_K, PH_V, PH_A, PH_D, PH_R, PH_A, PH_G_, PH_I, PH_N_, PH_T_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N }, /* новемквадрагинтиллион */
  };
#define NUMBER_PERIODS (sizeof(periods) / sizeof(periods[0]))
static const uint8_t fractions[] =
  {
    6, PH_D_, PH_E, PH_S_, PH_A, PH_PRIMARY_STRESS, PH_T,
    4, PH_S, PH_O, PH_PRIMARY_STRESS, PH_T,
    7, PH_T, PH_Y, PH_PRIMARY_STRESS, PH_S_, PH_A, PH_CH, PH_N,
    13, PH_D_, PH_E, PH_S_, PH_A, PH_T_, PH_I, PH_T, PH_Y, PH_PRIMARY_STRESS, PH_S_, PH_A, PH_CH, PH_N,
    10, PH_S, PH_T, PH_O, PH_T, PH_Y, PH_PRIMARY_STRESS, PH_S_, PH_A, PH_CH, PH_N,
    8, PH_M_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N, PH_N,
    14, PH_D_, PH_E, PH_S_, PH_A, PH_T_, PH_I, PH_M_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N, PH_N,
    11, PH_S, PH_T, PH_O, PH_M_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N, PH_N,
    9, PH_M_, PH_I, PH_L_, PH_I, PH_A, PH_PRIMARY_STRESS, PH_R, PH_T, PH_N,
    15, PH_D_, PH_E, PH_S_, PH_A, PH_T_, PH_I, PH_M_, PH_I, PH_L_, PH_I, PH_A, PH_PRIMARY_STRESS, PH_R, PH_T, PH_N,
    12, PH_S, PH_T, PH_O, PH_M_, PH_I, PH_L_, PH_I, PH_A, PH_PRIMARY_STRESS, PH_R, PH_T, PH_N,
    9, PH_T, PH_R_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N, PH_N,
    15, PH_D_, PH_E, PH_S_, PH_A, PH_T_, PH_I, PH_T, PH_R_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N, PH_N,
    12, PH_S, PH_T, PH_O, PH_T, PH_R_, PH_I, PH_L_, PH_I, PH_O, PH_PRIMARY_STRESS, PH_N, PH_N
  };
static const uint8_t suffixes[] =
  {
    2, PH_Y, PH_X,
    3, PH_A, PH_J, PH_A,
    2, PH_A, PH_F
  };
static const uint8_t one_int[] =
  {
    PH_A, PH_D, PH_N, PH_A, PH_PRIMARY_STRESS, PH_SPACE,
    PH_C, PH_E, PH_PRIMARY_STRESS, PH_L, PH_A, PH_J, PH_A, PH_SPACE
  };
static const uint8_t one_o[] = { PH_A, PH_D, PH_N, PH_O, PH_PRIMARY_STRESS, PH_SPACE };
static const uint8_t two_e[] = { PH_D, PH_V_, PH_E, PH_PRIMARY_STRESS, PH_SPACE };
static const uint8_t n_ints[] = { PH_C, PH_E, PH_PRIMARY_STRESS, PH_L, PH_Y, PH_X };


/* Local subroutines */

/* Pass specified list item to the consumer */
static void put_transcription(sink_t *consumer, const uint8_t *lst, uint8_t n)
{
  const uint8_t *item = list_item(lst, n);
  sink_write(consumer, item + 1, item[0]);
}

/* Output transcription for specified digit */
static void transcribe_digit(sink_t *consumer, char digit, char following)
{
  put_transcription(consumer, primary, digit - '0');
  if (following != ' ')
    sink_put(consumer, PH_SPACE);
}

/* Return true if specified character may be treated as decimal point. */
static int check_dec_point(sink_t *consumer, char c)
{
  int flags = ((ttscb_t *) (consumer->user_data))->flags;
  return ((flags & DEC_SEP_POINT) && (c == '.')) ||
    ((flags & DEC_SEP_COMMA) && (c == ','));
}

/* Compact leading zeroes only. Zeroes inside a cardinal or a decimal
   fraction carry place value and must not be counted as a separate run. */
static void transcribe_zeroes(sink_t *consumer, size_t count)
{
  char text[3 * sizeof(size_t) + 1];
  input_t input;
  /* "ноля", "нолей": the unstressed [о] is reduced to [а] as in "одна". */
  static const uint8_t few[] = { PH_N, PH_A, PH_L_, PH_A, PH_PRIMARY_STRESS };
  static const uint8_t many[] = { PH_N, PH_A, PH_L_, PH_E, PH_PRIMARY_STRESS, PH_J };
  size_t last_two = count % 100;
  int length;

  length = snprintf(text, sizeof(text), "%zu", count);
  input.text = input.start = text;
  input.end = text + length;
  process_number(&input, consumer);
  if (consumer->status)
    return;

  if ((last_two >= 11) && (last_two <= 14))
    sink_write(consumer, many, sizeof(many));
  else if (count % 10 == 1)
    put_transcription(consumer, primary, 0);
  else if ((count % 10 >= 2) && (count % 10 <= 4))
    sink_write(consumer, few, sizeof(few));
  else
    sink_write(consumer, many, sizeof(many));
  sink_put(consumer, PH_SPACE);
  sink_flush(consumer);
}


/* Global functions */

/* Transcribe numeric string from input and pass result to the consumer */
void process_number(input_t *input, sink_t *consumer)
{
  unsigned char c;
  uint8_t flags = 0;

  if ((input->start < input->end) && (input->start[0] == '0'))
    {
      char *zero_end = input->start;
      char *number_end;
      int zero_count_min = (((ttscb_t *)consumer->user_data)->flags & ZERO_COUNT_MIN_MASK) >> ZERO_COUNT_MIN_SHIFT;
      if ((zero_count_min < 2) || (zero_count_min > NEWFON_ZERO_COUNT_MIN_MAX))
        zero_count_min = NEWFON_ZERO_COUNT_MIN_DEFAULT;
      while ((zero_end < input->end) && (zero_end[0] == '0'))
        zero_end++;
      for (number_end = zero_end; (number_end < input->end) && IS_DIGIT(number_end[0]); number_end++);
      if (((zero_end - input->start) > 1) &&
          !(((input->end - number_end) > 1) && check_dec_point(consumer, number_end[0]) && IS_DIGIT(number_end[1])))
        {
          if ((zero_end - input->start) >= zero_count_min)
            transcribe_zeroes(consumer, zero_end - input->start);
          else
            while ((input->start < zero_end) && !consumer->status)
              {
                transcribe_digit(consumer, *input->start++, '0');
                sink_flush(consumer);
              }
          input->start = zero_end;
          if (consumer->status || (input->start == input->end) || !IS_DIGIT(input->start[0]))
            return;
        }
    }

  /* Above the named range, preserve every digit rather than interpreting
     successive chunks as unrelated large cardinals. */
  {
    char *number_end = input->start;
    while ((number_end < input->end) && IS_DIGIT(number_end[0]))
      number_end++;
    if ((size_t)(number_end - input->start) > 3 * (NUMBER_PERIODS + 1))
      {
        if (sink_last(consumer) != PH_SPACE)
          sink_put(consumer, PH_SPACE);
        while ((input->start < number_end) && !consumer->status)
          {
            transcribe_digit(consumer, *input->start++, '0');
            sink_flush(consumer);
          }
        return;
      }
  }

  for (c = input->start[0]; (input->start < input->end) && IS_DIGIT(c); c = *(++input->start))
    {
      char *s;
      uint8_t digits = 1;
      uint8_t triplets = 0;
      uint8_t lzn = 0;
      uint8_t nc = 0;
      uint8_t n;

      flags &= ~NON_ZERO;
      if (sink_last(consumer) != PH_SPACE)
        sink_put(consumer, PH_SPACE);
      for (s = input->start + 1; s < input->end; s++)
        if (IS_DIGIT(s[0]))
          {
            if (++digits > 3)
              {
                digits = 1;
                if (++triplets > NUMBER_PERIODS)
                  {
                    digits = 3;
                    triplets = NUMBER_PERIODS;
                    break;
                  }
              }
            else if ((flags &NUMBER_FRACTION) && (triplets > 3) && (digits > 1))
              break;
          }
        else break;
      n = triplets * 3 + digits;

      s = input->start;
      for (input->start += n; !consumer->status; s++)
        {
          c = s[0];
          nc = 0;
          if (c != '0')
            flags |= NON_ZERO;
          else if (IS_DIGIT(s[1]))
            lzn++;
          if ((c != '0') || !((flags & NON_ZERO) || IS_DIGIT(s[1])))
            {
              lzn = 0;
              switch (digits)
                {
                case 3:
                  put_transcription(consumer, hundreds, c - '1');
                  sink_put(consumer, PH_SPACE);
                  break;
                case 1:
                  if (c == '1')
                    {
                      nc = 1;
                      switch (triplets)
                        {
                        case 1:
                          sink_write(consumer, one_int, 6);
                          break;
                        case 0:
                          /* s[1] goes first: past the end of the text s[2] is out of the string */
                          if (s[1] && (s[2] == '+'))
                            {
                              if (s[1] == 'A')
                                {
                                  input->start += 2;
                                  s = input->start;
                                  sink_write(consumer, one_int, (s[0] != ' ') ? 6 : 5);
                                  break;
                                }
                              else if (s[1] == 'O')
                                {
                                  input->start += 2;
                                  s = input->start;
                                  sink_write(consumer, one_o, (s[0] != ' ') ? 6 : 5);
                                  break;
                                }
                            }
                        default:
                          if (flags & NUMBER_FRACTION)
                            {
                              if (((s + 1) < input->end) && IS_DIGIT(s[2]))
                                transcribe_digit(consumer, c, s[0]);
                              else sink_write(consumer, one_int, 6);
                            }
                          else if ((s >= input->end) || !check_dec_point(consumer, s[1]) || !IS_DIGIT(s[2]))
                            transcribe_digit(consumer, c, s[0]);
                          else sink_write(consumer, one_int, 14);
                          break;
                        }
                      break;
                    }
                  else if (c < '5')
                    {
                      nc = 2;
                      if (c == '2')
                        {
                          if ((triplets == 0) && (s[1] == 'E') && (s[2] == '+'))
                            {
                              input->start += 2;
                              s = input->start;
                              sink_write(consumer, two_e, (s[0] != ' ') ? 5 : 4);
                              break;
                            }
                          else if ((triplets == 1) ||
                                   ((flags & NUMBER_FRACTION) && (s == (input->start - 1))) ||
                                   (check_dec_point(consumer, s[1]) && IS_DIGIT(s[2])))
                            {
                              sink_write(consumer, two_e, 5);
                              break;
                            }
                        }
                    }
                  transcribe_digit(consumer, c, s[0]);
                  break;
                default:
                  if (c == '1')
                    {
                      put_transcription(consumer, secondary, *(++s) - '0');
                      nc = 0;
                      digits--;
                    }
                  else put_transcription(consumer, tens, c - '2');
                  sink_put(consumer, PH_SPACE);
                  break;
                }
            }
          else if (!flags)
            transcribe_digit(consumer, c, s[0]);
          if (!(--digits))
            {
              if (lzn == 3)
                {
                  lzn = 0;
                  if (triplets)
                    {
                      digits = 3;
                      triplets--;
                    }
                  else
                    {
                      sink_back(consumer);
                      break;
                    }
                }
              else
                {
                  lzn = 0;
                  if (triplets)
                    {
                      if (flags & NON_ZERO)
                        {
                          sink_write(consumer, periods[triplets - 1] + 1, periods[triplets - 1][0]);
                          if (triplets != 1)
                            {
                              if (nc != 1)
                                {
                                  if (sink_last(consumer) == PH_T)
                                    sink_replace(consumer, PH_D);
                                  if (nc > 1)
                                    sink_put(consumer, PH_A);
                                  else put_transcription(consumer, suffixes, 2);
                                }
                            }
                          else if (nc > 0)
                            sink_put(consumer, (nc > 1) ? PH_I : PH_A);
                          sink_flush(consumer);
                        }
                      digits = 3;
                      triplets--;
                    }
                  else
                    {
                      sink_back(consumer);
                      break;
                    }
                }
            }
        }

      if (consumer->status)
        break;
      else if (flags & NUMBER_FRACTION)
        {
          sink_put(consumer, PH_SPACE);
          put_transcription(consumer, fractions, n - 1);
          put_transcription(consumer, suffixes, (nc != 1) ? 0 : 1);
          break;
        }
      else if (((input->start + 1) < input->end) && check_dec_point(consumer, input->start[0]) && IS_DIGIT(input->start[1]))
        {
          flags |= NUMBER_FRACTION;
          sink_put(consumer, PH_SPACE);
          if (nc != 1)
            {
              sink_write(consumer, n_ints, 6);
              sink_flush(consumer);
            }
        }
      else
        {
          sink_put(consumer, PH_SPACE);
          break;
        }
    }
}
