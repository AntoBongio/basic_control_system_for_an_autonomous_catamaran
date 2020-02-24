#include <p30F4011.h>

#include "uart.h"
#include "timer.h"

void init_uart(int baud_rate){
    
    U2BRG = (int)(SOURCE_OSCILLATOR_FREQUENCY/4.0/(16.0*baud_rate) - 1); // FCY/(16*BaudRate) - 1 <-- BaudRate = 4800
    //U2BRG = 23;
    U2MODEbits.PDSEL = 0; // 8 bit no parity
    U2MODEbits.STSEL = 0; // 1 stop bit
    U2MODEbits.UARTEN = 1;
    U2STAbits.UTXEN = 1; // enable U2TX
}

void write_buffer(volatile CircularBuffer* circ_buff, char value){
    IEC1bits.U2TXIE = 0;
    circ_buff->buffer[circ_buff->writeIndex] = value; //write the value
    circ_buff->count++;
    circ_buff->writeIndex++;
    if(circ_buff->writeIndex == BUFFER_SIZE) // check if the index is out of bound
        circ_buff->writeIndex = 0;
    IEC1bits.U2TXIE = 1;
    if(circ_buff->count >= 4)  // Scatta interrupt se ho 4 o più char da inviare
        IFS1bits.U2TXIF = 1;
}

int read_buffer(volatile CircularBuffer* circ_buff, char* value){
    if(circ_buff->readIndex == circ_buff->writeIndex){ // if equals, do not read
        IEC1bits.U2RXIE = 1;
        return 0;
    }
    *value = circ_buff->buffer[circ_buff->readIndex]; //read
    circ_buff->count--;
    circ_buff->readIndex++;
    if(circ_buff->readIndex == BUFFER_SIZE) // check if the index is out of bound
        circ_buff->readIndex = 0;
    return 1;
}

// check buffer available space
int avl_in_buffer(volatile CircularBuffer* circ_buff){
    IEC1bits.U2RXIE = 0; // disable interrupt
    int wri = circ_buff->writeIndex;
    int rdi = circ_buff->readIndex;
    IEC1bits.U2RXIE = 1;
    if(wri>=rdi){
        return wri - rdi;
    }
    else{
        return wri - rdi + BUFFER_SIZE;
    }
}

void write_to_rx(volatile CircularBuffer* circ_buff, char value){
    circ_buff->buffer[circ_buff->writeIndex] = value; //write the value
    circ_buff->count++;
    circ_buff->writeIndex++;
    if(circ_buff->writeIndex == BUFFER_SIZE) // check if the index is out of bound
        circ_buff->writeIndex = 0;
}

int read_to_rx(volatile CircularBuffer* circ_buff, char* value){
    IEC1bits.U2RXIE = 0;
    if(circ_buff->readIndex == circ_buff->writeIndex){ // if equals, do not read
        IEC1bits.U2RXIE = 1;
        return 0;
    }
    *value = circ_buff->buffer[circ_buff->readIndex]; //read
    circ_buff->count--;
    circ_buff->readIndex++;
    if(circ_buff->readIndex == BUFFER_SIZE) // check if the index is out of bound
        circ_buff->readIndex = 0;
    IEC1bits.U2RXIE = 1;
    return 1;
}
