////////////////////////////////////////////////////////////////////////////////
//	main.h
//
//	Description:
//
//	Rev Notes:
//		200x-xx-xx: RobU and TonyH
//			Created.
//		2002-07-29: mik
//			Moved kModuleName #define to sidef.h
//		2002-08-01: mik
//			Fixed indenting (cosmetics).
//			Changed plxdefs.h to ../common/90xxdef.h
//		2002-08-08: mik
//			Changed all func names to SI_xxx.
//			Changed to only contain outside visible functions.
//		2002-08-15: mik
//			Fixed to work with kernel 2.2 (no BM yet).
//		2002-11-01: Colin
//			Re-write/Update of driver for kernel 2.4.x
//		2005-08-01: Colin
//			Upgraded for linux kernel 2.6.11
//		2008-09-16: Whipple
//			Implemented Meyer changes
//
#include <linux/types.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/stat.h>
#include <linux/version.h>

#include <generated/autoconf.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/mmzone.h> //has MAX_ORDER
#include <asm/page.h>
#include <asm/pgtable.h>
#include <linux/poll.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/mman.h>
#include <asm/bitops.h>
#include <linux/highmem.h>
//#include <linux/smp_lock.h>
#include <linux/rwlock_types.h>
#include <linux/pagemap.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>

#include "sidef.h"
#include "isr.h"
#include "plx.h"
#include "init.h"
#include "busmaster.h"

#ifndef CONFIG_PCI
#	error	"This driver needs PCI support to be available"
#endif

int SIPLX_release
(
	struct inode *inode, struct file *filp
);

int SIPLX_fasync
(
	int fd, struct file *filp, int mode
);

int SIPLX_do_mmap
(
	struct inode *inode, 
	struct file *filp, 
	unsigned long arg
);

long SIPLX_ioctl
(
//	struct inode *inode, struct file *filp,
	struct file *filp,
	unsigned int cmd, unsigned long arg
);
