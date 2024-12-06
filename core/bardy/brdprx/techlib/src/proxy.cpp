/*
 BARDY hack
*/

#include "tech.h"
#include "dict.h"

#include "brdapi.h"

S32 TSrvProxy::Ctrl( S32 cmd, void *pParam )
{

	DEV_CMD_Ctrl    param;

	param.arg = pParam;
	param.cmd = cmd;
	param.handle = (m_index<<16);
	param.nodeId = 0;

	SERV_CMD_IsAvailable *px = (SERV_CMD_IsAvailable*)pParam;

	S32 ret = m_pDevice->CallDriver( DEVcmd_CTRL, &param, 1 );

	if( BRD_errcmp(ret,BRDerr_CMD_UNSUPPORTED) )
	{
		//printf( "hello" );
		for( int i=0; i<sub.get_length(); i++ )
		{
			TSrv *psub = sub[i];

			if( psub )
			{
				ret = psub->Ctrl( cmd, pParam );

				if( !BRD_errcmp(ret,BRDerr_CMD_UNSUPPORTED) )
					return ret;
			}
		}

	}

	return ret;
}

S32 TSrvProxy::Lock()
{
    IPC_captureMutex( m_hMutex, -1 );

    IPC_setEvent( m_hEvent );

	return 0;
}

S32 TSrvProxy::Unlock()
{
    if( IPC_captureMutex( m_hMutex, 200 ) != IPC_OK )
		return 0;

    IPC_resetEvent( m_hEvent );
    IPC_releaseMutex( m_hMutex );

	return 0;
}

S32 TSrvProxy::isLocked()
{
    DWORD ret = IPC_waitEvent( m_hEvent, 0 );

    return (ret==0);
}

S32 TSrvProxy::Capture()
{
	/*
	DEV_CMD_Ctrl	paramCtrl;

	S32 ret;

	{
		//
		// Send DEVcmd_CTRL + SERVcmd_SYS_ISAVAILABLE to Service
		//
		SERV_CMD_IsAvailable av;

		paramCtrl.handle = (m_index<<16);
		paramCtrl.nodeId = 0;
		paramCtrl.cmd = SERVcmd_SYS_ISAVAILABLE;
		paramCtrl.arg = &av;

		ret = m_pDevice->CallDriver( DEVcmd_CTRL, &paramCtrl, 0 );

		if( !BRD_errcmp(ret,BRDerr_OK) && !BRD_errcmp(ret,BRDerr_CMD_UNSUPPORTED) )
			return 0;
	}

	{
		//
		// Send DEVcmd_CTRL + SERVcmd_SYS_CAPTURE to Service
		//
		paramCtrl.handle = (m_index<<16);
		paramCtrl.nodeId = 0;
		paramCtrl.cmd = SERVcmd_SYS_CAPTURE;
		paramCtrl.arg = NULL;
		ret = m_pDevice->CallDriver( DEVcmd_CTRL, &paramCtrl, 1 );

		if( !BRD_errcmp(ret,BRDerr_OK) && !BRD_errcmp(ret,BRDerr_CMD_UNSUPPORTED) )
			return 0;
	}
*/

	Ctrl(SERVcmd_SYS_CAPTURE, 0);

	return TSrv::Capture();
}
