
#include <p30F4011.h>

#include "adc.h"

void init_adc(){
    
    // <-- ADPCFG da settare???
    
    
    ADCON1bits.SSRC = 7; // Conversion starts immediately after sampling
    ADCON1bits.ASAM = 1; // Sampling begins immediately after last conversion completes.
    ADCON2bits.CHPS = 0; // Selects Channels CH0
    
    // Consider I acquire T at 10 Hz --> I want to obtain a value of T in less than 100 ms
    // Tcy = 1 / (7372800 / 4 MHz ) ~= 0.542 us
    // ADCS --> Tad = 32 * Tcy ~= 17.34 us ==> ADCS == 63 (Tcy/2 * (ADCS+1))
    // Tconv = 12*Tad = 6.504 us; Tsamp = 31 * Tad = 537.54 us; f_samp = 1/(Tsamp+Tconv) = 1.838 kHz
    ADCON3bits.ADCS = 63; // A/D Conversion Clock Select bits 
    ADCON3bits.SAMC = 31; //  Auto-Sample Time bits, 31 * Tad --> ~= 537.54 us

    // ADCHS: selects the inputs to the channels  
    // Set AN2 as input --> Termometer
    ADCHSbits.CH0NA = 0; // Negative input: VREFL for Channel 0
    ADCHSbits.CH0SA = 3; // Positive input: AN2 for Channel 0
    
    ADCON1bits.ADON = 1; // Converter module is operating
}


