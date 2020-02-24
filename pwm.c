#include "pwm.h"

const float pwm_prescaler[4] = {1.0, 4.0, 16.0, 64.0};

/**
* Initialize the PWM.
*/
void init_PWM(float fpwm, int duty[]){
        
    int index_pre = get_pwm_prescaler(fpwm);
    
    PTCONbits.PTOPS = 0; // Post Scaler
    PTCONbits.PTCKPS = index_pre; // Pre Scaler, lo calcolo con get_pwm_prescaler()
    
    PTCONbits.PTMOD = 0; // Free Running mode, PTMR = PTPER --> restart 
            
    PTCONbits.PTEN = 1; // Start PWM
    
    update_pwm(duty); // Tramite questa funzione calcolo PDC1 e PDC2
}


int get_pwm_prescaler(double fpwm)
{
    // Come il prescaler del timer, ma con prescaler {1, 4, 16, 64}
    // Si basa sulla formula: PTPER = (Fcy/(Fpwm*Prescaler) - 1
    uint32_t ptper = 0;
    int i;
    for (i = 0; i < 4; i++){
        ptper = (SOURCE_OSCILLATOR_FREQUENCY/4)/(fpwm*pwm_prescaler[i])- 1;
        if(ptper < UINT16_MAX){
            PTPER = ptper;
            return i;
        }
    }   
    return -1; 
}

void update_pwm(int duty[]){
    PDC1 = round(duty[0] / 50.0 * PTPER); // Deriva dalla proporzione: duty[x] : PDCx = 100.0 : 2*PTPER
    PDC2 = round(duty[1] /50.0 * PTPER);  // PDCx = (duty[x] * 2* PTPER) / 100.0
}

int round(double x){
    return (int) x+0.5f; // Mi dava problemi la round, allora l'ho implementata
}