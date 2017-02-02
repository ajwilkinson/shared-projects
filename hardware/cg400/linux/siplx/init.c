////////////////////////////////////////////////////////////////////////////////
//	init_ver_0204.c
//
//	Description:
//		Board initializtion routine		
//	Rev Notes:
//		2001-11-01: Colin
//			Created.
//		2005-08-01: Colin
//			Upgraded for linux kernel 2.6.11
//			removed the lock pages macro, do it ourselves
//			init new work queue to do bottom half
//		2008-09-16: Whipple
//		        Implemened Meyer changes
//		2009-03-05: Whipple
//		        Put back interrupt code mistakenly removed
//

#include "main.h"

int SIPLX_board_init
(
	int card_num
);

int SIPLX_release
(
	struct inode *inode, struct file *filp
);


////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_board_init(int card_num, uchar bus, uchar func)
//
//	Description:
//		Initialize board for given card/bus/func.
//
//	Parameters:
//
//	Return value:
//

int SIPLX_board_init(int card_num)
{
	uint temp_base_addr[PCI_MAX_NUM_REGS];
	uint addr_size[PCI_MAX_NUM_REGS];
	uchar temp_irq;
	int cnt;
	u16 chip;
	uchar bus, func;
	
	SIprintk0
	((
		kModuleName "-SIPLX_board_init: "
	));
	
	// Get IRQ
	//if(pci_read_config_byte(pdx[card_num].dev, PCI_INTERRUPT_LINE, &temp_irq)<0)  
	//	temp_irq = 0;
	//No drivers should read the interrupt number from config
	//registers directly (if they do, they are broken and should be fixed as
	//this scheme already breaks on the Ultra), they should use pci_dev->irq
	//instead which can be set by arch-dependent fixup function. [...]
	temp_irq = pdx[card_num].dev->irq;

	bus = pdx[card_num].dev->bus->number;
	func = pdx[card_num].dev->devfn;
	SIprintk0
	((
		"	card_num=%d, bus=%d, func=%d, \n"
		"	PCI_SLOT(func)=%d, PCI_FUNC(func)=%d\n", 
		card_num, bus, func, 
		PCI_SLOT(func), PCI_FUNC(func)
	));

	if (temp_irq == 0)
		SIprintk2((kModuleName "-SIPLX_board_init: WARNING-cannot get IRQ\n"));

	for(cnt = 0; cnt < PCI_MAX_NUM_REGS; cnt++)	{
	
		pdx[card_num].mem[cnt].base_addr = 0;
		temp_base_addr[cnt] = pci_resource_start(pdx[card_num].dev, cnt);
		addr_size[cnt] = ((pci_resource_end(pdx[card_num].dev, cnt))
							- temp_base_addr[cnt]);

		if(!addr_size[cnt])	{
			SIprintk1
			((
				kModuleName "-SIPLX_board_init: region %d is non existent\n",
				cnt
			));
			continue;			
		}

		addr_size[cnt] += 1;
		
		if(pdx[card_num].dev->resource[cnt].flags & IORESOURCE_IO) {

			SIprintk1
			((
				kModuleName "-SIPLX_board_init: region %d is IO mapped.\n", cnt
			));
			//addr_size[cnt] &= PCI_BASE_ADDRESS_IO_MASK;
			// if the device is IO mapped, add code here to support it.
			//	This DDK only works with mem mapped devices, so
			//	temp_base_addr and addr_size are not saved to pdx.

			if
			(
				!request_region
				(
					temp_base_addr[cnt], 
					addr_size[cnt], kModuleName
				)
			)
			{
				SIprintk3
				((
					kModuleName "-SIPLX_board_init: ERROR-"
					"region=%d, base=0x%x, size=0x%x failed request_region"
					"with %d\n", 
					cnt,
					temp_base_addr[cnt],
					addr_size[cnt], 
					-EBUSY
				));
				return -EBUSY;
			}
	
			// now the io port region is good. save it to pdx.
			pdx[card_num].mem[cnt].base_addr = temp_base_addr[cnt];
			pdx[card_num].mem[cnt].size = addr_size[cnt];
		}
		else {

			SIprintk1
			((
				kModuleName "-SIPLX_board_init: region %d is mem mapped.\n", cnt
			));
			
			SIprintk1
			((
				kModuleName "-SIPLX_board_init: "
				"addr_size[%d]=%d, temp_base_addr[%d]=0x%x\n", 
				cnt, addr_size[cnt], cnt, temp_base_addr[cnt]
			));
			
			// request_mem_region (or request_region) locks the virtual 
			//	address memory region. it's unlocked by release_mem_region
			//	(or release_region)
			
			if
			(
				!request_mem_region
				(
					temp_base_addr[cnt], 
					addr_size[cnt], kModuleName
				)
			)
			{
				SIprintk3
				((
					kModuleName "-SIPLX_board_init: ERROR-"
					"region=%d, base=0x%x, size=0x%x failed\
					request_mem_region"
					"with %d\n", 
					cnt,
					temp_base_addr[cnt],
					addr_size[cnt], 
					-EBUSY
				));
				return -EBUSY;
			}
			
			// ioremap is used to get kernel virtual address to access
			//	physical memory. temp_base_addr[cnt] becomes virtual addr
			//	to access the physical device.
			temp_base_addr[cnt] = 
				(uint) ioremap
				(
					temp_base_addr[cnt], addr_size[cnt]
				);			


			// now the mem mapped region is good. save it to pdx.
			pdx[card_num].mem[cnt].base_addr = temp_base_addr[cnt];
			pdx[card_num].mem[cnt].size = addr_size[cnt];

		}	// end if io or mem mapped

		SIprintk1
		((
			kModuleName
			"-SIPLX_board_init: region %d, size 0x%x, base addr=0x%x\n", 
			cnt, addr_size[cnt], temp_base_addr[cnt]
		));
	}
		

	// driver is block to block transfer mode in the beginning
	pdx[card_num].driverConfig.blockPoint = SI_CONFIGDRIVER_TRANSFERBLOCK;
	pdx[card_num].writetransfer.timer_value = 0;
	pdx[card_num].readtransfer.timer_value = 0;

	if (pci_read_config_word(pdx[card_num].dev, 0x2E, &chip)<0) {

		SIprintk2
		((
			kModuleName 
			"-SIPLX_board_init: WARNING-cannot read SubsystemID\n"
		));
		pdx[card_num].chip = e_DevType_9030;
	}
	else {
		if (chip == 0x9054)
			pdx[card_num].chip = e_DevType_9054;
		else
			pdx[card_num].chip = e_DevType_9030;
	}	
	
	switch (pdx[card_num].chip)	{
		case e_DevType_9030:
			pdx[card_num].dev_intcsr = PLX9030_INTCSR;
			break;
		case e_DevType_9054:
			pdx[card_num].dev_intcsr = PLX_INTCSR * 4;
			break;
	}

	pdx[card_num].irq = (uint) temp_irq;

	// IRQ may be valid. register it.

	if ( ( pdx[card_num].irq > 0 ) && ( pdx[card_num].irq < 0xff ) ) {
		if
		(
// IRQF_SHARED replaced SA_SHIRQ in 2.6.24 kernel 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
                        request_irq
                        (
                                pdx[card_num].irq, SIPLX_isr,
                                IRQF_SHARED, kModuleName, (ulong *)(card_num + 1)
                        )
#else
        		request_irq
		        (
		        	pdx[card_num].irq, SIPLX_isr,
		        	SA_SHIRQ, kModuleName, (ulong *)(card_num + 1)
		        )
#endif
		)
		{
			SIprintk3
			((
				kModuleName "-SIPLX_board_init: "
				"ERROR-Interrupt Request failed.\n"
				"    pdx[card_num].irq = %d, SIPLX_isr = 0x%x\n", 
				pdx[card_num].irq, (int)SIPLX_isr
			));
	
			pdx[card_num].irq = 0;
		}
		else {

			SIprintk1
			((
				kModuleName "-SIPLX_board_init: found IRQ %d\n", pdx[card_num].irq
			));
		}
	}
	else {

		SIprintk1
		((
			kModuleName "-SIPLX_board_init: "
			"No IRQ found.\n"
		));
		pdx[card_num].irq = 0;
		pdx[card_num].init = TRUE;
		
		// this is not error. board may not be IRQ capable.
		return 0;
	}


	SIprintk1
	((
		kModuleName "-SIPLX_board_init: "
		"Setting up work queue.\n"
	));
	// we have IRQ. we can continue with setting up DMA.
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)	
	INIT_WORK(&pdx[card_num].bh, SIPLX_isr_bottomhalf, (void *)(card_num + 1));
#else
	INIT_WORK(&pdx[card_num].bh, (void *) SIPLX_isr_bottomhalf);
#endif
	//if its 9030 or 9050 exit out
	if (pdx[card_num].chip != e_DevType_9054) {
		pdx[card_num].init = TRUE;
		return 0;
	}
	
	pdx[card_num].dma.dmrr = 
		readl
		(
			(void *)(pdx[card_num].mem[PLX_OPREGS].base_addr + ((ulong)0x1C))
		);

	pdx[card_num].dma.dmrr = ~pdx[card_num].dma.dmrr;
	pdx[card_num].dma.order = 0;
	for (cnt = pdx[card_num].dma.dmrr/PAGE_SIZE; cnt > 0;) {
		if (cnt < 2) {
			++pdx[card_num].dma.order;
			cnt = 0;
		}
		else {
			cnt = cnt/2;
			++pdx[card_num].dma.order;
		}
	}	

	pdx[card_num].readtransfer.status = __RW_LOCK_UNLOCKED(readtransfer);
	pdx[card_num].writetransfer.status = __RW_LOCK_UNLOCKED(writetransfer);
	pdx[card_num].readtransfer.setup = FALSE;
	pdx[card_num].writetransfer.setup = FALSE;

	SIprintk1
	((
		kModuleName "-SIPLX_board_init: "
		"allocating memory.\n"
	));
	if(SI_MemAlloc_Cont(&pdx[card_num].dma.pmem, pdx[card_num].dma.order)) {
		if(SI_MemAlloc_Cont(&pdx[card_num].dma.rpdscrpt, DESCRPT_ORDER))
			if(SI_MemAlloc_Cont(&pdx[card_num].dma.wpdscrpt, DESCRPT_ORDER))
				pdx[card_num].init = TRUE;
	}		

	SIprintk0
	((
		kModuleName "-SIPLX_board_init: Exiting\n"
	));
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
int SI_MemAlloc_Cont(ulong **ptr, u32 order) {

	u32 addr;
	
	if ((order>(MAX_ORDER-1)) || (order<0))	{
		printk("Continous memory region requested out of range\n");
		return(-1);
	}

	*ptr = (ulong *)__get_free_pages((GFP_DMA | GFP_ATOMIC), order);
	if (ptr == NULL)
		return(-1);
	
	for (addr=(ulong)*ptr; addr < (((ulong)*ptr)+(PAGE_SIZE<<order)); addr+=PAGE_SIZE)
		set_bit(PG_reserved, &((virt_to_page(addr))->flags));		
	
	return(1);
}

////////////////////////////////////////////////////////////////////////////////
void SI_MemDealloc_Cont(ulong *ptr, u32 order) {

	u32 addr;
	
	if(ptr!=NULL) {
		for (addr=(ulong)ptr; addr < (((ulong)ptr)+(PAGE_SIZE<<order)); addr+=PAGE_SIZE)
		{
			clear_bit(PG_reserved, &((virt_to_page(addr))->flags));
		}

		__free_pages(virt_to_page((u32)ptr), order);
	}
}

