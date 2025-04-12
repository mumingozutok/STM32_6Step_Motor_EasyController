/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define GPIO_UL_Pin GPIO_PIN_7
#define GPIO_UL_GPIO_Port GPIOA
#define GPIO_VL_Pin GPIO_PIN_0
#define GPIO_VL_GPIO_Port GPIOB
#define GPIO_WL_Pin GPIO_PIN_1
#define GPIO_WL_GPIO_Port GPIOB
#define ENCO_Z_Pin GPIO_PIN_10
#define ENCO_Z_GPIO_Port GPIOB
#define ENCO_Z_EXTI_IRQn EXTI15_10_IRQn
#define LED_Pin GPIO_PIN_12
#define LED_GPIO_Port GPIOB
#define PWM_UH_Pin GPIO_PIN_8
#define PWM_UH_GPIO_Port GPIOA
#define PWM_VH_Pin GPIO_PIN_9
#define PWM_VH_GPIO_Port GPIOA
#define PWM_WH_Pin GPIO_PIN_10
#define PWM_WH_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define ENCO_A_Pin GPIO_PIN_15
#define ENCO_A_GPIO_Port GPIOA
#define ENCO_B_Pin GPIO_PIN_3
#define ENCO_B_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
