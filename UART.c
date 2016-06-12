void eusart1_init(void)
{
    //configuration de la eusart
    RCSTA1bits.SPEN = 1; //active la eusart
    TXSTA1bits.TXEN = 1; //active l'émetteur
    TXSTA1bits.SYNC = 0; //mode asynchrone
	
    //avec une horloge interne à 1MHZ, cela fait 1200 bauds
	SPBRG1 = 12;
    SPBRGH = 0;
    
    // Configure RC6 et RC7 commen entrée digitales pour que la EUSART 
    // puisse les contrôler.
	TRISCbits.RC6 = 1;
    TRISCbits.RC7 = 1;
}