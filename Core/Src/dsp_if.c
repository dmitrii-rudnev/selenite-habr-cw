/**
  *******************************************************************************
  *
  * @file    dsp_if.c
  * @brief   Digital Signal Processor Interface
  * @version v2.2
  * @date    03.11.2022
  * @author  Dmitrii Rudnev
  *
  *******************************************************************************
  * Copyrigh &copy; 2022 Selenite Project. All rights reserved.
  *
  * This software component is licensed under [BSD 3-Clause license]
  * (http://opensource.org/licenses/BSD-3-Clause/), the "License".<br>
  * You may not use this file except in compliance with the License.
  *******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "dsp_if.h"
#include "cw_gen.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

DSP_Buff_TypeDef dsp_out_buff;

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

extern CW_Keyer cw_keyer;

/* Private functions ---------------------------------------------------------*/

/**
 * @brief This function writes a sample to DSP Out buffer
 *
 * @param i, q are samples to write
 *
 */

void dsp_out_buff_write (uint16_t i, uint16_t q)
{
  dsp_out_buff.i [dsp_out_buff.wr_ptr] = i;
  dsp_out_buff.q [dsp_out_buff.wr_ptr] = q;

  dsp_out_buff.wr_ptr++;

  if (dsp_out_buff.wr_ptr == DSP_BUFF_SIZE)
  {
    dsp_out_buff.wr_ptr = 0U;
  }
}

/**
 * @brief This function writes to DSP Out buffer
 *
 * This function writes to DSP Out buffer and prevent read/write areas overlay
 *
 * @param Source buffer pointer
 * @param Number of bytes to write
 *
 */

void DSP_Out_Buff_Write (uint8_t *pbuf, uint32_t size)
{
  uint16_t *buff = (uint16_t*) pbuf;

  size = size / 2U;

  for (uint32_t i = 0; i < size; i += 2U)
  {
    dsp_out_buff_write (buff [i + 0], buff [i + 1]);
  }
}

/**
 * @brief This function flush DSP Out buffer
 *
 * This function writes to DSP Out buffer zeros
 */

void DSP_Out_Buff_Mute (void)
{
  for (uint32_t i = 0U; i < DSP_BUFF_SIZE; i++)
  {
    dsp_out_buff.i [i] = 0U;
    dsp_out_buff.q [i] = 0U;
  }
}

/**
 * @brief This function writes to USBD In buffer
 *
 * @param USBD In buffer pointer
 * @param Number of bytes to write
 */

void DSP_In_Buff_Read (uint8_t *pbuf, uint32_t size)
{
  uint16_t *buff = (uint16_t*) pbuf;

  size = size / 2U;

  if (dsp_out_buff.buff_enable == 0U)
  {
    dsp_out_buff.rd_ptr = dsp_out_buff.wr_ptr + DSP_BUFF_HALF_SIZE;

    if (dsp_out_buff.rd_ptr >= DSP_BUFF_SIZE)
    {
      dsp_out_buff.rd_ptr = 0U;
    }

    dsp_out_buff.buff_enable = 1U;
  }

  for (uint32_t i = 0U; i < size; i += 2U)
  {
    buff [i + 0] = dsp_out_buff.i [dsp_out_buff.rd_ptr];
    buff [i + 1] = dsp_out_buff.q [dsp_out_buff.rd_ptr];

    dsp_out_buff.rd_ptr++;

    if (dsp_out_buff.rd_ptr >= DSP_BUFF_SIZE)
    {
      dsp_out_buff.rd_ptr = 0U;
    }
  }

  CW_Handler (pbuf, pbuf, size);
}


/**
 * @brief This function sets DSP to RX mode
 *
 */

void DSP_Set_RX (void)
{

}

/**
 * @brief This function sets DSP to TX mode
 *
 */

void DSP_Set_TX (void)
{

}

/**
 * @brief This function sets DSP Mode
 *
 */

void DSP_Set_Mode (uint8_t mode)
{

}

/**
 * @brief This function initialize DSP and CW generator
 *
 */

void DSP_Init (void)
{
  cw_keyer.pitch = 7U;
  cw_keyer.speed = 14U;

  cw_keyer.mode  = IAMBIC_B;

  CW_Set_Keyer ();
  CW_Set_Pitch (cw_keyer.pitch * 100U, USBD_AUDIO_FREQ);

  DSP_Out_Buff_Mute ();
  dsp_out_buff.wr_ptr = 0U;
}

/****END OF FILE****/
