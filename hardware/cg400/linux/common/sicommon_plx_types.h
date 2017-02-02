///////////////////////////////////////////////////////////////////////////////
//	sicommon_plx_types.h
//
//	Description:
//		Types used by SI PLX DDK.
//
//	Revision History:
//		2002-08-08: mik
//			Created for windows.
//		2002-09-25: mik
//			Added Linux definitions.
//		2003-06-04: mik
//			Made it work with DOS "driver".
//		2006-10-16: Ley
//			Expanded GetBoardInfo so now it also outputs
//			DSP Bus Speed and Boundary Factor.
//			For SISample related programs: Added read NVRam to have the DSP
//			Bus speed in order to calculate parameters for DDS
//			for Sampling Rate.
//		2006-01-30: Madrid
//			Added PLXDev memebers to Linux
//

#ifndef _SICOMMON_PLX_TYPES_H
#define _SICOMMON_PLX_TYPES_H

#if defined(_cplusplus)
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
// Windows defines. These are only valid for WIN32 Windows
#ifdef WIN32

#include <windows.h>

// types used by SI must come after OS specific defines.
#include "../../common_ddk/sitypes.h"

#define SI_MSG_EVENT_NAME "SIMESSAGEEVENT"

struct Struct_AddonInitBufferPointers
{
	UINT32 *lin;	//This pointer is used by the user application
	UINT32 *phys;	//This pointer is used by the hardware
	UINT32 *Mdl;	//This is needed to deallocate under
	UINT32 *driver;	//This is needed to deallocate under
};

// These are global variable organized into structure. These are needed to
//	interact with the driver.
typedef struct
{
	HANDLE		hDevice, 
				// hDriverCallbackEvent == hCallbackEvent, except for W95
				hCallbackEvent, hCallbackEventDriver,
				hMessageEvent, hMessageEventDriver, hMsg,
				hEventRead,	hEventWrite, 
				hEventBusMastered, hEventMisc;

	OVERLAPPED	overlappedRead,		*pOverlappedRead, 
				overlappedWrite,	*pOverlappedWrite,	
				overlappedMisc,		*pOverlappedMisc, 
				overlappedBusMastered, *pOverlappedBusMastered;

	struct Struct_AddonInitBufferPointers	addonInitPointers;

	UINT32 addonInitBufferSize;

	UINT32 paramPLL[2];		// used only by C671x boards.
// WDF driver flag. Use this flag to handle ERROR_IO_PENDING because there are
// ERROR_IO_PENDING errors for DeviceIoControl and WaitForSingleObject function calls
	BOOLEAN WDF;

} PLXDevice, *PPLXDevice;

#endif	// WIN32

///////////////////////////////////////////////////////////////////////////////
// DOS defines.
#ifdef DOS

#include "../../common_ddk/sitypes.h"		
#include "../../siddk_dos/sipcidos/common/sipcidos.h"

typedef struct
{
	UINT16 VID;
	UINT16 DID;
	UINT16 index;

	PCIDev	pciDev;
	PCIDev *pPCIDev;

	UINT32 pointOrBlock;	// API does point/block
	UINT32 addonInitBufferSize;	// dummy for now
}PLXDevice, *PPLXDevice;

#endif	// DOS

///////////////////////////////////////////////////////////////////////////////
// Linux defines.
#ifdef LINUX

// types used by SI must come after OS specific defines.
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "sitypes.h"		

#ifndef NULL
#define NULL	0
#endif

#define EVENT_MSG_TYPE		0x200
#define EVENT_CALL_TYPE		0x400

struct msgBuf { 
       long    msgType;  

       char    msgText[1];

}; 


typedef struct
{
	int		hDevice;
	UINT32	*addonInitPointer;
	UINT32	addonInitBufferSize;
	struct 	sigaction callback;
	
	struct  msgBuf eventMsg[1];
	struct  msgBuf eventCall[1];
	int		msgID;
	int		callID;
	
	int		wait_event;
	int		message_event;

	UINT32	paramPLL[2];	//Exclusive for C6711, moved to CPLXC6711

} PLXDevice, *PPLXDevice;

#endif	// LINUX

///////////////////////////////////////////////////////////////////////////////
#if defined(_cplusplus)
}
#endif

#endif //_SICOMMON_PLX_TYPES_H

