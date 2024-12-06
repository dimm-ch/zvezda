//****************** File Icr0098.h ***********************
//
//  ����������� �������� � �������� 
//	��� ������ � ���������������� ����
//  ��� ���������� FM216x250MDA
//
//*********************************************************

#ifndef _ICR0098_H
 #define _ICR0098_H

#pragma pack(push, 1)    

#define ADM_CFG_TAG 0x0098

// ���������������� ��������� ���������
typedef struct _ICR_Cfg0098 {
	U16	wTag;		// ��� ��������� (ADM_CFG_TAG)
	U16	wSize;		// ������ ���� ��������� ����� ���������
	U08	bAdmIfNum;	// ����� ���������� ADM
	U08	bAdcCnt;	// ���������� ���: 1, 2
	U08	bDacCnt;	// ���������� ���: 0, 1
	U32	dLPFCutoff;	// ������� ����� ������� ������ ������ (��)
	U08	bIsQuadMod;	// ������� ������������� ����������
	U08	bGenType;	// ��� ��������� �����. ����������: 0-�� �������-��, 1-Si571
	U08	bGenAdr;	// �������� ��� �����. ����������: 0x49 �� ���������
	U32	nGenRef;    // ��������� ��������� ������� �����. ���������� (��)
	U32	nGenRefMax; // ������������ ������� �����. ���������� (��)
} ICR_Cfg0098, *PICR_Cfg0098, ICR_CfgAdm, *PICR_CfgAdm;

#pragma pack(pop)    

#endif // _ICR0098_H

// ****************** End of file Icr0098.h **********************
