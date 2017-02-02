////////////////////////////////////////////////////////////////////////////////
/*

90xxdef.h

Sheldon Instruments, Inc.

Abstract:

	Include file for all PLX related defines.
	

Revision History:

    2001-??-??: lru
		Created.
	2002-07-12: mik
		Changed NUM_REGIONS to 8000 to accomodate larger BMDMA transfers.
	2002-08-01: mik
		Merged the different defines from Linux so bot Win and Linux can use
		this same file.
	2004-08-23 : colin
		Added defines for SI specific use of PLX elements like doorbell bits.
*/

//
// These are the valid values for the register arguement to OP R/W or Mailbox R/W
//

#define PLX_OPREGS	0
#define NUM_REGIONS 8000	//Set this number to your max transfer size/4096 bytes

// LOCAL CONFIGURATION REGISTERS
//		PLX_MARBR is mirrored at 0x02 and 0x2B
#define PLX_LAS0RR	0x00
#define PLX_LAS0BA	0x01
#define PLX_MARBR	0x02
#define PLX_BIGEND	0x03
#define PLX_EROMRR	0x04
#define PLX_EROMBA	0x05
#define PLX_LBRD0	0x06
#define PLX_DMRR	0x07
#define PLX_DMLBAM	0x08
#define PLX_DMLBAI	0x09
#define PLX_DMPBAM	0x0A
#define PLX_DMCFGA	0x0B

#define PLX_LAS1RR	0x3C
#define PLX_LAS1BA	0x3D
#define PLX_LBRD1	0x3E
#define PLX_DMDAC	0x3F


// RUNTIME REGISTERS
// When QSR[0] == 0, I2O becomes disabled 
//		PLX_MB0_IQP = PLX_MB0 
//		PLX_MB1_OQP = PLX_MB1 
// When QSR[0] == 1, I2O becomes enabled 
//		PLX_MB0_IQP = PLX_IQP
//		PLX_MB1_OQP = PLX_OQP
#define PLX_MB0_IQP	0x10
#define PLX_MB1_OQP	0x11
#define PLX_MB2		0x12
#define PLX_MB3		0x13
#define PLX_MB4		0x14
#define PLX_MB5		0x15
#define PLX_MB6		0x16
#define PLX_MB7		0x17
#define PLX_MB8		0x17 //Legacy
#define PLX_P2LDB	0x18
#define PLX_L2PDB	0x19
#define PLX_INTCSR	0x1A
#define PLX_CNTRL	0x1B
#define PCIHIDR		0x1C
#define PCIHREV		0x1D
#define PLX_MB0		0x1E
#define PLX_MB1		0x1F

// DMA REGISTERS
//		PLX_MARBR is mirrored at 0x02 and 0x2B
#define PLX_DMAMODE0	0x20
#define PLX_DMAPADR0	0x21
#define PLX_DMALADR0	0x22
#define PLX_DMASIZ0 	0x23
#define PLX_DMADPR0	0x24
#define PLX_DMAMODE1	0x25
#define PLX_DMAPADR1	0x26
#define PLX_DMALADR1	0x27
#define PLX_DMASIZ1	0x28
#define PLX_DMADPR1	0x29
#define PLX_DMACSR	0x2A
#define PLX_DMATHR	0x2C
#define PLX_DMADAC0	0x2D
#define PLX_DMADAC1	0x2E

// These two defines are not quad aligned. These are
// the absolute addresses for the DMACSR registers.
// These defines are used when performing writeb/readb
// to/from the DMACSR. 

#define PLX_DMACSR0     0xA8
#define PLX_DMACSR1     0xA9

// MESSAGE QUEUE REGISTERS (I2O)
// When QSR[0] == 0, I2O becomes disabled 
//		PLX_MB0_IQP = PLX_MB0 
//		PLX_MB1_OQP = PLX_MB1 
// When QSR[0] == 1, I2O becomes enabled 
//		PLX_MB0_IQP = PLX_IQP
//		PLX_MB1_OQP = PLX_OQP
#define PLX_OPQIS	0x0C
#define PLX_OPQIM	0X0D
#define PLX_IQP		0x10
#define PLX_OQP		0x11

#define PLX_MQCR	0x30
#define PLX_QBAR	0x31
#define PLX_IFHPR	0x32
#define PLX_IFTPR	0x33
#define PLX_IPHPR	0x34
#define PLX_IPTPR	0x35
#define PLX_OFHPR	0x36
#define PLX_OFTPR	0x37
#define PLX_OPHPR	0x38
#define PLX_OPTPR	0x39
#define PLX_QSR		0x3A

// CNTRL. These are for PLX9054 control register.
#define PLX_RELOAD_CONFIG_REGISTERS 0x20000000 // Bit 29
#define PLX_SOFTWARE_RESET          0x40000000 // Bit 30

//DMAMODE defines
#define PLX_8BIT	0x00000000
#define	PLX_16BIT	0x00000001
#define	PLX_32BIT	0x00000002
#define	PLX_0WS		0x00000000
#define	PLX_1WS		0x00000004
#define	PLX_2WS		0x00000008
#define	PLX_3WS		0x0000000C
#define	PLX_4WS		0x00000010
#define	PLX_5WS		0x00000014
#define	PLX_6WS		0x00000018
#define	PLX_7WS		0x0000001C
#define	PLX_8WS		0x00000020
#define	PLX_9WS		0x00000024
#define	PLX_10WS	0x00000028
#define	PLX_11WS	0x0000002C
#define	PLX_12WS	0x00000030
#define	PLX_13WS	0x00000034
#define	PLX_14WS	0x00000038
#define	PLX_15WS	0x0000003C
#define	PLX_TARDY	0x00000040
#define PLX_BTERM	0x00000080
#define	PLX_LOCBURS	0x00000100
#define	PLX_SCATTER 0x00000200
#define	PLX_DINT		0x00000400
#define	PLX_LOCINC		0x00000800
#define	PLX_DEMAND		0x00001000
#define	PLX_INVALIDATE	0x00002000
#define	PLX_EOT			0x00004000
#define	PLX_FASTTERM	0x00008000
#define	PLX_CLEARCOUNT	0x00010000
#define	PLX_PCIINT		0x00020000
#define	PLX_DACCHAIN	0x00040000

//DMADPR defines
#define	PLX_LOCAL	0x00000000
#define	PLX_PCI		0x00000001
#define	PLX_EOC		0x00000002
#define PLX_TINT	0x00000004
#define PLX_READ	0x00000008
#define PLX_WRITE	0x00000000

//DMACSR defines
#define PLX_ENABLE0	0x00000001
#define PLX_START0	0x00000002
#define PLX_ABORT0	0x00000004
#define PLX_CLEARI0	0x00000008
#define PLX_DONE	0x00000010
#define PLX_ENABLE1	0x00000100
#define PLX_START1	0x00000200
#define PLX_ABORT1	0x00000400
#define PLX_CLEARI1	0x00000800
#define PLX_DON1	0x00001000

//INTCSR
#define PLX_TEAIE	0x00000001
#define PLX_TEAE	0x00000002
#define PLX_SERRI	0x00000004
#define PLX_MBIE	0x00000008
#define PLX_PMIE	0x00000010
#define PLX_PMI 	0x00000020
#define PLX_LDPCE 	0x00000040
#define PLX_LDPC	0x00000080
#define PLX_PCIIE	0x00000100
#define PLX_PDBIE	0x00000200
#define PLX_PAIE 	0x00000400
#define PLX_LIIE	0x00000800
#define PLX_RAE		0x00001000
#define PLX_PDBI	0x00002000
#define PLX_PAI 	0x00004000
#define PLX_LII		0x00008000
#define PLX_LOIE	0x00010000
#define PLX_LDBIE	0x00020000
#define PLX_DMA0IE	0x00040000
#define PLX_DMA1IE	0x00080000
#define PLX_LDBI	0x00100000
#define PLX_DMA0I	0x00200000
#define PLX_DMA1I	0x00400000
#define PLX_BISTI	0x00800000
#define PLX_PCIABORT	0x01000000
#define PLX_DMA0ABORT	0x02000000
#define PLX_DMA1ABORT	0x04000000
#define PLX_256ABORT	0x08000000
#define PLX_PMB0		0x10000000
#define PLX_PMB1		0x20000000
#define PLX_PMB2		0x40000000
#define PLX_PMB3		0x80000000

//DMPBAM
#define PLX_PIMAE		0x00000001
#define PLX_PIIOAE		0x00000002
#define PLX_PICE		0x00000004
#define PLX_PIRPSC1		0x00000008
#define PLX_PIPRM		0x00000010
#define PLX_PAFF		0x000000E0
#define PLX_MWIM		0x00000200
#define PLX_PIPL		0x00000800
#define PLX_IORS		0x00002000
#define PLX_PIWD		0x0000C000
#define PLX_REMAPLOCALTOPCI 0xffff0000

#define	PLX_VPD_ADDR	0x4E
#define	PLX_VPD_DATA	0x50
#define PLX_VPD_WRITE	0x8000
#define PLX_VPD_READ	0x0000

#define PLX_NUM_BASE_ADDR_REGS			6
#define PLX_NT_LOCAL_ADDRESS_BADDR		2 //The first local region would normally start here (NT)
#define PLX_LOCAL_ADDRESS_BADDR			1 //but the pnp (95/WDM) does not put the I/O opregs in the Baddr array
#define PLX_FIRST_LOCAL_ADDRESS_REGION	0
//Pass through regions include Expansion ROM
#define PLX_LAST_LOCAL_ADDRESS_REGION	2
#define PLX_EPROM_ADDRESS_BADDR			4

#define PLX_MAX_TRANSFER_SIZE	67108864	// 2^26, max bytes per busmastered transfer

// Begin PLX9030 specific defines that is important to the driver.
// For full register definitions, see PLX9030 databook.
#define PLX9030_INTCSR	0x4c >> 2
#define PLX9030_LINT1	0x0004
#define PLX9030_LINT1_CLEAR	0x0400
#define PLX9030_LINT2	0x0020
#define PLX9030_LINT2_CLEAR	0x0800


//SI defines for use of PLX specific hardware fucntions
#define SI_PLXDB_AddOn		0x1
#define SI_PLXDB_Message	0x2
