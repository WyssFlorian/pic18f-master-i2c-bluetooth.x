#ifndef EMETTEUR__H
#define EMETTEUR__H

void maitreInterruptions();
void maitreMain(void);
void maitreInitialiseHardware(void);
void receptionSonar(unsigned char adr_i2c, unsigned char valeur);
void reception_RC(unsigned char adr_i2c, unsigned char valeur);

#endif