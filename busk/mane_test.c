#include "stm32f4xx.h"
#define BUF_SIZE 256  //size buff

// [] for DMA var 2: ot 127 do 0 (128 biet), next ot 255 do 0 (256 biet)
// finish 384 biet.
uint8_t dma_data[384]; 

// our buf
uint8_t rx_buffer[BUF_SIZE];
//place for put new biet
volatile uint16_t rx_head = 0;

// *place for take new bite for main
volatile uint16_t rx_tail = 0; 

//funktion for add biet on buff
void buffer_add(uint8_t data) {
    // if head = 255 -> 255+1=256, 256 % 256 = 0 go zero
    uint16_t next = (rx_head + 1) % BUF_SIZE;

    // if next_head == rx_tail, buff full
    //ignor new biet
    if (next != rx_tail) { 
        rx_buffer[rx_head] = data; // put biet in []
        rx_head = next;       // * -> next
    }
}


// funktion for give biet on buff (only use in main)
uint8_t buffer_get(void) {
    // if head == tail -> buff the void
    if (rx_head == rx_tail) {
        return 0; // заглушкa
    }
    
    uint8_t data = rx_buffer[rx_tail]; // take the first biet
    rx_tail = (rx_tail + 1) % BUF_SIZE; // tail go to kolco
    return data;
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


// фун шим TIM3
void PWM_Init(void) {
    // Предделитель: 16 МГц / (PSC+1) = 10 кГц (стр. 364)
    TIM3->PSC = 1599; 
    
    // Период: 10 кГц / (ARR+1) = 10 Гц (стр. 364)
    TIM3->ARR = 999;  
    
    // скважность 50%: CCR1 = 500 из 1000 (стр. 365)
    TIM3->CCR1 = 500; 

    // бит 3 (OC1PE=1): предзагрузкa CCR1 // от гличей (стр. 358)
    TIM3->CCMR1 |= (0x1 << 3);
    
    // биты 4-6 (OC1M=110): режим PWM Mode 1 (стр. 358)
    // ~x7 = ~111, сler, -> x6 = 110
    TIM3->CCMR1 &= ~(0x7 << 4);
    TIM3->CCMR1 |=  (0x6 << 4);

    // bit 0 | 1: для передачи сигнала на пин PC6 (стр. 362)
    TIM3->CCER |= (0x1 << 0);

    // bit 0 | 1 : запуск PSC, ARR, CCR1 сразу (стр. 357)
    TIM3->EGR |= (0x1 << 0);

    // Бит 0 |1: запуск счёт(стр. 350)
    TIM3->CR1 |= (0x1 << 0);
}

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

//init button for DMA
void Button_Init(void) {
    // Привязываем EXTI13 к порту C.
    // В регистре EXTICR[3] для линии 13 используется поле [7:4] (сдвиг 4).
    SYSCFG->EXTICR[3] &= ~(0xF << 4); // Очищаем
    SYSCFG->EXTICR[3] |=  (0x2 << 4); // 0x2 = Порт C (PC13)

    // Настраиваем на спад (задний фронт) для линии 13
    EXTI->FTSR |= (0x1 << 13);

    // Разрешаем прерывание на линии 13
    EXTI->IMR  |= (0x1 << 13); 

    // Включаем в NVIC
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void EXTI15_10_IRQHandler(void) {
    // see stop ot pin 12 (стр. 211)
    if (EXTI->PR & (0x1 << 13)) {
        // flag -> 1  (need 500)
        EXTI->PR |= (0x1 << 13); 

        // start DMA
        // reload if schetchik work before
        DMA1_Stream6->CR &= ~(0x1 << 0); 
        while (DMA1_Stream6->CR & (0x1 << 0)); 
        
        DMA1_Stream6->NDTR = 384; // back schetchik
        DMA1_Stream6->CR |= (0x1 << 0); // go stream
    }
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


//мейн
int main(void) {

    //вкл RCC
    // порт A для PA2 и PA3 для USART2
    // порт C для PC6 для ШИМ и PC12 для кнопки
    // вкл DMA1
    // RCC->AHB1ENR: биты 0 (GPIOAEN), 2 (GPIOCEN), 21 (DMA1EN) (стр. 118)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_DMA1EN;
     // for test
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(0x3 << (5 * 2));
    GPIOA->MODER |=  (0x1 << (5 * 2));
     // PA5 = выход

     // MODER: биты 26-27 (13 * 2 = 26). 0x3 = 11 ~ 00 (Вход)
    GPIOC->MODER &= ~(0x3 << 26); 

    // PUPDR: биты 26-27. 0x1 = 01 (Pull-Up)
    GPIOC->PUPDR &= ~(0x3 << 26); 
    GPIOC->PUPDR |=  (0x1 << 26); 
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

    //PC12 buton
    // MODER: bit 24-25 pin 12 . 0x3 = 11 ~ 00 (Вход) (стр. 158)
    GPIOC->MODER &= ~(0x3 << 24); 
    
    // PUPDR: биты 24-25 для пина 12 (12*2=24). 0x1 = 01 (Pull-Up) (стр. 159)
    GPIOC->PUPDR &= ~(0x3 << 24); // clear (00)
    GPIOC->PUPDR |=  (0x1 << 24); // 0x1 = 01 (Подтяжка к питанию)

    //init shim and usart
    PWM_Init();
    __enable_irq(); //глобальные прерывания
    USART2_Init();
    DMA_Init();
    Button_Init();

    USART2_SendChar('1');
    USART2_SendChar('4');
    USART2_SendChar('8');
    USART2_SendChar('8');
    USART2_SendChar(' '); USART2_SendChar('^'); USART2_SendChar('-'); USART2_SendChar('^'); USART2_SendChar('/');
    USART2_SendChar('\r');
    USART2_SendChar('\n');
    
    /* for (volatile int i = 0; i < 4000000; i++);

    // ПРИНУДИТЕЛЬНО ЗАПУСКАЕМ DMA (без кнопки)
    DMA1_Stream6->NDTR = 384;     // Восстанавливаем счетчик
    DMA1_Stream6->CR |= (0x1 << 0); // Включаем!*/
    // main in main
     while (1) {
        if (rx_head != rx_tail) {
            uint8_t c = buffer_get(); 
            
            // ОБРАБОТКА КОМАНД (Вариант 2)
            if (c == 0xAC) { 
                // Команда 0xAC: Запуск DMA
                DMA1_Stream6->CR &= ~(0x1 << 0);
                while (DMA1_Stream6->CR & (0x1 << 0));
                DMA1_Stream6->NDTR = 384;
                DMA1_Stream6->CR |= (0x1 << 0);
            }
            else if (c == 0xAB) {
                // Команда 0xAB: Остановка DMA
                DMA1_Stream6->CR &= ~(0x1 << 0);
            }
            else {
                // Эхо для остальных команд
                while (!(USART2->SR & (0x1 << 7))); 
                USART2->DR = c; 
            }
        }
    }
}