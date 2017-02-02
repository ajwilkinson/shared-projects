///////////////////////////////////////////////////////////////////////////////
//	SITypes.h
//
//	Description:
//		Common types used by typical SI program.
//
//	Revision History:
//		2001-11-06: mik
//			Created.
//		2002-03-06: mik
//			Added ifndef _BASETSD_H_ due to MS sometimes adding it for UINT8.
//		2002-06-14: mik
//			Changed typedefs to #defines.
//
///////////////////////////////////////////////////////////////////////////////

#if !defined( SITYPES_H )
#define SITYPES_H

#if defined(_cplusplus)
extern "C" {
#endif


#define INT8	char
#define INT16	short
#define INT32	int 

#define UINT8	unsigned INT8
#define UINT16	unsigned INT16
#define UINT32	unsigned INT32

#ifdef WIN32
#define SI_Thread	HANDLE
#endif //WIN32

#ifdef LINUX
#define SI_Thread	pthread_t
#endif //LINUX


#if defined(_cplusplus)
}
#endif
#endif

