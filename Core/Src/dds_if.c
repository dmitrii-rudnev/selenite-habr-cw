/**
  *******************************************************************************
  *
  * @file    dds_if.c
  * @brief   Digital Direct Synthesis Interface
  * @version v1.0
  * @date    01.11.2022
  * @author  Dmitrii Rudnev
  *
  *******************************************************************************
  * Copyrigh &copy; 2022 Selenite Project. All rights reserved.
  *
  * This software component is licensed under [BSD 3-Clause license]
  * (http://opensource.org/licenses/BSD-3-Clause/), the "License".<br>
  * You may not use this file except in compliance with the License.
  *
  * This software uses some source code from softdds.c
  * https://github.com/df8oe/UHSDR/blob/active-devel/mchf-eclipse/drivers/audio/softdds/
  * Copyright 2014 Krassi Atanassov M0NKA
  *
  *******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "dds_if.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

#define DDS_PTR_SHIFT       (32 - DDS_TBL_BITS)

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

soft_dds_t dbldds [2]; /* Two Tone DDS */
soft_dds_t cw_dds;     /* CW Tone DDS  */

/* This table represents 2*PI, i.e. a full sine wave */
const int16_t DDS_TABLE [DDS_TBL_SIZE] =
{
    0, 201, 402, 603, 804, 1005, 1206,
    1406, 1607, 1808, 2009, 2209, 2410, 2610, 2811, 3011, 3211, 3411, 3611,
    3811, 4011, 4210, 4409, 4608, 4807, 5006, 5205, 5403, 5601, 5799, 5997,
    6195, 6392, 6589, 6786, 6982, 7179, 7375, 7571, 7766, 7961, 8156, 8351,
    8545, 8739, 8932, 9126, 9319, 9511, 9703, 9895, 10087, 10278, 10469, 10659,
    10849, 11038, 11227, 11416, 11604, 11792, 11980, 12166, 12353, 12539, 12724,
    12909, 13094, 13278, 13462, 13645, 13827, 14009, 14191, 14372, 14552, 14732,
    14911, 15090, 15268, 15446, 15623, 15799, 15975, 16150, 16325, 16499, 16672,
    16845, 17017, 17189, 17360, 17530, 17699, 17868, 18036, 18204, 18371, 18537,
    18702, 18867, 19031, 19194, 19357, 19519, 19680, 19840, 20000, 20159, 20317,
    20474, 20631, 20787, 20942, 21096, 21249, 21402, 21554, 21705, 21855, 22004,
    22153, 22301, 22448, 22594, 22739, 22883, 23027, 23169, 23311, 23452, 23592,
    23731, 23869, 24006, 24143, 24278, 24413, 24546, 24679, 24811, 24942, 25072,
    25201, 25329, 25456, 25582, 25707, 25831, 25954, 26077, 26198, 26318, 26437,
    26556, 26673, 26789, 26905, 27019, 27132, 27244, 27355, 27466, 27575, 27683,
    27790, 27896, 28001, 28105, 28208, 28309, 28410, 28510, 28608, 28706, 28802,
    28897, 28992, 29085, 29177, 29268, 29358, 29446, 29534, 29621, 29706, 29790,
    29873, 29955, 30036, 30116, 30195, 30272, 30349, 30424, 30498, 30571, 30643,
    30713, 30783, 30851, 30918, 30984, 31049, 31113, 31175, 31236, 31297, 31356,
    31413, 31470, 31525, 31580, 31633, 31684, 31735, 31785, 31833, 31880, 31926,
    31970, 32014, 32056, 32097, 32137, 32176, 32213, 32249, 32284, 32318, 32350,
    32382, 32412, 32441, 32468, 32495, 32520, 32544, 32567, 32588, 32609, 32628,
    32646, 32662, 32678, 32692, 32705, 32717, 32727, 32736, 32744, 32751, 32757,
    32761, 32764, 32766, 32766, 32766, 32764, 32761, 32757, 32751, 32744, 32736,
    32727, 32717, 32705, 32692, 32678, 32662, 32646, 32628, 32609, 32588, 32567,
    32544, 32520, 32495, 32468, 32441, 32412, 32382, 32350, 32318, 32284, 32249,
    32213, 32176, 32137, 32097, 32056, 32014, 31970, 31926, 31880, 31833, 31785,
    31735, 31684, 31633, 31580, 31525, 31470, 31413, 31356, 31297, 31236, 31175,
    31113, 31049, 30984, 30918, 30851, 30783, 30713, 30643, 30571, 30498, 30424,
    30349, 30272, 30195, 30116, 30036, 29955, 29873, 29790, 29706, 29621, 29534,
    29446, 29358, 29268, 29177, 29085, 28992, 28897, 28802, 28706, 28608, 28510,
    28410, 28309, 28208, 28105, 28001, 27896, 27790, 27683, 27575, 27466, 27355,
    27244, 27132, 27019, 26905, 26789, 26673, 26556, 26437, 26318, 26198, 26077,
    25954, 25831, 25707, 25582, 25456, 25329, 25201, 25072, 24942, 24811, 24679,
    24546, 24413, 24278, 24143, 24006, 23869, 23731, 23592, 23452, 23311, 23169,
    23027, 22883, 22739, 22594, 22448, 22301, 22153, 22004, 21855, 21705, 21554,
    21402, 21249, 21096, 20942, 20787, 20631, 20474, 20317, 20159, 20000, 19840,
    19680, 19519, 19357, 19194, 19031, 18867, 18702, 18537, 18371, 18204, 18036,
    17868, 17699, 17530, 17360, 17189, 17017, 16845, 16672, 16499, 16325, 16150,
    15975, 15799, 15623, 15446, 15268, 15090, 14911, 14732, 14552, 14372, 14191,
    14009, 13827, 13645, 13462, 13278, 13094, 12909, 12724, 12539, 12353, 12166,
    11980, 11792, 11604, 11416, 11227, 11038, 10849, 10659, 10469, 10278, 10087,
    9895, 9703, 9511, 9319, 9126, 8932, 8739, 8545, 8351, 8156, 7961, 7766,
    7571, 7375, 7179, 6982, 6786, 6589, 6392, 6195, 5997, 5799, 5601, 5403,
    5205, 5006, 4807, 4608, 4409, 4210, 4011, 3811, 3611, 3411, 3211, 3011,
    2811, 2610, 2410, 2209, 2009, 1808, 1607, 1406, 1206, 1005, 804, 603, 402,
    201, 0, -201, -402, -603, -804, -1005, -1206, -1406, -1607, -1808, -2009,
    -2209, -2410, -2610, -2811, -3011, -3211, -3411, -3611, -3811, -4011, -4210,
    -4409, -4608, -4807, -5006, -5205, -5403, -5601, -5799, -5997, -6195, -6392,
    -6589, -6786, -6982, -7179, -7375, -7571, -7766, -7961, -8156, -8351, -8545,
    -8739, -8932, -9126, -9319, -9511, -9703, -9895, -10087, -10278, -10469,
    -10659, -10849, -11038, -11227, -11416, -11604, -11792, -11980, -12166,
    -12353, -12539, -12724, -12909, -13094, -13278, -13462, -13645, -13827,
    -14009, -14191, -14372, -14552, -14732, -14911, -15090, -15268, -15446,
    -15623, -15799, -15975, -16150, -16325, -16499, -16672, -16845, -17017,
    -17189, -17360, -17530, -17699, -17868, -18036, -18204, -18371, -18537,
    -18702, -18867, -19031, -19194, -19357, -19519, -19680, -19840, -20000,
    -20159, -20317, -20474, -20631, -20787, -20942, -21096, -21249, -21402,
    -21554, -21705, -21855, -22004, -22153, -22301, -22448, -22594, -22739,
    -22883, -23027, -23169, -23311, -23452, -23592, -23731, -23869, -24006,
    -24143, -24278, -24413, -24546, -24679, -24811, -24942, -25072, -25201,
    -25329, -25456, -25582, -25707, -25831, -25954, -26077, -26198, -26318,
    -26437, -26556, -26673, -26789, -26905, -27019, -27132, -27244, -27355,
    -27466, -27575, -27683, -27790, -27896, -28001, -28105, -28208, -28309,
    -28410, -28510, -28608, -28706, -28802, -28897, -28992, -29085, -29177,
    -29268, -29358, -29446, -29534, -29621, -29706, -29790, -29873, -29955,
    -30036, -30116, -30195, -30272, -30349, -30424, -30498, -30571, -30643,
    -30713, -30783, -30851, -30918, -30984, -31049, -31113, -31175, -31236,
    -31297, -31356, -31413, -31470, -31525, -31580, -31633, -31684, -31735,
    -31785, -31833, -31880, -31926, -31970, -32014, -32056, -32097, -32137,
    -32176, -32213, -32249, -32284, -32318, -32350, -32382, -32412, -32441,
    -32468, -32495, -32520, -32544, -32567, -32588, -32609, -32628, -32646,
    -32662, -32678, -32692, -32705, -32717, -32727, -32736, -32744, -32751,
    -32757, -32761, -32764, -32766, -32766, -32766, -32764, -32761, -32757,
    -32751, -32744, -32736, -32727, -32717, -32705, -32692, -32678, -32662,
    -32646, -32628, -32609, -32588, -32567, -32544, -32520, -32495, -32468,
    -32441, -32412, -32382, -32350, -32318, -32284, -32249, -32213, -32176,
    -32137, -32097, -32056, -32014, -31970, -31926, -31880, -31833, -31785,
    -31735, -31684, -31633, -31580, -31525, -31470, -31413, -31356, -31297,
    -31236, -31175, -31113, -31049, -30984, -30918, -30851, -30783, -30713,
    -30643, -30571, -30498, -30424, -30349, -30272, -30195, -30116, -30036,
    -29955, -29873, -29790, -29706, -29621, -29534, -29446, -29358, -29268,
    -29177, -29085, -28992, -28897, -28802, -28706, -28608, -28510, -28410,
    -28309, -28208, -28105, -28001, -27896, -27790, -27683, -27575, -27466,
    -27355, -27244, -27132, -27019, -26905, -26789, -26673, -26556, -26437,
    -26318, -26198, -26077, -25954, -25831, -25707, -25582, -25456, -25329,
    -25201, -25072, -24942, -24811, -24679, -24546, -24413, -24278, -24143,
    -24006, -23869, -23731, -23592, -23452, -23311, -23169, -23027, -22883,
    -22739, -22594, -22448, -22301, -22153, -22004, -21855, -21705, -21554,
    -21402, -21249, -21096, -20942, -20787, -20631, -20474, -20317, -20159,
    -20000, -19840, -19680, -19519, -19357, -19194, -19031, -18867, -18702,
    -18537, -18371, -18204, -18036, -17868, -17699, -17530, -17360, -17189,
    -17017, -16845, -16672, -16499, -16325, -16150, -15975, -15799, -15623,
    -15446, -15268, -15090, -14911, -14732, -14552, -14372, -14191, -14009,
    -13827, -13645, -13462, -13278, -13094, -12909, -12724, -12539, -12353,
    -12166, -11980, -11792, -11604, -11416, -11227, -11038, -10849, -10659,
    -10469, -10278, -10087, -9895, -9703, -9511, -9319, -9126, -8932, -8739,
    -8545, -8351, -8156, -7961, -7766, -7571, -7375, -7179, -6982, -6786, -6589,
    -6392, -6195, -5997, -5799, -5601, -5403, -5205, -5006, -4807, -4608, -4409,
    -4210, -4011, -3811, -3611, -3411, -3211, -3011, -2811, -2610, -2410, -2209,
    -2009, -1808, -1607, -1406, -1206, -1005, -804, -603, -402, -201
};


/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
 * Execute a single step in the sinus generation
 */

uint32_t softdds_nextSampleIndex (soft_dds_t *dds)
{
  uint32_t retval = dds->ptr >> DDS_PTR_SHIFT;

  if (retval > DDS_TBL_SIZE)
  {
    retval -= DDS_TBL_SIZE;
  }

  dds->ptr += dds->step;

  return retval;
}

/*
 * Get the index which represents a -90 degree shift compared to
 * k, i.e. get  k = sin(a) => cos(a)
 */

uint32_t softdds_phase_shift90 (uint32_t k)
{
  uint32_t retval = k + (3 * DDS_TBL_SIZE / 4);

  if (retval > DDS_TBL_SIZE)
  {
    retval -= DDS_TBL_SIZE;
  }

  return retval;
}

/**
 * Execute a single step in the sinus generation and return actual sample value
 */

int16_t softdds_nextSample (soft_dds_t *dds_ptr)
{
  int16_t retval = DDS_TABLE [softdds_nextSampleIndex (dds_ptr)];

  return retval;
}

/**
 * Initialize softdds for given frequency and sample rate
 */

void softdds_setFreqDDS (soft_dds_t* dds_ptr, uint32_t freq, uint32_t sample_rate, uint8_t smooth)
{
  uint64_t freq64_shifted = freq * DDS_TBL_SIZE;

  freq64_shifted <<= DDS_PTR_SHIFT;

  dds_ptr->step = freq64_shifted / sample_rate;

  /* Reset accumulator, if need smooth tone transition, do not reset it (e.g. wspr) */
  if (!smooth)
  {
    dds_ptr->ptr = 0;
  }
}


/**
 * Initialize softdds for given frequency and sample rate
 */

void softdds_configRunIQ (uint32_t freq [2], uint32_t sample_rate, uint8_t smooth)
{
  softdds_setFreqDDS (&dbldds [0], freq [0], sample_rate, smooth);
  softdds_setFreqDDS (&dbldds [1], freq [1], sample_rate, smooth);
}

void softdds_genIQSingleTone (soft_dds_t* dds, int16_t *i_buff, int16_t *q_buff, uint16_t size)
{
  for (uint16_t i = 0; i < size; i++)
  {
    uint32_t k = softdds_nextSampleIndex (dds);      /* Calculate next sample index */

    *i_buff = DDS_TABLE [k];                         /* Load I value (sin) from DDS table */
    *q_buff = DDS_TABLE [softdds_phase_shift90 (k)]; /* Load Q value (cos) from DDS table */

    i_buff++;
    q_buff++;
  }
}

/*
 * Generates the addition  of two sinus frequencies as IQ data stream
 * min/max value is +/-2^15-1
 * Frequencies need to be configured using softdds_setfreq_dbl
 */

void softdds_genIQTwoTone (soft_dds_t *ddsA, soft_dds_t *ddsB, uint16_t *i_buff, uint16_t *q_buff, uint16_t size)
{
  for (int i = 0; i < size; i++)
  {
    uint32_t k [2];

    k [0] = softdds_nextSampleIndex (ddsA);                    /* Calculate next sample index */
    k [1] = softdds_nextSampleIndex (ddsB);

    *i_buff = (DDS_TABLE [k [0]] +
               DDS_TABLE [k [1]]) / 2;                         /* Load I value 0.5 * (sin(a) + sin(b)) */

    *q_buff = (DDS_TABLE [softdds_phase_shift90 (k [0])] +
               DDS_TABLE [softdds_phase_shift90 (k [1])]) / 2; /* Load Q value 0.5 * (cos(a) + cos(b)) */

    i_buff++;
    q_buff++;
  }
}

/**
 * Overlays an audio stream with a beep signal
 * @param dds The previously initialized dds configuration
 * @param buffer audio buffer of blockSize (mono/single channel) samples
 * @param blockSize
 * @param scaling scale the resulting sine wave with this factor
 */

void softdds_addSingleTone (soft_dds_t *dds_ptr,
                            int16_t    *buffer,
                            uint8_t    scaling)
{
  int32_t buf, dds;

  dds = softdds_nextSample (dds_ptr);
  dds = (dds * scaling) / 256U;

  buf = *buffer;
  buf = (buf * (256 - scaling)) / 256U;

  *buffer = buf + dds;
}

/**
 * Overlays an audio stream with a beep signal
 * @param dds The previously initialized dds configuration
 * @param buffer0 audio buffer of blockSize (mono/single channel) samples
 * @param buffer1 audio buffer of blockSize (mono/single channel) samples
 * @param scaling scale the resulting sine wave with this factor
 */

void softdds_addSingleToneToTwobuffers (soft_dds_t *dds_ptr,
                                        int16_t    *buffer0,
                                        int16_t    *buffer1,
                                        uint8_t    scaling)
{
  int16_t  buffer [2], dds;
  int16_t  retval [2];

  retval [0] = *buffer0;
  retval [1] = *buffer1;

  dds = softdds_nextSample (dds_ptr);
  dds = (dds * scaling) / 256U;

  for (uint8_t k = 0U; k < 2U; k++)
  {
    buffer [k] = (retval [k] * (256 - scaling)) / 256U;
    retval [k] = buffer [k] + dds;
  }

  *buffer0 = retval [0];
  *buffer1 = retval [1];
}

/*
 * Generates the sinus frequencies as IQ data stream
 * min/max value is +/-2^15-1
 * Frequency needs to be configured using softdds_setfreq
 */

void softdds_runIQ (uint16_t *i_buff, uint16_t *q_buff, uint16_t size)
{
  if (dbldds [1].step > 0.0)
  {
    softdds_genIQTwoTone (&dbldds [0], &dbldds [1], i_buff, q_buff, size);
  }
  else
  {
    softdds_genIQSingleTone (&dbldds [0], i_buff, q_buff, size);
  }
}


void DDS_CW_Monitor (int16_t *buff0, int16_t *buff1, uint8_t scaling)
{
  softdds_addSingleToneToTwobuffers (&cw_dds, buff0, buff1, scaling);
}

void DDS_Set_CW_Pitch (uint32_t freq, uint32_t sample_rate)
{
  softdds_setFreqDDS (&cw_dds, freq, sample_rate, 1U);
}

void DDS_Get_Sample (int16_t *buff)
{
  *buff = softdds_nextSample (&cw_dds);
}

void DDS_Get_IQ_Sample (int16_t *i_buff, int16_t *q_buff)
{
  softdds_genIQSingleTone (&cw_dds, i_buff, q_buff, 1U);
}
