

#include "../common/sitypes.h"
#include "../common/sicommon_plx_types.h"


#define P9054_HANDLE UINT32//PLXDevice
#define P9030_HANDLE UINT32//PLXDevice
#define P9030HANDLE UINT32//PLXDevice


#define P9030_WriteSpaceWord P9030_WriteWord    // FIX FOR JUNGO -Space-
#define P9030_ReadSpaceWord P9030_ReadWord      // FIX FOR JUNGO -Space-
#define P9030_WriteSpaceBlock P9030_WriteBlock  // FIX FOR JUNGO -Space-
#define P9030_ReadSpaceBlock P9030_ReadBlock    // FIX FOR JUNGO -Space-


#define BOOL bool
#define DWORD UINT32
#define WORD UINT16
#define BYTE UINT8
#define PVOID void*
#define PWORD UINT16*
#define PDWORD UINT32*



typedef enum
{
    P9054_ADDR_REG     = 0,
    P9054_ADDR_REG_IO  = 1,
    P9054_ADDR_SPACE0  = 3,
    P9054_ADDR_SPACE1  = 4,
    P9054_ADDR_SPACE2  = 5,
    P9054_ADDR_SPACE3  = 6,
    P9054_ADDR_EPROM   = 7
} P9054_ADDR;

typedef enum
{
    P9030_ADDR_REG     = 0,
    P9030_ADDR_REG_IO  = 1,
    P9030_ADDR_SPACE0  = 2,
    P9030_ADDR_SPACE1  = 3,
    P9030_ADDR_SPACE2  = 4,
    P9030_ADDR_SPACE3  = 5,
    P9030_ADDR_EPROM   = 6
} P9030_ADDR;

typedef enum
{
    P9054_MODE_BYTE   = 0,
    P9054_MODE_WORD   = 1,
    P9054_MODE_DWORD  = 2
} P9054_MODE;

typedef enum
{
    P9030_MODE_BYTE   = 0,
    P9030_MODE_WORD   = 1,
    P9030_MODE_DWORD  = 2
} P9030_MODE;

typedef enum
{
    P9054_DMA_CHANNEL_0 = 0,
    P9054_DMA_CHANNEL_1 = 1
} P9054_DMA_CHANNEL;

// PLX register definitions 
enum {
    P9030_LAS0RR      = 0x00,
    P9030_LAS1RR      = 0x04,
    P9030_LAS2RR      = 0x08,
    P9030_LAS3RR      = 0x0c,
    P9030_EROMRR      = 0x10,
    P9030_LAS0BA      = 0x14,
    P9030_LAS1BA      = 0x18,
    P9030_LAS2BA      = 0x1c,
    P9030_LAS3BA      = 0x20,
    P9030_EROMBA      = 0x24,
    P9030_LAS0BRD     = 0x28,
    P9030_LAS1BRD     = 0x2c,
    P9030_LAS2BRD     = 0x30,
    P9030_LAS3BRD     = 0x34,
    P9030_EROMBRD     = 0x38,
    P9030_CS0BASE     = 0x3c,
    P9030_CS1BASE     = 0x40,
    P9030_CS2BASE     = 0x44,
    P9030_CS3BASE     = 0x48,
    P9030_INTCSR      = 0x4c,
    P9030_PROT_AREA   = 0x4e,
    P9030_CNTRL       = 0x50,
    P9030_GPIOC       = 0x54,
    P9030_PMDATASEL   = 0x70,
    P9030_PMDATASCALE = 0x74,
};


/*
#ifdef WIN32
typedef struct
{
    DWORD dwCounter;   // number of interrupts received
    DWORD dwLost;      // number of interrupts not yet dealt with
    BOOL fStopped;     // was interrupt disabled during wait
    DWORD dwStatusReg; // value of status register when interrupt occurred
} P9054_INT_RESULT;
typedef void (WINAPI *P9054_INT_HANDLER)( P9054_HANDLE hPlx, P9054_INT_RESULT *intResult);
#endif
*/



//OPEN DRIVER
//BOOL P9054_Open (P9054_HANDLE *phPlx, DWORD dwVendorID, DWORD dwDeviceID, DWORD nCardNum);
BOOL P9054_Open (P9054_HANDLE hPlx, DWORD dwVendorID, DWORD dwDeviceID, DWORD nCardNum);
void P9054_Close (P9054_HANDLE hPlx);

// access registers
DWORD P9054_ReadReg (P9054_HANDLE hPlx, DWORD dwReg);
void P9054_WriteReg (P9054_HANDLE hPlx, DWORD dwReg, DWORD dwData);

BYTE P9054_ReadByte (P9054_HANDLE hPlx, P9054_ADDR addrSpace, DWORD dwOffset);
void P9054_WriteByte (P9054_HANDLE hPlx, P9054_ADDR addrSpace, DWORD dwOffset, BYTE data);
WORD P9054_ReadWord (P9054_HANDLE hPlx, P9054_ADDR addrSpace, DWORD dwOffset);
void P9054_WriteWord (P9054_HANDLE hPlx, P9054_ADDR addrSpace, DWORD dwOffset, WORD data);
DWORD P9054_ReadDWord (P9054_HANDLE hPlx, P9054_ADDR addrSpace, DWORD dwOffset);
void P9054_WriteDWord (P9054_HANDLE hPlx, P9054_ADDR addrSpace, DWORD dwOffset, DWORD data);
void P9054_ReadBlock (P9054_HANDLE hPlx, DWORD dwOffset, PVOID buf, DWORD dwBytes, P9054_ADDR addrSpace, P9054_MODE mode);
void P9054_WriteBlock (P9054_HANDLE hPlx, DWORD dwOffset, PVOID buf, DWORD dwBytes, P9054_ADDR addrSpace, P9054_MODE mode);

void P9054_ReadWriteBlock (P9054_HANDLE hPlx, DWORD dwOffset, PVOID buf, DWORD dwBytes, BOOL fIsRead, P9054_ADDR addrSpace, P9054_MODE mode);

BOOL P9054_DMAReadWriteBlock 
(
	P9054_HANDLE hPlx, DWORD dwLocalAddr, PVOID buf, DWORD dwBytes, BOOL fIsRead, P9054_MODE mode, P9054_DMA_CHANNEL dmaChannel
);

// access PCI configuration registers
DWORD P9054_ReadPCIReg(P9054_HANDLE hPlx, DWORD dwReg);
void P9054_WritePCIReg(P9054_HANDLE hPlx, DWORD dwReg, DWORD dwData);

//EEPROM
BOOL P9054_EEPROMReadWord(P9054_HANDLE hPlx, DWORD dwOffset, PWORD pwData);
BOOL P9054_EEPROMWriteWord(P9054_HANDLE hPlx, DWORD dwOffset, WORD wData);
BOOL P9054_EEPROMReadDWord(P9054_HANDLE hPlx, DWORD dwOffset, PDWORD pdwData);
BOOL P9054_EEPROMWriteDWord(P9054_HANDLE hPlx, DWORD dwOffset, DWORD dwData);

// interrupt functions
BOOL P9054_IntIsEnabled (P9054_HANDLE hPlx);
BOOL P9054_IntEnable (P9054_HANDLE hPlx);//, P9054_INT_HANDLER funcIntHandler);
void P9054_IntDisable (P9054_HANDLE hPlx);

BOOL P9054_IsAddrSpaceActive(P9054_HANDLE hPlx, P9054_ADDR addrSpace);
DWORD P9054_CountCards (DWORD dwVendorID, DWORD dwDeviceID);

//##########################################################################
//     PLX 9030 for DA11000_16M
//##########################################################################

//OPEN DRIVER
BOOL P9030_Open (P9030_HANDLE hPlx, DWORD dwVendorID, DWORD dwDeviceID, DWORD nCardNum);
void P9030_Close (P9030_HANDLE hPlx);

// access PCI configuration registers
DWORD P9030_ReadPCIReg(P9030_HANDLE hPlx, DWORD dwReg);
void P9030_WritePCIReg(P9030_HANDLE hPlx, DWORD dwReg, DWORD dwData);

// access registers
DWORD P9030_ReadReg (P9030_HANDLE hPlx, DWORD dwReg);
void P9030_WriteReg (P9030_HANDLE hPlx, DWORD dwReg, DWORD dwData);

BYTE P9030_ReadByte (P9030_HANDLE hPlx, P9030_ADDR addrSpace, DWORD dwOffset);
void P9030_WriteByte (P9030_HANDLE hPlx, P9030_ADDR addrSpace, DWORD dwOffset, BYTE data);
WORD P9030_ReadWord (P9030_HANDLE hPlx, P9030_ADDR addrSpace, DWORD dwOffset);
void P9030_WriteWord (P9030_HANDLE hPlx, P9030_ADDR addrSpace, DWORD dwOffset, WORD data);
DWORD P9030_ReadDWord (P9030_HANDLE hPlx, P9030_ADDR addrSpace, DWORD dwOffset);
void P9030_WriteDWord (P9030_HANDLE hPlx, P9030_ADDR addrSpace, DWORD dwOffset, DWORD data);

void P9030_ReadBlock (P9030_HANDLE hPlx, DWORD dwOffset, PVOID buf, DWORD dwBytes, P9030_ADDR addrSpace, P9030_MODE mode);
void P9030_WriteBlock (P9030_HANDLE hPlx, DWORD dwOffset, PVOID buf, DWORD dwBytes, P9030_ADDR addrSpace, P9030_MODE mode);

void P9030_ReadWriteBlock (P9030_HANDLE hPlx, DWORD dwOffset, PVOID buf, DWORD dwBytes, BOOL fIsRead, P9030_ADDR addrSpace, P9030_MODE mode);

//EEPROM
BOOL P9030_EEPROMReadWord(P9030_HANDLE hPlx, DWORD dwOffset, PWORD pwData);
BOOL P9030_EEPROMWriteWord(P9030_HANDLE hPlx, DWORD dwOffset, WORD wData);
BOOL P9030_EEPROMReadDWord(P9030_HANDLE hPlx, DWORD dwOffset, PDWORD pdwData);
BOOL P9030_EEPROMWriteDWord(P9030_HANDLE hPlx, DWORD dwOffset, DWORD dwData);

// interrupt functions
BOOL P9030_IntIsEnabled (P9030_HANDLE hPlx);
BOOL P9030_IntEnable (P9030_HANDLE hPlx);//, P9030_INT_HANDLER funcIntHandler);
void P9030_IntDisable (P9030_HANDLE hPlx);

BOOL P9030_IsAddrSpaceActive(P9030_HANDLE hPlx, P9030_ADDR addrSpace);
DWORD P9030_CountCards (DWORD dwVendorID, DWORD dwDeviceID);


