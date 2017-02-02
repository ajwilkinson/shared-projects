///////////////////////////////////////////////////////////////////////////////
// silnxcommon_plx.cpp
//
// Description:
// 		Common hardware access routines for PLX. These calls are PLX specific.
// 		These are aka API functions.
//
// Revision History:
// 		2002-08-01: mik
//			Created. Derived from windows version of API
// 		2002-09-26: mik
//      		Using the same header file as windows API functions.
//		2006-08-17: Whipple
//			CloseDriver() now removes the msg instance created
//
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>              // for sprintf
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>
#include <linux/ioctl.h>
#include <memory.h>

#include "sicommon_plx.h"
#include "sicommon_plx_types.h"
#include "siddkapi_lnx.h"

#define TRUE 1

///////////////////////////////////////////////////////////////////////////////
// Performance timers. These globals are set used each time an API function is 
// entered and exited. Read the difference to determine the relative times
// of each API function execution. They can give numbers such as words
// transferred per second, etc. Note that these values are only valid on
// successful completion of the function. For error conditions, these are
// invalid. Although you can get these values throuhg use of extern and 
// perform subtraction yourself, use SI_PLX_ConvertBenchmarkToSeconds 
// instead to get the actual time.


UINT32 gSI_PLX_BenchmarkTime0, gSI_PLX_BenchmarkTime1;

///////////////////////////////////////////////////////////////////////////////
// INT32 SI_PLX_GetBenchmarkTime
//
// Description:
// 		Returns the benchmark time difference in seconds.
//
// Parameters:
// 		UINT32 elapsedTime      : 32 bit number for number of msec elapsed.

INT32 SI_PLX_ConvertBenchmarkToSeconds ( UINT32 *elapsedMsec )
{
 	*elapsedMsec = gSI_PLX_BenchmarkTime1 - gSI_PLX_BenchmarkTime0;

	return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_SetupCallbackEvent
//
//	Description:
//
//	Parameters:
//		PAMCCDevice pAMCCDev	: Handle to the device driver.

INT32 SI_PLX_SetupCallbackEvent(PPLXDevice pPLXDev, void *callbackFunction)
{
	pPLXDev->callback.sa_handler = (__sighandler_t)callbackFunction;

	sigemptyset( &pPLXDev->callback.sa_mask );
	pPLXDev->callback.sa_flags = SA_RESTART;
	
	if( sigaction(SIGIO, &pPLXDev->callback, 0) != 0 )
		return e_Err_MiscError;
	
	return e_Err_NoError;
}


///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_SetupMessageEvent
//
//	Description:
//
//	Parameters:
//		PPLXDevice pPLXDev	: Handle to the device driver.

INT32 SI_PLX_SetupMessageEvent
(
	PPLXDevice pPLXDev
)
{
	key_t key;
	int flg;

	key = IPC_PRIVATE;
	flg = IPC_CREAT;
	
	/*Returns the message queue identifier associated to the value 
	of the key argument.
	A new message queue is created if key has value IPC_PRIVATE*/
	pPLXDev->msgID = msgget(key, flg); 
	if(pPLXDev->msgID == -1)
	{
		perror("SetupMessageEvent");
		return e_Err_EventSetup;
	}

	pPLXDev->message_event = 0;

	return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_OpenDriver
//
//	Description:
//		Attempts to link a valid driver to our program
//
//	Parameters:
// 		UINT32 board            : ID Number of the board
//		PPLXDevice pPLXDev		: Handle to the driver
//

INT32 SI_PLX_OpenDriver
(
	UINT32 board, PPLXDevice pPLXDev
)
{
 	char deviceName[32];
	int flags = -1;
	
    sprintf(deviceName, "/dev/siplx%d", board);

	/*creates an open file description that refers to a file and a file 
		descriptor that refers to the open file description. The file 
		descriptor is used by other I/O functions to refer to that file.
		Returns a file descriptor for the named file that is the lowest 
		file descriptor not currently open for that process
		OPen for reading and writing*/
    pPLXDev->hDevice = open(deviceName, O_RDWR);

    if(pPLXDev->hDevice <= 0)
    	return e_Err_DeviceNotFound;

	/*Sets the owning process ID or process group ID for the specified 
		file descriptor.  Pass PID of owner(getpid())*/
	if(fcntl(pPLXDev->hDevice, F_SETOWN, getpid()) != 0)
	{
		SI_PLX_CloseDriver(pPLXDev);
		return e_Err_DeviceNotFound;	//Unable to set process ID
	}

	/*Get the file status flags and file access modes for the specified 
		file descriptor.*/
	flags = fcntl(pPLXDev->hDevice, F_GETFL);	//
	if(flags == -1)
	{
		SI_PLX_CloseDriver(pPLXDev);
		return e_Err_DeviceNotFound;	//Unable to get flags
	}

	/*If enabled for a file descriptor, and an owning process/process group 
		has been specified with the F_SETOWN command to fcntl(), then a SIGIO 
		signal is sent to the owning process/process group when input is 
		available on the file descriptor.*/
	if(fcntl(pPLXDev->hDevice, F_SETFL, flags | FASYNC) == -1)
	{
		SI_PLX_CloseDriver(pPLXDev);
		return e_Err_DeviceNotFound;	//Unable to set flags
	}

	return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_CloseDriver
//
//	Description:
//
//	Parameters:
//		PPLXDevice pPLXDev:	 Handle to the device driver.

INT32 SI_PLX_CloseDriver( PPLXDevice pPLXDev )
{
	SI_PLX_ReleaseDirectAccessAddr(pPLXDev);
	SI_PLX_ReleaseAddonBufferAddr(pPLXDev);
	
	/*Whipple*/
	msgctl(pPLXDev->msgID, IPC_RMID, NULL);

	if( close( pPLXDev->hDevice ) != 0 )
		return e_Err_DeviceNotFound;

	return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_DriverConfig_BlockPoint
//
//	Description:
//		Block/block or block/point for communicating with addon.
//
//	Parameters:
//		PPLXDevice pPLXDev	: Handle to the device driver.
// 		pointOrBlock    : point or block        : 
//      kFlagDriverConfig_Block = block, 
//      kFlagDriverConfig_Point = point

INT32 SI_PLX_DriverConfig_BlockPoint
(
	PPLXDevice pPLXDev, UINT32 pointOrBlock
)
{
	ulong driverParams[SI_PARAMS_COUNT];
 
    //gSI_PLX_BenchmarkTime0 = jiffies;
    switch (pointOrBlock)
    {
    case kFlagDriverConfig_Block:
    	driverParams[0] = SI_CONFIGDRIVER_TRANSFERBLOCK;
        break;

    case kFlagDriverConfig_Point:
    	driverParams[0] = SI_CONFIGDRIVER_TRANSFERPOINT;
        break;

    default:
    	return e_Err_UnknownCommand;
    }

    	if (ioctl
        (
         	pPLXDev->hDevice, 
		IOCTL_SHELDON_DRIVER_CONFIG, 
		driverParams
	) == -1)
		return e_Err_UnknownCommand;
	
//        gSI_PLX_BenchmarkTime1 = jiffies;

        return e_Err_NoError;
}
///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_ReadTarget8
//
//  Description:
//
//  Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 region   : PLX space
//      UINT32 count    : Number of UINT8 to access.
//      UINT32 addonAddr: Addon address (byte offset).
//      UINT8 *hostAddr : Pointer to host memory where the data resides.

INT32 SI_PLX_ReadTarget8
(
	PPLXDevice pPLXDev, 
	UINT32 region, UINT32 count, UINT32 addonAddr, UINT8 *hostAddr
)
{
	ulong driverParams[SI_PARAMS_COUNT];
        
    //gSI_PLX_BenchmarkTime0 = jiffies;

    if (count == 0)
    	return e_Err_NoError;

	driverParams[SI_PARAMS_INDEX_REGION]	= region;
	driverParams[SI_PARAMS_INDEX_OFFSET]	= addonAddr;
	driverParams[SI_PARAMS_INDEX_COUNT]		= count;
	driverParams[SI_PARAMS_INDEX_USERBUFFER]= (ulong)hostAddr;
        
    // 8-bit
    if
    (
    	ioctl
        (
         	pPLXDev->hDevice, 
            IOCTL_SHELDON_PASSTHROUGH_READ_8, 
            driverParams
    	) == -1
    )
    	return e_Err_TargetReadError;

    //gSI_PLX_BenchmarkTime1 = jiffies;

    return e_Err_NoError;

}

///////////////////////////////////////////////////////////////////////////////
//  INT32 SI_PLX_ReadTarget16
//
//  Description:
//
//  Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 region       : PLX space
//      UINT32 count    	: Number of UINT16 to access.
//      UINT32 addonAddr	: Addon address (byte offset).
//      UINT16 *hostAddr	: Pointer to host memory where the data resides.

INT32 SI_PLX_ReadTarget16
(
 	PPLXDevice pPLXDev, 
	UINT32 region, UINT32 count, UINT32 addonAddr, UINT16 *hostAddr
)
{
	ulong driverParams[SI_PARAMS_COUNT];
        
    //gSI_PLX_BenchmarkTime0 = jiffies;

    if (count == 0)
    	return e_Err_NoError;

	driverParams[SI_PARAMS_INDEX_REGION]	= region;
	driverParams[SI_PARAMS_INDEX_OFFSET]	= addonAddr;
	driverParams[SI_PARAMS_INDEX_COUNT]		= count;
	driverParams[SI_PARAMS_INDEX_USERBUFFER]= (ulong)hostAddr;
        
    // 16-bit
    if
    (
    	ioctl
        (
        	pPLXDev->hDevice, 
            IOCTL_SHELDON_PASSTHROUGH_READ_16, 
            driverParams
        ) == -1
    )
    	return e_Err_TargetReadError;
        
    //gSI_PLX_BenchmarkTime1 = jiffies;

    return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_ReadTarget
//
//  Description:
//
//  Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 region   : PLX space
//      UINT32 count    : Number of UINT32 to access.
//  	UINT32 addonAddr: Addon address (byte offset).
//      UINT32 *hostAddr: Pointer to host memory where the data resides.

INT32 SI_PLX_ReadTarget
(
 	PPLXDevice pPLXDev,
	UINT32 region, UINT32 count, UINT32 addonAddr, UINT32 *hostAddr
)
{
	ulong driverParams[SI_PARAMS_COUNT];
        
    //gSI_PLX_BenchmarkTime0 = jiffies;

    if (count == 0)
    	return e_Err_NoError;

	driverParams[SI_PARAMS_INDEX_REGION]	= region;
	driverParams[SI_PARAMS_INDEX_OFFSET]	= addonAddr;
	driverParams[SI_PARAMS_INDEX_COUNT]		= count;
	driverParams[SI_PARAMS_INDEX_USERBUFFER]= (ulong)hostAddr;
        
    if 
    (
	 	ioctl
        (
        	pPLXDev->hDevice, 
            IOCTL_SHELDON_PASSTHROUGH_READ_32, 
            driverParams
        ) == -1
    )
    	return e_Err_TargetReadError;

    //gSI_PLX_BenchmarkTime1 = jiffies;

    return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_WriteTarget8
//
//  Description:
//
//  Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 region    : PLX space
//      UINT32 count     : Number of UINT8 to access.
//      UINT32 addonAddr : Addon address (byte offset).
//      UINT8 *hostAddr  : Pointer to host memory where the data resides.

INT32 SI_PLX_WriteTarget8
(
 	PPLXDevice pPLXDev, 
	UINT32 region, UINT32 count, UINT32 addonAddr, UINT8 *hostAddr
)
{
	ulong driverParams[SI_PARAMS_COUNT];
        
    //gSI_PLX_BenchmarkTime0 = jiffies;

    if (count == 0)
    	return e_Err_NoError;

	driverParams[SI_PARAMS_INDEX_REGION]	= region;
	driverParams[SI_PARAMS_INDEX_OFFSET]	= addonAddr;
	driverParams[SI_PARAMS_INDEX_COUNT]		= count;
	driverParams[SI_PARAMS_INDEX_USERBUFFER]= (ulong)hostAddr;
        
    if
    (
    	ioctl
        (
        	pPLXDev->hDevice, 
            IOCTL_SHELDON_PASSTHROUGH_WRITE_8,
            driverParams
        ) == -1
     )
     	return e_Err_TargetWriteError;
        
    //gSI_PLX_BenchmarkTime1 = jiffies;

    return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_WriteTarget16
//
//  Description:
//
//  Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 region        : PLX space
//      UINT32 count         : Number of UINT16 to access.
//      UINT32 addonAddr	 : Addon address (byte offset).
//      UINT16 *hostAddr     : Pointer to host memory where the data resides.

INT32 SI_PLX_WriteTarget16
(
 	PPLXDevice pPLXDev, 
	UINT32 region, UINT32 count, UINT32 addonAddr, UINT16 *hostAddr
)
{
	ulong driverParams[SI_PARAMS_COUNT];
        
    //gSI_PLX_BenchmarkTime0 = jiffies;

    if (count == 0)
    	return e_Err_NoError;

	driverParams[SI_PARAMS_INDEX_REGION]	= region;
	driverParams[SI_PARAMS_INDEX_OFFSET]	= addonAddr;
	driverParams[SI_PARAMS_INDEX_COUNT]		= count;
	driverParams[SI_PARAMS_INDEX_USERBUFFER]= (ulong)hostAddr;
                
    // 16-bit
    if
    (
    	ioctl
        (
        	pPLXDev->hDevice, 
            IOCTL_SHELDON_PASSTHROUGH_WRITE_16,
            driverParams
 	     ) == -1
     )
     	return e_Err_TargetWriteError;
        
    //gSI_PLX_BenchmarkTime1 = jiffies;

    	return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
// 	INT32 SI_PLX_WriteTarget
//
//	Description:
//
//  Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 region   : PLX space
//      UINT32 count    : Number of UINT32 to access.
//      UINT32 addonAddr: Addon address (byte offset).
//      UINT32 *hostAddr: Pointer to host memory where the data resides.

INT32 SI_PLX_WriteTarget
(
 	PPLXDevice pPLXDev, 
	UINT32 region, UINT32 count, UINT32 addonAddr, UINT32 *hostAddr
)

{
	ulong driverParams[SI_PARAMS_COUNT];
        
    //gSI_PLX_BenchmarkTime0 = jiffies;

    if (count == 0)
    	return e_Err_NoError;

	driverParams[SI_PARAMS_INDEX_REGION]	= region;
	driverParams[SI_PARAMS_INDEX_OFFSET]	= addonAddr;
	driverParams[SI_PARAMS_INDEX_COUNT]		= count;
	driverParams[SI_PARAMS_INDEX_USERBUFFER]= (ulong)hostAddr;
        
    // 32-bit       
    if
    (
    	ioctl
        (
        	pPLXDev->hDevice, 
            IOCTL_SHELDON_PASSTHROUGH_WRITE_32, 
            driverParams
         ) == -1
     )
      	return e_Err_TargetWriteError;
        
     //gSI_PLX_BenchmarkTime1 = jiffies;

   return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT3SI_PLX_ReadBlockDMA
//
//	Description:
//
//  Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 count    : Number of UINT32 to access.
//      UINT32 addonAddr: Addon address (byte offset).
//      UINT32 *hostAddr: Pointer to host memory where the data resides.

INT32 SI_PLX_ReadBlockDMA
(
 	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 addonAddr, UINT32 *hostAddr
)
{
	ulong driverParams[SI_PARAMS_COUNT];
        
    //gSI_PLX_BenchmarkTime0 = jiffies;

	pPLXDev->wait_event = 0;

    if (count == 0)
    	return e_Err_NoError;


	driverParams[0]	= addonAddr;
	driverParams[1]	= count;
	driverParams[2] = (ulong)hostAddr;
        
    if
    (
     	ioctl
        (
         	pPLXDev->hDevice, 
            IOCTL_SHELDON_BUSMASTERED_READ, 
            driverParams
         ) == -1
     )
      	return e_Err_BusmasterReadError;
        
	while(1)
	{
		if(pPLXDev->wait_event == 1)
			break;
	}
/*
	driverParams[0]=TRUE;
	while(driverParams[0])   
	{
 	    	ioctl(pPLXDev->hDevice, IOCTL_SHELDON_BUSMASTERED_READ_DONE, driverParams); 
	}
*/
     //gSI_PLX_BenchmarkTime1 = jiffies;

     return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_WriteBlockDMA
//
//  Description:
//
//  Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 count    : Number of UINT32 to access.
//      UINT32 addonAddr: Addon address (byte offset).
//      UINT32 *hostAddIn file included from /usr/include/string.h:33,

INT32 SI_PLX_WriteBlockDMA
(
 	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 addonAddr, UINT32 *hostAddr
)
{
	ulong driverParams[SI_PARAMS_COUNT];
        
    //gSI_PLX_BenchmarkTime0 = jiffies;

	pPLXDev->wait_event = 0;

    if (count == 0)
    	return e_Err_NoError;


	driverParams[0]	= addonAddr;
	driverParams[1]	= count;
	driverParams[2] = (ulong)hostAddr;
        
    if
    (
     	ioctl
        (
         	pPLXDev->hDevice, 
            IOCTL_SHELDON_BUSMASTERED_WRITE, 
            driverParams
         ) == -1
    )
    	return e_Err_BusmasterWriteError;

	while(1)
	{
		if(pPLXDev->wait_event == 1)
			break;
	}
/*
	driverParams[0]=TRUE;
	while(driverParams[0])   
	{
 	    	ioctl(pPLXDev->hDevice, IOCTL_SHELDON_BUSMASTERED_WRITE_DONE, driverParams); 
	}
*/
    //gSI_PLX_BenchmarkTime1 = jiffies;

    return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//      INT32 SI_PLX_CancelBlockDMA
//
//      Description:
//
//      Parameters:
//              PPLXDevice pPLXDev	: Handle to the device driver.

INT32 SI_PLX_CancelBlockDMA
(
        PPLXDevice pPLXDev
)
{
//        gSI_PLX_BenchmarkTime0 = jiffies;

    	ioctl(pPLXDev->hDevice, IOCTL_SHELDON_CANCEL_BUSMASTERING); 

//        gSI_PLX_BenchmarkTime1 = jiffies;

        return e_Err_NoError;

}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_SetTimeout
//
//	Description:
//
//	Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 value       : Timeout value to set in driver.

INT32 SI_PLX_SetTimeout
(
 	PPLXDevice pPLXDev, UINT32 value
)
{
	ulong driverParams[SI_PARAMS_COUNT];
        
    //gSI_PLX_BenchmarkTime0 = jiffies;

	driverParams[SI_PARAMS_INDEX_REGION]	= value;
	driverParams[SI_PARAMS_INDEX_OFFSET]	= 0;
	driverParams[SI_PARAMS_INDEX_COUNT]	= 0;
	driverParams[SI_PARAMS_INDEX_USERBUFFER]= 0;

    if
    (
    	ioctl
      	(
         	pPLXDev->hDevice,
            IOCTL_SHELDON_BUSMASTERED_TIMEOUT, 
            driverParams
       ) == -1
    )
    	return e_Err_Timeout;

    //gSI_PLX_BenchmarkTime1 = jiffies;

    return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_ReadPCI_OpReg
//
//  Description:
//
//  Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 count    : Number of UINT32 to access.
//      UINT32 opRegNum : OpReg to access (byte offset).
//      UINT32 *values  : Pointer to host memory where the data resides.

INT32 SI_PLX_ReadPCI_OpReg
(
	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 opRegNum, UINT32 *values
)
{
	ulong driverParams[SI_PARAMS_COUNT];
        
    //gSI_PLX_BenchmarkTime0 = jiffies;

    if (count == 0)
    	return e_Err_NoError;
 
	driverParams[SI_PARAMS_INDEX_REGION]	= 0;
	driverParams[SI_PARAMS_INDEX_OFFSET]	= opRegNum;
	driverParams[SI_PARAMS_INDEX_COUNT]		= count;
	driverParams[SI_PARAMS_INDEX_USERBUFFER]= (ulong)values;
                
    if
    (
    	ioctl
        (
        	pPLXDev->hDevice,
            IOCTL_SHELDON_OPREG_READ,
            driverParams
    	) == -1
	)
            return e_Err_OpregReadError;
                        
    //gSI_PLX_BenchmarkTime1 = jiffies;

    return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_WritePCI_OpReg
//
// 	Description:
//
//  Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 count    : Number of bytes to access.
//      UINT32 opRegNum : OpReg to access (byte offset).
//      UINT32 *values  : Pointer to host memory where the data resides.

INT32 SI_PLX_WritePCI_OpReg
(
	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 opRegNum, UINT32 *values
)
{
	ulong driverParams[SI_PARAMS_COUNT];
        
    //gSI_PLX_BenchmarkTime0 = jiffies;

    if (count == 0)
    	return e_Err_NoError;

	driverParams[SI_PARAMS_INDEX_REGION]	= 0;
	driverParams[SI_PARAMS_INDEX_OFFSET]	= opRegNum;
	driverParams[SI_PARAMS_INDEX_COUNT]		= count;
	driverParams[SI_PARAMS_INDEX_USERBUFFER]= (ulong)values;
        
    if
    (
    	ioctl
        (
        	pPLXDev->hDevice,
            IOCTL_SHELDON_OPREG_WRITE, 
            driverParams
        ) == -1
    )
    	return e_Err_OpregWriteError;
        
    //gSI_PLX_BenchmarkTime1 = jiffies;

    return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_ReadMailbox
//
//  Description:
//
//  Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 count    : Number of mailboxes to access.
//      UINT32 mailboxNum: Mailbox number.
//      UINT32 *values  : Pointer to host memory where the data resides.

INT32 SI_PLX_ReadMailbox
(
 	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 mailBoxNum, UINT32 *values       
)
{
	INT32 error;
    UINT32 opRegNum;
                
    //gSI_PLX_BenchmarkTime0 = jiffies;

    if (count == 0)
    	return e_Err_NoError;

    // check if MB# is between 0 and 7      
    if (mailBoxNum < 0 || mailBoxNum > 7)
    	return e_Err_ReadError;
                
    if (mailBoxNum < 2)
    {
		if (mailBoxNum == 0)
    		opRegNum = PLX_MB0;
        else
        	opRegNum = PLX_MB1;
                
        if (opRegNum+count > PLX_MB1+1) 
        	return e_Err_ReadError;
     }
     else
     {
     	switch(mailBoxNum)
        {
            case 2:
				opRegNum = PLX_MB2;
				break;
                        
            case 3:
				opRegNum = PLX_MB3;
				break;
                        
            case 4:
				opRegNum = PLX_MB4;
				break;
                        
            case 5:
				opRegNum = PLX_MB5;
				break;
                        
            case 6:
				opRegNum = PLX_MB6;
				break;
                        
            case 7:
				opRegNum = PLX_MB7;
				break;
					
			default:
				return e_Err_ReadError;
            }       

         	if (opRegNum+count > PLX_MB7+1) 
            	return e_Err_ReadError;
        }
        
		opRegNum <<= 2;
		
		error = 
        	SI_PLX_ReadPCI_OpReg
            (
            	pPLXDev,
                count,
                opRegNum,
                values
            );
                
        if(error != e_Err_NoError)
        	return error;
        
        //gSI_PLX_BenchmarkTime1 = jiffies;

        return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_WriteMailbox
//
//  Description:
//
//  Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 count    : Number of mailboxes to access.
//      UINT32 mailboxNum: Mailbox number.
//      UINT32 *values  : Pointer to host memory where the data resides.

INT32 SI_PLX_WriteMailbox
(
 	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 mailBoxNum, UINT32 *values       
)
{
    INT32 error;
    UINT32 opRegNum;
                
    //gSI_PLX_BenchmarkTime0 = jiffies;

    if (count == 0)
    	return e_Err_NoError;

    // check if MB# is between 0 and 7      
    	if (mailBoxNum < 0 || mailBoxNum > 7)
        	return e_Err_WriteError;
                
        if (mailBoxNum < 2)
        {
         	if (mailBoxNum == 0)
            	opRegNum = PLX_MB0;
            else
             	opRegNum = PLX_MB1;

			if (opRegNum+count > PLX_MB1+1) 
        		return e_Err_ReadError;
         }
        else
        {
         	switch(mailBoxNum)
            {
                case 2:
                        opRegNum = PLX_MB2;
                        break;
                        
                case 3:
                        opRegNum = PLX_MB3;
                        break;
                        
                case 4:
                        opRegNum = PLX_MB4;
                        break;
                        
                case 5:
                        opRegNum = PLX_MB5;
                        break;
                        
                case 6:
                        opRegNum = PLX_MB6;
                        break;
                        
                case 7:
                        opRegNum = PLX_MB7;
                        break;
             }       
        
		    if (opRegNum+count > PLX_MB7+1) 
        		return e_Err_ReadError;
            
        }
        
	opRegNum <<= 2;
		
    error = 
    	SI_PLX_WritePCI_OpReg
        (
        	pPLXDev,
            count,
            opRegNum,
            values
         );
                
    if(error != e_Err_NoError)
    	return error;
        
    //gSI_PLX_BenchmarkTime1 = jiffies;

    return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_ReadPCI_ConfSpace
//
//	Description:
//
//	Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 count    : Number of UINT32 to access.
//      UINT32 offset   : Byte offset from 0 for PCI config space.
//      UINT32 *values  : Pointer to host memory where the data resides.

INT32 SI_PLX_ReadPCI_ConfSpace
(
	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 offset, UINT32 *values	
)
{
	ulong driverParams[SI_PARAMS_COUNT];
        
    //gSI_PLX_BenchmarkTime0 = jiffies;

    if (count == 0)
    	return e_Err_NoError;

	driverParams[SI_PARAMS_INDEX_REGION]	= 0;
	driverParams[SI_PARAMS_INDEX_OFFSET]	= offset;
	driverParams[SI_PARAMS_INDEX_COUNT]		= count;
	driverParams[SI_PARAMS_INDEX_USERBUFFER]= (ulong)values;
        
    if
    (
    	ioctl
        (
         	pPLXDev->hDevice, 
            IOCTL_SHELDON_CONFIG_READ, 
            driverParams
        ) == -1
     )
     	return e_Err_ReadError;
        
    //gSI_PLX_BenchmarkTime1 = jiffies;

    return e_Err_NoError;
}

//////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_ReadPCI_NVWord
//
//  Description:
//
//  Parameters:
//  	PPLXDevice pPLXDev	: Handle to the device driver.
//      UINT32 count    : Number of UINT32 to access.
//      UINT32 addrNVRam: NVRAM address to access (byte offset).
//      UINT32 *values  : Pointer to host memory where the data resides.

INT32 SI_PLX_ReadPCI_NVWord
(
 	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 addrNVRam, UINT32 *values
)
{
	ulong driverParams[SI_PARAMS_COUNT];
        
    //gSI_PLX_BenchmarkTime0 = jiffies;

    if (count == 0)
    	return e_Err_NoError;

	driverParams[SI_PARAMS_INDEX_REGION]	= 0;
	driverParams[SI_PARAMS_INDEX_OFFSET]	= addrNVRam;
	driverParams[SI_PARAMS_INDEX_COUNT]		= count;
	driverParams[SI_PARAMS_INDEX_USERBUFFER]= (ulong)values;
        
    if
    (
     	ioctl
        (
         	pPLXDev->hDevice, 
            IOCTL_SHELDON_NVRAM_READ, 
            driverParams
        ) == -1
     )
     	return e_Err_ReadError;
        
     //gSI_PLX_BenchmarkTime1 = jiffies;

     return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
// 	INT32 SI_PLX_WritePCI_NVWord
//
//  Description:
//
//  Parameters:
//  	PPLXDevice pPLXDev	: int to the device driver.
//  	UINT32 count    : Number of UINT32 to access.
//  	UINT32 addrNVRam: NVRAM address to access (byte offset).
//      UINT32 *values  : Pointer to host memory where the data resides.

INT32 SI_PLX_WritePCI_NVWord
(
 	PPLXDevice pPLXDev, 
	UINT32 count, UINT32 addrNVRam, UINT32 *values        
)
{
	INT32 error;
    UINT32 data32, data32Masked, opRegNum;
	ulong driverParams[SI_PARAMS_COUNT];
        
    //gSI_PLX_BenchmarkTime0 = jiffies;

    if (count == 0)
    	return e_Err_NoError;

	driverParams[SI_PARAMS_INDEX_REGION]	= 0;
	driverParams[SI_PARAMS_INDEX_OFFSET]	= addrNVRam;
	driverParams[SI_PARAMS_INDEX_COUNT]		= count;
	driverParams[SI_PARAMS_INDEX_USERBUFFER]= (ulong)values;
        
    // Set serial EEPROM Write-Protect off
    // 1. read from OpReg

    opRegNum = 0x0C;

    error = 
    	SI_PLX_ReadPCI_OpReg
        ( 
        	pPLXDev, 
            1, 
            opRegNum, 
            &data32 
 	     );
   if ( error != e_Err_NoError)
   		return error;

   // 2. mask bits and write to OpReg
   	data32Masked = data32 & 0xff00ffff;

    error = 
    	SI_PLX_WritePCI_OpReg
        (
        	pPLXDev,
            1, 
            opRegNum, 
            &data32Masked
        );
   if ( error != e_Err_NoError)
   		return error;
        
   // 3. write to NV RAM
   if
   (
   		ioctl
        (
        	pPLXDev->hDevice, 
            IOCTL_SHELDON_NVRAM_WRITE, 
            driverParams
        ) == -1
   )
  	return e_Err_WriteError;
        
    // 4. set serial EEPROM Write-Protect on
    error = 
    	SI_PLX_WritePCI_OpReg
        (
        	pPLXDev,
            1, 
            opRegNum, 
            &data32 
        );
    if ( error != e_Err_NoError)
    	return error;

    //gSI_PLX_BenchmarkTime1 = jiffies;

    return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//      INT32 SI_PLX_DirectAccessAddr
//
//      Description:
//
//      Parameters:
//              PPLXDevice pPLXDev	: Handle to the device driver.
//              UINT32 count    : Number of direct access pointers to get.
//              UINT32 *addr    : Pointer to direct access addresses

INT32 SI_PLX_DirectAccessAddr
(
	PPLXDevice pPLXDev,
    UINT32 count, UINT32 *addr[]    
)
{
	return e_Err_UnknownCommand;
/*

 	DWORD cbReturned;

    gSI_PLX_BenchmarkTime0 = jiffies;

    if (count == 0)
    	return e_Err_NoError;

    // Set up direct access pointers
    if
    (
    	!DeviceIoControl
                (
                        pPLXDev->hDevice, 
                        IOCTL_SHELDON_GET_BADDR, 
                        NULL, 
                        0, 
                        addr, 
                        count * sizeof(UINT32 *), 
                        &cbReturned, 
                        pOverlapped
                )
        )
                return e_Err_MiscError;

        gSI_PLX_BenchmarkTime1 = jiffies;

        return e_Err_NoError;
*/
}

///////////////////////////////////////////////////////////////////////////////
//      INT32 SI_PLX_ReleaseDirectAccessAddr
//
//      Description:
//
//      Parameters:
//              PPLXDevice pPLXDev: Handle to the device driver.

INT32 SI_PLX_ReleaseDirectAccessAddr
(
        PPLXDevice pPLXDev
)
{
	return e_Err_UnknownCommand;
/*
        DWORD cbReturned;

        gSI_PLX_BenchmarkTime0 = jiffies;

        if
        (
                !DeviceIoControl
                (
                        pPLXDev->hDevice, 
                        IOCTL_SHELDON_RELEASE_BADDR, 
                        NULL, 
                        0, 
                        NULL, 
                        0, 
                        &cbReturned, 
                        pOverlapped
                )
        )
                return e_Err_MiscError;

        gSI_PLX_BenchmarkTime1 = jiffies;

        return e_Err_NoError;
*/
}

///////////////////////////////////////////////////////////////////////////////
//      INT32 SI_PLX_GetAddonBufferAddr
//
//      Description:
//
//      Parameters:
//              PPLXDevice pPLXDev: Handle to the device driver.
//              

INT32 SI_PLX_GetAddonBufferAddr
(
        PPLXDevice pPLXDev, 
        UINT32 registerToUse
)
{
	INT32 error;
	ulong driverParams[SI_PARAMS_COUNT];
	
//	gSI_PLX_BenchmarkTime0 = GetTickCount();

	pPLXDev->addonInitBufferSize = 0;
	pPLXDev->addonInitPointer = NULL;

	// get addoninit buffer pointers
	if
	(
    	ioctl
        (
        	pPLXDev->hDevice, 
            IOCTL_SHELDON_GET_ADDON_ADDR, 
            driverParams
         ) == -1
	)
		return e_Err_MiscError;

	pPLXDev->addonInitPointer = (UINT32 *)driverParams[0];
	
	// Get addon-init size.
	error = 
		SI_PLX_ReadPCI_OpReg
		(
			pPLXDev, 
			1, registerToUse << 2, &pPLXDev->addonInitBufferSize
		);	
	if (error != e_Err_NoError)
		return error;

	// this is the addon initiated buffer in bytes
	pPLXDev->addonInitBufferSize = -(int)(pPLXDev->addonInitBufferSize);	

	// this is the addon initiated buffer in DWORDS
	pPLXDev->addonInitBufferSize /= sizeof (UINT32);
	printf("size is 0x%x\n", pPLXDev->addonInitBufferSize);

//	gSI_PLX_BenchmarkTime1 = GetTickCount();

	return e_Err_NoError;
}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_ReleaseAddonBufferAddr
//
//	Description:
//
//	Parameters:
//  	PPLXDevice pPLXDev: Handle to the device driver.

INT32 SI_PLX_ReleaseAddonBufferAddr
(
 	PPLXDevice pPLXDev
)
{
	pPLXDev->addonInitBufferSize *= sizeof (UINT32);

	munmap(pPLXDev->addonInitPointer, pPLXDev->addonInitBufferSize);

    return e_Err_NoError;

}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_WaitMessageFromDSP
//
//	Description:
//
//	Parameters:
//		PPLXDevice pPLXDev	: Handle to the device driver.
//		UINT32 waitMilliSecs: # of milliseconds to wait before timing out

INT32 SI_PLX_WaitMessageFromDSP
(
	PPLXDevice pPLXDev, 
	INT32 waitMilliSecs
)
{
	struct msgBuf mbuf;
	INT32 i = 0;
	//printf("INT32 SI_PLX_WaitMessageFromDSP\n");	//probly should do once

	/*ssize_t msgrcv(int msqid, struct msgbuf *msgp, size_t msgsz, 
			long msgtyp, int msgflg);*/
	/*The system call msgrcv reads a message from the message queue 
		specified by msqid into the msgbuf pointed to by the msgp 
		argument, removing the read message from the queue.*/
	while (i < 1000)
	{
		
		if(msgrcv(pPLXDev->msgID, &mbuf, sizeof(char), EVENT_MSG_TYPE, 0) == -1)
		{
			i++;
			
			
	/*
	E2BIG:	The message text length is greater than msgsz and MSG_NOERROR 
				isn't asserted in msgflg. 
	EACCES: The calling process does not have read permission on the 
				message queue. 
	EFAULT:	The address pointed to by msgp isn't accessible. 
	EIDRM:	While the process was sleeping to receive a message, the 
				message queue was removed. 
	EINTR:	While the process was sleeping to receive a message, 
				the process received a signal that had to be caught. 
	EINVAL: Illegal msgqid value, or msgsz less than 0. 
	ENOMSG:	IPC_NOWAIT was asserted in msgflg and no message of the 
				requested type existed on the message queue. 
	*/
    /*   		switch(errno)
				{
					case E2BIG:
						printf("E2BIG error\n");
					case EACCES:
						printf("EACCES error\n");
					case EFAULT:
						printf("EFAULT error\n");
					case EIDRM:
						printf("EIDRM error\n");
					case EINTR:
						printf("EINTR error\n");
					case EINVAL:
						printf("EINVAL error\n");
					case ENOMSG:
						printf("ENOMSG error\n");
					default:
						printf("unknown error\n");
 
			return e_Err_MsgError;
	*/
		}
		else
			break;
	}
	if ( i == 1000)
	{
		printf("\nmsgrcv error\n");
		return e_Err_MsgError;

	}
	
	if(mbuf.msgText[0] != 1)
	{
		printf("\nmbuf error\n");
		return e_Err_MsgError;
	}
/*
	while(1)
	{
		if(pPLXDev->message_event == 1)
			break;
	}
*/

	return e_Err_NoError;	// interrupt was detected.
}

///////////////////////////////////////////////////////////////////////////////
//      INT32 SI_PLX_ReadAddonInit
//
//      Description:
//              Addon device initiates the transfer.
//                      1. Host configures the addon to initiate a transfer. 
//                      2. Addon writes to the host memory directly.
//                      3. Addon causes host interrupt via doorbell.
//                      4. Host reads from the memory where addon accessed.
//      
//      Parameters:
//              PPLXDevice pPLXDev: Handle to the device driver.
//              UINT32 *addonBuffer             : Pointer to addon buffer.
//              UINT32 count    : Number of UINT32 to access.
//              UINT32 *hostAddr: Pointer to host memory where the data resides.

INT32 SI_PLX_ReadAddonInit
(
        PPLXDevice pPLXDev,
        UINT32 interruptAddr, UINT32 interruptValue,
        UINT32 count, UINT32 *hostAddr
)
{

//        ulong result;
        INT32 error;
//        UINT32 benchmarkTime0;
        
//        benchmarkTime0 = GetTickCount();

        if (count == 0)
                return e_Err_NoError;
        
		pPLXDev->wait_event = 0;

        // begin: Used only by SI boards.
        // Configure the addon side to write to directly write to host memory.
        // This step is specific to sheldon board, and your implementation will
        // be different.
        // cause interrupt from host
        error = 
                SI_PLX_WriteTarget
                (
                        pPLXDev,
                        0, 1, interruptAddr, &interruptValue
                );
        if ( error != e_Err_NoError )
                return error;
        // end: Used only by SI boards.

        // wait for event to occur.
		//this is a very poor way to have the process wait for the intterupt
		
	while(1)
	{
		if(pPLXDev->wait_event == 1)
			break;
	}
		
        // addon transferred to host mem. Now copy from host mem to host mem.
	 	memcpy(hostAddr, pPLXDev->addonInitPointer, count * sizeof(UINT32));
	
//		gSI_PLX_BenchmarkTime0 = benchmarkTime0;
//		gSI_PLX_BenchmarkTime1 = GetTickCount();

        return e_Err_NoError;   // no error return is 0

}

///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_WriteAddonInit
//
//	Description:
//  	Addon device initiates the transfer.
//      	1. Host write to addon-initiated memory. In this sample, we copy,
//             but one can just as easily work directly with addonBuffer.
//			 1. Host configures the addon to initiate a transfer. 
//           2. Addon reads from the host memory directly.
//           3. Addon causes host interrupt via doorbell.
//           4. Host knows the transfer is done.
//      
//  Parameters:
//  	PPLXDevice pPLXDev		: Handle to the device driver.
//  	UINT32 *addonBuffer     : Pointer to addon buffer.
//      UINT32 interruptAddr    : Used by SI to tell addon to do transfer.
//      UINT32 interruptValue   : Used by SI to tell addon to do transfer.
//      UINT32 count    		: Number of UINT32 to access.
//      UINT32 *hostAddr		: Pointer to host memory where the data resides.

INT32 SI_PLX_WriteAddonInit
(
 	PPLXDevice pPLXDev,
	UINT32 interruptAddr, UINT32 interruptValue,
    UINT32 count, UINT32 *hostAddr
)
{

 //       ulong result;
        INT32 error;
//        UINT32 benchmarkTime0;

// 	benchmarkTime0 = GetTickCount();

	if (count == 0)
		return e_Err_NoError;

	// addon transferred to host mem. Now copy from host mem to host mem.
	memcpy( pPLXDev->addonInitPointer, hostAddr, count * sizeof(UINT32));	

	pPLXDev->wait_event = 0;

	// begin: Used only by SI boards.
	// Configure the addon side to write to directly write to host memory.
	// This step is specific to sheldon board, and your implementation will
	// be different.
	// cause interrupt from host
	error = 
		SI_PLX_WriteTarget
		(
			pPLXDev, 
			0, 1, interruptAddr, &interruptValue
		);
	if ( error != e_Err_NoError )
		return error;
	// end: Used only by SI boards.
	// wait for event to occur.
	//this is a very poor way to have the process wait for the intterupt
	while(1)
	{
		if(pPLXDev->wait_event == 1)
			break;
	}
	
	
//	gSI_PLX_BenchmarkTime0 = benchmarkTime0;
//	gSI_PLX_BenchmarkTime1 = GetTickCount();

	return e_Err_NoError;	// no error return is 0
}


///////////////////////////////////////////////////////////////////////////////
//	INT32 SI_PLX_SetupAddonInitParams
//
//	Description:
//		Writes parameters to the addon processor so that it knows where to
//      transfer the data on addon-initiated accesses
//
// 	Parameters:
//  	PPLXDevice pPLXDev		: Handle to the device driver.
//      UINT32 addonAddr        : Location of parameters on addon side.
//      UINT32 mode, UINT32 count, UINT32 srcAddr, UINT32 dstAddr: params

INT32 SI_PLX_SetupAddonInitParams
(
	PPLXDevice pPLXDev, 
	UINT32 addonAddr, 
    UINT32 mode, UINT32 count, UINT32 srcAddr, UINT32 dstAddr
)
{
	UINT32 paramCount, params[10];

    // write parameters
    paramCount = 0;
    params[paramCount++] = mode;
    params[paramCount++] = count;
    params[paramCount++] = srcAddr;
    params[paramCount++] = dstAddr;
    params[paramCount++] = 0;               // flag

    return 
    	SI_PLX_WriteTarget
        (
        	pPLXDev, 0, 
			paramCount, 
			addonAddr, 
			params
       );
}
