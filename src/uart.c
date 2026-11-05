#include "stm32f4xx.h"
#include "uart.h"
#include "ring_buffer.h"

//usart funktion
void USART2_Init(void) {
    //USARTDIV = fCK / (16 * BaudRate); 16 000 000 / (16 * 9600) = 104.1666
    //104 = x68 | 0.1666 * 16 = 2.66 -> 3 
    USART2->BRR = 0x683;
    // Разрешаем прерывание по приему (когда придет байт от ПК)
    // bit 5 (RXNEIE = 1) (стр. 549) //прерыв
    USART2->CR1 |= (0x1 << 5);
    // Вкл сам USART, in & out:
    // Бит 13 (UE=1) - Вк USART (стр. 548)
    // Бит 3 (TE=1)  - Вкл передатчик TX
    // Бит 2 (RE=1)  - Вкл приемник RX
    USART2->CR1 |= (0x1 << 13) | (0x1 << 2) | (0x1 << 3);
    // Включаем прерывание USART2 в контроллере NVIC
    NVIC_EnableIRQ(USART2_IRQn);
}

// fynk go out 1 biet (wiht use flag dont stop)
void USART2_SendChar(uint8_t c) {
    // bit 7 в SR (Status Register) - its TXE (Transmit Data Register Empty).
    // if 0 -> last biet now go out, were spot go on (стр. 545)
    while (!(USART2->SR & (0x1 << 7))); 
    
    // регистр пусто -> кладем новый байт в DR
    // use TX (стр. 547)
    USART2->DR = c; 
}

//auto funktion when biet go here
void USART2_IRQHandler(void) {
    // bit 5 in SR (Status Register) -  RXNE (Receive Not Empty)
    // if 1 -> DR new biet (стр. 545)
    if (USART2->SR & (0x1 << 5)) {
        // read biet DR (стр. 547)
        // if we read biet -> flag RXNE -> 0
        uint8_t data = USART2->DR; 
        
        // give biet in kolce buff
        buffer_add(data);
    }
}