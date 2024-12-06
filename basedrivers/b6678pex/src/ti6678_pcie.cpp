/********************************************************************
 *  (C) Copyright 1992-2012 InSys Corp.  All rights reserved. 
 *
 *  Autor	- DOR
 *
 *  filename	- ti6678_pcie.cpp
 *  version 	- 1.0
 *  description:
 *		TI6678 PCIe Library
 *
 *
 *  Modify:
 ********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "utypes.h"
#include "ti6678_pcie.h"
#include "pcieLocalReset_6678.h"
#include	"b6678pex.h"

HANDLE	hDevice;
TI6678_DEVICE tiDev[MAX_TI6678_DEVICES];

static	int		no_load_print = 0;

//=******************** Pause() **************
//
static int	Pause(int counter)
{
	volatile int i;
	for(i=0;i<counter;i++);
	return 0;
}

int	TI6678PCIE_Close(TI6678_DEVICE *dev)
{
	return 0;
}

//============== TI6678PCIE_SetBootAddrIpcgr() =======================================
// core local reset TI6678
// -----------------------------------------------------------------------------
int	TI6678PCIE_SetBootAddrIpcgr(TI6678_DEVICE *dev, U32 core, U32 bootEntryAddr)
{
	U32	x;

	/* Check if the last 10 bits of addr is 0 */
	if ((bootEntryAddr & 0x3f) != 0) {
		//BRDC_printf(_BRDC("<TI6678PCIE_SetBootAddrIpcgr> The address is not 1K aligned - 0x%08X.\n"), bootEntryAddr );
		Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("<TI6678PCIE_SetBootAddrIpcgr> The address is not 1K aligned - 0x%08X.\n"), bootEntryAddr );

		//printf("The address is not 1K aligned.\n");
		return -1;
	}

	// Set MST_PRIV bit to access PSC via PCIE 
	x = *(U32*)(dev->mem_pciereg_va+PRIORITY); // 
	*(U32*)(dev->mem_pciereg_va+PRIORITY) = x|0x00010000;

	// Temporarily re-map IB region 1 to chip level registers
	*(U32*)(dev->mem_pciereg_va+IB_OFFSET(1)) = CHIP_LEVEL_BASE_ADDRESS;
	Pause(2000);

	/* Unlock KICK0, KICK1 */
	*(U32*)(dev->mem_bar1_va+KICK0) = KICK0_UNLOCK;
	*(U32*)(dev->mem_bar1_va+KICK1) = KICK1_UNLOCK;

	*(U32*)(dev->mem_bar1_va+DSP_BOOT_ADDR(core)) = bootEntryAddr;
	*(U32*)(dev->mem_bar1_va+IPCGR(core)) = 1;

	Pause(2000);

	return 0;
}

//============== TI6678PCIE_CoreLocalReset() =======================================
// core local reset TI6678
// dev  - device number
// node - node number
// pid  - power domain identifier
// mid  - module on power domain identifier
// state - control 
// -----------------------------------------------------------------------------
int	TI6678PCIE_CoreLocalReset(TI6678_DEVICE *dev, U32 core, U32 pid, U32 mid, U32 state )
{
	U32 x, counter;

	// Set MST_PRIV bit to access PSC via PCIE 
	x = *(U32*)(dev->mem_pciereg_va+PRIORITY); // restore core 0 address default
	*(U32*)(dev->mem_pciereg_va+PRIORITY) = x|0x00010000;

	// Temporarily re-map IB region 1 to PSC registers 
	*(U32*)(dev->mem_pciereg_va+IB_OFFSET(1)) = PSC_BASE_ADDRESS;
	Pause(4000);

	// Assert/De-assert local reset 
	x = *(U32*)(dev->mem_bar1_va+MDCTL(mid));
	if(state == LOC_RST_ASSERT) { 
		x = ((x & ~0x1F) | PSC_ENABLE) & (~0x100);
	} else {
		x = ((x & ~0x1F) | PSC_ENABLE) | (1 << 8);
	}
	*(U32*)(dev->mem_bar1_va+MDCTL(mid)) = x; 
	// wait for reset progress 
	Pause(4000);
	counter = 0;
	while (true) {
		x = *(U32*)(dev->mem_bar1_va+PTSTAT);
		if ((x & (1 << pid)) == 0) break;
		Pause(4000);
		IPC_delay(1);
		counter ++;
		if (counter > 10) {
			//printf("Previous transition in progress pid %d mid %d state: %d\n", pid, mid, state);
			break;
		}
	}
	Pause(4000);
	*(U32*)(dev->mem_bar1_va+PTCMD) = (1 << pid); 

	// Current transition finished 
	counter = 0;
	while (true) {
		x = *(U32*)(dev->mem_bar1_va+PTSTAT);
		if ((x & (1 << pid)) == 0) break;
		Pause(4000);
		IPC_delay(1);
		counter ++;
		if (counter > 10) {
			//printf("Current transition in progress pid %d mid %d state: %d\n", pid, mid, state);
			break;
		}
	}

	// Verifying state change 
	counter = 0;
	while (true) {
		x = *(U32*)(dev->mem_bar1_va+MDSTAT(mid));
		if ((x & 0x1F) == 3) break;
		Pause(4000);
		IPC_delay(1);
		counter ++;
		if (counter > 10) {
			//printf("MD stat for pid %d mid %d state: %d timeout\n", pid, mid, state);
			break;
		}
	}

	Pause(4000);

	return 0;
}

//============== TI6678PCIE_SetPscState() =======================================
//	Set a new power state for the specified domain id in a power controler
//  domain. Wait for the power transition to complete.
//   pid   -  power domain.
//   mid   -  module id to use for module in the specified power domain
//   state -  new state value to set (0 = RESET; 3 = ENABLE)
// -----------------------------------------------------------------------------
int	TI6678PCIE_SetPscState(TI6678_DEVICE *dev, U32 core, U32 pid, U32 mid, U32 state)
{
	U32	mdctl, pdctl, x, counter;

	// Set MST_PRIV bit to access PSC via PCIE 
	x = *(U32*)(dev->mem_pciereg_va+PRIORITY); // restore core 0 address default
	*(U32*)(dev->mem_pciereg_va+PRIORITY) = x|0x00010000;

	// Temporarily re-map IB region 1 to PSC registers 
	*(U32*)(dev->mem_pciereg_va+IB_OFFSET(1)) = PSC_BASE_ADDRESS;
	Pause(2000);

	mdctl = *(U32*)(dev->mem_bar1_va + MDCTL(mid));
	pdctl = *(U32*)(dev->mem_bar1_va + PDCTL(pid));


	/* No previous transition in progress */
	counter = 0;
	while (true) {
		x = *(U32*)(dev->mem_bar1_va + PTSTAT);
		if ((x & (1 << pid)) == 0) break;
		Pause(2000);
		IPC_delay(1);
		counter ++;
		if (counter > 10) {
			//printf("Previous transition in progress pid %d mid %d state: %d\n", pid, mid, state);
			break;
		}
	}

	// Set power domain control 
	*(U32*)(dev->mem_bar1_va + PDCTL(pid)) = pdctl | 0x1;

	// Set MDCTL NEXT to new state 
	mdctl = ((mdctl) & ~(0x1f)) | state;
	*(U32*)(dev->mem_bar1_va + MDCTL(mid)) = mdctl;

	// Start power transition by setting PTCMD GO to 1 
	x = *(U32*)(dev->mem_bar1_va + PTCMD);
	*(U32*)(dev->mem_bar1_va + PTCMD) = x | (0x1<<pid);

	// Current transition finished 
	counter = 0;
	while (true) {
		x = *(U32*)(dev->mem_bar1_va + PTSTAT);
		if ((x & (1 << pid)) == 0) break;
		Pause(2000);
		IPC_delay(1);
		counter ++;
		if (counter > 10) {
			//printf("Current transition in progress pid %d mid %d state: %d\n", pid, mid, state);
			break;
		}
	}

	// Verifying state change 
	counter = 0;
	while (true) {
		//x = *(U32*)(dev->mem_bar3_va + MDSTAT(mid));
		x = *(U32*)(dev->mem_bar1_va + MDSTAT(mid));
		if ((x & 0x1F) == state) break;
		Pause(2000);
		IPC_delay(1);
		counter ++;
		if (counter > 10) {
			//printf("MD stat for pid %d mid %d state: %d timeout\n", pid, mid, state);
			break;
		}
	}

	Pause(2000);

	return 0;
}

int	TI6678PCIE_LocalReset(TI6678_DEVICE *dev, U32 core)
{
	/* Local reset of all cores */
	TI6678PCIE_CoreLocalReset(dev, 0, PD8,  LPSC_C0_TIM0, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD9,  LPSC_C1_TIM1, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD10, LPSC_C2_TIM2, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD11, LPSC_C3_TIM3, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD12, LPSC_C4_TIM4, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD13, LPSC_C5_TIM5, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD14, LPSC_C6_TIM6, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD15, LPSC_C7_TIM7, LOC_RST_ASSERT);

	/* Disable all other modules */
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_EMIF16_SPI, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_TSIP, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_DEBUG, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_TETB_TRC, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SA, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SGMII, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_PA, PSC_SWRSTDISABLE);
	//TI6678PCIE_SetPscState(dev, 0, PD3, LPSC_PCIE, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD4, LPSC_SRIO, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD5, LPSC_HYPER, PSC_SWRSTDISABLE);
	//TI6678PCIE_SetPscState(dev, 0, PD6, LPSC_RESERV, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD7, LPSC_MSMCRAM, PSC_SWRSTDISABLE);

	return 0;
}

int	TI6678PCIE_LocalEntry(TI6678_DEVICE *dev, U32 core, U32 entry)
{
	if( TI6678PCIE_SetBootAddrIpcgr(dev,core,entry) < 0)
	{
		//BRDC_printf(_BRDC("<TI6678PCIE_LocalEntry> Core %d is not ready !!! \n"), core );
		Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("<TI6678PCIE_LocalEntry> Core %d is not ready !!! \n"), core );

		//printf("Core %d is not ready !!! \n", core);
		return -1;
	}
	return 0;
}

int	TI6678PCIE_LocalRelease(TI6678_DEVICE *dev, U32 core)
{
		/* Enable all other modules */
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_EMIF16_SPI, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_TSIP, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_DEBUG, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_TETB_TRC, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_PA, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SGMII, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SA, PSC_ENABLE);
	//TI6678PCIE_SetPscState(dev, 0, PD3, LPSC_PCIE, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD4, LPSC_SRIO, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD5, LPSC_HYPER, PSC_ENABLE);
	//TI6678PCIE_SetPscState(dev, 0, PD6, LPSC_RESERV, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD7, LPSC_MSMCRAM, PSC_ENABLE);

	/* Local out of reset of all cores */
	TI6678PCIE_CoreLocalReset(dev, 0, PD8,  LPSC_C0_TIM0, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD9,  LPSC_C1_TIM1, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD10, LPSC_C2_TIM2, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD11, LPSC_C3_TIM3, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD12, LPSC_C4_TIM4, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD13, LPSC_C5_TIM5, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD14, LPSC_C6_TIM6, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD15, LPSC_C7_TIM7, LOC_RST_DEASSERT);

	return 0;

}

//============== TI6678PCIE_ProcLocalReset() =======================================
// local reset TI6678
// dev  - device number
// node - node number
// -----------------------------------------------------------------------------
int	__TI6678PCIE_ProcLocalReset(TI6678_DEVICE *dev)
{

//---------- FIRST LOCAL RESET
	/* Local reset of all cores */
	TI6678PCIE_CoreLocalReset(dev, 0, PD8,  LPSC_C0_TIM0, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD9,  LPSC_C1_TIM1, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD10, LPSC_C2_TIM2, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD11, LPSC_C3_TIM3, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD12, LPSC_C4_TIM4, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD13, LPSC_C5_TIM5, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD14, LPSC_C6_TIM6, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD15, LPSC_C7_TIM7, LOC_RST_ASSERT);

	/* Disable all other modules */
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_EMIF16_SPI, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_TSIP, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_DEBUG, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_TETB_TRC, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SA, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SGMII, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_PA, PSC_SWRSTDISABLE);
	//TI6678PCIE_SetPscState(dev, 0, PD3, LPSC_PCIE, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD4, LPSC_SRIO, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD5, LPSC_HYPER, PSC_SWRSTDISABLE);
	//TI6678PCIE_SetPscState(dev, 0, PD6, LPSC_RESERV, PSC_SWRSTDISABLE);
	//TI6678PCIE_SetPscState(dev, 0, PD7, LPSC_MSMCRAM, PSC_SWRSTDISABLE);

	IPC_delay(5);

	for (int i = 0; i < 8; i++) {
		TI6678PCIE_SetBootAddrIpcgr(dev,i,0);
	}

	U32 x = 0x0001E003;
	U32 y = 0x0001E003;
	for (int i = 0; i < 8; i++) {
		TI6678PCIE_ProcWriteMem(dev, 0, 0x0C000000+4*i, &x, 4);
		TI6678PCIE_ProcReadMem(dev, 0, 0x0C000000+4*i, &y, 4);
	}

	for (int i = 0; i < 8; i++) {
		if( TI6678PCIE_SetBootAddrIpcgr(dev,i,0x0C000000) < 0)
		{
			//BRDC_printf(_BRDC("<TI6678PCIE_ProcLocalReset> Core %d is not ready !!! \n"), i );
			Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("<TI6678PCIE_ProcLocalReset> Core %d is not ready !!! \n"), i );

			//printf("Core %d is not ready !!! \n", i);
			return -1;
		}
	}

	IPC_delay(5);

	/* Enable all other modules */
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_EMIF16_SPI, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_TSIP, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_DEBUG, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_TETB_TRC, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_PA, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SGMII, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SA, PSC_ENABLE);
	//TI6678PCIE_SetPscStatdeve(dev, 0, PD3, LPSC_PCIE, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD4, LPSC_SRIO, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD5, LPSC_HYPER, PSC_ENABLE);
	//TI6678PCIE_SetPscState(dev, 0, PD6, LPSC_RESERV, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD7, LPSC_MSMCRAM, PSC_ENABLE);

	//Sleep(50);

	/* Local out of reset of all cores */

	TI6678PCIE_CoreLocalReset(dev, 0, PD8,  LPSC_C0_TIM0, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD9,  LPSC_C1_TIM1, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD10, LPSC_C2_TIM2, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD11, LPSC_C3_TIM3, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD12, LPSC_C4_TIM4, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD13, LPSC_C5_TIM5, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD14, LPSC_C6_TIM6, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD15, LPSC_C7_TIM7, LOC_RST_DEASSERT);

//-------------------------

	IPC_delay(5);

	/* Local reset of all cores */
	TI6678PCIE_CoreLocalReset(dev, 0, PD8,  LPSC_C0_TIM0, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD9,  LPSC_C1_TIM1, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD10, LPSC_C2_TIM2, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD11, LPSC_C3_TIM3, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD12, LPSC_C4_TIM4, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD13, LPSC_C5_TIM5, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD14, LPSC_C6_TIM6, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD15, LPSC_C7_TIM7, LOC_RST_ASSERT);

	/* Disable all other modules */
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_EMIF16_SPI, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_TSIP, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_DEBUG, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_TETB_TRC, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SA, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SGMII, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_PA, PSC_SWRSTDISABLE);
	//TI6678PCIE_SetPscState(dev, 0, PD3, LPSC_PCIE, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD4, LPSC_SRIO, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD5, LPSC_HYPER, PSC_SWRSTDISABLE);
	//TI6678PCIE_SetPscState(dev, 0, PD6, LPSC_RESERV, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD7, LPSC_MSMCRAM, PSC_SWRSTDISABLE);

	IPC_delay(5);

	for (int i = 0; i < 8; i++) {
		TI6678PCIE_SetBootAddrIpcgr(dev,i,0);
		U32 x = 0x778899AA;
		TI6678PCIE_ProcWriteMem(dev, i, 0x800000, &x, 4);
	}

	//Sleep(150);

	no_load_print = 1;

	for (int i = 0; i < 8; i++) {
		U32 entry;
		TI6678PCIE_ProcLoadMem(dev,i,localResetCode,sizeof(localResetCode),&entry);
		if( TI6678PCIE_SetBootAddrIpcgr(dev,i,entry) < 0)
		{
			BRDC_printf(_BRDC("<TI6678PCIE_ProcLocalReset> Core %d is not ready !!! \n"), i );
			Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("<TI6678PCIE_ProcLocalReset> Core %d is not ready !!! \n"), i );
			//printf("Core %d is not ready !!! \n", i);
			return -1;
		}
	}

	no_load_print = 0;

	IPC_delay(5);

	/* Enable all other modules */
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_EMIF16_SPI, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_TSIP, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_DEBUG, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_TETB_TRC, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_PA, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SGMII, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SA, PSC_ENABLE);
	//TI6678PCIE_SetPscStatdeve(dev, 0, PD3, LPSC_PCIE, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD4, LPSC_SRIO, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD5, LPSC_HYPER, PSC_ENABLE);
	//TI6678PCIE_SetPscState(dev, 0, PD6, LPSC_RESERV, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD7, LPSC_MSMCRAM, PSC_ENABLE);

	//Sleep(50);

	/* Local out of reset of all cores */

	TI6678PCIE_CoreLocalReset(dev, 0, PD8,  LPSC_C0_TIM0, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD9,  LPSC_C1_TIM1, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD10, LPSC_C2_TIM2, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD11, LPSC_C3_TIM3, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD12, LPSC_C4_TIM4, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD13, LPSC_C5_TIM5, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD14, LPSC_C6_TIM6, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD15, LPSC_C7_TIM7, LOC_RST_DEASSERT);

	IPC_delay(5);

	for (int i = 0; i < 8; i++) {
		U32 x;
		TI6678PCIE_ProcReadMem(dev, i, 0x800000, &x, 4);
		if(x!=0x33557799) {
			//BRDC_printf(_BRDC("<TI6678PCIE_ProcLocalReset> ###LocalReset ERROR Core %d \n"), i );
			Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("<TI6678PCIE_ProcLocalReset> ###LocalReset ERROR Core %d \n"), i );
			
			//printf("###LocalReset ERROR Core %d\n",i);
		}
	}

	return 0;
}

TI6678_DEVICE *TI6678PCIE_Open(int id)
{
	//if( id>=DevCnt) return NULL;
	return &tiDev[id];
}

//============== TI6678PCIE_ProcReadL2() =======================================
// read TI6678 L2 memory space
// dev  - device number (0..N)
// core - core number:	0..7 - core 0..7 L2 memory
// adr  - address TI6678 memory space (0x00800000..0x0087FFFF)
// dst  - addres HOST
// size - size of bytes
// -----------------------------------------------------------------------------
int	TI6678PCIE_ProcReadL2(TI6678_DEVICE *dev, U32 core, U32 adr, U32 *dst, long size)
{
	*(U32*)(dev->mem_pciereg_va+IB_OFFSET(1)) = 0x10800000+core*0x01000000; // core address
	Pause(2000);
	adr&=0x00FFFFFF;
	uintptr_t offset = adr-0x800000;

	{
		unsigned char *s,*d;
		s = (unsigned char*)(dev->mem_bar1_va+offset);
		d = (unsigned char*)(dst);
		memcpy(d,s,size);
	}
		
	return 0;
}

//============== TI6678PCIE_ProcWriteL2() =======================================
// write TI6678 L2 memory space
// dev  - device number (0..N)
// core - core number:	0..7 - core 0..7 L2 memory
// adr  - address TI6678 memory space (0x00800000..0x0087FFFF)
// src  - addres HOST
// size - size of bytes
// -----------------------------------------------------------------------------
int	TI6678PCIE_ProcWriteL2(TI6678_DEVICE *dev, U32 core, U32 adr, U32 *src, long size)
{
	*(U32*)(dev->mem_pciereg_va+IB_OFFSET(1)) = 0x10800000+core*0x01000000; // core address
	Pause(2000);
	adr&=0x00FFFFFF;
	uintptr_t offset = adr-0x800000;

	{
		unsigned char *s,*d;
		d = (unsigned char*)(dev->mem_bar1_va+offset);
		s = (unsigned char*)(src);
		memcpy(d,s,size);
	}
		
	return 0;
}


//============== TI6678PCIE_ProcReadArea() =======================================
// read TI6678 memory space 
// dev  - device number (0..N)
// core - core number:	0
// adr  - address TI6678 memory space 
// dst  - addres HOST
// size - size of bytes (<0x400000)
// -----------------------------------------------------------------------------
int	TI6678PCIE_ProcReadArea(TI6678_DEVICE *dev, U32 core, U32 adr, U32 *dst, long size)
{
	U32 save_base = *(U32*)(dev->mem_pciereg_va+IB_OFFSET(1));
	U32 _adr = adr;
	U32 _size = size;
	U32 *_dst = dst;
	S32 delta;

	do {
		// remap window
		U32 base = adr&0xFFF80000; // получаем базовый адрес дл€ окна в 512 кЅайт
		*(U32*)(dev->mem_pciereg_va+IB_OFFSET(1)) = base; 
		U32 offset = _adr-base;
		Pause(2000);

		delta = (offset+_size) - (0x80000);
		if(delta>0) {
			_size = 0x80000-offset;
			_adr += _size;
//			printf("Delta = %d\n",delta);
		}
		{
			unsigned char *s,*d;
			s = (unsigned char*)(dev->mem_bar1_va+offset);
			d = (unsigned char*)(_dst);
			memcpy(d,s,_size);
			_dst+=(_size/4);
		}
	} while(delta>0);

	return 0;
}

int	TI6678PCIE_ProcWriteArea(TI6678_DEVICE *dev, U32 core, U32 adr, U32 *src, long size)
{
	U32 save_base = *(U32*)(dev->mem_pciereg_va+IB_OFFSET(1));
	U32 _adr = adr;
	U32 _size = size;
	U32 *_src = src;

	S32 delta;

	do {
		// remap window
		//U32 base = adr&0xFFC00000; // получаем базовый адрес дл€ окна в 4 ћЅайта
		U32 base = _adr&0xFFF80000; // получаем базовый адрес дл€ окна в 512 кЅайт
		U32 offset = _adr-base;
		*(U32*)(dev->mem_pciereg_va+IB_OFFSET(1)) = base; 
		Pause(2000);

		delta = (offset+_size) - (0x80000);
		if(delta>0) {
			_size = 0x80000-offset;
			_adr += _size;
//			printf("Delta = %d\n",delta);
		}
		{
			unsigned char *s,*d;
			d = (unsigned char*)(dev->mem_bar1_va+offset);
			s = (unsigned char*)(_src);
			memcpy(d,s,_size);
			_src+=(_size/4);
		}
	} while(delta>0);

	return 0;
}

//============== TI6678PCIE_ProcReadMem() =======================================
// read TI6678 memory space
// dev  - device number
// node - node number
// adr  - address TI6678 memory space (0..0xFFFFFFFF)
// dst  - addres HOST
// size - size of bytes
// -----------------------------------------------------------------------------
int	TI6678PCIE_ProcReadMem(TI6678_DEVICE *dev, U32 core, U32 adr, U32 *dst, U32 size)
{
	U32 sig = adr&0xFFF00000;
	if(sig==L2_START) {	// L2 memory
		return TI6678PCIE_ProcReadL2(dev, core, adr, dst, size);
	}
	return TI6678PCIE_ProcReadArea(dev, core, adr, dst, size);
}

//============== TI6678PCIE_ProcWriteMem() =======================================
// read TI6678 memory space
// dev  - device number
// node - node number
// adr  - address TI6678 memory space (0..0xFFFFFFFF)
// dst  - addres HOST
// size - size of bytes
// -----------------------------------------------------------------------------
int	TI6678PCIE_ProcWriteMem(TI6678_DEVICE *dev, U32 core, U32 adr, U32 *src, U32 size)
{
	U32 sig = adr&0xFFF00000;
	if(sig==L2_START) {	// L2 memory 0 core
		return TI6678PCIE_ProcWriteL2(dev, core, adr, src, size);
	}
	return TI6678PCIE_ProcWriteArea(dev, core, adr, src, size);
}

//============== TI6678PCIE_ProcStart() =======================================
// start application TI6678
// dev  - device number
// core - core number
// entry - entry address
// -----------------------------------------------------------------------------
int	__TI6678PCIE_ProcStart(TI6678_DEVICE *dev, U32 core, U32 entry)
{
	U32 adr = 0x10000000+MAGIC_ADDR+(0x01000000*core);
	TI6678PCIE_ProcWriteMem(dev, core, adr, &entry, 4);
	return 0;
}

//static UCHAR	loadBufByte[0x200000];
//static U32	loadBuf[0x80000];
//static U32	checkBuf[0x80000];

//============== TI6678PCIE_ProcLoadBin() =======================================
// load bin application to TI6678
// dev  - device number
// node - node number
// file - binary file of format:
//		Entry Addr
//		Size of Block 1
//		Addr of Block 1
//		Data of Block 1
//		. . .
//		Size of Block N
//		Addr of Block N
//		Data of Block N
//		Size of Block = 0
// -----------------------------------------------------------------------------
int	TI6678PCIE_ProcLoadBin(TI6678_DEVICE *dev, U32 core, const BRDCHAR *fname, U32 *entry)
{

	// open binary file
	FILE	*fin = BRDC_fopen(fname, _BRDC("rb"));
	if(fin==NULL) {
		return -1;
	}

	UCHAR	*loadBufByte = dev->loadBufByte;

	// get size of file
	fseek(fin, 0, SEEK_END);
	long flen = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	//BRDC_printf(_BRDC("%s, file length = %ld\n"), fname, flen );
	Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("<TI6678PCIE_ProcLoadBin>: %s File Length = %ld\n"),fname,flen);

	// read all file to buffer
	fread(loadBufByte, 1, flen, fin);
	fclose(fin);

	switch(core) {
		case 7: TI6678PCIE_CoreLocalReset(dev, 0, PD15,  LPSC_C7_TIM7, LOC_RST_ASSERT); break;
		case 6: TI6678PCIE_CoreLocalReset(dev, 0, PD14,  LPSC_C6_TIM6, LOC_RST_ASSERT); break;
		case 5: TI6678PCIE_CoreLocalReset(dev, 0, PD13,  LPSC_C5_TIM5, LOC_RST_ASSERT); break;
		case 4: TI6678PCIE_CoreLocalReset(dev, 0, PD12,  LPSC_C4_TIM4, LOC_RST_ASSERT); break;
		case 3: TI6678PCIE_CoreLocalReset(dev, 0, PD11,  LPSC_C3_TIM3, LOC_RST_ASSERT); break;
		case 2: TI6678PCIE_CoreLocalReset(dev, 0, PD10,  LPSC_C2_TIM2, LOC_RST_ASSERT); break;
		case 1: TI6678PCIE_CoreLocalReset(dev, 0, PD9,  LPSC_C1_TIM1, LOC_RST_ASSERT); break;
		case 0: TI6678PCIE_CoreLocalReset(dev, 0, PD8,  LPSC_C0_TIM0, LOC_RST_ASSERT); break;
	};

	int ret = TI6678PCIE_ProcLoadMem(dev, core, loadBufByte, flen, entry);

	return ret;

}

//static	U32	load_adr[256];

//============== TI6678PCIE_ProcLoadMem() =======================================
// load application to TI6678 from memory array
// dev  - device number
// node - node number
// appBuf - aplication array address
// -----------------------------------------------------------------------------
int	TI6678PCIE_ProcLoadMem(TI6678_DEVICE *dev, U32 core, UCHAR *appBuf, U32 appsize, U32 *entry)
{
	U32		*loadBuf = (U32*)dev->loadBuf;
	U32		*checkBuf = (U32*)dev->checkBuf;

	// read all file to buffer
	for(U32 i=0;i<appsize/4;i++) {
		loadBuf[i] = appBuf[4*i];
		loadBuf[i] <<= 8;
		loadBuf[i] |= appBuf[4*i+1];
		loadBuf[i] <<= 8;
		loadBuf[i] |= appBuf[4*i+2];
		loadBuf[i] <<= 8;
		loadBuf[i] |= appBuf[4*i+3];
	}

	// read entry
	long dataCnt = 0;
	*entry = loadBuf[dataCnt];
	//printf( "BUS%d,DEV%d:<TI6678PCIE_ProcLoadMem>(core%d): Entry point = 0x%08X\n", dev->pcibus, dev->pcidev, core, loadBuf[dataCnt]);

	if(no_load_print == 0)
		//BRDC_printf( _BRDC("<TI6678PCIE_ProcLoadMem>: Entry point = 0x%08X\n") , loadBuf[dataCnt]);
		Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("<TI6678PCIE_ProcLoadMem>: Entry point = 0x%08X\n") , loadBuf[dataCnt]);

	dataCnt++;
	// load code to DSP
	U32 size;
	U32 addr;

//	for(U32 i=0;i<256;i++)	load_adr[i] = 0;

	//int	blk_cnt = 0;
	for(;;) {
		size =  loadBuf[dataCnt];
		if( size == 0) break;//continue;	// end of code
		dataCnt++;

		// ?alignment 4 bytes
		if ((size/4)*4 != size) {
			size = ((size/4)+1)*4;
		}
		addr = loadBuf[dataCnt];
		dataCnt++;
		//BRDC_printf(_BRDC("ProcLoadMem: addr = %8X size = %d first = %8X\n"), addr, size, loadBuf[dataCnt] );
		TI6678PCIE_ProcWriteMem(dev, core, addr, &loadBuf[dataCnt], size);
		for(U32 i=0;i<size/4;i++)
			TI6678PCIE_ProcReadMem(dev, core, addr+4*i, &checkBuf[i], 4);

		if(no_load_print == 0)
			//BRDC_printf( _BRDC("IS LOAD MEM (core=%d): ADDR=0x%08X     SIZE=0x%08X\n") ,core, addr, size);
			Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("IS LOAD MEM (core=%d): ADDR=0x%08X     SIZE=0x%08X\n") ,core, addr, size);

		if( memcmp(&checkBuf[0],&loadBuf[dataCnt],size) !=0 )
		{
			//BRDC_printf(_BRDC("###ERROR:\n") );
			Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("###ERROR:\n") );

			U32 i;
			for(i=0;i<size/4;i++) {
				if(checkBuf[i]!=loadBuf[dataCnt+i]) {
					//BRDC_printf( _BRDC("### (A=0x%08X) CHK 0x%08X != LD 0x%08X\n") ,addr+i*4, checkBuf[i], loadBuf[dataCnt+i]);
					Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("### (A=0x%08X) CHK 0x%08X != LD 0x%08X\n") ,addr+i*4, checkBuf[i], loadBuf[dataCnt+i]);

					TI6678PCIE_ProcReadMem(dev, core, addr+i*4, &checkBuf[i], 4);
					//BRDC_printf( _BRDC("###2 (A=0x%08X) CHK 0x%08X != LD 0x%08X\n") ,addr+i*4, checkBuf[i], loadBuf[dataCnt+i]);
					Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("###2 (A=0x%08X) CHK 0x%08X != LD 0x%08X\n") ,addr+i*4, checkBuf[i], loadBuf[dataCnt+i]);
				} else {
					//BRDC_printf( _BRDC("+++ (A=0x%08X) CHK 0x%08X == LD 0x%08X\n") ,addr+i*4, checkBuf[i], loadBuf[dataCnt+i]);
					//Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("+++ (A=0x%08X) CHK 0x%08X == LD 0x%08X\n") ,addr+i*4, checkBuf[i], loadBuf[dataCnt+i]);
				}
			}
			return -1;
		}

		dataCnt+=(size/4);
	}

	return 0;
}

//============== TI6678PCIE_CheckInterrupt() =======================================
// Check interrupt from TI6678 
// dev  - device number
// -----------------------------------------------------------------------------
int	TI6678PCIE_CheckInterrupt(TI6678_DEVICE *dev)
{
	int ret = *(U32*)(dev->mem_pciereg_va+EP_IRQ_STATUS); 
	return ret;
}

//============== TI6678PCIE_ClrInterrupt() =======================================
// Clr interrupt from TI6678 
// dev  - device number
// -----------------------------------------------------------------------------
int	TI6678PCIE_ClrInterrupt(TI6678_DEVICE *dev)
{
	*(U32*)(dev->mem_pciereg_va+EP_IRQ_CLR) = 1; 
	return 0;
}

//============== TI6678PCIE_EnableInterrupt() =======================================
// Enable interrupt from TI6678 
// dev  - device number
// -----------------------------------------------------------------------------
int	TI6678PCIE_EnableInterrupt(TI6678_DEVICE *dev)
{
	*(U32*)(dev->mem_pciereg_va+LEGACY_A_IRQ_ENABLE_SET) = 1; 
	return 0;
}

//============== TI6678PCIE_DisableInterrupt() =======================================
// Disable interrupt from TI6678 
// dev  - device number
// -----------------------------------------------------------------------------
int	TI6678PCIE_DisableInterrupt(TI6678_DEVICE *dev)
{
	*(U32*)(dev->mem_pciereg_va+LEGACY_A_IRQ_ENABLE_CLR) = 1; 
	return 0;
}

//============== TI6678PCIE_SendInterrupt() =======================================
// Send interrupt to TI6678 ( Interrupt A)
// dev  - device number
// -----------------------------------------------------------------------------
int	TI6678PCIE_SendInterrupt(TI6678_DEVICE *dev, U32 node)
{
	int intId = PCIEXpress_Legacy_INTA + (node&3);

	if( node<4) {
		*(U32*)(dev->mem_bar2_va+CIC_SYS_STAT_IDX_SET) = intId; 
	} else {
		*(U32*)(dev->mem_bar2_va+0x4000+CIC_SYS_STAT_IDX_SET) = intId; 
	}
	Pause(2000);

 	return 0;
}

//============== TI6678PCIE_SendInterruptMSI() =======================================
// Send interrupt to TI6678 ( Interrupt A)
// dev  - device number
// -----------------------------------------------------------------------------
int	TI6678PCIE_SendInterruptMSI(TI6678_DEVICE *dev)
{
	U32 x = *(U32*)(dev->mem_pciereg_va+0x50);
	*(U32*)(dev->mem_pciereg_va+0x50) = x|0x100; // MSI Enabled

	*(U32*)(dev->mem_pciereg_va+0x54) = 0; // MSI0
	*(U32*)(dev->mem_pciereg_va+0x58) = 0; // MSI0
	*(U32*)(dev->mem_pciereg_va+0x5C) = 0; // MSI0

	return 0;
}



//
////============== TI6678PCIE_ProcSoftReset() =======================================
//// Soft Reset to TI6678 
//// dev  - device number
//// -----------------------------------------------------------------------------
#define PLL_CONTROLLER_ADDRESS (0x02310000)
int	TI6678PCIE_ProcSoftReset(TI6678_DEVICE *dev, int state)
{
	//return 0;
	// Temporarily re-map IB region to CIC0 controller registers 
	*(U32*)(dev->mem_pciereg_va+IB_OFFSET(1)) = PLL_CONTROLLER_ADDRESS;
	Pause(2000);

	//U32 rstype = *(U32*)(dev->mem_bar1_va+0xE4); // RSTYPE
	//U32 rstctrl = *(U32*)(dev->mem_bar1_va+0xE8); // RSTCTRL
	//U32 rstcfg = *(U32*)(dev->mem_bar1_va+0xEC); // RSTCFG
	//U32 rsiso = *(U32*)(dev->mem_bar1_va+0xF0); // RSISO

//	*(U32*)(dev->mem_bar1_va+0xE8) = 0x00015A69; // RSTCTRL
//	*(U32*)(dev->mem_bar1_va+0xEC) = rstcfg|0x0000; // RSTCTRL

	if(state==0) {
		//*(U32*)(dev->mem_bar1_va+0xE8) = x&~(0x10000); // RSTCTRL
		*(U32*)(dev->mem_bar1_va+0xE8) = 0x5A69; // RSTCTRL
		*(U32*)(dev->mem_bar1_va+0xE8) = 0x5A69; // RSTCTRL
	} else {
		*(U32*)(dev->mem_bar1_va+0xE8) = 0x5A69; // RSTCTRL
		*(U32*)(dev->mem_bar1_va+0xE8) = 0x15A69; // RSTCTRL
	}

	return 0;
}

////============== TI6678PCIE_ReInit() =======================================
//// Initialization after Reset to TI6678 
//// dev  - device number
//// -----------------------------------------------------------------------------
int	TI6678PCIE_ReInit(TI6678_DEVICE *dev)
{
	// configure IB
	*(U32*)(dev->mem_pciereg_va+IB_BAR(0)) = 0;
	*(U32*)(dev->mem_pciereg_va+IB_START_LO(0)) = dev->mem_pciereg;	// предполагаем, что BAR0 (mem_pciereg) 32-разр€дный и в 64-битных системах
	*(U32*)(dev->mem_pciereg_va+IB_START_HI(0)) = 0;
	*(U32*)(dev->mem_pciereg_va+IB_OFFSET(0)) = PCIE_BASE_ADDRESS;
	Pause(2000);

	*(U32*)(dev->mem_pciereg_va+IB_BAR(1)) = 1;
	*(U32*)(dev->mem_pciereg_va+IB_START_LO(1)) = dev->mem_bar1;		// предполагаем, что BAR1 32-разр€дный и в 64-битных системах
	*(U32*)(dev->mem_pciereg_va+IB_START_HI(1)) = 0;
	*(U32*)(dev->mem_pciereg_va+IB_OFFSET(1)) = L2_START+(1<<28); // core 0 address
	Pause(2000);

	*(U32*)(dev->mem_pciereg_va+IB_BAR(2)) = 2;
	*(U32*)(dev->mem_pciereg_va+IB_START_LO(2)) = dev->mem_bar2;		// предполагаем, что BAR2 32-разр€дный и в 64-битных системах
	*(U32*)(dev->mem_pciereg_va+IB_START_HI(2)) = 0;
	*(U32*)(dev->mem_pciereg_va+IB_OFFSET(2)) = CIC0_BASE_ADDRESS; // interrupt controler address
	Pause(2000);

	*(U32*)(dev->mem_pciereg_va+IB_BAR(3)) = 3;
	*(U32*)(dev->mem_pciereg_va+IB_START_LO(3)) = dev->mem_bar3;			// предполагаем, что BAR3 32-разр€дный и в 64-битных системах
	*(U32*)(dev->mem_pciereg_va+IB_START_HI(3)) = 0;
	*(U32*)(dev->mem_pciereg_va+IB_OFFSET(3)) = MSM_START; // DDR3 memory
	Pause(2000);

	return 0;
}

int	TI6678PCIE_ProcLocalReset(TI6678_DEVICE *dev, U32 core)
{
	if(core!=-1) {
		TI6678PCIE_CoreLocalReset(dev, 0, PD8,  LPSC_C0_TIM0+core, LOC_RST_ASSERT);
		IPC_delay(10);
		TI6678PCIE_CoreLocalReset(dev, 0, PD8,  LPSC_C0_TIM0+core, LOC_RST_DEASSERT);
		return 0;
	}

//---------- FIRST LOCAL RESET
	/* Local reset of all cores */
	TI6678PCIE_CoreLocalReset(dev, 0, PD8,  LPSC_C0_TIM0, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD9,  LPSC_C1_TIM1, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD10, LPSC_C2_TIM2, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD11, LPSC_C3_TIM3, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD12, LPSC_C4_TIM4, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD13, LPSC_C5_TIM5, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD14, LPSC_C6_TIM6, LOC_RST_ASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD15, LPSC_C7_TIM7, LOC_RST_ASSERT);

	IPC_delay(5);

	for (int i = 0; i < 8; i++) {
		TI6678PCIE_SetBootAddrIpcgr(dev,i,0);
	}

	U32 x = 0x0001E003;
	U32 y = 0x0001E003;
	for (int i = 0; i < 8; i++) {
		TI6678PCIE_ProcWriteMem(dev, 0, 0x0C000000+4*i, &x, 4);
		TI6678PCIE_ProcReadMem(dev, 0, 0x0C000000+4*i, &y, 4);
	}

	for (int i = 0; i < 8; i++) {
		if( TI6678PCIE_SetBootAddrIpcgr(dev,i,0x0C000000) < 0)
		{
			//BRDC_printf( _BRDC("<TI6678PCIE_ProcLocalReset>: Core %d is not ready !!! \n") , i);
			Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("<TI6678PCIE_ProcLocalReset>: Core %d is not ready !!! \n") , i);
			//printf("Core %d is not ready !!! \n", i);
			return -1;
		}
	}

	IPC_delay(5);

	/* Enable all other modules */
	TI6678PCIE_CoreLocalReset(dev, 0, PD8,  LPSC_C0_TIM0, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD9,  LPSC_C1_TIM1, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD10, LPSC_C2_TIM2, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD11, LPSC_C3_TIM3, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD12, LPSC_C4_TIM4, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD13, LPSC_C5_TIM5, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD14, LPSC_C6_TIM6, LOC_RST_DEASSERT);
	TI6678PCIE_CoreLocalReset(dev, 0, PD15, LPSC_C7_TIM7, LOC_RST_DEASSERT);

	IPC_delay(5);

	for (int i = 0; i < 8; i++) {
		TI6678PCIE_SetBootAddrIpcgr(dev,i,0);
	}

	/* Disable all other modules */
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_EMIF16_SPI, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_TSIP, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_DEBUG, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_TETB_TRC, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SA, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SGMII, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_PA, PSC_SWRSTDISABLE);
	//TI6678PCIE_SetPscState(dev, 0, PD3, LPSC_PCIE, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD4, LPSC_SRIO, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD5, LPSC_HYPER, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD6, LPSC_RESERV, PSC_SWRSTDISABLE);
	TI6678PCIE_SetPscState(dev, 0, PD7, LPSC_MSMCRAM, PSC_SWRSTDISABLE);
	
	IPC_delay(5);
	
	TI6678PCIE_SetPscState(dev, 0, PD7, LPSC_MSMCRAM, PSC_ENABLE);
	IPC_delay(5);
    TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_EMIF16_SPI, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD0, LPSC_TSIP, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_DEBUG, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD1, LPSC_TETB_TRC, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_PA, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SGMII, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD2, LPSC_SA, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD3, LPSC_PCIE, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD4, LPSC_SRIO, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD5, LPSC_HYPER, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD6, LPSC_RESERV, PSC_ENABLE);
	TI6678PCIE_SetPscState(dev, 0, PD7, LPSC_MSMCRAM, PSC_ENABLE);

	IPC_delay(5);

	return 0;
}

int	TI6678PCIE_ProcStart(TI6678_DEVICE *dev, U32 core, U32 entry)
{
	switch(core) {
	case 0:	TI6678PCIE_CoreLocalReset(dev, 0, PD8,  LPSC_C0_TIM0, LOC_RST_ASSERT); break;
	case 1:	TI6678PCIE_CoreLocalReset(dev, 0, PD9,  LPSC_C1_TIM1, LOC_RST_ASSERT); break;
	case 2:	TI6678PCIE_CoreLocalReset(dev, 0, PD10, LPSC_C2_TIM2, LOC_RST_ASSERT); break;
	case 3:	TI6678PCIE_CoreLocalReset(dev, 0, PD11, LPSC_C3_TIM3, LOC_RST_ASSERT); break;
	case 4:	TI6678PCIE_CoreLocalReset(dev, 0, PD12, LPSC_C4_TIM4, LOC_RST_ASSERT); break;
	case 5:	TI6678PCIE_CoreLocalReset(dev, 0, PD13, LPSC_C5_TIM5, LOC_RST_ASSERT); break;
	case 6:	TI6678PCIE_CoreLocalReset(dev, 0, PD14, LPSC_C6_TIM6, LOC_RST_ASSERT); break;
	case 7:	TI6678PCIE_CoreLocalReset(dev, 0, PD15, LPSC_C7_TIM7, LOC_RST_ASSERT); break;
	}

	if(TI6678PCIE_SetBootAddrIpcgr(dev,core,entry) < 0)
	{
		{
			//BRDC_printf( _BRDC("<TI6678PCIE_ProcStart>: Core %d is not ready !!! \n") , core);
			Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("<TI6678PCIE_ProcStart>: Core %d is not ready !!! \n") , core);
			//printf("Core %d is not ready !!! \n", i);
			return -1;
		}
	}

	IPC_delay(5);

	switch(core) {
	case 0:	TI6678PCIE_CoreLocalReset(dev, 0, PD8,  LPSC_C0_TIM0, LOC_RST_DEASSERT); break;
	case 1:	TI6678PCIE_CoreLocalReset(dev, 0, PD9,  LPSC_C1_TIM1, LOC_RST_DEASSERT); break;
	case 2:	TI6678PCIE_CoreLocalReset(dev, 0, PD10, LPSC_C2_TIM2, LOC_RST_DEASSERT); break;
	case 3:	TI6678PCIE_CoreLocalReset(dev, 0, PD11, LPSC_C3_TIM3, LOC_RST_DEASSERT); break;
	case 4:	TI6678PCIE_CoreLocalReset(dev, 0, PD12, LPSC_C4_TIM4, LOC_RST_DEASSERT); break;
	case 5:	TI6678PCIE_CoreLocalReset(dev, 0, PD13, LPSC_C5_TIM5, LOC_RST_DEASSERT); break;
	case 6:	TI6678PCIE_CoreLocalReset(dev, 0, PD14, LPSC_C6_TIM6, LOC_RST_DEASSERT); break;
	case 7:	TI6678PCIE_CoreLocalReset(dev, 0, PD15, LPSC_C7_TIM7, LOC_RST_DEASSERT); break;
	}

	IPC_delay(5);

	return 0;
}

//
// End of file
//
