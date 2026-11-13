#include "stm32f4xx.h"
#include "pwm.h"

void PWM_Init(void) {
    // use TIM2 to pin PA5(ld)
    // Предделитель: 16 МГц / (PSC+1) = 10 кГц (стр. 364)
    TIM2->PSC = 1599; 
    
    // Период: 10 кГц / (ARR+1) = 10 Гц (стр. 364)
    TIM2->ARR = 999;  
    
    // скважность 50%: CCR1 = 500 из 1000 (стр. 365)
    TIM2->CCR1 = 500; 

    // bit 3 (OC1PE=1): predreload CCR1 // no glogh (стр. 358)
    TIM2->CCMR1 |= (0x1 << 3);
    
    // bit 4-6 (OC1M=110): -> PWM Mode 1 (стр. 358)
    TIM2->CCMR1 &= ~(0x7 << 4);
    TIM2->CCMR1 |=  (0x6 << 4);

    // bit 0 | 1: signal go (стр. 362)
    TIM2->CCER |= (0x1 << 0);

    // bit 0 | 1 : start PSC, ARR, CCR1 now (стр. 357)
    TIM2->EGR |= (0x1 << 0);

    // Бит 0 |1: start count(стр. 350)
    TIM2->CR1 |= (0x1 << 0);
}
// funktion for set skvagnost(duty)
void PWM_SetDuty(uint8_t duty) {
    if (duty > 100) duty = 100;
    // CCR1 = (ARR * duty) / 100. 
    //  ARR=99 и duty=50 -> 999*50/100 = 499 (половина)
    // (стр. 365) CCR1 - регистр сравнения, меняем скважность на лету
    TIM2->CCR1 = (TIM2->ARR * duty) / 100; // TIM2
}

// funktion for set freq (chastota) in Hz
void PWM_SetFrequency(uint32_t freq) {
    // if freq = 0 -> exit
    if (freq == 0) return;
    // PSC = 15 -> 16 МГц / (15+1) = 1 МГц
    // (стр. 364) PSC - предделитель
    TIM2->PSC = 15; //TIM2
    // ARR = (16 МГц / 16 / freq) - 1
    // (стр. 364) ARR - период, до сколька считает счетчик
    TIM2->ARR = (16000000 / 16 / freq) - 1;
}

// funktion for on/off PWM
void PWM_Toggle(int state) {
    // if state = 1 -> CC1E = 1 (выход включен, сигнал идет на пин)
    // if state = 0 -> CC1E = 0 (выход выключен, пин в покое)
    // (стр. 362) CC1E - bit 0 в CCER, разрешает выход канала
    if (state) TIM2->CCER |= (0x1 << 0);  //TIM2
    else TIM2->CCER &= ~(0x1 << 0);       
}