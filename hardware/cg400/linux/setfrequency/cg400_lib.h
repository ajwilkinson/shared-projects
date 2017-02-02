//---------------------------------------------------------------------------
#ifndef cg400_dllH
#define cg400_dllH
//---------------------------------------------------------------------------

#include "../common/sicommon_plx.h"

//---------------------------------------------------------------------------
//  USER ROUTINES
//---------------------------------------------------------------------------

DWORD cg400_CountCards(void);
DWORD cg400_Open(DWORD CardNum);
DWORD cg400_Close(DWORD CardNum);

void cg400_initialize(DWORD BrdNum);
DWORD cg400_GPIO(void);

void cg400_SetFrequency(DWORD BrdNum, double DDS_Frequency);


//---------------------------------------------------------------------------
//  FOR DEBUG ONLY
//---------------------------------------------------------------------------
void cg400_RegisterWinDriver(void);

DWORD cg400_ReadPCIConfigReg(DWORD BrdNum, DWORD dwReg);
void cg400_WritePCIConfigReg(DWORD BrdNum, DWORD dwReg, DWORD RegValue);
DWORD cg400_ReadPCILocalReg(DWORD BrdNum, DWORD dwReg);
void cg400_WritePCILocalReg(DWORD BrdNum, DWORD dwReg, DWORD RegValue);

DWORD cg400_EEPROMReadDWord(DWORD BrdNum, DWORD dwReg);
void cg400_EEPROMWriteDWord(DWORD BrdNum, DWORD dwReg, DWORD RegValue);

WORD cg400_ReadPortW(DWORD BrdNum, DWORD PortAddr);
void cg400_WritePortW(DWORD BrdNum, DWORD PortAddr, WORD PortValue);

WORD cg400_ReadRAM(DWORD BrdNum, DWORD Addr);
void cg400_WriteRAM(DWORD BrdNum, DWORD Addr, WORD Value);
void cg400_ReadBlock(DWORD BrdNum, DWORD StartAddr, DWORD Range, PVOID UserArrayPtr);
void cg400_WriteBlock(DWORD BrdNum, DWORD StartAddr, DWORD Range, PVOID UserArrayPtr);

//---------------------------------------------------------------------------

void SetDeviceID(DWORD NewDeviceID);

#endif


