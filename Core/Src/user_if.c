/**
  *******************************************************************************
  *
  * @file    user_if.c
  * @brief   User Interface
  * @version v1.0
  * @date    27.11.2022
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

#include "user_if.h"
#include "ssd1306.h"
#include "ptt_if.h"
#include "cw_gen.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

uint32_t displayed   = 0U;
uint16_t key_pressed = 0U;
uint16_t key_value   = 0U;

uint8_t i;
uint8_t item_color;
uint8_t focus = 0U;

char str[10];

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim3;

extern PTT_TypeDef ptt;
extern TRX_TypeDef trx;
extern CW_Keyer cw_keyer;

/* Private functions ---------------------------------------------------------*/

void ui_keyer_mode_to_string (uint8_t keyer_mode)
{
  switch (keyer_mode)
  {
    case 0:
      sprintf (str, " IAMBIC_B");  //0x00
      break;
    case 1:
      sprintf (str, " IAMBIC_A");  //0x01
      break;
    case 2:
      sprintf (str, " ULTIMATE");  //0x02
      break;
    case 3:
      sprintf (str, " STRAIGHT");  //0x03
      break;
  }
}

void ui_set_keyer_mode (void)
{
  if (trx.is_tx) return;

  uint8_t keyer_mode;

  keyer_mode = TIM3->CNT >> 2U;
  ui_keyer_mode_to_string (keyer_mode);

  switch (key_pressed)
  {
    case 0:
      break;
    case 2:
    case 3:
      cw_keyer.mode = keyer_mode;
    case 1:
      ui_keyer_mode_to_string (cw_keyer.mode);
      focus = 1U;
      TIM3->ARR = 15U;
      TIM3->CNT = focus << 2U;
      CW_Set_Keyer ();
      break;
  }
}

void ui_set_keyer_speed (void)
{
  if (trx.is_tx) return;

  uint8_t keyer_speed;

  keyer_speed = (TIM3->CNT >> 2U) + 4U;
  sprintf (str, " %d WPM ", keyer_speed);

  switch (key_pressed)
  {
    case 0:
      break;
    case 2:
    case 3:
      cw_keyer.speed = keyer_speed;
    case 1:
      sprintf (str, " %d WPM ", cw_keyer.speed);
      focus = 2U;
      TIM3->ARR = 15U;
      TIM3->CNT = focus << 2U;
      CW_Set_Keyer ();
      break;
  }
}

void ui_set_keyer_pitch (void)
{
  if (trx.is_tx) return;

  uint8_t keyer_pitch;

  keyer_pitch = (TIM3->CNT >> 2U) + 3U;
  sprintf (str, " %d Hz ", keyer_pitch * 100U);

  switch (key_pressed)
  {
    case 0:
      break;
    case 2:
    case 3:
      cw_keyer.pitch = keyer_pitch;
    case 1:
      sprintf (str, " %d Hz ", cw_keyer.pitch * 100U);
      focus = 3U;
      TIM3->ARR = 15U;
      TIM3->CNT = focus << 2U;
      CW_Set_Pitch (cw_keyer.pitch * 100U, 48000U);
      break;
  }
}

/* Public functions ----------------------------------------------------------*/

/**
  * @brief This function returns result of ADC
  *
  */

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc1)
{
  key_value = HAL_ADC_GetValue(hadc1);
}

/**
  * @brief This function initiates CW keyer UI
  *
  */

void UI_Init (void)
{
  displayed = trx.sysclock;

  HAL_ADC_Start_IT (&hadc1);
  HAL_TIM_Encoder_Start (&htim3, TIM_CHANNEL_ALL);
  TIM3->ARR = 15U;

  ssd1306_Init ();
  ssd1306_Fill (Black);
}

/**
  * @brief This function handles CW keyer UI events
  *
  */

void UI_Handler (void)
{
  if ((trx.sysclock - displayed) > 24)
  {
    displayed = trx.sysclock;
    sprintf (str, "         ");

    if (key_value < 2900) { key_pressed = 0; }
    if (key_value > 3000) { key_pressed = 1; }
    if (key_value > 3500) { key_pressed = 2; }
    if (key_value > 3900) { key_pressed = 3; }

    if (focus < 4U)
    {
      focus = TIM3->CNT >> 2U;
    }

    item_color = Black;

    if (focus == 4U)
    {
      ui_set_keyer_mode ();
    }
    else
    {
      ui_keyer_mode_to_string (cw_keyer.mode);

      if (focus == 1U)
      {
        if (key_pressed == 3U)
        {
          TIM3->ARR = 15U;
          TIM3->CNT = cw_keyer.mode << 2U;
          focus = 4U;
        }
      }
      else
      {
        item_color = White;
      }
    }

    ssd1306_SetCursor (5, 20);
    ssd1306_WriteString (" MODE  ", Font_7x10, item_color);

    ssd1306_SetCursor (54, 20);
    ssd1306_WriteString (str, Font_7x10, White);

    item_color = Black;

    if (focus == 5U)
    {
      ui_set_keyer_speed ();
    }
    else
    {
      sprintf (str, " %d WPM ", cw_keyer.speed);

      if (focus == 2U)
      {
        if (key_pressed == 3U)
        {
          TIM3->ARR = 227U;
          TIM3->CNT = (cw_keyer.speed - 4U) << 2U;
          focus = 5U;
        }
      }
      else
      {
        item_color = White;
      }
    }

    ssd1306_SetCursor (5, 32);
    ssd1306_WriteString (" SPEED ", Font_7x10, item_color);

    ssd1306_SetCursor (54, 32);
    ssd1306_WriteString (str, Font_7x10, White);


    item_color = Black;

    if (focus == 6U)
    {
      ui_set_keyer_pitch ();
    }
    else
    {
      sprintf (str, " %d Hz ", cw_keyer.pitch * 100U);

      if (focus == 3U)
      {
        if (key_pressed == 3U)
        {
          TIM3->ARR = 31U;
          TIM3->CNT = (cw_keyer.pitch - 3U) << 2U;
          focus = 6U;
        }
      }
      else
      {
        item_color = White;
      }
    }

    ssd1306_SetCursor (5, 44);
    ssd1306_WriteString (" PITCH ", Font_7x10, item_color);

    ssd1306_SetCursor (54, 44);
    ssd1306_WriteString (str, Font_7x10, White);

    HAL_ADC_Start_IT (&hadc1);

    ssd1306_UpdateScreen ();
  }
}

/****END OF FILE****/
