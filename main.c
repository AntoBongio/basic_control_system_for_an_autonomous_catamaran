// DSPIC30F4011 Configuration Bit Settings

// 'C' source line config statements

// FOSC
#pragma config FPR = XT                 // Primary Oscillator Mode (XT)
#pragma config FOS = PRI                // Oscillator Source (Primary Oscillator)
#pragma config FCKSMEN = CSW_FSCM_OFF   // Clock Switching and Monitor (Sw Disabled, Mon Disabled)

// FWDT
#pragma config FWPSB = WDTPSB_16        // WDT Prescaler B (1:16)
#pragma config FWPSA = WDTPSA_512       // WDT Prescaler A (1:512)
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

// FBORPOR
#pragma config FPWRT = PWRT_64          // POR Timer Value (64ms)
#pragma config BODENV = BORV20          // Brown Out Voltage (Reserved)
#pragma config BOREN = PBOR_ON          // PBOR Enable (Enabled)
#pragma config LPOL = PWMxL_ACT_HI      // Low-side PWM Output Polarity (Active High)
#pragma config HPOL = PWMxH_ACT_HI      // High-side PWM Output Polarity (Active High)
#pragma config PWMPIN = RST_IOPIN       // PWM Output Pin Reset (Control with PORT/TRIS regs)
#pragma config MCLRE = MCLR_EN          // Master Clear Enable (Enabled)

// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

// FICD
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <p30F4011.h>
#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "lcd.h"
#include "pwm.h"
#include "uart.h"
#include "parser.h"
#include "adc.h"

#define MAX_TASKS 10
#define CONTROL_MODE 0
#define TIMEOUT_MODE 1
#define SAFE_MODE 2
#define FEASABLE_min_TH -8000
#define FEASABLE_max_TH 8000

// struct which identifies elapsed HB and Period (expressed in function of HB) for each task
typedef struct {
int n;
int N;
} heartbeat;

volatile heartbeat schedInfo[MAX_TASKS];
// General variables
volatile int state = CONTROL_MODE;
volatile char to_print_1[20];
volatile char to_print_2[20]; 
volatile char to_print_3[20]; 
volatile char to_print_4[20];
volatile int rpm_l = 1000;
volatile int rpm_r = -7000;
volatile int sat_min = -5000;
volatile int sat_max = 5000;
volatile int pdc1 = 0;
volatile int pdc2 = 0;
volatile int lcd_toggle = 0;
// Needed for the buttons
volatile int previous_s5_state = 1;
volatile int previous_s6_state = 1;
// Needed for the temperature
volatile double temp[10];
volatile int i = 0;
volatile double avg_temp = 0.0;
#define VREFH 5 // Voltage reference High
#define VREFL 0 // Voltage reference Low
// needed for the PWM
volatile int duty[] = {50, 50};
// Needed for the UART TX
volatile CircularBuffer cb;
// Needed for the UART RX
volatile CircularBuffer cb_r;
volatile parser_state ps;


// Saturate function
void saturate(){
    if(rpm_l > sat_max)
        rpm_l = sat_max;
    if(rpm_l < sat_min)
        rpm_l = sat_min;
    
    if(rpm_r > sat_max)
        rpm_r = sat_max;
    if(rpm_r < sat_min)
        rpm_r = sat_min;
}

// Function to eval the duty
void eval_duty(){
    // considero che: rpm in [-10000, 10000] e duty in [0,100]
    // Dalla proporzione: rpm:20000 = duty:100
    // Aggiungo offset 50.0
    duty[0] = round((rpm_l * 100.0) / 20000.0 + 50.0);
    duty[1] = round((rpm_r * 100.0) / 20000.0 + 50.0);
}

// call for saturate()--> eval_duty()--> update_pwm()
void refresh_pwm(){
    saturate(); // Saturo le reference
    eval_duty(); // Calcolo il duty
    update_pwm(duty); // Aggiorno PDC1 e PDC2
}

// Obtain the charater defining each state --> used for the LCD
char from_state_to_char(int state){
    if (state == 0)
        return 'C';
    else if (state == 1)
        return 'T';
    else if (state == 2)
        return 'S';
    else
        return '?';
}

void set_scheduler(){
    
    // I consider an Heartbeat of 20ms
    // n: elapsed HB for task_i()
    // N: Number of HB between two task_i() executions
    schedInfo[0].n = 0; 
    schedInfo[0].N = 5; // task_acquire_temp()
    schedInfo[1].n = 0; 
    schedInfo[1].N = 50; // task_avgT_send()
    schedInfo[2].n = 0;
    schedInfo[2].N = 10; // task_send_fbk()
    schedInfo[3].n = 0; 
    schedInfo[3].N = 5; // task_refresh_pwm()
    schedInfo[4].n = 0; 
    schedInfo[4].N = 1; // task_receive_msg()
    schedInfo[5].n = 0; 
    schedInfo[5].N = 50; // task_blink_D3()
    schedInfo[6].n = 0; 
    schedInfo[6].N = 50; // task_blink_D4()
    schedInfo[7].n = 0; 
    schedInfo[7].N = 10; // task_LCD_write()
    schedInfo[8].n = 0; 
    schedInfo[8].N = 1; // task_check_S5()
    schedInfo[9].n = 0; 
    schedInfo[9].N = 1; // task_check_S6()
}

// Frequency: 10Hz
void task_acquire_temp(){
    
    double raw_value;
    while(!ADCON1bits.DONE); // Attendo fine conversione
    int output_adc = ADCBUF0;
    // Mappo il valore in [0.0,5.0]V 
    raw_value = VREFL + output_adc/1023.0 * (VREFH - VREFL); 
    // raw_value in mV e sottraggo il valore corrispondente a 25C(750mv). 
    // Slope è pari a 10mV/C
    temp[i] = 25.0 + (1000.0 * raw_value - 750.0) / 10.0;
    i = (i+1)%10; // Circular Buffer
}

// Frequency: 1Hz
void task_avgT_send(){

    int j = 0;
    char temp_to_send[15]; // 15 per far entrare il messaggio
    avg_temp = 0.0;
    for(j=0; j<10; j++)
        avg_temp = avg_temp + temp[j];
    avg_temp = avg_temp/10.0;
        
    sprintf(temp_to_send, "$MCTEM,%2.1f*", avg_temp);
    j = 0;
    while(temp_to_send[j] != '\0')
        write_buffer(&cb, temp_to_send[j++]);
}

// Frequency: 5Hz
void task_send_fbk(){

    char to_send[25]; // Dimensione sufficiente per il messaggio
    int j = 0;
    sprintf(to_send, "$MCFBK,%d,%d,%c*", rpm_l, rpm_r, from_state_to_char(state));
    while(to_send[j] != '\0')
        write_buffer(&cb, to_send[j++]); // Scrivo sul buffer circolare usato in trasmissione
}

// Frequency: 10Hz
void task_refresh_pwm(){
    refresh_pwm();
}

// Frequency: 10Hz
void task_receive_msg(){
    char c;
    while(read_to_rx(&cb_r, &c) == 1){
        if (parse_byte(&ps, c) == NEW_MESSAGE){
            
           if(strcmp(ps.msg_type, "HLREF") == 0){
                TMR2 = 0;
                int payload[2];
                sscanf(ps.msg_payload, "%d,%d", &payload[0],  &payload[1]);
                
                if(state != SAFE_MODE){
                   rpm_l = payload[0];
                   rpm_r = payload[1];
                  // refresh_pwm();
                }
                
                if(state == TIMEOUT_MODE){
                    T2CONbits.TON = 1;
                    state = CONTROL_MODE;
                }
            }
           else if(strcmp(ps.msg_type, "HLSAT") == 0){
                int payload[2];
                sscanf(ps.msg_payload, "%d,%d", &payload[0],  &payload[1]);
                
                int min = payload[0];
                int max = payload[1];
                
                if(min >= max || min < FEASABLE_min_TH || max > FEASABLE_max_TH || min > 0 || max < 0){
                    char to_send[20] = "$MCACK,SAT,0*";
                    int i = 0;
                    while(to_send[i] != '\0')
                        write_buffer(&cb, to_send[i++]);
                }
                else{
                    sat_min = min;
                    sat_max = max;
                    refresh_pwm();
                    char to_send[20] = "$MCACK,SAT,1*";
                    int i = 0;
                    while(to_send[i] != '\0')
                        write_buffer(&cb, to_send[i++]);
                }
                
           }
           else if(strcmp(ps.msg_type, "HLENA") == 0){
               
               if (state == SAFE_MODE){
                    TMR2 = 0;
                    T2CONbits.TON = 1;
                    state = CONTROL_MODE; // switch to control mode
                    // motor's rpm to 0
                    rpm_l = 0;
                    rpm_r = 0;
                    refresh_pwm();

                    // Send ack message
                    char to_send[15];
                    sprintf(to_send, "$MCACK,ENA,1*");

                    int i = 0;
                    while(to_send[i] != '\0')
                       write_buffer(&cb, to_send[i++]);
               }
           }
        }
    }
}

// Frequency: 1Hz
void task_blink_D3(){
    LATBbits.LATB0 = !LATBbits.LATB0;   
}

// Frequency: 1Hz
void task_blink_D4(){
    if (state == TIMEOUT_MODE)
        LATBbits.LATB1 = !LATBbits.LATB1;
    else
        LATBbits.LATB1 = 0;
}

// Frequency: 5Hz
void task_LCD_write(){
    
    if (lcd_toggle == 0){
        sprintf(to_print_1, "STA: %c, T:%2.1fC", from_state_to_char(state), avg_temp);
        sprintf(to_print_2, "L:%d, R:%d", rpm_l, rpm_r);
        clear_LCD_row(FIRST_ROW);
        write_string_LCD(to_print_1, FIRST_ROW, 0);
        clear_LCD_row(SECOND_ROW);
        write_string_LCD(to_print_2, SECOND_ROW, 0);
    }
    else{
        sprintf(to_print_3, "s:%d, S:%d", sat_min, sat_max);
        sprintf(to_print_4, "RPM: %d, %d", PDC1, PDC2);
        clear_LCD_row(FIRST_ROW);
        write_string_LCD(to_print_3, FIRST_ROW, 0);
        clear_LCD_row(SECOND_ROW);
        write_string_LCD(to_print_4, SECOND_ROW, 0);
    }
}

// Frequency: 50Hz
void task_check_S5(){
    // Read button S5
    int current_s5_state = PORTEbits.RE8;
    // check raising signal
    if(previous_s5_state && current_s5_state == 0){
        state = SAFE_MODE;
        rpm_l = 0;
        rpm_r = 0;
        refresh_pwm();
        T2CONbits.TON = 0;
        TMR2 = 0;
    }
    previous_s5_state = current_s5_state;
}

// Frequency: 50Hz
void task_check_S6(){
    // Read button S6
    int current_s6_state = PORTDbits.RD0;
    // check raising signal
    if(previous_s6_state && current_s6_state == 0){
        // LCD toggle
        lcd_toggle = (lcd_toggle + 1)%2;
    }
    previous_s6_state = current_s6_state;
}

void scheduler() {
    int i ;
    for (i = 0; i < MAX_TASKS; i++) {
        schedInfo[i].n++;
        if (schedInfo[i].n == schedInfo[i].N) {
            switch(i) {
                case 0:
                    task_acquire_temp();
                    break;
                case 1:
                    task_avgT_send();
                    break;
                case 2:
                    task_send_fbk() ;
                    break;
                case 3:
                    task_refresh_pwm() ;
                    break;
                case 4:
                    task_receive_msg() ;
                    break;
                case 5:
                    task_blink_D3() ;
                    break;
                case 6:
                    task_blink_D4() ;
                    break;
                case 7:
                    task_LCD_write() ;
                    break;
                case 8:
                    task_check_S5() ;
                    break;
                case 9:
                    task_check_S6() ;
                    break;
            }
        schedInfo[i].n = 0;
        }
    }
}

// TIMEOUT_MODE interrupt
void __attribute__((__interrupt__, __auto_psv__)) _T2Interrupt() { 
    
    // Stop timer
    T2CONbits.TON = 0;
    // Reset timer 2 flag
    IFS0bits.T2IF = 0;
    // Change state to timeout
    state = TIMEOUT_MODE;
    rpm_l = 0;
    rpm_r = 0;
    refresh_pwm();
}

void __attribute__ (( __interrupt__ , __auto_psv__ ) ) _U2TXInterrupt() {
    
    
    IFS1bits.U2TXIF = 0;
    char c;
    while(U2STAbits.UTXBF == 0){ // check se il buffer della UART ha spazio
        if (read_buffer(&cb, &c) == 1)
            U2TXREG = c;
        else
            break;
    }
}

void __attribute__ (( __interrupt__ , __auto_psv__ ) ) _U2RXInterrupt() {
    
    IFS1bits.U2RXIF = 0;
    
    if(U2STAbits.OERR == 0){
        while(U2STAbits.URXDA == 1){ // Finchè c'è roba da leggere
            write_to_rx(&cb_r, U2RXREG);
        }        
    }
    else
        U2STAbits.OERR = 0; //If error --> reset to 0, this will clear 
}


int main() {
    
    // Procedure to initialize SPI
    tmr_setup(1, 1000);
    T1CONbits.TON = 1; 
    tmr_wait(1);
    init_SPI();
    
    // Set input port for buttons
    TRISDbits.TRISD0 = 1;
    TRISEbits.TRISE8 = 1;
    
    // Set led D3 and D4
    TRISBbits.TRISB0 = 0; // (led D3)
    TRISBbits.TRISB1 = 0; // (led D4)
    
    // timer for TIMEOUT_MODE e interrupt
    tmr_setup(2, 5000);
    IEC0bits.T2IE = 1;
    
    // Init ADC
    init_adc();
    
    // Init PWM
    init_PWM(1000, duty);
    
    // Init UART
    init_uart(4800);
    
    // Set Heartbeat (HB)
    tmr_setup(1, 20);  
    
    // Init for the scheduler
    set_scheduler();
    
    // Initial setup for circular buffers
    cb.count = 0;
    cb_r.count = 0;
    cb.readIndex = 0;
    cb_r.readIndex = 0;
    cb.writeIndex = 0;
    cb_r.writeIndex = 0;
    
    // Parser initialization
	ps.state = STATE_DOLLAR;
	ps.index_type =  0; 
	ps.index_payload =  0;
    
    // Enable interrupt for UART in TX and RX
    IEC1bits.U2TXIE = 1; // interrupt in trasmissione
    IEC1bits.U2RXIE = 1; // interrupt in ricezione
    
    // Last thing: turn on timers for scheduling and timeout mode
    T2CONbits.TON = 1; 
    T1CONbits.TON = 1;
    while(1){
        scheduler();
        tmr_wait(1);
    }
        
    return 0;
}