// USER EXAMPLE PROGRAM - MAKES CALLS TO "DA11000_lib.cpp"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
//#include <sys/mman.h>	// for debug
#include <math.h>

#include "siabstract.h"
#include "cg400_lib.h"
#include "user_test.h"


//#############################################################################

int main(int argc, char *argv[]) {

   DWORD error = 0;
   DWORD CardNum = 1;

   printf("\n##################################################");


// DISPLAY BANNER
    printf("\nCHASE-SDK Test Application 1.0\n");
    printf("Chase Scientific Company, 2010\n");
    printf("Supporting the CG400-PCI Card \n\n");

    float setFrequency = 0.0;

    if (argc == 2)
    {
    	// Simple parse...
    	setFrequency = atof(argv[1]);
    	printf("Desired frequency - %f Mhz\n", setFrequency);
    }
    else
    {
    	printf("Resetting frequency - usage is setfrequency <mhz>\n\n");
    }




// OPEN DA11000 CARD
    error = cg400_Open(CardNum);
    if (error == 0) printf("CG400 Driver Opened Successfully for CardNum=%d.\n\n", CardNum);
    else {
        printf("Device Not Found. Code=%d.\n\n", error);
        return error;
    }


// INITIALIZE BOARD
    cg400_initialize(CardNum); 
    printf("g400_initialize(CardNum);\n");
    printf("==> Initialize Board.\n\n");

    if (setFrequency > 0.0) {

	// SET FREQUENCY
		cg400_SetFrequency(1, setFrequency * 1000000.00); // Sets Frequency to 155.52 MHz
		printf("Set Frequency to %f MHz.\n\n", setFrequency);


	//  WAIT FOR KEY HIT
		printf("Hit Key to Exit.\n\n");
		getchar();
    }

//    cg400_WriteDDS_AD9858(1, 0x00, 0x0000407E);
    cg400_SetFrequency(1, 0.0);

    cg400_Close(1);
    printf("Close CG400 Driver...");


   printf("\n##################################################\n\n");

    return 0;
}


//#############################################################################


// DEBUG - REGISTER DOWNLOAD ROUTINES

/*
// Read PCI Configuration Registers
    printf("Read PCI Configuration Registers.\n");
    for (DWORD x=0; x < 0x54; x = x+4) {
        DWORD TestRead = cg400_ReadPCIConfigReg(CardNum, x);
        if (x == 0x40) printf("\n");
        printf("0x%08x \n", TestRead);
    }
    printf("\n");

// Read Local Configuration Registers
    printf("Read Local Configuration Registers.\n");
    for (DWORD x=0; x < 0x30; x = x+4) {
        DWORD TestRead = cg400_ReadPCILocalReg(CardNum, x);
        printf("0x%08x \n",TestRead);
    }
    printf("\n");

// Read EEPROM Registers
    printf("Read EEPROM Registers.\n");
    for (DWORD x=0; x < 0x58; x = x+4) {
        DWORD TestRead = cg400_EEPROMReadDWord(CardNum, x);
        printf("0x%08x \n",TestRead);
    }
    printf("\n");
*/
