////////////////////////////////////////////////////////////////////////////////
//	isr.h
//
//	Description:
//
//	Rev Notes:
//		2002-08-08: mik
//			Created. Only contains outside visible functions.
//		2002-08-15: mik
//			Fixed to work with kernel 2.2 (no BM yet).
//		2002-11-01: Colin
//			Re-write/Update of driver for kernel 2.4.x
//		2005-08-01: Colin
//			Upgraded for linux kernel 2.6.11
//		2008-09-16: Whipple
//			Implemented Meyer changes
//

////////////////////////////////////////////////////////////////////////////////
// function prototypes

irqreturn_t SIPLX_isr
(
	int irq, void *dev_id
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)	
	, struct pt_regs *regs
#endif
);

void SIPLX_isr_bottomhalf
(
	void *dev_id
);

void SIPLX_read_time_out(ulong board_num);

void SIPLX_write_time_out(ulong board_num);

void SIPLX_CancelDMA(int board_num);
