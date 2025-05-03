/*
 * app_encoder.h
 *
 *  Created on: May 2, 2025
 *      Author: mumin
 */

#ifndef INC_APP_ENCODER_H_
#define INC_APP_ENCODER_H_

#include "main.h"

void encoder_init();
int32_t encoder_get_count(void);
void encoder_reset(void);
float encoder_get_angle_rad(void);
float encoder_get_angle_deg(void);

#endif /* INC_APP_ENCODER_H_ */
