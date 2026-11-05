#include "stm32f4xx.h"
#include "dma.h"

// [] for DMA var 2: ot 127 do 0 (128 biet), next ot 255 do 0 (256 biet)
// finish 384 biet.
uint8_t dma_data[384];

void DMA_Init(void) {
    // go put data in []
    for (int i = 0; i < 128; i++) dma_data[i] = 127 - i;      // 127, 126... 0
    for (int i = 0; i < 256; i++) dma_data[128 + i] = 255 - i; // 255, 254... 0

    //off stream
    DMA1_Stream6->CR &= ~(0x1 << 0); 
    while (DMA1_Stream6->CR & (0x1 << 0)); 

    // adres registr of data USART2 (стр. 194)
    DMA1_Stream6->PAR = (uint32_t)&(USART2->DR); 

    //adres [] (стр. 195)
    DMA1_Stream6->M0AR = (uint32_t)dma_data; 

    // count data biet (стр. 194)
    DMA1_Stream6->NDTR = 384; 

    // option register (стр. 191)
    // CHSEL = 4 (chanal 4). DIR = 01 (Mem-To-Periph). CIRC = 1 (Кольцо). MINC = 1 (Инкремент памяти).
    DMA1_Stream6->CR = (0x4 << 25) | (0x1 << 6) | (0x1 << 8) | (0x1 << 10);

    //online USART2 for usles DMA (bit 7 в CR3 - DMAT) (стр. 552)
    USART2->CR3 |= (0x1 << 7); 

    //DMA work when we take button
}

// [ИЗМЕНЕНО МНОЙ] Вынес запуск в функцию, чтобы вызывать и по кнопке, и по UART
void DMA_Start(void) {
    // reload if schetchik work before
    DMA1_Stream6->CR &= ~(0x1 << 0); 
    while (DMA1_Stream6->CR & (0x1 << 0)); 
    
    DMA1_Stream6->NDTR = 384; // back schetchik
    DMA1_Stream6->CR |= (0x1 << 0); // go stream
}

// [ИЗМЕНЕНО МНОЙ] Функция остановки DMA
void DMA_Stop(void) {
    DMA1_Stream6->CR &= ~(0x1 << 0);
}