////////////////////////////////////////////////////////////////////////////////
//	ioctl.h
//
//	Description:
//
//	Rev Notes:
//		200x-xx-xx: RobU and TonyH
//			Created.
//		2002-07-26: Av
//			Added comments 
//		2002-07-31: mik
//			Changed IOCTL values to IOCTL_SHELDON_xxx
//


#define SIPLX_IOCTL_MAGIC 'p'

#define IOCTL_SHELDON_MAILBOX_WRITE		_IOW(SIPLX_IOCTL_MAGIC, 1, ulong)

#define IOCTL_SHELDON_MAILBOX_READ		_IOR(SIPLX_IOCTL_MAGIC, 2, ulong)

#define IOCTL_SHELDON_CONFIG_READ		_IOR(SIPLX_IOCTL_MAGIC, 3, ulong)

#define IOCTL_SHELDON_OPREG_WRITE		_IOW(SIPLX_IOCTL_MAGIC, 6, ulong)

#define IOCTL_SHELDON_OPREG_READ		_IOR(SIPLX_IOCTL_MAGIC, 7, ulong)

#define IOCTL_SHELDON_NVRAM_WRITE		_IOW(SIPLX_IOCTL_MAGIC, 13, ulong)

#define IOCTL_SHELDON_NVRAM_READ		_IOR(SIPLX_IOCTL_MAGIC, 14, ulong)

#define IOCTL_SHELDON_BUSMASTERED_TIMEOUT	_IOW(SIPLX_IOCTL_MAGIC, 15, ulong)

#define IOCTL_SHELDON_PASSTHROUGH_WRITE_8	_IOW(SIPLX_IOCTL_MAGIC, 18, ulong)

#define IOCTL_SHELDON_PASSTHROUGH_READ_8	_IOR(SIPLX_IOCTL_MAGIC, 19, ulong)

#define IOCTL_SHELDON_PASSTHROUGH_WRITE_16	_IOW(SIPLX_IOCTL_MAGIC, 20, ulong)

#define IOCTL_SHELDON_PASSTHROUGH_READ_16	_IOR(SIPLX_IOCTL_MAGIC, 21, ulong)

#define IOCTL_SHELDON_PASSTHROUGH_WRITE_32	_IOW(SIPLX_IOCTL_MAGIC, 22, ulong)

#define IOCTL_SHELDON_PASSTHROUGH_READ_32	_IOR(SIPLX_IOCTL_MAGIC, 23, ulong)

#define IOCTL_SHELDON_GET_ADDON_ADDR	_IOW(SIPLX_IOCTL_MAGIC, 27, ulong)

#define IOCTL_SHELDON_BUSMASTERED_READ		_IOR(SIPLX_IOCTL_MAGIC, 28, ulong)
	 
#define IOCTL_SHELDON_BUSMASTERED_WRITE		_IOR(SIPLX_IOCTL_MAGIC, 29, ulong)

#define IOCTL_SHELDON_BUSMASTERED_READ_DONE	_IOW(SIPLX_IOCTL_MAGIC, 30, ulong)

#define IOCTL_SHELDON_BUSMASTERED_WRITE_DONE	_IOW(SIPLX_IOCTL_MAGIC, 31, ulong)

#define IOCTL_SHELDON_CANCEL_BUSMASTERING _IO(SIPLX_IOCTL_MAGIC, 32)

#define	IOCTL_SHELDON_DRIVER_CONFIG	_IOR(SIPLX_IOCTL_MAGIC, 5, ulong)

#define IOCTL_SHELDON_DEFINE_DEMAND_MODE	_IOW(SIPLX_IOCTL_MAGIC, 50, ulong)

#define IOCTL_SHELDON_HARD_RESET			_IO(SIPLX_IOCTL_MAGIC, 99)


/*
#define MAILBOX_WRITE _IOW(SIPLX_IOCTL_MAGIC, 1, opreg_info)
#define MAILBOX_READ _IOR(SIPLX_IOCTL_MAGIC, 2, opreg_info)
#define CONFIG_READ _IOR(SIPLX_IOCTL_MAGIC, 3, config_data)
#define OPREG_WRITE _IOW(SIPLX_IOCTL_MAGIC, 6, opreg_info)
#define OPREG_READ _IOR(SIPLX_IOCTL_MAGIC, 7, opreg_info)
#define DMA_CHAINED_WRITE _IOW(SIPLX_IOCTL_MAGIC, 9, data)
#define DMA_CHAINED_READ _IOR(SIPLX_IOCTL_MAGIC, 10, data)
#define NVRAM_WRITE _IOW(SIPLX_IOCTL_MAGIC, 13, opreg_info)
#define NVRAM_READ _IOR(SIPLX_IOCTL_MAGIC, 14, opreg_info)
#define BUSMASTERED_TIMEOUT _IOW(SIPLX_IOCTL_MAGIC, 15, data)
#define PASSTHROUGH_WRITE_8 _IOW(SIPLX_IOCTL_MAGIC, 18, data)
#define PASSTHROUGH_READ_8 _IOR(SIPLX_IOCTL_MAGIC, 19, data)
#define PASSTHROUGH_WRITE_16 _IOW(SIPLX_IOCTL_MAGIC, 20, data)
#define PASSTHROUGH_READ_16 _IOR(SIPLX_IOCTL_MAGIC, 21, data)
#define PASSTHROUGH_WRITE_32 _IOW(SIPLX_IOCTL_MAGIC, 22, data)
#define PASSTHROUGH_READ_32 _IOR(SIPLX_IOCTL_MAGIC, 23, data)
#define DMA_BLOCK_WRITE _IOW(SIPLX_IOCTL_MAGIC, 24, data)
#define DMA_BLOCK_READ  _IOR(SIPLX_IOCTL_MAGIC, 25, data)
#define DEFINE_DEMAND_MODE _IOW(SIPLX_IOCTL_MAGIC, 26, opreg_info)

#define HARD_RESET _IO(SIPLX_IOCTL_MAGIC, 99)
*/

////////////////////////////////////////////////////////////////////////////////
// The following defines specify the array index of information passed
// to and from the driver in an array of 32 bit numbers

#define SI_PARAMS_COUNT				4

#define SI_PARAMS_INDEX_REGION		0
#define SI_PARAMS_INDEX_OFFSET		1
#define SI_PARAMS_INDEX_COUNT		2
#define SI_PARAMS_INDEX_USERBUFFER	3

////////////////////////////////////////////////////////////////////////////////
// config driver call constants

// index to CONFIG DRIVER TYPE.
#define SI_CONFIGDRIVER_INDEX_TYPE			0

// value index to CONFIG DRIVER TYPE for block/point transfer config 
#define SI_CONFIGDRIVER_TYPE_BLOCKPOINT		0	

// index to data that indicates whether it's point or block
#define SI_CONFIGDRIVER_INDEX_BLOCKPOINT	1
#define SI_CONFIGDRIVER_TRANSFERBLOCK		1
#define SI_CONFIGDRIVER_TRANSFERPOINT		2

////////////////////////////////////////////////////////////////////////////////

