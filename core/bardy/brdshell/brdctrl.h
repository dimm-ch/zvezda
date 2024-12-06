/***************************************************
*
* BRDCTRL.H
*
* BRD_ctrl() cmd codes and data types
*
* (C) Instr.Sys. by Ekkore Dec, 1997-2001
*
****************************************************/


#ifndef	__BRDCTRL_H_
#define	__BRDCTRL_H_


#include	"utypes.h"

//
//=== BRD_ctrl() cmd codes
//

enum
{
	ctrlUNINSYSMEMALLOC		= 80,
	ctrlUNINSYSMEMFREE		= 81,
	ctrlUNINSYSMEMMAP		= 82,
	ctrlUNINSYSMEMUNMAP		= 83,
	ctrlASYNCCRAMRDWRSTART  = 90,
	ctrlASYNCCRAMRDWRWAIT	= 91,
	ctrlGETPLISSTATE		= 92,
	ctrlSETCRAMWAITSTATE	= 93,
	ctrlSETHPIFASTMODE		= 94,
	ctrlSETSEMAPHORE        = 95, //ADP6202, ADP6203
	ctrlGETSEMAPHORE        = 96, //ADP6202, ADP6203
	ctrlGETDEVICEHANDLE		= 97,
	ctrlLOADPLISRESET		= 98,
	ctrlLOADPLISWRITE		= 99,
	ctrlLOADPLISSTATE		= 100,
};

//
//=== BRD_ctrl() data types
//

typedef struct
{
	U32	offset;		// Start CRAM offset
	void	*hostAdr;	// Start HOST far address
	U32	len;		// Length in bytes
	U32	isRead;		// 0-Write, 1-Read
} TAsyncCRamRdWr;

typedef struct
{
	U32			size;			// Size of MemBlock (bytes)
	U32			linAdr;			// Linear Adr of MemBlock
	U32			phAdr;			// Phys Adr of MemBlock
} TUninsysMemBlock;

typedef struct
{
	U32			param;			// IN: flags; OUT: return value
	char			*fname;			// HEX file name
} TLoadPLIS;


#endif	// __BRDCTRL_H_ 

//
//  End of File
//


