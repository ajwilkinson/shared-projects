////////////////////////////////////////////////////////////////////////////////
//	init.h
//
//	Description:
//
//	Rev Notes:
//		2002-12-10: Colin
//			Created.
//

int SIPLX_board_init
(
	int card_num
);

int SI_MemAlloc_Cont
(
	ulong **ptr, u32 order
);

void SI_MemDealloc_Cont
(
	ulong *ptr, u32 order
);

