////////////////////////////////////////////////////////////////////////////////
//	isr_ver_0204.c
//
//	Description:
//
//	Rev Notes:
//		200x-xx-xx: RobU and TonyH
//			Created.
//		2002-07-26: Av
//			Added comments and fixed indenting (cosmetics).
//		2002-08-01: mik
//			Fixed indenting (cosmetics).
//		2002-08-08: mik
//			Changed all func names to SI_xxx.
//		2002-08-15: mik
//			Fixed to work with kernel 2.2 (no BM yet).
//		2002-11-01: Colin
//			Re-write/Update of driver for kernel 2.4.x
//		2002-11-25: Colin
//			Added SG DMA
//		2005-08-01: Colin
//			Upgraded for linux kernel 2.6.11
//			means no more bottom halfs, use work queues instead.
//		2008-09-16: Whipple
//			Implemented Meyer changes
//

#ifndef MODULE
#define MODULE
#endif

#ifndef __KERNEL__
#define __KERNEL__
#endif


#include <linux/sched.h>
//#include <linux/interrupt.h>

#include "main.h"

////////////////////////////////////////////////////////////////////////////////
//	void SIPLX_isr
//
//	Description:
//
//	Parameters:
//	int irq:
//	void *dev_id: 
//	struct pt_regs *regs:
//
//	Return Values:
//

irqreturn_t SIPLX_isr
(
	int irq,
	void *dev_id 
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)	
	,struct pt_regs *regs
#endif	
)
{
	ulong board_num;
	ulong intcsr_reg, doorbell;
	ulong dmacsr;
	ulong INTCSR;

	SIprintk0((kModuleName "-SIPLX_isr: Entering\n"));
	
	if (dev_id == NULL) 
		return IRQ_NONE;

	board_num = ((ulong) dev_id) - 1;
	
	INTCSR = pdx[board_num].dev_intcsr;
	
	intcsr_reg = 
			readl((void *)(pdx[board_num].mem[PLX_OPREGS].base_addr 
		+	INTCSR));

	switch (pdx[board_num].chip)
	{
		case e_DevType_9054:
			if (intcsr_reg & PLX_LII) 
			{
				// A local interrupt has been received.
			}

			dmacsr = 
			readl
			(
				(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_DMACSR * 4))
			);

			// EXTREME care should be used with printk statements inside the isr. The 
			// printk statements are NOT reentrant and this could cause problems!
			SIprintk1
			((
				kModuleName "-SIPLX_isr: intcsr_reg  0x%lx,dmacsr 0x%lx\n",
				intcsr_reg,	dmacsr
			));

			if
			(
				pdx[board_num].readtransfer.in_progress 
				&&	(intcsr_reg & PLX_DMA1I)
			)
			{
				SIprintk1
				((
					kModuleName "-SIPLX_isr:PLX_DMA1I recvd with read in progress\n"
				));

				// clear the interrupt
				writeb(0x08, (void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + PLX_DMACSR1));

				// update status
				pdx[board_num].readtransfer.just_completed = TRUE;

				// schedule bottom half 
				schedule_work(&pdx[board_num].bh);

				return IRQ_HANDLED;
			}

			if
			(
				pdx[board_num].writetransfer.in_progress 
				&&	(intcsr_reg & PLX_DMA0I)
			)
			{
				SIprintk1
				((
					kModuleName "-plxisr:PLX_DMA0I recvd with write in progress\n"
				));

				// clear the interrupt
				writeb(0x08, (void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + PLX_DMACSR0));

				// update status
				pdx[board_num].writetransfer.just_completed = TRUE;

				// schedule bottom half
				schedule_work(&pdx[board_num].bh);

				return IRQ_HANDLED;
			}

			if(intcsr_reg & PLX_PDBI)
			{
				SIprintk1
				((
					kModuleName "-plxisr: doorbell recvd with write in progress\n"
				));
		
				doorbell = 
					readl
					(
						(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr 
						+	(PLX_L2PDB * 4))
					);
	

				writel
				(
					doorbell,
					(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr + (PLX_L2PDB * 4))
				);

				pdx[board_num].nonbustransfer.just_completed = TRUE;

				// schedule bottom half
				schedule_work(&pdx[board_num].bh);

				return IRQ_HANDLED;
			}
			break;
		case e_DevType_9030:
			if(intcsr_reg & PLX9030_LINT1)			
			{	
				// clear the interrupt
				writel
				(
					intcsr_reg | PLX9030_LINT1_CLEAR, 
					(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr 
					+ INTCSR)
				);
				return IRQ_HANDLED;
			}
			if(intcsr_reg & PLX9030_LINT2)			
			{
				// clear the interrupt
				writel
				(
					intcsr_reg | PLX9030_LINT2_CLEAR, 
					(void *)(pdx[board_num].mem[PLX_OPREGS].base_addr 
					+ INTCSR)
				);
				return IRQ_HANDLED;
			}
			break;	
	}
	SIprintk0((kModuleName "-SIPLX_isr: Exiting\n"));

	return IRQ_NONE;

}

////////////////////////////////////////////////////////////////////////////////
//	void SIPLX_isr_bottomhalf
//
//	Description:
//	Now implemented as a work queue
//
//	Parameters:
//	void *dev_id: 
//
//	Return Values:
//

void SIPLX_isr_bottomhalf(void *dev_id)
{
	ulong board_num;
	

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	board_num = ((ulong) dev_id)-1;
#else
	struct phys_device_ext* pdx_addr = container_of(dev_id,struct phys_device_ext,bh);
	board_num = pdx_addr - pdx;
#endif

	SIprintk0((kModuleName "-SIPLX_isr_bottomhalf: Entering\n"));
	if(pdx[board_num].readtransfer.just_completed) {
		// stop the timeout
		if (pdx[board_num].readtransfer.timer_value) {
			del_timer(&pdx[board_num].readtransfer.time_out);
			pdx[board_num].readtransfer.timed_out = FALSE;
		}

		pdx[board_num].readtransfer.setup = TRUE;
		pdx[board_num].readtransfer.just_completed = FALSE;

		//more bytes to transfer
		if (pdx[board_num].readtransfer.bytes_remaining) {
			if (SIPLX_doBlockDMA_Read
			(
				board_num,
				pdx[board_num].readtransfer.local_addr,	
				pdx[board_num].readtransfer.user_addr,	
				pdx[board_num].readtransfer.bytes_remaining/4,	
				FALSE
			) != 0)
				return;				
		}
		else {
			SIprintk0((kModuleName "-SIPLX_isr_bottomhalf: Read Busmaster Transfer Completed\n"));
			SIPLX_Unmap_Pages
			(
				pdx[board_num].readtransfer.pages, 
				pdx[board_num].readtransfer.nr_pages,
				TRUE
			);
			
			kfree(pdx[board_num].readtransfer.pages);

			pdx[board_num].readtransfer.setup = FALSE;
			write_lock(&pdx[board_num].readtransfer.status);
			pdx[board_num].readtransfer.in_progress = FALSE;
			write_unlock(&pdx[board_num].readtransfer.status);
		kill_fasync(&pdx[board_num].async_queue, SIGIO, POLL_IN);
		}
		return;
	}

	if(pdx[board_num].writetransfer.just_completed) {
		// stop the timeout
		if (pdx[board_num].writetransfer.timer_value) {
			del_timer(&pdx[board_num].writetransfer.time_out);
			pdx[board_num].writetransfer.timed_out = FALSE;
		}

		pdx[board_num].writetransfer.setup = TRUE;
		pdx[board_num].writetransfer.just_completed = FALSE;

		//more bytes to transfer
		if (pdx[board_num].writetransfer.bytes_remaining) {
			if (SIPLX_doBlockDMA_Write
			(
				board_num,
				pdx[board_num].writetransfer.local_addr,	
				pdx[board_num].writetransfer.user_addr,	
				pdx[board_num].writetransfer.bytes_remaining/4,	
				FALSE
			) != 0)
				return;				
		}
		else {
			SIprintk0((kModuleName "-SIPLX_isr_bottomhalf: Write Busmaster Transfer Completed\n"));
			SIPLX_Unmap_Pages
			(
				pdx[board_num].writetransfer.pages, 
				pdx[board_num].writetransfer.nr_pages,
				TRUE
			);
			
			kfree(pdx[board_num].writetransfer.pages);

			pdx[board_num].writetransfer.setup = FALSE;
			write_lock(&pdx[board_num].writetransfer.status);
			pdx[board_num].writetransfer.in_progress = FALSE;
			write_unlock(&pdx[board_num].writetransfer.status);
		kill_fasync(&pdx[board_num].async_queue, SIGIO, POLL_IN);
		}
		return;
	}

	if(pdx[board_num].nonbustransfer.just_completed)
	{
		SIprintk0((kModuleName "-SIPLX_isr_bottomhalf: NonBusmaster Transfer Completed\n"));
		pdx[board_num].nonbustransfer.just_completed = FALSE;
				
		kill_fasync(&pdx[board_num].async_queue, SIGIO, POLL_IN);
	}
	
	SIprintk0((kModuleName "-SIPLX_isr_bottomhalf: Exiting\n"));
}

////////////////////////////////////////////////////////////////////////////////
//	int SIPLX_fasync
//
//	Description:
//
//	Parameters:
//	int fd:
//	struct file *filp:
//	int mode:
//
//	Return Values:
//

int SIPLX_fasync
(
	int fd, 
	struct file *filp, 
	int mode
)
{
	int board_num;
	int ret_value;

	struct inode *inode = filp->f_path.dentry->d_inode;
	
	SIprintk0((kModuleName "-SIPLX_fasync: Entering\n"));

	board_num = iminor(inode);

	ret_value = fasync_helper(fd, filp, mode,
	&(pdx[board_num].async_queue));

	SIprintk0((kModuleName "-SIPLX_fasync: Exiting\n"));

	return ret_value;
}

////////////////////////////////////////////////////////////////////////////////
//	void SIPLX_read_time_out
//
//	Description:
//
//	Parameters:
//	ulong board_num:
//
//	Return Values:
//

void SIPLX_read_time_out(ulong board_num)
{
	ulong dmacsrAddress;
	ulong DMACSR = 0;
	
	SIprintk0((kModuleName "-SIPLX_read_time_out: Entering\n"));
	
	dmacsrAddress = pdx[board_num].mem[PLX_OPREGS].base_addr + PLX_DMACSR1;

	// turn off the transfer
	writeb(0x0, (void *)(dmacsrAddress));
	writeb(0x4, (void *)(dmacsrAddress));
	
	while (DMACSR != PLX_DONE)
		DMACSR = readb((void *)(dmacsrAddress));
		
	// update status
	pdx[board_num].readtransfer.just_completed = FALSE;
	pdx[board_num].readtransfer.timed_out = TRUE;
	write_lock(&pdx[board_num].readtransfer.status);
	pdx[board_num].readtransfer.in_progress = FALSE;
	write_unlock(&pdx[board_num].readtransfer.status);

	SIprintk0((kModuleName "-SIPLX_read_time_out: Exiting\n"));

}

////////////////////////////////////////////////////////////////////////////////
//	void SIPLX_write_time_out
//
//	Description:
//
//	Parameters:
//	ulong board_num:
//
//	Return Values:
//

void SIPLX_write_time_out(ulong board_num)
{
	ulong dmacsrAddress;
	ulong DMACSR = 0;

	SIprintk0((kModuleName "-SIPLX_write_time_out: Entering\n"));

	dmacsrAddress = pdx[board_num].mem[PLX_OPREGS].base_addr + PLX_DMACSR0;

	// turn off the transfer
	writeb(0x0, (void *)(dmacsrAddress));
	writeb(0x4, (void *)(dmacsrAddress));
	
	while (DMACSR != PLX_DONE)
		DMACSR = readb((void *)(dmacsrAddress));

	// update status
	pdx[board_num].writetransfer.just_completed = FALSE;
	pdx[board_num].writetransfer.timed_out = TRUE;
	write_lock(&pdx[board_num].writetransfer.status);
	pdx[board_num].writetransfer.in_progress = FALSE;
	write_unlock(&pdx[board_num].writetransfer.status);

	SIprintk0((kModuleName "-SIPLX_write_time_out: Exiting\n"));

}

////////////////////////////////////////////////////////////////////////////////
//	void SIPLX_CancelDMA
//
//	Description:
//
//	Parameters:
//	ulong board_num:
//
//	Return Values:
//

void SIPLX_CancelDMA(int board_num)
{
	ulong dmacsrAddress;
	ulong DMACSR = 0;

	if (pdx[board_num].t_type == PLX_READ)
	{
		if (!pdx[board_num].readtransfer.in_progress)
			return;
			
		dmacsrAddress = pdx[board_num].mem[PLX_OPREGS].base_addr + PLX_DMACSR1;

		// turn off the transfer
		writeb(0x0, (void *)(dmacsrAddress));
		writeb(0x4, (void *)(dmacsrAddress));
	
		while (DMACSR != PLX_DONE)
			DMACSR = readb((void *)(dmacsrAddress));
		
		// update status
		pdx[board_num].readtransfer.just_completed = FALSE;
		write_lock(&pdx[board_num].readtransfer.status);
		pdx[board_num].readtransfer.in_progress = FALSE;
		write_unlock(&pdx[board_num].readtransfer.status);
	}
	else
	{
		if (!pdx[board_num].writetransfer.in_progress)
			return;

		dmacsrAddress = pdx[board_num].mem[PLX_OPREGS].base_addr + PLX_DMACSR0;

		// turn off the transfer
		writeb(0x0, (void *)(dmacsrAddress));
		writeb(0x4, (void *)(dmacsrAddress));
	
		while (DMACSR != PLX_DONE)
			DMACSR = readb((void *)(dmacsrAddress));	

		// update status
		pdx[board_num].writetransfer.just_completed = FALSE;
		write_lock(&pdx[board_num].writetransfer.status);
		pdx[board_num].writetransfer.in_progress = FALSE;
		write_unlock(&pdx[board_num].writetransfer.status);
	}
}
