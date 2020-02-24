#include "lcd.h"

/**
* Initialize the SPI.
*/
void init_SPI(){
        
    SPI1CONbits.MSTEN = 1; // enable SPI Master Node
    SPI1CONbits.MODE16 = 0; // select 8-bit mode
    SPI1CONbits.PPRE = 3; // primary prescaler --> 1:1
    SPI1CONbits.SPRE = 3; // secondary prescaler --> 1:1
    SPI1STATbits.SPIEN = 1; // enable SPI
}

/**
 * Function to put a character in the transmit buffer SPI1BUF, waiting if it is full
 * @param c Character to put in SPI1BUF
 */
void put_char_SPI(char c){
    while(SPI1STATbits.SPITBF == 1); 
    SPI1BUF = c;
}
/**
 * Function to write str on the LCD. The position is designated by row and column,
 * with this references it is easy to reconstruct the desired address. Keep in mind that 
 * the str is cut off if it overcome the row limit, starting from the initial point.
 * @param str String to write on the LCD
 * @param row Select between FIRST_ROW and SECOND_ROW
 * @param column Select a number between 0 and 15 to choose the column
 */
void write_string_LCD(char *str, int row, int column){
    
    int i;
    move_cursor_to(row, column);
    for (i = 0; str[i] != '\0' && (i+column) < 16; i++){
        put_char_SPI(str[i]);
    }
}

/**
 * Function to move the cursor in the designated point
 * @param row Possible values: {FIRST_ROW, SECOND_ROW}
 * @param column Possible values: {0, ..., 15}
 */
void move_cursor_to(int row, int column){
    
    if (row == FIRST_ROW)
        put_char_SPI(0x80 + column);
    if (row == SECOND_ROW)
        put_char_SPI(0xC0 + column);   
}

/**
 * Function to clear the selected row
 * @param row Select row to clear (FIRST_ROW/SECOND_ROW)
 */
void clear_LCD_row(int row){
    if(row == FIRST_ROW){
        write_string_LCD("                ", FIRST_ROW, 0);
    }
    if(row == SECOND_ROW){
        write_string_LCD("                ", SECOND_ROW, 0);
    }
}

/**
 * Function to entirely clear the LCD
 */
void clear_LCD(){
    write_string_LCD("                ", FIRST_ROW, 0);
    write_string_LCD("                ", SECOND_ROW, 0);
}
