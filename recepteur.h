#ifndef RECEPTEUR_H
#define RECEPTEUR_H

void recepteurInterruptions();
void recepteurMain(void);
void recepteurInitialiseHardware();
void recepteur_rc(void);
void recepteur_bt(void);
void pwmReinitialise(void);


#endif