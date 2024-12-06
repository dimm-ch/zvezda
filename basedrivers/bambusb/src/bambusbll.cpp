// clang-format off
#include "bambusb.h"
#ifdef _WINDOWS
#include <windows.h>
#include "process.h"
#endif

#define	CURRFILE "BAMBUSBLL"

typedef volatile struct _ADM2IF_TETRAD {
	USHORT	STATUS;		// (0x00) Status register
	USHORT	DATA;		// (0x02) Data register
	USHORT	CMD_ADR;	// (0x04) Address command register
	USHORT	CMD_DATA;	// (0x06) Data command register
} ADM2IF_TETRAD, *PADM2IF_TETRAD;

typedef union _MAIN_SELX {
	U32 AsWhole; // Board Mode Register as a Whole Word
	struct { // Mode Register as Bit Pattern
		ULONG	IrqNum	: 4, // Interrupt number
				Res1	: 4, // Reserved
				DmaTetr	: 4, // Tetrad number for DMA channel X
				DrqEnbl	: 1, // DMA request enable
				DmaMode	: 3; // DMA mode
	} ByBits;
} MAIN_SELX, *PMAIN_SELX;

ULONG CBambusb::LoadPld(UCHAR& PldStatus, PVOID pBuffer, ULONG BufferSize)
{
	PldStatus = 1;
	return BRDerr_OK;
}

ULONG CBambusb::GetPldStatus(ULONG& PldStatus, ULONG PldNum)
{
	PldStatus = 1;
	return BRDerr_OK;
}

ULONG CBambusb::WaitSemaReady(int SemaOrReady)
{
	return BRDerr_CMD_UNSUPPORTED;
}

ULONG CBambusb::EepromWordWrite(int idRom, U32 addr, U32 data)
{
	return BRDerr_CMD_UNSUPPORTED;
}

U32 CBambusb::EepromWordRead(int idRom, U32 addr)
{
	return BRDerr_CMD_UNSUPPORTED;
}

ULONG CBambusb::EepromBufReadFromBase(PUCHAR buf, U32 Offset, LONG Length)
{
	return BRDerr_CMD_UNSUPPORTED;
}

ULONG CBambusb::EepromBufWriteIntoBase(PUCHAR buf, U32 Offset, LONG Length)
{
	return BRDerr_CMD_UNSUPPORTED;
}

ULONG CBambusb::EepromBufReadFromAdm(PUSHORT buf, U32 Offset, LONG Length)
{
	return BRDerr_CMD_UNSUPPORTED;
}

ULONG CBambusb::EepromBufWriteIntoAdm(PUSHORT buf, U32 Offset, LONG Length)
{
	return BRDerr_CMD_UNSUPPORTED;
}

UCHAR CBambusb::FlashGetVendorID()
{
	return BRDerr_CMD_UNSUPPORTED;
}

UCHAR CBambusb::FlashGetDeviceID()
{
	return BRDerr_CMD_UNSUPPORTED;
}

U32 CBambusb::FlashGetStatus(U32 addr)
{
	return BRDerr_CMD_UNSUPPORTED;
}

ULONG CBambusb::FlashWordProg(U32 addr, U32 data)
{
	return BRDerr_CMD_UNSUPPORTED;
}

ULONG CBambusb::GetICR(PVOID pBuffer, U08 bICRType)
{
	U08 addrReg, dataReg, ctrlReg;
	int start[2] = {0}, stop[2] = {0}, commNum;
	LONG nLen;
	U32 lVal = 400000, hVal = (1 << 28);
	ULONG Status = 0;
	//захват I2C
	Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
	while (1)
	{
		Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 0);
		if (lVal == 400000)
			break;
		else
		{
			lVal = 400000;
			Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
		}
	}
	//

	rUsbHostCommand arCommands[128] = {0}, arReplays[128] = {0};//массивы команд и ответов
	for (int i = 0; i < 128; i++)
	{
		m_Trans++;
		arCommands[i].shortTransaction = m_Trans;//все команды обращаются к блоку FID
		arCommands[i].signatureValue = 0xA5;
		arCommands[i].taskID = m_TaskID;
		arCommands[i].command.hardware.type = 1;
		arCommands[i].command.hardware.port = 2;
	}
	commNum = 128;//количество команд для отправки
	ctrlReg = 0x20 + 0x16;//SPD_CTRL
	addrReg = 0x20 + 0x17;//SPD_ADDR
	dataReg = 0x20 + 0x18;//SPD_DATAL
	for (int i = 0; i < 50; i++)
		arCommands[i].valueH = ctrlReg;//ожидание бита READY
	stop[0] = 50;//последняя команда ожидания READY
	arCommands[50].valueH = 0x20 + 0x15; //FID + SPD_DEVICE
	if (bICRType == 0)
		arCommands[50].valueL = 7;//base ICR
	arCommands[50].command.hardware.write = 1;
	arCommands[51].valueL = 0;//начальный адрес 0
//	if (bICRType == 1)
//		arCommands[51].valueL = 0;//начальный адрес 1024
	arCommands[51].valueH = addrReg;
	arCommands[51].command.hardware.write = 1;
	if ((m_pDevice->ProductID == 0x3028)/*||(m_pDevice->ProductID == 0x3029)*/)
		arCommands[52].valueL = 1 + (0x53 << 4); //признак чтения + адрес ПЗУ
	else
		arCommands[52].valueL = 1 + (0x50 << 4); //признак чтения + адрес ПЗУ
	if (bICRType == 1)
		arCommands[52].valueL = 1 + (0x54 << 4); //признак чтения + адрес ПЗУ 0x50 + 256*4
	arCommands[52].valueH = ctrlReg;
	arCommands[52].command.hardware.write = 1;
	for (int i = 53; i < 127; i++)
		arCommands[i].valueH = ctrlReg;//ожидание бита READY
	stop[1] = 127;//последняя команда ожидания READY
	arCommands[127].valueH = dataReg;
	nLen = commNum * 16;//длина пачки команд
	if (SendBuffer(arCommands, arReplays, nLen) != BRDerr_OK)
	{
		Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);//освобождение I2C
		return BRDerr_ERROR;
	}
	int j = 0;
	for (int i = 0; i < commNum; i++)
	{
		if (arReplays[i].command.hardware.type != arCommands[i].command.hardware.type)
		{
			Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
			return BRDerr_ERROR;
		}
		if (arReplays[i].transaction != arCommands[i].transaction)
		{
			Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
			return BRDerr_ERROR;
		}
		if (arReplays[i].command.hardware.write == 0)//определение количества команд ожидания READY (сколько команд до установки бита)
			if (((arReplays[i].valueL & 0x8000) != 0) && (j < 2))
			{
				start[j] = i + 2;
				i = stop[j] - 1;
				j++;
			}
	}
	((U08*)pBuffer)[0] = arReplays[commNum - 1].valueL;
	//удаление лишних команд ожидания из пачки
	for (int i = start[1]; i < commNum; i++)
		arCommands[i] = arCommands[i+(stop[1] - start[1])];
	commNum -= (stop[1] - start[1]);
	for (int i = start[0]; i < commNum; i++)
		arCommands[i] = arCommands[i+(stop[0] - start[0])];
	commNum -= (stop[0] - start[0]);
	for (int i = 0; i < commNum; i++)
		if (arCommands[i].valueH == addrReg)
			stop[0] = i;
	if ((start[0] == 0)||(start[1] == 0))
	{
		Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
		return BRDerr_HW_BUSY;
	}
	if (commNum <= 0)
	{
		Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
		return BRDerr_HW_BUSY;
	}
	//чтение оставшихся байт ICR
	int maxByte = 511;
	if (bICRType == 2)
		maxByte = 1023;
	int nBytes = 128 / commNum;
	int nAddr = arCommands[stop[0]+1].valueL;
	for (int i = 1; i < nBytes; i++)
	{
		for (int j = 0; j < commNum; j++)
			arCommands[i*commNum+j] = arCommands[j];
	}
	for (int i = 0; i < maxByte; i += nBytes)
	{
		for (int j = 1; j <= nBytes; j++)
		{
			if (i == 0)
				arCommands[stop[0] + (j-1)*commNum].valueL += (i+j);//адрес++
			else
				arCommands[stop[0] + (j-1)*commNum].valueL += nBytes;//адрес++
			if (i+j-1 >= maxByte)
				break;
			arCommands[stop[0]+(j-1)*commNum+1].valueL = nAddr;
			if ((i+j-1 == 255)||(i+j-1 == 511)||(i+j-1 == 767))
				nAddr += (1 << 4);
		}
		nLen = nBytes*commNum*16;
		if (SendBuffer(arCommands, arReplays, nLen) != BRDerr_OK)
		{
			Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
			return BRDerr_ERROR;
		}
		for (int j = 1; j <= nBytes; j++)
			if (i + j < maxByte)
			{
				((U08*)pBuffer)[i + j] = arReplays[commNum*j - 1].valueL;
			}
	}
	Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);//освобождение I2C
	return BRDerr_OK;
}

ULONG CBambusb::ReadNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{//чтение ICR базового модуля
	//GetICR(m_baseICR, 0);
	memcpy(pBuffer, m_baseICR + Offset, BufferSize);
	return BRDerr_OK;
}

ULONG CBambusb::GetNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{//чтение ICR базового модуля
	memcpy(pBuffer, m_baseICR + Offset, BufferSize);
	return BRDerr_OK;
}

ULONG CBambusb::WriteNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{//запись в ICR базового модуля
	U32 hVal, lVal = 400000, bufIndex = 0, isBreak = 0;
	hVal = (1 << 28);
	ULONG Status = 0;
	//захват I2C
	Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
	while (1)
	{
		Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 0);
		if (lVal == 400000)
			break;
		else
		{
			lVal = 400000;
			Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
		}
	}
	//
	for (unsigned int i = Offset; i < BufferSize + Offset; i++)
	{
		while (1)
		{
			hVal = 0x20 + 0x016;//FID + SPD_CTRL
			lVal = 0;
			SendCommand(0, 0, hVal, lVal, 1, 2, 0, 0);//read SPD_CTRL
			if (((lVal >> 15) & 1) == 1) //ready
				break;
			isBreak++;
			if (isBreak > 10)
			{
				Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);//освобождение I2C
				return BRDerr_ERROR;
			}
		}
		hVal = 0x20 /* FID */ + 0x15;
		lVal = 7;
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//SPD_DEVICE = 7;
		hVal = 0x20 + 0x17;//FID + SPD_ADDR
		lVal = i;
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//SPD_ADDR
		hVal = 0x20 + 0x18;//FID + DATAL
		lVal = 0;
		memcpy(&lVal, (PUCHAR)pBuffer + bufIndex, 4);
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//DATAL
		hVal = 0x20 + 0x16;//FID + SPD_CTRL
		if ((m_pDevice->ProductID == 0x3028)/*||(m_pDevice->ProductID == 0x3029)*/)
		{
			if (i >= 256)
				lVal = 2 + (0x54 << 4);
			else
				lVal = 2 + (0x53 << 4);
		}
		else
		{
			if (i >= 256)
				lVal = 2 + (0x51 << 4);
			else
				lVal = 2 + (0x50 << 4);
		}
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//SPD_CTRL
		isBreak = 0;
		while (1)
		{
			hVal = 0x20 + 0x016;//FID + SPD_CTRL
			lVal = 0;
			SendCommand(0, 0, hVal, lVal, 1, 2, 0, 0);//read SPD_CTRL
			if (((lVal >> 15) & 1) == 1) //ready
				break;
			isBreak++;
			if (isBreak > 10)
			{
				Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);//освобождение I2C
				return BRDerr_ERROR;
			}
		}
		hVal = 0x20 + 0x16;//FID + SPD_CTRL
		lVal = 0;
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//SPD_CTRL
		bufIndex++;
		IPC_delay(10);
	}
	memcpy(m_baseICR + Offset, pBuffer, BufferSize);//дублируем в локальную копию
	Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);//освобождение I2C
	return BRDerr_OK;
}

ULONG CBambusb::ReadAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{//чтение ICR субмодуля
	memcpy(pBuffer, m_subICR + Offset, BufferSize);
	return BRDerr_OK;
}

ULONG CBambusb::WriteAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{//запись ICR субмодуля
	ULONG status = 0;
	status = WriteSubICR(0, pBuffer, BufferSize, Offset);
	return status;
}

ULONG CBambusb::ReadFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	U32 status = 0, copySize;
	status = GetICR(m_FRUID, 2);
	if (BufferSize > (1024 - Offset))
		copySize = 1024 - Offset;
	else
		copySize = BufferSize;
	memcpy(pBuffer, m_FRUID + Offset, copySize);
	return status;
}

ULONG CBambusb::WriteFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	U32 hVal, lVal=400000, bufIndex = 0, isBreak = 0;
	hVal = (1 << 28);
	ULONG Status = 0;
	//захват I2C
	Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
	while (1)
	{
		Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 0);
		if (lVal == 400000)
			break;
		else
		{
			lVal = 400000;
			Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
		}
	}
	//
	for (unsigned int i = Offset; i < BufferSize + Offset; i++)
	{
		while (1)
		{
			hVal = 0x20 + 0x016;//FID + SPD_CTRL
			lVal = 0;
			SendCommand(0, 0, hVal, lVal, 1, 2, 0, 0);//read SPD_CTRL
			if (((lVal >> 15) & 1) == 1) //ready
				break;
			isBreak++;
			if (isBreak > 10)
			{
				Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);//освобождение I2C
				return BRDerr_ERROR;
			}
		}
		hVal = 0x20 /* FID */ + 0x15;
		lVal = 0;
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//SPD_DEVICE = 0;
		hVal = 0x20 + 0x17;//FID + SPD_ADDR
		lVal = i;
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//SPD_ADDR
		hVal = 0x20 + 0x18;//FID + DATAL
		lVal = 0;
		memcpy(&lVal, (PUCHAR)pBuffer + bufIndex, 4);
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//DATAL
		hVal = 0x20 + 0x16;//FID + SPD_CTRL
		if (i >= 256)
			lVal = 2 + (0x51 << 4);
		else
			lVal = 2 + (0x50 << 4);
		if (i >= 512)
			lVal = 2 + (0x52 << 4);
		if (i >= 768)
			lVal = 2 + (0x53 << 4);
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//SPD_CTRL
		isBreak = 0;
		while (1)
		{
			hVal = 0x20 + 0x016;//FID + SPD_CTRL
			lVal = 0;
			SendCommand(0, 0, hVal, lVal, 1, 2, 0, 0);//read SPD_CTRL
			if (((lVal >> 15) & 1) == 1) //ready
				break;
			isBreak++;
			if (isBreak > 10)
			{
				Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);//освобождение I2C
				return BRDerr_ERROR;
			}
		}
		hVal = 0x20 + 0x16;//FID + SPD_CTRL
		lVal = 0;
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//SPD_CTRL
		bufIndex++;
		IPC_delay(10);
	}
	memcpy(m_FRUID + Offset, pBuffer, BufferSize);//дублируем в локальную копию
	Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);//освобождение I2C
	return BRDerr_OK;
}

ULONG CBambusb::FormCommand(ULONG AdmNum, ULONG Param, ULONG HValue, U32& LValue, U08 bType, U08 Port, U08 bOpCode, bool isWrite, PVOID pBuf)
{
	rUsbHostCommand rComm = {0};
	rComm.signatureValue = 0xA5;
	rComm.taskID = m_TaskID;
	rComm.valueH = HValue;
	rComm.valueL = LValue;
	rComm.shortTransaction = m_Trans;
	m_Trans++;
	switch(bType)
	{
	case 0://service commands
		rComm.command.system.type = bType;
		rComm.command.system.condition = bOpCode;
		rComm.command.system.parametr = Param;
		break;
	case 1://block main and direct registers
		rComm.command.hardware.type = bType;
		rComm.command.hardware.port = Port;
		rComm.command.hardware.write = isWrite;
		rComm.command.hardware.number = Param;
		break;
	case 2://indirect registers
		rComm.command.tetrad.type = bType;
		rComm.command.tetrad.number = Param;
		rComm.command.tetrad.condition = isWrite * 4;
		break;
	case 3://stream control
		rComm.command.stream.type = 3;
		rComm.command.stream.spar = 0;
		rComm.command.stream.name = (Param >> 5) & 7;
		rComm.command.stream.reference = Param & 0x1F;
		rComm.command.stream.condition = bOpCode;
		break;
	default:
		break;
	}
	memcpy(pBuf, (PVOID)&rComm, 16);
	return BRDerr_OK;
}

ULONG CBambusb::CheckCommand(PVOID pOutBuf, PVOID pInBuf)
{
	rUsbHostCommand rCommOut, rCommIn;
	memcpy(&rCommOut, pOutBuf, 16);
	memcpy(&rCommIn, pInBuf, 16);
	if (rCommIn.versionValue != 0)
		return 1;
	if (rCommIn.signatureValue != 0xB5)
		return 2;
	if ((rCommIn.taskID != rCommOut.taskID) && (rCommOut.taskID != 0))
		return 5;
	if (rCommIn.shortTransaction > rCommOut.shortTransaction)
		return 3;
	if (rCommIn.shortTransaction < rCommOut.shortTransaction)
		return 6;
	if (rCommIn.command.tetrad.type != rCommOut.command.tetrad.type)
		return (10 + rCommIn.command.error.code);
	return BRDerr_OK;
}

ULONG CBambusb::SendCommand(ULONG AdmNum, ULONG Param, ULONG HValue, U32& LValue, U08 bType, U08 Port, U08 bOpCode, bool isWrite)
{
	UCHAR bBuf[16] = {0}, bBufR[16] = {0}, bPacket[16] = {0};
	ULONG Status = 0;
	PUCHAR pRes, pResR;
	OVERLAPPED rOvLap, rOvLapR;
	IPC_TIMEVAL time1, time2;
	FILE *pFile;
#ifdef __IPC_WIN__
	LARGE_INTEGER freq, start, finish, delta;
	PVOID tmp_ptr = &freq;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);
#endif
	LONG nLen = 16;
	bool isRes = 0;
	int errCnt = 0;
	int bufOffset = 0;
	BRDCHAR commEvent[128]={0}, replayEvent[128]={0};
	{
		BRDCHAR pStr[128] = {0};
		BRDC_strcat(commEvent, _BRDC("Event_Commands_"));
		BRDC_strcat(commEvent, BRDC_itoa(m_TaskID, pStr, 10));
		BRDC_strcat(commEvent, _BRDC("_"));
		BRDC_strcat(commEvent, BRDC_itoa(m_PID, pStr, 10));
		BRDC_strcat(replayEvent, _BRDC("Event_Replays_"));
		BRDC_strcat(replayEvent, BRDC_itoa(m_TaskID, pStr, 10));
		BRDC_strcat(replayEvent, _BRDC("_"));
		BRDC_strcat(replayEvent, BRDC_itoa(m_PID, pStr, 10));
	}
	IPC_handle hComm, hReplay;

	if ((m_TaskID != 0) && ((DWORD)(IPC_getTickCount()) - m_LastComTime >= 61000))
	{
		m_TaskID = 0;
		U32 lVal = 0;
		SendCommand(0, 0, 0, lVal, 0, 0, 1, 1);
	}

	//формируем команду
	FormCommand(AdmNum, Param, HValue, LValue, bType, Port, bOpCode, isWrite, (PVOID)bPacket);

	memcpy(bBuf, bPacket, nLen);
	errCnt = 0;
	if (m_isLog == 1)
	{
		for (int i=0; i<15; i++)
		{
			pFile = fopen("commands.txt", "a");
			if (pFile != 0)
				break;
		}
		if (pFile != 0)
		{
			fprintf(pFile, "\ntry capture mutex");
			fprintf(pFile, "\t");
			fprintf(pFile, "%d\n", m_PID);
			fclose(pFile);
		}
	}
	while (1)
	{
		if (IPC_captureMutex(m_hCommand, 10) == 0)//ждём пока отправится другая команда
			break;
		errCnt++;
		if (errCnt >= 6000)
			return BRDerr_INSUFFICIENT_RESOURCES;
	}
#ifdef __IPC_WIN__
	QueryPerformanceCounter(&finish);
	delta.QuadPart = (finish.QuadPart - start.QuadPart) * 1000000 / freq.QuadPart;
	if (m_isLog)
	{
		for (int i = 0; i<15; i++)
		{
			pFile = fopen("commands.txt", "a");
			if (pFile != 0)
				break;
		}
		if (pFile != 0)
		{
			fprintf(pFile, "mutex captured in %lld us\n", delta.QuadPart);
			fclose(pFile);
		}
	}
#endif
	nLen = 16;
	DWORD dTimeStamp = IPC_getTickCount();
	IPC_getTime(&time1);
	if (m_isLog == 1)
	{
		for (int i=0; i<15; i++)
		{
			pFile = fopen("commands.txt", "a");
			if (pFile != 0)
				break;
		}
		if (pFile != 0)
		{
			for (int i = 0; i < 16; i++)
				fprintf(pFile, "%02X", bPacket[15-i]);
			fprintf(pFile, "\t");
			fclose(pFile);
		}
	}
#ifdef __IPC_WIN__
	QueryPerformanceCounter(&start);
#endif
	/*if ((m_pDevice->EndPoints[m_EPin]->UsbdStatus != 0)||(m_pDevice->EndPoints[m_EPin]->NtStatus != 0))
	{
		printf("ep error");
		for (int i=0; i<15; i++)
		{
			pFile = fopen("commands.txt", "a");
			if (pFile != 0)
				break;
		}
		if (pFile != 0)
		{
			for (int i = 0; i < 16; i++)
				fprintf(pFile, "%02X", bPacket[15-i]);
			fprintf(pFile, "\t");
			fclose(pFile);
		}
		return BRDerr_FATAL;
	}*/
	m_LastComTime = IPC_getTickCount();
	for (int i = 0; i < 100; i++)
	{
		hReplay = IPC_createEvent(replayEvent, false, false);
		if (hReplay != 0)
			break;
	}
	rOvLapR.hEvent = (HANDLE)IPC_getDescriptor(hReplay);
	//CreateEvent(NULL, false, false, NULL);
	pResR = m_pDevice->EndPoints[m_EPin]->BeginDataXfer(bBufR, nLen, &rOvLapR);
	if ((m_pDevice->EndPoints[m_EPin]->UsbdStatus != 0)||(m_pDevice->EndPoints[m_EPin]->NtStatus != 0))
	{
		if (m_isLog == 1)
		{
			for (int i=0; i<15; i++)
			{
				pFile = fopen("commands.txt", "a");
				if (pFile != 0)
					break;
			}
			if (pFile != 0)
			{
				fprintf(pFile, "endpoint error 0x%X 0x%X\t", m_pDevice->EndPoints[m_EPin]->UsbdStatus, m_pDevice->EndPoints[m_EPin]->NtStatus);
				IPC_getTime(&time2);
				dTimeStamp = IPC_getDiffTime(&time1, &time2);
				fprintf(pFile, "%u\t", dTimeStamp);
				fprintf(pFile, "%d\n", m_PID);
				fclose(pFile);
			}
		}
		isRes = false;
	}
	//else
	{
		while (1)
		{
			for (int i = 0; i< 100; i++)
			{
				hComm = IPC_createEvent(commEvent, false, false);
				if (hComm != 0)
					break;
			}
			rOvLap.hEvent = (HANDLE)IPC_getDescriptor(hComm);//CreateEvent(NULL, false, false, NULL);
			//isRes = m_pDevice->EndPoints[1]->XferData(bBuf+bufOffset, nLen);
			pRes = m_pDevice->EndPoints[1]->BeginDataXfer(bBuf + bufOffset, nLen, &rOvLap);
			errCnt = 0;
			while (errCnt < 10)
			{
				if (m_pDevice->EndPoints[1]->WaitForXfer(&rOvLap, 100) == true)
					break;
				errCnt++;
			}
			isRes = m_pDevice->EndPoints[1]->FinishDataXfer(bBuf + bufOffset, nLen, &rOvLap, pRes);
			IPC_deleteEvent(hComm);
			if (!isRes)
				break;
			if (nLen + bufOffset < 16)
			{
				bufOffset += nLen;
				nLen = 16 - bufOffset;
			}
			else
				break;
		}
	}
	nLen = 16;
	if (isRes)
	{
		if (nLen != 16)
		{
			IPC_releaseMutex(m_hCommand);
			if (m_isLog == 1)
			{
				for (int i=0; i<15; i++)
				{
					pFile = fopen("commands.txt", "a");
					if (pFile != 0)
						break;
				}
				if (pFile != 0)
				{
					fprintf(pFile, "release mutex");
					fprintf(pFile, "\t");
					fprintf(pFile, "%d\n", m_PID);
					fclose(pFile);
				}
			}
			return BRDerr_ERROR;
		}
		errCnt = 0;
		while (errCnt < 10)
		{
			if (m_pDevice->EndPoints[m_EPin]->WaitForXfer(&rOvLapR, 100) == true)
				break;
			errCnt++;
		}
		isRes = m_pDevice->EndPoints[m_EPin]->FinishDataXfer(bBufR, nLen, &rOvLapR, pResR);
		IPC_deleteEvent(hReplay);
		if (isRes && (nLen < 16))
			while (1)
			{
				bufOffset = 16 - nLen;
				nLen = 16 - bufOffset;
				for (int i = 0; i < 100; i++)
				{
					hReplay = IPC_createEvent(replayEvent, false, false);
					if (hReplay != 0)
						break;
				}
				rOvLapR.hEvent = (HANDLE)IPC_getDescriptor(hReplay);
				//CreateEvent(NULL, false, false, NULL);
				pResR = m_pDevice->EndPoints[m_EPin]->BeginDataXfer(bBufR + bufOffset, nLen, &rOvLapR);
				errCnt = 0;
				while (errCnt < 10)
				{
					if (m_pDevice->EndPoints[m_EPin]->WaitForXfer(&rOvLapR, 100) == true)
						break;
					errCnt++;
				}
				isRes = m_pDevice->EndPoints[m_EPin]->FinishDataXfer(bBufR + bufOffset, nLen, &rOvLapR, pResR);
				IPC_deleteEvent(hReplay);
				if (!isRes)
					break;
				if (nLen + bufOffset == 16)
				{
					Status = BRDerr_OK;
					break;
				}
			}
		if (isRes)
		{
			Status = CheckCommand(bBuf, bBufR);
#ifdef __IPC_WIN__
			QueryPerformanceCounter(&finish);
			delta.QuadPart = (finish.QuadPart - start.QuadPart) * 1000000 / freq.QuadPart;
			if (m_isLog)
			{
				for (int i = 0; i<15; i++)
				{
					pFile = fopen("commands.txt", "a");
					if (pFile != 0)
						break;
				}
				if (pFile != 0)
				{
					fprintf(pFile, "\nanswer received in %lld us\n", delta.QuadPart);
					fclose(pFile);
				}
			}
#endif
			if (m_isLog == 1)
			{
				for (int i=0; i<15; i++)
				{
					pFile = fopen("commands.txt", "a");
					if (pFile != 0)
						break;
				}
				if (pFile != 0)
				{
					for (int i = 0; i < 16; i++)
						fprintf(pFile, "%02X", bBufR[15-i]);
					IPC_getTime(&time2);
					dTimeStamp = IPC_getDiffTime(&time1, &time2);
					fprintf(pFile, "\t%u\t", dTimeStamp);
					fprintf(pFile, "%d\n", m_PID);
					fclose(pFile);
				}
			}
#ifdef __IPC_WIN__
			QueryPerformanceCounter(&start);
#endif
			if (nLen != 16)
			{
				IPC_releaseMutex(m_hCommand);
				if (m_isLog == 1)
				{
					for (int i=0; i<15; i++)
					{
						pFile = fopen("commands.txt", "a");
						if (pFile != 0)
							break;
					}
					if (pFile != 0)
					{
						fprintf(pFile, "release mutex");
						fprintf(pFile, "\t");
						fprintf(pFile, "%d\n", m_PID);
						fclose(pFile);
					}
				}
				return BRDerr_FATAL;
			}
			if (Status > 4)
			{//обработка ошибок
				if (Status == 5)//task ID
				{
					Status = BRDerr_ERROR;
					int getTaskId = 15;
					while (getTaskId != 0)
					{
						for (int i = 0; i < 100; i++)
						{
							hReplay = IPC_createEvent(replayEvent, false, false);
							if (hReplay != 0)
								break;
						}
						rOvLapR.hEvent = (HANDLE)IPC_getDescriptor(hReplay);
						//CreateEvent(NULL, false, false, NULL);
						pResR = m_pDevice->EndPoints[m_EPin]->BeginDataXfer(bBufR, nLen, &rOvLapR);
						while (errCnt < 10)
						{
							if (m_pDevice->EndPoints[m_EPin]->WaitForXfer(&rOvLapR, 100) == true)
								break;
							errCnt++;
						}
						isRes = m_pDevice->EndPoints[m_EPin]->FinishDataXfer(bBufR, nLen, &rOvLapR, pResR);
						IPC_deleteEvent(hReplay);
						if (isRes)
						{
							getTaskId--;
							if (m_isLog == 1)
							{
								for (int i=0; i<15; i++)
								{
									pFile = fopen("commands.txt", "a");
									if (pFile != 0)
										break;
								}
								if (pFile != 0)
								{
									fprintf(pFile, "\t\t\t\t");
									for (int i = 0; i < 16; i++)
										fprintf(pFile, "%02X", bBufR[15-i]);
									IPC_getTime(&time2);
									dTimeStamp = IPC_getDiffTime(&time1, &time2);
									fprintf(pFile, "\t%u\t", IPC_getTickCount() - dTimeStamp);
									fprintf(pFile, "%d\n", m_PID);
									fclose(pFile);
								}
							}
							if (bBufR[10] == bBuf[10])
								Status = BRDerr_OK;
						}
						else
						{
							if (m_isLog == 1)
							{
								for (int i=0; i<15; i++)
								{
									pFile = fopen("commands.txt", "a");
									if (pFile != 0)
										break;
								}
								if (pFile != 0)
								{
									fprintf(pFile, "\t\t\t\tno answer\t");
									IPC_getTime(&time2);
									dTimeStamp = IPC_getDiffTime(&time1, &time2);
									fprintf(pFile, "%u\t", dTimeStamp);
									fprintf(pFile, "%d\n", m_PID);
									fclose(pFile);
								}
							}
							getTaskId = 0;
						}
					}
					if (Status == BRDerr_OK)
					{
						if (m_TaskID == 0)
							m_TaskID = ((char *)bBufR)[8];
					}
					else
					{
						IPC_releaseMutex(m_hCommand);
						if (m_isLog == 1)
						{
							for (int i=0; i<15; i++)
							{
								pFile = fopen("commands.txt", "a");
								if (pFile != 0)
									break;
							}
							if (pFile != 0)
							{
								fprintf(pFile, "release mutex");
								fprintf(pFile, "\t");
								fprintf(pFile, "%d\n", m_PID);
								fclose(pFile);
							}
						}
						return BRDerr_ERROR;
					}
				}
				if (Status == 6)//transaction
				{
					int nRepeat = ((bPacket[10] - bBufR[10])&0xFF);
					while (nRepeat != 0)
					{
						for (int i = 0; i < 100; i++)
						{
							hReplay = IPC_createEvent(replayEvent, false, false);
							if (hReplay != 0)
								break;
						}
						rOvLapR.hEvent = (HANDLE)IPC_getDescriptor(hReplay);
						//CreateEvent(NULL, false, false, NULL);
						pResR = m_pDevice->EndPoints[m_EPin]->BeginDataXfer(bBufR, nLen, &rOvLapR);
						while (errCnt < 10)
						{
							if (m_pDevice->EndPoints[m_EPin]->WaitForXfer(&rOvLapR, 100) == true)
								break;
							errCnt++;
						}
						isRes = m_pDevice->EndPoints[m_EPin]->FinishDataXfer(bBufR, nLen, &rOvLapR, pResR);
						IPC_deleteEvent(hReplay);
						if (isRes)
						{
							nRepeat--;
							if (m_isLog == 1)
							{
								for (int i=0; i<15; i++)
								{
									pFile = fopen("commands.txt", "a");
									if (pFile != 0)
										break;
								}
								if (pFile != 0)
								{
									fprintf(pFile, "\t\t\t\t");
									for (int i = 0; i < 16; i++)
										fprintf(pFile, "%02X", bBufR[15-i]);
									IPC_getTime(&time2);
									dTimeStamp = IPC_getDiffTime(&time1, &time2);
									fprintf(pFile, "\t%u\t", dTimeStamp);
									fprintf(pFile, "%d\n", m_PID);
									fclose(pFile);
								}
							}
						}
						else
						{
							if (m_isLog == 1)
							{
								for (int i=0; i<15; i++)
								{
									pFile = fopen("commands.txt", "a");
									if (pFile != 0)
										break;
								}
								if (pFile != 0)
								{
									fprintf(pFile, "\t\t\t\tno answer\t");
									IPC_getTime(&time2);
									dTimeStamp = IPC_getDiffTime(&time1, &time2);
									fprintf(pFile, "%u\t", dTimeStamp);
									fprintf(pFile, "%d\n", m_PID);
									fclose(pFile);
								}
							}
							nRepeat = 0;
						}
					}
				}
				if (Status > 10)//return error code
				{
					IPC_releaseMutex(m_hCommand);
					if (m_isLog == 1)
					{
						for (int i=0; i<15; i++)
						{
							pFile = fopen("commands.txt", "a");
							if (pFile != 0)
								break;
						}
						if (pFile != 0)
						{
							fprintf(pFile, "release mutex");
							fprintf(pFile, "\t");
							fprintf(pFile, "%d\n", m_PID);
							fclose(pFile);
						}
					}
					return BRDerr_ERROR;
				}
			}
			else
				if (Status != 0)
				{
					IPC_releaseMutex(m_hCommand);
					if (m_isLog == 1)
					{
						for (int i=0; i<15; i++)
						{
							pFile = fopen("commands.txt", "a");
							if (pFile != 0)
								break;
						}
						if (pFile != 0)
						{
							fprintf(pFile, "release mutex");
							fprintf(pFile, "\t");
							fprintf(pFile, "%d\n", m_PID);
							fclose(pFile);
						}
					}
					return BRDerr_FATAL;
				}
			if (m_TaskID == 0)
				m_TaskID = ((char *)bBufR)[8];
			memcpy(&LValue, bBufR, 4);
			memcpy(&HValue, bBufR+4, 4);
		}
		else
		{
			U08 tmpBuf[16] = {0};
			if (m_isLog == 1)
			{
				for (int i=0; i<15; i++)
				{
					pFile = fopen("commands.txt", "a");
					if (pFile != 0)
						break;
				}
				if (pFile != 0)
				{
					fprintf(pFile, "no answer\t");
					IPC_getTime(&time2);
					dTimeStamp = IPC_getDiffTime(&time1, &time2);
					fprintf(pFile, "%u\t", dTimeStamp);
					fprintf(pFile, "%d\n", m_PID);
					fclose(pFile);
				}
			}
			Status = BRDerr_NOT_READY;
			int nRepeat = 0;
			while (nRepeat != 0)
			{
				IPC_delay(100);
				nLen = 16;
				for (int i = 0; i < 100; i++)
				{
					hReplay = IPC_createEvent(replayEvent, false, false);
					if (hReplay != 0)
						break;
				}
				rOvLapR.hEvent = (HANDLE)IPC_getDescriptor(hReplay);
				//CreateEvent(NULL, false, false, NULL);
				pResR = m_pDevice->EndPoints[m_EPin]->BeginDataXfer(tmpBuf, nLen, &rOvLapR);
				while (errCnt < 10)
				{
					if (m_pDevice->EndPoints[m_EPin]->WaitForXfer(&rOvLapR, 100) == true)
						break;
					errCnt++;
				}
				isRes = m_pDevice->EndPoints[m_EPin]->FinishDataXfer(tmpBuf, nLen, &rOvLapR, pResR);
				IPC_deleteEvent(hReplay);
				if (isRes && (nLen == 16))
				{
					Status = CheckCommand(bBuf, tmpBuf);
					if (Status == 0)
					{
						if (m_TaskID == 0)
							m_TaskID = ((char *)tmpBuf)[8];
						memcpy(&LValue, tmpBuf, 4);
						memcpy(&HValue, tmpBuf+4, 4);
						nRepeat = 0;
					}
					if (m_isLog == 1)
					{
						for (int i=0; i<15; i++)
						{
							pFile = fopen("commands.txt", "a");
							if (pFile != 0)
								break;
						}
						if (pFile != 0)
						{
							fprintf(pFile, "ask again\t\t");
							for (int i = 0; i < 16; i++)
								fprintf(pFile, "%02X", tmpBuf[15-i]);
							IPC_getTime(&time2);
							dTimeStamp = IPC_getDiffTime(&time1, &time2);
							fprintf(pFile, "\t%u", dTimeStamp);
							fprintf(pFile, "\t");
							fprintf(pFile, "%d\n", m_PID);
							fclose(pFile);
						}
					}
					Status = 0;
				}
				else
				{
					if (m_isLog == 1)
					{
						for (int i=0; i<15; i++)
						{
							pFile = fopen("commands.txt", "a");
							if (pFile != 0)
								break;
						}
						if (pFile != 0)
						{
							fprintf(pFile, "\t");
							//for (int i = 0; i < 16; i++)
							//	fprintf(pFile, "%02X", tmpBuf[15-i]);
							fprintf(pFile, "\t");
							fprintf(pFile, "ask again\t\tno answer\t");
							IPC_getTime(&time2);
							dTimeStamp = IPC_getDiffTime(&time1, &time2);
							fprintf(pFile, "%u\t", dTimeStamp);
							fprintf(pFile, "%d\n", m_PID);
							fclose(pFile);
						}
					}
					nRepeat--;
				}
			}
			IPC_releaseMutex(m_hCommand);
			if (m_isLog == 1)
			{
				for (int i=0; i<15; i++)
				{
					pFile = fopen("commands.txt", "a");
					if (pFile != 0)
						break;
				}
				if (pFile != 0)
				{
					fprintf(pFile, "release mutex");
					fprintf(pFile, "\t");
					fprintf(pFile, "%d\n", m_PID);
					fclose(pFile);
				}
			}
			return BRDerr_NOT_READY;

		}
	}
	else
	{
		if (m_isLog == 1)
		{
			for (int i=0; i<15; i++)
			{
				pFile = fopen("commands.txt", "a");
				if (pFile != 0)
					break;
			}
			if (pFile != 0)
			{
				fprintf(pFile, "can't send\t");
				IPC_getTime(&time2);
				dTimeStamp = IPC_getDiffTime(&time1, &time2);
				fprintf(pFile, "%u\t", dTimeStamp);
				fprintf(pFile, "%d\n", m_PID);
				fclose(pFile);
			}
		}
		m_pDevice->EndPoints[m_EPin]->FinishDataXfer(bBufR, nLen, &rOvLapR, pResR);
		IPC_deleteEvent(hReplay);
		IPC_releaseMutex(m_hCommand);
		if (m_isLog == 1)
			{
				for (int i=0; i<15; i++)
				{
					pFile = fopen("commands.txt", "a");
					if (pFile != 0)
						break;
				}
				if (pFile != 0)
				{
					fprintf(pFile, "release mutex");
					fprintf(pFile, "\t");
					fprintf(pFile, "%d\n", m_PID);
					fclose(pFile);
				}
			}
		return BRDerr_NOT_READY;
	}
	memcpy(&LValue, bBufR, 4);
	memcpy(&HValue, bBufR+4, 4);
#ifdef __IPC_WIN__
	QueryPerformanceCounter(&finish);
	delta.QuadPart = (finish.QuadPart - start.QuadPart) * 1000000 / freq.QuadPart;
	if (m_isLog)
	{
		for (int i = 0; i<15; i++)
		{
			pFile = fopen("commands.txt", "a");
			if (pFile != 0)
				break;
		}
		if (pFile != 0)
		{
			fprintf(pFile, "finish transaction in %lld us\n", delta.QuadPart);
			fclose(pFile);
		}
	}
#endif
	IPC_releaseMutex(m_hCommand);
	if (m_isLog == 1)
			{
				for (int i=0; i<15; i++)
				{
					pFile = fopen("commands.txt", "a");
					if (pFile != 0)
						break;
				}
				if (pFile != 0)
				{
					fprintf(pFile, "release mutex");
					fprintf(pFile, "\t");
					fprintf(pFile, "%d\n", m_PID);
					fclose(pFile);
				}
			}
	return Status;
}

ULONG CBambusb::SendBuffer(PVOID pOutBuf, PVOID pInBuf, LONG& nOutSize)
{
	BRDCHAR replayEvent[128]={0};
	BRDCHAR pStr[10];
	BRDC_strcat(replayEvent, _BRDC("Event_Replays_"));
	BRDC_strcat(replayEvent, BRDC_itoa(m_PID, pStr, 10));
	DWORD dTimeStamp = IPC_getTickCount();
	IPC_TIMEVAL time1, time2;
	LONG nLen = nOutSize;
	IPC_handle hReplay;
	if (nOutSize <= 0)
	{
		//printf("wrong out size");
		return BRDerr_ERROR;
	}
	if (!pOutBuf)
	{
		//printf("wrong out pointer");
		return BRDerr_ERROR;
	}
	if (!pInBuf)
	{
		//printf("wrong in pointer");
		return BRDerr_ERROR;
	}
	int errCnt = 0;
	while (1)
	{
		if (IPC_captureMutex(m_hCommand, 10) == 0)
			break;
		errCnt++;
		if (errCnt >= 6000)
			return BRDerr_INSUFFICIENT_RESOURCES;
	}
	m_LastComTime = IPC_getTickCount();
	IPC_getTime(&time1);
	OVERLAPPED rOvLap;
	PUCHAR pRes;
	LONG nLenR=nOutSize;
	for (int i = 0; i < 100; i++)
	{
		hReplay = IPC_createEvent(replayEvent, false, false);
		if (hReplay != 0)
			break;
	}
	rOvLap.hEvent = (HANDLE)IPC_getDescriptor(hReplay);
	//CreateEvent(NULL, false, false, NULL);
	m_LastComTime = IPC_getTickCount();
	pRes = m_pDevice->EndPoints[m_EPin]->BeginDataXfer((PUCHAR)pInBuf, nLenR, &rOvLap);
	if (m_pDevice->EndPoints[1]->XferData((PUCHAR)pOutBuf, nOutSize) != true)
	{
		if (m_isLog == 1)
		{
			FILE* pFile;
			while(1)
			{
				pFile = fopen("commands.txt", "a");
				if (pFile != 0)
					break;
			}
			IPC_getTime(&time2);
			dTimeStamp = IPC_getDiffTime(&time1, &time2);
			fprintf(pFile, "can't send %d commands %u\t", nLen/16, dTimeStamp);
			fprintf(pFile, "%d\n", m_PID);
			fclose(pFile);
		}
		m_pDevice->EndPoints[m_EPin]->FinishDataXfer((PUCHAR)pInBuf, nLenR, &rOvLap, pRes);
		IPC_deleteEvent(hReplay);
		IPC_releaseMutex(m_hCommand);
		return BRDerr_HW_BUSY;
	}
	if (m_isLog == 1)
	{
		FILE* pFile;
		while(1)
		{
			pFile = fopen("commands.txt", "a");
			if (pFile != 0)
				break;
		}
		fprintf(pFile, "%d send %d commands\t", m_TaskID, nLen/16);
		fclose(pFile);
	}
	while (1)
		if (m_pDevice->EndPoints[m_EPin]->WaitForXfer(&rOvLap, 100) == true)
			break;
	if (m_pDevice->EndPoints[m_EPin]->FinishDataXfer((PUCHAR)pInBuf, nLenR, &rOvLap, pRes) != true)
	{
		if (m_isLog == 1)
		{
			FILE* pFile;
			while(1)
			{
				pFile = fopen("commands.txt", "a");
				if (pFile != 0)
					break;
			}
			IPC_getTime(&time2);
			dTimeStamp = IPC_getDiffTime(&time1, &time2);
			fprintf(pFile, "can't get answer %u\t", dTimeStamp);
			fprintf(pFile, "%d\n", m_PID);
			fclose(pFile);
		}
		IPC_deleteEvent(hReplay);
		IPC_releaseMutex(m_hCommand);
		return BRDerr_HW_ERROR;
	}
	if (m_isLog == 1)
	{
		FILE* pFile;
		while(1)
		{
			pFile = fopen("commands.txt", "a");
			if (pFile != 0)
				break;
		}
		IPC_getTime(&time2);
		dTimeStamp = IPC_getDiffTime(&time1, &time2);
		fprintf(pFile, "receive %d commands %u\t", nLenR/16, dTimeStamp);
		fprintf(pFile, "%d\n", m_PID);

		UCHAR *pOut, *pIn;
		pOut = (PUCHAR)pOutBuf;
		pIn = (PUCHAR)pInBuf;
		fprintf(pFile, "start of pack\n");
		for (int i = 0; i < nOutSize / 16; i++)
		{
			for (int j = 15; j >= 0; j--)
				fprintf(pFile, "%02X", pOut[j + i * 16]);
			fprintf(pFile, "\t");
			for (int j = 15; j >= 0; j--)
				fprintf(pFile, "%02X", pIn[j + i * 16]);
			fprintf(pFile, "\n");
		}
		fprintf(pFile, "end of pack\n\n");
		fclose(pFile);
	}
	IPC_deleteEvent(hReplay);
	IPC_releaseMutex(m_hCommand);
	return BRDerr_OK;
}

ULONG CBambusb::WriteRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
{
	ULONG status = 0;
	int i = 1;
	while (i > 0)
	{
		status = SendCommand(AdmNum, TetrNum, RegNum, RegVal, 2, 0, 0, 1);
		if (status == BRDerr_OK)
			break;
		i--;
	}
	if (status == BRDerr_ERROR)
		status = SendCommand(AdmNum, TetrNum, RegNum, RegVal, 2, 0, 0, 1);
	return status;
}

ULONG CBambusb::WriteRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
{
	ULONG status = 0;
	int i = 1;
	while (i > 0)
	{
		status = SendCommand(AdmNum, TetrNum, RegNum, RegVal, 1, 1, 0, 1);
		if (status == BRDerr_OK)
			break;
		i--;
	}
	if (status == BRDerr_ERROR)
		status = SendCommand(AdmNum, TetrNum, RegNum, RegVal, 1, 1, 0, 1);
	return status;
}

ULONG CBambusb::ReadRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
{
	ULONG status = 0;
	int i = 1;
	while (i > 0)
	{
		status = SendCommand(AdmNum, TetrNum, RegNum, RegVal, 2, 0, 0, 0);
		if (status == BRDerr_OK)
			break;
		i--;
	}
	if (status == BRDerr_ERROR)
		status = SendCommand(AdmNum, TetrNum, RegNum, RegVal, 2, 0, 0, 0);
	RegVal &= 0xFFFF;
	return status;
}

ULONG CBambusb::ReadRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
{
	ULONG status = 0;
	int i = 1;
	while (i > 0)
	{
		status = SendCommand(AdmNum, TetrNum, RegNum, RegVal, 1, 1, 0, 0);
		if (status == BRDerr_OK)
			break;
		i--;
	}
	if (status == BRDerr_ERROR)
		status = SendCommand(AdmNum, TetrNum, RegNum, RegVal, 1, 1, 0, 0);
	return status;
}

ULONG CBambusb::WriteRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
	ULONG status = 0;
	rUsbHostCommand arOutComs[128] = { 0 }, arInComs[128] = {0};
	U32 RegVal;
	int nComm = 0;
	for (unsigned int i = 0; i < RegBufSize; i += 2)
	{
		if ((i + 2) > RegBufSize)
		{
			RegVal = 0;
			memcpy(&RegVal, (PUCHAR)RegBuf + i, 1);
		}
		else
			memcpy(&RegVal, (PUCHAR)RegBuf + i, 2);
		FormCommand(AdmNum, TetrNum, RegNum, RegVal, 2, 0, 0, 1, (PVOID)(&(arOutComs[nComm])));
		RegNum++;
		nComm++;
		if (nComm == 128)
		{
			LONG nLen = nComm * 16;
			status = SendBuffer(arOutComs, arInComs, nLen);
			nComm = 0;
		}
		if (status != 0)
			break;
	}
	if (nComm != 0)
	{
		LONG nLen = nComm * 16;
		status = SendBuffer(arOutComs, arInComs, nLen);
	}
	//U32 RegVal = 0, bufSize = 2;
	//
	//for (unsigned int i = 0; i < RegBufSize; i += 2)
	//{
	//	if ((i+2) > RegBufSize)
	//		bufSize = 1;
	//	memcpy(&RegVal, (PUCHAR)RegBuf + i, bufSize);
	//
	//	int j = 1;
	//	while (j > 0)
	//	{
	//		status = SendCommand(AdmNum, TetrNum, RegNum, RegVal, 2, 0, 0, 1);
	//		if (status == BRDerr_OK)
	//			break;
	//		j--;
	//	}
	//	RegNum++;
	//}
	return status;
}

ULONG CBambusb::WriteRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
	ULONG status = 0;
	rUsbHostCommand arOutComs[128] = { 0 }, arInComs[128] = { 0 };
	U32 RegVal;
	int nComm = 0;
	for (unsigned int i = 0; i < RegBufSize; i += 4)
	{
		if ((i + 4) > RegBufSize)
		{
			RegVal = 0;
			memcpy(&RegVal, (PUCHAR)RegBuf + i, RegBufSize - (i - 1));
		}
		else
			memcpy(&RegVal, (PUCHAR)RegBuf + i, 4);
		FormCommand(AdmNum, TetrNum, RegNum, RegVal, 1, 1, 0, 1, (PVOID)(&(arOutComs[nComm])));
		nComm++;
		if (nComm == 128)
		{
			LONG nLen = nComm * 16;
			status = SendBuffer(arOutComs, arInComs, nLen);
			nComm = 0;
		}
		if (status != 0)
			break;
	}
	if (nComm != 0)
	{
		LONG nLen = nComm * 16;
		status = SendBuffer(arOutComs, arInComs, nLen);
	}
	return status;
	/*U32 RegVal = 0, bufSize = 4;
	ULONG status = 0;
	for (unsigned int i = 0; i < RegBufSize; i += 4)
	{
		if ((i+4) > RegBufSize)
			bufSize = RegBufSize - (i - 1);
		memcpy(&RegVal, (PUCHAR)RegBuf + i, bufSize);
		int j = 1;
		while (j > 0)
		{
			status = SendCommand(AdmNum, TetrNum, RegNum, RegVal, 1, 1, 0, 1);
			if (status == BRDerr_OK)
				break;
			j--;
		}
	}
	return BRDerr_OK;*/
}

ULONG CBambusb::ReadRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
	ULONG status = 0;
	rUsbHostCommand arOutComs[128] = { 0 }, arInComs[128] = { 0 };
	U32 RegVal;
	int nComm = 0, nPack = 0;
	for (unsigned int i = 0; i < RegBufSize; i += 2)
	{
		RegVal = 0;
		FormCommand(AdmNum, TetrNum, RegNum, RegVal, 2, 0, 0, 0, (PVOID)(&(arOutComs[nComm])));
		nComm++;
		if (nComm == 128)
		{
			LONG nLen = nComm * 16;
			status = SendBuffer(arOutComs, arInComs, nLen);
			for (int j = 0; j < nComm; j++)
			{
				memcpy((PUCHAR)RegBuf + 2 * (nPack*128 + j), &(arInComs[j].valueL), 2);
			}
			nComm = 0;
			nPack++;
		}
		if (status != 0)
			break;
	}
	if (nComm != 0)
	{
		LONG nLen = nComm * 16;
		status = SendBuffer(arOutComs, arInComs, nLen);
		for (int j = 0; j < nComm; j++)
		{
			memcpy((PUCHAR)RegBuf + 2 * (nPack * 128 + j), &(arInComs[j].valueL), 2);
		}
	}
	return status;
	/*U32 RegVal = 0, bufSize = 2;
	ULONG status = 0;
	for (unsigned int i = 0; i < RegBufSize; i += 2)
	{
		int j = 1;
		while (j > 0)
		{
			status = SendCommand(AdmNum, TetrNum, RegNum, RegVal, 2, 0, 0, 0);
			if (status == BRDerr_OK)
				break;
			j--;
		}
		if ((i+2) > RegBufSize)
			bufSize = 1;
		memcpy((PUCHAR)RegBuf + i, &RegVal, bufSize);
		RegNum++;
	}
	return BRDerr_OK;*/
}

ULONG CBambusb::ReadRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
	ULONG status = 0;
	rUsbHostCommand arOutComs[128] = { 0 }, arInComs[128] = { 0 };
	U32 RegVal;
	int nComm = 0, nPack = 0;
	for (unsigned int i = 0; i < RegBufSize; i += 4)
	{
		RegVal = 0;
		FormCommand(AdmNum, TetrNum, RegNum, RegVal, 1, 1, 0, 0, (PVOID)(&(arOutComs[nComm])));
		nComm++;
		if (nComm == 128)
		{
			LONG nLen = nComm * 16;
			status = SendBuffer(arOutComs, arInComs, nLen);
			for (int j = 0; j < nComm; j++)
			{
				memcpy((PUCHAR)RegBuf + 4 * (nPack * 128 + j), &(arInComs[j].valueL), 4);
			}
			nComm = 0;
			nPack++;
		}
		if (status != 0)
			break;
	}
	if (nComm != 0)
	{
		LONG nLen = nComm * 16;
		status = SendBuffer(arOutComs, arInComs, nLen);
		for (int j = 0; j < nComm; j++)
		{
			memcpy((PUCHAR)RegBuf + 4 * (nPack * 128 + j), &(arInComs[j].valueL), 4);
		}
	}
	return status;
	/*U32 RegVal = 0, bufSize = 4;
	ULONG status = 0;
	for (unsigned int i = 0; i < RegBufSize; i += 4)
	{
		int j = 1;
		while (j > 0)
		{
			status = SendCommand(AdmNum, TetrNum, RegNum, RegVal, 1, 1, 0, 0);
			if (status == BRDerr_OK)
				break;
			j--;
		}
		if ((i + 4) > RegBufSize)
			bufSize = RegBufSize - (i - 1);
		memcpy((PUCHAR)RegBuf + i, &RegVal, bufSize);
	}
	return BRDerr_OK;*/
}

ULONG CBambusb::PackExecute(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
	ULONG status = 0;
	int commNum = RegBufSize / sizeof(DEVS_CMD_Reg), messageNum = 1;
	LONG nLen = 0;
	PDEVS_CMD_Reg commands = (PDEVS_CMD_Reg)RegBuf;
	rUsbHostCommand outComms[128] = {0}, inComms[128] = {0};
	if (commNum > 128)
	{
		messageNum = commNum / 128;
		if (commNum % 128 != 0)
			messageNum++;
	}
	for (int i = 0; i < messageNum; i++)
	{
		if (IPC_getTickCount() - m_LastComTime >= 61000)
		{
			m_TaskID = 0;
			U32 lVal = 0;
			SendCommand(0, 0, 0, lVal, 0, 0, 1, 1);
		}
		nLen = 0;
		for (int j = 0; j < 128; j++)
		{
			if ((i*128 + j) == commNum)
				break;
			outComms[j].taskID = m_TaskID;
			outComms[j].signatureValue = 0xA5;
			outComms[j].shortTransaction = m_Trans;
			m_Trans++;
			switch(commands[i*128 + j].idxMain)
			{
			case 0://REG READ DIR
				outComms[j].command.hardware.number = commands[i*128 + j].tetr;
				outComms[j].command.hardware.port = 1;
				outComms[j].command.hardware.type = 1;
				outComms[j].command.hardware.write = 0;
				outComms[j].valueH = commands[i*128 + j].reg;
				outComms[j].valueL = 0;
				if (m_isLog == 1)
				{
					FILE *pFile;
					for (int i = 0; i<15; i++)
					{
						pFile = fopen("commands.txt", "a");
						if (pFile != 0)
							break;
					}
					if (pFile != 0)
					{
						fprintf(pFile, "read direct tetr=%d reg=%d val=%d \t", outComms[j].command.hardware.number, outComms[j].valueH, outComms[j].valueL);
						//GetSystemTime(&time2);
						//dTimeStamp = ((time2.wHour * 60 + time2.wMinute) * 60 + time2.wSecond) * 1000 + time2.wMilliseconds - ((time1.wHour * 60 + time1.wMinute) * 60 + time1.wSecond) * 1000 - time1.wMilliseconds;
						//fprintf(pFile, "\t%u\t", dTimeStamp);
						fprintf(pFile, "%d\n", m_PID);
						fclose(pFile);
					}
				}
				break;
			case 2://REG READ IND
				outComms[j].command.tetrad.condition = 0;
				outComms[j].command.tetrad.number = commands[i*128 + j].tetr;
				outComms[j].command.tetrad.type = 2;
				outComms[j].valueH = commands[i*128 + j].reg;
				outComms[j].valueL = 0;
				if (m_isLog == 1)
				{
					FILE *pFile;
					for (int i = 0; i<15; i++)
					{
						pFile = fopen("commands.txt", "a");
						if (pFile != 0)
							break;
					}
					if (pFile != 0)
					{
						fprintf(pFile, "read tetr=%d reg=%d val=%d \t", outComms[j].command.tetrad.number, outComms[j].valueH, outComms[j].valueL);
						//GetSystemTime(&time2);
						//dTimeStamp = ((time2.wHour * 60 + time2.wMinute) * 60 + time2.wSecond) * 1000 + time2.wMilliseconds - ((time1.wHour * 60 + time1.wMinute) * 60 + time1.wSecond) * 1000 - time1.wMilliseconds;
						//fprintf(pFile, "\t%u\t", dTimeStamp);
						fprintf(pFile, "%d\n", m_PID);
						fclose(pFile);
					}
				}
				break;
			case 4://REG WRITE DIR
				outComms[j].command.hardware.number = commands[i*128 + j].tetr;
				outComms[j].command.hardware.port = 1;
				outComms[j].command.hardware.type = 1;
				outComms[j].command.hardware.write = 1;
				outComms[j].valueH = commands[i*128 + j].reg;
				outComms[j].valueL = commands[i*128 + j].val;
				if (m_isLog == 1)
				{
					FILE *pFile;
					for (int i = 0; i<15; i++)
					{
						pFile = fopen("commands.txt", "a");
						if (pFile != 0)
							break;
					}
					if (pFile != 0)
					{
						fprintf(pFile, "write direct tetr=%d reg=%d val=%d \t", outComms[j].command.hardware.number, outComms[j].valueH, outComms[j].valueL);
						//GetSystemTime(&time2);
						//dTimeStamp = ((time2.wHour * 60 + time2.wMinute) * 60 + time2.wSecond) * 1000 + time2.wMilliseconds - ((time1.wHour * 60 + time1.wMinute) * 60 + time1.wSecond) * 1000 - time1.wMilliseconds;
						//fprintf(pFile, "\t%u\t", dTimeStamp);
						fprintf(pFile, "%d\n", m_PID);
						fclose(pFile);
					}
				}
				break;
			case 6://REG WRITE IND
				outComms[j].command.tetrad.condition = 4;
				outComms[j].command.tetrad.number = commands[i*128 + j].tetr;
				outComms[j].command.tetrad.type = 2;
				outComms[j].valueH = commands[i*128 + j].reg;
				outComms[j].valueL = commands[i*128 + j].val;
				if (m_isLog == 1)
				{
					FILE *pFile;
					for (int i = 0; i<15; i++)
					{
						pFile = fopen("commands.txt", "a");
						if (pFile != 0)
							break;
					}
					if (pFile != 0)
					{
						fprintf(pFile, "write tetr=%d reg=%d val=%d \t", outComms[j].command.tetrad.number, outComms[j].valueH, outComms[j].valueL);
						//GetSystemTime(&time2);
						//dTimeStamp = ((time2.wHour * 60 + time2.wMinute) * 60 + time2.wSecond) * 1000 + time2.wMilliseconds - ((time1.wHour * 60 + time1.wMinute) * 60 + time1.wSecond) * 1000 - time1.wMilliseconds;
						//fprintf(pFile, "\t%u\t", dTimeStamp);
						fprintf(pFile, "%d\n", m_PID);
						fclose(pFile);
					}
				}
				break;
			default:
				status = BRDerr_BAD_PARAMETER;
				break;
			}
			if (status != BRDerr_OK)
				break;
			nLen++;
		}
		if (status != BRDerr_OK)
			break;
		nLen *= 16;
		status = SendBuffer(outComms, inComms, nLen);
		for (int j = 0; j < 128; j++)
		{
			if ((i*128 + j) == commNum)
				break;
			switch(commands[i*128 + j].idxMain)
			{
			case 0://REG READ DIR
				commands[i * 128 + j].val = inComms[j].valueL;
				if (m_isLog == 1)
				{
					FILE *pFile;
					for (int i = 0; i<15; i++)
					{
						pFile = fopen("commands.txt", "a");
						if (pFile != 0)
							break;
					}
					if (pFile != 0)
					{
						fprintf(pFile, "read direct tetr=%d reg=%d val=%d sign=%X\t", inComms[j].command.hardware.number, inComms[j].valueH, inComms[j].valueL, inComms[j].signatureValue);
						//GetSystemTime(&time2);
						//dTimeStamp = ((time2.wHour * 60 + time2.wMinute) * 60 + time2.wSecond) * 1000 + time2.wMilliseconds - ((time1.wHour * 60 + time1.wMinute) * 60 + time1.wSecond) * 1000 - time1.wMilliseconds;
						//fprintf(pFile, "\t%u\t", dTimeStamp);
						fprintf(pFile, "%d\n", m_PID);
						fclose(pFile);
					}
				}
				break;
			case 2://REG READ IND
				commands[i*128 + j].val = inComms[j].valueL;
				if (m_isLog == 1)
				{
					FILE *pFile;
					for (int i = 0; i<15; i++)
					{
						pFile = fopen("commands.txt", "a");
						if (pFile != 0)
							break;
					}
					if (pFile != 0)
					{
						fprintf(pFile, "read tetr=%d reg=%d val=%d sign=%X\t", inComms[j].command.tetrad.number, inComms[j].valueH, inComms[j].valueL, inComms[j].signatureValue);
						//GetSystemTime(&time2);
						//dTimeStamp = ((time2.wHour * 60 + time2.wMinute) * 60 + time2.wSecond) * 1000 + time2.wMilliseconds - ((time1.wHour * 60 + time1.wMinute) * 60 + time1.wSecond) * 1000 - time1.wMilliseconds;
						//fprintf(pFile, "\t%u\t", dTimeStamp);
						fprintf(pFile, "%d\n", m_PID);
						fclose(pFile);
					}
				}
				break;
			case 4://REG WRITE DIR
				if (m_isLog == 1)
				{
					FILE *pFile;
					for (int i = 0; i<15; i++)
					{
						pFile = fopen("commands.txt", "a");
						if (pFile != 0)
							break;
					}
					if (pFile != 0)
					{
						fprintf(pFile, "write direct tetr=%d reg=%d val=%d sign=%X\t", inComms[j].command.hardware.number, inComms[j].valueH, inComms[j].valueL, inComms[j].signatureValue);
						//GetSystemTime(&time2);
						//dTimeStamp = ((time2.wHour * 60 + time2.wMinute) * 60 + time2.wSecond) * 1000 + time2.wMilliseconds - ((time1.wHour * 60 + time1.wMinute) * 60 + time1.wSecond) * 1000 - time1.wMilliseconds;
						//fprintf(pFile, "\t%u\t", dTimeStamp);
						fprintf(pFile, "%d\n", m_PID);
						fclose(pFile);
					}
				}
				break;
			case 6://REG WRITE IND
				if (m_isLog == 1)
				{
					FILE *pFile;
					for (int i = 0; i<15; i++)
					{
						pFile = fopen("commands.txt", "a");
						if (pFile != 0)
							break;
					}
					if (pFile != 0)
					{
						fprintf(pFile, "write tetr=%d reg=%d val=%d sign=%X\t", inComms[j].command.tetrad.number, inComms[j].valueH, inComms[j].valueL, inComms[j].signatureValue);
						//GetSystemTime(&time2);
						//dTimeStamp = ((time2.wHour * 60 + time2.wMinute) * 60 + time2.wSecond) * 1000 + time2.wMilliseconds - ((time1.wHour * 60 + time1.wMinute) * 60 + time1.wSecond) * 1000 - time1.wMilliseconds;
						//fprintf(pFile, "\t%u\t", dTimeStamp);
						fprintf(pFile, "%d\n", m_PID);
						fclose(pFile);
					}
				}
				break;
			default:
				break;
			}
		}
	}
	if (m_isLog == 1)
	{
		FILE *pFile;
		for (int i = 0; i<15; i++)
		{
			pFile = fopen("commands.txt", "a");
			if (pFile != 0)
				break;
		}
		if (pFile != 0)
		{
			fprintf(pFile, "\n");
			fclose(pFile);
		}
	}
	return status;
}

ULONG CBambusb::I2CCapture()
{
	ULONG Status;
	U32 lVal = 400000;
	//захват I2C
	Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
	while (1)
	{
		Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 0);
		if (lVal == 400000)
			break;
		else
		{
			lVal = 400000;
			Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
		}
	}
	//
	return Status;
}

ULONG CBambusb::I2CRelease()
{
	U32 lVal = 400000;
	ULONG Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
	return Status;
}

ULONG CBambusb::SetMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	ULONG Status = BRDerr_OK;
	PAMB_MEM_DMA_CHANNEL arg = (PAMB_MEM_DMA_CHANNEL)pParam;
	int i = arg->DmaChanNum;
	m_pStreamDscr[i]->hBlockEndEvent = (HANDLE)arg->hBlockEndEvent;
	m_pStreamDscr[i]->hBufEndEvent = NULL;
#ifdef _WIN64
	m_pStreamDscr[i]->hBufEndEvent = arg->hTransEndEvent;
#endif
	m_pStreamDscr[i]->BlockCount = arg->BlockCnt;
	m_pStreamDscr[i]->BlockSize = (arg->BlockSize >> 10) << 10;
	if (m_pStreamDscr[i]->BlockSize == 0)
		return BRDerr_BUFFER_TOO_SMALL;
	if (m_pStreamDscr[i]->BlockSize != arg->BlockSize)
	{
		arg->BlockSize = m_pStreamDscr[i]->BlockSize;
		Status = BRDerr_PARAMETER_CHANGED;
	}
	m_pStreamDscr[i]->pBlock = new PVOID[m_pStreamDscr[i]->BlockCount];
	for (ULONG j=0; j<m_pStreamDscr[i]->BlockCount; j++)
		m_pStreamDscr[i]->pBlock[j] = arg->pBlock[j];
	m_pStreamDscr[i]->CurState.state = BRDstrm_STAT_STOP;
	m_pStreamDscr[i]->CurState.totalCounter = 0;
	m_pStreamDscr[i]->CurState.lastBlock = -1;
	m_pStreamDscr[i]->isBreak = 2;
	arg->pStub = &(m_pStreamDscr[i]->CurState);
	if (arg->MemType == 0)
	{
		if (arg->Direction == BRDstrm_DIR_INOUT)
		{
			//FILE *pFile;
			//pFile = fopen("commands.txt", "a");
			//fprintf(pFile, "wrong stream direction\n", m_PID);
			//fclose(pFile);
			return BRDerr_BAD_PARAMETER;
		}
		//ищем подходящий для стрима свободный EndPoint
		if (arg->Direction == BRDstrm_DIR_IN)
		{
			m_pStreamDscr[i]->bEndPoint = m_EPin + 1;
			for (int jj = 0; jj < 4; jj ++)
				for (int j = 0; j < 4; j++)
					if ((m_pStreamDscr[j]->bEndPoint == m_pStreamDscr[i]->bEndPoint) && (i != j))
						m_pStreamDscr[i]->bEndPoint++;
			if (m_pStreamDscr[i]->bEndPoint > m_EPin + 5)
			{
				//FILE *pFile;
				//pFile = fopen("commands.txt", "a");
				//fprintf(pFile, "no free in stream\n", m_PID);
				//fclose(pFile);
				Status = BRDerr_ERROR;
			}

		}
		else
		{
			m_pStreamDscr[i]->bEndPoint = 2;
			for (int jj = 0; jj < 5; jj ++)
				for (int j = 0; j < 5; j++)
					if ((m_pStreamDscr[j]->bEndPoint == m_pStreamDscr[i]->bEndPoint) && (i != j))
						m_pStreamDscr[i]->bEndPoint++;
			if (m_pStreamDscr[i]->bEndPoint > 5)
			{
				//FILE *pFile;
				//pFile = fopen("commands.txt", "a");
				//fprintf(pFile, "no free out stream\n", m_PID);
				//fclose(pFile);
				Status = BRDerr_ERROR;
			}
		}
	}
	else
	{
		//FILE *pFile;
		//pFile = fopen("commands.txt", "a");
		//fprintf(pFile, "wrong memory type\n", m_PID);
		//fclose(pFile);
		return BRDerr_BAD_PARAMETER;
	}
	return Status;
}

ULONG CBambusb::FreeMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	ULONG Status = BRDerr_OK;
	PAMB_MEM_DMA_CHANNEL arg = (PAMB_MEM_DMA_CHANNEL)pParam;
	int i = arg->DmaChanNum;
	AMB_STATE_DMA_CHANNEL param = {0};
	param.DmaChanNum = i;
	StopMemory(&param, sizeof(AMB_STATE_DMA_CHANNEL), NULL);
	//if (m_pStreamDscr[i]->CurState.state == BRDstrm_STAT_RUN)
	//{//если стрим не остановлен, останавливаем
	//	m_pStreamDscr[i]->isBreak = 1;
	//	U08 isHang = 0;
	//	while (isHang < 20)
	//	{
	//		if (IPC_waitThread(m_pStreamDscr[i]->hThread, 500) == 0)
	//			break;
	//		isHang++;
	//	}
	//	if (isHang == 20)
	//	{
	//		//IPC_stopThread(m_pStreamDscr[i]->hThread);
	//	}
	//	//m_pStreamDscr[i]->CurState.state = BRDstrm_STAT_DESTROY;
	//}
	U08 bType, bDir=0;
	U32 RegVal, lVal = 0;
	ULONG AdmNum = 0;
	bType = 3;
	if (m_pStreamDscr[i]->bEndPoint > 6)
		bDir = 1;
	//if (m_pStreamDscr[i]->Cycling == false)
	//{
	//	DmaDisable(0, m_pStreamDscr[i]->nTetrNum);
	//	RegVal = 3;
	//	Status = SendCommand(AdmNum, ((m_pStreamDscr[i]->bEndPoint - 2 - (bDir * (m_EPin - 1))) << 5), 0, lVal, bType, 0, RegVal, 1); //dummy stop
	//	RegVal = 5;
	//	Status = SendCommand(AdmNum, ((m_pStreamDscr[i]->bEndPoint - 2 - (bDir * (m_EPin - 1))) << 5), 0, lVal, bType, 0, RegVal, 1); //stop
	//	RegVal = 6;
	//	Status = SendCommand(0, ((m_pStreamDscr[i]->bEndPoint - 2 - (bDir * (m_EPin - 1))) << 5), 0, lVal, bType, 0, RegVal, 1); //clear
	//}
	RegVal = 7;
	//закрываем стрим в ARM
	Status = SendCommand(0, ((m_pStreamDscr[i]->bEndPoint - 2 - (bDir * (m_EPin - 1))) << 5), 0, lVal, bType, 0, RegVal, 1); //close
	//m_pDevice->EndPoints[m_pStreamDscr[i]->bEndPoint]->Abort();
	//m_pDevice->EndPoints[m_pStreamDscr[i]->bEndPoint]->Reset();
	if (m_isLog)
	{
		FILE *pFile;
		while(true)
		{
			pFile = fopen("commands.txt", "a");
			if (pFile != 0)
				break;
		}
		if (pFile != 0)
		{
			fprintf(pFile, "close thread %d", i);
			fprintf(pFile, "\t");
			fprintf(pFile, "%d\n", m_PID);
			fclose(pFile);
		}
	}
	m_pStreamDscr[i]->bEndPoint = 0xFF;
	m_pStreamDscr[i]->nTetrNum = 0xFF;
	delete [] m_pStreamDscr[i]->pBlock;
	m_pStreamDscr[i]->pBlock = 0;
	m_pStreamDscr[i]->nDone = 0;
	m_pStreamDscr[i]->isAdjust = 0;
	return Status;
}

HANDLE g_hStreamSem;

typedef struct _THREAD_PARAM {
	PUSB_STREAM_DESC pStreamDscr;
	CCyFX3Device* pDev;
	int idx;
        /*IPC_handle*/int hStream;
	IPC_handle	m_hCommandMutex;
} THREAD_PARAM, *PTHREAD_PARAM;

#ifdef WIN32
unsigned __stdcall ThreadOutput(void* pParams)
#else
unsigned ThreadOutput(void* pParams)
#endif // WIN32
{
	/*PUCHAR *apBuf = new*/ PUCHAR apBuf[30]={0};
	/*PUCHAR *apRes = new*/ PUCHAR apRes[30]={0};
	OVERLAPPED arOverlap[30] = {0};
	U08 aCloseBuf[30] = {0};
	IPC_handle hComm[30] = {0};
	IPC_handle	m_hCommand;
	ULONG Status = BRDerr_OK, nQueue;
	LONG nLen = 512*1024, nSendLen;
	bool isSend=true, status;
	UINT nBufGet=0, nBufPut=0, /*nBufSet=0, nFreeBuf = 8,*/ nBufPerBlock = 1, nBlockPut = 0, isMutex = 0;
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
//#else
//	pthread_t thId = pthread_self();
//	pthread_attr_t thAttr;
//	int policy = 0;
//	int max_prio_for_policy = 0;
//	pthread_attr_init(&thAttr);
//	pthread_attr_getschedpolicy(&thAttr, &policy);
//	max_prio_for_policy = sched_get_priority_max(policy);
//	pthread_setschedprio(thId, max_prio_for_policy);
//	pthread_attr_destroy(&thAttr);
#endif
	THREAD_PARAM threadParam;
    PTHREAD_PARAM pThreadParam;
	threadParam = *(PTHREAD_PARAM)pParams;
    pThreadParam = (PTHREAD_PARAM)pParams;
	CCyFX3Device* pDevice = threadParam.pDev;
	PUSB_STREAM_DESC pStreamDscr = threadParam.pStreamDscr;
	m_hCommand = threadParam.m_hCommandMutex;
    //IPC_unlockSemaphore(threadParam.hStream);
    pThreadParam->hStream = 1;
	//ReleaseSemaphore(threadParam.hStream, 1, 0);
	pStreamDscr->CurState.state = BRDstrm_STAT_RUN;
	pStreamDscr->CurState.lastBlock = -1;
	pStreamDscr->CurState.totalCounter = 0;
	pStreamDscr->CurState.offset = 0;
	pStreamDscr->nDone = -1;
	S32 tmpLastBlock = pStreamDscr->CurState.lastBlock;
	/*
	long long data_ex_cnt = 0;
	long long data_ex_noise = 1;
	long long data_ex_psd = 2;
	U32 buf_current = 0;
	U32 block_mode = 0;

	data_ex_cnt = 0;
	data_ex_noise = 1;
	data_ex_psd = 2;
	long long data_ex_psd_high = 0xAA55;
	long long data_ex_psd256[8] = { 0 };
	data_ex_psd256[0] = 2;
	data_ex_psd256[1] = 0xAA55;
	data_ex_psd256[2] = 0xBB66;
	data_ex_psd256[3] = 0xCC77;

	data_ex_psd256[4] = 0xDD88;
	data_ex_psd256[5] = 0xEE99;
	data_ex_psd256[6] = 0xFFAA;
	data_ex_psd256[7] = 0x100BB;
	*/
	//определяем количество буферов в очереди
	if (nLen >= pStreamDscr->BlockSize)
	{
		UINT08 nAddQueue = 0;
		for (int i = 0; i < 511; i++)
		{
			if ((nLen % pStreamDscr->BlockSize) == 0)
				break;
			nLen -= 1024; //nLen = nLen / (nAddQueue + 1 + (nLen / pStreamDscr->BlockSize));
			nAddQueue++;
		}
		nQueue = (nLen / pStreamDscr->BlockSize);
		if (nQueue > 8)
			nQueue = 8;
		if (nQueue > pStreamDscr->BlockCount)
		{
			nQueue = pStreamDscr->BlockCount;
			//if (pStreamDscr->Cycling == 0)
			//	nFreeBuf = 0;
		}
		//nFreeBuf = pStreamDscr->BlockCount - nQueue;
		//if (nFreeBuf > 8)
		//	nFreeBuf = 8;
		nLen = pStreamDscr->BlockSize;
		if (nQueue == 1)
		{
			nQueue = 2;
			nLen = pStreamDscr->BlockSize/2;
			nBufPerBlock = 2;
			//nFreeBuf = 0;
			//nBufSet = 0;
		}
		else
		{
			nBufPerBlock = 1;
			//nBufSet = 0;
		}
	}
	else
	{
		//printf("buffer size more than nLen\n");
		//nLen *= 2;
		UINT08 nAddQueue = 0;
		nLen = 256 * 1024;
		for (int i = 0; i < 255; i++)
		{
			if ((pStreamDscr->BlockSize % nLen) == 0)
				break;
			nLen -= 1024;//pStreamDscr->BlockSize / (nAddQueue + 1 + (pStreamDscr->BlockSize / nLen));
			nAddQueue++;
		}
		nQueue = pStreamDscr->BlockSize / nLen;
		if (nQueue > 8)
			nQueue = 8;
		//if ((pStreamDscr->BlockSize / 2) < nLen)
		//	nLen = pStreamDscr->BlockSize / 2;
		nBufPerBlock = pStreamDscr->BlockSize / nLen;
		//nFreeBuf = 0;
		//if (nQueue > 1)
		//	nBufSet = nQueue - 1;
		//else
			//nBufSet = 0;
	}
	nBufPut = 0;
	nBlockPut = 0;
	isSend = true;
	//printf ("nLen %d block size %d queue %d buf set %d buf put %d\n", nLen, pStreamDscr->BlockSize, nQueue, nBufSet, nBufPut);
#ifndef __CYPRESS_API
	if (pStreamDscr->Cycling)
		IPC_delay(250);
#endif
	for (unsigned int i = 0; i < nQueue; i++)
	{
#ifdef __IPC_WIN__
		{
			BRDCHAR pStr[10], eventName[128]={0};
			wchar_t *tmpWChar;
			U32	devPID = wcstol(pDevice->SerialNumber, &tmpWChar, 10);//_wtoi(pDevice->SerialNumber);
			BRDC_strcat(eventName, _BRDC("stream_event_"));
			BRDC_strcat(eventName, BRDC_itoa(i, pStr, 10));
			BRDC_strcat(eventName, _BRDC("_"));
			BRDC_strcat(eventName, BRDC_itoa(pStreamDscr->bEndPoint, pStr, 10));
			BRDC_strcat(eventName, _BRDC("_"));
			BRDC_strcat(eventName, BRDC_itoa(devPID, pStr, 10));
			for (int j = 0; j < 100; j++)
			{
				hComm[i] = IPC_createEvent(eventName, false, false);
				if (hComm[i] != 0)
					break;
			}
		}
		//IPC_resetEvent(hComm[i]);
		arOverlap[i].hEvent = (HANDLE)IPC_getDescriptor(hComm[i]);
#else
		arOverlap[i].hEvent = hComm[i];
#endif

		//CreateEvent(NULL, false, false, NULL);
		nSendLen = nLen;
		//if (i < nQueue)
		//ставим буфера в очередь
		apBuf[i] = (PUCHAR)pStreamDscr->pBlock[nBlockPut] + nBufPut * nLen;
		apRes[i] = pDevice->EndPoints[pStreamDscr->bEndPoint]->BeginDataXfer(apBuf[i], nSendLen, &(arOverlap[i]));
		aCloseBuf[i] = 1;
		if (pDevice->EndPoints[pStreamDscr->bEndPoint]->NtStatus || pDevice->EndPoints[pStreamDscr->bEndPoint]->UsbdStatus)
		{
			isSend = false;
			break;
		}
		nBufPut++;
		if (nBufPut >= nBufPerBlock)
		{
			nBufPut = 0;
			nBlockPut++;
		}
	}
#ifndef __CYPRESS_API
	if (pStreamDscr->Cycling)
		IPC_delay(500);
#endif
	nBufGet = 0;
	int nHang = 0;
	do
	{
		if (pStreamDscr->isBreak >= 1)
			break;
		unsigned int i = 0;
		for (;;)
		{
			if (!isSend)
			{
				//printf("ERROR\n");
				pStreamDscr->isBreak = 1;
				break;
			}
			if (pStreamDscr->isBreak >= 1)
			{
				break;
			}
			isSend = true;
			if (!pDevice->EndPoints[pStreamDscr->bEndPoint]->WaitForXfer(&(arOverlap[nBufGet]), 100))//ждём пока передача завершится
			{
				//printf ("error 2\r");
				//isSend = false;
				//break;
				//BRDC_printf(_BRDC("awaits data\n"));
				if (nHang++ < 500)
					continue;
				else
				{
					//BRDC_printf(_BRDC("no data\n"));
					nHang = 0;
				}
			}
			nHang = 0;
			nSendLen = nLen;
			status = pDevice->EndPoints[pStreamDscr->bEndPoint]->FinishDataXfer(apBuf[nBufGet], nSendLen, &(arOverlap[nBufGet]), apRes[nBufGet]);//закрываем передачу
			IPC_deleteEvent(hComm[nBufGet]);
			aCloseBuf[nBufGet] = 0;
			if (nSendLen != nLen)
			{//передача завершилась с неправильным размером
				nSendLen = nLen;
//				apBuf[nBufGet] += nSendLen;
//				nSendLen = nLen - nSendLen;
//#ifdef __IPC_WIN__
//				{
//					BRDCHAR pStr[10], eventName[128]={0};
//					wchar_t *tmpWChar;
//					U32	devPID = wcstol(pDevice->SerialNumber, &tmpWChar, 10);//_wtoi(pDevice->SerialNumber);
//					BRDC_strcat(eventName, _BRDC("stream_event_"));
//					BRDC_strcat(eventName, BRDC_itoa(nBufGet, pStr, 10));
//					BRDC_strcat(eventName, _BRDC("_"));
//					BRDC_strcat(eventName, BRDC_itoa(pStreamDscr->bEndPoint, pStr, 10));
//					BRDC_strcat(eventName, _BRDC("_"));
//					BRDC_strcat(eventName, BRDC_itoa(devPID, pStr, 10));
//					for (int j = 0; j < 100; j++)
//					{
//						hComm[nBufGet] = IPC_createEvent(eventName, false, false);
//						if (hComm[nBufGet] != 0)
//							break;
//					}
//				}
//				arOverlap[nBufGet].hEvent = (HANDLE)IPC_getDescriptor(hComm[nBufGet]);
//#else
//				arOverlap[nBufGet].hEvent = hComm[nBufGet];
//#endif
//				apRes[nBufGet] = pDevice->EndPoints[pStreamDscr->bEndPoint]->BeginDataXfer(apBuf[nBufGet], nSendLen, &(arOverlap[nBufGet]));
//				aCloseBuf[nBufGet] = 1;
			}
//			else
			{
				/*
				for (int chn = 0; chn < 4; chn++)
				{
					U32 nnSize = nLen / 4;
					U32 d0, d1, ii, jj;
					U32 di0, di1;
					U32 cnt_err = 0;
					__int64 *ptr = (__int64*)apBuf[nBufGet];
					U32 size256 = nnSize / 8;
					__int64 data_ex;
					__int64 data_in;
					__int64 data_sig;
					__int64 data_ex1;
					__int64 data_ex2;
					data_ex = data_ex_psd256[chn];
					ptr += chn;
					for (ii = 0; ii<size256; ii++)
					{
						data_in = *ptr; ptr += 4;
						if (data_ex != data_in)
						{
							if (data_in)
							{
								data_ex = data_in;
							}
							else
							{
								data_ex = 1;
							}
						}
						{
							U32 data_h = data_ex >> 32;
							U32 f63 = data_h >> 31;
							U32 f62 = data_h >> 30;
							U32 f60 = data_h >> 28;
							U32 f59 = data_h >> 27;
							U32 f0 = (f63 ^ f62 ^ f60 ^ f59) & 1;

							data_ex <<= 1;
							data_ex &= ~1;
							data_ex |= f0;
						}
					}
					data_ex_psd256[chn] = data_ex;
				}
				*/
				pStreamDscr->CurState.offset += nSendLen;
				if (pStreamDscr->CurState.offset >= pStreamDscr->BlockSize)
				{//блок завершён
					tmpLastBlock++;
					if (tmpLastBlock >= (S32)pStreamDscr->BlockCount)
						tmpLastBlock = 0;
					i++;
				}
				if (pStreamDscr->CurState.lastBlock != tmpLastBlock)
				{
					pStreamDscr->CurState.lastBlock = tmpLastBlock;
					pStreamDscr->CurState.totalCounter++;
					pStreamDscr->CurState.offset = 0;
#ifdef _WIN32
					SetEvent(pStreamDscr->hBlockEndEvent);
#endif

					if (pStreamDscr->isAdjust == 1)
					{//ожидание для согласованного режима
						if (pStreamDscr->nDone != -1)
							for (int d = 0; d < 1000; d++)
							{
								if (pStreamDscr->CurState.lastBlock == pStreamDscr->nDone)
									IPC_delay(1);
							}
						else
							while (((pStreamDscr->CurState.lastBlock == 0) || (pStreamDscr->CurState.lastBlock == pStreamDscr->BlockCount)) && (i <= 1) && (pStreamDscr->nDone == -1))
								IPC_delay(1);

					}
				}
				if ((pStreamDscr->Cycling)||(nBlockPut < pStreamDscr->BlockCount))
				{
					nSendLen = nLen;
					apBuf[nBufGet] = (PUCHAR)pStreamDscr->pBlock[nBlockPut] + nBufPut * nLen;
					//memset(apBuf[nBufSet], nBufSet+1, nSendLen);
#ifdef __IPC_WIN__
					{
						BRDCHAR pStr[10], eventName[128]={0};
						wchar_t *tmpWChar = 0;
						U32	devPID = wcstol(pDevice->SerialNumber, &tmpWChar, 10);//_wtoi(pDevice->SerialNumber);
						BRDC_strcat(eventName, _BRDC("stream_event_"));
						BRDC_strcat(eventName, BRDC_itoa(nBufGet, pStr, 10));
						BRDC_strcat(eventName, _BRDC("_"));
						BRDC_strcat(eventName, BRDC_itoa(pStreamDscr->bEndPoint, pStr, 10));
						BRDC_strcat(eventName, _BRDC("_"));
						BRDC_strcat(eventName, BRDC_itoa(devPID, pStr, 10));
						for (int j = 0; j < 100; j++)
						{
							hComm[nBufGet] = IPC_createEvent(eventName, false, false);
							if (hComm[nBufGet] != 0)
								break;
						}
					}
					//IPC_resetEvent(hComm[nBufGet]);
					arOverlap[nBufGet].hEvent = (HANDLE)IPC_getDescriptor(hComm[nBufGet]);
#else
					arOverlap[nBufGet].hEvent = hComm[nBufGet];
#endif
					//CreateEvent(NULL, false, false, NULL);
					apRes[nBufGet] = pDevice->EndPoints[pStreamDscr->bEndPoint]->BeginDataXfer(apBuf[nBufGet], nSendLen, &(arOverlap[nBufGet]));//добавляем следующий буфер в очередь
					aCloseBuf[nBufGet] = 1;
					//nBufSet++;
					//if (nBufSet >= nQueue /*+ nFreeBuf*/)
					//	nBufSet = 0;
					nBufPut++;
					if (nBufPut >= nBufPerBlock)
					{
						nBlockPut++;
						if ((nBlockPut >= pStreamDscr->BlockCount) && (pStreamDscr->Cycling))
							nBlockPut = 0;
						nBufPut = 0;
					}
				}
				nBufGet++;
				if (nBufGet >= nQueue /*+ nFreeBuf*/)
					nBufGet = 0;
			}
			if (i >= pStreamDscr->BlockCount)
				break;
			if (pDevice->EndPoints[pStreamDscr->bEndPoint]->NtStatus || pDevice->EndPoints[pStreamDscr->bEndPoint]->UsbdStatus)
			{
			//	printf ("error 4 nt %x usbd %x ep %d\n", pDevice->EndPoints[pStreamDscr->bEndPoint]->NtStatus, pDevice->EndPoints[pStreamDscr->bEndPoint]->UsbdStatus, pStreamDscr->bEndPoint);
			//	isSend = false;
				break;
			}
		}
	}
	while (pStreamDscr->Cycling);
	if (pStreamDscr->Cycling == 0)
	{
		pStreamDscr->CurState.state = BRDstrm_STAT_STOP;
		//IPC_delay(5);
	}
	int errCnt = 0;
	while (1)
	{
		if (IPC_captureMutex(m_hCommand, 10) == 0)//ждём пока отправится другая команда
			break;
		errCnt++;
		if (errCnt >= 6000)
			break;
	}
	for (unsigned int i=0; i < (nQueue /*+ nFreeBuf*/); i++)//чистим очередь
	{
		//apBuf[i] = NULL;
		if (aCloseBuf[i] == 1)
		{

#ifndef __CYPRESS_API
			pDevice->EndPoints[pStreamDscr->bEndPoint]->WaitForXfer(&(arOverlap[i]), 10);
			if (pDevice->EndPoints[pStreamDscr->bEndPoint]->FinishDataXfer(apBuf[i], nSendLen, &(arOverlap[i]), apRes[i]) == false)
				pDevice->EndPoints[pStreamDscr->bEndPoint]->WaitForXfer(&(arOverlap[i]), 1000);
#else
			pDevice->EndPoints[pStreamDscr->bEndPoint]->WaitForXfer(&(arOverlap[i]), 200);
			pDevice->EndPoints[pStreamDscr->bEndPoint]->FinishDataXfer(apBuf[i], nSendLen, &(arOverlap[i]), apRes[i]);
#endif
			IPC_deleteEvent(hComm[i]);
		}

		//apRes[i] = NULL;
	}
	IPC_releaseMutex(m_hCommand);

	//pDevice->EndPoints[pStreamDscr->bEndPoint]->Abort();
	//pDevice->EndPoints[pStreamDscr->bEndPoint]->Reset();
	//delete [] apBuf;
	//delete [] apRes;
	//_endthreadex(0);
#ifdef _WIN32
	SetEvent(pStreamDscr->hBufEndEvent);
#endif
	//pStreamDscr->hThread = 0;
	//_endthreadex(0);
	return 0;
}

ULONG CBambusb::StartMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	ULONG Status = BRDerr_OK;
	THREAD_PARAM pThreadParam = {0};
	PAMB_START_DMA_CHANNEL pArg = (PAMB_START_DMA_CHANNEL)pParam;

	int i = pArg->DmaChanNum;
	//DmaDisable(0, m_pStreamDscr[i]->nTetrNum);
#ifndef _WIN64
	#ifdef _WIN32
		m_pStreamDscr[i]->hBufEndEvent = pOverlap->hEvent;
	#endif
#endif
	//ResetEvent(m_pStreamDscr[i]->hBufEndEvent);
	m_pStreamDscr[i]->Cycling = pArg->IsCycling;
	AMB_STATE_DMA_CHANNEL param = {0};
	param.DmaChanNum = i;
	//m_pStreamDscr[i]->isBreak = 0;
	pThreadParam.idx = i;
	pThreadParam.pStreamDscr = m_pStreamDscr[i];
	pThreadParam.pDev = m_pDevice;

	U32 RegVal=0, lVal = 0;
	ULONG AdmNum = 0;
	U08 bType = 3, bDir = 0;
	if ((m_pStreamDscr[i]->bEndPoint > (m_EPin + 4)) || (m_pStreamDscr[i]->bEndPoint == m_EPin-1))
	{
		//FILE *pFile;
		//pFile = fopen("commands.txt", "a");
		//fprintf(pFile, "try start stream with nonexist endpoint\n", m_PID);
		//fclose(pFile);
		return BRDerr_INSUFFICIENT_RESOURCES;
	}
	while (1)
	{
		if (IPC_captureMutex(m_hStartStop, 10) == 0)//ждём пока отправится другая команда
			break;
	}
	if (m_pStreamDscr[i]->bEndPoint > 6)
		bDir = 1;
	if (pArg->IsCycling == 1)
		Status = StopMemory(&param, sizeof(AMB_STATE_DMA_CHANNEL), NULL);
	else
	{
		if (m_pStreamDscr[i]->hThread != 0)
			IPC_deleteThread(m_pStreamDscr[i]->hThread);
		lVal = 0;
		RegVal = 5;
		Status = SendCommand(AdmNum, ((m_pStreamDscr[i]->bEndPoint - 2 - (bDir * (m_EPin - 1))) << 5), 0, lVal, bType, 0, RegVal, 1);//stop
	}
	//m_pDevice->EndPoints[m_pStreamDscr[i]->bEndPoint]->Abort();
	//m_pDevice->EndPoints[m_pStreamDscr[i]->bEndPoint]->Reset();
	RegVal = 2;
	//
	//
	if (Status != 0)
	{
		lVal = 0;
		Status = SendCommand(AdmNum, ((m_pStreamDscr[i]->bEndPoint - 2 - (bDir * (m_EPin - 1))) << 5) + 2, 0, lVal, bType, 0, RegVal, 1); //dummy start
		if (Status != 0)
		{
			//FILE *pFile;
			//pFile = fopen("commands.txt", "a");
			//fprintf(pFile, "dummy start error %x\n", Status);
			//fclose(pFile);
			IPC_releaseMutex(m_hStartStop);
			return Status;
		}
		DmaEnable(0, m_pStreamDscr[i]->nTetrNum);
	}
	m_pStreamDscr[i]->isBreak = 0;
	RegVal = 4;
	lVal = 0;
	Status = SendCommand(AdmNum, ((m_pStreamDscr[i]->bEndPoint - 2 - (bDir * (m_EPin - 1))) << 5), 0, lVal, bType, 0, RegVal, 1); //start
	if (Status != BRDerr_OK)
	{
		//FILE *pFile;
		//pFile = fopen("commands.txt", "a");
		//fprintf(pFile, "start error %x\n", Status);
		//fclose(pFile);
		IPC_releaseMutex(m_hStartStop);
		return Status;
	}
	//
	IPC_releaseMutex(m_hStartStop);
	//
	U32 isHang = 0;
	if (m_isLog)
	{
		FILE *pFile;
		while(true)
		{
			pFile = fopen("commands.txt", "a");
			if (pFile != 0)
				break;
		}
		if (pFile != 0)
		{
			fprintf(pFile, "try start thread %d", i);
			fprintf(pFile, "\t");
			fprintf(pFile, "%d\n", m_PID);
			fclose(pFile);
		}
	}
        /*IPC_handle hStreamSem;
	{
		BRDCHAR pStr[10], semName[128]={0};
		BRDC_strcat(semName, _BRDC("semaphore_thread_start_"));
		BRDC_strcat(semName, BRDC_itoa(i, pStr, 10));
		BRDC_strcat(semName, _BRDC("_"));
		BRDC_strcat(semName, BRDC_itoa(m_PID, pStr, 10));
		for (int j = 0; j < 100; j++)
		{
			hStreamSem = IPC_createSemaphore(semName, 0);//CreateSemaphore(0, 0, 1, 0);
			if (hStreamSem != 0)
				break;
		}
        }*/
    pThreadParam.hStream = 0;
	pThreadParam.m_hCommandMutex = m_hCommand;
	m_pStreamDscr[i]->hThread = 0;
	BRDCHAR pStr[10], streamName[128]={0};
	BRDC_strcat(streamName, _BRDC("stream_"));
	BRDC_strcat(streamName, BRDC_itoa(i, pStr, 10));
	BRDC_strcat(streamName, _BRDC("_"));
	BRDC_strcat(streamName, BRDC_itoa(m_PID, pStr, 10));
	for (int j = 0; j < 100; j++)
	{
		m_pStreamDscr[i]->hThread = IPC_createThread(streamName, (thread_func*)&ThreadOutput, (void*)&pThreadParam);
		if (m_pStreamDscr[i]->hThread != 0)
			break;
	}
	if (m_pStreamDscr[i]->hThread == 0)
	{
		//FILE *pFile;
		//pFile = fopen("commands.txt", "a");
		//fprintf(pFile, "cant create thread\n");
		//fclose(pFile);
		return BRDerr_FATAL;
    }
    IPC_TIMEVAL tStart, tCur;
    IPC_getTime(&tStart);
    IPC_getTime(&tCur);
    while (IPC_getDiffTime(&tStart, &tCur) < 150000)
    {
            if (pThreadParam.hStream == 1)
                    break;
            IPC_getTime(&tCur);
    }
        /*while(isHang < 5000)
	{//ожидание обработки переданных в тред параметров
                //if (IPC_lockSemaphore(hStreamSem, 10) == 0)
                if (pThreadParam.hStream == 1)
			break;
		isHang++;
	}
        if (isHang >= 500)*/
    if (pThreadParam.hStream != 1)
	{
		//FILE *pFile;
		//pFile = fopen("commands.txt", "a");
		//fprintf(pFile, "thread is hanged\n");
		//fclose(pFile);
		return BRDerr_WAIT_TIMEOUT;
	}
        //IPC_deleteSemaphore(hStreamSem);
	if (m_isLog)
	{
		FILE *pFile;
		while(true)
		{
			pFile = fopen("commands.txt", "a");
			if (pFile != 0)
				break;
		}
		if (pFile != 0)
		{
			fprintf(pFile, "started thread %d", i);
			fprintf(pFile, "\t");
			fprintf(pFile, "%d\n", m_PID);
			fclose(pFile);
		}
	}
	return Status;
}

ULONG CBambusb::StopMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	PAMB_STATE_DMA_CHANNEL pStateDmaChan = (PAMB_STATE_DMA_CHANNEL)pParam;
	ULONG i = pStateDmaChan->DmaChanNum, isHang = 0;
	ULONG Status = BRDerr_OK;
	m_pStreamDscr[i]->isBreak = 1;
	m_pStreamDscr[i]->CurState.state = BRDstrm_STAT_STOP;
	DWORD waitRes = 0;
	if (m_pStreamDscr[i]->hThread != NULL)
	{
		while (isHang < 1000)
		{
			waitRes = IPC_waitThread(m_pStreamDscr[i]->hThread, 1);//check if thread already terminated, i.e. waitRes==-1
			if ((waitRes == NULL)||(waitRes == -1))
				break;
			waitRes = IPC_waitThread(m_pStreamDscr[i]->hThread, 10);
			if ((waitRes == NULL)||(waitRes == -1)) // Wait until thread terminates
				break;
			isHang++;
		}
		if (isHang >= 25)
		{
			for (int j = 0; j < 3; j++)
				if (j != i)
					if (m_pStreamDscr[i]->hThread == m_pStreamDscr[j]->hThread)
						isHang = 0;
			if (isHang != 0)
				IPC_stopThread(m_pStreamDscr[i]->hThread);
		}
		IPC_deleteThread(m_pStreamDscr[i]->hThread);
	}
	m_pStreamDscr[i]->hThread = 0;
	//
	while (1)
	{
		if (IPC_captureMutex(m_hStartStop, 10) == 0)//ждём пока отправится другая команда
			break;
	}
	//
	//if (m_pStreamDscr[i]->Cycling == 1)
	DmaDisable(0, m_pStreamDscr[i]->nTetrNum);
	m_pStreamDscr[i]->hThread = NULL;
	if ((m_pStreamDscr[i]->bEndPoint > (m_EPin + 5)) || (m_pStreamDscr[i]->bEndPoint == m_EPin-1))
	{
		//FILE *pFile;
		//pFile = fopen("commands.txt", "a");
		//fprintf(pFile, "try stop stream with non-exist ep\n");
		//fclose(pFile);
		return BRDerr_INSUFFICIENT_RESOURCES;
	}
	if (m_pStreamDscr[i]->bEndPoint == 0xFF)
	{
		//FILE *pFile;
		//pFile = fopen("commands.txt", "a");
		//fprintf(pFile, "try stop non allocated stream\n");
		//fclose(pFile);
		return BRDerr_STREAM_NOT_ALLOCATED_YET;
	}

	pStateDmaChan->BlockNum = m_pStreamDscr[i]->CurState.lastBlock;
	pStateDmaChan->BlockCntTotal = m_pStreamDscr[i]->CurState.totalCounter;
	pStateDmaChan->OffsetInBlock = m_pStreamDscr[i]->CurState.offset;
	pStateDmaChan->DmaChanState = m_pStreamDscr[i]->CurState.state;

	U32 RegVal=0, lVal = 0;
	U08 bType = 1, bDir = 0;
	ULONG AdmNum = 0;

	bType = 3;
	if (m_pStreamDscr[i]->bEndPoint > 6)
		bDir = 1;
	//if (m_pStreamDscr[i]->Cycling == 1)
	//{
		Status = SendCommand(AdmNum, ((m_pStreamDscr[i]->bEndPoint - 2 - (bDir * (m_EPin - 1))) << 5), 0, lVal, bType, 0, 3, 1); //dummy stop
		m_pStreamDscr[i]->isBreak = 2;
	//}
	lVal = 0;
	RegVal = 5;
	Status = SendCommand(AdmNum, ((m_pStreamDscr[i]->bEndPoint - 2 - (bDir * (m_EPin - 1))) << 5), 0, lVal, bType, 0, RegVal, 1); //stop
	//
	IPC_releaseMutex(m_hStartStop);
	//
	//m_pDevice->EndPoints[m_pStreamDscr[i]->bEndPoint]->Abort();
	//m_pDevice->EndPoints[m_pStreamDscr[i]->bEndPoint]->Reset();
	//m_pStreamDscr[i]->isBreak = 0;
	if (m_isLog)
	{
		FILE *pFile;
		while(true)
		{
			pFile = fopen("commands.txt", "a");
			if (pFile != 0)
				break;
		}
		if (pFile != 0)
		{
			fprintf(pFile, "stop thread %d", i);
			fprintf(pFile, "\t");
			fprintf(pFile, "%d\n", m_PID);
			fclose(pFile);
		}
	}
	return Status;
}

ULONG CBambusb::StateMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	ULONG Status = BRDerr_OK;
	PAMB_STATE_DMA_CHANNEL pStateDmaChan = (PAMB_STATE_DMA_CHANNEL)pParam;
	ULONG i = pStateDmaChan->DmaChanNum;
	pStateDmaChan->BlockNum = m_pStreamDscr[i]->CurState.lastBlock;
	pStateDmaChan->BlockCntTotal = m_pStreamDscr[i]->CurState.totalCounter;
	pStateDmaChan->OffsetInBlock = m_pStreamDscr[i]->CurState.offset;
	pStateDmaChan->DmaChanState = m_pStreamDscr[i]->CurState.state;
	return Status;
}

ULONG CBambusb::SetDirection(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	return BRDerr_FUNC_UNIMPLEMENTED;
	ULONG Status = BRDerr_OK;
	PAMB_SET_DMA_CHANNEL pSetDirParam = (PAMB_SET_DMA_CHANNEL)pParam;
	ULONG i = pSetDirParam->DmaChanNum;
	if (pSetDirParam->Param == BRDstrm_DIR_IN)
	{
		if ((m_pStreamDscr[i]->bEndPoint < m_EPin)||(m_pStreamDscr[i]->bEndPoint == 0xFF))
		{
			m_pStreamDscr[i]->bEndPoint = m_EPin + 1;
			for (int jj = 0; jj < 4; jj ++)
				for (int j = 0; j < 4; j++)
					if ((m_pStreamDscr[j]->bEndPoint == m_pStreamDscr[i]->bEndPoint) && (i != j))
						m_pStreamDscr[i]->bEndPoint++;
			if (m_pStreamDscr[i]->bEndPoint > m_EPin + 5)
				Status = BRDerr_ERROR;
		}
		else
			return BRDerr_OK;
	}
	if (pSetDirParam->Param == BRDstrm_DIR_OUT)
	{
		if ((m_pStreamDscr[i]->bEndPoint > m_EPin)||(m_pStreamDscr[i]->bEndPoint == 0xFF))
		{
			m_pStreamDscr[i]->bEndPoint = 2;
			for (int jj = 0; jj < 5; jj ++)
				for (int j = 0; j < 5; j++)
					if ((m_pStreamDscr[j]->bEndPoint == m_pStreamDscr[i]->bEndPoint) && (i != j))
						m_pStreamDscr[i]->bEndPoint++;
			if (m_pStreamDscr[i]->bEndPoint > 5)
				Status = BRDerr_ERROR;
		}
		else
			return BRDerr_OK;
	}
	if (pSetDirParam->Param == BRDstrm_DIR_INOUT)
		return BRDerr_BAD_PARAMETER;
	return Status;
}

ULONG CBambusb::SetSource(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	ULONG Status = BRDerr_OK;
	PAMB_SET_DMA_CHANNEL arg = (PAMB_SET_DMA_CHANNEL)pParam;
	if (m_pStreamDscr[arg->DmaChanNum]->CurState.state != BRDstrm_STAT_STOP)
	{//если стрим не остановлен
		AMB_STATE_DMA_CHANNEL stopParam = {0};
		stopParam.DmaChanNum = arg->DmaChanNum;
		Status = StopMemory(&stopParam, sizeof(AMB_STATE_DMA_CHANNEL), NULL);
		if (Status != BRDerr_OK)
		{
			//FILE *pFile;
			//pFile = fopen("commands.txt", "a");
			//fprintf(pFile, "try set source for non stopped thread\n");
			//fclose(pFile);
			return BRDerr_STREAM_NOT_STOPPED_YET;
		}
	}
	U32 RegVal=0, lVal = 0;
	ULONG TetrNum, AdmNum = 0;
	U08 bType = 3, bDir = 0;
	TetrNum = arg->Param;
	if (m_pStreamDscr[arg->DmaChanNum]->nTetrNum == TetrNum)
		return BRDerr_OK;//эта тетрада уже привязана к этому стриму
	m_pStreamDscr[arg->DmaChanNum]->nTetrNum = TetrNum;
	if (m_pStreamDscr[arg->DmaChanNum]->bEndPoint == 0xFF)
	{
		//FILE *pFile;
		//pFile = fopen("commands.txt", "a");
		//fprintf(pFile, "try set source for nonallocated stream\n");
		//fclose(pFile);
		return BRDerr_STREAM_NOT_ALLOCATED_YET;
	}
	if (m_pStreamDscr[arg->DmaChanNum]->bEndPoint < 6)
	{
		RegVal = 0;
		bDir = 1;
	}
	else
	{
		RegVal = 1;
		bDir = 0;
	}
	while (true)//ищем свободный стрим
	{
		lVal = 0;
		Status = SendCommand(AdmNum, ((m_pStreamDscr[arg->DmaChanNum]->bEndPoint - 2 - ((m_EPin - 1)*RegVal)) << 5) + TetrNum, 1, lVal, bType, 0, bDir, 1);
		if (Status == BRDerr_OK)
			break;
		m_pStreamDscr[arg->DmaChanNum]->bEndPoint++;
		for (int i = 0; i < 4; i++)
			for (int ii = 0; ii < 4; ii++)
				if ((m_pStreamDscr[arg->DmaChanNum]->bEndPoint == m_pStreamDscr[ii]->bEndPoint) && (ii != arg->DmaChanNum))
					m_pStreamDscr[arg->DmaChanNum]->bEndPoint++;
		if ((m_pStreamDscr[arg->DmaChanNum]->bEndPoint > (m_EPin + 4)) || (m_pStreamDscr[arg->DmaChanNum]->bEndPoint == m_EPin))
		{
			//FILE *pFile;
			//pFile = fopen("commands.txt", "a");
			//fprintf(pFile, "no free streams\n");
			//fclose(pFile);
			return BRDerr_INSUFFICIENT_RESOURCES;
		}
	}
	//SetDmaMode(m_pStreamDscr[arg->DmaChanNum]->bEndPoint - 2 - ((m_EPin - 1)*RegVal), 0, m_pStreamDscr[arg->DmaChanNum]->nTetrNum);
	if (m_isLog)
	{
		FILE *pFile;
		for (int i=0; i<15; i++)
		{
			pFile = fopen("commands.txt", "a");
			if (pFile != 0)
				break;
		}
		if (pFile != 0)
		{
			fprintf(pFile, "open thread %d tetrad %d", arg->DmaChanNum, m_pStreamDscr[arg->DmaChanNum]->nTetrNum);
			fprintf(pFile, "\t");
			fprintf(pFile, "%d\n", m_PID);
			fclose(pFile);
		}
	}
	return Status;
}

ULONG CBambusb::SetDmaRequest(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	ULONG Status = BRDerr_OK;
	return Status;
}

ULONG CBambusb::GetDmaChanInfo(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	ULONG Status = BRDerr_OK;
	PAMB_GET_DMA_INFO prDmaInfo = (PAMB_GET_DMA_INFO)pParam;
	prDmaInfo->Direction = BRDstrm_DIR_INOUT;
	prDmaInfo->FifoSize = 1024*8;
	prDmaInfo->MaxDmaSize = 4*1024*1024;
	return Status;
}

ULONG CBambusb::GetTetrNum(ULONG& tetrNum)
{
	ULONG Status = 0;
	return Status;
}

ULONG CBambusb::SetDmaMode(ULONG NumberOfChannel, ULONG AdmNumber, ULONG TetrNumber)
{
	ULONG Status = BRDerr_OK;
	return Status;
}

ULONG CBambusb::DmaEnable(ULONG AdmNumber, ULONG TetrNumber)
{
	ULONG Status = BRDerr_OK;
	U32 Value=0;
	Status = ReadRegData(AdmNumber, TetrNumber, 0, Value);
	if (Status != BRDerr_OK)
		return Status;
	Value |= 0x8;
	Status = WriteRegData(AdmNumber, TetrNumber, 0, Value);
	return Status;
}

ULONG CBambusb::DmaDisable(ULONG AdmNumber, ULONG TetrNumber)
{
	ULONG Status = BRDerr_OK;
	U32 Value=0;
	Status = ReadRegData(AdmNumber, TetrNumber, 0, Value);
	if (Status != BRDerr_OK)
		return Status;
	Value &= 0xfff7;
	Status = WriteRegData(AdmNumber, TetrNumber, 0, Value);
	return Status;
}

ULONG CBambusb::ResetFIFO(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	ULONG Status = BRDerr_OK;
	PAMB_SET_DMA_CHANNEL arg = (PAMB_SET_DMA_CHANNEL) pParam;
	U32 RegVal=0, lVal = 0, i = arg->DmaChanNum, isHang = 0;
	U08 bType = 1, bDir = 0;
	ULONG AdmNum = 0;
	AMB_STATE_DMA_CHANNEL param = {0};
	param.DmaChanNum = i;
	Status = StopMemory(&param, sizeof(AMB_STATE_DMA_CHANNEL), NULL);
	if ((Status == BRDerr_INSUFFICIENT_RESOURCES)||(Status == BRDerr_STREAM_NOT_ALLOCATED_YET))
	{
            if (m_isLog)
            {
                FILE *pFile;
                pFile = fopen("commands.txt", "a");
                fprintf(pFile, "cant stop thread to reset fifo\n");
                fclose(pFile);
            }
		return Status;
	}
	m_pStreamDscr[arg->DmaChanNum]->isBreak = 2;
	U32 Value=0, TetrNumber, RegNum = 0;
	TetrNumber = m_pStreamDscr[arg->DmaChanNum]->nTetrNum;
	bType = 3;
	bDir = 0;
	if (m_pStreamDscr[arg->DmaChanNum]->bEndPoint > 6)
		bDir = 1;
	RegVal = 6;
	Status = SendCommand(0, ((m_pStreamDscr[arg->DmaChanNum]->bEndPoint - 2 - (bDir * (m_EPin - 1))) << 5), 0, Value, bType, 0, RegVal, 1); //clear
	return Status;
}

ULONG CBambusb::WaitDMABlock(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	ULONG Status = BRDerr_OK;
	IPC_TIMEVAL tStart, tCur;
#ifdef __IPC_WIN__
	PAMB_SET_DMA_CHANNEL arg = (PAMB_SET_DMA_CHANNEL)pParam;
	U32 tOut = arg->Param;
#else
	PAMB_STATE_DMA_CHANNEL arg = (PAMB_STATE_DMA_CHANNEL)pParam;
	U32 tOut = arg->Timeout;
#endif // __IPC_WIN__

	UINT nCurBlock = m_pStreamDscr[arg->DmaChanNum]->CurState.totalCounter;

        if (m_isLog)
        {
            FILE *pFile;
            pFile = fopen("commands.txt", "a");
            fprintf(pFile, "wait block >%d\n", nCurBlock);
            fclose(pFile);
        }
	IPC_getTime(&tStart);
	IPC_getTime(&tCur);
	while (IPC_getDiffTime(&tStart, &tCur) < tOut)
	{
		if (m_pStreamDscr[arg->DmaChanNum]->CurState.totalCounter > nCurBlock)
			break;
		IPC_getTime(&tCur);
	}
	if (m_pStreamDscr[arg->DmaChanNum]->CurState.totalCounter <= nCurBlock)
		Status = BRDerr_WAIT_TIMEOUT;
        if (m_isLog)
        {
            FILE *pFile;
            pFile = fopen("commands.txt", "a");
            fprintf(pFile, "wait block %d\n", Status);
            fclose(pFile);
        }
	return Status;
}

ULONG CBambusb::WaitDMABuffer(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	ULONG Status = BRDerr_OK;
	IPC_TIMEVAL tStart, tCur;
#ifdef __IPC_WIN__
	PAMB_SET_DMA_CHANNEL arg = (PAMB_SET_DMA_CHANNEL)pParam;
	U32 tOut = arg->Param;
#else
	PAMB_STATE_DMA_CHANNEL arg = (PAMB_STATE_DMA_CHANNEL)pParam;
	U32 tOut = arg->Timeout;
#endif // __IPC_WIN__

    if (m_isLog)
    {
        FILE *pFile;
        pFile = fopen("commands.txt", "a");
        fprintf(pFile, "wait buf\n");
        fclose(pFile);
    }
	IPC_getTime(&tStart);
	IPC_getTime(&tCur);
	while (IPC_getDiffTime(&tStart, &tCur) < tOut)
	{
		if (m_pStreamDscr[arg->DmaChanNum]->CurState.state == BRDstrm_STAT_STOP)
			break;
		IPC_getTime(&tCur);
	}
	if (m_pStreamDscr[arg->DmaChanNum]->CurState.state != BRDstrm_STAT_STOP)
		Status = BRDerr_WAIT_TIMEOUT;
    if (m_isLog)
    {
        FILE *pFile;
        pFile = fopen("commands.txt", "a");
        fprintf(pFile, "wait buf %d\n", Status);
        fclose(pFile);
    }
	return Status;
}

ULONG CBambusb::Adjust(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	ULONG Status = BRDerr_OK;
	PAMB_SET_DMA_CHANNEL arg = (PAMB_SET_DMA_CHANNEL)pParam;
	m_pStreamDscr[arg->DmaChanNum]->isAdjust = 1;
	return Status;
}

ULONG CBambusb::BufferDone(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	ULONG Status = BRDerr_OK;
	PAMB_SET_DMA_CHANNEL arg = (PAMB_SET_DMA_CHANNEL)pParam;
	if (m_pStreamDscr[arg->DmaChanNum]->isAdjust == 1)
		m_pStreamDscr[arg->DmaChanNum]->nDone = arg->Param;
	else
		Status = BRDerr_STREAM_ERROR;
	return Status;
}
