
#include	"utypes.h"
#include	"rw_i2c.h"

//***************************************************************************************
U32 getACK(ULONG* blockmem, int busnum, int devadr)
{
	//ULONG* BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
	//ULONG blk_id = BlockMem[0];
	//ULONG blk_ver = BlockMem[2];
	U32 pkg_o[128];
	U32 pkg_i[128];
	U32 ii = 0;
	U32 cnt_r = 0;
	U32 cnt_i = 0;
	U32 data_i;
	U32 cnt_wr = 0;
	U32 cnt_rd = 0;
	U32 max_cnt_wr = 0;
	U32 max_cnt_rd = 0;

	//pkg_o[ii++] = 0x8000;
	pkg_o[ii++] = 0x8000;
	pkg_o[ii++] = 0xC000 + devadr; max_cnt_rd++;
	pkg_o[ii++] = 0x8000;
	//pkg_o[ii++] = 0x8000;
	max_cnt_wr = ii;

	blockmem[52] = 0;		 // I2CB_DATA
	blockmem[42] = busnum;	// SPD_DEVICE

	for (; ; )
	{
		data_i = blockmem[52]; // читаем I2CB_DATA
		if (data_i & 0x2000) // проверяем бит TX_PAF
		{ // 1 – разрешение записи 16-ти слов в FIFO
			if (cnt_wr < max_cnt_wr)
			{
				blockmem[52] = pkg_o[cnt_wr];
				cnt_wr++;
			}
		}
		cnt_i = (data_i >> 8) & 0xF; // получаем номер прочитанного слова
		if (cnt_i != cnt_r)
		{
			if (cnt_rd < max_cnt_rd)
			{
				pkg_i[cnt_rd] = data_i;
				cnt_rd++;
			}
			cnt_r++;
			cnt_r &= 0xF;

		}

		if ((cnt_wr == max_cnt_wr) && (cnt_rd == max_cnt_rd))
			break;
	}

	do
	{
		data_i = blockmem[52]; // читаем I2CB_DATA
	} while (data_i & 0x4000); // проверяем бит RUN (когда он становится равен 0, выходим из цикла)

	blockmem[52] = 0; // освобождение шины

	return (pkg_i[0] & 0x1000); // ACK
}

//***************************************************************************************
ULONG readI2C(ULONG* blockmem, int busnum, int devadr, UCHAR* buf, USHORT offset, int length, int eeprom16kb)
{
	ULONG Status = 0;
	//ULONG* BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
	//ULONG blk_id = BlockMem[0];
	//ULONG blk_ver = BlockMem[2];

	U32 pkg_o[280];
	U32 pkg_i[280];
	U32 data_i;

	blockmem[52] = 0;		// I2CB_DATA
	blockmem[42] = busnum;	// SPD_DEVICE

							//length = 16;
	int off_cur = offset;
	int len_cur;
	int len_ost = length;
	do
	{
		int ii = 0;
		U32 cnt_r = 0;
		U32 cnt_i = 0;
		U32 cnt_wr = 0;
		U32 cnt_rd = 0;
		U32 max_cnt_wr = 0;
		U32 max_cnt_rd = 0;

		//pkg_o[ii++] = 0x8000;
		pkg_o[ii++] = 0x8000;
		if (eeprom16kb)
		{	// 16 kBytes EEPROM
			int psize_cur = 256 - (off_cur % 256);
			len_cur = len_ost < psize_cur ? len_ost : psize_cur;
			pkg_o[ii++] = 0xC000 + (devadr + ((off_cur / 256) << 1))/*0xC0A8*/; max_cnt_rd++; // запись 
			pkg_o[ii++] = 0xC000 + (off_cur % 256)/*0xC010*/; max_cnt_rd++;
		}
		else
		{	// 32 kBytes EEPROM
			len_cur = length;
			pkg_o[ii++] = 0xC000 + devadr; /*0xC0A8*/; max_cnt_rd++;
			pkg_o[ii++] = 0xC000 + (offset >> 8 & 0xFF)/*0xC000*/; max_cnt_rd++;
			pkg_o[ii++] = 0xC000 + (offset & 0xFF)/*0xC010*/; max_cnt_rd++;
		}
		pkg_o[ii++] = 0x8000;
		//pkg_o[ii++] = 0x8000;
		//pkg_o[ii++] = 0x8000;
		if (eeprom16kb)
		{
			pkg_o[ii++] = 0xC000 + (devadr + ((off_cur / 256) << 1)) + 1/*0xC0A9*/;  max_cnt_rd++; // транзакция чтения
		}
		else
		{
			pkg_o[ii++] = 0xC000 + devadr + 1; /*0xC0A8*/; max_cnt_rd++;
		}
		U32 idx = max_cnt_rd;
		for (int jj = 0; jj < len_cur - 1; jj++)
		{
			pkg_o[ii++] = 0xC000;
			max_cnt_rd++;
		}
		pkg_o[ii++] = 0x8001;  max_cnt_rd++;
		//pkg_o[ii++] = 0x8001;  //max_cnt_rd++;
		//pkg_o[ii++] = 0x8001;  //max_cnt_rd++;
		max_cnt_wr = ii;

		for (; ; )
		{
			data_i = blockmem[52]; // читаем I2CB_DATA
			if (data_i & 0x2000) // проверяем бит TX_PAF
			{ // 1 – разрешение записи 16-ти слов в FIFO
				if (cnt_wr < max_cnt_wr)
				{
					blockmem[52] = pkg_o[cnt_wr];
					cnt_wr++;
				}
			}
			cnt_i = (data_i >> 8) & 0xF; // получаем номер прочитанного слова
			if (cnt_i != cnt_r)
			{
				if (cnt_rd < max_cnt_rd)
				{
					pkg_i[cnt_rd] = data_i;
					cnt_rd++;
				}
				cnt_r++;
				cnt_r &= 0xF;

			}

			if ((cnt_wr == max_cnt_wr) && (cnt_rd == max_cnt_rd))
				break;
		}

		do
		{
			data_i = blockmem[52]; // читаем I2CB_DATA
		} while (data_i & 0x4000); // проверяем бит RUN (когда он становится равен 0, выходим из цикла)

		int boff = off_cur - offset;
		for (ii = 0; ii < len_cur; ii++)
		{
			data_i = pkg_i[idx + ii];
			buf[boff + ii] = data_i & 0xFF;
		}

		off_cur += len_cur;
		len_ost -= len_cur;

	} while (len_ost);

	blockmem[52] = 0; // освобождение шины

					  //for (int i = 0; i < Length; i++)
					  //{
					  //	int sector = Offset / 256;
					  //	blockmem[46] = Offset; // SPD_ADR
					  //	blockmem[44] = ((devadr + sector) << 4) + 1; // SPD_CTRL, type operation - read
					  //	if (WaitFidReady(blockmem) != 0)
					  //	{
					  //		buf[i] = (UCHAR)blockmem[48]; // read data
					  //		Status = 1;
					  //		break;
					  //	}
					  //	buf[i] = (UCHAR)blockmem[48]; // read data
					  //	Offset++;
					  //}
					  //blockmem[44] = 0; // SPD_CTRL

	return Status;
}

//***************************************************************************************
ULONG writeI2C(ULONG* blockmem, int busnum, int devadr, UCHAR* buf, USHORT offset, int length, int eeprom16kb)
{
	ULONG Status = 0;
	//ULONG* BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
	//ULONG blk_id = BlockMem[0];
	//ULONG blk_ver = BlockMem[2];

	U32 pkg_o[16];
	//U32 pkg_i[16];
	U32 data_i;

	blockmem[52] = 0;		// I2CB_DATA
	blockmem[42] = busnum;	// SPD_DEVICE

							//length = 40;
	for (int jj = 0; jj < length; jj++)
	{
		//	buf[jj] = jj;
		U32 ii = 0;
		U32 cnt_r = 0;
		U32 cnt_i = 0;
		U32 cnt_wr = 0;
		U32 cnt_rd = 0;
		U32 max_cnt_wr = 0;
		U32 max_cnt_rd = 0;

		//pkg_o[ii++] = 0x8000;
		pkg_o[ii++] = 0x8000;
		if (eeprom16kb)
		{	// 16kbits EEPROM
			int padr = ((offset + jj) / 256) << 1; // page address (for 16kbits EEPROM)
			int radr = (offset + jj) % 256; // register address on the page (for 16kbits EEPROM)
			pkg_o[ii++] = 0xC000 + (devadr + padr)/*0xC0A8*/; max_cnt_rd++;			// slave address + page address + транзакция записи
			pkg_o[ii++] = 0xC000 + radr/*0xC010*/; max_cnt_rd++;		// адрес регистра
		}
		else
		{	// 32kbits EEPROM
			int radr = offset + jj; // two bytes register address
			pkg_o[ii++] = 0xC000 + devadr; /*0xC0A8*/; max_cnt_rd++;				// транзакция записи
			pkg_o[ii++] = 0xC000 + (radr >> 8 & 0xFF)/*0xC000*/; max_cnt_rd++;	// старший байт адреса регистра
			pkg_o[ii++] = 0xC000 + (radr & 0xFF)/*0xC010*/; max_cnt_rd++;	// младший байт адреса регистра
		}

		pkg_o[ii++] = 0xC000 + buf[jj]; max_cnt_rd++;		// данные
		pkg_o[ii++] = 0x8001;
		//pkg_o[ii++] = 0x8001;  //max_cnt_rd++;
		//pkg_o[ii++] = 0x8001;  //max_cnt_rd++;
		max_cnt_wr = ii;

		for (; ; )
		{
			data_i = blockmem[52]; // читаем I2CB_DATA
			if (data_i & 0x2000) // проверяем бит TX_PAF
			{ // 1 – разрешение записи 16-ти слов в FIFO
				if (cnt_wr < max_cnt_wr)
				{
					blockmem[52] = pkg_o[cnt_wr];
					cnt_wr++;
				}
			}
			cnt_i = (data_i >> 8) & 0xF; // получаем номер прочитанного слова
			if (cnt_i != cnt_r)
			{
				if (cnt_rd < max_cnt_rd)
				{
					//pkg_i[cnt_rd] = data_i;
					cnt_rd++;
				}
				cnt_r++;
				cnt_r &= 0xF;

			}

			if ((cnt_wr == max_cnt_wr) && (cnt_rd == max_cnt_rd))
				break;
		}

		do
		{
			data_i = blockmem[52]; // читаем I2CB_DATA
		} while (data_i & 0x4000); // проверяем бит RUN (когда он становится равен 0, выходим из цикла)

		U32 ack = 1;
		do
		{
			ack = getACK(blockmem, busnum, devadr);
		} while (ack);
	}

	blockmem[52] = 0; // освобождение шины

	return Status;
}
