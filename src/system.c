#include "stm32f4xx.h"
#include "system.h"

void System_Init(void) {
    //вкл RCC
    // порт A для PA2 и PA3 для USART2
    // порт C для PC6 для ШИМ и PC13 для кнопки (изменено)
    // вкл DMA1
    // RCC->AHB1ENR: биты 0 (GPIOAEN), 2 (GPIOCEN), 21 (DMA1EN) (стр. 118)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_DMA1EN;
    
    // for test
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(0x3 << (5 * 2));
    GPIOA->MODER |=  (0x1 << (5 * 2));
    // PA5 = выход

    // TIM3 (ШИМ) и USART2 (UART): биты 1 (TIM3EN), 17 (USART2EN) в APB1ENR (стр. 119)
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN | RCC_APB1ENR_USART2EN;

    // SYSCFG привязка кнопки к EXTI: бит 14 в APB2ENR (стр. 122)
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    //PC6 ШИМ, TIM3_CH1
    // MODER: биты 12-13 для пина 6. x3 = 11, ~ 00, -> cl (стр. 158)
    GPIOC->MODER &= ~(0x3 << 12);
    // x2 = 10 aльт функция (стр. 158)
    GPIOC->MODER |=  (0x2 << 12); 

    // AFR[0]: биты 24-27 (4 bit). xf = 1111, ~xf 0000 cl (стр. 162)
    GPIOC->AFR[0] &= ~(0xF << 24);
    // 0x2 = AF2 (для TIM3_CH1 used pc6) (стр. 162)
    GPIOC->AFR[0] |=  (0x2 << 24);

    // OSPEEDR: bit 12-13 for pin 6. x3 = 11 more speed (стр. 159)
    GPIOC->OSPEEDR |= (0x3 << 12);

    //PA2 (TX) и PA3 (RX) для USART2 
    // MODER: PA2 4, PA3 6 (стр. 158)
    // cl 00
    GPIOA->MODER &= ~( (0x3 << 4) | (0x3 << 6) );
    // x2 aльт ф(стр. 158)
    GPIOA->MODER |=  ( (0x2 << 4) | (0x2 << 6) ); 

    // AFR[0]: PA2 8, PA3 12 (стр. 162) ~f - ~1111
    GPIOA->AFR[0] &= ~( (0xF << 8) | (0xF << 12) );
    // x7 = AF7 (для USART2) (стр. 162)
    GPIOA->AFR[0] |=  ( (0x7 << 8) | (0x7 << 12) );

    //PC13 buton (изменено)
    // MODER: биты 26-27 (13 * 2 = 26). 0x3 = 11 ~ 00 (Вход)
    GPIOC->MODER &= ~(0x3 << 26); 

    // PUPDR: биты 26-27. 0x1 = 01 (Pull-Up)
    GPIOC->PUPDR &= ~(0x3 << 26); 
    GPIOC->PUPDR |=  (0x1 << 26); 

        // --- НАСТРОЙКА PA5 (встроенный светодиод LD2, TIM2_CH1) ---
    // Включаем тактирование TIM2 (бит 0 в APB1ENR) (стр. 119)
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // PA5 - это TIM2_CH1, Альтернативная функция 1 (AF1)
    // MODER: биты 10-11 (5 * 2 = 10). 0x3 = 11 ~ 00, -> 0x2 (AF) (стр. 158)
    GPIOA->MODER &= ~(0x3 << (5 * 2));
    GPIOA->MODER |=  (0x2 << (5 * 2)); 
    
    // AFR[0]: биты 20-23 (5 * 4 = 20). 0x1 = AF1 (стр. 162)
    GPIOA->AFR[0] &= ~(0xF << (5 * 4));
    GPIOA->AFR[0] |=  (0x1 << (5 * 4)); 
    
    // OSPEEDR: биты 10-11. 0x3 = 11 (высокая скорость) (стр. 159)
    GPIOA->OSPEEDR |= (0x3 << (5 * 2));
}