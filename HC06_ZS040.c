#include <xc.h>
#include <stdio.h>

/* Fonction qui transmet un caractère à l'EUSART.
 * Cette implémentation envoie le caractère à l'EUSART. Le terminal
 * bluetooth est connecté à la sortie TX, il trasmettra le carctère.
 * @param data Le code ASCII du caractère à transmettre.
*/
void putch(unsigned char data) {
    // Attend que le registre tampon d'envoi soit plein.
    while(! TX1IF);
    // Le caractère est écrit sur le port série (et TX1IF est remis à zéro).
    TXREG1 = data;
}

void putst(unsigned char *data) {
    while(*data) {
        putch(*data);   // Envoi un caractère pointé par data.
        data++;         // Incrémente la position du pointeur pour le prochain caractère.
    }
}

/* Fonction qui reçois un caractère depuis l'EUSART.
 * Cette implémentation récupère un caractère sur l'EUSART. Le terminal
 * bluetooth est connecté à la sortie RX, il envoit le carctère.
 * @param unsigned char Le code ASCII du caractère à recevoir.
*/
unsigned char getch() { 
    // Attend que le registre tampon de réception soit vide.
    while(! RC1IF); 
    // Le caractère est lu sur le port série (et RC1IF est remis à zéro).
    return RCREG1;
} 

/*
 * Initialise la sortie 1 de l'EUSART.
 * Si le UP est à 1MHz, le Virtual Terminal dans Proteus
 * doit être configuré comme suit:
 * - 9600 bauds.
 * - Transmission 8 bits.
 * - Bit de stop activé.
 */
void Initialisation_EUSART() {
    // Pour une fréquence de 1MHz, ceci donne 9600 bauds:
    SPBRG = 25;
    SPBRGH = 0;
    // Configure RC6 et RC7 comme entrées digitales, pour que
    // la EUSART puisse en prendre le contrôle:
    TRISCbits.RC6 = 1;
    TRISCbits.RC7 = 1;
    
    // Configure l'EUSART:
    BAUDCONbits.BRG16 = 1;  // Prise en compte du registre SPBRGH.
    // (TX9 est à sa valeur par défaut)
    RCSTAbits.SPEN = 1;     // Active l'EUSART.
    TXSTAbits.SYNC = 0;     // Mode asynchrone.
    RCSTAbits.CREN = 1;     // Active le récepteur.
    TXSTAbits.BRGH = 1;     // Mode haute vitesse.
}