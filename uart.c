#include <xc.h>
#include <stdio.h>
#include "file.h"
#include "test.h"
#include "uart.h"

/**
 * File des caractères reçus.
 * La file est peuplée par {@link #uartReception} et
 * vidée par {@link #uartGetCh}.
 */
File fileReception;

/**
 * File des caractères à transmettre.
 * La file est peuplée par {@link #uartPutch} et
 * vidée par {@link uartTransmission}.
 */
File fileTransmission;

typedef enum {
    UART_TRANSMISSION_EN_REPOS,
    UART_TRANSMISSION_EN_COURS
} UartStatus;

/**
 * État actuel de la transmission.
 */
UartStatus uartEtat = UART_TRANSMISSION_EN_REPOS;

/**
 * Ajoute un caractère à la file de transmission.
 * Si la file de transmission est pleine, la méthode attend qu'il
 * y ait de la place.
 * @param data Le caractère à placer.
 */
void uartPutch(char data) {
    if (uartEtat == UART_TRANSMISSION_EN_REPOS) {
        uartEtat = UART_TRANSMISSION_EN_COURS;
        TXREG1 = data;
        PIE1bits.TX1IE = 1;
    } else {
        while (fileEstPleine(&fileTransmission));
        fileEnfile(&fileTransmission, data);            
    }
}

/**
 * Indique qu'il n'y a plus de caractères à transmettre.
 * @return 255 si il n'y a plus de caractères à transmettre.
 * @see uartTransmission
 */
unsigned char uartCaracteresDisponiblesPourTransmission() {
    if (fileEstVide(&fileTransmission)) {
        uartEtat = UART_TRANSMISSION_EN_REPOS;
        return 0;
    } else {
        return 255;
    }
}

/**
 * Récupère un caractère de la file de réception.
 * Si la file de réception est vide, la méthode attend qu'un
 * caractère soit disponible.
 * @return Un caractère de la file de réception
 */
char uartGetch() {
    unsigned char n = 0;
    while(fileEstVide(&fileReception)) {
        n++;
    };
    return fileDefile(&fileReception);    
}

/**
 * Fonction à appeler chaque fois que la UART reçoit un caractère.
 * Cette fonction est normalement appelée depuis la routine de gestion
 * des interruptions, en réponse à une interruption de réception.
 * <pre>
 * void interrupt interruptions() {
 *  if (PIR1bits.RC1IF) {
 *      uartReception();
 *  }
 * }
 * </pre>
 * @param c Le caractère reçu.
 */
void uartReception() {
    fileEnfile(&fileReception, RCREG1);
}

/**
 * Fonction à appeler pour récupérer le prochain caractère à transmettre
 * à travers la UART.
 * Cette fonction est typiquement appelée depuis la routine de transmission
 * des interruptions, en réponse à une interruption de transmission.
 * <pre>
 * void interrupt interruptions() {
 *  if (IPR1bits.TX1IF) {
 *      uartTransmission();
 *  }
 * }
 * </pre>
 * @return Le caractère à envoyer.
 */
void uartTransmission() {
    if (uartCaracteresDisponiblesPourTransmission()) {
        TXREG1 = fileDefile(&fileTransmission);
    } else {
        PIE1bits.TX1IE = 0;
    }
}

/**
 * Réinitialise la UART.
 */
void uartReinitialise() {
    fileReinitialise(&fileReception);
    fileReinitialise(&fileTransmission);
    uartEtat = UART_TRANSMISSION_EN_REPOS;
}

#ifndef TEST
/**
 * Implémentation d'une fonction système qui est appelée par <code>printf</code>.
 * Cette implémentation ajoute le caractère à la file de transmission.
 * @param data Le code ASCII du caractère à afficher.
 */
void putch(char data) {
    uartPutch(data);
}

/**
 * Implémentation d'une fonction système qui est appelée par <code>scanf</code>, 
 * <code>getc</code>, <code>getchar</code>, etc.
 * Cette implémentation récupère le caractère à la file de réception.
 * @param data Le code ASCII du récupéré.
 */
char getche() {
    char data = getch();
    putch(data);
    return data;
}

/**
 * Implémentation d'une fonction système qui est appelée par <code>scanf</code>, 
 * <code>getc</code>, <code>getchar</code>, etc.
 * Cette implémentation récupère le caractère à la file de réception.
 * @param data Le code ASCII du récupéré.
 */
char getch() {
    return uartGetch();
}
#endif

#ifdef TEST

void testUartTransmission() {
    uartReinitialise();
    uartPutch('A');
    uartPutch('T');
    testeEgaliteChars  ("UPAT00", uartEtat, UART_TRANSMISSION_EN_COURS);
    testeEgaliteChars  ("UPAT01", dernierCaractereTransmis, 'A');
    testeEgaliteChars  ("UPAT02", uartTransmission(), 'T');
    testeEgaliteEntiers("UPAT03", uartCaracteresDisponiblesPourTransmission(), 0);
}
void testUartReception() {
    uartReinitialise();
    uartReception('A');
    uartReception('T');
    testeEgaliteChars("UARC01", uartGetch(), 'A');
    testeEgaliteChars("UARC02", uartGetch(), 'T');
}
void testUart() {
    testUartTransmission();
    testUartReception();
}
#endif 