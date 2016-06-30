#include <xc.h>
#include "pwm.h"
#include "test.h"
#include "recepteur.h"
#include "i2c.h"
#include "maitrei2c.h"


#define CAPTURE_FLANC_MONTANT 0b101
#define CAPTURE_FLANC_DESCENDANT 0b100


void recepteurInitialiseHardware() {

    // Prépare le temporisateur 1 pour capture de signal
    T1CONbits.TMR1CS = 0;       // Source est FOSC/4
    T1CONbits.T1CKPS = 2;       // Diviseur de fréquence 1:4, égale à TMR2.
    T1CONbits.T1RD16 = 1;       // Temporisateur de 16 bits.
    T1CONbits.TMR1ON = 1;       // Active le temporisateur.
    
    // Configure les modules de capture CCP5 et CCP4
    TRISAbits.RA4 = 1;          // Port RA4 comme entrée digitale.
    TRISBbits.RB0 = 1;          // Port RB0 comme entrée...
    ANSELBbits.ANSB0 = 0;       // ... digitale.

    CCP4CONbits.CCP4M = CAPTURE_FLANC_MONTANT;
    CCPTMRS1bits.C4TSEL = 0;    // Utilise le temporisateur 1.
    PIE4bits.CCP4IE = 1;        // Active les interruptions
    IPR4bits.CCP4IP = 0;        // ... de basse priorité.

    CCP5CONbits.CCP5M = CAPTURE_FLANC_MONTANT;
    CCPTMRS1bits.C5TSEL = 0;    // Utilise le temporisateur 1.
    PIE4bits.CCP5IE = 1;        // Active les interruptions...
    IPR4bits.CCP5IP = 0;        // ... de basse priorité.
    
    // Prépare Temporisateur 2 pour PWM (compte jusqu'à 125 en 2ms):
    T2CONbits.T2CKPS = 1;       // Diviseur de fréquence 1:4
    T2CONbits.T2OUTPS = 0;      // Pas de diviseur de fréquence à la sortie.
    T2CONbits.TMR2ON = 1;       // Active le temporisateur.
    
    PIE1bits.TMR2IE = 1;        // Active les interruptions ...
    IPR1bits.TMR2IP = 0;        // ... de basse priorité ...
    PIR1bits.TMR2IF = 0;        // ... pour le temporisateur 2.
}

/**
 * Point d'entrée des interruptions basse priorité.
 */
void recepteurInterruptions() {
    unsigned char p1, p2;
    
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;
        if (pwmEspacement()) {
            p1 = pwmValeur(0);
            p2 = pwmValeur(1);
            reception_RC(ECRITURE_MOTEUR_DC,p2);
            reception_RC(ECRITURE_STEPPER,p2);
            reception_RC(ECRITURE_SERVO_DC,p1);
            reception_RC(ECRITURE_SERVO_ST,p1);
        } /*else {
            CCPR3L = 0;
            CCPR1L = 0;
        }*/
    }

    if (PIR4bits.CCP4IF) {
        if (PORTBbits.RB0) {
            pwmDemarreCapture(1, CCPR4);
            CCP4CONbits.CCP4M = CAPTURE_FLANC_DESCENDANT;
        } else {
            pwmCompleteCapture(1, CCPR4);            
            CCP4CONbits.CCP4M = CAPTURE_FLANC_MONTANT;
        }
        PIR4bits.CCP4IF = 0;
    }

    if (PIR4bits.CCP5IF) {
        if (PORTAbits.RA4) {
            pwmDemarreCapture(0, CCPR5);
            CCP5CONbits.CCP5M = CAPTURE_FLANC_DESCENDANT;
        } else {
            pwmCompleteCapture(0, CCPR5);            
            CCP5CONbits.CCP5M = CAPTURE_FLANC_MONTANT;
        }
        PIR4bits.CCP5IF = 0;        
    }
}

/**
 * Point d'entrée pour l'émetteur de radio contrôle.
 */
void recepteurMain(void) {
    
    recepteurInitialiseHardware();
    pwmReinitialise();

    while(1);
}