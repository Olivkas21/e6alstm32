#ifndef RING_BUFFER_H
#define RING_BUFFER_H
#include <stdint.h>

#define BUF_SIZE 256  //size buff
extern uint8_t rx_buffer[BUF_SIZE]; // our buf
extern volatile uint16_t rx_head; //place for put new biet
extern volatile uint16_t rx_tail; // *place for take new bite for main

void buffer_add(uint8_t data);
uint8_t buffer_get(void);

#endif