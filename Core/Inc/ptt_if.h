/**
  *******************************************************************************
  *
  * @file    ptt_if.h
  * @brief   Header for ptt_if.c file
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef PTT_IF_H_
#define PTT_IF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint8_t  dtr_is_on;
  uint8_t  rts_is_on;
  uint8_t  key_dah_is_on;
  uint8_t  key_dit_is_on;
  uint32_t key_off_time;
} PTT_TypeDef;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

void PTT_Init (void);
void PTT_Handler (void);

void PTT_Set_Mode (uint8_t);
void PTT_CAT_TX (uint8_t);
void PTT_DTR_TX (uint8_t);
void PTT_RTS_TX (uint8_t);
void PTT_Key_Off_Time (void);

void VFO_Toggle_VFO (void);
void VFO_Set_Tune (uint32_t);
void VFO_Set_Tune_BCD (uint32_t);
void VFO_Set_Tune_UI (uint32_t);
uint32_t VFO_Get_Tune (void);
uint32_t VFO_Get_Tune_BCD (void);
void VFO_Set_Split (uint8_t);

/* Private defines -----------------------------------------------------------*/

#define KEY_TIMEOUT                           25 // 250 ms

#ifdef __cplusplus
}
#endif

#endif /* PTT_IF_H_ */
