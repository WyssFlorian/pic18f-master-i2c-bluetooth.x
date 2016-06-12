#include <xc.h>
#include "test.h"


/**
 * Initialise le hardware pour l'émetteur.
 */
static void recepteurInitialiseHardware() {  // à compléter/contrôler
    
    //configuration de la eusart
    RCSTA1bits.SPEN = 1; //active la eusart
    TXSTA1bits.TXEN = 1; //active l'émetteur
    TXSTA1bits.SYNC = 0; //mode asynchrone
	
    //avec une horloge interne à 1MHZ, cela fait 1200 bauds
	SPBRG1 = 12;
    SPBRGH = 0;
    
    // Configure RC6 et RC7 commen entrée digitales pour que la EUSART 
    // puisse les contrôler.
	TRISCbits.RC6 = 1;
    TRISCbits.RC7 = 1;

    // Prépare le temporisateur 1
    T1CONbits.TMR1CS = 0;       // Source est FOSC/4
    T1CONbits.T1CKPS = 2;       // Diviseur de fréquence 1:4, égale à TMR2.
    T1CONbits.T1RD16 = 1;       // Temporisateur de 16 bits.
    T1CONbits.TMR1ON = 1;       // Active le temporisateur.
    
    // Configure PWM 1 pour réceptionner le signal de radio-contrôle:
    ANSELCbits.ANSC2 = 0;
    TRISCbits.RC2 = 0;

    // Active les interruptions générales:
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
}

/**
 * Point d'entrée pour l'émetteur de radio contrôle.
 */

/**
 * Point d'entrée des interruptions basse priorité.
 */
void recepteurInterruptions() {
    
}

void recepteurMain(void) {
    recepteurInitialiseHardware();
    pwmReinitialise();

    while(1);
}
