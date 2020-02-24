#ifndef TIMER_HEADER_TEMPLATE_H
#define	TIMER_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  

#define SOURCE_OSCILLATOR_FREQUENCY  7372800
#define TIMER1 1
#define TIMER2 2
#define TIMER3 3
#define TIMER4 4
#define TIMER5 5

void from_sec_to_prescaler(int ms, int *tckps, int *pr);

void tmr_setup(int timer, int ms);

void tmr_wait(int timer);

#endif /*TIMER_HEADER_TEMPLATE_H*/
