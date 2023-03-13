/**
  *******************************************************************************
  *
  * @file    ptt_if.c
  * @brief   PTT driver
  * @version v2.1
  * @date    10.11.2022
  * @author  Dmitrii Rudnev
  *
  *******************************************************************************
  * Copyrigh &copy; 2020 Selenite Project. All rights reserved.
  *
  * This software component is licensed under [BSD 3-Clause license]
  * (http://opensource.org/licenses/BSD-3-Clause/), the "License".<br>
  * You may not use this file except in compliance with the License.
  *******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dsp_if.h"
#include "ptt_if.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

PTT_TypeDef ptt;

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

extern TRX_TypeDef trx;

/* Private functions ---------------------------------------------------------*/


/**
  * @brief This function sets TX mode
  *
  * The function also sets TX mode for VFO and DSP
  *
  */

void ptt_set_tx (void)
{
  ptt.key_off_time = 0U;

  if (!trx.is_tx)
  {
    trx.is_tx = 1U;
    HAL_GPIO_WritePin (TX_GPIO_Port, TX_Pin, GPIO_PIN_RESET);
  }
}

/**
  * @brief This function sets RX mode
  *
  * The function sets RX mode if the PTT button is not pressed,
  * the telegraph key is not used and CAT status is not TX.
  * The function also sets RX mode for VFO and DSP
  *
  */

void ptt_set_rx (void)
{
  if (ptt.key_dah_is_on || ptt.key_dit_is_on
      || ptt.dtr_is_on || ptt.rts_is_on) return;

  if (trx.is_tx)
  {
    trx.is_tx = 0U;
    HAL_GPIO_WritePin (TX_GPIO_Port, TX_Pin, GPIO_PIN_SET);
  }
}

/**
  * @brief This function sets TX mode from DTR line
  *
  * DTR line works like external telegraph key
  *
  */

void PTT_DTR_TX (uint8_t dtr)
{
  if (ptt.dtr_is_on != dtr)
  {
    ptt.dtr_is_on = dtr;

    if (dtr)
    {
      ptt_set_tx ();
    }
  }
}

/**
  * @brief This function sets TX mode from RTS line
  *
  * RTS line works like external PTT
  *
  */

void PTT_RTS_TX (uint8_t rts)
{
  if (ptt.rts_is_on != rts)
  {
    ptt.rts_is_on = rts;

    if (rts)
    {
      ptt_set_tx ();
    }
    else
    {
      ptt_set_rx ();
    }
  }
}

/**
  * @brief This function sets telegraph key off time
  *
  */

void PTT_Key_Off_Time (void)
{
  ptt.key_off_time = trx.sysclock;
}

/**
  * @brief This function initialize PTT, VFO and DSP
  *
  */

void PTT_Init (void)
{
  trx.is_tx = 0U;
  HAL_GPIO_WritePin (TX_GPIO_Port, TX_Pin, GPIO_PIN_SET);

  DSP_Init ();
}

/**
  * @brief This function processes the timeout in switching to RX mode
  * after releasing the telegraph key
  *
  */

void PTT_Handler (void)
{
  if (ptt.key_off_time != 0U)
  {
    if ((trx.sysclock - ptt.key_off_time) > KEY_TIMEOUT)
    {
      ptt.key_off_time = 0U;
      ptt_set_rx ();
    }
  }
}

/**
  * @brief This function is GPIO_EXTI handler
  *
  * @param KEY_DAH_Pin - Telegraph key DAH paddle
  * @param KEY_DIT_Pin - Telegraph key DIT paddle
  *
  */

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin)
{
  switch (GPIO_Pin)
  {
    case KEY_DAH_Pin:
      ptt.key_dah_is_on = !HAL_GPIO_ReadPin (KEY_DAH_GPIO_Port, KEY_DAH_Pin);

      if (ptt.key_dah_is_on)
      {
        ptt_set_tx ();
      }

      break;
    case KEY_DIT_Pin:
      ptt.key_dit_is_on = !HAL_GPIO_ReadPin (KEY_DIT_GPIO_Port, KEY_DIT_Pin);

      if (ptt.key_dit_is_on)
      {
        ptt_set_tx ();
      }

      break;
  }
}

/****END OF FILE****/
