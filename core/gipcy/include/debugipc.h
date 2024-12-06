
#ifndef __DEBUGIPC_H__
#define __DEBUGIPC_H__

#ifndef __IPCLIB_H__
    #include "ipclib.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum IPC_DEBUGLEVEL
{
	IPCD_NONE = 0,
	IPCD_LOG = 1,
	IPCD_WARN = 2,
	IPCD_ERR = 4,
};

GIPCY_API IPC_handle IPC_InitDebug(const IPC_str *name, int screenflags, int fileflags);

GIPCY_API void IPC_Logf(const IPC_str *format, ...);
GIPCY_API void IPC_Errorf(const IPC_str *format, ...); 
GIPCY_API void IPC_Debugf(const IPC_str *format, ...);

GIPCY_API void IPC_VPrint(int flags,const IPC_str *format, va_list args);
GIPCY_API void IPC_Printf(int flags,const IPC_str *format, ...);

#ifdef __cplusplus
}
#endif

#endif //__DEBUGIPC_H__
