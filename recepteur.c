#include <xc.h>
#include "test.h"
#include "recepteur.h"
#include "uart.h"


/**
 * Initialise le hardware pour l'émetteur.
 */
void recepteurInitialiseHardware() {
    
    // Pour une fréquence de 8MHz, ceci donne 9600 bauds :
    TXSTA1bits.BRGH = 1;    // Mode haute vitesse.
    BAUDCON1bits.BRG16 = 1; // Prise en compte du registre SPBRGH.
    SPBRG = 207;            //Baudrate = FOSC / (4 * (N + 1)) = 9615Bauds.
    SPBRGH = 0;

    // Configure RC6 et RC7 comme entrées digitales, pour que
    // la EUSART puisse en prendre le contrôle:
    ANSELCbits.ANSC6 = 0;
    ANSELCbits.ANSC7 = 0;
    TRISCbits.RC6 = 1;
    TRISCbits.RC7 = 1;
   
    // Configure l'EUSART:    
    // (BRGH et BRG16 sont à leur valeurs par défaut)
    // (TX9 est à sa valeur par défaut)
    TXSTAbits.SYNC = 0;     // Mode asynchrone.
    TXSTAbits.TXEN = 1;     // Active l'émetteur.
    RCSTAbits.CREN = 1;     // Active le récepteur.
    RCSTAbits.SPEN = 1;     // Active l'EUSART.
    
    // Active les interruptions (basse priorité):
    PIE1bits.TX1IE = 1;
    IPR1bits.TX1IP = 0;
    PIE1bits.RC1IE = 1;
    IPR1bits.RC1IP = 0;
    
    uartReinitialise();
}
/**  // Configure PWM 1 pour réceptionner le signal de radio-contrôle:
*    ANSELCbits.ANSC2 = 0;
*    TRISCbits.RC2 = 0;
*
}*/

/**
 * Point d'entrée pour le récepteur de radio contrôle.

 *void recepteur_rc() {
    
} */

/**
 * Point d'entrée des interruptions basse priorité.
 
 *void recepteurInterruptions() {
    
 *}


 * Point d'entrée pour le récepteur Bluetooth.
 
 *void recepteur_bt() {
    
}*/

void recepteurMain(void) {
    recepteurInitialiseHardware();
    pwmReinitialise();

    while(1);
}
