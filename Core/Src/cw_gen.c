/**
  *******************************************************************************
  *
  * @file    cw_gen.c
  * @brief   CW Generator
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
  *
  * This software uses some source code from cw_gen.c
  * https://github.com/df8oe/UHSDR/blob/active-devel/mchf-eclipse/drivers/audio/cw/
  * Copyright 2014 Krassi Atanassov M0NKA
  * re-implemented from:
  * https://www.wrbishop.com/ham/arduino-iambic-keyer-and-side-tone-generator/
  * by Steven T. Elliott K1EL (1998)
  *
  *******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "cw_gen.h"
#include "ptt_if.h"
#include "dds_if.h"

/* Private typedef -----------------------------------------------------------*/

typedef struct PaddleState
{
  /* State machine and port states */
  uint32_t port_state;
  uint32_t cw_state;
  uint32_t key_state;

  /* Element duration */
  int32_t dit_time;
  int32_t dah_time;
  int32_t pause_time;
  int32_t space_time;

  /* Timers */
  int32_t key_timer;
  int32_t break_timer;
  int32_t space_timer;

  /* Key clicks smoothing table current ptr */
  uint32_t sm_tbl_ptr;
  uint32_t sm_smooth_len;

  uint32_t ultim;

  uint32_t cw_char;
  uint32_t sending_char;
} PaddleState;


/* Private define ------------------------------------------------------------*/
// States
#define CW_IDLE             0
#define CW_DIT_CHECK        1
#define CW_DAH_CHECK        3
#define CW_KEY_DOWN         4
#define CW_KEY_UP           5
#define CW_PAUSE            6
#define CW_WAIT             7

#define CW_DIT_L            0x01
#define CW_DAH_L            0x02
#define CW_DIT_PROC         0x04
#define CW_END_PROC         0x10

#define CW_IAMBIC_A         0x00
#define CW_IAMBIC_B         0x10

#define CW_SMOOTH_LEN       2

/**
 *  CW_SMOOTH_STEPS = (CW_SMOOTH_TBL_SIZE * CW_SMOOTH_LEN) / (USBD_AUDIO_FREQ / 1000U) + 1U
 *
 *  with CW_SMOOTH_TBL_SIZE = 128
 *  2 => ~5.3ms for edges, ~ 6 steps of 1ms are required
 *  1 => ~2.7ms for edges, ~ 3 steps of 1ms are required
 *  3 => =8.0ms for edges, = 8 steps of 1ms are required
 */
#define CW_SMOOTH_STEPS     6       /* 1 step = 1ms for internal keyer */

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Public paddle state */

PaddleState  ps;
CW_Keyer cw_keyer;

/* Blackman-Harris function to keep CW signal bandwidth narrow */

#define CW_SMOOTH_TBL_SIZE  128

static const float sm_table [CW_SMOOTH_TBL_SIZE] =
{
    0.0000000000000000, 0.0000686004957883,
    0.0000945542356652, 0.0001383179721175, 0.0002006529396865,
    0.0002826248158738, 0.0003856036471126, 0.0005112637206964,
    0.0006615833583608, 0.0008388446022225, 0.0010456327590236,
    0.0012848357641460, 0.0015596433227128, 0.0018735457812732,
    0.0022303326801620, 0.0026340909336119, 0.0030892025821568,
    0.0036003420597745, 0.0041724729166402, 0.0048108439372987,
    0.0055209845935232, 0.0063086997711494, 0.0071800637107376,
    0.0081414131030473, 0.0091993392820062, 0.0103606794601087,
    0.0116325069539978, 0.0130221203513468, 0.0145370315740592,
    0.0161849527972270, 0.0179737821882093, 0.0199115884355984,
    0.0220065940436903, 0.0242671573743564, 0.0267017534248783,
    0.0293189533373318, 0.0321274026424438, 0.0351357982484650,
    0.0383528641934459, 0.0417873261873527, 0.0454478849786327,
    0.0493431885881256, 0.0534818034615354, 0.0578721846000008,
    0.0625226447365667, 0.0674413226345205, 0.0726361505915551,
    0.0781148212415210, 0.0838847537530600, 0.0899530595316440,
    0.0963265075384118, 0.1030114893456630, 0.1100139840548870,
    0.1173395232087210, 0.1249931558332360, 0.1329794137513460,
    0.1413022773119520, 0.1499651416825930, 0.1589707838558700,
    0.1683213305216760, 0.1780182269583640, 0.1880622070962730,
    0.1984532649066170, 0.2091906272675180, 0.2202727284569900,
    0.2316971864198910, 0.2434607809523230, 0.2555594339426260,
    0.2679881918029900, 0.2807412102198870, 0.2938117413448940,
    0.3071921235401840, 0.3208737737849510, 0.3348471828403530,
    0.3491019132612700, 0.3636266003332790, 0.3784089560027740,
    0.3934357758572420, 0.4086929492012480, 0.4241654722618500,
    0.4398374645449960, 0.4556921883519000, 0.4717120714516800,
    0.4878787328935640, 0.5041730119288920, 0.5205749999999980,
    0.5370640757398780, 0.5536189429134430, 0.5702176712181650,
    0.5868377398491150, 0.6034560837207990, 0.6200491422259460,
    0.6365929103995070, 0.6530629923446170, 0.6694346567663080,
    0.6856828944482830, 0.7017824774982180, 0.7177080201778180,
    0.7334340411253520, 0.7489350267705930, 0.7641854957350820,
    0.7791600640044710, 0.7938335106543350, 0.8081808439064380,
    0.8221773672888840, 0.8357987456709900, 0.8490210709420900,
    0.8618209271027980, 0.8741754545375520, 0.8860624132385800,
    0.8974602447536510, 0.9083481326332570, 0.9187060611570670,
    0.9285148721246550, 0.9377563195016140, 0.9464131217191720,
    0.9544690114333140, 0.9619087825581750, 0.9687183343980040,
    0.9748847127123550, 0.9803961475602210, 0.9852420877805500,
    0.9894132319790110, 0.9929015559037870, 0.9957003361066840,
    0.9978041697997700, 0.9992089908321180, 0.9999120817258750
};

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

extern PTT_TypeDef ptt;

/* Private functions ---------------------------------------------------------*/

/**
 * @brief This function turns device to RX mode
 *
 */

static void cw_ptt_set_rx (void)
{
  PTT_Key_Off_Time ();
}

/**
 * @brief This function sets CW delay
 *
 */

static void cw_set_delay (void)
{
  ps.break_timer = 1;
}

/**
 * @brief This function checks paddles state for iambic keyer mode
 *
 */

static void cw_get_paddle_state (void)
{
  if (ptt.key_dah_is_on)
  {
    ps.port_state |= CW_DAH_L;
  }

  if (ptt.key_dit_is_on)
  {
    ps.port_state |= CW_DIT_L;
  }

  if (ps.cw_state == CW_IDLE)
  {
    /* See original! There is any procedures for letter decoding? */
  }
}

/**
 * @brief This function checks paddles state for ultimate keyer mode
 *
 */

static void cw_get_first_paddle (void)
{
  if (cw_keyer.mode == ULTIMATE)
  {
    if (!ptt.key_dit_is_on && ptt.key_dah_is_on)
    {
      ps.ultim = 1U;
    }

    if (ptt.key_dit_is_on && !ptt.key_dah_is_on)
    {
      ps.ultim = 0U;
    }
  }
}


/**
 * @brief This function removes clicks at start end end of tone
 *
 */

static void cw_chirp_off (int16_t *i_buffer, int16_t *q_buffer, uint8_t rising)
{
  *i_buffer = (int16_t) ((float) *i_buffer * sm_table [ps.sm_tbl_ptr]);
  *q_buffer = (int16_t) ((float) *q_buffer * sm_table [ps.sm_tbl_ptr]);

  ps.sm_smooth_len++;

  if (ps.sm_smooth_len == CW_SMOOTH_LEN)
  {
    ps.sm_smooth_len = 0U;

    if (rising)
    {
      if (ps.sm_tbl_ptr < (CW_SMOOTH_TBL_SIZE - 1U))
      {
        ps.sm_tbl_ptr++;
      }
    }
    else
    {
      if (ps.sm_tbl_ptr > 0U)
      {
        ps.sm_tbl_ptr--;
      }
    }
  }
}

/**
 * @brief This function puts CW tone to audio buffer
 *
 * @param IQ audio buffer pointers
 * @param CW keyer mode: 1 = straight mode, 0 = any other modes
 * @param scaling = CW tone volume 0...256
 */

static void cw_tone_gen (int16_t *i_buffer, int16_t *q_buffer, uint8_t straight)
{
  uint8_t scaling = 96U; /* CW tone volume 0...255 */

  int16_t i_buf = *i_buffer;
  int16_t q_buf = *q_buffer;

  int16_t i_dds, q_dds;

  DDS_Get_IQ_Sample (&i_dds, &q_dds);

  if (straight)
  {
    /* It's a rising edge of CW signal */
    if (ps.key_state == 3U)
    {
      cw_chirp_off (&i_dds, &q_dds, 1U);

      if (ps.sm_tbl_ptr >= (CW_SMOOTH_TBL_SIZE - 1U))
      {
        /* There is a constant CW signal */
        ps.key_state  = 2U;
        ps.sm_tbl_ptr = (CW_SMOOTH_TBL_SIZE - 1U);
      }
    }

    /* It's a falling edge of CW signal */
    if (ps.key_state == 1U)
    {
      cw_chirp_off (&i_dds, &q_dds, 0U);

      if (ps.sm_tbl_ptr == 0U)
      {
        /* There isn't CW signal */
        ps.key_state = 0U;
      }
    }
  }
  else
  {
    /* It's a rising edge of CW signal */
    if (ps.key_timer > (ps.dit_time / 2))
    {
      cw_chirp_off (&i_dds, &q_dds, 1U);

      if (ps.sm_tbl_ptr >= (CW_SMOOTH_TBL_SIZE - 1U))
      {
        /* There is a constant CW signal */
        ps.sm_tbl_ptr = (CW_SMOOTH_TBL_SIZE - 1U);
      }
    }

    /* It's a falling edge of CW signal */
    if (ps.key_timer <= CW_SMOOTH_STEPS)
    {
      cw_chirp_off (&i_dds, &q_dds, 0U);

      if (ps.sm_tbl_ptr == 0U)
      {
        /* There isn't CW signal */
        ps.key_timer = 0U;
      }
    }
  }

  i_dds = (i_dds * scaling) / 256U;
  q_dds = (q_dds * scaling) / 256U;

  i_buf = (i_buf * (256 - scaling)) / 256U;
  q_buf = (q_buf * (256 - scaling)) / 256U;

  *i_buffer = i_dds + i_buf;
  *q_buffer = q_dds + q_buf;
}

/**
 * @brief This function handles iambic keyer events
 *
 * @param IQ audio buffer pointers
 * @param number of samples
 */

void cw_iambic_keyer_handler (int16_t *i_buffer, int16_t *q_buffer, uint16_t size)
{
  uint8_t  repeat;
  uint16_t i;

  do
  {
    repeat = 0U;

    switch (ps.cw_state)
    {
      case CW_IDLE:
      {
        repeat = 0U;

        cw_get_paddle_state ();
        /* If at least one paddle is still or has been recently pressed */
        if (ps.port_state & ( CW_DAH_L | CW_DIT_L))
        {
          ps.cw_state = CW_WAIT;
          repeat = 1U;
        }
        else
        {
          if (ps.port_state & CW_END_PROC)
          {
            ps.port_state &= ~CW_END_PROC;
            ps.space_timer = ps.space_time;
          }

          if (ps.space_timer > 0)
          {
            ps.space_timer--;

            if (ps.space_timer == 0)
            {
              if (ps.sending_char == 1)
              {
                ps.sending_char = 0;
              }
            }
          }

          if (ps.break_timer > 0 && !ps.sending_char)
          {
            ps.break_timer--;

            if (ps.break_timer == 0)
            {
              cw_ptt_set_rx ();
            }
          }
        }
      }
        break;
      case CW_WAIT:
      {
        ps.cw_state = CW_DIT_CHECK;
        repeat = 1U;
      }
        break;
      case CW_DIT_CHECK:
      {
        if (ps.port_state & CW_DIT_L)
        {
          ps.port_state |= CW_DIT_PROC;
          ps.cw_state    = CW_KEY_DOWN;
          ps.key_timer   = ps.dit_time;
          ps.cw_char     = ps.cw_char * 4 + 2;
        }
        else
        {
          ps.cw_state = CW_DAH_CHECK;
        }
        repeat = 1U;
      }
        break;
      case CW_DAH_CHECK:
      {
        if (ps.port_state & CW_DAH_L)
        {
          ps.cw_state  = CW_KEY_DOWN;
          ps.key_timer = ps.dah_time;
          ps.cw_char   = ps.cw_char * 4 + 3;
        }
        else
        {
          cw_set_delay ();
          ps.cw_state = CW_IDLE;
        }
        repeat = 1U;
      }
        break;
      case CW_KEY_DOWN:
      {
        ps.sm_tbl_ptr = 0;

        for (i = 0U; i < size; i += 2U)
        {
          /* This is CW tone generation call */
          cw_tone_gen (&i_buffer [i+0], &i_buffer [i+1], 0U);

          if (ps.key_timer == 0U) break;
        }

        if (ps.key_timer)
        {
          ps.key_timer--;
        }

        ps.port_state &= ~(CW_DIT_L + CW_DAH_L);
        ps.cw_state    = CW_KEY_UP;
      }
        break;
      case CW_KEY_UP:
      {
        if (ps.key_timer == 0)
        {
          ps.key_timer = ps.pause_time;
          ps.cw_state  = CW_PAUSE;
        }
        else
        {
          for (i = 0U; i < size; i += 2U)
          {
            /* This is CW tone generation call */
            cw_tone_gen (&i_buffer [i+0], &i_buffer [i+1], 0U);

            if (ps.key_timer == 0U) break;
          }

          if (ps.key_timer)
          {
            ps.key_timer--;
          }

          if (cw_keyer.mode == IAMBIC_B)
          {
            cw_get_paddle_state ();
          }
        }
      }
        break;
      case CW_PAUSE:
      {
        cw_get_paddle_state ();

        ps.key_timer--;

        if (ps.key_timer == 0)
        {
          if (ps.cw_char > 50000)
          {
            ps.port_state &= ~CW_END_PROC;
          }
          else
          {
            ps.port_state |= CW_END_PROC;
          }

          if (cw_keyer.mode == IAMBIC_A || cw_keyer.mode == IAMBIC_B)
          {
            if (ps.port_state & CW_DIT_PROC)
            {
              ps.port_state &= ~(CW_DIT_L + CW_DIT_PROC);
              ps.cw_state    = CW_DAH_CHECK;
            }
            else
            {
              ps.port_state &= ~(CW_DAH_L);
              ps.cw_state    = CW_IDLE;
              cw_set_delay ();
            }
          }
          else
          {
            cw_get_first_paddle ();

            if ((ps.port_state & CW_DAH_L) && ps.ultim == 0)
            {
              ps.port_state &= ~(CW_DIT_L + CW_DIT_PROC);
              ps.cw_state    = CW_DAH_CHECK;
            }
            else
            {
              ps.port_state &= ~(CW_DAH_L);
              ps.cw_state    = CW_IDLE;
              cw_set_delay ();
            }
          }

          repeat = 1U;
        }
      }
        break;
      default:
        break;
    }
  }
  while (repeat);
}

/**
 * @brief This function sets iambic keyer speed in WPS
 *
 */

void CW_Set_Speed (void)
{
  /* 1200000 = 1.2s per dit == 1WPM ; 1200 impulse per second audio irq = 1200 ticks per 1 WPM dit
   * we scale with 100 for better precision and easier use of the weight value which is scaled by 100.
   * so 1200 * 100 = 120000
   */

  /* weight 1.00 */
  int32_t dit_time   = 1 * 120000 / cw_keyer.speed + CW_SMOOTH_STEPS * 100;  /* +9 =  6ms * 1/1200 =  0,006*1200 */
  int32_t pause_time = 1 * 120000 / cw_keyer.speed - CW_SMOOTH_STEPS * 100;  /* -9 = -6ms * 1/1200 = -0,006*1200 */
  int32_t dah_time   = 3 * 120000 / cw_keyer.speed + CW_SMOOTH_STEPS * 100;  /* +9 =  6ms * 1/1200 =  0,006*1200 */
  int32_t space_time = 6 * 120000 / cw_keyer.speed;

  //int32_t weight_corr = ((int32_t)ts.cw_keyer_weight - 100) * dit_time / 100;
  int32_t weight_corr = 0;

  /* we add the correction value to both dit and dah and subtract from pause. dah gets less change proportionally because of this */
  ps.dit_time   = (dit_time   + weight_corr) / 100;
  ps.dah_time   = (dah_time   + weight_corr) / 100;
  ps.pause_time = (pause_time - weight_corr) / 100;
  ps.space_time =  space_time / 100;
}

/**
 * @brief This function initiates CW keyer mode
 *
 */

void CW_Set_Keyer (void)
{
  CW_Set_Speed ();

  cw_set_delay ();

  ps.cw_state  = CW_IDLE;
  ps.key_timer = 0;
  ps.key_state = 0;


  switch (cw_keyer.mode)
  {
    case IAMBIC_B:
      ps.port_state = CW_IAMBIC_B;
      break;
    case IAMBIC_A:
      ps.port_state = CW_IAMBIC_A;
      break;
    default:
      break;
  }

  ps.cw_char      = 0;
  ps.space_timer  = 0;
  ps.sending_char = 0;
}

/**
 * @brief This function sets CW tone pitch
 *
 * @param CW tone pitch in Hz
 * @param Audio devise sample rate in Hz
 */

void CW_Set_Pitch (uint32_t freq, uint32_t sample_rate)
{
  DDS_Set_CW_Pitch (freq, sample_rate);
}

/**
 * @brief called every 1000u (== 1000Hz) from I2S IRQ, does cw tone generation
 *
 * @param IQ audio buffer pointers
 * @param number of samples
 */

void CW_Handler (int16_t *i_buffer, int16_t *q_buffer, uint16_t size)
{
  if (ptt.dtr_is_on || ((cw_keyer.mode == STRAIGHT) && (ptt.key_dah_is_on || ptt.key_dit_is_on)))
  {
    if (ps.key_state == 0U)   /* Is a CW key up? */
    {
      ps.key_state = 3U;      /* It's a rising edge of CW signal */
    }
  }
  else if (ps.key_state)
  {
      ps.key_state = 1U;      /* It's a falling edge of CW signal */
  }

  if (ps.key_state)
  {
    for (uint16_t i = 0U; i < size; i += 2U)
    {
      cw_tone_gen (&i_buffer [i + 0], &i_buffer [i + 1], 1U);

      if (ps.key_state == 0U) /* Is a CW key up? */
      {
        PTT_Key_Off_Time ();  /* Set mode to RX with timeout */
        break;
      }
    }
  }
  else if (cw_keyer.mode < STRAIGHT)
  {
    cw_iambic_keyer_handler (i_buffer, q_buffer, size);
  }
}

/****END OF FILE****/
