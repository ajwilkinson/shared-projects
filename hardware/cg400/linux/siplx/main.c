////////////////////////////////////////////////////////////////////////////////
//	main.c
//
//	Description:
//		Main entry point for SIPLX kernel module driver. Note that devID for
//		IRQ is board number + 1. For example, devID is 1 for board 0.
//
//	Rev Notes:
//		2001-??-??: robu
//			Created.
//		2002-07-26: mik
//			Modified to work with kernel 2.4.
//		2002-07-31: mik
//			Made it work with card that has no IRQ (hence, no DMA)
//		2002-08-08: mik
//			Changed all func names to SI_xxx.
//		2002-08-15: mik
//			Fixed to work with kernel 2.2 (no BM yet).
//		2002-10-07: mik
//			Parameterized VID and DID.
//		2002-11-01: Colin
//			Re-write/Update of driver for kernel 2.4.x
//		2005-08-01: Colin
//			Upgraded for linux kernel 2.6.11
//			new module entry and cleanup function attatching
//			new remap range requirments in mmap
//			new fops
//		2008-09-16: Whipple
//			Implemented Meyer changes
//

////////////////////////////////////////////////////////////////////////////////
#ifndef MODULE
#define MODULE
#endif

#ifndef __KERNEL__
#define __KERNEL__
#endif

// this contains LINUX_VERSION_CODE
#include  <linux/module.h>


#include "main.h"

int plx_major, plx_minor;

#define VM_RESERVED (VM_DONTEXPAND|VM_DONTDUMP)


////////////////////////////////////////////////////////////////////////////////
// Non-exported function prototypes


int SIPLX_open
(
	struct inode *inode, struct file *filp
);

unsigned int SIPLX_poll
(
	struct file *fp, poll_table *pTable
);

ssize_t SIPLX_read
(
	struct file *fp, char *buf, size_t count, loff_t *f_pos
);

ssize_t SIPLX_write
(
	struct file *fp, const char *buf, size_t count, loff_t *f_pos
);

int SIPLX_mmap
(
	struct file *filp, struct vm_area_struct *vma
);

// phys_device is defined in main.h
struct phys_device_ext pdx[PCI_MAX_DEV];

int SI_VID = VENDOR_ID, SI_DID = DEVICE_ID;
module_param(SI_VID, int, 0);
module_param(SI_DID, int, 0);
MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Colin Hankins");


////////////////////////////////////////////////////////////////////////////////

// this struct is used to setup "driver callbacks" for linux
static struct file_operations plx_fops = 
{
	owner: 		THIS_MODULE,
	read:   	SIPLX_read,
	write:  	SIPLX_write,
	poll: 		SIPLX_poll,
	unlocked_ioctl: SIPLX_ioctl,
	mmap:		SIPLX_mmap,
	open:   	SIPLX_open,
	release:	SIPLX_release,
	fasync:  	SIPLX_fasync
};

////////////////////////////////////////////////////////////////////////////////
//	int init_module(void)
//
//	Description:
//		Entry point for the driver.
//
//	Parameters:
//
//	Return value:
//

static int __init module_entry(void) 
{
	int result;
	int card_num;
	struct pci_dev *from = NULL;
	dev_t dev;
				
	SIprintk0(("\n\n\n\n"));
	SIprintk0
	((
		kModuleName "-init_module: Entering - VID=0x%x, DID=0x%x\n", 
		SI_VID, SI_DID
	));
	
	/*result = register_chrdev(0, kModuleName, &plx_fops);*/
	result = alloc_chrdev_region(&dev, 0, PCI_MAX_DEV, kModuleName);

	if(result < 0) {
		SIprintk3((kModuleName "-init_module: ERROR-register_chrdev < 0\n"));
		return result;
	}
	
	plx_major = MAJOR(dev);
	plx_minor = MINOR(dev);
	
	// clear pdx.
	memset((char *) &pdx[0], 0, sizeof(pdx));	// clears all

	// iterate all PCI cards with same VID and DID.
	for (card_num = 0; card_num < PCI_MAX_DEV;) 
	{
		pdx[card_num].major_num = plx_major;
		SIprintk1
		((
			kModuleName "-init_module: Searching for card_num=%d\n" , 
			card_num
		));
		pdx[card_num].dev = pci_get_device(SI_VID, SI_DID, from);
		from = pdx[card_num].dev;
		if(!pdx[card_num].dev)
			break;
			
		cdev_init(&pdx[card_num].cdev, &plx_fops);
		pdx[card_num].cdev.owner = THIS_MODULE;
		pdx[card_num].cdev.ops = &plx_fops;
		result = cdev_add(&pdx[card_num].cdev, MKDEV(plx_major, plx_minor + card_num), 1);
		
		if(result)
			printk("<1>" kModuleName "-init_module: Error %d adding pdx[%d].cdev", result, card_num);

		if(pci_enable_device(pdx[card_num].dev))
			continue;		
		
		result = SIPLX_board_init(card_num);
		if(result) {
			// on error, unload driver.
			SIprintk3
			((
				kModuleName "-init_module: Warning-Unable to init "
				"board %d. Error code %d.\n", 
				card_num, result
			));
			cdev_del(&pdx[card_num].cdev);	
			cleanup_module();
			return result;
		}			

		card_num += 1;
	}

	if (card_num == 0) {
		// no card found, unload driver.
		SIprintk3((kModuleName "-init_module: error-no card found.\n"));
		cleanup_module();		
		return -ENODEV;
	}
	SIprintk0((kModuleName "-init_module: Exiting. Successful init_module\n"));

	return result;	// result should be 0.
}

////////////////////////////////////////////////////////////////////////////////
//	void cleanup_module(void)
//
//	Description:
//		Exit point for the driver.
//
//	Parameters:
//
//	Return value:
//

static void module_cleanup(void)
{
	uint cnt;
	int card_num;
	
	dev_t devno = MKDEV(plx_major, plx_minor);


	//*SIprintk0((kModuleName "-cleanup_module: Entering\n"));*/
	printk(KERN_ALERT "Cleanup Module:begin\n");

	// This code will abort any active DMA when the module is unloaded
	for(card_num = 0; card_num < PCI_MAX_DEV; card_num++) 
	{
		if(pdx[card_num].init != TRUE) 
			continue;

		// card was initialized. does it have IRQ?
		if(pdx[card_num].irq == 0) 
		{
			SIprintk1
			((
				kModuleName "-cleanup_module: no IRQ for card %d\n", 
				card_num
			));
			continue;
		}

		// Free IRQ.
		SIprintk1((kModuleName "-cleanup_module: free IRQ.\n"));		
		free_irq(pdx[card_num].irq, (ulong *)(card_num+1));

		cdev_del(&pdx[card_num].cdev);		
	}	// end for card_num

	SIprintk1((kModuleName "-cleanup_module: Unregistering chrdev.\n"));
			
	unregister_chrdev_region(devno, PCI_MAX_DEV);
	
	// free mem regions that were allocated.
	for(card_num = 0; card_num < PCI_MAX_DEV; card_num++) 
	{

		if(pdx[card_num].init != TRUE)

			break;
						
		for(cnt = 0; cnt < PCI_MAX_NUM_REGS; cnt++) 
		{
			if(pdx[card_num].mem[cnt].size)
			{
				if(pdx[card_num].dev->resource[cnt].flags & IORESOURCE_MEM) 
				{
					iounmap((ulong *)pdx[card_num].mem[cnt].base_addr);
					release_mem_region
					(
						pci_resource_start(pdx[card_num].dev, cnt),
						pdx[card_num].mem[cnt].size
					);
					SIprintk1
					((
						kModuleName "-cleanup_module: "
						"released mem region %d for card %d\n", cnt, card_num
					));
				}
				else if(pdx[card_num].dev->resource[cnt].flags & IORESOURCE_IO) 
				{
					release_region
					(
						pci_resource_start(pdx[card_num].dev, cnt),
						pdx[card_num].mem[cnt].size
					);
					SIprintk1
					((
						kModuleName "-cleanup_module: "
						"released io region %d for card %d\n", cnt, card_num
					));
				} 
			}
		}
		SIprintk1((kModuleName "-cleanup_module: Free Allocated Mem.\n"));		
		SI_MemDealloc_Cont(pdx[card_num].dma.pmem, pdx[card_num].dma.order);
		SI_MemDealloc_Cont(pdx[card_num].dma.rpdscrpt, DESCRPT_ORDER);
		SI_MemDealloc_Cont(pdx[card_num].dma.wpdscrpt, DESCRPT_ORDER);
	}
	
	/*SIprintk0((kModuleName "-cleanup_module: Exiting.\n"));*/
	printk(KERN_ALERT "Cleanup Module:end\n");
	
}

//let the kernel know what our entry and exit points are
module_init(module_entry);
module_exit(module_cleanup);

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_open(int card_num, uchar bus, uchar func)
//
//	Description:
//
//	Parameters:
//
//	Return value:
//

int SIPLX_open(struct inode *inode, struct file *filp)
{
	int board_num;

	SIprintk0
	((
		kModuleName "-SIPLX_open: Entering\n"
	));

	board_num = iminor(inode);

	if(board_num >= PCI_MAX_DEV) 
	{
		SIprintk3
		((
			kModuleName "-SIPLX_open: ERROR-"
			"board_num >= PCI_MAX_DEV %d\n", 
			board_num
		));
		return -ENXIO;
	}

	if(!pdx[board_num].init) 
	{
		SIprintk3
		((
			kModuleName "-SIPLX_open: ERROR-"
			"!pdx[%d].init\n", board_num
		));
		return -ENXIO;
	}
  
	SIprintk0
	((
		kModuleName "-SIPLX_open: Exiting\n"
	));

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_release(int card_num, uchar bus, uchar func)
//
//	Description:
//
//	Parameters:
//
//	Return value:
//

int SIPLX_release(struct inode *inode, struct file *filp)
{
	int board_num;

	SIprintk0
	((
		kModuleName "-SIPLX_release: Entering\n"
	));

	board_num = iminor(inode);

	SIPLX_fasync(-1, filp, 0);

	SIprintk0
	((
		kModuleName "-SIPLX_release: Exiting\n"
	));

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_poll(int card_num, uchar bus, uchar func)
//
//	Description:
//
//	Parameters:
//
//	Return value:
//

unsigned int SIPLX_poll(struct file *fp, poll_table *pTable)
{
	SIprintk0
	((
		kModuleName "-SIPLX_poll\n"
	));

	// Implementation is application dependent

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_read(int card_num, uchar bus, uchar func)
//
//	Description:
//
//	Parameters:
//
//	Return value:
//

ssize_t SIPLX_read(struct file *fp, char *buf, size_t count, loff_t *f_pos)
{
	SIprintk0
	((
		kModuleName "-SIPLX_read: Entering\n"
	));
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_write(int card_num, uchar bus, uchar func)
//
//	Description:
//
//	Parameters:
//
//	Return value:
//

ssize_t SIPLX_write(struct file *fp, const char *buf, size_t count, loff_t *f_pos)
{
	SIprintk0
	((
		kModuleName "-SIPLX_write\n"
	));

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_ioctl
//
//	Description:
//
//	Parameters:
//		struct inode *inode	:
//		struct file *filp	:
//		unsigned int cmd	:
//		unsigned long arg	:
//
//	Return Values:
//		-EINVAL	:
//

long SIPLX_ioctl
(
//	struct inode *inode, struct file *filp,
	struct file *filp,
	unsigned int cmd, unsigned long arg
)
{
	int result = 0;
	int board_num;
	u32 count, dspaddr, hostaddr;
	
	struct inode *inode = filp->f_path.dentry->d_inode;
	
	SIprintk0((kModuleName "-SIPLX_ioctl: Entering\n"));

	board_num = iminor(inode);

	switch(cmd)
	{
	case IOCTL_SHELDON_CONFIG_READ:
		result = SIPLX_ConfigRead(board_num, arg);
		break;
	
	case IOCTL_SHELDON_OPREG_WRITE:
		result = SIPLX_OpregWrite(board_num, arg);
		break;
	
	case IOCTL_SHELDON_OPREG_READ:
		result = SIPLX_OpregRead(board_num, arg);
		break;
	
	case IOCTL_SHELDON_NVRAM_WRITE:
		result = SIPLX_NVRamWrite(board_num, arg);
		break;

	case IOCTL_SHELDON_NVRAM_READ:
		result = SIPLX_NVRamRead(board_num, arg);
		break;
	
	case IOCTL_SHELDON_PASSTHROUGH_WRITE_8:
		result = 
			SIPLX_PassthroughDo
			( 
				board_num, arg, FALSE, 1, sizeof(uchar)
			);
		break;
	
	case IOCTL_SHELDON_PASSTHROUGH_READ_8:
		result = 
			SIPLX_PassthroughDo
				( 
					board_num, arg, TRUE, 1, sizeof(uchar)
				);
		break;

	case IOCTL_SHELDON_PASSTHROUGH_WRITE_16:
		result = 
			SIPLX_PassthroughDo
			( 
				board_num, arg, FALSE, 2, sizeof(ushort)
			);
		break;

	case IOCTL_SHELDON_PASSTHROUGH_READ_16:
		result = 
			SIPLX_PassthroughDo
			( 
				board_num, arg, TRUE, 2, sizeof(ushort)
			);
		break;

	case IOCTL_SHELDON_PASSTHROUGH_WRITE_32:
		result = 
			SIPLX_PassthroughDo
			( 
				board_num, arg, FALSE, 4, sizeof(ulong)
			);
		break;

	case IOCTL_SHELDON_PASSTHROUGH_READ_32:
		result = 
			SIPLX_PassthroughDo
			( 
				board_num, arg, TRUE, 4, sizeof(ulong)
			);			
		break;
	case IOCTL_SHELDON_DRIVER_CONFIG:
		get_user(pdx[board_num].driverConfig.blockPoint, (ulong *)arg);
		break;	
	case IOCTL_SHELDON_GET_ADDON_ADDR:
		if(pdx[board_num].chip != e_DevType_9054)
			return -1;

		if
		(
			SIPLX_GetAddOn
			(
				inode, filp, arg
			)
		)
			break;
		
		result = 1;
		break;	
	case IOCTL_SHELDON_BUSMASTERED_TIMEOUT:
		get_user(pdx[board_num].writetransfer.timer_value,(ulong *)arg);
		get_user(pdx[board_num].readtransfer.timer_value,(ulong *)arg);
		break;
	case IOCTL_SHELDON_BUSMASTERED_READ:
		if(pdx[board_num].chip != e_DevType_9054)
			return -1;
			
		get_user(dspaddr, (u32 *)arg);
		get_user(count, (u32 *)(arg + sizeof(u32)));
		get_user(hostaddr, (u32 *)(arg + (2 * sizeof(u32))));
		
		if (SIPLX_doBlockDMA_Read 
		(
			board_num, (ulong)dspaddr, (ulong)hostaddr,
			(ulong)count, TRUE
		) != 0)
			return -1;
		break;
	case IOCTL_SHELDON_BUSMASTERED_WRITE:
		if(pdx[board_num].chip != e_DevType_9054)
			return -1;
			
		get_user(dspaddr, (u32 *)arg);
		get_user(count, (u32 *)(arg + sizeof(u32)));
		get_user(hostaddr, (u32 *)(arg + (2 * sizeof(u32))));

		if (SIPLX_doBlockDMA_Write 
		(
			board_num, (ulong)dspaddr, (ulong)hostaddr,
			(ulong)count, TRUE
		) != 0)
			return -1;
		break;
	case IOCTL_SHELDON_BUSMASTERED_READ_DONE:
		if(pdx[board_num].chip != e_DevType_9054)
			return -1;

		read_lock(&pdx[board_num].readtransfer.status);
		put_user
		(
		pdx[board_num].readtransfer.in_progress,(ulong *)arg
		);
		read_unlock(&pdx[board_num].readtransfer.status);
		break;
	case IOCTL_SHELDON_BUSMASTERED_WRITE_DONE:
		if(pdx[board_num].chip != e_DevType_9054)
			return -1;

		read_lock(&pdx[board_num].writetransfer.status);
		put_user
		(
		pdx[board_num].writetransfer.in_progress,(ulong *)arg
		);
		read_unlock(&pdx[board_num].writetransfer.status);
		break;
	case IOCTL_SHELDON_CANCEL_BUSMASTERING:
		SIPLX_CancelDMA(board_num);
		break;	
	case IOCTL_SHELDON_HARD_RESET:
		result = SIPLX_release(inode, filp);
		break;

	default:
	printk("hit default\n");
		SIprintk1((kModuleName "-SIPLX_ioctl:Default case\n"));
		return -EINVAL;
	}

	SIprintk0((kModuleName "-SIPLX_ioctl: Exiting\n"));

	return result;
}

////////////////////////////////////////////////////////////////////////////////
int SIPLX_mmap(struct file *filp, struct vm_area_struct *vma) 
{
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long size = vma->vm_end - vma->vm_start;
	
	if (offset & ~PAGE_MASK) {
		SIprintk1((kModuleName "-SIPLX_mmap: offset not aligned\n"));
		return -ENXIO;
	}

	if ((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED)) {
		SIprintk1((kModuleName "-SIPLX_mmap: writeable mappings must be shared\n"));
		return -EINVAL;
		}
		
	if (offset >= __pa(high_memory) || (filp->f_flags & O_SYNC))
		vma->vm_flags |= VM_IO;
	
	vma->vm_flags |= VM_RESERVED | VM_LOCKED; //dont want the area swapped
	if(remap_pfn_range(vma, vma->vm_start, (__pa(offset)) >> PAGE_SHIFT,
		 size, vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
int SIPLX_do_mmap(struct inode *inode, struct file *filp, unsigned long arg) 
{
	unsigned long uAdr;
	unsigned long size;
	unsigned long board_num;
		
	board_num = iminor(inode);
	size = PAGE_SIZE << pdx[board_num].dma.order;

	if (size & ~PAGE_MASK) {
		SIprintk1((kModuleName "-SIPLX_do_mmap: offset not aligned\n"));
		return -ENXIO;
	}
		
	if (size > (PAGE_SIZE << pdx[board_num].dma.order)) {
		SIprintk1((kModuleName "-SIPLX_do_mmap: size is too big\n"));
		return -ENXIO;
	}
	
	task_lock(current->group_leader);
	current->signal->rlim[RLIMIT_MEMLOCK].rlim_cur = RLIM_INFINITY;
	current->signal->rlim[RLIMIT_MEMLOCK].rlim_max = RLIM_INFINITY;
	task_unlock(current->group_leader);

	down_write(&current->mm->mmap_sem);

//    unsigned long pgoff = (((u32)pdx[board_num].dma.pmem) & PAGE_MASK) >> PAGE_SHIFT;
//    unsigned long populate = 0;

    uAdr = vm_mmap(filp, 0,
         (size + ~PAGE_MASK) & PAGE_MASK, PROT_READ | PROT_WRITE,
         MAP_SHARED | MAP_LOCKED, (((u32)pdx[board_num].dma.pmem) & PAGE_MASK));


/*  	uAdr = do_mmap (filp, 0,
                  (size + ~PAGE_MASK) & PAGE_MASK,
		  PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_LOCKED, 
		  ((u32)pdx[board_num].dma.pmem) & PAGE_MASK);

*/

	up_write(&current->mm->mmap_sem);
	
	if(uAdr <= 0)
		return -1;
	
	put_user(uAdr, (u32 *)arg);
	
	return 1;	
}

