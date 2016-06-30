#include "test.h"
#include "pwm.h"

#define PWM_NOMBRE_DE_CANAUX 2
#define PWM_ESPACEMENT 6

 static int canal_prepare;
 static int canal_tab[PWM_NOMBRE_DE_CANAUX];
 static int compt_espacement;
 static unsigned int val_capt[PWM_NOMBRE_DE_CANAUX];
 
 /* Remarques sur la simulation de ce programme: 
 *  Il y a un décalage de 0.2 à 0.6 ° entre les servo commandés directement
 *  et ceux commandés en copiant les valeurs des premiers,
 *  ceci peut être dû au temps de traitement du signal,
 *  à la surcharge du CPU (message d'avertissement) ou à d'autres
 *  raisons que je ne parviens pas à identifier. F.Wyss
 */
 
 /*
 * Convertit une valeur signée générique vers une valeur directement
 * utilisable pour la génération PWM.
 * @param valeur Une valeur entre 0 et 255.
 * @return Une valeur entre 62 et 125.
 */
/*unsigned char pwmConversion(unsigned char valeurGenerique) {
    int val;
    val = valeurGenerique/(85/21)+62;
    return val;
}*/

/**
 * Indique sur quel canal la valeur doit changer.
 * @param canal Le numéro de canal.
 */
/*void pwmPrepareValeur(unsigned char canal) {
*    canal_prepare = canal;
}/*

/**
 * Établit la valeur du canal spécifié par {@link #pwmPrepareValeur}.
 * @param valeur La valeur du canal.
 */
/*void pwmEtablitValeur(unsigned char valeur) {
    canal_tab[canal_prepare] = pwmConversion(valeur);
}/*

/**
 * Rend la valeur PWM correspondante au canal.
 * @param canal Le cana.
 * @return La valeur PWM correspondante au canal.
 */
unsigned char pwmValeur(unsigned char canal) {
  return canal_tab[canal];
}

/**
 * Indique si il est temps d'émettre une pulsation PWM.
 * Sert à espacer les pulsation PWM pour les rendre compatibles
 * avec la norme de radio contrôle.
 * @return 255 si il est temps d'émettre une pulse. 0 autrement.
 */
unsigned char pwmEspacement() {
    if (compt_espacement == PWM_ESPACEMENT){
        compt_espacement = 0;
        return 255;
    }else{
        compt_espacement++;
        return 0;
    }
}
    
/**
 * Démarre une capture sur le canal indiqué.
 * @param canal Numéro du canal.
 * @param instant Instant de démarrage de la capture.
 */
void pwmDemarreCapture(unsigned char canal, unsigned int instant) {
    val_capt[canal] = instant;  
}

/**
 * Complète une capture PWM, et met à jour le canal indiqué.
 * @param canal Le numéro de canal.
 * @param instant L'instant de finalisation de la capture.
 */
void pwmCompleteCapture(unsigned char canal, unsigned int instant) {  
    canal_tab[canal] = instant - val_capt[canal];
}

/**
 * Réinitialise le système PWM.
 */
void pwmReinitialise() {
    compt_espacement == PWM_ESPACEMENT;
}

#ifdef TEST
void testConversionPwm() {
    testeEgaliteEntiers("PWMC001", pwmConversion(  0),  62);
    testeEgaliteEntiers("PWMC002", pwmConversion(  4),  63);

    testeEgaliteEntiers("PWMC003", pwmConversion(126),  93);
    
    testeEgaliteEntiers("PWMC004", pwmConversion(127),  93);
    testeEgaliteEntiers("PWMC005", pwmConversion(128),  94);
    testeEgaliteEntiers("PWMC006", pwmConversion(129),  94);
    
    testeEgaliteEntiers("PWMC007", pwmConversion(132),  95);

    testeEgaliteEntiers("PWMC008", pwmConversion(251), 124);
    testeEgaliteEntiers("PWMC009", pwmConversion(255), 125);
}
void testEtablitEtLitValeurPwm() {
    pwmReinitialise();
    
    pwmPrepareValeur(0);
    pwmEtablitValeur(80);
    testeEgaliteEntiers("PWMV01", pwmValeur(0), pwmConversion(80));
    testeEgaliteEntiers("PWMV02", pwmValeur(1), 0);

    pwmPrepareValeur(1);
    pwmEtablitValeur(180);
    testeEgaliteEntiers("PWMV03", pwmValeur(0), pwmConversion( 80));
    testeEgaliteEntiers("PWMV04", pwmValeur(1), pwmConversion(180));
}
void testEspacementPwm() {
    unsigned char n;
    
    pwmReinitialise();

    for (n = 0; n < PWM_ESPACEMENT; n++) {
        testeEgaliteEntiers("PWME00", pwmEspacement(), 0);
    }

    testeEgaliteEntiers("PWME01", pwmEspacement(), 255);
    
    for (n = 0; n < PWM_ESPACEMENT; n++) {
        testeEgaliteEntiers("PWME00", pwmEspacement(), 0);
    }

    testeEgaliteEntiers("PWME01", pwmEspacement(), 255);    
}
void testCapturePwm() {
    
    pwmDemarreCapture(0, 0);
    pwmCompleteCapture(0, 80);
    
    pwmDemarreCapture(1, 10000);
    pwmCompleteCapture(1, 10090);

    testeEgaliteEntiers("PWMC01a", pwmValeur(0), 80);
    testeEgaliteEntiers("PWMC01b", pwmValeur(1), 90);
    
    pwmDemarreCapture(0, 65526);
    pwmCompleteCapture(0, 80);
    
    pwmDemarreCapture(1, 65516);
    pwmCompleteCapture(1, 80);
    
    testeEgaliteEntiers("PWMC02a", pwmValeur(0), 90);
    testeEgaliteEntiers("PWMC02b", pwmValeur(1), 100);    
}
void testPwm() {    
    testConversionPwm();
    testEtablitEtLitValeurPwm();
    testEspacementPwm();
    testCapturePwm();
}

#endif