// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef LCD_HEADER_TEMPLATE_H
#define	LCD_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.

#define FIRST_ROW 1
#define SECOND_ROW 2

void init_SPI();

void put_char_SPI(char c);

void write_string_LCD(char *str, int row, int column);

void move_cursor_to(int row, int column);

void clear_LCD_row(int row);

void clear_LCD();


#endif	/* LCD_HEADER_TEMPLATE_H */


