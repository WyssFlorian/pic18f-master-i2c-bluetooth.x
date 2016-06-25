#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "i2c.h"
#include "HC06_ZS040.h"
#include "uart.h"
#include "recepteur.h"

 static I2cAdresse i2cAdresse;
 static int adresse;
 static int data;
 static int bloque_av = 0;
 static int bloque_ar = 0;
 static int compteurCapteur = 0;
 
/**
 * Point d'entrée des interruptions pour le maître.
 */
void maitreInterruptions() {
    static I2cAdresse i2cAdresse;
  
    if (INTCON3bits.INT1F) { // drapeau d'interruption externe INT1
        INTCON3bits.INT1F = 0;
        i2cAdresse = ECRITURE_MOTEUR_DC;
        i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,20);
        i2cAdresse = ECRITURE_STEPPER;
        i2cPrepareCommandePourEmission(ECRITURE_STEPPER,20);
    }
    
    if (INTCON3bits.INT2F) { // drapeau d'interruption externe INT2
        INTCON3bits.INT2F = 0;
        i2cAdresse = ECRITURE_MOTEUR_DC;
        i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,-20);
        i2cAdresse = ECRITURE_STEPPER;
        i2cPrepareCommandePourEmission(ECRITURE_STEPPER,-20);
    }

    /** réception de l'adresse puis des data par l'EUSART configurée en mode
      * détection d'adresse.
    */
    
    if (PIR1bits.RC1IF) {
        uartReception();
        RCSTA1bits.ADDEN = 0; // 
        adresse = RCREG1;     //

        while (!PIR1bits.RC1IF){
            RCSTA1bits.ADDEN = 1;
            data = RC1IF;
            switch (adresse){
                case ECRITURE_MOTEUR_DC:
                    if (data > 0 && bloque_av == 0){
                        i2cPrepareCommandePourEmission(adresse, data);
                        bloque_ar = 0;}
                    else if (data < 0 && bloque_ar == 0){
                        i2cPrepareCommandePourEmission(adresse, data);
                        bloque_av = 0;
                    }else{
                        break;
                    }
                    break;
                    
                case ECRITURE_STEPPER:
                    if (data > 0 && bloque_av == 0){
                        i2cPrepareCommandePourEmission(adresse, data);
                        bloque_ar = 0;
                    }
                    else if (data < 0 && bloque_ar == 0){
                        i2cPrepareCommandePourEmission(adresse, data);
                        bloque_av = 0;
                    }else{
                        break;
                    }
                    break; 
            }
        }
    }
    if (PIR1bits.TX1IF) {
        uartTransmission();
    }

   // if (PIR1bits.ADIF) {     // drapeau de fin de conversion A/D
   //     i2cPrepareCommandePourEmission(I2cAdresse, ADRESH);
   //     PIR1bits.ADIF = 0;
   //}
    
   if (PIR1bits.TMR1IF) {   // Interruption 4x par sec sur timer 1
        PIR1bits.TMR1IF = 0;
        TMR1 = 3035;
        
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
    
    if (PIR1bits.SSP1IF) { // drapeau de fin de tâche master i2c
        i2cMaitre();
        PIR1bits.SSP1IF = 0;
    }
}

/**
 * Initialise le hardware pour le maître.
 */
static void maitreInitialiseHardware() {
    
    // Horloge principale Fosc = 1MHz
    OSCCONbits.IRCF = 0b110;    // Mis @8MHz car problème avec en uart @1MHz
    
    ANSELA = 0x00;              // Désactive les convertisseurs A/D.
    ANSELB = 0x00;              // Active les convertisseurs A/D.
    ANSELC = 0x00;              // Désactive les convertisseurs A/D.
    
    // Prépare Temporisateur 1 pour 4 interruptions par sec.
    T1CONbits.TMR1CS = 0;       // Source FOSC/4
    T1CONbits.T1CKPS = 0;       // Pas de diviseur de fréquence.
    T1CONbits.T1RD16 = 1;       // Compteur de 16 bits.
    //T1CONbits.TMR1ON = 1;     // Active le temporisateur.

    PIE1bits.TMR1IE = 1;        // Active les interruptions...
    IPR1bits.TMR1IP = 0;        // ... de basse priorité.
    
    // Interruptions INT1 et INT2
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

    // Active le module de conversion A/D:
    TRISBbits.RB3 = 1;          // Active RB3 comme entrée.
    ANSELBbits.ANSB3 = 1;       // Active AN09 comme entrée analogique.
    TRISBbits.RB4 = 1;          // Active RB4 comme entrée.
    ANSELBbits.ANSB4 = 1;       // Active AN11 comme entrée analogique.
    ADCON0bits.ADON = 1;        // Allume le module A/D.
    ADCON0bits.CHS = 9;         // Branche le convertisseur sur AN09
    ADCON2bits.ADFM = 0;        // Les 8 bits plus signifiants sur ADRESH.
    ADCON2bits.ACQT = 3;        // Temps d'acquisition à 6 TAD.
    ADCON2bits.ADCS = 0;        // À 1MHz, le TAD est à 2us.

    PIE1bits.ADIE = 1;          // Active les interruptions A/D
    IPR1bits.ADIP = 0;          // Interruptions A/D sont de basse priorité.

    // Active le MSSP1 en mode Maître I2C:           à contrôler !!!
    TRISCbits.RC3 = 1;          // RC3 comme entrée...              
    ANSELCbits.ANSC3 = 0;       // ... digitale.
    TRISCbits.RC4 = 1;          // RC4 comme entrée...
    ANSELCbits.ANSC4 = 0;       // ... digitale.

    SSP1CON1bits.SSPEN = 1;     // Active le module SSP.
    
    SSP1CON3bits.PCIE = 1;      // Active l'interruption en cas STOP.
    SSP1CON3bits.SCIE = 1;      // Active l'interruption en cas de START.
    SSP1CON1bits.SSPM = 0b1000; // SSP1 en mode maître I2C.
    SSP1ADD = 31;                // FSCL = FOSC / (4 * (SSP1ADD + 1)) = 62500 Hz.

    PIE1bits.SSP1IE = 1;        // Interruption en cas de transmission I2C...
    IPR1bits.SSP1IP = 0;        // ... de basse priorité.
    

    /* Active les interruptions générales:
     */ 
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
}

void receptionSonar(unsigned char adr_i2c, unsigned char valeur) {
    switch (adr_i2c){
        case LECTURE_CAPTEUR_AV:
            if (valeur < 50){
                i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,0); 
                i2cPrepareCommandePourEmission(ECRITURE_STEPPER,0);
                bloque_av = 1;
                printf("Un obstacle est à %d cm à l'avant", valeur);
            }
            break;
            
        case LECTURE_CAPTEUR_AR:
            if (valeur < 50){
                i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,0); 
                i2cPrepareCommandePourEmission(ECRITURE_STEPPER,0);
                bloque_ar = 1;
                printf("Un obstacle est à %d cm à l'arriere", valeur);
            }
            break;
            
        case LECTURE_CAPTEUR_DR:
             if (valeur < 20){
                i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,0); 
                i2cPrepareCommandePourEmission(ECRITURE_STEPPER,0);
                printf("Un obstacle est à %d cm sur la droite", valeur);
            }
            break;
            
        case LECTURE_CAPTEUR_GA:
             if (valeur < 20){
                i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,0); 
                i2cPrepareCommandePourEmission(ECRITURE_STEPPER,0);
                printf("Un obstacle est à %d cm sur la gauche", valeur);
            }
            break;
    }           
}

/**
 * Point d'entrée pour l'émetteur de radio contrôle.
 */
void maitreMain(void) {
    maitreInitialiseHardware();
    i2cReinitialise();
    i2cRappelCommande(receptionSonar);
    recepteurInitialiseHardware();
    
    //while(1);
    //*
    while(1) {  // fait planter la simulation ! à corriger (accents par exemple)
        char buffer[40];
        int adresseDevice, dataValeur, angle;

        //initialiseHardware();
        printf("Salut !\r\n");
        printf("Exemple de sequence de comande :\r\n");
        printf("Deplacement + combien + angle\r\n");
        printf("Quel type de deplacement ?\r\n");
        printf("A : avancer ou R : Reculer\r\n");
        printf("De combien ?\r\n");
        printf("-100 a 100 : en %%\r\n");
        printf("Quel angle ?\r\n");
        printf("-100 a 100 : en %%\r\n");
        printf("Quel type de deplacement ?\r\n");
        gets(buffer);
        printf("Vous avez dit: %s\r\n", buffer);
        adresseDevice = atoi(buffer);
        printf("De combien ?\r\n");
        gets(buffer);
        printf("Vous avez dit: %s\r\n", buffer);
        dataValeur = atoi(buffer);
        printf("Quel angle ?\r\n");
        gets(buffer);
        printf("Vous avez dit: %s\r\n", buffer);
        angle = atoi(buffer);
        printf("Depacement: %d, de %d avec un angle de %d\r\n", adresseDevice, dataValeur, angle);
    }
    //*/
}
