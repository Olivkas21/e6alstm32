#include "stm32f4xx.h"
#include "system.h"
#include "ring_buffer.h"
#include "pwm.h"
#include "uart.h"
#include "dma.h"
#include "button.h"

// switch
#define RUN_LAB 6 

int main(void) {
    System_Init(); //RCC, GPIO + button PC13
    __enable_irq(); //nado prosto vkluchit preryvaniya, chtoby rabotalo DMA and UART

    #if RUN_LAB == 4
        // lb4
        // need to do shim + 5% 
        PWM_Init();
        
        uint8_t duty_lab4 = 0;
        while(1) {
            PWM_SetDuty(duty_lab4);
            duty_lab4 += 1;
            if (duty_lab4 > 50) duty_lab4 = 0;
            
            //задержка для 16 МГц
            for(volatile int i = 0; i < 16000000; i++); 
        }

        #elif RUN_LAB == 5
        // lb5 UART
        PWM_Init();
        USART2_Init();

        uint8_t packet5[8];
        uint8_t idx5 = 0;

        USART2_SendChar('S'); USART2_SendChar('T'); USART2_SendChar('A'); USART2_SendChar('R'); USART2_SendChar('T'); USART2_SendChar('\r'); USART2_SendChar('\n');

        while(1) {
            if (rx_head != rx_tail) {
                uint8_t c = buffer_get(); 
                
                if (idx5 == 0 && c == 0x7C) {
                    packet5[0] = c;
                    idx5 = 1;
                } 
                else if (idx5 > 0) {
                    packet5[idx5++] = c;
                    
                    if (idx5 >= 8) { // need 8 biet
                        idx5 = 0; 
                        
                        // 1. check XOR
                        uint8_t xor_sum = 0;
                        for(int i = 1; i < 7; i++) xor_sum ^= packet5[i];
                        
                        // 2. check in cmd: paket her? and ok XOR or not
                        USART2_SendChar('P'); // Packet received
                        if (xor_sum == packet5[7]) {
                            USART2_SendChar('+'); // XOR OK
                            
                            uint8_t cmd = packet5[1];
                            if (cmd == 0x02) {
                                // nomber skvagnosti
                                USART2_SendChar('D');
                                USART2_SendChar('=');
                                // nombers 0 - 100
                                if (packet5[2] >= 100) {
                                    USART2_SendChar('1'); USART2_SendChar('0'); USART2_SendChar('0');
                                } else {
                                    USART2_SendChar('0' + (packet5[2] / 10) % 10);
                                    USART2_SendChar('0' + packet5[2] % 10);
                                }
                                USART2_SendChar('\r'); USART2_SendChar('\n');
                                
                                // ВЫЗОВ ШИМ
                                PWM_SetDuty(packet5[2]);
                            }
                            
                            // eho (8 biet)
                            for(int i=0; i<8; i++) USART2_SendChar(packet5[i]);
                        } else {
                            USART2_SendChar('-'); // XOR FAIL
                            USART2_SendChar('\r'); USART2_SendChar('\n');
                        }
                    }
                }
            }
        }
    #elif RUN_LAB == 6
        //lr6
        //lr5+ DMA (0xAC/0xAB) + button
        PWM_Init();
        USART2_Init();
        DMA_Init();
        Button_Init();

        uint8_t packet6[7];
        uint8_t idx6 = 0;

        while (1) {
            if (rx_head != rx_tail) {
                uint8_t c = buffer_get(); 
                
                if (idx6 == 0 && c == 0x7C) {
                    packet6[0] = c;
                    idx6 = 1;
                } 
                else if (idx6 > 0) {
                    packet6[idx6++] = c;
                    
                    if (idx6 >= 7) {
                        idx6 = 0; 
                        
                        uint8_t xor_sum = 0;
                        for(int i = 1; i < 6; i++) xor_sum ^= packet6[i];
                        
                        if (xor_sum == packet6[6]) {
                            uint8_t cmd = packet6[1];
                            
                            if (cmd == 0xAC) { 
                                // comand to start DMA (with not use eho)
                                DMA_Start();
                            }
                            else if (cmd == 0xAB) {
                                // comand to stop DMA (with eho)
                                DMA_Stop();
                                for(int i=0; i<7; i++) USART2_SendChar(packet6[i]);
                            }
                            else if (cmd == 0x01) { 
                                uint32_t freq = (packet6[2]<<24)|(packet6[3]<<16)|(packet6[4]<<8)|packet6[5];
                                PWM_SetFrequency(freq);
                                for(int i=0; i<7; i++) USART2_SendChar(packet6[i]);
                            }
                            else if (cmd == 0x02) { 
                                PWM_SetDuty(packet6[2]);
                                for(int i=0; i<7; i++) USART2_SendChar(packet6[i]);
                            }
                            else if (cmd == 0x03) { 
                                PWM_Toggle(packet6[2] & 0x10); 
                                for(int i=0; i<7; i++) USART2_SendChar(packet6[i]);
                            }
                            else {
                                // eho fom more comand
                                for(int i=0; i<7; i++) USART2_SendChar(packet6[i]);
                            }
                        }
                    }
                }
            }
        }
    #endif
}