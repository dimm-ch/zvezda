//****************** File Icr00A5.h ***********************
//
//  ����������� �������� � �������� 
//	��� ������ � ���������������� ����
//  ��� ��������� FM401S
//
//*********************************************************

#ifndef _ICR00A5_H
 #define _ICR00A5_H

#pragma pack(push, 1)    

#define ADM_CFG_TAG 0x00A5

// ���������������� ��������� ���������
typedef struct _ICR_Cfg00A5 {
	U16	wTag;		// ��� ��������� (ADM_CFG_TAG)
	U16	wSize;		// ������ ���� ��������� ����� ���������
	U08	bAdmIfNum;	// ����� ���������� ADM
	U08	bSfpCnt;		// ���������� ����� SFP: 4, 8
	U08	bGen1Type;	// ��� ��������� �����. ���������� G1: 0-�� �������-��, 1-Si571
	U08	bGen1Adr;		// �������� ��� �����. ���������� G1: 0x49 �� ���������
	U32	nGen1Ref;   	// ��������� ��������� ������� �����. ���������� G1 (��)
	U32	nGen1RefMax; 	// ������������ ������� �����. ���������� G1 (��)
} ICR_Cfg00A5, *PICR_Cfg00A5, ICR_CfgAdm, *PICR_CfgAdm;

#pragma pack(pop)    

#endif // _ICR00A5_H

// ****************** End of file Icr00A5.h **********************