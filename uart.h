#ifndef UART_HEADER_TEMPLATE_H
#define	UART_HEADER_TEMPLATE_H 

#define BUFFER_SIZE 60

typedef struct{
    char buffer[BUFFER_SIZE];
    int readIndex;
    int writeIndex;
    int count;
} CircularBuffer;

void init_uart(int baud_rate);

void write_buffer(volatile CircularBuffer* cb, char value);

int read_buffer(volatile CircularBuffer* cb, char*value);

int avl_in_buffer(volatile CircularBuffer* cb);

void write_to_rx(volatile CircularBuffer* circ_buff, char value);

int read_to_rx(volatile CircularBuffer* circ_buff, char* value);

#endif /*UART_HEADER_TEMPLATE_H*/
