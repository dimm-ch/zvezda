
#include "baxizynq.h"
#ifdef __linux__
#include <sys/time.h>
#endif

#define	CURRFILE _BRDC("BAXIZYNQLL")


ULONG GetLastError(void)
{
	return errno;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetPldStatus(ULONG& PldStatus, ULONG PldNum)
{
	PldStatus = 1;

	return BRDerr_OK;
}

static UCHAR EepromBase[512];

//-----------------------------------------------------------------------------

ULONG CAxizynq::ReadNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	memset(pBuffer, 0xff, BufferSize);

	int base_offset = OFFSET_BASE_CFGMEM / 2;
	int base_size = MAX_BASE_CFGMEM_SIZE / 2;
	int res = m_blk_main->ReadEEPROM(1, (uint16_t*)EepromBase + base_offset, base_offset, base_size);
	if (res < 0) {
		fprintf(stderr, "%s(): Error in ReadEEPROM()\n", __FUNCTION__);
		return ~0x0;
	}

	memcpy(pBuffer, EepromBase + Offset, BufferSize);
/*
    fprintf(stderr, "IOCTL_AMB_READ_NVRAM\n");
    u8 *ptr = (u8*)pBuffer;
    for(int i=0; i<base_size; i++) {
        if((i%16) == 0) {
            fprintf(stderr, "\n");
        }
        fprintf(stderr, "0x%02x ", ptr[i]);
    }
    fprintf(stderr, "\n");
*/
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	memcpy(pBuffer, EepromBase + Offset, BufferSize);
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::WriteNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	if(!BufferSize)
		return BRDerr_OK;

	memcpy(EepromBase + Offset, pBuffer, BufferSize);

	int base_offset = OFFSET_BASE_CFGMEM / 2;
	int base_size = MAX_BASE_CFGMEM_SIZE / 2;
	int res = m_blk_main->WriteEEPROM(1, (uint16_t*)EepromBase + base_offset, base_offset, base_size);
	if (res < 0) {
		fprintf(stderr, "%s(): Error in WriteEEPROM()\n", __FUNCTION__);
		return ~0x0;
	}

    IPC_delay(10);
	return BRDerr_OK;
}

static UCHAR EepromSubmod[512];

//-----------------------------------------------------------------------------

ULONG CAxizynq::ReadSubICR(int submod, PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG Status = BRDerr_OK;

	memset(EepromSubmod, 0xaa, 256);

    int sub_offset = 0x400;
    int sub_size = 256;
    int res = m_blk_fid->ReadFmcEeprom(submod, 0x50, (uint8_t*)EepromSubmod, sub_offset, sub_size);
	if (res < 0) {
		fprintf(stderr, "%s(): Error in ReadEEPROM()\n", __FUNCTION__);
		return ~0x0;
	}

	memcpy(pBuffer, EepromSubmod + Offset, BufferSize);
/*
	fprintf(stderr, "ReadSubICR\n");
    u8 *ptr = (u8*)pBuffer;
    for(int i=0; i<64; i++) {
        if((i%16) == 0) {
            fprintf(stderr, "\n");
        }
        fprintf(stderr, "0x%02x ", ptr[i]);
    }
    fprintf(stderr, "\n");
*/
	//if (m_blk_fid->is_valid())
	//{
	//	memset(EepromSubmod, 0xaa, 256);
	//	if(submod)
	//		Status = ReadFidSpd(2, 0x50, EepromSubmod, 0x400, 256); // FMC2
	//	else
	//		Status = ReadFidSpd(0, 0x50, EepromSubmod, 0x400, 256); // FMC1

	//	memcpy(pBuffer, EepromSubmod + Offset, BufferSize);
	//}
	return Status;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::WriteSubICR(int submod, PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG Status = BRDerr_OK;

	memcpy(EepromSubmod + Offset, pBuffer, BufferSize);

    int sub_offset = 0x400;
    int sub_size = 256;
    int res = m_blk_fid->WriteFmcEeprom(submod, 0x50, (uint8_t*)EepromSubmod, sub_offset, sub_size);
	if (res < 0) {
		fprintf(stderr, "%s(): Error in WriteEEPROM()\n", __FUNCTION__);
		return ~0x0;
	}

	//if (m_blk_fid->is_valid())
	//{
	//	memcpy(EepromSubmod + Offset, pBuffer, BufferSize);
	//	if(submod)
	//	{
	//		Status = WriteFidSpd(2, 0x50, EepromSubmod, 0x400, 256); // FMC2
	//		//Status = ReadFidSpd(2, 0x50, EepromSubmod, 0x400, 256); // FMC2
	//	}
	//	else
	//		Status = WriteFidSpd(0, 0x50, EepromSubmod, 0x400, 256); // FMC1
	//}
	return Status;
}

//static UCHAR FruidEeprom[1024];

//-----------------------------------------------------------------------------

ULONG CAxizynq::ReadFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	return BRDerr_CMD_UNSUPPORTED;
//	memset(FruidEeprom, 0xff, 256);
//	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::WriteFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	return BRDerr_CMD_UNSUPPORTED;
//	if(!BufferSize)
//		return BRDerr_OK;
//	memcpy(FruidEeprom + Offset, pBuffer, BufferSize);
//	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetLocation(PAMB_LOCATION pAmbLocation)
{
    pAmbLocation->BusNumber = -1;
    pAmbLocation->DeviceNumber = -1;
    pAmbLocation->SlotNumber = -1;
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetDeviceID(USHORT& DeviceID)
{
    DeviceID = FMC130E_DEVID;
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetRevisionID(UCHAR& RevisionID)
{
    RevisionID = 0x1;
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::WriteRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
{
	int idx = TetrNum;
	fmcx_trd_t trd = get_trd(m_dev, idx);
	trd->wi(RegNum, RegVal);

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::WriteRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
{
	int idx = TetrNum;
	fmcx_trd_t trd = get_trd(m_dev, idx);
	trd->wd(RegNum, RegVal);

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::ReadRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
{
	int idx = TetrNum;
	fmcx_trd_t trd = get_trd(m_dev, idx);
	RegVal = trd->ri(RegNum);

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::ReadRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
{
	int idx = TetrNum;
	fmcx_trd_t trd = get_trd(m_dev, idx);
	RegVal = trd->rd(RegNum);
	
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::WriteRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
	return BRDerr_CMD_UNSUPPORTED;
//	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::WriteRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
	int idx = TetrNum;
	fmcx_trd_t trd = get_trd(m_dev, idx);

	uint32_t val = 0;
	uint32_t* buf = (uint32_t*)RegBuf;
	int size = RegBufSize / sizeof(uint32_t);
	for (int i = 0; i < size; i++)
	{
		val = buf[i];
		trd->wd(RegNum, val);
	}

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::ReadRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
	return BRDerr_CMD_UNSUPPORTED;
//	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::ReadRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
	int idx = TetrNum;
	fmcx_trd_t trd = get_trd(m_dev, idx);

	uint32_t val = 0;
	uint32_t* buf = (uint32_t*)RegBuf;
	int size = RegBufSize / sizeof(uint32_t);
	for (int i = 0; i < size; i++)
	{
		val = trd->rd(RegNum);
		buf[i] = val;
	}

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::SetAxiSwitch(unsigned val_id)
{
	// Работа с AXI-интреконнектом
	axi_switch_t axisw = get_axi_switch(m_dev, 0);
	if (!axisw->is_valid()) {
		fprintf(stderr, "AXI SWITCH: AXI interconnect not found!\n");
		return -1;
	}
	else {
		fprintf(stderr, "MIMUX[0]    - 0x%X\n", axisw->rd(mi_mux_reg));
		fprintf(stderr, "MIMUX[1]    - 0x%X\n", axisw->rd(mi_mux_reg + 4));
	}

	if (val_id == 0xE5) // DDR3
	{// DMA -> DDR3
		fprintf(stderr, "DMA -> DDR3\n");
		axisw->wd(mi_mux_reg, 0x80000000);
		axisw->wd(mi_mux_reg + 4, 0x0);
	}
	else
	{// DMA -> DAC FIFO
		fprintf(stderr, "DMA -> DAC FIFO\n");
		axisw->wd(mi_mux_reg, 0x0);
		axisw->wd(mi_mux_reg + 4, 0x80000000);
	}
	axisw->wd(ctrl_reg, 0x2);

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::SetMemory(PVOID pParam, ULONG sizeParam)
{
	AMB_MEM_DMA_CHANNEL *pMemDescr = (AMB_MEM_DMA_CHANNEL *)pParam;

	m_dma_tetr_num[pMemDescr->DmaChanNum] = pMemDescr->LocalAddr;
	fmcx_trd_t trd = get_trd(m_dev, m_dma_tetr_num[pMemDescr->DmaChanNum]);
	unsigned val_id = trd->ri(0x100); // ADM2IFnr_ID

	SetAxiSwitch(val_id);

	dma_memory_type mem_type;
	if (pMemDescr->MemType == 1)
		mem_type = dma_memory_type::KERNEL_MEMORY_DMA;
	else
		if (pMemDescr->MemType == 0 || pMemDescr->MemType == 2)
			mem_type = dma_memory_type::USER_MEMORY_DMA;
		else
			return BRDerr_BAD_PARAMETER;

	unsigned block_count = pMemDescr->BlockCnt;
	unsigned block_size = pMemDescr->BlockSize;

	if (!m_dma[pMemDescr->DmaChanNum]->dma_prepare(block_count, block_size, mem_type)) {
		fprintf(stderr, "%s, %d: %s() - Error in dma_prepare().\n", __FILE__, __LINE__, __FUNCTION__);
		return ~0x0;
	}

    pMemDescr->pStub = m_dma[pMemDescr->DmaChanNum]->dma_stub();

	std::vector<void*> pBlocks = m_dma[pMemDescr->DmaChanNum]->dma_blocks();
	for (unsigned iBlock = 0; iBlock < block_count; iBlock++)
	{
		pMemDescr->pBlock[iBlock] = pBlocks.at(iBlock);
	}

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::FreeMemory(PVOID pParam, ULONG sizeParam)
{
	// нужно только для linux (под windows это функции-заглушки)
    AMB_MEM_DMA_CHANNEL *pMemDescr = (AMB_MEM_DMA_CHANNEL *)pParam;

	m_dma[pMemDescr->DmaChanNum]->dma_free();
	
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::StartMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	AMB_START_DMA_CHANNEL* st_dma = (AMB_START_DMA_CHANNEL*)pParam;

	fmcx_trd_t trd = get_trd(m_dev, m_dma_tetr_num[st_dma->DmaChanNum]);
	unsigned val = trd->ri(0); // MODE0
	val |= 0x08;
	trd->wi(0, val);

    //fprintf(stderr, "MODE0 = 0x%X\n", val);

	m_dma[st_dma->DmaChanNum]->dma_start((bool)st_dma->IsCycling);
	//m_dma[st_dma->DmaChanNum]->dma_start(true);

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::StopMemory(PVOID pParam, ULONG sizeParam)
{
	AMB_STATE_DMA_CHANNEL* state_dma = (AMB_STATE_DMA_CHANNEL*)pParam;
	//state_dma->Timeout
	m_dma[state_dma->DmaChanNum]->dma_stop();

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::StateMemory(PVOID pParam, ULONG sizeParam)
{
	AMB_STATE_DMA_CHANNEL* state_dma = (AMB_STATE_DMA_CHANNEL*)pParam;

	struct dma_state_t state;
	//bool ret = m_dma[state_dma->DmaChanNum]->dma_state(state);
	m_dma[state_dma->DmaChanNum]->dma_state(state);

	state_dma->BlockNum = state.completed_block_counter;
	state_dma->BlockCntTotal = state.completed_desc_counter;
	state_dma->DmaChanState = state.status;
	state_dma->OffsetInBlock = 0;

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::SetDirection(PVOID pParam, ULONG sizeParam)
{
	return BRDerr_CMD_UNSUPPORTED;
	//return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::SetSource(PVOID pParam, ULONG sizeParam)
{
	AMB_SET_DMA_CHANNEL* set_dma = (AMB_SET_DMA_CHANNEL*)pParam;;

	m_dma_tetr_num[set_dma->DmaChanNum] = set_dma->Param;
	fmcx_trd_t trd = get_trd(m_dev, m_dma_tetr_num[set_dma->DmaChanNum]);
	unsigned val_id = trd->ri(0x100); // ADM2IFnr_ID

	SetAxiSwitch(val_id);

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::ResetFifo(PVOID pParam, ULONG sizeParam)
{
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------
//BRDstrm_DIR_IN = 0x1,				// To HOST
//BRDstrm_DIR_OUT = 0x2,				// From HOST
//BRDstrm_DIR_INOUT = 0x3				// Both Directions

//typedef struct _AMB_GET_DMA_INFO {
//	ULONG	DmaChanNum;		// IN
//	ULONG	Direction;		// OUT
//	ULONG	FifoSize;		// OUT
//	ULONG	MaxDmaSize;		// OUT
//							//	ULONG	PciAddr;		// OUT
//							//	ULONG	LocalAddr;		// OUT
//} AMB_GET_DMA_INFO, *PAMB_GET_DMA_INFO;

ULONG CAxizynq::GetDmaChanInfo(PVOID pParam, ULONG sizeParam)
{
	AMB_GET_DMA_INFO* dma_info = (AMB_GET_DMA_INFO*)pParam;
	if (dma_info->DmaChanNum == 0)
		dma_info->Direction = BRDstrm_DIR_OUT;
	else
		if (dma_info->DmaChanNum == 1)
			dma_info->Direction = BRDstrm_DIR_IN;
		else
			return BRDerr_BAD_PARAMETER;
	return BRDerr_OK;
}

// ***************************************************************************************
ULONG CAxizynq::WaitDmaBlock(void* pParam, ULONG sizeParam)
{
	AMB_STATE_DMA_CHANNEL* state_dma = (AMB_STATE_DMA_CHANNEL*)pParam;
	int msTimeout = state_dma->Timeout;
	m_dma[state_dma->DmaChanNum]->dma_wait_block(msTimeout);

    return BRDerr_OK;
}

// ***************************************************************************************
ULONG CAxizynq::WaitDmaBuffer(void* pParam, ULONG sizeParam)
{
	AMB_STATE_DMA_CHANNEL* state_dma = (AMB_STATE_DMA_CHANNEL*)pParam;
	int msTimeout = state_dma->Timeout;
	m_dma[state_dma->DmaChanNum]->dma_wait_buffer(msTimeout);
	
    return BRDerr_OK;
}
