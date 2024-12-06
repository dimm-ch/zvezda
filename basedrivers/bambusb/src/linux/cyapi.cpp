#include "cyapi.h"

//libusb_context* g_pCtx;
HANDLE	hEventThread;
#ifdef __IPC_WIN__
LARGE_INTEGER xTimeStart, xTimeEnd, xTimeDelta, xFreq;
#endif
#define MAX_FWIMG_SIZE		(512 * 1024)	// Maximum size of the firmware binary.
#define MAX_WRITE_SIZE		(2 * 1024)	// Max. size of data that can be written through one vendor command.

#define I2C_PAGE_SIZE		(64)		// Page size for I2C EEPROM.
#define I2C_SLAVE_SIZE		(64 * 1024)	// Max. size of data that can fit on one EEPROM address.

#define SPI_PAGE_SIZE		(256)		// Page size for SPI flash memory.
#define SPI_SECTOR_SIZE		(64 * 1024)	// Sector size for SPI flash memory.

struct rThreadParams
{
	libusb_context* pContext;
	UINT32* nStop;
	int* nTransfer;
};

//IPC_handle m_EventHandleMutex;

#ifdef WIN32
unsigned __stdcall event_handling_thread(void* pParams)
#else
unsigned event_handling_thread(void* pParams)
#endif
{
	rThreadParams* params = (rThreadParams*)pParams;
	UINT32* isStop = params->nStop;
	libusb_context *pCtx = params->pContext;
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#else
    pthread_t thId = pthread_self();
    pthread_attr_t thAttr;
    int policy = 0;
    int max_prio_for_policy = 0;
    pthread_attr_init(&thAttr);
    pthread_attr_getschedpolicy(&thAttr, &policy);
    max_prio_for_policy = sched_get_priority_max(policy);
    pthread_setschedprio(thId, max_prio_for_policy);
    pthread_attr_destroy(&thAttr);
#endif
	timeval tOut;
    tOut.tv_sec = 0;
    tOut.tv_usec = 10000;
	int nCompl = 0;		
	//BRDC_printf(_BRDC("start handling\n"));
    while (*isStop != 0)
		libusb_handle_events_timeout_completed(pCtx, &tOut, &nCompl);	//0 on success
	//IPC_releaseMutex(m_EventHandleMutex);
	//m_EventHandleMutex = 0;
	(*isStop)++;
	return 0;
}

CCyUSBDevice::CCyUSBDevice(HANDLE hnd, /*GUID guid,*/ bool bOpen)
{
        //FILE *pFile = fopen("events.txt", "w");
        //fclose(pFile);
#ifdef __IPC_WIN__
	QueryPerformanceFrequency(&xFreq);
	QueryPerformanceCounter(&xTimeStart);
#endif
	libusb_device **apDevs;
	libusb_context *pContx = NULL;
    const libusb_version *rV;
	int r;
	ssize_t cnt;
    r = libusb_init(&pContx);
	libusb_set_debug(pContx, LIBUSB_LOG_LEVEL_NONE);
    rV = libusb_get_version();
	cnt = libusb_get_device_list(pContx, &apDevs);
	Devices = 0;
	ctx = pContx;
	apDevices = (libusb_device**) malloc(256 * cnt);
	for (int i = 0; i < cnt; i++)
	{
		libusb_device_descriptor hDesc;
		int r = libusb_get_device_descriptor(apDevs[i], &hDesc);
		if (r == 0)
			if ((hDesc.idVendor == 0x4953)||(hDesc.idVendor == 0x04b4))//we need only InSys or Cypress devices
			{
				apDevices[Devices] = apDevs[i];
				Devices++;
			}
	}
	hEventThread = 0;
	USBCfgs[0] = new CCyUSBConfig();
	EndPoints = USBCfgs[0]->Interfaces[0]->EndPoints;
	isOpened = false;
#ifdef __IPC_WIN__
	QueryPerformanceCounter(&xTimeEnd);
	xTimeDelta.QuadPart = (xTimeEnd.QuadPart - xTimeStart.QuadPart) * 1000 / xFreq.QuadPart;
	//pFile = fopen("events.txt", "a");
	//fprintf(pFile, "\nlibusb init in %lld us\n", xTimeDelta.QuadPart);
	//fclose(pFile);
#endif
}

CCyUSBDevice::~CCyUSBDevice()
{
	if (this->IsOpen())
		this->Close();
        //libusb_free_device_list(apDevices, 1);	
	libusb_exit(ctx);
}

UCHAR CCyUSBDevice::DeviceCount()
{
	libusb_device **apDevs;	
	int cnt = libusb_get_device_list(ctx, &apDevs);
	Devices = 0;
	for (int i = 0; i < cnt; i++)
	{
		libusb_device_descriptor hDesc;
		int r = libusb_get_device_descriptor(apDevs[i], &hDesc);
		if (r == 0)
			if ((hDesc.idVendor == 0x4953)||(hDesc.idVendor == 0x04b4))//we need only InSys or Cypress devices
			{
				apDevices[Devices] = apDevs[i];
				Devices++;
			}
	}
	return Devices;
}

bool CCyUSBDevice::Open(UCHAR dev)
{
#ifdef __IPC_WIN__
	QueryPerformanceCounter(&xTimeStart);
#endif
	if (this->IsOpen())
		this->Close();
	libusb_device_descriptor hDesc;
	int r = libusb_get_device_descriptor(apDevices[dev], &hDesc);
	if (r < 0)
		return false;
	r = libusb_open(apDevices[dev], &hDevice);
	if (r == 0)
	{
                libusb_set_auto_detach_kernel_driver(hDevice, 1);
		isOpened = true;		
		unsigned char stringDesc[256];
		r = libusb_get_string_descriptor(hDevice, hDesc.iSerialNumber, 0, stringDesc, 32);
		BcdDevice = hDesc.bcdDevice;
		memcpy(SerialNumber, stringDesc+2, stringDesc[0]-2);
		SerialNumber[(stringDesc[0] / 2) - 1] = 0;
#ifdef __IPC_LINUX__
		wchar_t wSerial[16] = {0};
		for (int i = 0; i < stringDesc[0] - 2; i++)
			wSerial[i] = stringDesc[2*i + 2];
		wcscpy(SerialNumber, wSerial);
#endif
		r = libusb_get_string_descriptor(hDevice, hDesc.iProduct, 0, stringDesc, 32);		
		memcpy(DeviceName, stringDesc + 2, stringDesc[0] - 2);
		for (int i = 1; i < (stringDesc[0]) - 1; i++)
		{
			for (int j = i; j < (stringDesc[0]) - 2; j++)
				DeviceName[j] = DeviceName[j + 1];
		}
		DeviceName[(stringDesc[0] / 2) - 1] = 0;
		if (r <= 0)
		{
			const char* errName = libusb_error_name(r);
			r++;
		}
		hCfgDesc=0;
		libusb_get_config_descriptor(apDevices[dev], 0, &hCfgDesc);
		const libusb_interface *pIntfc;
		const libusb_interface_descriptor *pIntfcDesc;
		const libusb_endpoint_descriptor *pEpDesc;
		if (hCfgDesc != 0)
		{		
			pIntfc = &(hCfgDesc->interface[0]);
			pIntfcDesc = &(pIntfc->altsetting[0]);
			USBCfgs[0]->Interfaces[0]->bInterfaceNumber = pIntfcDesc->bInterfaceNumber;
			AltInterfaces = pIntfc->num_altsetting;
			for (int i = 1; i <= pIntfcDesc->bNumEndpoints; i++)
			{//fill endpoint descriptors
				pEpDesc = &pIntfcDesc->endpoint[i-1];
				USBCfgs[0]->Interfaces[0]->EndPoints[i]->Address = pEpDesc->bEndpointAddress;
				USBCfgs[0]->Interfaces[0]->EndPoints[i]->Attributes = pEpDesc->bmAttributes;
				USBCfgs[0]->Interfaces[0]->EndPoints[i]->Interval = pEpDesc->bInterval;
				USBCfgs[0]->Interfaces[0]->EndPoints[i]->MaxPktSize = pEpDesc->wMaxPacketSize;
				USBCfgs[0]->Interfaces[0]->EndPoints[i]->DscType = pEpDesc->bDescriptorType;
				USBCfgs[0]->Interfaces[0]->EndPoints[i]->hDevice = hDevice;
				USBCfgs[0]->Interfaces[0]->EndPoints[i]->TimeOut = 1000;
				if (USBCfgs[0]->Interfaces[0]->EndPoints[i]->Address > 0x80)
					USBCfgs[0]->Interfaces[0]->EndPoints[i]->bIn = true;
				else
					USBCfgs[0]->Interfaces[0]->EndPoints[i]->bIn = false;
				USBCfgs[0]->Interfaces[0]->EndPoints[i]->NtStatus = 0;
				USBCfgs[0]->Interfaces[0]->EndPoints[i]->UsbdStatus = 0;
			}
			ControlEndPt = new CCyControlEndPoint();
			ControlEndPt->Address = 0;
			ControlEndPt->Attributes = 0;
			ControlEndPt->MaxPktSize = 512;
			ControlEndPt->Interval = 255;
			ControlEndPt->hDevice = hDevice;
			USBCfgs[0]->Interfaces[0]->EndPoints[0] = ControlEndPt;
		}
		ProductID = hDesc.idProduct;
		VendorID = hDesc.idVendor;
        r = libusb_claim_interface(hDevice, 0);
        r = libusb_set_interface_alt_setting(hDevice, 0, 0);
		IntfcIndex = 0;
		NtStatus = 0;
		UsbdStatus = 0;
		BRDCHAR threadName[128]={0}, pStr[16]={0};
		BRDC_strcat(threadName, _BRDC("libusb_event_handling_thread_"));
#ifdef __IPC_WIN__
		BRDC_strcat(threadName, BRDC_itoa(_wtoi(SerialNumber), pStr, 10));
#else
		BRDC_strcat(threadName, BRDC_itoa(wcstol(SerialNumber, 0, 10), pStr, 10));
#endif
		if (hEventThread == 0)
		{
			//nStopServThread = 0;
			//IPC_delay(250);

			nStopServThread = 1;
			rThreadParams params;
			params.pContext = ctx;
			params.nStop = &nStopServThread;
			for (int j = 0; j < 100; j++)
			{
				hEventThread = //(HANDLE)_beginthreadex(NULL, 0, &ThreadOutput, &pThreadParam, 0, &m_pStreamDscr[i]->threadID);
					IPC_createThread(threadName, (thread_func*)&event_handling_thread, (void*)&params);
				if (hEventThread != 0)
					break;
			}
		}
		IPC_delay(250);
        if (r < 0)
			return false;
		else
		{
#ifdef __IPC_WIN__
					QueryPerformanceCounter(&xTimeEnd);
					xTimeDelta.QuadPart = (xTimeEnd.QuadPart - xTimeStart.QuadPart) * 1000 / xFreq.QuadPart;
					/*FILE *pFile = fopen("events.txt", "a");
					fprintf(pFile, "open device in %lld us\n", xTimeDelta.QuadPart);
					fclose(pFile);*/
#endif
			return true;
		}
	}
	else
		return false;
}

bool CCyUSBDevice::IsOpen()
{
	return isOpened;
}

UCHAR CCyUSBDevice::AltIntfc(void)
{
	return IntfcIndex;
}

UCHAR CCyUSBDevice::AltIntfcCount()
{
	return AltInterfaces;
}

bool CCyUSBDevice::SetAltIntfc(UCHAR alt)
{
	int r;
	r = libusb_set_interface_alt_setting(hDevice, 0, alt);
	
	IPC_delay(1000);
	if (r != 0)
		return false;
	else
	{
		IntfcIndex = alt;
		const libusb_interface *pIntfc;
		const libusb_interface_descriptor *pIntfcDesc;
		const libusb_endpoint_descriptor *pEpDesc;		
		pIntfc = &(hCfgDesc->interface[0]);
		pIntfcDesc = &(pIntfc->altsetting[alt]);		
		for (int i = 1; i <= pIntfcDesc->bNumEndpoints; i++)
		{//fill endpoint descriptors
			pEpDesc = &pIntfcDesc->endpoint[i - 1];
			USBCfgs[0]->Interfaces[0]->EndPoints[i]->Address = pEpDesc->bEndpointAddress;
			USBCfgs[0]->Interfaces[0]->EndPoints[i]->Attributes = pEpDesc->bmAttributes;
			USBCfgs[0]->Interfaces[0]->EndPoints[i]->Interval = pEpDesc->bInterval;
			USBCfgs[0]->Interfaces[0]->EndPoints[i]->MaxPktSize = pEpDesc->wMaxPacketSize;
			USBCfgs[0]->Interfaces[0]->EndPoints[i]->DscType = pEpDesc->bDescriptorType;
			USBCfgs[0]->Interfaces[0]->EndPoints[i]->hDevice = hDevice;
			USBCfgs[0]->Interfaces[0]->EndPoints[i]->TimeOut = 1000;
			if (USBCfgs[0]->Interfaces[0]->EndPoints[i]->Address > 0x80)
				USBCfgs[0]->Interfaces[0]->EndPoints[i]->bIn = true;
			else
				USBCfgs[0]->Interfaces[0]->EndPoints[i]->bIn = false;
			USBCfgs[0]->Interfaces[0]->EndPoints[i]->NtStatus = 0;
			USBCfgs[0]->Interfaces[0]->EndPoints[i]->UsbdStatus = 0;
			r = libusb_clear_halt(hDevice, i);
		}
		ControlEndPt->Address = 0;
		ControlEndPt->Attributes = 0;
		ControlEndPt->MaxPktSize = 512;
		ControlEndPt->Interval = 255;
		ControlEndPt->hDevice = hDevice;
		r = libusb_clear_halt(hDevice, 0);
		return true;
	}
}

void CCyUSBDevice::Close()
{
#ifdef __IPC_WIN__
	QueryPerformanceCounter(&xTimeStart);
#endif
	nStopServThread = 0;
	for (int i = 0; i < 200; i++)
		if (nStopServThread >= 2)
			break;
		else
			IPC_delay(10);
	IPC_deleteThread(hEventThread);
	hEventThread = 0;
	libusb_release_interface(hDevice, 0);
	libusb_close(hDevice);
	/*if (m_EventHandleMutex != 0)
		IPC_releaseMutex(m_EventHandleMutex);*/
	isOpened = false;
#ifdef __IPC_WIN__
	QueryPerformanceCounter(&xTimeEnd);
	xTimeDelta.QuadPart = (xTimeEnd.QuadPart - xTimeStart.QuadPart) * 1000 / xFreq.QuadPart;
	/*FILE *pFile = fopen("events.txt", "a");
	fprintf(pFile, "close device in %lld us\n", xTimeDelta.QuadPart);
	fclose(pFile);*/
#endif
}

CCyUSBConfig::CCyUSBConfig()
{
	for (int i = 0; i < MAX_INTERFACES; i++)
		Interfaces[i] = new CCyUSBInterface(0, NULL, 0);
}

CCyUSBInterface::CCyUSBInterface(HANDLE handle, PUSB_INTERFACE_DESCRIPTOR pIntfcDescriptor, UCHAR usb30Dummy)
{
	EndPoints[0] = new CCyControlEndPoint();
	for (int i = 1; i < MAX_ENDPTS; i++)
		EndPoints[i] = new CCyBulkEndPoint();
}

CCyControlEndPoint::CCyControlEndPoint()
{
}

CCyControlEndPoint::CCyControlEndPoint(CCyControlEndPoint& ept)
{

}

bool CCyControlEndPoint::XferData(PUCHAR buf, LONG &len, CCyIsoPktInfo* pktInfos)
{
	int res = 0;
	if (this->Direction == DIR_TO_DEVICE)
		res = libusb_control_transfer(hDevice, (this->ReqType << 5) & 0x7F, this->ReqCode, this->Value, this->Index, buf, len, this->TimeOut); 
	else
		res = libusb_control_transfer(hDevice, (this->ReqType << 5) | 0x80, this->ReqCode, this->Value, this->Index, buf, len, this->TimeOut);
	
	if (res >= 0)
		return true;
	else
		return false;
}

PUCHAR CCyControlEndPoint::BeginDataXfer(PUCHAR buf, LONG len, OVERLAPPED * ov)
{
	return 0;
}

CCyUSBEndPoint::CCyUSBEndPoint()
{
}

PUCHAR CCyUSBEndPoint::BeginBufferedXfer(PUCHAR buf, LONG bufLen, OVERLAPPED *ov)
{
	return 0;
}

PUCHAR CCyUSBEndPoint::BeginDirectXfer(PUCHAR buf, LONG bufLen, OVERLAPPED *ov)
{
	return 0;
}

bool CCyUSBEndPoint::XferData(PUCHAR buf, LONG &len, CCyIsoPktInfo *pktInfos)
{
	int nActLen = 0, r = 0;
	r = libusb_bulk_transfer(hDevice, this->Address, buf, len, &nActLen, this->TimeOut);
	len = nActLen;
	if (r == 0)
		return true;
	else
		return false;
}

bool CCyUSBEndPoint::WaitForXfer(OVERLAPPED *ov, ULONG tOut)
{
#ifdef __IPC_WIN__
	QueryPerformanceCounter(&xTimeEnd);
	xTimeDelta.QuadPart = (xTimeEnd.QuadPart - xTimeStart.QuadPart) * 1000 / xFreq.QuadPart;

	//if (this->Address == 1)
	//{
	//	FILE *pFile = fopen("events.txt", "a");
	//	fprintf(pFile, "begin wait transfer 0x%X for %lld us\n", ov->hEvent, xTimeEnd.QuadPart);
	//	fclose(pFile);
	//}
#endif        
	IPC_TIMEVAL tStart, tCur;
	IPC_getTime(&tStart);
	IPC_getTime(&tCur);
	while (IPC_getDiffTime(&tStart, &tCur) < tOut)
	{
#ifdef __IPC_WIN__
		if (WaitForSingleObject(ov->hEvent, 0) == 0)
		{
			SetEvent(ov->hEvent);
			break;
		}
		IPC_getTime(&tCur);
#else
		if (ov->Internal == (ULONG_PTR)1)
		{
			break;
		}
#endif
		
    }
#ifdef __IPC_WIN__
	if (WaitForSingleObject(ov->hEvent, 0) == 0)
	{
		SetEvent(ov->hEvent);
		QueryPerformanceCounter(&xTimeEnd);
		xTimeDelta.QuadPart = (xTimeEnd.QuadPart - xTimeStart.QuadPart) * 1000 / xFreq.QuadPart;	
#else
    if (ov->Internal == (ULONG_PTR)1)
	{
#endif
    
		/*FILE *pFile = fopen("events.txt", "a");
		fprintf(pFile, "wait transfer success ep 0x%X, event 0x%X\n", this->Address, ov->hEvent);
		fclose(pFile);*/				
		//if (this->Address == 1)
		//{
		//	FILE *pFile = fopen("events.txt", "a");
		//	fprintf(pFile, "wait transfer 0x%X for %lld us\n", ov->hEvent, xTimeEnd.QuadPart);
		//	fclose(pFile);
		//}

		return true;
	}
	else
	{
		/*if (x == 0xFFFFFFFF)
			r = GetLastError();*/
        /*FILE *pFile = fopen("events.txt", "a");
        fprintf(pFile, "wait transfer fail ep 0x%X, event 0x%X waitErr 0x%X\n", this->Address, ov->hEvent, x);
        fclose(pFile);*/
#ifdef __IPC_WIN__
		QueryPerformanceCounter(&xTimeEnd);
		xTimeDelta.QuadPart = (xTimeEnd.QuadPart - xTimeStart.QuadPart) * 1000 / xFreq.QuadPart;		
		//if (this->Address == 1)
		//{
		//	FILE *pFile = fopen("events.txt", "a");
		//	fprintf(pFile, "fail wait transfer 0x%X after %lld us\n", ov->hEvent, xTimeEnd.QuadPart);
		//	fclose(pFile);
		//}		
#endif
		return false;
	}
}

bool CCyUSBEndPoint::FinishDataXfer(PUCHAR buf, LONG &len, OVERLAPPED* ov, PUCHAR pXmitBufm, CCyIsoPktInfo *pktInfos)
{
    libusb_transfer *rBuf = (libusb_transfer *)pXmitBufm;
	libusb_transfer_status status = rBuf->status;
    /*FILE *pFile = fopen("events.txt", "a");
    fprintf(pFile, "finish transfer ep 0x%X, event 0x%X\n", this->Address, ov->hEvent);
    fclose(pFile);*/
#ifdef __IPC_WIN__
	if ((status == LIBUSB_TRANSFER_COMPLETED) && (WaitForSingleObject(ov->hEvent, 0) == 0))
#else
	if ((status == LIBUSB_TRANSFER_COMPLETED)&&(ov->Internal == (ULONG_PTR)1))
#endif
	{
		libusb_free_transfer(rBuf);
#ifdef __IPC_WIN__
		QueryPerformanceCounter(&xTimeEnd);
		xTimeDelta.QuadPart = (xTimeEnd.QuadPart - xTimeStart.QuadPart) * 1000 / xFreq.QuadPart;
				
				//if (this->Address == 1)
				//{
				//	FILE *pFile = fopen("events.txt", "a");
				//	fprintf(pFile, "finish transfer 0x%X in %lld us\n", ov->hEvent, xTimeEnd.QuadPart);
				//	fclose(pFile);
				//}
#endif
		return true;
        }
	else
	{
		if (status == LIBUSB_TRANSFER_STALL)
			libusb_clear_halt(hDevice, this->Address);		
		libusb_cancel_transfer(rBuf);
		ov->Internal = (ULONG_PTR)2;
#ifdef __IPC_WIN__
		QueryPerformanceCounter(&xTimeEnd);
		xTimeDelta.QuadPart = (xTimeEnd.QuadPart - xTimeStart.QuadPart) * 1000 / xFreq.QuadPart;
				
				//if (this->Address == 1)
				//{
				//	FILE *pFile = fopen("events.txt", "a");
				//	fprintf(pFile, "fail transfer 0x%X in %lld us\n", ov->hEvent, xTimeEnd.QuadPart);
				//	fclose(pFile);
				//}
				
#endif
		return false;
        }
}

CCyIsocEndPoint::CCyIsocEndPoint()
{
}

PUCHAR CCyIsocEndPoint::BeginBufferedXfer(PUCHAR buf, LONG bufLen, OVERLAPPED *ov)
{
	return 0;
}

PUCHAR CCyIsocEndPoint::BeginDirectXfer(PUCHAR buf, LONG bufLen, OVERLAPPED *ov)
{
	return 0;
}

PUCHAR CCyIsocEndPoint::BeginDataXfer(PUCHAR buf, LONG len, OVERLAPPED * ov)
{
	return 0;
}

CCyBulkEndPoint::CCyBulkEndPoint()
{
}

void LIBUSB_CALL AsyncXferCB(libusb_transfer *transfer)
{	
	FILE *pFile;// = fopen("callbacks.txt", "a");
	switch (transfer->status)
	{		
		case LIBUSB_TRANSFER_COMPLETED:
#ifdef __IPC_WIN__
			SetEvent(((OVERLAPPED*)(transfer->user_data))->hEvent);
#else
            ((OVERLAPPED*)(transfer->user_data))->Internal = (ULONG_PTR)1;
#endif
            //fprintf(pFile, "callback success ep 0x%X, event 0x%X\n", transfer->endpoint, ((OVERLAPPED*)(transfer->user_data))->hEvent);
#ifdef __IPC_WIN__
			QueryPerformanceCounter(&xTimeEnd);
			xTimeDelta.QuadPart = (xTimeEnd.QuadPart - xTimeStart.QuadPart) * 1000 / xFreq.QuadPart;
												
						//if (transfer->endpoint == 1)
						//{
						//	pFile = fopen("events.txt", "a");
						//	fprintf(pFile, "callback 0x%X completed after %lld us\n", ((OVERLAPPED*)(transfer->user_data))->hEvent, xTimeEnd.QuadPart);
						//	fclose(pFile);
						//}
						
#endif
			break;
		case LIBUSB_TRANSFER_STALL:	
            //printf("callback stall ep 0x%X, event 0x%X\n", transfer->endpoint, ((OVERLAPPED*)(transfer->user_data))->hEvent);
            //fprintf(pFile, "callback stall ep 0x%X, event 0x%X\n", transfer->endpoint, ((OVERLAPPED*)(transfer->user_data))->hEvent);
#ifdef __IPC_WIN__
			QueryPerformanceCounter(&xTimeEnd);
			xTimeDelta.QuadPart = (xTimeEnd.QuadPart - xTimeStart.QuadPart) * 1000 / xFreq.QuadPart;
			
			//if (transfer->endpoint == 1)
			//{
			//	pFile = fopen("events.txt", "a");
			//	fprintf(pFile, "callback 0x%X stall after %lld us\n", ((OVERLAPPED*)(transfer->user_data))->hEvent, xTimeEnd.QuadPart);
			//	fclose(pFile);
			//}
			
#endif			
			break;
        case LIBUSB_TRANSFER_CANCELLED:
			while (1)
				if (((OVERLAPPED*)(transfer->user_data))->Internal == (ULONG_PTR)2)
					break;
#ifdef __IPC_WIN__
			SetEvent(((OVERLAPPED*)(transfer->user_data))->hEvent);
#else
			((OVERLAPPED*)(transfer->user_data))->Internal = (ULONG_PTR)1;
#endif
			libusb_free_transfer(transfer);
#ifdef __IPC_WIN__
			QueryPerformanceCounter(&xTimeEnd);
			xTimeDelta.QuadPart = (xTimeEnd.QuadPart - xTimeStart.QuadPart) * 1000 / xFreq.QuadPart;
			
			//if (transfer->endpoint == 1)
			//{
			//	pFile = fopen("events.txt", "a");
			//	fprintf(pFile, "callback 0x%X cancel in %lld us\n", ((OVERLAPPED*)(transfer->user_data))->hEvent, xTimeEnd.QuadPart);
			//	fclose(pFile);
			//}
			
#endif
            //fprintf(pFile, "callback cancelled ep 0x%X, event 0x%X\n", transfer->endpoint, ((OVERLAPPED*)(transfer->user_data))->hEvent);
            break;
		default:
			
            //fprintf(pFile, "callback other %d, ep 0x%X, event 0x%X\n", transfer->status, transfer->endpoint, ((OVERLAPPED*)(transfer->user_data))->hEvent);
			
			break;
	}
	/*fclose(pFile);*/
}

PUCHAR CCyBulkEndPoint::BeginDataXfer(PUCHAR buf, LONG len, OVERLAPPED * ov)
{
#ifdef __IPC_WIN__
	QueryPerformanceCounter(&xTimeStart);
#endif
        int r = 0;
        libusb_transfer *rBuf = libusb_alloc_transfer(0);
        libusb_fill_bulk_transfer(rBuf, hDevice, this->Address, buf, len, &AsyncXferCB, (void *)ov, this->TimeOut);
#ifdef __IPC_WIN__
		ResetEvent(ov->hEvent);
#else
		ov->Internal = 0;
#endif
		r = libusb_submit_transfer(rBuf);
    /*FILE *pFile = fopen("events.txt", "a");
    fprintf(pFile, "begin transfer ep 0x%X, event 0x%X\n", this->Address, ov->hEvent);
    fclose(pFile);*/
#ifdef __IPC_WIN__
		QueryPerformanceCounter(&xTimeEnd);
		xTimeDelta.QuadPart = (xTimeEnd.QuadPart - xTimeStart.QuadPart) * 1000 / xFreq.QuadPart;
		//if (this->Address == 1)
		//{
		//	FILE *pFile = fopen("events.txt", "a");
		//	fprintf(pFile, "started transfer 0x%X in %lld us\n", ov->hEvent, xTimeEnd.QuadPart);
		//	fclose(pFile);
		//}
#endif
        return (PUCHAR)rBuf;
}

bool CCyUSBEndPoint::Abort(void)
{
	return true;
}

bool CCyUSBEndPoint::Reset(void)
{
	//libusb_control_transfer(hDevice, );
	return true;
}

UCHAR CCyUSBDevice::EndPointCount()
{
	const libusb_interface *pIntfc;
	const libusb_interface_descriptor *pIntfcDesc;
	const libusb_endpoint_descriptor *pEpDesc;
	pIntfc = &(hCfgDesc->interface[0]);
	pIntfcDesc = &(pIntfc->altsetting[AltIntfc()]);
	return pIntfcDesc->bNumEndpoints;
}

CCyFX3Device::CCyFX3Device()
{	
}

CCyFX3Device::~CCyFX3Device()
{
}

bool CCyFX3Device::IsBootLoaderRunning()
{
	if ((this->VendorID == 0x04b4) && (this->ProductID == 0x00F3))
		return true;
	else
		return false;
}

FX3_FWDWNLOAD_ERROR_CODE CCyFX3Device::DownloadFw(char *fileName, FX3_FWDWNLOAD_MEDIA_TYPE enMediaType)
{
	FX3_FWDWNLOAD_ERROR_CODE Status = SUCCESS;
	unsigned char *pBuf;
	unsigned int *dataP, checksum, length, address;
	int index;
	FILE *pFile;
	pBuf = (unsigned char *)calloc(1, MAX_FWIMG_SIZE);
	pFile = fopen(fileName, "r+b");
	if (pFile == NULL)
		Status = INVALID_FILE;
	else
	{
		fseek(pFile, 0, SEEK_END);
		int nFileSize = ftell(pFile);
		if (nFileSize > MAX_FWIMG_SIZE)
			Status = INCORRECT_IMAGE_LENGTH;
		else
		{
			if (nFileSize <= 5)
				Status = INVALID_FILE;
			else
			{
				fseek(pFile, 0, SEEK_SET);
				//fread(pBuf, 1, nFileSize, pFile);
				for (int i = 0; i < nFileSize; i++)
					fread(pBuf+i, 1, 1, pFile);
				switch (enMediaType)
				{
				case RAM:
					index = 4;
					checksum = 0;
					while (index < nFileSize)
					{
						dataP = (unsigned int *)(pBuf + index);
						length = dataP[0];
						address = dataP[1];
						if (length != 0)
						{
							for (int i = 0; i < length; i++)
								checksum += dataP[2 + i];
							int len = length * 4;
							LONG size = 0, r = 0;
							int nWriteIndex = 0;
							while (len > 0)
							{
								size = (len > MAX_WRITE_SIZE) ? MAX_WRITE_SIZE : len;
								this->ControlEndPt->Direction = DIR_TO_DEVICE;
								this->ControlEndPt->ReqCode = 0xA0;
								this->ControlEndPt->ReqType = REQ_VENDOR;
								this->ControlEndPt->Value = (address & 0xFFFF);
								this->ControlEndPt->Index = (address >> 16);
								r = this->ControlEndPt->XferData(pBuf + index + nWriteIndex + 8, size);
								if (r != true)
								{
									Status = FAILED;
									break;
								}
								address += size;
								nWriteIndex += size;
								len -= size;
							}
						}
						else
						{
							if (checksum != dataP[2])
							{
								Status = CORRUPT_FIRMWARE_IMAGE_FILE;
							}
							this->ControlEndPt->Direction = DIR_TO_DEVICE;
							this->ControlEndPt->ReqCode = 0xA0;
							this->ControlEndPt->ReqType = REQ_VENDOR;
							this->ControlEndPt->Value = (address & 0xFFFF);
							this->ControlEndPt->Index = (address >> 16);
							LONG size = 0, r = 0;
							r = this->ControlEndPt->XferData(NULL, size);

							if (r != true)
								Status = FAILED;
							break;
						}

						index += (8 + length * 4);
					}
					break;
				case I2CE2PROM:
					break;
				case SPIFLASH:
					break;
				default:
					Status = INVALID_MEDIA_TYPE;
					break;
				}
			}
		}
		fclose(pFile);
	}
	free(pBuf);
	return Status;
}
