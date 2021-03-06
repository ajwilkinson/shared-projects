
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/sicommon_plx.h"

PLXDevice gPLXDev;
#define P9054_HANDLE UINT32//PLXDevice
P9054_HANDLE pPLXDev = (UINT32) &gPLXDev;


#include "siabstract.h"


#define kMaxBufferSize	( 10000 )

#ifdef WIN32
//	Windows callback. This occurs when callbackEvent is generated by the 
//	driver after the PCI interrupt.

DWORD WINAPI callback(LPVOID dummy)
{
	static ints = 0;

	while(1)
	{
		if(WaitForSingleObject(gPLXDev.hCallbackEvent, INFINITE))
			printf("\nWaiting for interrupt on Event 0x%x\n", gPLXDev.hCallbackEvent);
		else
			printf("\nInterrupt # %d has been generated\n", ++ints);
	}

	return 0;
}

#endif


#ifdef LINUX
void callback(int param)
{
	static int trigger = 0;

	printf("\nReceived Signal %d Interrupt #%d generated\n", param,
          ++trigger);
	
	//pPLXDev->wait_event = 1;	
	gPLXDev.wait_event = 1;
	
	printf("\nReceived Signal done\n");
	//fflush(stdout);
}
#endif


BOOL P9054_IntIsEnabled (P9054_HANDLE hPlx)
{
	INT32 error;
	UINT32 value, *data;
	DWORD dwReg= 0x68;	//Define as INTCSR

	data = &value;
	
	error = SI_PLX_ReadPCI_OpReg((PLXDevice *)hPlx, 1, dwReg, data);
	if (error != e_Err_NoError)
	{
		printf("Driver reported an error in OpReg Read. Code=%d.\n", error);
		return 0;
	}	

	if(value & 0x100)
		return true;
	else
		return false;
}
BOOL P9054_IntEnable (P9054_HANDLE hPlx)//, P9054_INT_HANDLER funcIntHandler)
{
	INT32 error;
	DWORD value, *data;
	DWORD dwReg= 0x68;	//Define as INTCSR

	data = &value;
	
	error = SI_PLX_ReadPCI_OpReg((PLXDevice *)hPlx, 1, dwReg, data);
	if (error != e_Err_NoError)
	{
		printf("Driver reported an error in OpReg Read. Code=%d.\n", error);
		return 0;
	}
	
	value |= (1<<8);
	
	error = SI_PLX_WritePCI_OpReg((PLXDevice *)hPlx, 1, dwReg, data);
	if (error != e_Err_NoError)
	{
		printf("Driver reported an error in OpReg Read. Code=%d.\n", error);
		return 0;
	}
	
	error = SI_PLX_SetupCallbackEvent((PLXDevice *) hPlx, (void *)callback );
	if (error != e_Err_NoError)
	{
		printf("Setup callback failed. Press enter to continue. Code=%d.\n", error);
		getchar();
		return 0;
	}
	
	return 1;
}

void P9054_IntDisable (P9054_HANDLE hPlx)
{
	INT32 error;
	DWORD value, *data;
	DWORD dwReg= 0x68;	//Define as INTCSR

	data = &value;
	
	error = SI_PLX_ReadPCI_OpReg((PLXDevice *) hPlx, 1, dwReg, data);
	if (error != e_Err_NoError)
	{
		printf("Driver reported an error in OpReg Read. Code=%d.\n", error);
		return;
	}
	
	value &= ~(1<<8);
	
	error = SI_PLX_WritePCI_OpReg((PLXDevice *) hPlx, 1, dwReg, data);
	if (error != e_Err_NoError)
	{
		printf("Driver reported an error in OpReg Read. Code=%d.\n", error);
		return;
	}

}


BOOL P9054_Open
(
	//P9054_HANDLE *phPlx, DWORD dwVendorID, DWORD dwDeviceID, DWORD nCardNum
	P9054_HANDLE hPlx, DWORD dwVendorID, DWORD dwDeviceID, DWORD nCardNum
)
{
	INT32 error;

	error = SI_PLX_OpenDriver( nCardNum, (PLXDevice *)hPlx );
	if (error != e_Err_NoError)
	{
//		printf("Device Not Found. Press enter to continue. Code=%d.\n", error);
//		getchar();
//		return false;
	}

	return true;
}

//CLOSE DRIVER
void P9054_Close 
(
	P9054_HANDLE hPlx
)
{
	SI_PLX_CloseDriver((PLXDevice *) hPlx);

	return;
}

//DMA - Our Driver is passed a count*4 value
BOOL P9054_DMAReadWriteBlock 
(
	P9054_HANDLE hPlx, DWORD dwLocalAddr, PVOID buf, DWORD dwBytes, BOOL fIsRead, P9054_MODE mode, P9054_DMA_CHANNEL dmaChannel
)
{
	INT32 error;
	//BOOL err;
	//UINT32 ramp[kMaxBufferSize];
	//UINT32 cnt;

	UINT32* data;
	data = (UINT32 *)malloc(sizeof(UINT32));
	
	
	if (fIsRead)
	{
		if ( dwLocalAddr < 0x800000 )
		{
			*data = 0x10;
			SI_PLX_WriteTarget((PLXDevice *) hPlx, 0, 1, 0x800000, data);
		}
		
		//error = SI_PLX_ReadBlockDMA((PLXDevice *) hPlx, dwBytes/4, dwOffset, ramp);
		error = SI_PLX_ReadBlockDMA((PLXDevice *) hPlx, dwBytes, dwLocalAddr, (UINT32*) buf);
		if (error != e_Err_NoError)
		{
			printf("Driver reported an error in Busmastered Read. Code=%d.\n", error);
			return false;
		}
		printf("Successful Transfer, Data Read:\n");
/*		
		for(cnt=0;cnt<dwBytes;cnt++)
		{
			printf("0x%x ", ((UINT32*) buf)[cnt]);//ramp[cnt]);
			if( (cnt+1) % 16 == 0)
				printf("\n");
		}
*/
	}
	else
	{	
	
		// create a ramp pattern to dump to the FIFO
		//for(cnt=0; cnt<dwBytes; cnt++)
		//	ramp[cnt] = cnt;	
	
		if ( dwLocalAddr < 0x800000 )
		{
			*data = 0x11;
			SI_PLX_WriteTarget((PLXDevice *) hPlx, 0, 1, 0x800000, data);
		}
		

		//error = SI_PLX_WriteBlockDMA((PLXDevice *) hPlx, dwBytes/4, dwOffset, (UINT32*) buf );
		error = SI_PLX_WriteBlockDMA((PLXDevice *) hPlx, dwBytes, dwLocalAddr, (UINT32*) buf );
		if (error != e_Err_NoError)
		{
			printf("Driver reported an error in Busmastered Write. Code=%d.\n", error);
			return false;
		}
	
		printf("Successful Transfer, Data Written");
	}
	free(data);
	
	return true;
}

DWORD P9054_ReadReg (P9054_HANDLE hPlx, DWORD dwReg)
{
	INT32 error;
	UINT32 value, *data;

	data = &value;
	
	error = SI_PLX_ReadPCI_OpReg((PLXDevice *) hPlx, 1, dwReg, data);
	if (error != e_Err_NoError)
	{
		printf("Driver reported an error in OpReg Read. Code=%d.\n", error);
		return 0;
	}	
	else
	{
		return value;
	}
}
void P9054_WriteReg (P9054_HANDLE hPlx, DWORD dwReg, DWORD dwData)
{
	INT32 error;
	DWORD *data;
	
	data = &dwData;
	
	error = SI_PLX_WritePCI_OpReg((PLXDevice *) hPlx, 1, dwReg, data);
	if (error != e_Err_NoError)
	{
		printf("Driver reported an error in OpReg Read. Code=%d.\n", error);
		return;
	}
}


BYTE P9054_ReadByte (P9054_HANDLE hPlx, P9054_ADDR addrSpace, DWORD dwOffset)
{
	INT32 error = 0;
	DWORD bufferUINT8;
	DWORD region = 0;
	DWORD count = 1;
	
	error = SI_PLX_ReadTarget((PLXDevice *) hPlx, region, count, dwOffset, &bufferUINT8);
	if (error != e_Err_NoError)
	{
		printf("\nREAD FAILED. Code=%d.\n", error);
		return 0;
	}
	
	return bufferUINT8;
}

WORD P9054_ReadWord (P9054_HANDLE hPlx, P9054_ADDR addrSpace, DWORD dwOffset)
{
	INT32 error = 0;
	DWORD bufferUINT16;
	DWORD region = 0;
	DWORD count = 1;
	
	error = SI_PLX_ReadTarget((PLXDevice *) hPlx, region, count, dwOffset, &bufferUINT16);
	if (error != e_Err_NoError)
	{
		printf("\nREAD FAILED. Code=%d.\n", error);
		return 0;
	}
	
	return bufferUINT16;
}

DWORD P9054_ReadDWord (P9054_HANDLE hPlx, P9054_ADDR addrSpace, DWORD dwOffset)
{
	INT32 error = 0;
	DWORD bufferUINT32;
	DWORD region = 0;
	DWORD count = 1;
	
	error = SI_PLX_ReadTarget((PLXDevice *) hPlx, region, count, dwOffset, &bufferUINT32);
	if (error != e_Err_NoError)
	{
		printf("\nREAD FAILED. Code=%d.\n", error);
		return 0;
	}
	
	return bufferUINT32;
}

void P9054_WriteByte (P9054_HANDLE hPlx, P9054_ADDR addrSpace, DWORD dwOffset, BYTE data)
{
	INT32 error = 0;
	DWORD region = 0;
	DWORD count = 1;
	
	error = SI_PLX_WriteTarget8((PLXDevice *) hPlx, region, count, dwOffset, &data);
	if (error != e_Err_NoError)
	{
		printf("\nREAD FAILED. Code=%d.\n", error);
	}
	return;
}
void P9054_WriteWord (P9054_HANDLE hPlx, P9054_ADDR addrSpace, DWORD dwOffset, WORD data)
{
	INT32 error = 0;
	DWORD region = 0;
	DWORD count = 1;
	
	error = SI_PLX_WriteTarget16((PLXDevice *) hPlx, region, count, dwOffset, &data);
	if (error != e_Err_NoError)
	{
		printf("\nREAD FAILED. Code=%d.\n", error);
	}
	return;
}
void P9054_WriteDWord (P9054_HANDLE hPlx, P9054_ADDR addrSpace, DWORD dwOffset, DWORD data)
{
	INT32 error = 0;
	DWORD region = 0;
	DWORD count = 1;
	
	error = SI_PLX_WriteTarget((PLXDevice *) hPlx, region, count, dwOffset, &data);
	if (error != e_Err_NoError)
	{
		printf("\nREAD FAILED. Code=%d.\n", error);
	}
	return;
}


void P9054_ReadWriteBlock (P9054_HANDLE hPlx, DWORD dwOffset, PVOID buf, DWORD dwBytes, BOOL fIsRead, P9054_ADDR addrSpace, P9054_MODE mode)
{
	INT32 error;
	//UINT32 ramp[kMaxBufferSize];
	//UINT32 cnt;
	UINT32 region = 0;

	UINT32* data;
	data = (UINT32 *)malloc(sizeof(UINT32));
	
	// create a ramp pattern to dump to the sram
	//for(cnt=0; cnt<dwBytes; cnt++)
	//	ramp[cnt] = cnt;
	
	if (fIsRead)
	{
		if ( dwOffset < 0x800000 )
		{
			*data = 0x10;
			SI_PLX_WriteTarget((PLXDevice *) hPlx, 0, 1, 0x800000, data);
		}
		
		switch(mode)
		{
			case 0 :
				error = SI_PLX_ReadTarget8((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT8*) buf);
				break;
		
			case 1 :
				error = SI_PLX_ReadTarget16((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT16*) buf);
				break;
		
			case 2 :
				error = SI_PLX_ReadTarget((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT32*) buf);
				break;
		}
			
		if (error != e_Err_NoError)
		{
			printf("\nREAD FAILED. Code=%d.\n", error);
			return;
		}	
		printf("Successful Transfer, Data Read:\n");
	}
	else
	{		
		if ( dwOffset < 0x800000 )
		{
			*data = 0x11;
			SI_PLX_WriteTarget((PLXDevice *) hPlx, 0, 1, 0x800000, data);
		}
	
		switch(mode)
		{
			case 0 :
				error = SI_PLX_WriteTarget8((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT8*) buf);
				break;
		
			case 1 :
				error = SI_PLX_WriteTarget16((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT16*) buf);
				break;
		
			case 2 :
				error = SI_PLX_WriteTarget((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT32*) buf);
				break;
		}
		if (error != e_Err_NoError)
		{
			printf("\nWRITE FAILED. Code=%d.\n", error);
			return;
		}
		printf("Successful Transfer, Data Written");
	}
	
	return;
}
/////////////////////////////////////////

void P9054_ReadBlock (P9054_HANDLE hPlx, DWORD dwOffset, PVOID buf, DWORD dwBytes, P9054_ADDR addrSpace, P9054_MODE mode)
{
	INT32 error;
	UINT32 region = 0;

	UINT32* data;
	data = (UINT32 *)malloc(sizeof(UINT32));
	

	if ( dwOffset < 0x800000 )
	{
		*data = 0x10;
		SI_PLX_WriteTarget((PLXDevice *) hPlx, 0, 1, 0x800000, data);
	}
	
	switch(mode)
	{
		case 0 :
			error = SI_PLX_ReadTarget8((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT8*) buf);
			break;
	
		case 1 :
			error = SI_PLX_ReadTarget16((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT16*) buf);
			break;
	
		case 2 :
			error = SI_PLX_ReadTarget((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT32*) buf);
			break;
	}
		
	if (error != e_Err_NoError)
	{
		printf("\nREAD FAILED. Code=%d.\n", error);
		return;
	}
	
	printf("Successful Transfer, Data Read:\n");
	return;
}

void P9054_WriteBlock (P9054_HANDLE hPlx, DWORD dwOffset, PVOID buf, DWORD dwBytes, P9054_ADDR addrSpace, P9054_MODE mode)
{
	INT32 error;
//	UINT32 ramp[kMaxBufferSize];
//	UINT32 cnt;
	UINT32 region = 0;

	UINT32* data;
	data = (UINT32 *)malloc(sizeof(UINT32));
	
	// create a ramp pattern to dump to the FIFO
//	for(cnt=0; cnt<dwBytes; cnt++)
//		ramp[cnt] = cnt;
	
		
	if ( dwOffset < 0x800000 )
	{
		*data = 0x11;
		SI_PLX_WriteTarget((PLXDevice *) hPlx, 0, 1, 0x800000, data);
	}

	switch(mode)
	{
		case 0 :
			error = SI_PLX_WriteTarget8((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT8*) buf);
			break;
	
		case 1 :
			error = SI_PLX_WriteTarget16((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT16*) buf);
			break;
	
		case 2 :
			error = SI_PLX_WriteTarget((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT32*) buf);
			break;
	}
	if (error != e_Err_NoError)
	{
		printf("\nWRITE FAILED. Code=%d.\n", error);
		return;
	}
	printf("Successful Transfer, Data Written");

	
	return;
}
/////////////////////////////////////////

DWORD P9054_ReadPCIReg(P9054_HANDLE hPlx, DWORD dwReg)
{
	INT32 error;
	UINT32 count = 4;
	DWORD bufferUINT8[4];
	
//	printf("\nSI_PLX_ReadPCI_ConfSpace\n");
	// read PCI configuration space
	//printf("siabstract-dwReg= % x\n", dwReg);
	error = SI_PLX_ReadPCI_ConfSpace((PLXDevice *) hPlx, count, dwReg, bufferUINT8);	//pPLXDev
	if (error != e_Err_NoError)
	{
//		printf("Driver couldn't read configuration space. Code=%d.\n", error);
//		return 0;
	}
	return bufferUINT8[0];
}

void P9054_WritePCIReg(P9054_HANDLE hPlx, DWORD dwReg, DWORD dwData)
{
/*	INT32 error;
	UINT32 count = 4;
	
	printf("\nSI_PLX_WritePCI_ConfSpace\n");
	// read PCI configuration space
	error = SI_PLX_WritePCI_ConfSpace((PLXDevice *) hPlx, count, dwReg, &dwData);	//pPLXDev
	if (error != e_Err_NoError)
	{
		printf("Driver couldn't write configuration space. Code=%d.\n", error);
	}
*/
	return;
}




BOOL P9054_EEPROMWriteWord(P9054_HANDLE hPlx, DWORD dwOffset, WORD wData)
{
	INT32 error;
	UINT32 value, *data;
	UINT32 temp;

	data = &value;


	error = SI_PLX_ReadPCI_NVWord((PLXDevice *) hPlx, 1, dwOffset, data);
	if (error != e_Err_NoError)
	{
		printf("Driver reported an error in NvRam Read. Code=%d.\n", error);
		return 0;
	}

//	value = value & 0xFFFF0000;
//	value = value | (UINT32) wData;


	value = value & 0x0000FFFF;
	temp = (UINT32) wData;
	temp = temp << 16;
	value = temp | value;


	error = SI_PLX_WritePCI_NVWord((PLXDevice *) hPlx, 1, dwOffset, data);
	if (error != e_Err_NoError)
	{
		printf("\nWrite Failed. Code=%d.\n", error);
		return 0;
	}

	return 1;
}

BOOL P9054_EEPROMWriteDWord(P9054_HANDLE hPlx, DWORD dwOffset, DWORD dwData)
{
	INT32 error;
	UINT32 *data;

	data = &dwData;

	error = SI_PLX_WritePCI_NVWord((PLXDevice *) hPlx, 1, dwOffset, data);
	if (error != e_Err_NoError)
	{
		printf("\nWrite Failed. Code=%d.\n", error);
		return 0;
	}

	return 1;
}


BOOL P9054_EEPROMReadWord(P9054_HANDLE hPlx, DWORD dwOffset, PWORD pwData)
{
	INT32 error;
	UINT32 value, *data;

	data = &value;

	error = SI_PLX_ReadPCI_NVWord((PLXDevice *) hPlx, 1, dwOffset, data);//(UINT32*) pwData);
	if (error != e_Err_NoError)
	{
		printf("Driver reported an error in NvRam Read. Code=%d.\n", error);
		return 0;
	}
	
	value = value >> 16;
	*pwData = value;

	return 1;
}
BOOL P9054_EEPROMReadDWord(P9054_HANDLE hPlx, DWORD dwOffset, PDWORD pdwData)
{
	INT32 error;

	error = SI_PLX_ReadPCI_NVWord((PLXDevice *) hPlx, 1, dwOffset, (UINT32*) pdwData);
	if (error != e_Err_NoError)
	{
		printf("Driver reported an error in NvRam Read. Code=%d.\n", error);
		return 0;
	}

	return 1;
}



BOOL P9054_IsAddrSpaceActive(P9054_HANDLE hPlx, P9054_ADDR addrSpace)
{

	switch (addrSpace)
	{
		case(P9054_ADDR_REG):
			return true;
			break;
		default:
			return false;
			break;
	}

}

DWORD P9054_CountCards (DWORD dwVendorID, DWORD dwDeviceID)
{
/*	INT32 error;
	UINT32 cards, *data;

	data = &cards;

	error = SI_PLX_CountCards(&gPLXDev, dwVendorID, dwDeviceID, data);
	if (error != e_Err_NoError)
	{
		printf("Driver reported an error in SI_PLX_CountCards. Code=%d.\n", error);
		return 0;
	}

	return cards;
*/
return(0);
}

//##########################################################################
//      PLX 9030 CALLS [ADDED BY CHASE ON 2-6-2010
//##########################################################################


//OPEN DRIVER
BOOL P9030_Open (P9030_HANDLE hPlx, DWORD dwVendorID, DWORD dwDeviceID, DWORD nCardNum){
    return P9054_Open (hPlx, dwVendorID, dwDeviceID, nCardNum); }
void P9030_Close (P9030_HANDLE hPlx) { P9054_Close (hPlx); }

//##########################################################################

// access PCI configuration registers
DWORD P9030_ReadPCIReg(P9030_HANDLE hPlx, DWORD dwReg) { return P9054_ReadPCIReg(hPlx, dwReg); }
void P9030_WritePCIReg(P9030_HANDLE hPlx, DWORD dwReg, DWORD dwData) { P9054_WritePCIReg(hPlx, dwReg, dwData); }

//##########################################################################

// access registers
DWORD P9030_ReadReg (P9030_HANDLE hPlx, DWORD dwReg) { return P9054_ReadReg (hPlx, dwReg); }
void P9030_WriteReg (P9030_HANDLE hPlx, DWORD dwReg, DWORD dwData) { P9054_WriteReg (hPlx, dwReg, dwData); }

BYTE P9030_ReadByte (P9030_HANDLE hPlx, P9030_ADDR addrSpace, DWORD dwOffset) 
{
	INT32 error = 0;
	DWORD bufferUINT8;
	DWORD region = 0;
	DWORD count = 1;

	if (addrSpace == P9030_ADDR_SPACE0) region = 0; else region = 1;    //#################
	
	error = SI_PLX_ReadTarget((PLXDevice *) hPlx, region, count, dwOffset, &bufferUINT8);
	if (error != e_Err_NoError)
		return 0;
	
	return bufferUINT8;
}

void P9030_WriteByte (P9030_HANDLE hPlx, P9030_ADDR addrSpace, DWORD dwOffset, BYTE data)
{
	INT32 error = 0;
	DWORD region = 0;
	DWORD count = 1;

	if (addrSpace == P9030_ADDR_SPACE0) region = 0; else region = 1;    //#################

	error = SI_PLX_WriteTarget8((PLXDevice *) hPlx, region, count, dwOffset, &data);
	if (error != e_Err_NoError)
		return;

	return;
}


WORD P9030_ReadWord (P9030_HANDLE hPlx, P9030_ADDR addrSpace, DWORD dwOffset)
{
	INT32 error = 0;
	DWORD bufferUINT16;
	DWORD region = 0;
	DWORD count = 1;

	if (addrSpace == P9030_ADDR_SPACE0) region = 0; else region = 1;    //#################


	error = SI_PLX_ReadTarget((PLXDevice *) hPlx, region, count, dwOffset, &bufferUINT16);
	if (error != e_Err_NoError)
		return 0;
	
	return bufferUINT16;
}


void P9030_WriteWord (P9030_HANDLE hPlx, P9030_ADDR addrSpace, DWORD dwOffset, WORD data)
{
	INT32 error = 0;
	DWORD region = 0;
	DWORD count = 1;

	if (addrSpace == P9030_ADDR_SPACE0) region = 0; else region = 1;    //#################

	error = SI_PLX_WriteTarget16((PLXDevice *) hPlx, region, count, dwOffset, &data);
	if (error != e_Err_NoError)
		return;
	
	return;
}
DWORD P9030_ReadDWord (P9030_HANDLE hPlx, P9030_ADDR addrSpace, DWORD dwOffset)
{
	INT32 error = 0;
	DWORD bufferUINT32 = 0xDEADBEEF;
	DWORD region = 0;
	DWORD count = 1;

	if (addrSpace == P9030_ADDR_SPACE0) region = 0; else region = 1;    //#################

	error = SI_PLX_ReadTarget((PLXDevice *) hPlx, region, count, dwOffset, &bufferUINT32);
	if (error != e_Err_NoError)
		return 0;
	
	return bufferUINT32;
}


void P9030_WriteDWord (P9030_HANDLE hPlx, P9030_ADDR addrSpace, DWORD dwOffset, DWORD data)
{
	INT32 error = 0;
	DWORD region = 0;
	DWORD count = 1;

	if (addrSpace == P9030_ADDR_SPACE0) region = 0; else region = 1;    //#################

	error = SI_PLX_WriteTarget((PLXDevice *) hPlx, region, count, dwOffset, &data);
	if (error != e_Err_NoError)
		return;

	return;
}

void P9030_ReadWriteBlock (P9030_HANDLE hPlx, DWORD dwOffset, PVOID buf, DWORD dwBytes, BOOL fIsRead, 
                                  P9030_ADDR addrSpace, P9030_MODE mode)
{
	INT32 error;
	UINT32 region = 0;

	if (addrSpace == P9030_ADDR_SPACE0) region = 0; else region = 1;    //#################
	
	if (fIsRead)
	{

		switch(mode)
		{
			case 0 :
				error = SI_PLX_ReadTarget8((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT8*) buf);
				break;
		
			case 1 :
				error = SI_PLX_ReadTarget16((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT16*) buf);
				break;
		
			case 2 :
				error = SI_PLX_ReadTarget((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT32*) buf);
				break;
		}
			
		if (error != e_Err_NoError)
			return;
	}
	else
	{		
	
		switch(mode)
		{
			case 0 :
				error = SI_PLX_WriteTarget8((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT8*) buf);break;
		
			case 1 :
				error = SI_PLX_WriteTarget16((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT16*) buf);
				break;
		
			case 2 :
				error = SI_PLX_WriteTarget((PLXDevice *) hPlx, region, dwBytes, dwOffset, (UINT32*) buf);
				break;
		}
		if (error != e_Err_NoError)
			return;
	}
	
	return;
}

void P9030_ReadBlock (P9030_HANDLE hPlx, DWORD dwOffset, PVOID buf, DWORD dwBytes, P9030_ADDR addrSpace, P9030_MODE mode)
{
	P9030_ReadWriteBlock (hPlx, dwOffset, buf, dwBytes, true, addrSpace, mode);
}
void P9030_WriteBlock (P9030_HANDLE hPlx, DWORD dwOffset, PVOID buf, DWORD dwBytes, P9030_ADDR addrSpace, P9030_MODE mode)
{
	P9030_ReadWriteBlock (hPlx, dwOffset, buf, dwBytes, false, addrSpace, mode);
}

//##########################################################################

//EEPROM
BOOL P9030_EEPROMReadWord(P9030_HANDLE hPlx, DWORD dwOffset, PWORD pwData) 
{ 
	return P9054_EEPROMReadWord(hPlx, dwOffset, pwData); 
}
BOOL P9030_EEPROMWriteWord(P9030_HANDLE hPlx, DWORD dwOffset, WORD wData)
{
	return P9054_EEPROMWriteWord(hPlx, dwOffset, wData);
}
BOOL P9030_EEPROMReadDWord(P9030_HANDLE hPlx, DWORD dwOffset, PDWORD pdwData)
{
	return P9054_EEPROMReadDWord(hPlx, dwOffset, pdwData);
}
BOOL P9030_EEPROMWriteDWord(P9030_HANDLE hPlx, DWORD dwOffset, DWORD dwData)
{
	return P9054_EEPROMWriteDWord(hPlx, dwOffset, dwData);
}

//##########################################################################

// interrupt functions
BOOL P9030_IntIsEnabled (P9030_HANDLE hPlx)
{
	return P9054_IntIsEnabled (hPlx);
}
BOOL P9030_IntEnable (P9030_HANDLE hPlx) //, P9030_INT_HANDLER funcIntHandler);
{
	return P9054_IntEnable (hPlx);
}
void P9030_IntDisable (P9030_HANDLE hPlx)
{
	P9054_IntDisable (hPlx);
}

//##########################################################################

// misc. functions

BOOL P9030_IsAddrSpaceActive(P9030_HANDLE hPlx, P9030_ADDR addrSpace)
{
	switch (addrSpace)
	{
		case(P9054_ADDR_REG):
			return true;
			break;
		default:
			return false;
			break;
	}
}

DWORD P9030_CountCards (DWORD dwVendorID, DWORD dwDeviceID)
{
/*	INT32 error;
	UINT32 cards, *data;

	data = &cards;

	error = SI_PLX_CountCards(&gPLXDev, dwVendorID, dwDeviceID, data);
	if (error != e_Err_NoError)
		return 0;

	return cards;
*/
return(0);
}

