////////////////////////////////////////////////////////////////////////////////
//	plx.h
//
//	Description:
//
//	Rev Notes:
//		200x-xx-xx: RobU and TonyH
//			Created.
//		2002-07-26: Av
//			Added comments 
//		2002-08-08: mik
//			Changed all func names to SIPLX_xxx.
//			Changed to only contain outside visible functions.
//		2002-08-15: mik
//			Fixed to work with kernel 2.2 (no BM yet).
//

int SIPLX_GetParams
(
	ulong arg, ulong *region, ulong *count, ulong *offset, ulong *buffer
);
int SIPLX_ConfigRead
(
	int board_num,
	ulong arg
);

int SIPLX_OpregWrite
(
	int board_num,
	ulong arg
);

int SIPLX_OpregRead
(
	int board_num,
	ulong arg
);

int SIPLX_NVRamWrite
(
	int board_num,
	ulong arg
);

int SIPLX_NVRamRead
(
	int board_num,
	ulong arg
);

int SIPLX_PassthroughDo
(
	int board_num, ulong arg, 
	int readFromPlx, ulong plxDataSize, ulong hostDataSize
);

int SIPLX_GetAddOn
(
	struct inode *inode,
	struct file *filp, 
	unsigned long arg
);

