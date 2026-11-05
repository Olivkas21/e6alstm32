#ifndef PWM_H
#define PWM_H
#include <stdint.h>

void PWM_Init(void);
void PWM_SetDuty(uint8_t duty);      
void PWM_SetFrequency(uint32_t freq); 
void PWM_Toggle(int state);           

#endif