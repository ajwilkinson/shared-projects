///////////////////////////////////////////////////////////////////////////////
//	SIErrors.h
//
//	Description:
//		Following are SI error codes. These error codes should be used by all 
//		code other than the driver. This means .exe, .lib, .dll, must use 
//		these codes.
//
//	Revision History:
//		2002-04-17: mik
//			Created.
//		2002-05-01: mik
//			Added #if !defined(_SIERRORS_H).
//		2002-03-06: mik
//			Added file read/write error codes.
//		2003-04-08: mik
//			Added Index out of bounds
//		2003-06-04: mik
//			Made it work with DOS "driver".
//		2003-06-13: mik
//			Added e_Err_SyncAccessNotReady
//		2003-07-07: mik
//			Reorganized error codes (more room).
//	
///////////////////////////////////////////////////////////////////////////////

#if !defined(_SIERRORS_H)
#define _SIERRORS_H

#if defined(_cplusplus)
extern "C" {
#endif

enum VQ_Error
{
	// System errors
	e_Err_DeviceNotFound = -33, // arbitrary, to be backward compatible
	e_Err_BadVendorID, 

	// Following are not-so-serious errors, to be continued below
	e_Err_ScopeUnderflow = 1,	// 1

	e_Err_NoError = 0,			// 0

	// Misc errors
	e_Err_MiscError = 2,		// 2
	e_Err_UnknownCommand, 
	e_Err_EventSetup, 
	e_Err_MsgError, 
	e_Err_InvalidNumOfParams, 
	e_Err_InvalidParams, 
	e_Err_8, 
	e_Err_9, 
	
	
	// Buffer errors
	e_Err_Timeout,				// 10
	e_Err_BufferTooSmall, 
	e_Err_CountTooBig,
	e_Err_CompareError, 
	e_Err_DSPOutofMemory, 
	e_Err_DSPAllocateBuffer,
	e_Err_ScopePointerError,
	e_Err_17, 
	e_Err_18, 
	e_Err_19, 

	// Read errors
	e_Err_ReadError,			// 20
	e_Err_NotAllWordsRead,
	e_Err_OpregReadError,
	e_Err_TargetReadError, 
	e_Err_BusmasterReadError,
	e_Err_AddonInitReadError, 
	e_Err_HostpollReadError, 
	e_Err_NVReadError, 
	e_Err_28, 
	e_Err_29, 

	// Write errors
	e_Err_WriteError,			// 30
	e_Err_NotAllWordsWrite,
	e_Err_OpregWriteError,
	e_Err_TargetWriteError, 
	e_Err_BusmasterWriteError,
	e_Err_AddonInitWriteError, 
	e_Err_HostpollWriteError, 
	e_Err_NVWriteError, 
	e_Err_38, 
	e_Err_39, 

	// DSP Errors
	e_Err_ResetError,			// 40
	e_Err_ResetInvalidMode, 
	e_Err_ResetAssertError, 
	e_Err_ResetDeassertError,
	e_Err_ResetTBCError,
	e_Err_AddonInitTimeoutError,
	e_Err_NoInterruptFromDSP, 
	e_Err_47, 
	e_Err_48, 
	e_Err_49, 

	// Following are used by ISA
	e_Err_FileTooLarge,			// 50
	e_Err_StatusBitError, 
	e_Err_ISACommError, 
	e_Err_ISAClearHandshake, 
	e_Err_BeginCmdSend,
	e_Err_ISASendType, 
	e_Err_ISASendCount, 
	e_Err_ISASendSrc,
	e_Err_ISASendDst, 
	e_Err_ISAStatusValueError,

	// Following are used by CAC
	e_Err_FdspHalt,				// 60
	e_Err_FdspDlExec, 
	e_Err_FfindLabelName, 
	e_Err_FdspRun, 
	e_Err_64, 
	e_Err_65, 
	e_Err_66, 
	e_Err_67, 
	e_Err_68, 
	e_Err_69, 

	// Following are used by C6x parallel
	e_Err_ClassInitError,		// 70
	e_Err_CloseError, 
	e_Err_PortOpenError, 
	e_Err_CoffloadError,
	e_Err_GenerateIntError, 
	e_Err_75, 
	e_Err_76, 
	e_Err_77, 
	e_Err_78, 
	e_Err_79, 

	// Following are used by PLXC6711
	e_Err_SdramAckError,		// 80
	e_Err_SdramConfig_tCL,
	e_Err_SdramConfig_tREFI,
	e_Err_SdramConfig_colSize,
	e_Err_SdramConfig_rowSize,
	e_Err_SdramConfig_bankSize, 
	e_Err_SdramConfig_DeviceAddr,
	e_Err_SdramConfig_Param, 
	e_Err_88, 
	e_Err_89, 

	// File system errors
	e_Err_FileNotFound,			// 90
	e_Err_CreateFile,
	e_Err_UnableToReadFile, 
	e_Err_WindowsHandle, 	
	e_Err_WindowsMemError, 
	e_Err_InvalidFile,
	e_Err_FileWriteError,
	e_Err_FileReadError, 
	e_Err_98, 
	e_Err_99, 

	// Following are not-so-serious errors
	//	see above for this error. e_Err_ScopeUnderflow = 1, 
	e_Err_ScopeTrigger,			// 100
	e_Err_ScopeADCOff, 
	e_Err_ScopeOverflow, 
	e_Err_103, 
	e_Err_104, 
	e_Err_105, 
	e_Err_106, 
	e_Err_107, 
	e_Err_108, 
	e_Err_109, 

	// Following are bit file loader specific errors.
	e_Err_FPGA_DoneNotLow,		// 110
	e_Err_FPGA_DoneNotHigh, 
	e_Err_FPGA_TypeNotFound, 
	e_Err_FPGA_NotAllFPGALoaded, 
	e_Err_FPGA_MaxNumberOfFPGAs, 
	e_Err_FPGA_TypeNotCorrect, 
	e_Err_116, 
	e_Err_117, 
	e_Err_118, 
	e_Err_119, 

	// Following are COFF loader specific errors.
	e_Err_CoffTypeError,		// 120
	e_Err_CoffSectionsError,
	e_Err_122, 
	e_Err_123, 
	e_Err_124, 
	e_Err_125, 
	e_Err_126, 
	e_Err_127, 
	e_Err_128, 
	e_Err_129, 

	// errors caused by formula node (todo: is this needed?)
	e_Err_IndexOutofBounds,		// 130
	e_Err_131, 
	e_Err_132, 
	e_Err_133, 
	e_Err_134, 
	e_Err_135, 
	e_Err_136, 
	e_Err_137, 
	e_Err_138, 
	e_Err_139, 

	//Following are only for VQ host dll and used by SI boards only.
	e_Err_GetGeneration,		// 140
	e_Err_GetCurVQFelAddr,
	e_Err_AddToPQFList,
	e_Err_AddToFlist, 
	e_Err_144, 
	e_Err_145, 
	e_Err_146, 
	e_Err_147, 
	e_Err_148, 
	e_Err_149, 

	// Following are DOS DDK error codes
	e_Err_PCIBIOSCallFailed,	// 150
	e_Err_DMPIBIOSCallFailed,
	e_Err_PCIBIOSNotPresent, 
	e_Err_PCISignature, 
	e_Err_ClassNotFound, 
	e_Err_SpecialCycles, 
	e_Err_InvalidBitWidth, 
	e_Err_ReadConfig, 
	e_Err_WriteConfig,
	e_Err_HostTooFast,

	// following are synchronous read/write errors
	e_Err_SyncAccessNotReady,	// 160
	e_Err_161, 
	e_Err_162, 
	e_Err_163, 
	e_Err_164, 
	e_Err_165, 
	e_Err_166, 
	e_Err_167, 
	e_Err_168, 
	e_Err_169,
	
	//Following are send and receive messaging errors
	e_Err_RecvMsg_NoMsg,		// 170
	e_Err_171, 
	e_Err_172, 
	e_Err_173, 
	e_Err_174, 
	e_Err_SendMsg_Pending,
	e_Err_SendMsg_Timeout, 
	e_Err_177, 
	e_Err_178, 
	e_Err_179, 

	// Following are used for MPC network applications
	// Network errors
	e_Err_LoadWinsocketError,	// 180
	e_Err_InvalidIPAddress,
	e_Err_CreateSocketError,
	e_Err_ConnectServerError,
	e_Err_SendError,
	e_Err_ReceiveError,
	e_Err_186,
	e_Err_187,
	e_Err_CreateThreadError,
	e_Err_189,

	//SIMPC errors TODO: Clean up this whole file
	e_Err_UPMRAMError,
	e_Err_ECCMError,
	e_Err_NANDBusWidthError,
	e_Err_NANDBadBlockError,
	e_Err_194,
	e_Err_195,
	e_Err_196,
	e_Err_197,
	e_Err_198,
	e_Err_199,

	// last error is always the last error
	e_Err_LastError

};

int TranslateErrorCode(int error, char errorMsg[]);

#if defined(_cplusplus)
}
#endif
#endif


