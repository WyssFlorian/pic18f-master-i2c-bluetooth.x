#include <xc.h>
#include "pwm.h"

/**
 * Initialise le hardware pour l'émetteur.
 */
static void emetteurInitialiseHardware() {
    
    // Interruptions INT1 et INT2:
    TRISBbits.RB1 = 1;          // Port RB1 comme entrée...
    ANSELBbits.ANSB1 = 0;       // ... digitale.
    TRISBbits.RB2 = 1;          // Port RB2 comme entrée...
    ANSELBbits.ANSB2 = 0;       // ... digitale.
    
    INTCON2bits.RBPU = 0;       // Active les résistances de tirage...
    WPUBbits.WPUB1 = 1;         // ... pour INT1 ...
    WPUBbits.WPUB2 = 1;         // ... et INT2.
    
    INTCON3bits.INT1E = 1;      // INT1
    INTCON2bits.INTEDG1 = 0;    // Flanc descendant.
    INTCON3bits.INT2E = 1;      // INT2
    INTCON2bits.INTEDG2 = 0;    // Flanc descendant.

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

    // Active le module de conversion A/D:
    TRISBbits.RB3 = 1;      // Active RB4 comme entrée.
    ANSELBbits.ANSB3 = 1;   // Active AN11 comme entrée analogique.
    ADCON0bits.ADON = 1;    // Allume le module A/D.
    ADCON0bits.CHS = 9;     // Branche le convertisseur sur AN09
    ADCON2bits.ADFM = 0;    // Les 8 bits plus signifiants sur ADRESH.
    ADCON2bits.ACQT = 3;    // Temps d'acquisition à 6 TAD.
    ADCON2bits.ADCS = 0;    // À 1MHz, le TAD est à 2us.

    PIE1bits.ADIE = 1;      // Active les interruptions A/D
    IPR1bits.ADIP = 0;      // Interruptions A/D sont de basse priorité.

    // Active les interruptions générales:
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
}

/**
 * Point d'entrée des interruptions pour l'émetteur.
 */
void emetteurInterruptions() {
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
    
    if (INTCON3bits.INT1F) {
        INTCON3bits.INT1F = 0;
        pwmPrepareValeur(1); //attention on va te fournir la valeur pour le canal 1
        ADCON0bits.GO = 1;
    }
    
    if (INTCON3bits.INT2F) {
        INTCON3bits.INT2F = 0;
        pwmPrepareValeur(0); //attention on va te fournir la valeur pour le canal 2
        ADCON0bits.GO = 1;
    }
    
    if (PIR1bits.ADIF) {
        PIR1bits.ADIF = 0;
        pwmEtablitValeur(ADRESH); //attention, on va te fournir la valeur
    }
}

/**
 * Point d'entrée pour l'émetteur de radio contrôle.
 */
void emetteurMain(void) {
    emetteurInitialiseHardware();
    pwmReinitialise();

    while(1);
}
