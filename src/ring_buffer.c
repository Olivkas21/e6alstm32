#include "ring_buffer.h"

// our byf
uint8_t rx_buffer[BUF_SIZE];
//place for put new biet
volatile uint16_t rx_head = 0;

// *place for take new bite for main
volatile uint16_t rx_tail = 0; 

//funktion for add biet on byff
void buffer_add(uint8_t data) {
    // if head = 255 -> 255+1=256, 256 % 256 = 0 go zero
    uint16_t next = (rx_head + 1) % BUF_SIZE;

    // if next_head == rx_tail, byff full
    //ignor new biet
    if (next != rx_tail) { 
        rx_buffer[rx_head] = data; // put biet in []
        rx_head = next;       // * -> next
    }
}

// funktion for give biet on byff (only use in main)
uint8_t buffer_get(void) {
    // if head == tail -> buff the void
    if (rx_head == rx_tail) {
        return 0; // chtobi bilo
    }
    
    uint8_t data = rx_buffer[rx_tail]; // take the first biet
    rx_tail = (rx_tail + 1) % BUF_SIZE; // tail go to konech
    return data;
}