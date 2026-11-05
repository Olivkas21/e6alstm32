#include "stm32f4xx.h"
#include "button.h"
#include "dma.h"

//init button for DMA
void Button_Init(void) {
    // Привязываем EXTI13 к порту C.
    // В регистре EXTICR[3] для линии 13 используется поле [7:4] (сдвиг 4).
    SYSCFG->EXTICR[3] &= ~(0xF << 4); // Очищаем
    SYSCFG->EXTICR[3] |=  (0x2 << 4); // 0x2 = Порт C (PC13)

    // Настраиваем на спад (задний фронт) для линии 13
    EXTI->FTSR |= (0x1 << 13);

    // Разрешаем прерывание на линии 13
    EXTI->IMR  |= (0x1 << 13); // [ИЗМЕНЕНО МНОЙ] Исправлена опечатка EXT1 -> EXTI

    // Включаем в NVIC
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void EXTI15_10_IRQHandler(void) {
    // [ИЗМЕНЕНО МНОЙ] see stop ot pin 13 (стр. 211) (так как используем PC13)
    if (EXTI->PR & (0x1 << 13)) {
        // flag -> 1  (need 500)
        EXTI->PR |= (0x1 << 13); 

        // start DMA
        DMA_Start(); // [ИЗМЕНЕНО МНОЙ] Вызов функции запуска DMA
    }
}