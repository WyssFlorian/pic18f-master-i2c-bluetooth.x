#include <xc.h>
#include "maitrei2c.h"
#include "i2c.h"
#include "test.h"

/**
 * Bits de configuration:
 */
#pragma config FOSC = INTIO67   // Osc. interne, A6 et A7 comme IO.
#pragma config IESO = OFF       // Pas d'osc. au démarrage.
#pragma config FCMEN = OFF      // Pas de monitorage de l'oscillateur.

// Nécessaires pour ICSP / ICD:
#pragma config MCLRE = EXTMCLR  // RE3 est actif comme master reset.
#pragma config WDTEN = OFF      // Watchdog inactif.
#pragma config LVP = OFF        // Single Supply Enable bits off.

// Configuration de l'entrée RC -> PWM:

// A compléter

// Configuration 

#ifndef TEST

/**
 * Point d'entrée des interruptions basse priorité.
 */
void low_priority interrupt interruptionsBassePriorite() {
    
        maitreInterruptions();  
}

/**
 * Point d'entrée.
 * 
 * Lancement du programme en récepteur.
 */
void main(void) {
  
    maitreMain();
    
    
    while(1);
}
#endif

#ifdef TEST
void main() {
    initialiseTests();
    testI2c();
    finaliseTests();
    while(1);
}
#endif
