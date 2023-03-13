/**
  *******************************************************************************
  *
  * @file    cw_gen.h
  * @brief   Header for cw_gen.c file
  * @version v1.0
  * @date    02.11.2022
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INC_CW_GEN_H_
#define INC_CW_GEN_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"

/* Exported types ------------------------------------------------------------*/

typedef struct CW_Keyer
{
  uint8_t  mode;
  uint8_t  speed;
  uint8_t  pitch;
} CW_Keyer;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

void CW_Set_Pitch (uint32_t, uint32_t);
void CW_Set_Keyer (void);
void CW_Set_Speed (void);
void CW_Handler   (int16_t*, int16_t*, uint16_t);

/* Private defines -----------------------------------------------------------*/

#define IAMBIC_B            0
#define IAMBIC_A            1
#define ULTIMATE            2
#define STRAIGHT            3

#endif /* INC_CW_GEN_H_ */
