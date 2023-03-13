/**
  *******************************************************************************
  *
  * @file    dds_if.h
  * @brief   Header for dsp_if.c file
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
  *******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INC_DDS_IF_H_
#define INC_DDS_IF_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"

/* Exported types ------------------------------------------------------------*/

/* Soft DDS public structure */
typedef struct
{
  uint32_t ptr;   /* DDS pointer */
  uint32_t step;  /* DDS step - not working if part of the structure */
} soft_dds_t;

/* Exported constants --------------------------------------------------------*/

#define DDS_TBL_BITS        10
#define DDS_TBL_SIZE        (1 << DDS_TBL_BITS) /* 2^10 = 1024 */

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

void DDS_Set_CW_Pitch  (uint32_t freq, uint32_t sample_rate);
void DDS_CW_Monitor    (int16_t *buff0, int16_t *buff1, uint8_t scaling);
void DDS_Get_Sample    (int16_t *buff);
void DDS_Get_IQ_Sample (int16_t *i_buff, int16_t *q_buff);


void softdds_setFreqDDS (soft_dds_t *dds, uint32_t freq, uint32_t sample_rate, uint8_t smooth);
void softdds_genIQSingleTone (soft_dds_t *dds, int16_t *i_buff, int16_t *q_buff, uint16_t size);
void softdds_genIQTwoTone (soft_dds_t *ddsA, soft_dds_t *ddsB, uint16_t *i_buff, uint16_t *q_buff, uint16_t size);

void softdds_addSingleTone (soft_dds_t *dds_ptr, int16_t *buff, uint8_t scaling);
void softdds_addSingleToneToTwobuffers (soft_dds_t *dds_ptr, int16_t *buff0, int16_t *buff1, uint8_t scaling);

void softdds_runIQ (uint16_t *i_buff, uint16_t *q_buff, uint16_t size);
void softdds_configRunIQ (uint32_t freq [2], uint32_t samp_rate, uint8_t smooth);

/* Private defines -----------------------------------------------------------*/



#endif /* INC_DDS_IF_H_ */
