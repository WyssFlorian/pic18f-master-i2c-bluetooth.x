#ifndef I2C__H
#define I2C__H

#define I2C_NOMBRE_ADRESSES_PAR_ESCLAVE 4
#define I2C_MASQUE_ADRESSES_LOCALES 0b11
#define I2C_MASQUE_ADRESSES_ESCLAVES 0b11111000

typedef enum {
    ECRITURE_MOTEUR_DC = 0b00010010,
    ECRITURE_SERVO_DC = 0b00010100,
    ECRITURE_STEPPER = 0b00011010,
    ECRITURE_SERVO_ST = 0b00011100,
    LECTURE_CAPTEUR_AV  = 0b00100001,
    LECTURE_CAPTEUR_DR  = 0b00100111,
    LECTURE_CAPTEUR_AR  = 0b00100011,
    LECTURE_CAPTEUR_GA  = 0b00100101,
} I2cAdresse;

typedef struct {
    I2cAdresse adresse;
    unsigned char valeur;
} I2cCommande;

typedef void (*I2cRappelCommande)(unsigned char, unsigned char);
void i2cRappelCommande(I2cRappelCommande r);
void i2cExposeValeur(unsigned char adresse, unsigned char valeur);
void i2cPrepareCommandePourEmission(I2cAdresse adresse, unsigned char valeur);
unsigned char i2cDonneesDisponiblesPourEmission();
unsigned char i2cRecupereCaracterePourEmission();

void i2cMaitre();
void i2cEsclave();

void i2cReinitialise();

#ifdef TEST
void testI2c();
#endif

#endif