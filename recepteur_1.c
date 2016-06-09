#include <xc.h>
#include "pwm.h"
#include "test.h"

#define CAPTURE_FLANC_MONTANT 0b101
#define CAPTURE_FLANC_DESCENDANT 0b100

/**
 * Initialise le hardware pour l'émetteur.
 */
static void recepteurInitialiseHardware() {

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

    // Configure PWM 1 et 3 pour émettre le signal de radio-contrôle:
    ANSELCbits.ANSC2 = 0;
    TRISCbits.RC2 = 0;
    ANSELCbits.ANSC6 = 0;
    TRISCbits.RC6 = 0;
    
    CCP3CONbits.P3M = 0;        // Simple PWM sur la sortie P3A.
    CCP3CONbits.CCP3M = 12;     // Active le CCP3 comme PWM.
    CCPTMRS0bits.C3TSEL = 0;    // Branché sur le temporisateur 2.
    
    CCP1CONbits.P1M = 0;        // Simple PWM sur la sortie P1A
    CCP1CONbits.CCP1M = 12;     // Active le CCP1 comme PWM.
    CCPTMRS0bits.C1TSEL = 0;    // Branche le CCP1 sur le temporisateur 2.

    PR2 = 200;                  // Période est 2ms plus une marge de sécurité.
                                // (Proteus n'aime pas que CCPRxL dépasse PRx)

    // Active les interruptions générales:
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
}

/**
 * Point d'entrée des interruptions basse priorité.
 */
void recepteurInterruptions() {
    unsigned char p1, p3;
    
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;
        if (pwmEspacement()) {
            p1 = pwmValeur(0);
            p3 = pwmValeur(1);
            CCPR3L = p3;
            CCPR1L = p1;
        } else {
            CCPR3L = 0;
            CCPR1L = 0;
        }
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
