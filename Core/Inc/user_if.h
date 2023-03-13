/**
  *******************************************************************************
  *
  * @file    user_if.h
  * @brief   Header for user_if.c file
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

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef INC_USER_IF_H_
#define INC_USER_IF_H_

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "stm32f4xx_hal.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

void UI_Init (void);
void UI_Handler (void);

/* Private defines -----------------------------------------------------------*/


#endif /* INC_USER_IF_H_ */
