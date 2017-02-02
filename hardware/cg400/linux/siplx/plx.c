////////////////////////////////////////////////////////////////////////////////
//	plx.c
//
//	Description:
//
//	Rev Notes:
//		200x-xx-xx: RobU and TonyH
//			Created.
//		2002-07-26: Av
//			Added comments and fixed indenting (cosmetics).
//		2002-08-01: Av
//			Added comments and fixed indenting (cosmetics).
//		2002-08-06: mik
//			Fixed all functions to use standard parameter model.
//		2002-08-07: mik
//			Changed all func names to SIPLX_xxx.
//		2002-08-15: mik
//			Fixed to work with kernel 2.2 (no BM yet).
//		2002-08-22: mik
//			Fixed NVRAM read/write to work with DWORD.
//			PLX can only use DWORD, not byte or word.
//		2002-11-01: Colin
//			Re-write/Update of driver for kernel 2.4.x
//		2005-08-01: Colin
//			Upgraded for linux kernel 2.6.11
//

//todo: add these to common
#define SI_CONFIGDRIVER_TYPE_DEMANDMODE		2
#define SI_CONFIGDRIVER_INDEX_DEMANDMODE_RW		SI_CONFIGDRIVER_INDEX_TYPE+1
#define SI_CONFIGDRIVER_INDEX_DEMANDMODE_MODE	SI_CONFIGDRIVER_INDEX_TYPE+2
//todo: end



#ifndef MODULE
#define MODULE
#endif

#ifndef __KERNEL__
#define __KERNEL__
#endif

#include "main.h"

#include <asm/uaccess.h>

#include <asm/io.h>

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_GetParams
//
//	Description:
//
//	Parameters:
//		int board_num: 
//		ulong arg	 :	 
//
//	Return Values:
//		

int SIPLX_GetParams
(
	ulong arg, ulong *region, ulong *count, ulong *offset, ulong *buffer
)
{
	// get app level params.
	get_user
	(
		 (*region), 
		 (ulong *) ( arg + sizeof(ulong *) * SI_PARAMS_INDEX_REGION ) 
	);
	
	get_user
	(
		(*count),	
		(ulong *) ( arg + sizeof(ulong *) * SI_PARAMS_INDEX_COUNT ) 
	);
	
	get_user
	(
		(*offset),	
		(ulong *) ( arg + sizeof(ulong *) * SI_PARAMS_INDEX_OFFSET ) 
	);
	
	get_user
	(
		(*buffer),	
		(ulong *) ( arg + sizeof(ulong *) * SI_PARAMS_INDEX_USERBUFFER ) 
	);
	
	SIprintk1
	((
		kModuleName "-SIPLX_GetParams: \n"
		"	arg=0x%lx, region=0x%lx, count=0x%lx, offset=0x%lx, buffer=0x%lx\n", 
		arg, *region, *count, *offset, *buffer
	));
		
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_PassthroughDo
//
//	Description:
//
//	Parameters:
//		int board_num: 
//		ulong arg	 :	 
//
//	Return Values:
//		

int SIPLX_PassthroughDo
(
	int board_num, ulong arg, 
	int readFromPlx, ulong plxDataSize, ulong hostDataSize
)
{
	ulong cnt, value;
	ulong space, count, offset, buffer;
	uint plxBaseAddr;

	SIprintk0((kModuleName "-SIPLX_PassthroughDo: Entering\n"));
	
	// get app level params.
	SIPLX_GetParams( arg, &space, &count, &offset, &buffer );
	
	if
	(
			space > PLX_LAST_LOCAL_ADDRESS_REGION 
		||	space < PLX_FIRST_LOCAL_ADDRESS_REGION
	)
		return -EINVAL;

	if ( count == 0)
		return -EINVAL;
		
	if( offset > pdx[board_num].mem[space + 2].size)
		return -EINVAL;
	
	if( buffer == 0)
		return -EINVAL;

	plxBaseAddr = pdx[board_num].mem[space + 2].base_addr +	offset;
	for(cnt = 0; cnt<count; cnt++)
	{
		if ( readFromPlx )
		{	
			// Read from PLX
			// 3. read from device			
			// 4. write to app level buffer. 
			switch (hostDataSize)
			{
			case sizeof(uchar): // byte
				value = readb( (void *)plxBaseAddr );
				put_user( (uchar)value, (uchar *)buffer );
				break;
			case sizeof(ushort): // short
				value = readw( (void *)plxBaseAddr );
				put_user( (ushort)value, (ushort *)buffer );
				break;
			case sizeof(ulong):	// ulong
				value = readl( (void *)plxBaseAddr );
				put_user( (ulong)value, (ulong *)buffer );
				break;
			default:
				return -EINVAL;
			}
		}
		else
		{
			// Write to PLX
			// 3. get value from app level buffer. 
			// 4. write to device.
			switch (hostDataSize)
			{
			case sizeof(uchar): // byte
				get_user( value, (uchar *)buffer );
				writeb( (uchar)value, (void *)plxBaseAddr );
				break;
			case sizeof(ushort): // short
				get_user( value, (ushort *)buffer );
				writew( (ushort)value, (void *)plxBaseAddr );
				break;
			case sizeof(ulong):	// ulong
				get_user( value, (ulong *)buffer );
				writel( (ulong)value, (void *)plxBaseAddr );
				break;
			default:
				return -EINVAL;
			}
		}
		
		// if blocktoblocktransfer 	// increment plx addr
		if 
		(
			pdx[board_num].driverConfig.blockPoint 
			== SI_CONFIGDRIVER_TRANSFERBLOCK
		)
			plxBaseAddr += plxDataSize;

		// increment app level addr
		buffer += hostDataSize;
	}
	
	SIprintk0((kModuleName "-SIPLX_PassthroughDo: Exiting\n"));
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_OpregRead
//
//	Description:
//		region	: ignore
//		count	: DWORD count
//		offset	: byte offset
//		buffer	: DWORD buffer
//
//	Parameters:
//		int board_num: 
//		ulong arg	 :	 
//
//	Return Values:
//		

int SIPLX_OpregRead
(
	int board_num, 
	ulong arg
)
{
	ulong cnt;
	ulong region, count, offset, buffer, value;
	ulong offsetSize, bufferSize;
	offsetSize = sizeof(ulong);
	bufferSize = sizeof(ulong);

	SIprintk0((kModuleName "-SIPLX_OpregRead: Entering\n"));

	SIPLX_GetParams( arg, &region, &count, &offset, &buffer );

	for ( cnt=0; cnt<count; cnt++, buffer+=bufferSize, offset+=offsetSize)
	{
		value = 
			readl
			(
				(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + offset)
			);
		put_user( value, (ulong *)buffer );
	}

	SIprintk0((kModuleName "-SIPLX_OpregRead: Exiting\n"));

	return 0;
}
	

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_OpregWrite
//
//	Description:
//		region	: ignore
//		count	: DWORD count
//		offset	: byte offset
//		buffer	: DWORD buffer
//
//	Parameters:
//		int board_num: 
//		ulong arg	 :	 
//
//	Return Values:
//		

int SIPLX_OpregWrite
(
	int board_num, 
	ulong arg
)
{
	ulong cnt;
	ulong region, count, offset, buffer, value;
	ulong offsetSize, bufferSize;
	offsetSize = sizeof(ulong);
	bufferSize = sizeof(ulong);

	SIprintk0((kModuleName "-SIPLX_OpregWrite: Entering\n"));

	SIPLX_GetParams( arg, &region, &count, &offset, &buffer );

	for ( cnt=0; cnt<count; cnt++, buffer+=bufferSize, offset+=offsetSize)
	{
		get_user( value, (ulong *)buffer );
		writel
		(
			value, 
			(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + offset)
		);
	}
		
	SIprintk0((kModuleName "-SIPLX_OpregWrite: Exiting\n"));

	return 0;

}

