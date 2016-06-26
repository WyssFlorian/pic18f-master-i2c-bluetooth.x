#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "i2c.h"
#include "uart.h"
#include "recepteur.h"

 static I2cAdresse i2cAdresse;
 static int adresse;
 static int data;
 static int bloque_av = 0;
 static int bloque_ar = 0;
 static int compteurCapteur = 0;
 
 // Etat commande : Bluetooth ou RC
 typedef enum {
    COMMANDE_BLUETOOTH,
    COMMANDE_RC
} ComandeStatus;
 
 /**
 * État actuel de la commande.
 */
 ComandeStatus commandeEtat = COMMANDE_BLUETOOTH;
 
/**
 * Point d'entrée des interruptions pour le maître.
 */
void maitreInterruptions() {
    static I2cAdresse i2cAdresse;
  
    if (INTCON3bits.INT1F) { // Drapeau d'interruption externe INT1
        INTCON3bits.INT1F = 0;
        
        i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,20);
        i2cPrepareCommandePourEmission(ECRITURE_STEPPER,20);
    }
    
    if (INTCON3bits.INT2F) { // Drapeau d'interruption externe INT2
        INTCON3bits.INT2F = 0;
        
        i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,-20);
        i2cPrepareCommandePourEmission(ECRITURE_STEPPER,-20);
    }

    /** rDeclenchement des iterrptions pour l'EUSART
    */
    if (PIR1bits.RC1IF) {
        commandeEtat = COMMANDE_BLUETOOTH;
        uartReception();
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
        TMR1 = 3036;
        
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
    
    if (PIR1bits.SSP1IF) { // Drapeau de fin de tâche master i2c
        i2cMaitre();
        PIR1bits.SSP1IF = 0;
    }
    
    /*
     if (je sais pas trop quoi - PWM) {
        commandeEtat = COMMANDE_RC;
    */
    
}

/**
 * Initialise le hardware pour le maître.
 */
static void maitreInitialiseHardware() {
    
    // Horloge principale Fosc = 8MHz :
    OSCCONbits.IRCF = 0b110;    // Mis @8MHz car problème avec en uart @1MHz
    
    ANSELA = 0x00;              // Désactive les convertisseurs A/D.
    ANSELB = 0x00;              // Désactive les convertisseurs A/D.
    ANSELC = 0x00;              // Désactive les convertisseurs A/D.
    
    // Prépare Temporisateur 1 pour 4 interruptions par sec. : 
    T1CONbits.TMR1CS = 0;       // Source FOSC/4
    T1CONbits.T1CKPS = 0b11;    // Diviseur de de fréquence 1:8.
    T1CONbits.T1RD16 = 1;       // Compteur de 16 bits.
    T1CONbits.TMR1ON = 1;       // Active le temporisateur.

    PIE1bits.TMR1IE = 1;        // Active les interruptions...
    IPR1bits.TMR1IP = 0;        // ... de basse priorité.
    
    // Interruptions INT1 et INT2 :
    TRISBbits.RB1 = 1;          // Port RB1 comme entrée...
    ANSELBbits.ANSB1 = 0;       // ... digitale.
    TRISBbits.RB2 = 1;          // Port RB2 comme entrée...
    ANSELBbits.ANSB2 = 0;       // ... digitale.
    
    INTCON2bits.RBPU = 0;       // Active les résistances de tirage...
    WPUBbits.WPUB1 = 1;         // ... pour INT1 ...
    WPUBbits.WPUB2 = 1;         // ... et INT2.
    
    INTCON3bits.INT1E = 1;      // Active les interruptions pour INT1...
    INTCON2bits.INTEDG1 = 0;    // Flanc descendant.
    INTCON3bits.INT2E = 1;      // Active les interruptions pour INT2...
    INTCON2bits.INTEDG2 = 0;    // Flanc descendant.

    // Active le module de conversion A/D :
    /**
    TRISBbits.RB3 = 1;          // Active RB3 comme entrée.
    ANSELBbits.ANSB3 = 1;       // Active AN09 comme entrée analogique.
    TRISBbits.RB4 = 1;          // Active RB4 comme entrée.
    ANSELBbits.ANSB4 = 1;       // Active AN11 comme entrée analogique.
    ADCON0bits.ADON = 1;        // Allume le module A/D.
    ADCON0bits.CHS = 9;         // Branche le convertisseur sur AN09
    ADCON2bits.ADFM = 0;        // Les 8 bits plus signifiants sur ADRESH.
    ADCON2bits.ACQT = 3;        // Temps d'acquisition à 6 TAD.
    ADCON2bits.ADCS = 0b101;    // Fosc/16 : À 8MHz, le TAD est à 2us.

    PIE1bits.ADIE = 1;          // Active les interruptions A/D
    IPR1bits.ADIP = 0;          // Interruptions A/D sont de basse priorité.
     */

    // Active le module de capture PWM sur RB3 et RB4 :
    
    TRISBbits.RB3 = 1;          // Active RB3 comme entrée.
    ANSELBbits.ANSB3 = 0;       // Active AN09 comme entrée digitale.
    TRISBbits.RB4 = 1;          // Active RB4 comme entrée.
    ANSELBbits.ANSB4 = 0;       // Active AN11 comme entrée digitale.
    
    
    
    
    
    
    
    
    
    
    // Active le MSSP1 en mode Maître I2C :
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
    

    //Active les interruptions générales : 
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
                printf("Un obstacle est devant a %d [cm].", valeur);
            }
            break;
            
        case LECTURE_CAPTEUR_AR:
            if (valeur < 50){
                i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,0);
                i2cPrepareCommandePourEmission(ECRITURE_STEPPER,0);
                bloque_ar = 1;
                printf("Un obstacle est derriere a %d [cm].", valeur);
            }
            break;
            
        case LECTURE_CAPTEUR_DR:
             if (valeur < 20){
                i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,0);
                i2cPrepareCommandePourEmission(ECRITURE_STEPPER,0);
                printf("Un obstacle est a %d [cm] sur la droite", valeur);
            }
            break;
            
        case LECTURE_CAPTEUR_GA:
             if (valeur < 20){
                i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,0); 
                i2cPrepareCommandePourEmission(ECRITURE_STEPPER,0);
                printf("Un obstacle est a %d [cm] sur la gauche", valeur);
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
    recepteurInitialiseHardware();
    
    char buffer[40];
    int dataValeur, angle;
    
    while(1) {
        
        i2cRappelCommande(receptionSonar);
        
        // Commande via Bluetooth (hyperterminal)
        if (commandeEtat == COMMANDE_BLUETOOTH) {
            printf("MENU UTILISATEUR\r\n");
            printf("Seuence de comande :\r\n");
            printf("Deplacement DC ou Deplacement stepper et angle.\r\n");
            printf("Exemple de sequence :\r\n");
            printf("Deplacement ? (DC : -100 <-> 100 [%%] ou stepper : -127 <-> 127 [pas])\r\n");
            printf("87\r\n");
            printf("Quel angle ? (tourner a droite : valeur positive)\r\n");
            printf("-23\r\n\r\n");
            printf("Deplacement ? (DC : -100 <-> 100 [%%] ou stepper : -127 <-> 127 [pas])\r\n");
            gets(buffer);
            printf("Vous avez dit: %s\r\n", buffer);
            dataValeur = atoi(buffer);
            printf("Quel angle ? (tourner a droite : valeur positive)\r\n");
            gets(buffer);
            printf("Vous avez dit: %s\r\n", buffer);
            angle = atoi(buffer);
            printf("Deplacement de %d avec un angle de %d.\r\n", dataValeur, angle);
            
            
            // Commandes servo avant les commande moteurs, sinon pas de direction ou differees.
            // Commande pour servo DC :
            i2cPrepareCommandePourEmission(ECRITURE_SERVO_DC,(char)angle);
            // Commande pour servo stepper :
            i2cPrepareCommandePourEmission(ECRITURE_SERVO_ST,(char)angle);

            if ((dataValeur > 0) && (bloque_av == 0)) {
                // Commande pour DC :
                i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,(char)dataValeur);
                // Commande pour stepper :
                i2cPrepareCommandePourEmission(ECRITURE_STEPPER,(char)dataValeur);
                // Plus de d'obstacle à l'avant
                bloque_ar = 0;
            }
            else if ((dataValeur < 0) && (bloque_ar == 0)) {
                // Commande pour DC :
                i2cPrepareCommandePourEmission(ECRITURE_MOTEUR_DC,(char)dataValeur);
                // Commande pour stepper :
                i2cPrepareCommandePourEmission(ECRITURE_STEPPER,(char)dataValeur);
                // Plus de d'obstacle à l'arriere
                bloque_av = 0;
            }
        }
        else {  // Commande via telecommande RC
            printf("A present la la commande se fait via la telecommande RC.\r\n");
            printf("Pressez un touche du clavier pour reprendre le controle via l'hyper terminal\r\n");
            
            /*
             Je pense qu'il aura différents switch avec valeur issues des PWMs,
             avec ta gestion des colisions.
             * - Servo
             * - Valeur_deplacement
            */ 
        }
    }
}
