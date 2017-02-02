///////////////////////////////////////////////////////////////////////////////
//	sicommon_plx.h
//
//	Description:
//		Common hardware access routines for PLX. These calls are PLX specific.
//
//	Revision History:
//		2002-03-04: mik
//			Created.
//		2002-03-19: mik
//			Renamed AddonInit constants to DSPActiveComm constants.
//		2002-04-12: mik
//			Added FPGALoad parameter for indicator
//		2002-04-16: mik
//			Added SI_PLX_DriverConfig_BlockPoint.
//		2002-04-17: mik
//			Shuffled #define paths to reflect directory changes. Note that
//			the SIDDKAPI.H is not visible here and it should not be visible
//			anywhere else except sicommon_plx.cpp.
//			Added opendriver calls.
//		2002-06-14: mik
//			Added sitypes.h to overcome MS problem of bad basetsd.h file.
//		2002-06-25: mik
//			Added kAddonTransferTimout_Max to simplify timeout for addon-init.
//		2002-06-28: mik
//			Added SI_PLX_ConvertBenchmarkToSeconds.
//		2002-07-01: mik
//			Changed all instances of SIHW_ to SI_
//		2002-07-03: mik
//			Replaced SI_OpenDriver to SI_PLX_OpenDriver.
//		2002-09-16: mik
//			Moved all pOverlapped and callback stuff into API (see .cpp file).
//		2002-09-27: mik
//			Removed SI_PLX_SetupCallbackEvent (only used in cpp).
//		2002-11-15: mik
//			Moved SI_PLX_SetupCallbackEvent back to .h (for Linux compatibility)
//		2003-05-22: mik
//			Added SI_PLX_WaitInterruptFromDSP.
//
///////////////////////////////////////////////////////////////////////////////
#if !defined(_SICOMMON_PLX_H)
#define _SICOMMON_PLX_H

#if defined(_cplusplus)
extern "C" {
#endif

#include "sicommon_plx_types.h"
#include "90xxdef.h"					// 90xx register definitions
#include "sierrors.h"		            // error codes

///////////////////////////////////////////////////////////////////////////////
//	Flags for all chipsets

//	Driver config flags
#define kFlagDriverConfig_Block		1
#define kFlagDriverConfig_Point		2

///////////////////////////////////////////////////////////////////////////////
//	Addon-init transfer timeout values. Change these to suit your hardware
//	transfer speed.

//	Maximum amount of time to wait for addon side to signal the host
//	via interrupt in msec.
#define kAddonTransferTimout_Max	5000

///////////////////////////////////////////////////////////////////////////////
// Begin: Common to all PLX devices

INT32 SI_PLX_ConvertBenchmarkToSeconds ( UINT32 *elapsedMsec );

//INT32 SI_PLX_Sleep( UINT32 sleepMSec );

INT32 SI_PLX_OpenDriver
(
	UINT32 board, PPLXDevice pPLXDev
);

INT32 SI_PLX_CloseDriver
(
	PPLXDevice pPLXDev
);

INT32 SI_PLX_DriverConfig_BlockPoint
(
	PPLXDevice pPLXDev, 
	UINT32 pointOrBlock
);	

INT32 SI_PLX_ReadTarget8
(
	PPLXDevice pPLXDev, 
	UINT32 region, UINT32 count, UINT32 addonAddr, UINT8 *hostAddr
);

INT32 SI_PLX_ReadTarget16
(
	PPLXDevice pPLXDev, 
	UINT32 region, UINT32 count, UINT32 addonAddr, UINT16 *hostAddr
);

INT32 SI_PLX_ReadTarget
(
	PPLXDevice pPLXDev, 
	UINT32 region, UINT32 count, UINT32 addonAddr, UINT32 *hostAddr
);

INT32 SI_PLX_WriteTarget8
(
	PPLXDevice pPLXDev, 
	UINT32 region, UINT32 count, UINT32 addonAddr, UINT8 *hostAddr
);

INT32 SI_PLX_WriteTarget16
(
	PPLXDevice pPLXDev, 
	UINT32 region, UINT32 count, UINT32 addonAddr, UINT16 *hostAddr
);

INT32 SI_PLX_WriteTarget
(
	PPLXDevice pPLXDev, 
	UINT32 region, UINT32 count, UINT32 addonAddr, UINT32 *hostAddr
);

INT32 SI_PLX_ReadBlockDMA
(
	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 addonAddr, UINT32 *hostAddr
);

INT32 SI_PLX_WriteBlockDMA
(
	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 addonAddr, UINT32 *hostAddr
);

INT32 SI_PLX_CancelBlockDMA
(
	PPLXDevice pPLXDev
);

INT32 SI_PLX_SetTimeout
(
	PPLXDevice pPLXDev, UINT32 value
);

INT32 SI_PLX_ReadPCI_OpReg
(
	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 opRegNum, UINT32 *values
);

INT32 SI_PLX_WritePCI_OpReg
(
	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 opRegNum, UINT32 *values
);

INT32 SI_PLX_ReadMailbox
(
	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 mailboxNum, UINT32 *values	
);

INT32 SI_PLX_WriteMailbox
(
	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 mailboxNum, UINT32 *values	
);

INT32 SI_PLX_ReadPCI_ConfSpace
(
	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 offset, UINT32 *values	
);

INT32 SI_PLX_ReadPCI_NVWord
(
	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 addrNVRam, UINT32 *values
);

INT32 SI_PLX_WritePCI_NVWord
(
	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 addrNVRam, UINT32 *values	
);

INT32 SI_PLX_DirectAccessAddr
(
	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 *addr[]	
);

INT32 SI_PLX_ReleaseDirectAccessAddr
(
	PPLXDevice pPLXDev
);

INT32 SI_PLX_GetAddonBufferAddr
(
	PPLXDevice pPLXDev, 
	UINT32 registerToUse
);

INT32 SI_PLX_ReleaseAddonBufferAddr
(
	PPLXDevice pPLXDev
);

INT32 SI_PLX_WaitInterruptFromDSP
(
	PPLXDevice pPLXDev, 
	INT32 waitMilliSecs
);

INT32 SI_PLX_WaitMessageFromDSP
(
	PPLXDevice pPLXDev, 
	INT32 waitMilliSecs
);

// End: Common to all PLX devices

// Begin: Added by SI to support features of PLX

INT32 SI_PLX_ReadAddonInit
(
	PPLXDevice pPLXDev, 
	UINT32 interruptAddr, UINT32 interruptValue,
	UINT32 count, UINT32 *hostAddr
);

INT32 SI_PLX_WriteAddonInit
(
	PPLXDevice pPLXDev, 
	UINT32 interruptAddr, UINT32 interruptValue,
	UINT32 count, UINT32 *hostAddr
);

INT32 SI_PLX_SetupAddonInitParams
(
	PPLXDevice pPLXDev, 
	UINT32 addonAddr, 
	UINT32 mode, UINT32 count, UINT32 srcAddr, UINT32 dstAddr
);

INT32 SI_PLX_SetupCallbackEvent
(
	PPLXDevice pPLXDev, void *callbackFunction
);

INT32 SI_PLX_SetupMessageEvent
(
	PPLXDevice pPLXDev
);

// End: Added by SI to support features of PLX
#if defined(_cplusplus)
}
#endif
#endif
