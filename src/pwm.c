#include "stm32f4xx.h"
#include "pwm.h"

void PWM_Init(void) {
    // [ИЗМЕНЕНО ДЛЯ ЛР4] Используем TIM2 для пина PA5
    // Предделитель: 16 МГц / (PSC+1) = 10 кГц (стр. 364)
    TIM2->PSC = 1599; 
    
    // Период: 10 кГц / (ARR+1) = 10 Гц (стр. 364)
    TIM2->ARR = 99;  
    
    // скважность 50%: CCR1 = 500 из 1000 (стр. 365)
    TIM2->CCR1 = 500; 

    // бит 3 (OC1PE=1): предзагрузкa CCR1 // от гличей (стр. 358)
    TIM2->CCMR1 |= (0x1 << 3);
    
    // биты 4-6 (OC1M=110): режим PWM Mode 1 (стр. 358)
    TIM2->CCMR1 &= ~(0x7 << 4);
    TIM2->CCMR1 |=  (0x6 << 4);

    // bit 0 | 1: для передачи сигнала (стр. 362)
    TIM2->CCER |= (0x1 << 0);

    // bit 0 | 1 : запуск PSC, ARR, CCR1 сразу (стр. 357)
    TIM2->EGR |= (0x1 << 0);

    // Бит 0 |1: запуск счёт(стр. 350)
    TIM2->CR1 |= (0x1 << 0);
}

void PWM_SetDuty(uint8_t duty) {
    if (duty > 100) duty = 100;
    TIM2->CCR1 = (TIM2->ARR * duty) / 100; // [ИЗМЕНЕНО] TIM3 -> TIM2
}

void PWM_SetFrequency(uint32_t freq) {
    if (freq == 0) return;
    TIM2->PSC = 15; // [ИЗМЕНЕНО] TIM3 -> TIM2
    TIM2->ARR = (16000000 / 16 / freq) - 1;
}

void PWM_Toggle(int state) {
    if (state) TIM2->CCER |= (0x1 << 0);  // [ИЗМЕНЕНО] TIM3 -> TIM2
    else TIM2->CCER &= ~(0x1 << 0);       
}