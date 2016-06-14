#include <xc.h>
#include "i2c.h"
#include "HC06_ZS040.h"

 static I2cAdresse i2cAdresse;
  
 
/**
 * Point d'entrée des interruptions pour le maître.
 */
void maitreInterruptions() {
  
    if (INTCON3bits.INT1F) { // drapeau d'interruption externe INT1
        INTCON3bits.INT1F = 0;
        
        i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,20);
        i2cPrepareCommandePourEmission(ECRITURE_STEPPER,20);
      
        ADCON0bits.GO = 1;
    }
    
    if (INTCON3bits.INT2F) { // drapeau d'interruption externe INT2
        INTCON3bits.INT2F = 0;
        
        i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,-20);
        i2cPrepareCommandePourEmission(ECRITURE_STEPPER,-20);
        
        ADCON0bits.GO = 1;

    if (PIR1bits.ADIF) {     // Drapeau de fin de conversion A/D
        i2cPrepareCommandePourEmission("commande RC", ADRESH);
        PIR1bits.ADIF = 0;
    }
    
    if (PIR1bits.TMR1IF) {   // Interruption 4x par sec sur timer 1
        char compteurCapteur = 0;
        //TMR1H = 11;
        //TMR1L = 220;
        TMR1 = 3036; // ce n'est pas interdit ???
        PIR1bits.TMR1IF = 0;
        switch (compteurCapteur) {
            case 0:
                i2cPrepareCommandePourEmission(LECTURE_CAPTEUR_AV, 0);
                break;
            case 1:
                i2cPrepareCommandePourEmission(LECTURE_CAPTEUR_DR, 0); 
                break;
            case 2:
                i2cPrepareCommandePourEmission(LECTURE_CAPTEUR_AR, 0);
                break;
            case 3:
                i2cPrepareCommandePourEmission(LECTURE_CAPTEUR_GA, 0);
                break;
        }
        compteurCapteur ++;
        
        if (compteurCapteur > 3){
            compteurCapteur = 0;
        }
     }
    
    if (PIR1bits.SSP1IF) {
        i2cMaitre();
        PIR1bits.SSP1IF = 0;
    }
    }
}

/**
 * Initialise le hardware pour le maître.
 */
static void maitreInitialiseHardware() {
//    // Prépare PORTA pour entrée digitale:
//    TRISA = 0xF1;
//    ANSELA = 0;
    
    // Prépare Temporisateur 1 pour 4 interruptions par sec.
    T1CONbits.TMR1CS = 0;   // Source FOSC/4
    T1CONbits.T1CKPS = 0;   // Pas de diviseur de fréquence.
    T1CONbits.T1RD16 = 1;   // Comfpteur de 16 bits.
    T1CONbits.TMR1ON = 1;   // Active le temporisateur.

    PIE1bits.TMR1IE = 1;    // Active les interruptions...
    IPR1bits.TMR1IP = 0;    // ... de basse priorité.
    
    // Interruptions INT1 et INT2:
    TRISBbits.RB1 = 1;          // Port RB1 comme entrée...         à modifier pour RC
    ANSELBbits.ANSB1 = 0;       // ... digitale.
    TRISBbits.RB2 = 1;          // Port RB2 comme entrée...
    ANSELBbits.ANSB2 = 0;       // ... digitale.
    
    INTCON2bits.RBPU = 0;       // Active les résistances de tirage...
    WPUBbits.WPUB1 = 1;         // ... pour INT1 ...
    WPUBbits.WPUB2 = 1;         // ... et INT2.
    
    // Linges externes :
    INTCON3bits.INT1E = 1;      // INT1
    INTCON2bits.INTEDG1 = 0;    // Flanc descendant.
    INTCON3bits.INT2E = 1;      // INT2
    INTCON2bits.INTEDG2 = 0;    // Flanc descendant.

    // Active le module de conversion A/D:
    TRISBbits.RB3 = 1;      // Active RB3 comme entrée.
    ANSELBbits.ANSB3 = 1;   // Active AN09 comme entrée analogique.
    ADCON0bits.ADON = 1;    // Allume le module A/D.
    ADCON0bits.CHS = 9;     // Branche le convertisseur sur AN09
    ADCON2bits.ADFM = 0;    // Les 8 bits plus signifiants sur ADRESH.
    ADCON2bits.ACQT = 3;    // Temps d'acquisition à 6 TAD.
    ADCON2bits.ADCS = 0;    // À 1MHz, le TAD est à 2us.

    PIE1bits.ADIE = 1;      // Active les interruptions A/D
    IPR1bits.ADIP = 0;      // Interruptions A/D sont de basse priorité.

    // Active le MSSP1 en mode Maître I2C:           à contrôler !!!
    TRISCbits.RC3 = 1;      // RC3 comme entrée...              
    ANSELCbits.ANSC3 = 0;   // ... digitale.
    TRISCbits.RC4 = 1;      // RC4 comme entrée...
    ANSELCbits.ANSC4 = 0;   // ... digitale.

    SSP1CON1bits.SSPEN = 1;     // Active le module SSP.
    
    SSP1CON3bits.PCIE = 1;      // Active l'interruption en cas STOP.
    SSP1CON3bits.SCIE = 1;      // Active l'interruption en cas de START.
    SSP1CON1bits.SSPM = 0b1000; // SSP1 en mode maître I2C.
    SSP1ADD = 3;                // FSCL = FOSC / (4 * (SSP1ADD + 1)) = 62500 Hz.

    PIE1bits.SSP1IE = 1;        // Interruption en cas de transmission I2C...
    IPR1bits.SSP1IP = 0;        // ... de basse priorité.
    
    // Initilise l'EUSART @9600bits/s :
    Initialisation_EUSART();
    
    // Active les interruptions générales:
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
}

void receptionSonar(unsigned char adresse, unsigned char valeur) {
    
}

/**
 * Point d'entrée pour l'émetteur de radio contrôle.
 */
void maitreMain(void) {
    maitreInitialiseHardware();
    i2cReinitialise();
    i2cRappelCommande(receptionSonar);   //créer une fonction remplaçant celle-là pour traiter (adresse I2c, valeur)
    recepteurInitialiseHardware();

    while(1);
}
