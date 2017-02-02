////////////////////////////////////////////////////////////////////////////////
//	busmaster.c
//
//	Description:
//		busmaster for plx		
//	Rev Notes:
//		2001-11-01: Colin
//			Created.
//		2002-11-25: Colin
//			Added SG DMA
//		2005-08-01: Colin
//			Upgraded for linux kernel 2.6.11
//			means no more kiobufs, do all the page handling ourselves 
//		2008-09-161: Whipple
//			Fixed NRPAGES(removed ';')
//

#include "main.h"

////////////////////////////////////////////////////////////////////////////////
//macro to determine number of pages in an Xfer
#define NRPAGES(addr, cnt)	(((addr & ~PAGE_MASK) + cnt + ~PAGE_MASK) >> PAGE_SHIFT)

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_doBlockDMA_Read
//
//	Description:
//
//	Parameters:
//	int board_num, 
//	ulong dspaddr, ulong hostaddr, ulong count,
//	int start
//
//	Return Values:
//	int 

int SIPLX_doBlockDMA_Read
(
	int board_num, 
	ulong dspaddr, ulong hostaddr, ulong count,
	int start
)
{
	struct transfer_info *transfer_info = NULL;
	ulong intcsr_reg;
	ulong dmamode_reg;
	ulong dmamodeAddress;
	ulong dmapadrAddress;
	ulong dmaladrAddress;
	ulong dmasizAddress;
	ulong dmadprAddress;
	ulong enableBit;
	ulong startBit;
	ulong interruptBit;
	ulong dmacsrAddress;

	int error;
	
	ulong iosize, i;
	ulong max_size, nr_pages;
	ulong startAddr, localbusAddr, dmadprstuff;
	
	SIprintk0((kModuleName "-SIPLX_doBlockDMA: Entering\n"));

	
	if (start == TRUE)
		while 
		(
			pdx[board_num].readtransfer.in_progress 
		);

	if 
	(
		!pdx[board_num].readtransfer.setup 
	)
	{
		
		//transfer in progress
		pdx[board_num].readtransfer.in_progress = TRUE;

	}
	
	//convert word to byte
	count *= 4;

	//determine transfer size
	max_size = (DESCRPT_PAGES * PAGE_SIZE / sizeof(struct descrpt)) * PAGE_SIZE;
	max_size -= (hostaddr & ~PAGE_MASK);
	iosize = count < max_size ? count : max_size;

	nr_pages = NRPAGES(hostaddr, count);
	pdx[board_num].readtransfer.nr_pages = nr_pages;

	SIprintk1
	((
		kModuleName "-SIPLX_doBlockDMA: "
		"Getting Page mem.\n" 
	));

	if 
	(
		(pdx[board_num].readtransfer.pages = kmalloc(nr_pages * sizeof(*pdx[board_num].readtransfer.pages), GFP_KERNEL)) 
		== NULL
	)
		return -ENOMEM;

	//quad word align 
	pdx[board_num].readtransfer.Descriptor = (struct descrpt*)
		(((ulong)pdx[board_num].dma.rpdscrpt + 0xf)& ~0xf);

	//build transfer list
	error = SIPLX_Build_SGL
	(
		dspaddr, hostaddr, count, 
		pdx[board_num].readtransfer.Descriptor, 
		pdx[board_num].readtransfer.pages, READ
	);
	
	if ( error < nr_pages ) {
		pdx[board_num].readtransfer.in_progress = FALSE;
		kfree(pdx[board_num].readtransfer.pages);
		
		SIprintk1
		((
			kModuleName "-SIPLX_doBlockDMA: ERROR! "
			"Number of pages=0x%x Number requested=0x%x\n", error, (u32)nr_pages
		));
		
		return -error;
	}

	//set values to handle transfer of remaining bytes
	pdx[board_num].readtransfer.user_addr = hostaddr + iosize;
	pdx[board_num].readtransfer.local_addr = dspaddr + iosize;
	pdx[board_num].readtransfer.bytes_requested = iosize;
	pdx[board_num].readtransfer.bytes_remaining = count - iosize;

	transfer_info = &pdx[board_num].readtransfer;

	//setup PLX register and start the Xfer
	dmamodeAddress = 
		pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_DMAMODE1 * 4);

	dmapadrAddress = 
		pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_DMAPADR1 * 4);
		
	dmaladrAddress = 
		pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_DMALADR1 * 4);
		
	dmasizAddress  = 
		pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_DMASIZ1 * 4);
		
	dmadprAddress  = 
		pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_DMADPR1 * 4);

	enableBit      = 0x1;
	startBit       = 0x2;
	interruptBit   = PLX_DMA1IE;
	dmacsrAddress = pdx[board_num].mem[PLX_OPREGS].base_addr + PLX_DMACSR1;
	startAddr =
		transfer_info->Descriptor[0].pciaddr;

	if (iosize > (PAGE_SIZE - (hostaddr & ~PAGE_MASK)))
		dmadprstuff =
		(__pa(&pdx[board_num].readtransfer.Descriptor[0].pciaddr) | PLX_READ | PLX_PCI);
	else	
		dmadprstuff = 
			pdx[board_num].readtransfer.Descriptor[0].next;

	localbusAddr = 
			transfer_info->Descriptor[0].localaddr;

	// Disable interrupts
	intcsr_reg = 
		readl
		(
			(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_INTCSR * 4))
		);
	intcsr_reg &= ~interruptBit;
	writel
	(
		intcsr_reg,
		(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_INTCSR * 4))
	);

	// Set up DMA registers
	writel( startAddr, (void *)(dmapadrAddress) );
	writel( localbusAddr, (void *)(dmaladrAddress) );
	writel( transfer_info->Descriptor[0].size, (void *)(dmasizAddress) );
	writel( dmadprstuff, (void *)(dmadprAddress) );

	// Set DMA mode
	dmamode_reg = 0;
	dmamode_reg |= (transfer_info->demandMode ? PLX_DEMAND : PLX_LOCBURS);
	dmamode_reg |= PLX_TARDY;
	dmamode_reg |= PLX_32BIT | PLX_DINT | PLX_PCIINT | PLX_SCATTER;

	//if it is a point transfer, set all local addresses
	if 
	(
		pdx[board_num].driverConfig.blockPoint 
		== SI_CONFIGDRIVER_TRANSFERPOINT
	)
	{
		for (i=0; i < nr_pages; i++)
			transfer_info->Descriptor[i].localaddr = dspaddr;		

		pdx[board_num].readtransfer.local_addr = dspaddr;
		pdx[board_num].writetransfer.local_addr = dspaddr;

		dmamode_reg |= PLX_LOCINC;
	}
	
	writel(dmamode_reg, (void *)(dmamodeAddress));

	SIprintk1
	((
		kModuleName "-SIPLX_doBlockDMA: "
		"dmamode_reg=0x%lx", 
		dmamode_reg
	));
		

	// Enable interrupts
	intcsr_reg = 
		readl
		(
			(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_INTCSR * 4))
		);
	intcsr_reg |= PLX_LIIE | PLX_PCIIE | interruptBit;
	
	writel
	(
		intcsr_reg, 
		(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_INTCSR * 4))
	);

	// Setup Timer
	if (pdx[board_num].readtransfer.timer_value)
	{
		init_timer(&pdx[board_num].readtransfer.time_out);
		pdx[board_num].readtransfer.time_out.function = SIPLX_read_time_out;
		pdx[board_num].readtransfer.time_out.data = board_num;
		pdx[board_num].readtransfer.time_out.expires = jiffies +
			(pdx[board_num].readtransfer.timer_value * HZ);

		add_timer(&pdx[board_num].readtransfer.time_out);
	}
	
	// Start DMA 
	writeb(enableBit, (void *)(dmacsrAddress));
	writeb(enableBit | startBit, (void *)(dmacsrAddress));

	SIprintk0((kModuleName "-SIPLX_doBlockDMA: Exiting\n"));

	return 0;		
}

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_doBlockDMA_Write
//
//	Description:
//
//	Parameters:
//	int board_num, 
//	ulong dspaddr, ulong hostaddr, ulong count,
//	int new
//
//	Return Values:
//	int 

int SIPLX_doBlockDMA_Write
(
	int board_num, 
	ulong dspaddr, ulong hostaddr, ulong count,
	int start
)
{
	struct transfer_info *transfer_info = NULL;
	ulong intcsr_reg;
	ulong dmamode_reg;
	ulong dmamodeAddress;
	ulong dmapadrAddress;
	ulong dmaladrAddress;
	ulong dmasizAddress;
	ulong dmadprAddress;
	ulong enableBit;
	ulong startBit;
	ulong interruptBit;
	ulong dmacsrAddress;

	int error;
	
	ulong iosize, i;
	ulong max_size, nr_pages;
	ulong startAddr, localbusAddr, dmadprstuff;
	
	SIprintk0((kModuleName "-SIPLX_doBlockDMA: Entering\n"));

	
	if (start == TRUE)
		while 
		(
			pdx[board_num].writetransfer.in_progress
		);
	if 
	(
		!pdx[board_num].writetransfer.setup
	)
	{
		//transfer in progress
		pdx[board_num].writetransfer.in_progress = TRUE;
	}
	
	//convert word to byte
	count *= 4;

	//determine transfer size
	max_size = (DESCRPT_PAGES * PAGE_SIZE / sizeof(struct descrpt)) * PAGE_SIZE;
	max_size -= (hostaddr & ~PAGE_MASK);
	iosize = count < max_size ? count : max_size;

	nr_pages = NRPAGES(hostaddr, count);
	pdx[board_num].writetransfer.nr_pages = nr_pages;

	if 
	(
		(pdx[board_num].writetransfer.pages = kmalloc(nr_pages * sizeof(*pdx[board_num].writetransfer.pages), GFP_KERNEL)) 
		== NULL
	)
		return -ENOMEM;

	//quad word align 
	pdx[board_num].writetransfer.Descriptor = (struct descrpt*)
		(((ulong)pdx[board_num].dma.wpdscrpt + 0xf)& ~0xf);

	//build transfer list
	error = SIPLX_Build_SGL
	(
		dspaddr, hostaddr, count, 
		pdx[board_num].writetransfer.Descriptor, 
		pdx[board_num].writetransfer.pages, 
		WRITE
	);
	
	if ( error < nr_pages ) {
		pdx[board_num].writetransfer.in_progress = FALSE;
		kfree(pdx[board_num].writetransfer.pages);
		SIprintk1
		((
			kModuleName "-SIPLX_doBlockDMA: ERROR! "
			"Number of pages=0x%x Number requested=0x%x\n", error, (u32)nr_pages
		));
		return -error;
	}
		
	pdx[board_num].writetransfer.user_addr = hostaddr + iosize;
	pdx[board_num].writetransfer.local_addr = dspaddr + iosize;
	pdx[board_num].writetransfer.bytes_requested = iosize;
	pdx[board_num].writetransfer.bytes_remaining = count - iosize;
	
	transfer_info = &pdx[board_num].writetransfer;
	
	//setup PLX register and start the Xfer
	dmamodeAddress = 
		pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_DMAMODE0 * 4);
		
	dmapadrAddress = 
		pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_DMAPADR0 * 4);
		
	dmaladrAddress = 
		pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_DMALADR0 * 4);
		
	dmasizAddress  = 
		pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_DMASIZ0 * 4);
		
	dmadprAddress  = 
		pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_DMADPR0 * 4);
		
	enableBit      = 0x1;
	startBit       = 0x2;
	interruptBit   = PLX_DMA0IE;
	dmacsrAddress = pdx[board_num].mem[PLX_OPREGS].base_addr + PLX_DMACSR0;
	startAddr =
	transfer_info->Descriptor[0].pciaddr;

	if (iosize > (PAGE_SIZE - (hostaddr & ~PAGE_MASK)))
		dmadprstuff =
			(__pa(&pdx[board_num].writetransfer.Descriptor[0].pciaddr)
			| PLX_WRITE | PLX_PCI);
	else	
		dmadprstuff = 
			pdx[board_num].writetransfer.Descriptor[0].next;
	
	localbusAddr = 
		transfer_info->Descriptor[0].localaddr;

	// Disable interrupts
	intcsr_reg = 
		readl
		(
			(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_INTCSR * 4))
		);
	intcsr_reg &= ~interruptBit;
	writel
	(
		intcsr_reg,
		(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_INTCSR * 4))
	);

	// Set up DMA registers
	writel( startAddr, (void *)(dmapadrAddress) );
	writel( localbusAddr, (void *)(dmaladrAddress) );
	writel( transfer_info->Descriptor[0].size, (void *)(dmasizAddress) );
	writel( dmadprstuff, (void *)(dmadprAddress) );

	// Set DMA mode
	dmamode_reg = 0;
	dmamode_reg |= (transfer_info->demandMode ? PLX_DEMAND : PLX_LOCBURS);
	dmamode_reg |= PLX_TARDY;
	dmamode_reg |= PLX_32BIT | PLX_DINT | PLX_PCIINT | PLX_SCATTER;

	if 
	(
		pdx[board_num].driverConfig.blockPoint 
		== SI_CONFIGDRIVER_TRANSFERPOINT
	)
	{
		for (i=0; i < nr_pages; i++)
			transfer_info->Descriptor[i].localaddr = dspaddr;		

		pdx[board_num].readtransfer.local_addr = dspaddr;
		pdx[board_num].writetransfer.local_addr = dspaddr;

		dmamode_reg |= PLX_LOCINC;
	}
	
	writel(dmamode_reg, (void *)(dmamodeAddress));

	SIprintk1
	((
		kModuleName "-SIPLX_doBlockDMA: "
		"dmamode_reg=0x%lx", 
		dmamode_reg
	));
		

	// Enable interrupts
	intcsr_reg = 
		readl
		(
			(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_INTCSR * 4))
		);
	intcsr_reg |= PLX_LIIE | PLX_PCIIE | interruptBit;
	
	writel
	(
		intcsr_reg, 
		(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_INTCSR * 4))
	);
	
	// Setup Timer
	if(pdx[board_num].writetransfer.timer_value)
	{
		init_timer(&pdx[board_num].writetransfer.time_out);
		pdx[board_num].writetransfer.time_out.function = SIPLX_write_time_out;
	
		pdx[board_num].writetransfer.time_out.data = board_num;
		pdx[board_num].writetransfer.time_out.expires = jiffies +
			(pdx[board_num].writetransfer.timer_value * HZ);

		add_timer(&pdx[board_num].writetransfer.time_out);
	}

	// Start DMA 
	writeb(enableBit, (void *)(dmacsrAddress));
	writeb(enableBit | startBit, (void *)(dmacsrAddress));

	SIprintk0((kModuleName "-SIPLX_doBlockDMA: Exiting\n"));
	
	return 0;		
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_Build_SGL
//
//	Description:
//
//	Parameters:
//	ulong targetaddr, ulong hostaddr, ulong count,
//	struct descrpt *tscrpt, int rw
//
//	Return Values:
//	number of pages  

int SIPLX_Build_SGL
(
	ulong targetaddr, ulong hostaddr, ulong count, 
	struct descrpt *tscrpt, struct page **pages, int rw
)
{
	int i = 1, error = 0;
	ulong nr_pages, max_pages, dirflag;

	// count is in bytes
	nr_pages = NRPAGES(hostaddr, count);

	max_pages = DESCRPT_PAGES * PAGE_SIZE / sizeof(struct descrpt);

	if (nr_pages > max_pages)
		return -ENOMEM;

	if (count == 0)
		return 0;

	SIprintk1((kModuleName "-SIPLX_Build_SGL: NR Pages:0x%x\n", (u32)nr_pages));
	
	//fault in the pages 
	down_read(&current->mm->mmap_sem);
	
	error = get_user_pages(
		current,
		current->mm,
		hostaddr,
		nr_pages,
		rw == READ,
		0, 
		pages,
		NULL);
	
	up_read(&current->mm->mmap_sem);

	//Error
	if (error < nr_pages) {
		SIprintk1((kModuleName "-SIPLX_Build_SGL: ERROR! NR Pages rcvd:0x%x\n", error));
		if (error > 0) {
			for (i=0; i < error; i++)
				page_cache_release(pages[i]);
		}
		return error;
	}

	//now build the scatter gather list
	dirflag = rw == READ ? PLX_READ : PLX_WRITE;
	
	tscrpt[0].pciaddr = (page_to_phys(pages[0]) + (hostaddr & ~PAGE_MASK));
	tscrpt[0].localaddr = targetaddr;
	if (nr_pages > 1) {
		tscrpt[0].size = PAGE_SIZE - (hostaddr & ~PAGE_MASK);
		count -= tscrpt[0].size;
		targetaddr += tscrpt[0].size;

		tscrpt[0].next = __pa(&(tscrpt[1].pciaddr));
		tscrpt[0].next |= dirflag | PLX_PCI;
		for (i=1; i < nr_pages ; i++) {
			tscrpt[i].pciaddr = page_to_phys(pages[i]);
			tscrpt[i].localaddr = targetaddr;

			tscrpt[i].size = count < PAGE_SIZE ? count : PAGE_SIZE;
			count -= tscrpt[i].size;
			targetaddr += tscrpt[i].size;

			tscrpt[i].next = __pa(&(tscrpt[(i + 1)].pciaddr));
			tscrpt[i].next |= dirflag | PLX_PCI;

		}
	}
	else {
		tscrpt[0].size = count;

		tscrpt[0].next = 0x00000000;
		tscrpt[0].next |= PLX_EOC | dirflag | PLX_PCI;
		
		SIprintk1((kModuleName "-SIPLX_Build_SGL: Exiting with NR Pages:0x%x\n", (u32)nr_pages));
		return nr_pages;
	}

	tscrpt[(i - 1)].next = 0x00000000;
	tscrpt[(i - 1)].next |= PLX_EOC | dirflag | PLX_PCI;
	
	SIprintk1((kModuleName "-SIPLX_Build_SGL: Exiting with NR Pages:0x%x\n", (u32)nr_pages));
	return nr_pages;

}

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_Unmap_Pages
//
//	Description:
//
//	Parameters:
//	struct page **pages, 
//	ulong nr_pages,
//	int dirtied
//
//	Return Values:
//	zero, success  

int SIPLX_Unmap_Pages
(
	struct page **pages, 
	ulong nr_pages,
	int dirtied
)
{
	int i;

	for (i=0; i < nr_pages; i++) {
		if (dirtied && !PageReserved(pages[i]))
			SetPageDirty(pages[i]);
		/* FIXME: cache flush missing for rw==READ
		 * FIXME: call the correct reference counting function
		 */
		page_cache_release(pages[i]);
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
