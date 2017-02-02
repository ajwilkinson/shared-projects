//---------------------------------------------------------------------------
// MAIN LIBRARY FOR CG400 FUNCTION CALLS (Filename = cg400_lib.cpp)
//---------------------------------------------------------------------------
//
//   DATE        REV     DESCRIPTION
//   ------      -----   -------------
//   12/18/10    x.xx    Beta
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// POSSIBLE PROBLEMS
// 1)
// 2)

//#include <windows.h>
//#include <winioctl.h>
//#include "windrvr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <exception>
//#include <iostream.h>

#include "siabstract.h"  // Same function as "p9054_lib.h" => Sheldon Wrapper
//#include "chase_lib.h"   // Changed to Sheldon Code on 12-14-2010
#include "cg400_lib.h"
#include "bits.h"


P9054_HANDLE xNULL = 0;  // Creates effective NULL-like pointer to Structure
                     // #define P9054_HANDLE UINT32  //PLXDevice [BOGUS HANDLE]

extern UINT32 pPLXDev;            // Sheldon Fudge

/*****************************************************************************/

enum {
    R0 = 0,     //  AWG1200 R/W CONTROL REGISTERS OFFSETS FROM
    W0 = 0,     //  ADDRESS SPACE1
    W1 = 1,     //

    W2 = 2,     //  LTCH_RPT_CNT
    W3 = 3,     //  LTCH_LW_LIMIT
    W4 = 4      //  LTCH_UW_LIMIT
};


WORD WGR[3];// = {0x059F,0x0042};   // Create and initialize ghost registers.

//HANDLE cg400_hWD = xNULL;           // HANDLE TO WINDRIVER
P9030_HANDLE cg400_hPlx = xNULL;     // HANDLE TO 1st cg400 PLX9030 CARD
P9030_HANDLE cg400_hPlx1 = xNULL;     // HANDLE TO 1st cg400 PLX9030 CARD
P9030_HANDLE cg400_hPlx2 = xNULL;     // HANDLE TO 2nd cg400 PLX9030 CARD
P9030_HANDLE cg400_hPlx3 = xNULL;     // HANDLE TO 3rd cg400 PLX9030 CARD
P9030_HANDLE cg400_hPlx4 = xNULL;     // HANDLE TO 4th cg400 PLX9030 CARD

DWORD NumCards = 0;                 // Stores Number of P9050 Cards found (Global)
DWORD cg400_Card_Cnt[8] = {0,0,0,0,0,0,0,0};  // Stores actual card locations of real cg400  (e.g. cg400.1 = P9030.3)

DWORD DeviceID = 0x9035;  // Default for CG400-PCI
DWORD VendorID = 0x10B5;

/*****************************************************************************/
/*int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*)
{
    return 1;
}
*/
//---------------------------------------------------------------------------

void SetDeviceID(DWORD NewDeviceID)  {
    DeviceID = 0x0000FFFF & NewDeviceID;
    VendorID = NewDeviceID >> 16;
}

//---------------------------------------------------------------------------

void RegisterWinDriver()
{
// Placeholder
}

/*****************************************************************************/

DWORD cg400_CountCards(void)         // ADD QUALIFIERS TO DETECT cg400 IN ADDITION TO P9030
{
//    RegisterWinDriver();  //  Registers WinDriver with latest 4.33 license.
//    return(P9030_CountCards(VendorID, DeviceID));

    DWORD BrdCnt = 0;
    DWORD DeviceVendor = 0;  // Actual Hardware Value
    DWORD DeviceIDVendorID = (DeviceID << 16) + VendorID;  // Requested Value

 //   P9030_HANDLE cg400_hPlx;

    if ( P9030_Open (pPLXDev, VendorID, DeviceID, 0) )
    {
         DeviceVendor = P9030_ReadPCIReg(pPLXDev, 0);
         P9030_Close(pPLXDev);
         if (DeviceVendor == DeviceIDVendorID) BrdCnt = BrdCnt + 1; DeviceVendor = 0;
    }
    if ( P9030_Open (pPLXDev, VendorID, DeviceID, 1) )
    {
         DeviceVendor = P9030_ReadPCIReg(pPLXDev, 0);
         P9030_Close(pPLXDev);
         BrdCnt = BrdCnt + 1;
         if (DeviceVendor == DeviceIDVendorID) BrdCnt = BrdCnt + 1; DeviceVendor = 0;
    }
    if ( P9030_Open (pPLXDev, VendorID, DeviceID, 2) )
    {
         DeviceVendor = P9030_ReadPCIReg(pPLXDev, 0);
         P9030_Close(pPLXDev);
         BrdCnt = BrdCnt + 1;
         if (DeviceVendor == DeviceIDVendorID) BrdCnt = BrdCnt + 1; DeviceVendor = 0;
    }
    if ( P9030_Open (pPLXDev, VendorID, DeviceID, 3) )
    {
         DeviceVendor = P9030_ReadPCIReg(pPLXDev, 0);
         P9030_Close(pPLXDev);
         BrdCnt = BrdCnt + 1;
         if (DeviceVendor == DeviceIDVendorID) BrdCnt = BrdCnt + 1; DeviceVendor = 0;
    }

    if (cg400_hPlx) cg400_hPlx = 0;

    return (BrdCnt);

}

/*****************************************************************************/
/*****************************************************************************/

bool BrdNumOK(DWORD BrdNum)
{
    if ((BrdNum == 1) & (cg400_hPlx1 != xNULL)) return(true);
    if ((BrdNum == 2) & (cg400_hPlx2 != xNULL)) return(true);
    if ((BrdNum == 3) & (cg400_hPlx3 != xNULL)) return(true);
    if ((BrdNum == 4) & (cg400_hPlx4 != xNULL)) return(true);

    return(false);
}

P9030_HANDLE GetBrdHandle(DWORD BrdNum) {
    if ((BrdNum == 1) & (cg400_hPlx1 != xNULL)) return(cg400_hPlx1);
    if ((BrdNum == 2) & (cg400_hPlx2 != xNULL)) return(cg400_hPlx2);
    if ((BrdNum == 3) & (cg400_hPlx3 != xNULL)) return(cg400_hPlx3);
    if ((BrdNum == 4) & (cg400_hPlx4 != xNULL)) return(cg400_hPlx4);

    return(xNULL);
}

/*****************************************************************************/
/*****************************************************************************/

DWORD cg400_Open(DWORD CardNum)        // ADD QUALIFIERS TO DETECT cg400 IN ADDITION TO P9030
{
    NumCards = cg400_CountCards();
    DWORD ReturnCode;

    ReturnCode = 1;  // Default as "No P9030 cards detected" if no other code determined

    if (NumCards > 0){
        if (CardNum > NumCards) ReturnCode = 6;  //  Card number exceeds number of cards

        switch (CardNum) {
            case 1: if (cg400_hPlx1 != xNULL) ReturnCode = 3; break;    // Board already open
            case 2: if (cg400_hPlx2 != xNULL) ReturnCode = 3; break;
            case 3: if (cg400_hPlx3 != xNULL) ReturnCode = 3; break;
            case 4: if (cg400_hPlx4 != xNULL) ReturnCode = 3; break;
        }

        switch (CardNum) {
 //       case 1:  if (P9030_Open (pPLXDev, VendorID, DeviceID, CardNum-1)) {ReturnCode = 0; da11000_hPlx1 = pPLXDev; break;}
            case 1:  if (P9030_Open (pPLXDev, VendorID, DeviceID, CardNum-1)) {ReturnCode = 0; cg400_hPlx1 = pPLXDev; break;}
                        else ReturnCode = 2; break;
            case 2:  if (P9030_Open (pPLXDev, VendorID, DeviceID, CardNum-1)) {ReturnCode = 0; cg400_hPlx2 = pPLXDev; break;}
                        else ReturnCode = 2; break;
            case 3:  if (P9030_Open (pPLXDev, VendorID, DeviceID, CardNum-1)) {ReturnCode = 0; cg400_hPlx3 = pPLXDev; break;}
                        else ReturnCode = 2; break;
            case 4:  if (P9030_Open (pPLXDev, VendorID, DeviceID, CardNum-1)) {ReturnCode = 0; cg400_hPlx4 = pPLXDev; break;}
                        else ReturnCode = 2; break;
        }
    }
    
//  SETUP INTERRUPT ==> NEEDED FOR CALLBACK
    DWORD error = P9030_IntEnable (pPLXDev);//, P9054_INT_HANDLER funcIntHandler)
    if (!error)
    {
        printf("Setup callback failed. Press enter to continue. Code=%d.\n", error);
        getchar();
        return error;
    }


//    if (ReturnCode == 0) cg400_SetTriggerMode(CardNum, 0, 0);

    return(ReturnCode);
}

/*****************************************************************************/

DWORD cg400_Close(DWORD CardNum)      // ADD QUALIFIERS TO DETECT cg400 IN ADDITION TO P9030
{
    if (!BrdNumOK(CardNum)) return(5);
    P9030_HANDLE cg400_hPlx = GetBrdHandle(CardNum);

    P9030_Close(cg400_hPlx);
    cg400_hPlx = xNULL;

    return(0);
}

//***************************************************************************

DWORD cg400_ReadPCIConfigReg(DWORD BrdNum, DWORD dwReg)
{
    if (!BrdNumOK(BrdNum)) return(0);
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

    DWORD RegValue = 0;

    RegValue = P9030_ReadPCIReg(cg400_hPlx, dwReg);
    return(RegValue);
}

void cg400_WritePCIConfigReg(DWORD BrdNum, DWORD dwReg, DWORD RegValue)
{
    if (!BrdNumOK(BrdNum)) return;
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

    P9030_WriteReg (cg400_hPlx, dwReg, RegValue);
    return;
}

//***************************************************************************

DWORD cg400_ReadPCILocalReg(DWORD BrdNum, DWORD dwReg)
{
    if (!BrdNumOK(BrdNum)) return(0);
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

    DWORD RegValue = 0;

    RegValue = P9030_ReadReg(cg400_hPlx, dwReg);  // Same => P9050_ReadSpaceDWord(AD4200_1_hPlx, P9050_ADDR_REG, dwReg);
    return(RegValue);

}

void cg400_WritePCILocalReg(DWORD BrdNum, DWORD dwReg, DWORD RegValue)
{
    if (!BrdNumOK(BrdNum)) return;
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

    P9030_WriteReg(cg400_hPlx, dwReg, RegValue);
    return;

}

/*****************************************************************************/

// BOOL P9030_EEPROMReadDWord(P9030_HANDLE hPlx, DWORD dwOffset, PDWORD pdwData);
/*
DWORD cg400_EEPROMReadDWord(DWORD BrdNum, DWORD dwReg)
{
    if (!BrdNumOK(BrdNum)) return(0);
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

    DWORD RegValue = 0;
    BOOL fEnd = FALSE;
    int i;

//    P9030_EEPROMReadDWord(cg400_hPlx, dwReg, &RegValue);

  	for (i=0; !fEnd && (i<100); i++)
   	{
        if ( P9030_EEPROMReadDWord(cg400_hPlx, dwReg, &RegValue) )
            fEnd = TRUE;
  	}
    return(RegValue);
}

void cg400_EEPROMWriteDWord(DWORD BrdNum, DWORD dwReg, DWORD RegValue)
{
    if (!BrdNumOK(BrdNum)) return;
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

    BOOL fEnd = FALSE;
    int i;

//    P9030_EEPROMWriteDWord(cg400_hPlx, dwReg, RegValue);

   	for (i=0; !fEnd && (i<100); i++)
   	{
        if ( P9030_EEPROMWriteDWord(cg400_hPlx, dwReg, RegValue) )
            fEnd = TRUE;
   	}
    return;
}
*/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

DWORD P9030_CNTRL_DATA;

void EEPROM_DELAY(DWORD BrdNum)
{
    if (!BrdNumOK(BrdNum)) return;
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

//    clock_t start, end;
//    start = clock();

    for (int x = 0; x < 33; x++)
    P9030_CNTRL_DATA = P9030_ReadReg(cg400_hPlx, P9030_CNTRL);  // Save 0x54 Register Value

//    end = clock();
//    printf("The time was: %f\n", (end - start) / CLK_TCK);

    return;
}

/*****************************************************************************/

void WriteSer(DWORD BrdNum, BYTE BinValue)
{
    if (!BrdNumOK(BrdNum)) return;
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

//DWORD P9030_ReadReg(P9030_HANDLE hPlx, DWORD dwReg);
//void P9030_WriteReg(P9030_HANDLE hPlx, DWORD dwReg, DWORD dwData);

// EECS, EESK, EEDI, EEDO

EEPROM_DELAY(BrdNum);
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA & 0xFAFFFFFF);  // Set SER_DIN, SER_CLK to 0
EEPROM_DELAY(BrdNum);

    if (BinValue == 1) {
        P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA | 0x04000000);  // Set SER_DIN to 1
    }

EEPROM_DELAY(BrdNum);
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA | 0x01000000);  // Set SER_CLK to 1
EEPROM_DELAY(BrdNum);
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA & 0xFAFFFFFF);  // Resets both SER_DIN, SER_CLK
EEPROM_DELAY(BrdNum);

}

/*****************************************************************************/

DWORD cg400_EEPROMReadDWord(DWORD BrdNum, DWORD Addr)
{
    if (!BrdNumOK(BrdNum)) return(0);
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

    Addr = Addr / 2;  // Normalize to word boundaries

// EXIT IF PARAMETERS OUT OF RANGE
    if (Addr > 127) return(0);

    P9030_CNTRL_DATA = P9030_ReadReg(cg400_hPlx, P9030_CNTRL);  // Save 0x54 Register Value

// ---------WRITE ENABLE---------------------------------------------
// Make sure SER_DIN, SER_CLK are low
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA & 0xFAFFFFFF);  // Set SER_DIN, SER_CLK to 0
EEPROM_DELAY(BrdNum);

// SET "93C56" CS HIGH TO ENABLE CLOCKING ACTION
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA | 0x02000000);
EEPROM_DELAY(BrdNum);

// LOAD WEN COMMAND FIRST
    WORD WEN_Command = 0x04C0;
    for (int x = 0; x < 11; x++) {
        if ( ( WEN_Command & 0x0400) > 0 ) WriteSer(BrdNum, 1);
        else WriteSer(BrdNum, 0);
        WEN_Command = WEN_Command << 1;
    }
EEPROM_DELAY(BrdNum);
// SET "93C56" CS LOW TO FINISH WRITE
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA & 0xFDFFFFFF);
EEPROM_DELAY(BrdNum);

// ---------READ FIRST WORD---------------------------------------------

// SET "CS_93C56" HIGH TO ENABLE CLOCKING ACTION
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA | 0x02000000);
EEPROM_DELAY(BrdNum);

// LOAD READ COMMAND + ADDRESS
    WORD READ_Command = 0x0600 | Addr;
    for (int x = 0; x < 11; x++) {
        if ( ( READ_Command & 0x0400) > 0 ) WriteSer(BrdNum, 1);
        else WriteSer(BrdNum, 0);
        READ_Command = READ_Command << 1;
    }
EEPROM_DELAY(BrdNum);

// READ ACTUAL E2PROM DATA
    DWORD READ_Data = 0;
    WriteSer(BrdNum, 0);  // Send 1st clock for dummy "0"
EEPROM_DELAY(BrdNum);
    for (int x = 0; x < 32; x++) {
        DWORD TestWord = P9030_ReadReg(cg400_hPlx, P9030_CNTRL);
EEPROM_DELAY(BrdNum);
        READ_Data = READ_Data << 1;
        if ( (TestWord & 0x08000000) > 0 )
            READ_Data = READ_Data + 1;
        WriteSer(BrdNum, 0); // Send clock to read next bit
    }
EEPROM_DELAY(BrdNum);

// SET "CS_93C56" LOW TO FINISH WRITE
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA & 0xFDFFFFFF);

    return(READ_Data);
}

/*****************************************************************************/

void cg400_EEPROMWriteDWord(DWORD BrdNum, DWORD Addr, DWORD RegValue)
{
    if (!BrdNumOK(BrdNum)) return;
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

    Addr = Addr / 2;  // Normalize to word boundaries
EEPROM_DELAY(BrdNum);
    P9030_WriteReg(cg400_hPlx, P9030_PROT_AREA, 0x00000000);  // Removed Protected Area

// EXIT IF PARAMETERS OUT OF RANGE
    if (Addr > 128) return;

EEPROM_DELAY(BrdNum);
    P9030_CNTRL_DATA = P9030_ReadReg(cg400_hPlx, P9030_CNTRL);  // Save 0x54 Register Value

// ---------WRITE ENABLE---------------------------------------------
// Make sure SER_DIN, SER_CLK are low
EEPROM_DELAY(BrdNum);
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA & 0xFAFFFFFF);  // Set SER_DIN, SER_CLK to 0

// SET "93C56" CS HIGH TO ENABLE CLOCKING ACTION
EEPROM_DELAY(BrdNum);
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA | 0x02000000);

// LOAD WEN COMMAND FIRST
    WORD WEN_Command = 0x04C0;
    for (int x = 0; x < 11; x++) {
        if ( ( WEN_Command & 0x0400) > 0 ) WriteSer(BrdNum, 1);
        else WriteSer(BrdNum, 0);
        WEN_Command = WEN_Command << 1;
    }
// SET "93C56" CS LOW TO FINISH WRITE
EEPROM_DELAY(BrdNum);
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA & 0xFDFFFFFF);

    WORD WRITE_Command = 0x0500 | (Addr);

// --------WRITE FIRST WORD----------------------------------------------

// Make sure SER_DIN, SER_CLK are low
EEPROM_DELAY(BrdNum);
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA & 0xFAFFFFFF);  // Set SER_DIN, SER_CLK to 0

// SET "CS_93C56" HIGH TO ENABLE CLOCKING ACTION
EEPROM_DELAY(BrdNum);
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA | 0x02000000);

// LOAD WRITE COMMAND + ADDRESS + DATA
    WRITE_Command = 0x0500 | (Addr);
    for (int x = 0; x < 11; x++) {
        if ( ( WRITE_Command & 0x0400) > 0 ) WriteSer(BrdNum, 1);
        else WriteSer(BrdNum, 0);
        WRITE_Command = WRITE_Command << 1;
    }
    for (int x = 0; x < 16; x++) {
        if ( ( RegValue & 0x80000000) > 0 ) WriteSer(BrdNum, 1);
        else WriteSer(BrdNum, 0);
        RegValue = RegValue << 1;
    }

// SET "CS_93C56" LOW TO FINISH WRITE
EEPROM_DELAY(BrdNum);
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA & 0xFDFFFFFF);

// ---------WRITE SECOND WORD---------------------------------------------

for (int x = 0; x < 500; x++) EEPROM_DELAY(BrdNum);

// Make sure SER_DIN, SER_CLK are low
EEPROM_DELAY(BrdNum);
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA & 0xFAFFFFFF);  // Set SER_DIN, SER_CLK to 0

// SET "CS_93C56" HIGH TO ENABLE CLOCKING ACTION
EEPROM_DELAY(BrdNum);
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA | 0x02000000);

// LOAD WRITE COMMAND + ADDRESS + DATA    (Second Word)
    WRITE_Command = 0x0500 | (Addr+1);
    for (int x = 0; x < 11; x++) {
        if ( ( WRITE_Command & 0x0400) > 0 ) WriteSer(BrdNum, 1);
        else WriteSer(BrdNum, 0);
        WRITE_Command = WRITE_Command << 1;
    }
    for (int x = 0; x < 16; x++) {
        if ( ( RegValue & 0x80000000) > 0 ) WriteSer(BrdNum, 1);
        else WriteSer(BrdNum, 0);
        RegValue = RegValue << 1;
    }

// SET "CS_93C56" LOW TO FINISH WRITE
EEPROM_DELAY(BrdNum);
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA & 0xFDFFFFFF);


//    clock_t start, end;
//    start = clock();

for (int x = 0; x < 500; x++) EEPROM_DELAY(BrdNum);

//    end = clock();

/*
// ---------WRITE DISABLE---------------------------------------------

// Make sure SER_DIN, SER_CLK are low
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA & 0xFAFFFFFF);  // Set SER_DIN, SER_CLK to 0

// SET "93C56" CS HIGH TO ENABLE CLOCKING ACTION
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA | 0x02000000);

// LOAD WDS COMMAND TO PREVENT FURTHER WRITES
    WEN_Command = 0x0400;
    for (int x = 0; x < 11; x++) {
        if ( ( WEN_Command & 0x0400) > 0 ) WriteSer(BrdNum, 1);
        else WriteSer(BrdNum, 0);
        WEN_Command = WEN_Command << 1;
    }
// SET "93C56" CS LOW TO FINISH WRITE
    P9030_WriteReg(cg400_hPlx, P9030_CNTRL, P9030_CNTRL_DATA & 0xFDFFFFFF);
*/

}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

WORD cg400_ReadPortW(DWORD BrdNum, DWORD PortAddr)
{
    if (!BrdNumOK(BrdNum)) return(0);
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

    WORD RegValue = 0;

    RegValue = P9030_ReadSpaceWord (cg400_hPlx, P9030_ADDR_SPACE1, PortAddr*2);

    return(RegValue);
//WORD P9030_ReadSpaceWord (P9030_HANDLE hPlx, P9030_ADDR addrSpace, DWORD dwOffset);
}

void cg400_WritePortW(DWORD BrdNum, DWORD PortAddr, WORD PortValue)
{
    if (!BrdNumOK(BrdNum)) return;
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

    P9030_WriteSpaceWord (cg400_hPlx, P9030_ADDR_SPACE1, PortAddr*2, PortValue);
    WGR[PortAddr] = PortValue;

    return;
//void P9030_WriteSpaceWord (P9030_HANDLE hPlx, P9030_ADDR addrSpace, DWORD dwOffset, WORD data);

}

/*****************************************************************************/

WORD cg400_ReadRAM(DWORD BrdNum, DWORD Addr)
{
    if (!BrdNumOK(BrdNum)) return(0);
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

    cg400_WritePortW(BrdNum, W0, (WGR[0] & ~BIT2) );  // Enable RAM OE

    WORD RegValue = P9030_ReadSpaceWord (cg400_hPlx, P9030_ADDR_SPACE0, Addr*2);

    return(RegValue);
}

void cg400_WriteRAM(DWORD BrdNum, DWORD Addr, WORD Value)
{
    if (!BrdNumOK(BrdNum)) return;
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

    cg400_WritePortW(BrdNum, W0, (WGR[0] | BIT2) );  // Disable RAM OE

    P9030_WriteSpaceWord (cg400_hPlx, P9030_ADDR_SPACE0, Addr*2, Value);

    return;
}

/*****************************************************************************/

void cg400_ReadBlock(DWORD BrdNum, DWORD StartAddr, DWORD Range, PVOID UserArrayPtr)
{
    if (!BrdNumOK(BrdNum)) return;
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

    cg400_WritePortW(BrdNum, W0, (WGR[0] & ~BIT2) );  // Enable RAM OE

    P9030_ReadSpaceBlock(cg400_hPlx, StartAddr*2, UserArrayPtr, Range*2,
                          P9030_ADDR_SPACE0, P9030_MODE_WORD);
}

void cg400_WriteBlock(DWORD BrdNum, DWORD StartAddr, DWORD Range, PVOID UserArrayPtr)
{
    if (!BrdNumOK(BrdNum)) return;
    P9030_HANDLE cg400_hPlx = GetBrdHandle(BrdNum);

    cg400_WritePortW(BrdNum, W0, (WGR[0] | BIT2) );  // Disable RAM OE

    P9030_WriteSpaceBlock(cg400_hPlx, StartAddr*2, UserArrayPtr, Range*2,
                          P9030_ADDR_SPACE0, P9030_MODE_WORD);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

DWORD GPIO_Value;  // Current value of GPIOC

void Set_GPIO(DWORD BrdNum, DWORD RegNum, DWORD BitValue)
{

    if (BitValue == 0) {
        switch (RegNum) {
            case 0: { GPIO_Value = (GPIO_Value & 0xFFFFFFFB); }; break;
            case 1: { GPIO_Value = (GPIO_Value & 0xFFFFFFDF); }; break;
            case 2: { GPIO_Value = (GPIO_Value & 0xFFFFFEFF); }; break;
            case 3: { GPIO_Value = (GPIO_Value & 0xFFFFF7FF); }; break;
            case 4: { GPIO_Value = (GPIO_Value & 0xFFFFBFFF); }; break;
            case 5: { GPIO_Value = (GPIO_Value & 0xFFFDFFFF); }; break;
            case 6: { GPIO_Value = (GPIO_Value & 0xFFEFFFFF); }; break;
            case 7: { GPIO_Value = (GPIO_Value & 0xFF7FFFFF); }; break;
            case 8: { GPIO_Value = (GPIO_Value & 0xFBFFFFFF); }; break;
        }
        cg400_WritePCILocalReg(BrdNum, 0x54, GPIO_Value);
    }

    if (BitValue == 1) {
        switch (RegNum) {
            case 0: { GPIO_Value = (GPIO_Value | 0x00000004); }; break;
            case 1: { GPIO_Value = (GPIO_Value | 0x00000020); }; break;
            case 2: { GPIO_Value = (GPIO_Value | 0x00000100); }; break;
            case 3: { GPIO_Value = (GPIO_Value | 0x00000800); }; break;
            case 4: { GPIO_Value = (GPIO_Value | 0x00004000); }; break;
            case 5: { GPIO_Value = (GPIO_Value | 0x00020000); }; break;
            case 6: { GPIO_Value = (GPIO_Value | 0x00100000); }; break;
            case 7: { GPIO_Value = (GPIO_Value | 0x00800000); }; break;
            case 8: { GPIO_Value = (GPIO_Value | 0x04000000); }; break;
        }
        cg400_WritePCILocalReg(BrdNum, 0x54, GPIO_Value);
    }

}

//  GPIO_0
//  GPIO_1
//  GPIO_2
//  GPIO_3
//  GPIO_4
//  GPIO_5
//  GPIO_6
//  GPIO_7
//  GPIO_8

//*****************************************************************************
//   READ/WRITE PORTS
//*****************************************************************************
//
// ADDR    NAME          BIT    DESCRIPTION
// ----  -------        -----  -----------
//    0  WR_PORT0n        0    DDS_RST
//                        1    STRP
//                        2    not used
//                        3    PLL_SER_CS1n
//                        4    PLL_SER_DIN
//                        5    PLL_SER_SCLK
//
//    1  WR_PORT1n     15-0    DDS_15..DDS_0
//
//    2  WR_PORT2n     15-0    DDS_31..DDS_16
//
//    0  RD_PORT0n        0    PLL_LCK_DET

DWORD DDS_RST       = 0;  //  Controls for DS852 from Euvis
DWORD STRP          = 1;  //  Strobe data works on negative edge __|~|__
//DWORD STRN          = 2;

DWORD PLL_SER_CS1n  = 3;
DWORD PLL_SER_DIN   = 4;
DWORD PLL_SER_SCLK  = 5;

DWORD Port0_Value;  // Current value of Port0

//*****************************************************************************

void Set_Port0(DWORD BrdNum, DWORD RegNum, DWORD BitValue)
{

    if (BitValue == 0) {
        switch (RegNum) {
            case 0: { Port0_Value = (Port0_Value & 0xFFFE); }; break;
            case 1: { Port0_Value = (Port0_Value & 0xFFFD); }; break;
            case 2: { Port0_Value = (Port0_Value & 0xFFFB); }; break;
            case 3: { Port0_Value = (Port0_Value & 0xFFF7); }; break;
            case 4: { Port0_Value = (Port0_Value & 0xFFEF); }; break;
            case 5: { Port0_Value = (Port0_Value & 0xFFDF); }; break;
            case 6: { Port0_Value = (Port0_Value & 0xFFBF); }; break;
            case 7: { Port0_Value = (Port0_Value & 0xFF7F); }; break;
            case 8: { Port0_Value = (Port0_Value & 0xFEFF); }; break;
        }
        cg400_WritePortW(BrdNum, 0, Port0_Value);
    }

    if (BitValue == 1) {
        switch (RegNum) {
            case 0: { Port0_Value = (Port0_Value | 0x0001); }; break;
            case 1: { Port0_Value = (Port0_Value | 0x0002); }; break;
            case 2: { Port0_Value = (Port0_Value | 0x0004); }; break;
            case 3: { Port0_Value = (Port0_Value | 0x0008); }; break;
            case 4: { Port0_Value = (Port0_Value | 0x0010); }; break;
            case 5: { Port0_Value = (Port0_Value | 0x0020); }; break;
            case 6: { Port0_Value = (Port0_Value | 0x0040); }; break;
            case 7: { Port0_Value = (Port0_Value | 0x0080); }; break;
            case 8: { Port0_Value = (Port0_Value | 0x0100); }; break;
        }
        cg400_WritePortW(BrdNum, 0, Port0_Value);
    }

}
//*****************************************************************************

void Load_PLL(DWORD BrdNum, DWORD RegValue)  // LMX2316
{
//  LSB                                                       SHIFTED IN FIRST ==>  MSB
//  C1  C2  F1  F2  F3  F4  F5  F6  F7  F8  F9  F10 F11 F12 F13 F14 F15 F16 F17 F18 F19

    Set_Port0(BrdNum,PLL_SER_SCLK,0);   // Set SCLK to ZERO
    Set_Port0(BrdNum,PLL_SER_DIN,0);   // Set DATA to ZERO
    Set_Port0(BrdNum,PLL_SER_CS1n,0);    // Allows clocking data into PLL

    for (DWORD i=0; i < 21; i++) {
        if ((RegValue & 0x100000) > 0) Set_Port0(BrdNum,PLL_SER_DIN,1);   // SET DATA VALUE
        else Set_Port0(BrdNum,PLL_SER_DIN,0);

        Set_Port0(BrdNum,PLL_SER_SCLK,1);Set_Port0(BrdNum,PLL_SER_SCLK,0);  // CLOCK DATA
        RegValue = RegValue << 1;                                 // Shift left to Grab Next Bit
    }

    Set_Port0(BrdNum,PLL_SER_CS1n,1);  // LOADS REGISTER VALUES INTO DEVICE


//        if ((Addr & 0x00000080) > 0) Set_Port0(BrdNum,PLL_SER_DIN_DDS,1);  // SET DATA VALUE
//        else Set_Port0(BrdNum,PLL_SER_DIN_DDS,0);

//        Set_Port0(BrdNum,SER_SCLK_DDS,1);Set_Port0(BrdNum,SER_SCLK_DDS,0);  // CLOCK DATA
//        Addr = Addr << 1;                                                 // Shift Right to Grab Next Bit


}

//*****************************************************************************/
//*****************************************************************************/

DWORD cg400_GPIO(void) {return(GPIO_Value);}

void cg400_initialize(DWORD BrdNum)
{
    if (!BrdNumOK(BrdNum)) return;

// GPIOC Initialization for PCI9030
    GPIO_Value = 0x02482492;
    cg400_WritePCILocalReg(BrdNum, 0x54, GPIO_Value);  // Set PCI9030 GPIOC to Output GPIO0-GPIO8, all outputs ZERO
                                                       // EXCEPT MAKE GPIO5 INPUT

// DDS & Serial I/O Initialization for DS852 and LMX2316
    Port0_Value = 0;
    cg400_WritePortW(BrdNum, 0, Port0_Value);

    Set_Port0(BrdNum,DDS_RST,1); Set_Port0(BrdNum,STRP,0);// Set_Port0(BrdNum,DDS_RST,0);  // Reset DS852 DDS
    Set_Port0(BrdNum,PLL_SER_CS1n,1);  // Deselect PLL

// Clear DDS Freq & Phase


// LMX2316 PROGRAMMING
// LOAD FUNCTION LATCH (0x10), THEN R COUNTER (0x00), THEN N COUNTER (0x01)

    DWORD Function_LTCH = 0x000092;   // 0x000092
    DWORD R_Counter     = 0x100028;   // 0x100050 => 20MHz, 0x100028 => 10MHz
    DWORD N_Counter     = 0x000FA1;   // 0x000FA1 (31,8: 1000MHz );  0x001361 (38,24: 1240MHz); 0x001381 (39,0: 1248MHz)

    Load_PLL(BrdNum, Function_LTCH);
    Load_PLL(BrdNum,     R_Counter);
    Load_PLL(BrdNum,     N_Counter);

}

//*****************************************************************************/

void cg400_SetFrequency(DWORD BrdNum, double DDS_Frequency) {
    if (!BrdNumOK(BrdNum)) return;

    if ((DDS_Frequency >= 4294967296LL) | (DDS_Frequency < 0)) return;

    double FTW_FLOAT = (DDS_Frequency * 4294967296LL)/1e9;

    DWORD FTW = ceil(FTW_FLOAT);

    if (FTW == 0) Set_Port0(BrdNum,DDS_RST,1);  // SHUTDOWN
    else {
        Set_Port0(BrdNum,DDS_RST,0);  // ENABLE DS852 DDS

        cg400_WritePortW(BrdNum, 1, FTW         & 0x0000FFFF );
        cg400_WritePortW(BrdNum, 2, (FTW >> 16) & 0x0000FFFF );

        Set_Port0(BrdNum,STRP,1);
        Set_Port0(BrdNum,STRP,0); // Latches in New Accumulator Values
    }

//    Set_Port0(BrdNum,RND_RST,1); Set_Port0(BrdNum,RND_RST,0);  // RESET RANDOM GENERATOR

}

/*****************************************************************************/
/*****************************************************************************/

