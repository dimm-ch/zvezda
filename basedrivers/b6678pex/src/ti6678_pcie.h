/********************************************************************
 *  (C) Copyright 1992-2012 InSys Corp.  All rights reserved. 
 *
 *  Autor	- DOR
 *
 *  filename	- ti6678_pcie.h
 *  version 	- 1.0
 *  description:
 *		TI6678 PCIe Library
 *
 *
 *  Modify:
 ********************************************************************/

#ifndef	__TI6678_PCIE__
#define	__TI6678_PCIE__

#include "ti6678hw.h"

//
//==== Types
//
typedef	unsigned char	uint8_t;

int	TI6678PCIE_Init();
int	TI6678PCIE_ReInit(TI6678_DEVICE *dev);
int	TI6678PCIE_Cleanup(void);
TI6678_DEVICE *TI6678PCIE_Open(int id);
int	TI6678PCIE_Close(TI6678_DEVICE *dev);
int	TI6678PCIE_ProcReadMem(TI6678_DEVICE *dev, U32 core, U32 adr, U32 *dst, U32 size);
int	TI6678PCIE_ProcWriteMem(TI6678_DEVICE *dev, U32 core, U32 adr, U32 *src, U32 size);
int	TI6678PCIE_ProcLoadBin(TI6678_DEVICE *dev, U32 core, const BRDCHAR *fname, U32 *entry);
int	TI6678PCIE_ProcLoadMem(TI6678_DEVICE *dev, U32 core, UCHAR *appBuf, U32 appSize, U32 *entry);
int	TI6678PCIE_ProcStart(TI6678_DEVICE *dev, U32 core, U32 entry);
int	TI6678PCIE_ProcLocalReset(TI6678_DEVICE *dev, U32 core);
int	TI6678PCIE_ProcSoftReset(TI6678_DEVICE *dev, int state);
int	TI6678PCIE_ReInit(TI6678_DEVICE *dev);
int	TI6678PCIE_ProcSysReset(TI6678_DEVICE *dev);
int	TI6678PCIE_LocalReset(TI6678_DEVICE *dev, U32 core);
int	TI6678PCIE_LocalEntry(TI6678_DEVICE *dev, U32 core, U32 entry);
int	TI6678PCIE_LocalRelease(TI6678_DEVICE *dev, U32 core);

int	TI6678PCIE_WriteDMAState(TI6678_DEVICE *dev, U32 chan, U32 *state);
int	TI6678PCIE_ReadDMAState(TI6678_DEVICE *dev, U32 chan, U32 *state);

int	TI6678PCIE_EnableInterrupt(TI6678_DEVICE *dev);
int	TI6678PCIE_DisableInterrupt(TI6678_DEVICE *dev);
int	TI6678PCIE_CheckInterrupt(TI6678_DEVICE *dev);
int	TI6678PCIE_ClrInterrupt(TI6678_DEVICE *dev);
int	TI6678PCIE_SendInterrupt(TI6678_DEVICE *dev, U32 node);
int	TI6678PCIE_SendInterruptMSI(TI6678_DEVICE *dev);


#endif	//__TI6678_PCIE__
