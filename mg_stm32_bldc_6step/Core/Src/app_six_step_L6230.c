/*
 * app_six_step.c
 *
 *  Created on: May 2, 2025
 *      Author: mumin
 */


#include <app_six_step_L6230.h>
#include "stm32g4xx_hal_gpio_ex.h"

extern TIM_HandleTypeDef htim1;
static uint8_t current_step = 0;

#define true 1
#define false 0

// 6-step tablosu
//CW rotation

static const CommutationStep comm_table[6] = {
    {PWM,  LOW, HIZ},
    {PWM,  HIZ, LOW},
    {HIZ,  PWM, LOW},
    {LOW,  PWM, HIZ},
    {LOW,  HIZ, PWM},
    {HIZ,  LOW, PWM}
};

/*
//CCW rotation
static const CommutationStep comm_table[6] = {
    //U,   V,   W
    {HIZ,  LOW, PWM},
    {LOW,  HIZ, PWM},
    {LOW,  PWM, HIZ},
    {HIZ,  PWM, LOW},
    {PWM,  HIZ, LOW},
    {PWM,  LOW, HIZ}
};
*/

void switch_pwm_gpio_mode(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t is_pwm) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    if (is_pwm) {
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Alternate = GPIO_AF6_TIM1;  // TIM1 için AF1, G431RB'ye göre kontrol et
    } else {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    }

    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

// === U Fazı (PWM_UH: PA8, EN1: PC10) ===
void Low_U(void) {
    HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_SET); // EN1 -> PC10
}

void set_PWM_U(void) {
    switch_pwm_gpio_mode(PWM_UH_GPIO_Port, PWM_UH_Pin, true);
    Low_U();
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

void hiZ_U(void) {
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    switch_pwm_gpio_mode(PWM_UH_GPIO_Port, PWM_UH_Pin, false);
    HAL_GPIO_WritePin(PWM_UH_GPIO_Port, PWM_UH_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_RESET);
}


// === V Fazı (PWM_VH: PA9, EN2: PC11) ===
void Low_V(void) {
    HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_SET); // EN2 -> PC11
}

void set_PWM_V(void) {
    switch_pwm_gpio_mode(PWM_VH_GPIO_Port, PWM_VH_Pin, true);
    Low_V();
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
}

void hiZ_V(void) {
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    switch_pwm_gpio_mode(PWM_VH_GPIO_Port, PWM_VH_Pin, false);
    HAL_GPIO_WritePin(PWM_VH_GPIO_Port, PWM_VH_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_RESET);
}


// === W Fazı (PWM_WH: PA10, EN3: PC12) ===
void Low_W(void) {
    HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_SET); // EN3 -> PC12
}

void set_PWM_W(void) {
    switch_pwm_gpio_mode(PWM_WH_GPIO_Port, PWM_WH_Pin, true);
    Low_W();
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
}

void hiZ_W(void) {
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
    switch_pwm_gpio_mode(PWM_WH_GPIO_Port, PWM_WH_Pin, false);
    HAL_GPIO_WritePin(PWM_WH_GPIO_Port, PWM_WH_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_RESET);
}

void hizAllPWMChannels(){
	hiZ_U();hiZ_V();hiZ_W();
}



void set_duty_cycle(uint16_t duty) {
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty); // U
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, duty); // V
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, duty); // W
}

//-----------------------------------------------------------------

// Step uygula (bir adım ilerle)
void motor_commutation_step(uint8_t inc) {
    const CommutationStep s = comm_table[current_step];

    switch (s.U) {
            case PWM:
            	set_PWM_U();
                break;
            case LOW:
            	Low_U();
                break;
            case HIZ:
            default:
            	hiZ_U();
                break;
        }

        // --- V Fazı ---
        switch (s.V) {
        case PWM:
        	set_PWM_V();
            break;
        case LOW:
        	Low_V();
            break;
        case HIZ:
        default:
        	hiZ_V();
            break;
        }

        // --- W Fazı ---
        switch (s.W) {
        case PWM:
        	set_PWM_W();
            break;
        case LOW:
        	Low_W();
            break;
        case HIZ:
        default:
        	hiZ_W();
            break;
        }

        if(inc == 1)
        	current_step = (current_step + 1) % 6;
}


void motor_set_step(uint8_t step) {
    if (step < 6) {
        current_step = step;
    }
}

uint8_t motor_get_step(void) {
    return current_step;
}
