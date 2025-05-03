/*
 * app_six_step.h
 *
 *  Created on: May 2, 2025
 *      Author: mumin
 */

#ifndef INC_APP_SIX_STEP_L6230_H_
#define INC_APP_SIX_STEP_L6230_H_

#include "main.h"

typedef enum { HIZ = -1, LOW = 0, PWM = 1 } PhaseAction;

typedef struct {
    PhaseAction U;
    PhaseAction V;
    PhaseAction W;
} CommutationStep;

void motor_commutation_init(TIM_HandleTypeDef* comm_timer);
void motor_commutation_step(uint8_t inc);
void motor_set_step(uint8_t step);  // Manuel step ataması
uint8_t motor_get_step(void);
void set_duty_cycle(uint16_t duty) ;
void hizAllPWMChannels();

#endif /* INC_APP_SIX_STEP_L6230_H_ */
