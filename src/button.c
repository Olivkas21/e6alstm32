#include "stm32f4xx.h"
#include "button.h"
#include "dma.h"

//init button for DMA
void Button_Init(void) {
    // EXTI13 with port C.
    //EXTICR[3] for line 13 use [7:4] (dvigaem 4).
    SYSCFG->EXTICR[3] &= ~(0xF << 4); // cl 0000
    SYSCFG->EXTICR[3] |=  (0x2 << 4); // x2 = 0010, port C (PC13)

    // last front for line 13
    EXTI->FTSR |= (0x1 << 13);

    // Разрешаем прерывание на линии 13
    EXTI->IMR  |= (0x1 << 13);
    
    // Включаем в NVIC
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void EXTI15_10_IRQHandler(void) {
    //see stop ot pin 13 (стр. 211) (buton PC13)
    if (EXTI->PR & (0x1 << 13)) {
        // flag -> 1  (need obnul)
        EXTI->PR |= (0x1 << 13); 

        // start DMA
        DMA_Start(); // call function DMA
    }
}