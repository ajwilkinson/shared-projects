////////////////////////////////////////////////////////////////////////////////
//	plx_ver_0204.c
//
//	Description:
//		PLX access that may use kernel 2.4.x specific calls
//
//	Rev Notes:
//		2002-11-01: Colin
//			Created.
//		2005-08-01: Colin
//			Upgraded for linux kernel 2.6.11
//

#include "main.h"


////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_ConfigRead
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

int SIPLX_ConfigRead
(
	int board_num, 
	ulong arg
)
{
	ulong cnt;
	ulong region, count, offset, buffer; 
	u32 value;
	ulong offsetSize, bufferSize;
	offsetSize = sizeof(ulong);
	bufferSize = sizeof(ulong);

	SIprintk0((kModuleName "-SIPLX_ConfigRead: Entering\n"));

	SIPLX_GetParams( arg, &region, &count, &offset, &buffer );

	for ( cnt=0; cnt<count; cnt++, buffer+=bufferSize, offset+=offsetSize)
	{
		pci_read_config_dword(pdx[board_num].dev, offset, &value);
		put_user( value, (ulong *)buffer );
	}

	SIprintk0((kModuleName "-SIPLX_ConfigRead: Exiting\n"));

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_NVRamRead
//
//	Description:
//		NOTE:	NVRAM is accessed only as DWORD. Do not attempt to make it byte.
//
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

int SIPLX_NVRamRead
(
	int board_num, 
	ulong arg
)
{
	ulong cnt, waitflag, done;
	ulong region, count, offset, buffer, value;
	ushort VPDAddr, VPDFlag;

	SIprintk0((kModuleName "-SIPLX_NVRamWrite: Entering\n"));

	SIPLX_GetParams( arg, &region, &count, &offset, &buffer );

	for ( cnt=0; cnt<count; cnt++ )
	{
		done = 0;
		do {
			VPDAddr = (ushort) offset | PLX_VPD_READ;
			VPDFlag = VPDAddr;

			pci_write_config_word
			(
				pdx[board_num].dev, 
				PLX_VPD_ADDR,
				VPDAddr
			);

			for(waitflag = 0; waitflag < 200; waitflag++)
			{
				pci_read_config_word
				(
					pdx[board_num].dev, 
					PLX_VPD_ADDR,
					&VPDFlag
				);

				if(VPDFlag != VPDAddr)
				{
					done = 1;
					break;
				}
				else
					udelay(1);
			}
		}while(!done);

		pci_read_config_dword
		(
			pdx[board_num].dev, 
			PLX_VPD_DATA,
			(u32 *)&value
		);

		put_user( value, (ulong *)buffer);

		buffer += sizeof(ulong);
		offset += sizeof(ulong);	
	}
	
	SIprintk0((kModuleName "-SIPLX_NVRamRead: Exiting\n"));

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_NVRamWrite
//
//	Description:
//		NOTE:	NVRAM is accessed only as DWORD. Do not attempt to make it byte.
//
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

int SIPLX_NVRamWrite
(
	int board_num, 
	ulong arg
)
{
	ulong cnt, waitflag, done;
	ulong region, count, offset, buffer, value;
	ushort VPDAddr, VPDFlag;

	SIprintk0((kModuleName "-SIPLX_NVRamWrite: Entering\n"));

	SIPLX_GetParams( arg, &region, &count, &offset, &buffer );

	for ( cnt=0; cnt<count; cnt++ )
	{
		get_user( value, (ulong *)buffer );
		
		done = 0;
		do {

			VPDAddr = (ushort) offset | PLX_VPD_WRITE;
			VPDFlag = VPDAddr;
		
			pci_write_config_dword
			(
				pdx[board_num].dev, 
				PLX_VPD_DATA,
				(u32)value
			);
		
			pci_write_config_word
			(
				pdx[board_num].dev, 
				PLX_VPD_ADDR,
				VPDAddr
			);
	
			for(waitflag = 0; waitflag < 200; waitflag++)
			{
				pci_read_config_word
				(
					pdx[board_num].dev, 
					PLX_VPD_ADDR,
					&VPDFlag
				);

				if(VPDFlag != VPDAddr)
				{
					done = 1;
					break;
				}
				else
					udelay(1);
			}
		}while(!done);
		
		buffer += sizeof(ulong);
		offset += sizeof(ulong);	
	}
	
	SIprintk0((kModuleName "-SIPLX_NVRamWrite: Exiting\n"));

	return 0;
}

int SIPLX_GetAddOn(struct inode *inode, struct file *filp, unsigned long arg)
{
	uint board_num;
	ulong intCSR, INTCSR;
	
	SIprintk0((kModuleName "- Entering SheldonTransferAddOn\n"));
	
	board_num = iminor(inode);
	
	INTCSR = pdx[board_num].dev_intcsr;
	
	if(!SIPLX_do_mmap(inode, filp, arg))
		return -1;

	//enable Lint and db ints
	intCSR = readl((void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + INTCSR));
	intCSR |= 0xB00;
	writel(intCSR, (void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + INTCSR));

	//write dspbase of 0x00C00000 (byte offset is left shift by 2)
	// Only true for SIC33 board. Set this to required value for your HW.
	// This can also be overwritten in application code
	writel
	(
		(0x00C00000 << 2) | 0xFC000000,
		(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_DMLBAM * 4))
	);

	//write hostphys address and enable
	writel
	(
		(
			(ulong)(__pa(pdx[board_num].dma.pmem)) & 0xffff0000
		) + 1,
		(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_DMPBAM * 4))
	);

	SIprintk0((kModuleName "- Exiting SheldonTransferAddOn\n"));

	return 1;	
}
