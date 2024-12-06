//****************** File ddw6678pex.h *********************************
//  TI6678 user application interface
//
//	Copyright (c) 2012, Instrumental Systems,Corp.
//  Written by Dorokhin Andrey
//
//  History:
//  12-12-12 - builded
//
//*******************************************************************

#ifndef _DDW6678PEX_H_
 #define _DDW6678PEX_H_

#ifdef _WIN64
#define TI6678DeviceName L"W6678pex"
#else
#ifdef __linux__
#define TI6678DeviceName "l6678pex"
#else
#define TI6678DeviceName "W6678pex"
#endif
#endif

#define MAX_STRING_LEN	255


#ifdef __linux__

#ifndef __KERNEL__
#include <sys/ioctl.h>
#endif

#define TI6678_DEVICE_TYPE             'm'
#define TI6678_MAKE_IOCTL(c) _IO(TI6678_DEVICE_TYPE, (c))

#else

#define TI6678_DEVICE_TYPE             0x8000
#define TI6678_MAKE_IOCTL(c)\
		(ULONG)CTL_CODE(TI6678_DEVICE_TYPE, 0x800+(c), METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif

#define IOCTL_TI6678_GET_VERSION			TI6678_MAKE_IOCTL(0)

#define IOCTL_TI6678_SET_BUS_CONFIG			TI6678_MAKE_IOCTL(2)
#define IOCTL_TI6678_GET_BUS_CONFIG			TI6678_MAKE_IOCTL(3)
#define IOCTL_TI6678_GET_LOCATION			TI6678_MAKE_IOCTL(5)
#define IOCTL_TI6678_GET_CONFIGURATION		TI6678_MAKE_IOCTL(7)
#define IOCTL_TI6678_MEM_ALLOC				TI6678_MAKE_IOCTL(8)
#define IOCTL_TI6678_MEM_FREE				TI6678_MAKE_IOCTL(9)

#define IOCTL_TI6678_WRITE_MEM				TI6678_MAKE_IOCTL(12)
#define IOCTL_TI6678_READ_MEM				TI6678_MAKE_IOCTL(13)

#define IOCTL_TI6678_READ_ICR				TI6678_MAKE_IOCTL(14)
#define IOCTL_TI6678_GET_ICR				TI6678_MAKE_IOCTL(15)
#define IOCTL_TI6678_WAIT_EVENT				TI6678_MAKE_IOCTL(16)
#define IOCTL_TI6678_RESET_EVENT			TI6678_MAKE_IOCTL(17)

#define IOCTL_TI6678_RESTORE_PCICFG			TI6678_MAKE_IOCTL(18)


#define INSYS_VENDOR_ID		0x4953

#define FMC110PDSP_DEVID		0x6610 // DeviceID for FMC110PDSP
#define FMC111PDSP_DEVID		0x6611 // DeviceID for FMC111PDSP
#define FMC112PDSP_DEVID		0x6612 // DeviceID for FMC112PDSP
#define FMC114VDSP_DEVID		0x6614 // DeviceID for FMC114VDSP
#define PEXSRIO_DEVID			0x6615 // DeviceID for PEXSRIO
#define FMC117PDSP_DEVID		0x6617 // DeviceID for FMC117PDSP
#define DSP6678PEX_DEVID		0x6620 // DeviceID for DSP6678PEX
#define DSP6678V_DEVID			0x6622 // DeviceID for DSP6678V

#pragma pack(push,1)

#ifdef __KERNEL__

typedef u32 ULONG;
typedef s32 LONG;
typedef u8  UCHAR;
typedef u32 UINT;
typedef u16 USHORT;
typedef void* PVOID;
typedef void* HANDLE;

//#endif

//#ifdef __linux__
//! Описывает параметры для команд управления устройством
struct ioctl_param {
    void *srcBuf;       //!< буфер с данными для устройства (через него передаются данные В драйвер нулевого кольца)
    size_t srcSize;        //!< размер буфера с данными для устройства
    void *dstBuf;       //!< буфер с данными от устройства  (через него передаются данные ИЗ драйвера нулевого кольца)
    size_t dstSize;        //!< dstSize - размер буфера с данными от устройства
}__attribute__((packed));

#endif

typedef struct _TI6678_DATA_BUF {
	void*	pBuffer;
	ULONG	BufferSize;
	ULONG	Offset;
} TI6678_DATA_BUF, *PTI6678_DATA_BUF;

// data structure for read/write buffer from/to memory of board
typedef struct _TI6678_DATA_MEM {
	void*	pBuffer;
	ULONG	BufferSize;
	ULONG	Address;
	ULONG	CoreNumber;
} TI6678_DATA_MEM, *PTI6678_DATA_MEM;

// board location data structure
typedef struct _TI6678_LOCATION {
	ULONG	BusNumber;		// OUT
	ULONG	DeviceNumber;	// OUT
	ULONG	SlotNumber;		// OUT
} TI6678_LOCATION, *PTI6678_LOCATION;

// board configuration data structure
typedef struct _TI6678_CONFIGURATION {
#ifdef _WIN64
	__int64	PhysAddress[6];	// OUT
#else
	size_t	PhysAddress[6];	// OUT
#endif
	void*	VirtAddress[6];	// OUT
	ULONG	Size[6];		// OUT
	ULONG	InterruptLevel;	// OUT
	ULONG	InterruptLine;	// OUT
	ULONG	InterruptPin;	// OUT
} TI6678_CONFIGURATION, *PTI6678_CONFIGURATION;

// Memory Allocation/Free
typedef struct _TI6678_MEM {
	unsigned long long	PhysAddress;	// OUT - Physical address
	void*		VirtAddress;	// OUT - Virtual application address
	ULONG		Size;			// IN - size of bytes
	ULONG		MemLimit;		// IN - flag of memory limit (0 - NO limit, 1 - less 4 Gb)
} TI6678_MEM, *PTI6678_MEM;

// interrupt event wait data structure
typedef struct _TI6678_EVENT {
	ULONG	EventNum;		// IN
	ULONG	EventCount;		// OUT
	LONG	Timeout;		// IN
} TI6678_EVENT, *PTI6678_EVENT;


#pragma pack(pop)

#endif // _DDW6678PEX_H_

// ****************** End of file ddw6678pex.h **********************
