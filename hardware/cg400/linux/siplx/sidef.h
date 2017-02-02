////////////////////////////////////////////////////////////////////////////////
//	sidef.h
//
//	Description:
//		This file should be included in all driver C files. This file defines
//		all driver related includes, data structures and #defines.
//		
//	Rev Notes:
//		200x-xx-xx: RobU and TonyH
//			Created.
//		2002-07-26: Av
//			Added comments 
//		2002-07-29: mik
//			Added kModuleName #define.
//			Added debug level switches as follows. Use these in each CPP file 
//			before #include driver.h.
//				#define	SIDebugLevelAll	// turn on all debugs.
//				#define SIDebugLevel0	// used for enter/exit function messages
//				#define SIDebugLevel1	// used for informational messages (not essential)
//				#define SIDebugLevel2	// used for warning messages
//				#define SIDebugLevel3	// used for error messages
//
//				#define SIDebugLevel10	// user defined
//				#define SIDebugLevel11	// user defined
//				#define SIDebugLevel12	// user defined
//				#define SIDebugLevel13	// user defined
//		2002-08-15: mik
//			Fixed to work with kernel 2.2 (no BM yet).
//		2005-08-01: Colin
//			Upgraded for linux kernel 2.6.11
//

#include <linux/types.h>
#include <linux/pci.h>



////////////////////////////////////////////////////////////////////////////////
// SI defines.

#define kModuleName	"siplx"

#include <linux/cdev.h>
#include "../common/90xxdef.h"
#include "../common/siddkapi_lnx.h"

////////////////////////////////////////////////////////////////////////////////

#define MIN(a, b) ( ((a) < (b)) ? (a) : (b) )

////////////////////////////////////////////////////////////////////////////////
#define DESCRPT_ORDER			0x2
#define DESCRPT_PAGES			(0x2 << (DESCRPT_ORDER - 1))
	
#define PCI_MAX_NUM_REGS 6
#define PCI_MAX_DEV 8

//These defines are for the DSP card from Sheldon Instruments
#define VENDOR_ID 0x80bb
#define DEVICE_ID 0x1042

#define ORDER2N 2

#define uchar unsigned char
#define BOOLEAN uint
#define FALSE 0
#define TRUE 1

////////////////////////////////////////////////////////////////////////////////

enum chipset 
{
	e_DevType_9030,
	e_DevType_9054,	
};

struct base_address_regions 
{
	char access;
	uint base_addr;
	uint size;
	uchar *phys;
};

struct descrpt 
{
	ulong pciaddr;
	ulong localaddr;
	ulong size;
	ulong next;
};

struct dma
{
	ulong *pmem;
	ulong order;

	ulong *rpdscrpt;
	ulong *wpdscrpt;

	ulong dmrr;
	ulong lowpart;
};

struct transfer_info 
{
	unsigned long long filler;
	ulong filler2;
	struct descrpt *Descriptor;  // Must be quad word aligned
	struct page **pages;
	ulong nr_pages;
	ulong bytes_requested;
	ulong bytes_remaining;
	ulong user_addr;
	ulong size;
	ulong local_addr;
	ulong list_index;
	ulong nr_buf;
	struct timer_list time_out;
	ulong timer_value;
	rwlock_t status;
	BOOLEAN in_progress;
	BOOLEAN just_completed;
	BOOLEAN timed_out;
	BOOLEAN demandMode;
	BOOLEAN aborted;
	BOOLEAN setup;
};


struct t_driverConfig
{
	ulong blockPoint;
};

struct phys_device_ext 
{
	ulong major_num;
	BOOLEAN init;
	struct  pci_dev *dev;
	struct base_address_regions mem[PCI_MAX_NUM_REGS];
	uint irq;
	struct fasync_struct *async_queue;
	struct work_struct bh;
	struct transfer_info readtransfer;
	struct transfer_info writetransfer;
	int t_type;
	struct transfer_info nonbustransfer;
	uchar bus_num;
	uchar func_num;
	ulong dev_intcsr;
	
	enum chipset chip;
	
	struct dma dma;
	
	// driver config for busy, count, offset, etc
	struct t_driverConfig driverConfig;	
	
	struct cdev cdev;
};

////////////////////////////////////////////////////////////////////////////////
// pdx stores values used by the driver. It is global and used by all.

extern struct phys_device_ext pdx[];


////////////////////////////////////////////////////////////////////////////////
// debug level based printk

#ifdef SIDebugLevelAll

#define SIDebugLevel0	// used for enter/exit function messages
#define SIDebugLevel1	// used for informational messages (not essential)
#define SIDebugLevel2	// used for warning messages
#define SIDebugLevel3	// used for error messages

#define SIDebugLevel10	// user defined
#define SIDebugLevel11	// user defined
#define SIDebugLevel12	// user defined
#define SIDebugLevel13	// user defined

#endif

//	Individual SIprintk levels
#ifdef SIDebugLevel0
#define	SIprintk0(x)	printk	x
#else
#define	SIprintk0(x)
#endif

#ifdef SIDebugLevel1
#define	SIprintk1(x)	printk x
#else
#define	SIprintk1(x)
#endif

#ifdef SIDebugLevel2
#define	SIprintk2(x)	printk	x
#else
#define	SIprintk2(x)
#endif

#ifdef SIDebugLevel3
#define	SIprintk3(x)	printk	x
#else
#define	SIprintk3(x)
#endif

#ifdef SIDebugLevel10
#define	SIprintk10(x)	printk	x
#else
#define	SIprintk10(x)
#endif

#ifdef SIDebugLevel11
#define	SIprintk11(x)	printk	x
#else
#define	SIprintk11(x)
#endif

#ifdef SIDebugLevel12
#define	SIprintk12(x)	printk	x
#else
#define	SIprintk12(x)
#endif

#ifdef SIDebugLevel13
#define	SIprintk13(x)	printk	x
#else
#define	SIprintk13(x)
#endif

