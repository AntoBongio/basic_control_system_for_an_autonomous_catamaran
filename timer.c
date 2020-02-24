#include "timer.h"

const float prescaler[4] = {1.0, 8.0, 64.0, 256.0};

void from_sec_to_prescaler(int ms, int *tckps, int *pr)
{
    uint32_t fcy = SOURCE_OSCILLATOR_FREQUENCY/4;
    int i = 0;
    while(fcy > UINT16_MAX)
    {
        fcy = (SOURCE_OSCILLATOR_FREQUENCY/4.0)/prescaler[i]*(ms/1000.0);
        if(fcy < UINT16_MAX)
        {
            *pr = fcy;
            *tckps = i;
            return;
        }
        i++;
    }
    *pr = fcy;
    *tckps = i;
    return; 
}
void tmr_setup(int timer, int ms){
    
    int tckps, pr;
    
    from_sec_to_prescaler(ms, &tckps, &pr);
    
    if (timer == TIMER1){
    //  Stop the TIMER
        T1CONbits.TON = 0;
    //  Reset the timer    
        TMR1 = 0;
    //  Set prescaler value
        T1CONbits.TCKPS = tckps;
    //  Period to count
        PR1 = pr;
    //  Reset the interrupt flag    
        IFS0bits.T1IF = 0;
    // Start the TIMER
        //T1CONbits.TON = 1;
    }
    
    if (timer == TIMER2){
        
        T2CONbits.TON = 0;
        TMR2 = 0;
        T2CONbits.TCKPS = tckps;
        PR2 = pr;
        IFS0bits.T2IF = 0;
        //T2CONbits.TON = 1;
    }
    
    if (timer == TIMER3){
        
        T3CONbits.TON = 0;
        TMR3 = 0;
        T3CONbits.TCKPS = tckps;
        PR3 = pr;
        IFS0bits.T3IF = 0;
        //T3CONbits.TON = 1;
    }
    
    if (timer == TIMER4){
        
        T4CONbits.TON = 0;
        TMR4 = 0;
        T4CONbits.TCKPS = tckps;
        PR4 = pr;
        IFS1bits.T4IF = 0;
        //T4CONbits.TON = 1;
    }
    
    if (timer == TIMER5){
        
        T5CONbits.TON = 0;
        TMR5 = 0;
        T5CONbits.TCKPS = tckps;
        PR5 = pr;
        IFS1bits.T5IF = 0;
        //T5CONbits.TON = 1;
    }
    
    return;
}

void tmr_wait(int timer){
    
    if (timer == TIMER1){
        while(IFS0bits.T1IF == 0); // wait timer trigger elapsed time event
        //T1CONbits.TON = 0; // Stop timer 1
        //TMR1 = 0;
        IFS0bits.T1IF = 0; // reset the event flag to 0
    }
    
    if (timer == TIMER2){
        while(IFS0bits.T2IF == 0); // wait timer trigger elapsed time event
        //T2CONbits.TON = 0; // Stop timer 2
        //TMR2 = 0;
        IFS0bits.T2IF = 0; // reset the event flag to 0
    }
    
    if (timer == TIMER3){
        while(IFS0bits.T3IF == 0); // wait timer trigger elapsed time event
        //T3CONbits.TON = 0; // Stop timer 3
        //TMR3 = 0;
        IFS0bits.T3IF = 0; // reset the event flag to 0
    }
    
    if (timer == TIMER4){
        while(IFS1bits.T4IF == 0); // wait timer trigger elapsed time event
        //T4CONbits.TON = 0; // Stop timer 4
        //TMR4 = 0;
        IFS1bits.T4IF = 0; // reset the event flag to 0
    }
    
    if (timer == TIMER5){
        while(IFS1bits.T5IF == 0); // wait timer trigger elapsed time event
        //T5CONbits.TON = 0; // Stop timer 5
        //TMR5 = 0;
        IFS1bits.T5IF = 0; // reset the event flag to 0
    }
    
    return;
}
