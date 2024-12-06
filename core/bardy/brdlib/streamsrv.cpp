/*
 ***************** File StreamSrv.cpp ***********************
 *
 * BRD Driver for Stream service
 *
 * (C) InSys by Dorokhin A. Feb 2004
 *
 ******************************************************
*/

#include "basemodule.h"
#include "streamsrv.h"

#define	CURRFILE "STREAMSRV"

CMD_Info ALLOCBUF_CMD		= { BRDctrl_STREAM_CBUF_ALLOC,		0, 0, 0, NULL};
CMD_Info FREEBUF_CMD		= { BRDctrl_STREAM_CBUF_FREE,		0, 0, 0, NULL};
CMD_Info ATTACHBUF_CMD		= { BRDctrl_STREAM_CBUF_ATTACH,		1, 0, 0, NULL};
CMD_Info DETACHBUF_CMD		= { BRDctrl_STREAM_CBUF_DETACH,		1, 0, 0, NULL};
CMD_Info STARTBUF_CMD		= { BRDctrl_STREAM_CBUF_START,		0, 0, 0, NULL};
CMD_Info STOPBUF_CMD		= { BRDctrl_STREAM_CBUF_STOP,		0, 0, 0, NULL};
CMD_Info STATEBUF_CMD		= { BRDctrl_STREAM_CBUF_STATE,		1, 0, 0, NULL};
CMD_Info WAITBUF_CMD		= { BRDctrl_STREAM_CBUF_WAITBUF,	1, 0, 0, NULL};
CMD_Info WAITBUFEX_CMD		= { BRDctrl_STREAM_CBUF_WAITBUFEX,	1, 0, 0, NULL};
CMD_Info WAITBLK_CMD		= { BRDctrl_STREAM_CBUF_WAITBLOCK,	1, 0, 0, NULL};
CMD_Info WAITBLKEX_CMD		= { BRDctrl_STREAM_CBUF_WAITBLOCKEX,1, 0, 0, NULL};
CMD_Info SETDIR_CMD			= { BRDctrl_STREAM_SETDIR,    0, 0, 0, NULL};
CMD_Info SETSRC_CMD			= { BRDctrl_STREAM_SETSRC,    0, 0, 0, NULL};
CMD_Info SETDRQ_CMD			= { BRDctrl_STREAM_SETDRQ,    0, 0, 0, NULL};
CMD_Info RESETFIFO_CMD		= { BRDctrl_STREAM_RESETFIFO,    0, 0, 0, NULL};
//CMD_Info GETDIR_CMD			= { BRDctrl_STREAM_GETDIR,    1, 0, 0, NULL};
CMD_Info SETIRQ_CMD			= { BRDctrl_STREAM_SETIRQMODE,    0, 0, 0, NULL};

CMD_Info ADJUST_CMD			= { BRDctrl_STREAM_CBUF_ADJUST,    0, 0, 0, NULL};
CMD_Info DONE_CMD			= { BRDctrl_STREAM_CBUF_DONE,    0, 0, 0, NULL};

//***************************************************************************************
CStreamSrv::CStreamSrv(int idx, int srv_num, CModule* pModule, PSTREAMSRV_CFG pCfg) :
	CService(idx, _BRDC("BASESTREAM"), srv_num, pModule, pCfg, sizeof(STREAMSRV_CFG))
{
	m_attribute = 
			BRDserv_ATTR_STREAM |
			BRDserv_ATTR_DIRECTION_IN |
			BRDserv_ATTR_DIRECTION_OUT |
//			BRDserv_ATTR_UNVISIBLE |
//			BRDserv_ATTR_SUBSERVICE_ONLY |
			BRDserv_ATTR_EXCLUSIVE_ONLY;

	m_StreamNum = srv_num;

	Init(&ALLOCBUF_CMD, (CmdEntry)&CStreamSrv::CtrlAllocBufIo);
	Init(&FREEBUF_CMD, (CmdEntry)&CStreamSrv::CtrlFreeBufIo);
	Init(&ATTACHBUF_CMD, (CmdEntry)&CStreamSrv::CtrlAttachBufIo);
	Init(&DETACHBUF_CMD, (CmdEntry)&CStreamSrv::CtrlFreeBufIo);
	Init(&STARTBUF_CMD, (CmdEntry)&CStreamSrv::CtrlStartBufIo);
	Init(&STOPBUF_CMD, (CmdEntry)&CStreamSrv::CtrlStopBufIo);
	Init(&STATEBUF_CMD, (CmdEntry)&CStreamSrv::CtrlStateBufIo);
	Init(&WAITBUF_CMD, (CmdEntry)&CStreamSrv::CtrlWaitBufIo);
	Init(&WAITBUFEX_CMD, (CmdEntry)&CStreamSrv::CtrlWaitBufIoEx);
	Init(&WAITBLK_CMD, (CmdEntry)&CStreamSrv::CtrlWaitBlockIo);
	Init(&WAITBLKEX_CMD, (CmdEntry)&CStreamSrv::CtrlWaitBlockIoEx);

	Init(&SETDIR_CMD, (CmdEntry)&CStreamSrv::CtrlSetDir);
	Init(&SETSRC_CMD, (CmdEntry)&CStreamSrv::CtrlSetSrc);
	Init(&SETDRQ_CMD, (CmdEntry)&CStreamSrv::CtrlSetDrq);
	Init(&SETIRQ_CMD, (CmdEntry)&CStreamSrv::CtrlSetIrqMode);

	Init(&RESETFIFO_CMD, (CmdEntry)&CStreamSrv::CtrlResetFifo);
	Init(&ADJUST_CMD, (CmdEntry)&CStreamSrv::CtrlAdjust);
	Init(&DONE_CMD, (CmdEntry)&CStreamSrv::CtrlDone);

	m_blkNum = 0;
	m_pStub = NULL;
	m_irqMode = 0; // режим прерываний в Хост при окончании ПДП
	m_irqCount = 0; // число прерываний обычно совпадает с числов блоков

#ifdef _WIN32
	memset(&m_OvlStartStream, 0, sizeof(OVERLAPPED));
//	m_OvlStartStream.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL); // начальное состояние Non-Signaled
	m_OvlStartStream.hEvent = CreateEvent (NULL, TRUE, TRUE, NULL); // начальное состояние Signaled

	BRDCHAR nameEvent[MAX_PATH];
	BRDC_sprintf(nameEvent, _BRDC("event_%s_%d"), m_name, pModule->GetPID());
//	m_hBlockEndEvent = CreateEvent(NULL, TRUE, FALSE, nameEvent); // начальное состояние Non-Signaled
	m_hBlockEndEvent = CreateEvent(NULL, TRUE, TRUE, nameEvent); // начальное состояние Signaled
#endif
	//m_SGTblSaveAddress = NULL;
	//m_bTableSize = 0;
}

//***************************************************************************************
CStreamSrv::~CStreamSrv()
{
//	BRD_StreamState State;
//	State.timeout = 0;
//	ULONG Status = Stop(m_pModule, &State);
    if(m_blkNum && m_pStub)
        if(m_pStub->state == BRDstrm_STAT_RUN)
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
            IPC_delay(1000);
#else
            Sleep(1000);
#endif
        Stop(m_pModule);
        Free(m_pModule);

#ifdef _WIN32
	if(m_hBlockEndEvent)
		CloseHandle(m_hBlockEndEvent);
	if(m_OvlStartStream.hEvent)
		CloseHandle(m_OvlStartStream.hEvent);
#endif
}

//***************************************************************************************
// функция понадобится при реализации возможности одной службе захватывать больше одного стрима
//ULONG CStreamSrv::IsCmd(ULONG cmd)
//{
//	if(BRDctrl_STREAM_ALLOCBUF == cmd && m_blkNum)
//		return BRDerr_CMD_UNSUPPORTED;
//	else
//		return CService::IsCmd(cmd);
//}

//***************************************************************************************
int CStreamSrv::CtrlIsAvailable(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
//	CModule* pModule = (CModule*)pDev;
	CBaseModule* pModule = (CBaseModule*)pDev;
	PSERV_CMD_IsAvailable pServAvailable = (PSERV_CMD_IsAvailable)args;
	pServAvailable->attr = m_attribute;
#ifdef _WIN32
	if(m_OvlStartStream.hEvent)
		m_isAvailable = 1;
	else
		m_isAvailable = 0;
#else
	m_isAvailable = 1;
#endif

	ULONG Status = BRDerr_OK;
	AMB_GET_DMA_INFO InfoDescrip;
	for(int iStreamNum = 0; iStreamNum < MAX_NUMBER_OF_STREAMS; iStreamNum++)
	{
		InfoDescrip.DmaChanNum = iStreamNum;
		//InfoDescrip.Direction = m_dir;
		Status = pModule->StreamCtrl(STRMcmd_GETDMACHANINFO, &InfoDescrip, sizeof(AMB_GET_DMA_INFO));
		if(Status == BRDerr_OK)
			m_dirs[iStreamNum] = InfoDescrip.Direction;
		else
#ifdef _WIN32
			if(Status == ERROR_INVALID_PARAMETER)
#else
			if(Status < 0)
#endif
				m_dirs[iStreamNum] = -1;
			else
				m_dirs[iStreamNum] = 0;
	}
	//m_blkNum = 0;
	pServAvailable->isAvailable = m_isAvailable;
	return BRDerr_OK;
}

//***************************************************************************************
// буфер в системной памяти
//***************************************************************************************
int CStreamSrv::SysBuf(CBaseModule* pModule,
					   PBRDctrl_StreamCBufAlloc pStream,
					   PAMB_MEM_DMA_CHANNEL pMemDescrip,
					   int DescripSize)
{
	ULONG Status = BRDerr_OK;
	ULONG iBlk = 0;
	for(iBlk = 0; iBlk < m_blkNum; iBlk++)
		pMemDescrip->pBlock[iBlk] = NULL;
	Status = pModule->StreamCtrl(STRMcmd_SETMEMORY, pMemDescrip, DescripSize);
	if(Status || !pMemDescrip->BlockSize)
	{ // error
		delete[] m_pStrmBlk;
		m_blkNum = 0;		
	}
	else
		for(iBlk = 0; iBlk < m_blkNum; iBlk++)
		{
			m_pStrmBlk[iBlk].hFileMap = NULL;
			m_pStrmBlk[iBlk].pBlock = pMemDescrip->pBlock[iBlk];
			pStream->ppBlk[iBlk] = m_pStrmBlk[iBlk].pBlock;
		}
	return Status;
}

//***************************************************************************************
// буфер в пользовательской памяти (размещается через отображаемые файлы)
//***************************************************************************************
int CStreamSrv::FileMapBuf(CBaseModule* pModule,
						   PBRDctrl_StreamCBufAlloc pStream,
						   PAMB_MEM_DMA_CHANNEL pMemDescrip,
						   int DescripSize)
{
	ULONG Status = BRDerr_OK;
	ULONG iBlk = 0;
	ULONG nBlk = m_blkNum;
	for(iBlk = 0; iBlk < m_blkNum; iBlk++)
	{
		BRDCHAR nameFileMap[MAX_PATH];
		BRDC_sprintf(nameFileMap, _BRDC("blk_%s_%d_%d"), m_name, pModule->GetPID(), iBlk);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		m_pStrmBlk[iBlk].hFileMap = IPC_createSharedMemory(nameFileMap, m_blkSize);
		if(!m_pStrmBlk[iBlk].hFileMap)
			break;
		m_pStrmBlk[iBlk].pBlock = IPC_mapSharedMemory(m_pStrmBlk[iBlk].hFileMap);
		if(!m_pStrmBlk[iBlk].pBlock)
		{
			IPC_deleteSharedMemory(m_pStrmBlk[iBlk].hFileMap);
			break;
		}
#else
		m_pStrmBlk[iBlk].hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE,
											NULL, PAGE_READWRITE,
											0, m_blkSize,
											nameFileMap);
		if(!m_pStrmBlk[iBlk].hFileMap)
			break;
//		int isAlreadyCreated = ( GetLastError() == ERROR_ALREADY_EXISTS ) ? 1 : 0;
		m_pStrmBlk[iBlk].pBlock = (PVOID)MapViewOfFile(m_pStrmBlk[iBlk].hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		if(!m_pStrmBlk[iBlk].pBlock)
		{
			UnmapViewOfFile(m_pStrmBlk[iBlk].hFileMap);
			break;
		}
#endif
		pStream->ppBlk[iBlk] = m_pStrmBlk[iBlk].pBlock;
	}
	DescripSize -= (nBlk - iBlk) * sizeof(PVOID);
	pMemDescrip->BlockCnt = nBlk = iBlk; // могли не все блоки отмаппироваться -> pMemDescrip->BlockCnt уменьшилось
	for(iBlk = 0; iBlk < nBlk; iBlk++)
		pMemDescrip->pBlock[iBlk] = m_pStrmBlk[iBlk].pBlock;
	if(!pMemDescrip->BlockCnt)
	{
		m_blkNum = 0;
#ifdef _WIN32
		Status = ERROR_NO_SYSTEM_RESOURCES;
#else
		Status = ENOMEM;
#endif
		return Status;
	}
	Status = pModule->StreamCtrl(STRMcmd_SETMEMORY, pMemDescrip, DescripSize); //  на 0-кольце могли не все блоки закрепиться -> pMemDescrip->BlockCnt ещё уменьшилось
	if(Status || !pMemDescrip->BlockSize)
	{ // error
		for(iBlk = 0; iBlk < nBlk; iBlk++) // столько, сколько отмаппировалось 
		{
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
			IPC_unmapSharedMemory(m_pStrmBlk[iBlk].hFileMap);
			IPC_deleteSharedMemory(m_pStrmBlk[iBlk].hFileMap);
#else
			UnmapViewOfFile(m_pStrmBlk[iBlk].pBlock);
			CloseHandle(m_pStrmBlk[iBlk].hFileMap);
#endif
		}
		delete[] m_pStrmBlk;
		m_blkNum = 0;		
	}
	return Status;
}

//***************************************************************************************
// буфер уже размещен приложением в пользовательской памяти
//***************************************************************************************
int CStreamSrv::VirtualBuf(CBaseModule* pModule,
						   PBRDctrl_StreamCBufAlloc pStream,
						   PAMB_MEM_DMA_CHANNEL pMemDescrip,
						   int DescripSize)
{
	ULONG Status = BRDerr_OK;
	ULONG iBlk = 0;
	for(iBlk = 0; iBlk < m_blkNum; iBlk++)
	{
		m_pStrmBlk[iBlk].pBlock = pStream->ppBlk[iBlk];
		pMemDescrip->pBlock[iBlk] = m_pStrmBlk[iBlk].pBlock;
	}
	Status = pModule->StreamCtrl(STRMcmd_SETMEMORY, pMemDescrip, DescripSize);
	if(Status || !pMemDescrip->BlockSize)
	{ // error
		m_blkNum = 0;		
	}
	else
		if(m_isSysMem == 8) // physical memory address 
		{
			for(iBlk = 0; iBlk < m_blkNum; iBlk++)
			{
				m_pStrmBlk[iBlk].hFileMap = NULL;
				m_pStrmBlk[iBlk].pBlock = pMemDescrip->pBlock[iBlk];
				pStream->ppBlk[iBlk] = m_pStrmBlk[iBlk].pBlock;
			}
		}
	return Status;
}

//***************************************************************************************
// размещение буфера в режиме Exclusive
//***************************************************************************************
int CStreamSrv::CtrlAllocBufIo(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	//LARGE_INTEGER Frequency, StartPerformCount, StopPerformCount;
	//int bHighRes = QueryPerformanceFrequency (&Frequency);
	//QueryPerformanceCounter (&StartPerformCount);

	CBaseModule* pModule = (CBaseModule*)pDev;
	PBRDctrl_StreamCBufAlloc pStream = (PBRDctrl_StreamCBufAlloc)args;
	if(m_blkNum)
	{ // возвращаем уже размещенный буфер
//		return BRDerr_CMD_UNSUPPORTED; // понадобится при реализации возможности одной службе захватывать больше одного стрима
		pStream->dir = m_dir;
		pStream->isCont = m_isSysMem;
		pStream->blkNum = m_blkNum;
		pStream->blkSize = m_blkSize;
		for(ULONG iBlk = 0; iBlk < m_blkNum; iBlk++)
			pStream->ppBlk[iBlk] = m_pStrmBlk[iBlk].pBlock;
		pStream->pStub = m_pStub;
		return BRDerr_OK;
	}
//	if(pStream->blkSize > 64000000L)
//		return BRDerr_BUFFER_TOO_BIG;
	if(!pStream->blkSize)
		return BRDerr_BUFFER_TOO_SMALL;

	m_dir = pStream->dir;
	if(m_dirs[m_StreamNum] && (m_dirs[m_StreamNum] != BRDstrm_DIR_INOUT))
	{
		int num = -1;
		for(int iStreamNum = 0; iStreamNum < MAX_NUMBER_OF_STREAMS; iStreamNum++)
		{
			if(m_dirs[iStreamNum] == m_dir)
			//if(m_dirs[iStreamNum] & m_dir)
			{
				m_StreamNum = iStreamNum;
				num = 0;
				break;
			}
		}
		if (num == -1)
		{
			return BRDerr_BAD_PARAMETER;
		}
	}
	m_isSysMem = pStream->isCont;
	m_blkNum = pStream->blkNum;
	m_blkSize = pStream->blkSize;
	m_pStrmBlk = new STREAMSRV_BLOCK[m_blkNum];
	m_irqCount = m_blkNum;		// на всякий случай

	int DescripSize = sizeof(AMB_MEM_DMA_CHANNEL) + (m_blkNum - 1) * sizeof(PVOID);
	PAMB_MEM_DMA_CHANNEL pMemDescrip = (PAMB_MEM_DMA_CHANNEL)new UCHAR[DescripSize];
//	PBRDCHAR pBuf = m_name + (strlen(m_name) - 2);
	pMemDescrip->DmaChanNum = m_StreamNum;//_tcschr(pBuf, '1') ? 1 : 0;
	pMemDescrip->Direction = m_dir;
	pMemDescrip->MemType = (m_isSysMem == 1) ? 1 : 0;
	pMemDescrip->BlockCnt = m_blkNum;
	pMemDescrip->BlockSize = m_blkSize;
#ifdef _WIN32
	pMemDescrip->hBlockEndEvent = m_hBlockEndEvent;
#ifdef _WIN64
	pMemDescrip->hTransEndEvent = m_OvlStartStream.hEvent;
#endif
#else
	pMemDescrip->hBlockEndEvent = -1;
#endif

	pMemDescrip->pStub = NULL;
	ULONG Status = BRDerr_OK;
	ULONG iBlk = 0;

	switch(m_isSysMem)
	{
	case 0:
		Status = FileMapBuf(pModule, pStream, pMemDescrip, DescripSize);
		break;
	case 1:
		Status = SysBuf(pModule, pStream, pMemDescrip, DescripSize);
		break;
	case 2:
		Status = VirtualBuf(pModule, pStream, pMemDescrip, DescripSize);
		break;
	case 8:
		pMemDescrip->MemType = 8;
		Status = VirtualBuf(pModule, pStream, pMemDescrip, DescripSize);
		break;
	}
#ifdef _WIN32
	if(Status == ERROR_INVALID_PARAMETER)
		Status = BRDerr_BAD_PARAMETER;
	if(Status == ERROR_NO_SYSTEM_RESOURCES)
		Status = BRDerr_INSUFFICIENT_RESOURCES;
	if(Status == ERROR_NOT_SUPPORTED)
		Status = BRDerr_CMD_UNSUPPORTED;
	if(Status == ERROR_NOT_ENOUGH_MEMORY)
		Status = BRDerr_NOT_ENOUGH_MEMORY;
#else
    if(Status != 0)
	{
		if(Status == ENOMEM)
			Status = BRDerr_NOT_ENOUGH_MEMORY;
		else
			Status = BRDerr_BAD_PARAMETER;
	}
#endif
	if(!pMemDescrip->BlockSize)
		Status = BRDerr_BUFFER_TOO_SMALL;
	if(!Status)
	{
		if(m_blkSize != pMemDescrip->BlockSize)
			Status = BRDerr_PARAMETER_CHANGED;
		pStream->blkSize = pMemDescrip->BlockSize;
		m_blkSize = pStream->blkSize;
		if(m_blkNum != pMemDescrip->BlockCnt)
			Status = BRDerr_PARAMETER_CHANGED;
		pStream->blkNum = pMemDescrip->BlockCnt;
		m_blkNum = pStream->blkNum;
		for(iBlk = 0; iBlk < m_blkNum; iBlk++)
		{
			if(pMemDescrip->pBlock[iBlk] != m_pStrmBlk[iBlk].pBlock)
			{
				m_blkNum = 0;		
				Status = BRDerr_NOT_ALIGNED;
			}
		}
		pStream->pStub = m_pStub = (PBRDstrm_Stub)pMemDescrip->pStub;
		if(m_pStub)
		{
			ULONG* psgtable = (ULONG*)pStream->ppBlk[0];
			if (2 != m_isSysMem)
			{
				psgtable[3] = m_pStub->lastBlock;
				psgtable[4] = m_pStub->totalCounter;
			}
			m_irqCount = m_pStub->offset;
			m_pStub->lastBlock = -1;
			m_pStub->totalCounter = 0;
			m_pStub->offset = 0;
		}
	}

/*	if(m_isSysMem)
	{
		for(iBlk = 0; iBlk < m_blkNum; iBlk++)
			pMemDescrip->pBlock[iBlk] = NULL;
		Status = pModule->StreamCtrl(STRMcmd_SETMEMORY, pMemDescrip, DescripSize, NULL);
		if(Status)
		{
			delete[] m_pStrmBlk;
			m_blkNum = 0;		
		}
		else
			for(iBlk = 0; iBlk < m_blkNum; iBlk++)
			{
				m_pStrmBlk[iBlk].hFileMap = NULL;
				m_pStrmBlk[iBlk].pBlock = pMemDescrip->pBlock[iBlk];
				pStream->ppBlk[iBlk] = m_pStrmBlk[iBlk].pBlock;
			}
	}
	else
	{
		for(iBlk = 0; iBlk < m_blkNum; iBlk++)
		{
	//		m_ppBlk[iBlk] = VirtualAlloc(NULL, m_blkSize, );
			BRDCHAR nameFileMap[MAX_PATH];
			sprintf_s(nameFileMap, _BRDC("blk_%s_%d_%d"), m_name, pModule->GetPID(), iBlk);
			m_pStrmBlk[iBlk].hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE,
												NULL, PAGE_READWRITE,
												0, m_blkSize,
												nameFileMap);
	//		int isAlreadyCreated = ( GetLastError() == ERROR_ALREADY_EXISTS ) ? 1 : 0;
			m_pStrmBlk[iBlk].pBlock = (PVOID)MapViewOfFile(m_pStrmBlk[iBlk].hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
			pStream->ppBlk[iBlk] = m_pStrmBlk[iBlk].pBlock;
		}
		for(iBlk = 0; iBlk < m_blkNum; iBlk++)
			pMemDescrip->pBlock[iBlk] = m_pStrmBlk[iBlk].pBlock;
		Status = pModule->StreamCtrl(STRMcmd_SETMEMORY, pMemDescrip, DescripSize, NULL);
		if(Status)
		{
			for(iBlk = 0; iBlk < m_blkNum; iBlk++)
			{
				UnmapViewOfFile(m_pStrmBlk[iBlk].pBlock);
				CloseHandle(m_pStrmBlk[iBlk].hFileMap);
			}
			delete[] m_pStrmBlk;
			m_blkNum = 0;		
		}
	}*/

	delete pMemDescrip;

	//QueryPerformanceCounter (&StopPerformCount);
	//double msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
	//BRDCHAR msg[256];
	//sprintf(msg, _BRDC("Allocate memory = %f msec"), msTime);
	//MessageBox(NULL, msg, "Debug", MB_OK);

	//ULONG* psgtable = (ULONG*)pStream->ppBlk[0];
	//m_SGTableAddress = (void*)psgtable[0];
	////psgtable[1] = m_ScatterGatherTableEntryCnt;
	//ULONG ScatterGatherBlockCnt = psgtable[2];
	//m_bTableSize = ScatterGatherBlockCnt * 512;
	//m_SGTblSaveAddress = new BYTE[m_bTableSize];
	//memcpy(m_SGTblSaveAddress, m_SGTableAddress, m_bTableSize);

	return Status;
//	return BRDerr_OK;
}

//***************************************************************************************
// размещение буфера в режиме Spy
//***************************************************************************************
int CStreamSrv::CtrlAttachBufIo(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CBaseModule* pModule = (CBaseModule*)pDev;
	PBRDctrl_StreamCBufAttach pStream = (PBRDctrl_StreamCBufAttach)args;
	if(m_blkNum)
	{ // возвращаем уже размещенный буфер
		pStream->dir = m_dir;
		pStream->isCont = m_isSysMem;
		pStream->blkNum = m_blkNum;
		pStream->blkSize = m_blkSize;
		for(ULONG iBlk = 0; iBlk < m_blkNum; iBlk++)
			pStream->ppBlk[iBlk] = m_pStrmBlk[iBlk].pBlock;
		pStream->pStub = m_pStub;
		return BRDerr_OK;
	}
	m_dir = pStream->dir;
	m_isSysMem = pStream->isCont;
	m_blkNum = pStream->blkNum;
	m_blkSize = pStream->blkSize;
	m_pStrmBlk = new STREAMSRV_BLOCK[m_blkNum];

	int DescripSize = sizeof(AMB_MEM_DMA_CHANNEL) + (m_blkNum - 1) * sizeof(PVOID);
	PAMB_MEM_DMA_CHANNEL pMemDescrip = (PAMB_MEM_DMA_CHANNEL)new UCHAR[DescripSize];
//	PBRDCHAR pBuf = m_name + (strlen(m_name) - 2);
	pMemDescrip->DmaChanNum = m_StreamNum;//_tcschr(pBuf, '1') ? 1 : 0;
	pMemDescrip->Direction = m_dir;
	pMemDescrip->MemType = m_isSysMem;
	pMemDescrip->BlockCnt = m_blkNum;
	pMemDescrip->BlockSize = m_blkSize;
	pMemDescrip->pStub = NULL;
	ULONG Status = BRDerr_OK;
	ULONG iBlk = 0;
	if(m_isSysMem)
	{
		for(iBlk = 0; iBlk < m_blkNum; iBlk++)
			pMemDescrip->pBlock[iBlk] = NULL;
		Status = pModule->StreamCtrl(STRMcmd_SETMEMORY, pMemDescrip, DescripSize);
		if(Status)
		{
			delete[] m_pStrmBlk;
			m_blkNum = 0;		
		}
		else
			for(iBlk = 0; iBlk < m_blkNum; iBlk++)
			{
				m_pStrmBlk[iBlk].hFileMap = NULL;
				m_pStrmBlk[iBlk].pBlock = pMemDescrip->pBlock[iBlk];
				pStream->ppBlk[iBlk] = m_pStrmBlk[iBlk].pBlock;
			}
	}
	else
	{
		for(iBlk = 0; iBlk < m_blkNum; iBlk++)
		{
	//		m_ppBlk[iBlk] = VirtualAlloc(NULL, m_blkSize, );
			BRDCHAR nameFileMap[MAX_PATH];
			BRDC_sprintf(nameFileMap, _BRDC("blk_%s_%d_%d"), m_name, pModule->GetPID(), iBlk);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
			int isAlreadyCreated;
			m_pStrmBlk[iBlk].hFileMap = IPC_createSharedMemoryEx(nameFileMap, m_blkSize, &isAlreadyCreated);
#else
			m_pStrmBlk[iBlk].hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE,
												NULL, PAGE_READWRITE,
												0, m_blkSize,
												nameFileMap);
			int isAlreadyCreated = ( GetLastError() == ERROR_ALREADY_EXISTS ) ? 1 : 0;
#endif
			if(!isAlreadyCreated)
			{
				delete pMemDescrip;
				delete[] m_pStrmBlk;
				return BRDerr_STREAM_NOT_ALLOCATED_YET;
			}
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
			m_pStrmBlk[iBlk].pBlock = IPC_mapSharedMemory(m_pStrmBlk[iBlk].hFileMap);
#else
			m_pStrmBlk[iBlk].pBlock = (PVOID)MapViewOfFile(m_pStrmBlk[iBlk].hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
#endif
			pStream->ppBlk[iBlk] = m_pStrmBlk[iBlk].pBlock;
		}
		for(iBlk = 0; iBlk < m_blkNum; iBlk++)
			pMemDescrip->pBlock[iBlk] = m_pStrmBlk[iBlk].pBlock;
		Status = pModule->StreamCtrl(STRMcmd_SETMEMORY, pMemDescrip, DescripSize);
		if(Status)
		{
			for(iBlk = 0; iBlk < m_blkNum; iBlk++)
			{
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
				IPC_unmapSharedMemory(m_pStrmBlk[iBlk].hFileMap);
				IPC_deleteSharedMemory(m_pStrmBlk[iBlk].hFileMap);
#else
				UnmapViewOfFile(m_pStrmBlk[iBlk].pBlock);
				CloseHandle(m_pStrmBlk[iBlk].hFileMap);
#endif
			}
			delete[] m_pStrmBlk;
			m_blkNum = 0;		
		}
	}
	pStream->pStub = m_pStub = (PBRDstrm_Stub)pMemDescrip->pStub;

	delete pMemDescrip;

#ifdef _WIN32
	if(Status == ERROR_INVALID_PARAMETER)
		Status = BRDerr_BAD_PARAMETER;
#endif
	return Status;
//	return BRDerr_OK;
}

//***************************************************************************************
ULONG CStreamSrv::Free(void* pDev)
{
	ULONG Status = BRDerr_OK;
	CBaseModule* pModule = (CBaseModule*)pDev;
	if(m_blkNum)
	{
		int DescripSize = sizeof(AMB_MEM_DMA_CHANNEL) + (m_blkNum - 1) * sizeof(PVOID);
		PAMB_MEM_DMA_CHANNEL pMemDescrip = (PAMB_MEM_DMA_CHANNEL)new UCHAR[DescripSize];
		pMemDescrip->DmaChanNum = m_StreamNum;
		pMemDescrip->MemType = m_isSysMem;
		pMemDescrip->BlockCnt = m_blkNum;
		pMemDescrip->BlockSize = m_blkSize;
		pMemDescrip->pStub = m_pStub;
		for(ULONG iBlk = 0; iBlk < m_blkNum; iBlk++)
			pMemDescrip->pBlock[iBlk] = m_pStrmBlk[iBlk].pBlock;
		Status = pModule->StreamCtrl(STRMcmd_FREEMEMORY, pMemDescrip, DescripSize);
		if(!m_isSysMem)
		{
			for(ULONG iBlk = 0; iBlk < m_blkNum; iBlk++)
			{
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
				IPC_unmapSharedMemory(m_pStrmBlk[iBlk].hFileMap);
				IPC_deleteSharedMemory(m_pStrmBlk[iBlk].hFileMap);
#else
				BOOL ret = UnmapViewOfFile(m_pStrmBlk[iBlk].pBlock);
				ret = CloseHandle(m_pStrmBlk[iBlk].hFileMap);
#endif
			}
		}
		delete[] m_pStrmBlk;
		m_blkNum = 0;
		delete pMemDescrip;
		m_pStub = NULL;
	}
	return Status;
}	

//***************************************************************************************
int CStreamSrv::CtrlFreeBufIo(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	//CBaseModule* pModule = (CBaseModule*)pDev;
// закомментированный фрагмент понадобится при реализации возможности одной службе захватывать больше одного стрима
//	PBRD_StreamBuf pStream = (PBRD_StreamBuf)args;
	//if(m_blkNum)
	//{ // проверяем тот ли стрим
	//	if(pStream->pStub != m_pStub)
	//		return BRDerr_CMD_UNSUPPORTED;
	//	for(ULONG iBlk = 0; iBlk < m_blkNum; iBlk++)
	//	{
	//		if(pStream->ppBlk[iBlk] != m_pStrmBlk[iBlk].pBlock)
	//			return BRDerr_CMD_UNSUPPORTED;
	//	}
	//}
	//else
	//	return BRDerr_OK;
	if(!m_blkNum)
		return BRDerr_STREAM_NOT_ALLOCATED_YET;
//	BRD_StreamState State;
//	State.timeout = 0;
//	ULONG Status = Stop(m_pModule, &State);
	ULONG Status = Stop(m_pModule);
	if(Status == BRDerr_OK)
	{
		if(m_pStub->state != BRDstrm_STAT_STOP)
			return BRDerr_STREAM_NOT_STOPPED_YET;
		Status = Free(pDev);
		if(Status == BRDerr_OK && pServData)
		{
			//delete m_SGTblSaveAddress;
			PUNDERSERV_Cmd pReleaseCmd = (PUNDERSERV_Cmd)pServData;
			pReleaseCmd->code = SERVcmd_SYS_RELEASE;
		}
	}
#ifdef _WIN32
	if(Status == ERROR_INVALID_PARAMETER)
#else
	if(Status < 0)
#endif
		Status = BRDerr_BAD_PARAMETER;
	return Status;
//	return BRDerr_OK;
}

//***************************************************************************************
int CStreamSrv::CtrlStartBufIo(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CBaseModule* pModule = (CBaseModule*)pDev;
	PBRDctrl_StreamCBufStart pStart = (PBRDctrl_StreamCBufStart)args;
	ULONG Status = BRDerr_OK;
	if(m_blkNum && m_pStub)
	{
		if(m_pStub->state != BRDstrm_STAT_STOP)
			return BRDerr_STREAM_ALREADY_STARTED;
#ifdef _WIN32
		ResetEvent(m_OvlStartStream.hEvent); // сброс в состояние Non-Signaled перед стартом
		ResetEvent(m_hBlockEndEvent); // сброс в состояние Non-Signaled перед стартом
#endif
		m_isCycling = pStart->isCycle;
		AMB_START_DMA_CHANNEL StartDescrip;
		StartDescrip.DmaChanNum = m_StreamNum;
		StartDescrip.IsCycling = m_isCycling;
//		StartDescrip.hBlockEndEvent = m_hBlockEndEvent;
#if defined (_WIN64) || defined (__linux__)
		Status = pModule->StreamCtrl(STRMcmd_STARTMEMORY, &StartDescrip, sizeof(AMB_START_DMA_CHANNEL));
#else
		Status = pModule->StreamCtrl(STRMcmd_STARTMEMORY, &StartDescrip, sizeof(AMB_START_DMA_CHANNEL), &m_OvlStartStream);
		if(Status == ERROR_IO_PENDING)
		{
			//memcpy(m_SGTblSaveAddress, m_SGTableAddress, m_bTableSize);
			Status = BRDerr_OK;
		}
#endif
	}
	else
		return BRDerr_STREAM_NOT_ALLOCATED_YET;
#ifdef _WIN32
	if(Status == ERROR_INVALID_PARAMETER)
#else
	if(Status < 0)
#endif
		Status = BRDerr_BAD_PARAMETER;
	return Status;
//	return BRDerr_OK;
}

//***************************************************************************************
//ULONG CStreamSrv::Stop(void		*pDev,
//					   PBRD_StreamState pState)
ULONG CStreamSrv::Stop(void		*pDev)
{
	ULONG Status = BRDerr_OK;
	CBaseModule* pModule = (CBaseModule*)pDev;
    if(m_blkNum && m_pStub)
	{
		if(m_pStub->state == BRDstrm_STAT_RUN)
		{
			AMB_STATE_DMA_CHANNEL StateDescrip;
			StateDescrip.DmaChanNum = m_StreamNum;
			StateDescrip.Timeout = 0;//pState->timeout; останавливает немедленно (в 0-кольце оставлю пока возможность ожидания)
			Status = pModule->StreamCtrl(STRMcmd_STOPMEMORY, &StateDescrip, sizeof(AMB_STATE_DMA_CHANNEL));
			//if(Status == BRDerr_OK)
			//{
			//	pState->blkNum = StateDescrip.BlockNum;
			//	pState->blkNumTotal = StateDescrip.BlockCntTotal;
			//	pState->offset = StateDescrip.OffsetInBlock;
			//	pState->state = StateDescrip.DmaChanState;
			//}
		}
		//else
		//{
		//	pState->blkNum = m_pStub->lastBlock;
		//	pState->blkNumTotal = m_pStub->totalCounter;
		//	pState->offset = m_pStub->offset;
		//	pState->state = m_pStub->state;
		//}
	}
	else
		return BRDerr_STREAM_NOT_ALLOCATED_YET;
	return Status;
}

//***************************************************************************************
int CStreamSrv::CtrlStopBufIo(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
//	CBaseModule* pModule = (CBaseModule*)pDev;
//	PBRD_StreamState pState = (PBRD_StreamState)args;
//	ULONG Status = Stop(pDev, pState);
	ULONG Status = Stop(pDev);
#ifdef _WIN32
	if(Status == ERROR_INVALID_PARAMETER)
#else
	if(Status < 0)
#endif
		Status = BRDerr_BAD_PARAMETER;
	return Status;
//	return BRDerr_OK;
}

//***************************************************************************************
int CStreamSrv::CtrlStateBufIo(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CBaseModule* pModule = (CBaseModule*)pDev;
	PBRDctrl_StreamCBufState pState = (PBRDctrl_StreamCBufState)args;
	ULONG Status = BRDerr_OK;
	if(m_blkNum)
	{
#ifdef _WIN32
		ULONG flwt = 0;
		if(pState->timeout)
		{
			Status = WaitForSingleObject(m_hBlockEndEvent, pState->timeout);
			if(Status != WAIT_TIMEOUT) 
				ResetEvent(m_hBlockEndEvent); // сброс в состояние Non-Signaled после завершения блока
			else
				flwt = 1;
		}
#endif
		AMB_STATE_DMA_CHANNEL StateDescrip;
		StateDescrip.DmaChanNum = m_StreamNum;
		StateDescrip.Timeout = 0;//pState->timeout;
		Status = pModule->StreamCtrl(STRMcmd_STATEMEMORY, &StateDescrip, sizeof(AMB_STATE_DMA_CHANNEL));
		if(Status == BRDerr_OK)
		{
			pState->blkNum = StateDescrip.BlockNum;
			pState->blkNumTotal = StateDescrip.BlockCntTotal;
			pState->offset = StateDescrip.OffsetInBlock;
			pState->state = StateDescrip.DmaChanState;
#ifdef _WIN32
			if(flwt)
			{
				//int res = memcmp(m_SGTblSaveAddress, m_SGTableAddress, m_bTableSize);
				Status = BRDerr_WAIT_TIMEOUT;
			}
#endif
		}
	}
	else
	{
		pState->blkNum = 0;
		pState->blkNumTotal = 0;
		pState->offset = 0;
		pState->state = 0;
		Status = BRDerr_STREAM_NOT_ALLOCATED_YET;
	}
#ifdef _WIN32
	if(Status == ERROR_INVALID_PARAMETER)
#else
	if(Status < 0)
#endif
		Status = BRDerr_BAD_PARAMETER;
	return Status;
//	return BRDerr_OK;
}

//***************************************************************************************
int CStreamSrv::CtrlWaitBufIo(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_OK;
	ULONG msTimeout = *(PULONG)args;
	if(m_blkNum)
	{
		if(m_irqMode)
		{	// режим записи по адресу из таблицы при окончании ПДП
			CBaseModule* pModule = (CBaseModule*)pDev;
			AMB_SET_DMA_CHANNEL SetDescrip;
			SetDescrip.DmaChanNum = m_StreamNum;
			//SetDescrip.Param = msTimeout;
			int tcnt = msTimeout / 50;
			int iCnt = 0;
			for(iCnt = 0; iCnt < tcnt; iCnt++)
			{
				Status = pModule->StreamCtrl(STRMcmd_GETIRQCNT, &SetDescrip, sizeof(AMB_SET_DMA_CHANNEL));
				//if(SetDescrip.Param >= m_blkNum)
				if(SetDescrip.Param >= m_irqCount)
					break;
				#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
						IPC_delay(50);
				#else
						Sleep(50);
				#endif
			}
			if(iCnt >= tcnt) 
				Status = BRDerr_WAIT_TIMEOUT;
		}
		else
		{	// режим прерываний в Хост при окончании ПДП
#ifdef _WIN32
			Status = WaitForSingleObject(m_OvlStartStream.hEvent, msTimeout);
			if(Status == WAIT_TIMEOUT) 
				Status = BRDerr_WAIT_TIMEOUT;
#ifdef _WIN64
			else
				ResetEvent(m_OvlStartStream.hEvent); // сброс в состояние Non-Signaled после завершения всего буфера
#endif
#else
			CBaseModule* pModule = (CBaseModule*)pDev;
			AMB_STATE_DMA_CHANNEL StateDescrip;
			StateDescrip.DmaChanNum = m_StreamNum;
			StateDescrip.Timeout = msTimeout;
			Status = pModule->StreamCtrl(STRMcmd_WAITDMABUFFER, &StateDescrip, sizeof(AMB_STATE_DMA_CHANNEL));
            if(Status == (int)ETIMEDOUT) {
				Status = BRDerr_WAIT_TIMEOUT;
			}
#endif
		}
	}
	else
		Status = BRDerr_STREAM_NOT_ALLOCATED_YET;
	return Status;
}

//***************************************************************************************
int CStreamSrv::CtrlWaitBufIoEx(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
//	CModule* pModule = (CModule*)pDev;
#ifdef _WIN32
	if(m_blkNum)
	{
		//ULONG msTimeout = *(PULONG)args;
		//int Status = WaitForSingleObject(m_OvlStartStream.hEvent, msTimeout);
		//if(Status == WAIT_TIMEOUT) 
		//	return BRDerr_WAIT_TIMEOUT;
		PBRD_WaitEvent pWaitEvent = (PBRD_WaitEvent)args;
		ULONG msTimeout = pWaitEvent->timeout;
		HANDLE Events[2];
		Events[0] = pWaitEvent->hAppEvent;
		Events[1] = m_OvlStartStream.hEvent;
		int Status = WaitForMultipleObjects( 2, Events, FALSE, msTimeout);
		if(Status == WAIT_TIMEOUT) 
			return BRDerr_WAIT_TIMEOUT;
		else
		{
#ifdef _WIN64
			if(Status - WAIT_OBJECT_0)
				ResetEvent(m_hBlockEndEvent); // сброс в состояние Non-Signaled после завершения сбора
			else
				return BRDerr_SIGNALED_APPEVENT;
#else
			if(!(Status - WAIT_OBJECT_0))
				return BRDerr_SIGNALED_APPEVENT;
#endif
		}
	}
	else
		return BRDerr_STREAM_NOT_ALLOCATED_YET;
#else
	fprintf(stdout, "%s: %s()\n", __FILE__, __FUNCTION__);
#endif
	return BRDerr_OK;
}

//***************************************************************************************
int CStreamSrv::CtrlWaitBlockIo(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_OK;
	ULONG msTimeout = *(PULONG)args;
	if(m_blkNum)
	{
		if(m_irqMode)
		{	// режим записи по адресу из таблицы при окончании ПДП
			CBaseModule* pModule = (CBaseModule*)pDev;
			AMB_SET_DMA_CHANNEL SetDescrip;
			SetDescrip.DmaChanNum = m_StreamNum;
			Status = pModule->StreamCtrl(STRMcmd_GETIRQCNT, &SetDescrip, sizeof(AMB_SET_DMA_CHANNEL));
			ULONG pre_blk_num = SetDescrip.Param;
			//SetDescrip.Param = msTimeout;
			int tcnt = msTimeout / 50;
			int iCnt = 0;
			for(iCnt = 0; iCnt < tcnt; iCnt++)
			{
				Status = pModule->StreamCtrl(STRMcmd_GETIRQCNT, &SetDescrip, sizeof(AMB_SET_DMA_CHANNEL));
				if(SetDescrip.Param != pre_blk_num)
					break;
                #if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
						IPC_delay(50);
				#else
						Sleep(50);
				#endif
			}
			if(iCnt >= tcnt) 
				Status = BRDerr_WAIT_TIMEOUT;
		}
		else
		{	// режим прерываний в Хост при окончании ПДП
#ifdef _WIN32
			Status = WaitForSingleObject(m_hBlockEndEvent, msTimeout);
			if(Status == WAIT_TIMEOUT) 
				Status = BRDerr_WAIT_TIMEOUT;
			else
				ResetEvent(m_hBlockEndEvent); // сброс в состояние Non-Signaled после завершения блока
#else
			CBaseModule* pModule = (CBaseModule*)pDev;
			AMB_STATE_DMA_CHANNEL StateDescrip;
			StateDescrip.DmaChanNum = m_StreamNum;
			StateDescrip.Timeout = msTimeout;
			Status = pModule->StreamCtrl(STRMcmd_WAITDMABLOCK, &StateDescrip, sizeof(AMB_STATE_DMA_CHANNEL));
            if(Status == (int)ETIMEDOUT) {
				Status = BRDerr_WAIT_TIMEOUT;
			}
#endif
		}
	}
	else
		Status = BRDerr_STREAM_NOT_ALLOCATED_YET;
	return Status;
}

//***************************************************************************************
int CStreamSrv::CtrlWaitBlockIoEx(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
//	CModule* pModule = (CModule*)pDev;
#ifdef _WIN32
	if(m_blkNum)
	{
		//ULONG msTimeout = *(PULONG)args;
		//int Status = WaitForSingleObject(m_hBlockEndEvent, msTimeout);
		//if(Status == WAIT_TIMEOUT) 
		//	return BRDerr_WAIT_TIMEOUT;
		//else
		//	ResetEvent(m_hBlockEndEvent); // сброс в состояние Non-Signaled после завершения блока

		PBRD_WaitEvent pWaitEvent = (PBRD_WaitEvent)args;
		ULONG msTimeout = pWaitEvent->timeout;
		HANDLE Events[2];
		Events[0] = pWaitEvent->hAppEvent;
		Events[1] = m_hBlockEndEvent;
		int Status = WaitForMultipleObjects( 2, Events, FALSE, msTimeout);
		if(Status == WAIT_TIMEOUT) 
			return BRDerr_WAIT_TIMEOUT;
		else
		{
			if(Status - WAIT_OBJECT_0)
				ResetEvent(m_hBlockEndEvent); // сброс в состояние Non-Signaled после завершения сбора
			else
				return BRDerr_SIGNALED_APPEVENT;
		}
	}
	else
		return BRDerr_STREAM_NOT_ALLOCATED_YET;
#else
	fprintf(stdout, "%s: %s()\n", __FILE__, __FUNCTION__);
#endif
	return BRDerr_OK;
}

//***************************************************************************************
int CStreamSrv::CtrlSetDir(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CBaseModule* pModule = (CBaseModule*)pDev;
	ULONG direction = *(PULONG)args;
	ULONG Status = BRDerr_OK;
	AMB_SET_DMA_CHANNEL SetDescrip;
	SetDescrip.DmaChanNum = m_StreamNum;
	SetDescrip.Param = direction;
	Status = pModule->StreamCtrl(STRMcmd_SETDIR, &SetDescrip, sizeof(AMB_SET_DMA_CHANNEL));
#ifdef _WIN32
	if(Status == ERROR_INVALID_PARAMETER)
#else
	if(Status < 0)
#endif
		Status = BRDerr_BAD_PARAMETER;
	return Status;
}

//***************************************************************************************
int CStreamSrv::CtrlSetSrc(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CBaseModule* pModule = (CBaseModule*)pDev;
	ULONG source = *(PULONG)args;
	ULONG Status = BRDerr_OK;
	AMB_SET_DMA_CHANNEL SetDescrip;
	SetDescrip.DmaChanNum = m_StreamNum;
	if(source > 1024)				 // вообще-то это номер тетрады, а он не может быть больше 16, 
		return BRDerr_BAD_PARAMETER; //но тут защита от дурака, который вообще не проверяет возвращаемые статусы
	SetDescrip.Param = source;
	Status = pModule->StreamCtrl(STRMcmd_SETSRC, &SetDescrip, sizeof(AMB_SET_DMA_CHANNEL));
#ifdef _WIN32
	if(Status == ERROR_INVALID_PARAMETER)
#else
	if(Status < 0)
#endif
		Status = BRDerr_BAD_PARAMETER;
	return Status;
}

//***************************************************************************************
int CStreamSrv::CtrlSetDrq(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CBaseModule* pModule = (CBaseModule*)pDev;
	ULONG flag = *(PULONG)args;
	ULONG Status = BRDerr_OK;
	AMB_SET_DMA_CHANNEL SetDescrip;
	SetDescrip.DmaChanNum = m_StreamNum;
	SetDescrip.Param = flag;
	Status = pModule->StreamCtrl(STRMcmd_SETDRQ, &SetDescrip, sizeof(AMB_SET_DMA_CHANNEL));
#ifdef _WIN32
	if(Status == ERROR_INVALID_PARAMETER)
#else
	if(Status < 0)
#endif
		Status = BRDerr_BAD_PARAMETER;
	return Status;
}

//***************************************************************************************
int CStreamSrv::CtrlResetFifo(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CBaseModule* pModule = (CBaseModule*)pDev;
	ULONG Status = BRDerr_OK;
	AMB_SET_DMA_CHANNEL SetDescrip;
	SetDescrip.DmaChanNum = m_StreamNum;
//	SetDescrip.Param = flag;
	Status = pModule->StreamCtrl(STRMcmd_RESETFIFO, &SetDescrip, sizeof(AMB_SET_DMA_CHANNEL));
#ifdef _WIN32
	if(Status == ERROR_INVALID_PARAMETER)
#else
	if(Status < 0)
#endif
		Status = BRDerr_BAD_PARAMETER;
	return Status;
}

//***************************************************************************************
int CStreamSrv::CtrlSetIrqMode(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CBaseModule* pModule = (CBaseModule*)pDev;
	PBRDctrl_StreamSetIrqMode pIrqMode = (PBRDctrl_StreamSetIrqMode)args;
	ULONG Status = BRDerr_OK;
	PAMB_SET_IRQ_TABLE pSetDescrip = NULL;
	int DescripSize = sizeof(AMB_SET_IRQ_TABLE);

	if(pIrqMode->mode)
	{
		if(pIrqMode->irqNum > 512 && pIrqMode->irqNum < 4)
			return BRDerr_BAD_PARAMETER;
		DescripSize = sizeof(AMB_SET_IRQ_TABLE) + (pIrqMode->irqNum - 4) * sizeof(U32);
		pSetDescrip = (PAMB_SET_IRQ_TABLE)new UCHAR[DescripSize];
		pSetDescrip->DmaChanNum = m_StreamNum;
		pSetDescrip->Mode = pIrqMode->mode;
		pSetDescrip->TableNum = pIrqMode->irqNum;
		for(U32 iAdr = 0; iAdr < pIrqMode->irqNum; iAdr++)
			pSetDescrip->AddrTable[iAdr] = pIrqMode->pIrqTable[iAdr];
	}
	else
	{
		pSetDescrip = (PAMB_SET_IRQ_TABLE)new UCHAR[DescripSize];
		pSetDescrip->DmaChanNum = m_StreamNum;
		pSetDescrip->Mode = 0;
		pSetDescrip->TableNum = 4;
		for(U32 iAdr = 0; iAdr < 4; iAdr++)
			pSetDescrip->AddrTable[iAdr] = 0;
	}
	Status = pModule->StreamCtrl(STRMcmd_SETIRQMODE, pSetDescrip, DescripSize);
	if(Status == BRDerr_OK)
		m_irqMode = pIrqMode->mode;
	else
		pIrqMode->mode = 0;
#ifdef _WIN32
	if(Status == ERROR_INVALID_PARAMETER)
		Status = BRDerr_BAD_PARAMETER;
	if(Status == ERROR_NOT_SUPPORTED)
		Status = BRDerr_CMD_UNSUPPORTED;
#else
	if(Status < 0)
	{
		if(Status == ENOSYS)
			Status = BRDerr_CMD_UNSUPPORTED;
		else
			Status = BRDerr_BAD_PARAMETER;
	}
#endif
	return Status;
}

//***************************************************************************************
int CStreamSrv::CtrlAdjust(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CBaseModule* pModule = (CBaseModule*)pDev;
	ULONG flag = *(PULONG)args;
	ULONG Status = BRDerr_OK;
	AMB_SET_DMA_CHANNEL SetDescrip;
	SetDescrip.DmaChanNum = m_StreamNum;
	SetDescrip.Param = flag;
	Status = pModule->StreamCtrl(STRMcmd_ADJUST, &SetDescrip, sizeof(AMB_SET_DMA_CHANNEL));
#ifdef _WIN32
	if(Status == ERROR_INVALID_PARAMETER)
#else
	if(Status < 0)
#endif
		Status = BRDerr_BAD_PARAMETER;
	return Status;
}

//***************************************************************************************
int CStreamSrv::CtrlDone(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CBaseModule* pModule = (CBaseModule*)pDev;
	ULONG blknum = *(PULONG)args;
	ULONG Status = BRDerr_OK;
	AMB_SET_DMA_CHANNEL SetDescrip;
	SetDescrip.DmaChanNum = m_StreamNum;
	SetDescrip.Param = blknum;
	Status = pModule->StreamCtrl(STRMcmd_DONE, &SetDescrip, sizeof(AMB_SET_DMA_CHANNEL));
#ifdef _WIN32
	if(Status == ERROR_INVALID_PARAMETER)
#else
	if(Status < 0)
#endif
		Status = BRDerr_BAD_PARAMETER;
	return Status;
}

//***************************************************************************************
//int CStreamSrv::CtrlGetDir(
//						void		*pDev,		// InfoSM or InfoBM
//						void		*pServData,	// Specific Service Data
//						ULONG		cmd,		// Command Code (from BRD_ctrl())
//						void		*args 		// Command Arguments (from BRD_ctrl())
//						)
//{
//	CBaseModule* pModule = (CBaseModule*)pDev;
//	ULONG Status = BRDerr_OK;
//	AMB_GET_DMA_INFO InfoDescrip;
//	InfoDescrip.DmaChanNum = m_StreamNum;
//	//InfoDescrip.Direction = m_dir[m_StreamNum];
//	Status = pModule->StreamCtrl(STRMcmd_GETDMACHANINFO, &InfoDescrip, sizeof(AMB_GET_DMA_INFO), NULL);
//	*(PULONG)args = InfoDescrip.Direction;
//	return Status;
//}

// ***************** End of file StreamSrv.cpp ***********************
