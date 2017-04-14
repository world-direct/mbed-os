/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal_gpio.h"


/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define GSM_NetLig_Pin GPIO_PIN_2
#define GSM_NetLig_GPIO_Port GPIOE
#define GSM_RFTxMon_Pin GPIO_PIN_3
#define GSM_RFTxMon_GPIO_Port GPIOE
#define ETH_nINT_Pin GPIO_PIN_4
#define ETH_nINT_GPIO_Port GPIOE
#define BUS_nINT_Pin GPIO_PIN_5
#define BUS_nINT_GPIO_Port GPIOE
#define GSM_nReset_Pin GPIO_PIN_6
#define GSM_nReset_GPIO_Port GPIOE
#define GSM_RI_Pin GPIO_PIN_13
#define GSM_RI_GPIO_Port GPIOC
#define ETH_RXER_Pin GPIO_PIN_14
#define ETH_RXER_GPIO_Port GPIOC
#define GSM_CTS_Pin GPIO_PIN_0
#define GSM_CTS_GPIO_Port GPIOA
#define GSM_RXD_Pin GPIO_PIN_3
#define GSM_RXD_GPIO_Port GPIOA
#define ONEWIRE_TxH_Pin GPIO_PIN_0
#define ONEWIRE_TxH_GPIO_Port GPIOB
#define ONEWIRE_Tx_Pin GPIO_PIN_1
#define ONEWIRE_Tx_GPIO_Port GPIOB
#define ONEWIRE_Rx_Pin GPIO_PIN_2
#define ONEWIRE_Rx_GPIO_Port GPIOB
#define DBG_RX_Pin GPIO_PIN_7
#define DBG_RX_GPIO_Port GPIOE
#define DBG_TX_Pin GPIO_PIN_8
#define DBG_TX_GPIO_Port GPIOE
#define BUS_LED_Pin GPIO_PIN_10
#define BUS_LED_GPIO_Port GPIOE
#define RGBLED1_BL_Pin GPIO_PIN_11
#define RGBLED1_BL_GPIO_Port GPIOE
#define RGBLED1_RT_Pin GPIO_PIN_12
#define RGBLED1_RT_GPIO_Port GPIOE
#define RGBLED1_GN_Pin GPIO_PIN_13
#define RGBLED1_GN_GPIO_Port GPIOE
#define RGBLED2_BL_Pin GPIO_PIN_14
#define RGBLED2_BL_GPIO_Port GPIOE
#define RGBLED2_RT_Pin GPIO_PIN_15
#define RGBLED2_RT_GPIO_Port GPIOE
#define RGBLED2_GN_Pin GPIO_PIN_10
#define RGBLED2_GN_GPIO_Port GPIOB
#define GSM_PWRMON_Pin GPIO_PIN_14
#define GSM_PWRMON_GPIO_Port GPIOB
#define GSM_ON_OFF_Pin GPIO_PIN_15
#define GSM_ON_OFF_GPIO_Port GPIOB
#define nWakeUp_Pin GPIO_PIN_9
#define nWakeUp_GPIO_Port GPIOD
#define OLED_A0_Pin GPIO_PIN_10
#define OLED_A0_GPIO_Port GPIOD
#define SPI1_nCS0_Pin GPIO_PIN_12
#define SPI1_nCS0_GPIO_Port GPIOD
#define SPI1_nCS1_Pin GPIO_PIN_13
#define SPI1_nCS1_GPIO_Port GPIOD
#define SPI1_nCS2_Pin GPIO_PIN_14
#define SPI1_nCS2_GPIO_Port GPIOD
#define SPI1_nCS3_Pin GPIO_PIN_15
#define SPI1_nCS3_GPIO_Port GPIOD
#define SPI1_nCS4_Pin GPIO_PIN_6
#define SPI1_nCS4_GPIO_Port GPIOC
#define HW_Version8_Pin GPIO_PIN_7
#define HW_Version8_GPIO_Port GPIOC
#define HW_Version4_Pin GPIO_PIN_8
#define HW_Version4_GPIO_Port GPIOC
#define HW_Version2_Pin GPIO_PIN_9
#define HW_Version2_GPIO_Port GPIOC
#define HW_Version1_Pin GPIO_PIN_8
#define HW_Version1_GPIO_Port GPIOA
#define USBID_Pin GPIO_PIN_10
#define USBID_GPIO_Port GPIOA
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA
#define BUS_TxD0_Pin GPIO_PIN_10
#define BUS_TxD0_GPIO_Port GPIOC
#define BUS_RxD0_Pin GPIO_PIN_11
#define BUS_RxD0_GPIO_Port GPIOC
#define TXT4_Pin GPIO_PIN_12
#define TXT4_GPIO_Port GPIOC
#define RTS4_Pin GPIO_PIN_0
#define RTS4_GPIO_Port GPIOD
#define RXT4_Pin GPIO_PIN_2
#define RXT4_GPIO_Port GPIOD
#define GSM_RTS_Pin GPIO_PIN_4
#define GSM_RTS_GPIO_Port GPIOD
#define GSM_TXD_Pin GPIO_PIN_5
#define GSM_TXD_GPIO_Port GPIOD
#define Tx2_Pin GPIO_PIN_6
#define Tx2_GPIO_Port GPIOB
#define Rx2_Pin GPIO_PIN_7
#define Rx2_GPIO_Port GPIOB
#define RX1_Pin GPIO_PIN_0
#define RX1_GPIO_Port GPIOE
#define TX1_Pin GPIO_PIN_1
#define TX1_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
