#include "_brd.h"

#include "gipcy.h"
#include <unordered_map>
#ifndef __linux__
#include <unordered_map>
#else
//#include <hash_map>
#endif
#include <vector>

class _Stream
{
public:

	_Stream()
	{
		pStub = NULL;
		device = 0;
	}

	BRD_Handle device;
	U32 size;
	U32 blknum;
    void **ptr;
    BRDstrm_Stub *pStub;
	//U32 dir;
	int src;
	int dir;
};

#ifdef WIN32
class _SHELL
{
public:

	CRITICAL_SECTION cs;//multithreated safety

	_SHELL()
	{
		InitializeCriticalSection( &cs );
	}

	~_SHELL()
	{
		DeleteCriticalSection( &cs );
	}

	static void lock()
	{
		EnterCriticalSection( &shell.cs );
	}

	static void unlock()
	{
		LeaveCriticalSection( &shell.cs );
	}

	static _SHELL shell;
};
#else
class _SHELL
{
public:

    IPC_handle m_hMutex;

    _SHELL()
    {
        m_hMutex = IPC_createMutex("ISAGENT", 0);
    }

    ~_SHELL()
    {
        IPC_deleteMutex(m_hMutex);
    }

    static void lock()
    {
        if(!shell.m_hMutex)
            shell.m_hMutex = IPC_createMutex("ISAGENT", 0);

        IPC_captureMutex(shell.m_hMutex, 100);
    }

    static void unlock()
    {
        IPC_releaseMutex(shell.m_hMutex);
    }

    static _SHELL shell;
};
#endif // WIN32

_SHELL _SHELL::shell;

class _BRD
{
public:
	U32 lid;

	BRD_Handle device;
	BRD_Handle reg;

	_Stream streams[4];

	int rc;

	_BRD()
	{
		device = 0;
		reg = 0;

		rc = 0;

		memset( streams, 0, sizeof(streams) );
	}

	~_BRD()
	{
		if( device )
			S32 err = BRD_close( device );

		device = 0;
	}

};


typedef std::unordered_map< BRD_Handle, _BRD* > BRDHASH;

BRDHASH _map;

BRD_Handle	_BRD_open(U32 lid, U32 flag, void *ptr)
{

	//_SHELL::lock();

	BRDHASH::iterator p;

	for (p = _map.begin(); p != _map.end(); ++p)
	{
		if (p->second->lid == lid)
		{
			//_SHELL::unlock();

			if (ptr)
				*((U32*)ptr) = BRDcapt_SHARED;

			p->second->rc++;

			return p->second->device;
		}
	}

	U32 sharedFlag = BRDcapt_EXCLUSIVE;
	BRD_Handle h = BRD_open(lid, sharedFlag, NULL);

	if (ptr)
		*((U32*)ptr) = BRDcapt_SHARED;


	_BRD *b = new _BRD();

	b->device = h;
	b->lid = lid;
	b->rc++;

	_map[h] = b;

	b->reg = _BRD_reg(h);

	if (b->reg>0)
	{
		U32 ItemReal = 0;
		BRD_serviceList(h, 0, NULL, 0, &ItemReal);
	}

    //_SHELL::unlock();

	return h;
}

S32		_BRD_close(BRD_Handle handle )
{
	BRDHASH::iterator p;

	for (p = _map.begin(); p != _map.end(); ++p)
	{
		if (p->second->device == handle )
		{

			p->second->rc--;

			if (p->second->rc <= 0)
			{


				for (int i = 0; i < 4; i++)
				{

					BRD_release( p->second->streams[i].device, 0);
				}

				BRD_release(p->second->reg, 0);

				delete p->second;

				_map.erase(handle);
			}

			break;
		}
	}

	return 0;
}

BRD_Handle		_BRD_getStream(BRD_Handle handle, int id, BRDctrl_StreamCBufAlloc *pAlloc, U32 timeout )
{
	U32 ItemReal = 0;

	BRD_serviceList( handle, 0, NULL, 0, &ItemReal );

	if( ItemReal == 0 )
		return 0;

	BRD_ServList *pList = new  BRD_ServList[ItemReal];

	BRD_serviceList( handle, 0, pList, ItemReal, &ItemReal );

	int streamIndex = -1;

	int strmCnt = 0;

	for( int i=0; i<(int)ItemReal; i++ )
	{
		BRD_ServList *item = &pList[i];

		if( item->attr & BRDserv_ATTR_STREAM )
		{


			if( i != (int)id )
				continue;



            if(((item->attr & BRDserv_ATTR_DIRECTION_IN) && (pAlloc->dir==1)) || ((item->attr & BRDserv_ATTR_DIRECTION_OUT) && (pAlloc->dir==2)) )
			{
				streamIndex = i;
				break;
			}


		}
	}

	if( streamIndex == -1 )
		return 0;

	BRD_ServList *item = &pList[streamIndex];

	U32 mode = BRDcapt_EXCLUSIVE;
	BRD_Handle h = BRD_capture( handle, 0, &mode, item->name, timeout );

	return h;
}

BRD_Handle		_BRD_reg(BRD_Handle handle)
{
	_BRD *b = _map[handle];

	if (b->reg )
		return b->reg;

	U32 mode = 0;
	BRD_Handle hReg= BRD_capture( b->device, 0, &mode, _BRDC("REG0"), 100);

	if (hReg >= 0)
	{
		BRD_Reg arg = { 0, 0x108, 0, 0, 0 };

		S32 ret = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &arg);

		if(ret<0)
			hReg = -1;
	}

	b->reg = hReg;

	return b->reg;
}

BRD_Handle		_BRD_findStream(BRD_Handle handle, int src )
{
	BRD_Handle h = 0;
	U32 index = 0;

	_SHELL::lock();
	_BRD *b = _map[handle];

	if (b == NULL)
		goto fatal;

	index = -1;

	for (int i = 0; i < 4; i++)
	{

		if (b->streams[i].src != src)
			continue;

		index = i;
		break;
	}

	if (index != -1)
		 h = b->streams[index].device;

	fatal:

	_SHELL::unlock();

	return h;
}
BRD_Handle		_BRD_captureStream(BRD_Handle handle, int src, BRDctrl_StreamCBufAlloc *pAlloc, U32 timeout )
{
	_SHELL::lock();
	_BRD *b = _map[handle];


	U32 index = -1;

	for (int i = 0; i < 4; i++)
	{

		if ( (b->streams[i].device != 0) && (b->streams[i].src != src) )
			continue;

		index = i;
		break;
	}

	if (index == -1)
		return -1;

	BRD_Handle h = b->streams[index].device;



	if (h == 0)
	{
		h = _BRD_getStream( b->device, index, pAlloc, timeout );

		if( h == 0 )
        {
            _SHELL::unlock();
			return h;
        }
	}

    {
		if( pAlloc->blkSize &&
			pAlloc->blkNum &&
			((b->streams[index].size != pAlloc->blkSize) ||
			(b->streams[index].blknum != pAlloc->blkNum)))
		{
			S32 err;

			err = BRD_ctrl(h, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
			err = BRD_ctrl( h,0, BRDctrl_STREAM_CBUF_FREE, NULL );

            err = BRD_ctrl( h,0, BRDctrl_STREAM_CBUF_ALLOC, pAlloc );

			/*
			if (err < 0)
			{

				err = BRD_ctrl(h, 0, BRDctrl_STREAM_CBUF_ALLOC, pAlloc);
			}
			*/

			if( err < 0 )
            {
                _SHELL::unlock();
				return -1;
            }

            b->streams[index].ptr = pAlloc->ppBlk;
			b->streams[index].size = pAlloc->blkSize;
			b->streams[index].blknum = pAlloc->blkNum;
            b->streams[index].pStub = pAlloc->pStub;
		}
		else
		{
            pAlloc->ppBlk = b->streams[index].ptr;
			pAlloc->blkSize = b->streams[index].size;
			pAlloc->blkNum = b->streams[index].blknum;
            pAlloc->pStub = b->streams[index].pStub;
		}

		b->streams[index].device = h;
		b->streams[index].src = src;

	}

    _SHELL::unlock();

	return h;

}

S32				_BRD_release( BRD_Handle handle, U32 nodeId )
{
	return 0;
}

S32				_BRD_cleanup( void )
{
	BRDHASH::iterator p;

	for (p = _map.begin(); p != _map.end(); ++p)
	{
		_BRD* b = p->second;

		delete b;
	}

	_map.empty();

	return 0;
}
