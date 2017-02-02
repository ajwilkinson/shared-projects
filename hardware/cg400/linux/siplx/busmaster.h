////////////////////////////////////////////////////////////////////////////////
//	busmaste.h
//
//	Description:
//		header for busmaster_ver_0204.c		
//	Rev Notes:
//		2001-11-01: Colin
//			Created.
//		2005-08-01: Colin
//			Upgraded for linux kernel 2.6.11
//
int SIPLX_doBlockDMA_Read
(
	int board_num, 
	ulong dspaddr, ulong hostaddr, ulong count,
	int start
);

int SIPLX_doBlockDMA_Write
(
	int board_num, 
	ulong dspaddr, ulong hostaddr, ulong count,
	int start
);

int SIPLX_Build_SGL
(
	ulong targetaddr, ulong hostaddr, ulong count, 
	struct descrpt *tscrpt, struct page **pages, int rw
);

int SIPLX_Unmap_Pages
(
	struct page **pages, 
	ulong nr_pages,
	int dirtied
);
