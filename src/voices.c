/* voices.c -- The Newfon voice set
 *
 * The four voices of Newfon are two sound banks combined with four
 * hard wired tone ranges. The values below are taken verbatim from the
 * voice records of the original newfon_nvda.dll (data at 0x419c80):
 *
 *   voice  name      mintone  maxtone  threshold  bank
 *   0      male 1        90      130      105     male
 *   1      female 1     190      330       55     female
 *   2      male 2        60      120      105     male
 *   3      female 2     220      340       55     female
 *
 * The very same tone ranges were duplicated in the Python part of the
 * original add-on (pitchTable). They are kept here, inside the core, so
 * that no configuration parameter can spoil a voice.
 *
 * SPDX-License-Identifier: MIT
 */

#include "voice.h"


const newfon_voice_t newfon_voices[NEWFON_VOICE_COUNT] =
  {
    { &male, "male 1", 90, 130, 105 },
    { &female, "female 1", 190, 330, 55 },
    { &male, "male 2", 60, 120, 105 },
    { &female, "female 2", 220, 340, 55 }
  };
