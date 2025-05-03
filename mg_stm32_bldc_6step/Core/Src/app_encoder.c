/*
 * app_encoder.c
 *
 *  Created on: May 2, 2025
 *      Author: mumin
 */

#include "app_encoder.h"
#include "math.h"

#define ENCODER_CPR 600  // 600 pulses per revolution
#define COUNTS_PER_REV (ENCODER_CPR * 4)

extern TIM_HandleTypeDef htim2; //encoder timer

void encoder_init() {
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    __HAL_TIM_SET_COUNTER(&htim2, 0);  // reset counter
}

int32_t encoder_get_count(void) {
    return (int32_t)__HAL_TIM_GET_COUNTER(&htim2);
}

void encoder_reset(void) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
}

float encoder_get_angle_rad(void) {
    int32_t count = encoder_get_count();
    float angle = ((float)count / (float)COUNTS_PER_REV) * 2.0f * (float)M_PI;
    return angle;
}

float encoder_get_angle_deg(void) {
    int32_t count;
    float angle;

    count = encoder_get_count();
    angle = ((float)count / (float)COUNTS_PER_REV) * 360.0f;

    return angle;
}
