// ----- include files -------------------------
#include	<malloc.h>
#include	<stdio.h>
//#include	<conio.h>
#include	<string.h>
#include	<ctype.h>
//#include	<dos.h>
#include	<time.h>
//#include	<windows.h>
//#include	<winioctl.h>

//#define	DEV_API		DllExport
//#define	DEVS_API	DllExport

#define	DEV_API_EXPORTS
#define	DEVS_API_EXPORTS

#include	"utypes.h"
#include	"brdapi.h"
#include	"ctpapi.h"
#include	"dbprintf.h"
#include	"buni.h"
#include	"extn.h"
#include	"tech.h"
#include	"basedriver.h"
#include	"servstrm.h"
#include	"gipcy.h"

//
//==== Types
//




BUNI2Driver::BUNI2Driver(  ) : BaseDriver( )
{
    m_pEntry = (void*)DEV_entry;
    m_pSideEntry = (void*)DEVS_entry;

	
}

BUNI2Driver::~BUNI2Driver()
{
	CTP_Close( (CTP_DEV*)pCTPDev );

	CTP_Disconnect( (CTP_HOST*)pCTPHost );

	//Serv_ClearServList( this );
}

TSrv* BUNI2Driver::findServ( BRDCHAR *name )
{
	for( int i=0; i<_loadedServList.get_length();i++ )
	{
		TSrv *ps = _loadedServList[i];

		if( BRDC_strstr( ps->defName, name ) != NULL  )
		{
			return ps;
		}

		
	}
		
	return NULL;
}

void BUNI2Driver::fillServList( TDict *dict )
{
	TSrv* pStream[2];
		
	for( int i=0; i<2; i++)
	{
		pStream[i] = new TStream( i );

		pStream[i]->owner = this;//root device

		//FIXME
		_servList.push( pStream[i] );
		_loadedServList.push( pStream[i] );
	}

	//ps->pModule = this;
	
	if( pCTPDev == 0 )
	{
		pCTPDev = CTP_Open( (CTP_HOST*)pCTPHost, brdId );
	}

	if( dict )
	{
		TDict *sd = dict->getDictByName( _BRDC("SUBUNIT") );

		if( sd )
		{
			BRDCHAR *type = sd->getString(_BRDC("btype0") );

			if( type )
				loadSide( type, sd );
		
			type = sd->getString(_BRDC("btype1") );

			if( type )
				loadSide( type, sd );
		
			type = sd->getString(_BRDC("type") );
		
			if( type )
				loadSide( type, sd );
		}
	}

	int nStreamUsed = 0;

	for( int i=0; i<_loadedServList.get_length();i++ )
	{
		TSrv *ps = _loadedServList[i];

		if( nStreamUsed < 2 && ps->attr & (BRDserv_ATTR_STREAMABLE_IN|BRDserv_ATTR_STREAMABLE_OUT) )
		{
			//printf( "stream %s\n", ps->name );
			TSrv *strm;

			strm = pStream[nStreamUsed];
			nStreamUsed++;

			ps->stream = strm;
			
			if( strm )
				ps->sub.push( strm ); 
		}		
		
		if( ps->attr & (BRDserv_ATTR_ADCABLE) )
		{
			//printf( "stream %s\n", ps->name );
			TSrv *padc = findServ( _BRDC("ADC") );

			if( padc )
				ps->sub.push( padc ); 
		}

		
	}
	
	CTP_Close( (CTP_DEV*)pCTPDev );
	pCTPDev = NULL;
}

#include "icr.h" 

// ADM_ID_TAG
// SUBMOD_CFGMEM_SIZE
S32 BUNI2Driver::readIcr( void *pData, U32 tag, U32 size,U32* sizeOut )
{
	
	ULONG puAdmIcrId = 0;

	BRD_PuList PuList[64];
	U32 ItemReal;



	S32 status;

	{
		DEV_CMD_PuList param = { PuList, 64, &ItemReal };

		puList( &param );
	}

	for(ULONG ii = 0; ii < ItemReal; ii++)
	{
		if(PuList[ii].puCode == tag)
		{
			puAdmIcrId = PuList[ii].puId;
			break;
		}
	}

	//char pAdmCfgMem[SUBMOD_CFGMEM_SIZE];
        U32 nRealAdmCfgSize = 0;

	S32 nSuccess;

	if( puAdmIcrId )
	{
		//char msg[128];
		DEV_CMD_PuRead param = { puAdmIcrId, 0, (void*)pData, size };

		status = puRead( &param );

		//sprintf(msg, "Reading submodule ICR, status = 0X%08X", status);
		//MessageBox(NULL, msg, "IcrBardy", MB_OK);
#ifndef __linux__
        if(!BRD_errcmp(status, BRDerr_OK))
			MessageBox(NULL, _BRDC("ERROR by reading submodule ICR"), _BRDC("IcrBardy"), MB_OK);
		else
#endif
			if(*(PUSHORT)pData == tag)
			{
				nRealAdmCfgSize = *((PUSHORT)pData + 2);
				nSuccess = 1;
			}
	}

	if( sizeOut )
	{
		*sizeOut = nRealAdmCfgSize;
	}
	
	return nSuccess;
}

void BUNI2Driver::loadSide( BRDCHAR *type, TDict *dict )
{
	TSideProxy *pTech = CreateSideDrv( type, (void*)this, this , dict );

	if( !pTech )
		return;

	//pTech->SetServList();

	int len = pTech->GetServiceCount();

	for( int i=0; i<len; i++ )
	{
		TSrvInfo *pi = pTech->GetService( i );
		
		

		U32 mode = 0;

		TSrv *srv = pTech->ProxySrv( &mode, pi->name, i, -1 );

		if( srv )
		{
			srv->owner = this;//root device

			if( !(pi->attr & BRDserv_ATTR_UNVISIBLE) )
				_servList.push( srv );

			_loadedServList.push( srv );
		}

	}

	/*
	TSrvProxy *px = pTech->Capture( NULL, "REG", 0 );

	BRD_Reg tr;
	
	px->Ctrl( BRDctrl_REG_READDIR, &tr );
	*/

	_list.push( pTech );


}



//=********************** CMD_init ******************
//=*****************************************************
S32 BUNI2Driver::init(  DEV_CMD_Init *pParam )
{
	    //
		//=== Dummy fuction
		//

	pCTPHost = CTP_Connect( ipAddr, ipPort,0 );

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_cleanup ******************
//=*****************************************************
S32 BUNI2Driver::cleanup(  DEV_CMD_Cleanup *pParam )
{   
    Dev_FreeBoard( this );  //Освобождение ресурсов, выделенных для платы

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_reinit ******************
// используется для устройств с "горячей" заменой
//=*****************************************************
S32 BUNI2Driver::reinit(  DEV_CMD_Reinit *pParam )
{
		//
		//=== Dummy fuction
		//
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_checkExistance ******************
// Функция проверяет, что устройство уже используется
//=*****************************************************
S32 BUNI2Driver::checkExistance( DEV_CMD_CheckExistance *pParam )
{
    

	//
	//=== TODO: If Board is in Work, return 1.
	//
    *(pParam->state) = 1;

    return DRVERR(BRDerr_OK);
}

#include "ctrlreg.h"

//=********************** CMD_open ******************
//=*****************************************************
S32 BUNI2Driver::open( DEV_CMD_Open *ptr)
{


	CTP_DEV *pCTPDev = CTP_Open( (CTP_HOST*)this->pCTPHost, this->brdId );

	this->pCTPDev = pCTPDev;

	

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_close ******************
//=*****************************************************
S32 BUNI2Driver::close(  void *pParam )
{   
	if( !this->pCTPDev )
		return DRVERR(BRDerr_ERROR);

	CTP_Close( (CTP_DEV*)this->pCTPDev );
	this->pCTPDev = NULL;

	return DRVERR(BRDerr_OK);
}

IPC_sockaddr _IPC_resolve(IPC_str* addr);

//=********************** CMD_getinfo ******************
//=*****************************************************
S32 BUNI2Driver::getinfo(  void *pParam )
{
	DEV_CMD_Getinfo *ptr = (DEV_CMD_Getinfo*)pParam;

	TCTP_GetInfo sGetInfo;

	if( CTP_GetInfo( (CTP_HOST*)this->pCTPHost, this->brdId, &sGetInfo ) != 0 )
    {
        //HFree( pDev );
		return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
	}
	
	ptr->pInfo->boardType   = sGetInfo.boardType;
    ptr->pInfo->pid         = sGetInfo.pid;        
	ptr->pInfo->busType		= BRDbus_ETHERNET;

	U32 ip = (U32)_IPC_resolve(this->ipAddr).addr.ip; 	

	if (INADDR_NONE == ip || INADDR_ANY == ip)
	{


		ip = IPC_gethostbyname(this->ipAddr).addr.ip;


	}
	
	ptr->pInfo->bus         = ip;
    ptr->pInfo->dev         = this->ipPort;
    ptr->pInfo->slot        = 0;       
    ptr->pInfo->verMajor    = VER_MAJOR;    
    ptr->pInfo->verMinor    = VER_MINOR;    
    
	ptr->pInfo->base[0]     = ip;
    ptr->pInfo->base[2]		= this->ipPort;
	
	ptr->pInfo->vectors[0]  = 0;  

	
	
#ifdef _WIN64
	BRDCHAR buffer[64];

	mbstowcs(buffer, sGetInfo.name, 64 );
#else
	BRDCHAR* buffer = sGetInfo.name;
#endif

	BRDC_sprintf( ptr->pInfo->name, _BRDC("BUNI2_%s"), buffer);

    return DRVERR(BRDerr_OK);
}

int fsize( FILE * _File )
{
	int cur = ftell( _File );

	fseek( _File, 0, SEEK_END );
	int res = ftell( _File );

	fseek( _File, cur, SEEK_SET );

	return res;
}

BRDCHAR outstr[64];

BRDCHAR* ExtractFileName(BRDCHAR *fullpath )
{
	size_t l = BRDC_strlen(fullpath);
	int offset = 0;
	unsigned int i = 0;
	
	for (i=0; i<l; i++)
	{
		if ( (fullpath[i] == '/') || (fullpath[i] == '\\') )
		offset = i+1;
	}

	BRDC_strcpy( outstr, &fullpath[offset] );

	return outstr;
}

S32	LL_UploadFile( BRDDRV_Board *pDev, BRDCHAR *fname )
{
	BRDCHAR *buffer = 0;

	int size = 0;

	BRDCHAR *pData = 0;
	U32 nSize = 0;
	
	FILE *fin = BRDC_fopen( fname, _BRDC("rb") );

	if( fin == NULL )
		return 0;

	size = fsize( fin );

	buffer = (BRDCHAR*)malloc( size );

	fread( buffer, 1, size, fin );

	fclose( fin );

	fname = ExtractFileName( fname );
	CTP_UploadFile( (CTP_DEV*)pDev->pCTPDev, fname, buffer, size );

	return 0;
}

//=********************** LL_LinkBootConfig ************
//  BOOT Load Applications from link
//=*****************************************************
S32	LL_ProcessLinkBoot(  BRDDRV_Board *pDev, const BRDCHAR *fname )
{
	FILE    *in;
	BRDCHAR    *ptr;
	BRDCHAR    str[128];
	BRDCHAR    buf[128];
	BRDCHAR    bFileName[256];

	int res = 0;

	int     i,j,length;

		//=== read network configuration file *.tsx
	in = BRDC_fopen(fname,_BRDC("rt"));
	if(in == NULL) return 1;

	for (i=1;i<64;i++)
    {
	    fseek(in,0L,SEEK_SET);
			//=== find node
		BRDC_strcpy(str,_BRDC("NODE"));
		BRDC_itoa(i,buf,10);
		BRDC_strcat(str,buf);
	    length = BRDC_strlen(str);
	    for (;;)
        {
			BRDC_fgets(buf,127,in);
		    if(feof(in)) break;
		    if (BRDC_strncmp(str,buf,length) != 0) continue;
		    if( *(buf+length+1) != ' ') continue;
		    ptr = buf+length;
		    while(isalnum(*ptr++)==0);
		    ptr--;
		    for (j=0;j<(int)BRDC_strlen(buf);j++)
		        if( isspace(ptr[j]) != 0 ) break;
			BRDC_strncpy(bFileName,ptr,j);
		    bFileName[j]='\0';
		    ptr+=j;
		    while(isalnum(*ptr++)==0);
		    ptr--;
		  //  syscon = strtoul(ptr,&endptr,16);
		    for (j=0;j<(int)BRDC_strlen(buf);j++)
		        if( isspace(ptr[j]) != 0 ) break;
		    ptr+=j;
		    while(isalnum(*ptr++)==0);
		    ptr--;
		   // sdrcon = strtoul(ptr,&endptr,16);

            //if (i==1) 
			{
				res = LL_UploadFile( pDev, bFileName );
				continue;
            }

		    break;
	    }
	}
    
	fclose(in);

	return res;
}


//=********************** CMD_load ******************
//=*****************************************************
S32 BUNI2Driver::load(  void *pParam )
{
	DEV_CMD_Load    *ptr = (DEV_CMD_Load*)pParam;
    
	BRDCHAR *filename = (BRDCHAR*)ptr->fileName;
	BRDCHAR *buffer = 0;
	
	BRDCHAR     fname[512];
	int i,length = BRDC_strlen(filename);

	int err = DRVERR(BRDerr_OK);

	//=== check file extension
	BRDC_strcpy(fname,filename);
	BRDC_strlwr(fname);

		//=== check file extension
    for(i=0;i<length;i++)
        if( filename[i] == '.') break;

    if( i==length)		// can't find extension
    {
		BRDC_strcat(fname,_BRDC(".dxe"));
    } 

	int size = 0;

	if(BRDC_strncmp( ptr->fileName, DEF_CTPPNAME, DEF_CTPPLEN ) == 0 )
	{
		filename = (BRDCHAR*)&ptr->fileName[DEF_CTPPLEN];
	}
	else
	{
		LL_UploadFile( this,filename );
		
		if(BRDC_strncmp( &fname[i],_BRDC(".tsx"),3) == 0)	// network config file
			err = LL_ProcessLinkBoot( this,filename );

		filename = ExtractFileName( filename );
	}

	BRDCHAR *argv[32];

	if( ptr->argc <=0 )
	{
		ptr->argc = 1;
		ptr->argv = argv;
	}
	
	((BRDCHAR**)ptr->argv)[0] = filename;

	err = CTP_LoadDSP( (CTP_DEV*)this->pCTPDev, ptr->nodeId, (BRDCHAR**)ptr->argv, ptr->argc );

	if( buffer )
		free( buffer );

	return (err == 0 )?DRVERR(BRDerr_OK):DRVERR(BRDerr_ERROR);
}

//=********************** CMD_puLoad *******************
//=*****************************************************
S32 BUNI2Driver::puLoad(  void *pParam )
{
	DEV_CMD_PuLoad    *ptr = (DEV_CMD_PuLoad*)pParam;
    
	BRDCHAR *filename = (BRDCHAR*)ptr->fileName;
	BRDCHAR *buffer = 0;

	int size = 0;

	if(BRDC_strncmp( ptr->fileName, DEF_CTPPNAME, DEF_CTPPLEN ) == 0 )
	{
		filename = (BRDCHAR*)&ptr->fileName[DEF_CTPPLEN];
	}
	else
	{
		LL_UploadFile( this, filename );
		filename = ExtractFileName( filename );
	}

	S32 err = CTP_PULoad( (CTP_DEV*)this->pCTPDev, filename, ptr->puId );

	*(ptr->state) = (err<0) ? 0 : 1;

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_puState ******************
//=*****************************************************
S32 BUNI2Driver::puState(  void *pParam )
{
	DEV_CMD_PuState     *ptr = (DEV_CMD_PuState*)pParam;

	S32 err = CTP_PUState( (CTP_DEV*)this->pCTPDev,ptr->puId );

	*(ptr->state) = (err<=0) ? 0 : 1;

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_puList *******************
//=*****************************************************
S32 BUNI2Driver::puList(  DEV_CMD_PuList *pParam )
{
	DEV_CMD_PuList  *ptr = (DEV_CMD_PuList*)pParam;

	S32 err = 0;

	TCTP_PuList tmplist[32];
    int nntmp = BRD_min( 32, ptr->item );


	U32 itemReal = 0;

	if( !ptr )
		return DRVERR(BRDerr_OK);

	//HACK HACK HACK
	if( this->pCTPDev == 0 )
	{
		CTP_DEV *pCTPDev = CTP_Open( (CTP_HOST*)this->pCTPHost, this->brdId );
		err = CTP_PUList( pCTPDev,tmplist,nntmp );
		CTP_Close( pCTPDev );
	}
	else
		err = CTP_PUList( (CTP_DEV*)this->pCTPDev,tmplist,nntmp );
		
	if( ptr->pItemReal )
	{
		*(ptr->pItemReal) = err;

	}

	if( ptr->pList && (ptr->item>0) )
	{
		for( int i=0; i<ptr->item; i++ )
		{
			ptr->pList[i].puAttr = tmplist[i].puAttr;
			ptr->pList[i].puCode = tmplist[i].puCode;
			ptr->pList[i].puId = tmplist[i].puId;

#ifdef _WIN64
			mbstowcs( ptr->pList[i].puDescription, tmplist[i].puDescription, 128 );
#else
			memcpy( ptr->pList[i].puDescription, tmplist[i].puDescription, 128 );
#endif

		}
	}

	if( err <= 0 )
		return -1;

    return DRVERR(BRDerr_OK);
}
//=********************** CMD_puRead *******************
//=*****************************************************
S32 BUNI2Driver::puRead(  void *pParam )
{
	DEV_CMD_PuRead     *ptr = (DEV_CMD_PuRead*)pParam;

	S32 err = CTP_ReadEx( (CTP_DEV*)this->pCTPDev, 0, CTP_IO_PU, ptr->puId, ptr->offset, ptr->pBuf, ptr->size, 0 );

    return DRVERR(BRDerr_OK);
}
//=********************** CMD_puWrite ******************
//=*****************************************************
S32 BUNI2Driver::puWrite(  void *pParam )
{
	DEV_CMD_PuWrite     *ptr = (DEV_CMD_PuWrite*)pParam;

	S32 err = CTP_Write( (CTP_DEV*)this->pCTPDev,ptr->puId, 0,ptr->offset, ptr->pBuf, ptr->size );

    return DRVERR(BRDerr_OK);
}
//=********************** CMD_reset ******************
//=*****************************************************
S32 BUNI2Driver::reset(  void *pParam )
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_start ********************
//=*****************************************************
S32 BUNI2Driver::start(  void *pParam )
{
	DEV_CMD_Start *ptr = (DEV_CMD_Start*)pParam;
	
	CTP_Start( (CTP_DEV*)this->pCTPDev, ptr->nodeId );

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_stop *********************
//=*****************************************************
S32 BUNI2Driver::stop(  void *pParam )
{
	DEV_CMD_Stop *ptr = (DEV_CMD_Stop*)pParam;

	CTP_Stop( (CTP_DEV*)this->pCTPDev, ptr->nodeId );

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_capture ******************
//=*****************************************************
S32 BUNI2Driver::capture(  void *pParam )
{
   DEV_CMD_Capture    *ptr = (DEV_CMD_Capture*)pParam;

   S32				idxServ = (ptr->handle >> 16) & 0x3F;
	
   _servList[idxServ]->Capture();

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_release ******************
//=*****************************************************
S32 BUNI2Driver::release(  void *pParam )
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_ctrl *********************
//=*****************************************************
S32 BUNI2Driver::ctrl(  void *pParam )
{
    DEV_CMD_Ctrl    *ptr = (DEV_CMD_Ctrl*)pParam;



	S32				idxServ = (ptr->handle >> 16) & 0x3F;
	U32				mode   = (ptr->handle >> 23) & 0x3;

	return _servList[idxServ]->Ctrl( ptr->cmd, ptr->arg );
}

//=********************** CMD_extension ****************
//  Дополнительные функции - общие для всех модулей
//  и уникальные для конкретного модуля
//=*****************************************************
S32 BUNI2Driver::extension(  void *pParam )
{
DEV_CMD_Extension   *ptr = (DEV_CMD_Extension*)pParam;

    switch(ptr->cmd)
    {
	    //=== Allocate nonpageble block of system memory
    case BRDextn_GET_MINPERBYTE:    
        {
            BRDextn_GetMinperbyte  *pGM = (BRDextn_GetMinperbyte*)ptr->arg;

            pGM->val = 4;
            break;
        }
		//=== Allocate nonpageble block of system memory
    default:
            return DRVERR(BRDerr_CMD_UNSUPPORTED);

    }

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_symbol *******************
// Функция возвращает адрес символьной переменной в исполняемом файле для ЦОС.
// Исполняемый файл должен содержать символьную информацию
//=*****************************************************
S32 BUNI2Driver::symbol( void *pParam) 
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_version ******************
//=*****************************************************
S32 BUNI2Driver::version(  void *pParam )
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_peek *********************
// чтение ячейки памяти из модуля ЦОС
//=*****************************************************
S32  BUNI2Driver::peek( void *pParam)
{
	DEV_CMD_Peek  *ptr = (DEV_CMD_Peek*)pParam;

	U32 val[32];

	S32 ret = CTP_Read( (CTP_DEV*)this->pCTPDev, ptr->nodeId, CTP_IO_RAM, ptr->brdAdr,&val,sizeof(val) );

	ptr->val = val[0];

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_poke *********************
// запись ячейки памяти в модуле ЦОС
//=*****************************************************
S32  BUNI2Driver::poke(  void *pParam )
{
	DEV_CMD_Poke  *ptr = (DEV_CMD_Poke*)pParam;

	U32 val[32];

	val[0] = ptr->val;

	S32 ret = CTP_Write( (CTP_DEV*)this->pCTPDev, ptr->nodeId, CTP_IO_RAM, ptr->brdAdr,&val,sizeof(val) );

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_readRAM ******************
// чтение блока памяти из модуля ЦОС
//=*****************************************************
S32 BUNI2Driver::readRAM(  void *pParam )
{
	DEV_CMD_ReadRAM  *ptr = (DEV_CMD_ReadRAM*)pParam;

	S32 ret = CTP_Read( (CTP_DEV*)this->pCTPDev, ptr->nodeId, CTP_IO_RAM, ptr->brdAdr,ptr->hostAdr,ptr->itemNum*ptr->itemSize );

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_writeRAM ******************
// запись блока памяти в модуле ЦОС
//=*****************************************************
S32 BUNI2Driver::writeRAM(  void *pParam )
{
	DEV_CMD_WriteRAM  *ptr = (DEV_CMD_WriteRAM*)pParam;

	S32 ret = CTP_Write( (CTP_DEV*)this->pCTPDev, ptr->nodeId, CTP_IO_RAM, ptr->brdAdr,ptr->hostAdr,ptr->itemNum*ptr->itemSize );

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_readFIFO ******************
//=*****************************************************
S32 BUNI2Driver::readFIFO(  void *pParam )
{
	DEV_CMD_ReadFIFO     *ptr = (DEV_CMD_ReadFIFO*)pParam;

	U32 tCnt = 0;

	S32 timeout = ( (S32)ptr->timeout > 100 )?100:ptr->timeout;

	while( true )
	{
		S32 ret = CTP_Read( (CTP_DEV*)this->pCTPDev,ptr->nodeId, CTP_IO_FIFO,0,ptr->hostAdr, ptr->size, timeout  );

		if( (ret != ptr->size) && (ptr->size>0) )
		{
			tCnt++;

			if( ptr->timeout == -1 )
				continue;

			if( tCnt > ptr->timeout )
				return BRDerr_WAIT_TIMEOUT;

			continue;
		}
		else
			break;
	}

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_writeFIFO ******************
//=*****************************************************
S32 BUNI2Driver::writeFIFO(  void *pParam )
{
	DEV_CMD_WriteFIFO     *ptr = (DEV_CMD_WriteFIFO*)pParam;

	U32 tCnt = 0;
	
	S32 timeout = ( (S32)ptr->timeout > 100 )?100:ptr->timeout;

	while( true )
	{
		

		S32 ret = CTP_Write( (CTP_DEV*)this->pCTPDev,ptr->nodeId, CTP_IO_FIFO,0,ptr->hostAdr, ptr->size, timeout  );

		if( ret != ptr->size )
		{
			tCnt++;

			if( ptr->timeout == -1 )
				continue;

			if( tCnt > ptr->timeout )
				return BRDerr_WAIT_TIMEOUT;

			continue;
		}
		else
			break;
	}

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_readDPRAM ******************
//=*****************************************************
S32 BUNI2Driver::readDPRAM(  void *pParam )
{
	DEV_CMD_ReadDPRAM  *ptr = (DEV_CMD_ReadDPRAM*)pParam;

	S32 ret = CTP_Read( (CTP_DEV*)this->pCTPDev, ptr->nodeId, CTP_IO_DPRAM, ptr->offset, ptr->hostAdr,ptr->size );

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_writeDPRAM ******************
//=*****************************************************
S32 BUNI2Driver::writeDPRAM(  void *pParam )
{
	DEV_CMD_WriteDPRAM  *ptr = (DEV_CMD_WriteDPRAM*)pParam;

	S32 ret = CTP_Write( (CTP_DEV*)this->pCTPDev, ptr->nodeId, CTP_IO_DPRAM, ptr->offset,ptr->hostAdr,ptr->size );

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_read ******************
//=*****************************************************
S32 BUNI2Driver::read(  void *pParam )
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_write ******************
//=*****************************************************
S32 BUNI2Driver::write(  void *pParam )
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_GetMsg *******************
//=*****************************************************
S32 BUNI2Driver::getMsg(  void *pParam )
{
	DEV_CMD_GetMsg  *ptr = (DEV_CMD_GetMsg*)pParam;

	U32 tCnt = 0;

	while( true )
	{
		S32 ret = CTP_GetMsg( (CTP_DEV*)this->pCTPDev, ptr->nodeId, ptr->hostAdr,ptr->size );

		if( ret == BRDerr_WAIT_TIMEOUT )
		{
			tCnt++;

			if( ptr->timeout == -1 )
				continue;

			if( tCnt > ptr->timeout )
				return BRDerr_WAIT_TIMEOUT;

			continue;
		}

		if( ret == DRVERR(BRDerr_OK))
		{
			break;
		}
	}

    return DRVERR(BRDerr_OK);
}


//=********************** CMD_putMsg *******************
//=*****************************************************
S32 BUNI2Driver::putMsg(  void *pParam )
{
	DEV_CMD_PutMsg  *ptr = (DEV_CMD_PutMsg*)pParam;

	U32 tCnt = 0;

	while( true )
	{
		S32 ret = CTP_PutMsg( (CTP_DEV*)this->pCTPDev, ptr->nodeId, ptr->hostAdr,ptr->size );

		if( ret == BRDerr_WAIT_TIMEOUT )
		{
			tCnt++;

			if( ptr->timeout == -1 )
				continue;

			if( tCnt > ptr->timeout )
				return BRDerr_WAIT_TIMEOUT;

			continue;
		}

		if( ret == DRVERR(BRDerr_OK))
		{
			break;
		}
	}

    return DRVERR(BRDerr_OK);
}



//=********************** CMD_signalSend ******************
//=********************************************************
S32 BUNI2Driver::signalSend(  void *pParam )
{
	DEV_CMD_SignalSend  *ptr = (DEV_CMD_SignalSend*)pParam;

	S32 ret = CTP_Signal( (CTP_DEV*)this->pCTPDev, ptr->nodeId, CTP_SIGNAL_SEND, ptr->sigId, 0 );

    return ret;
}

//=********************** CMD_signalGrab ******************
//=*****************************************************
S32 BUNI2Driver::signalGrab(  void *pParam )
{
	DEV_CMD_SignalGrab  *ptr = (DEV_CMD_SignalGrab*)pParam;

	U32 tCnt = 0;

	while( true )
	{
		S32 ret = CTP_Signal( (CTP_DEV*)this->pCTPDev, ptr->nodeId, CTP_SIGNAL_GRAB, ptr->sigId, &ptr->sigCounter );

		if( ret == BRDerr_WAIT_TIMEOUT )
		{
			tCnt++;

			if( ptr->timeout == -1 )
				continue;

			if( tCnt > ptr->timeout )
				return BRDerr_WAIT_TIMEOUT;

			continue;
		}

		if( ret == DRVERR(BRDerr_OK))
		{
			break;
		}
	}

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_signalWait ******************
//=*****************************************************
S32 BUNI2Driver::signalWait(  void *pParam )
{
	DEV_CMD_SignalWait  *ptr = (DEV_CMD_SignalWait*)pParam;

	U32 tCnt = 0;

	while( true )
	{
		S32 ret = CTP_Signal( (CTP_DEV*)this->pCTPDev, ptr->nodeId, CTP_SIGNAL_WAIT, ptr->sigId, &ptr->sigCounter );

		if( ret == BRDerr_WAIT_TIMEOUT )
		{
			tCnt++;

			if( ptr->timeout == -1 )
				continue;

			if( tCnt > ptr->timeout )
				return BRDerr_WAIT_TIMEOUT;

			continue;
		}

		if( ret == DRVERR(BRDerr_OK))
		{
			break;
		}
	}

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_signalIack ******************
//=*****************************************************
S32 BUNI2Driver::signalIack(  void *pParam )
{
	DEV_CMD_SignalIack  *ptr = (DEV_CMD_SignalIack*)pParam;
	S32 ret = CTP_Signal( (CTP_DEV*)this->pCTPDev, ptr->nodeId, CTP_SIGNAL_IACK, ptr->sigId, 0 );

    return ret;
}

//=********************** CMD_signalFresh *****************
//=*****************************************************
S32 BUNI2Driver::signalFresh(  void *pParam )
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_signalInfo ******************
//=*****************************************************
S32 BUNI2Driver::signalInfo(  void *pParam )
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_signalList ***************
//=*****************************************************
S32 BUNI2Driver::signalList(  void *pParam )
{
    DEV_CMD_SignalList  *ptr = (DEV_CMD_SignalList*)pParam;
    
    *(ptr->pItemReal) = 0;
/*
    //
    //=== Check List Size
    //
    if( ptr->item < 2 )
    {
        return DRVERR(BRDerr_BUFFER_TOO_SMALL);
    }

    //
    //=== Fill List
    //
    ptr->pList[0].sigId   = 0x1;
    ptr->pList[0].sigCode = 0x0;
    ptr->pList[0].sigAttr = BRDsig_ATTR2HOST;
    strcpy( (char*)ptr->pList[0].sigDescription, "IRQ from Device to HOST" );

    ptr->pList[1].sigId   = 0x2;
    ptr->pList[1].sigCode = 0x0;
    ptr->pList[1].sigAttr = BRDsig_ATTR2DEVICE;
    strcpy( (char*)ptr->pList[1].sigDescription, "IRQ from HOST to Device" );
*/
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_devHardwareInit **********
//=*****************************************************
S32 BUNI2Driver::devHardwareInit( void *pParam)
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_devHardwareCleanup *******
//=*****************************************************
S32 BUNI2Driver::devHardwareCleanup( void *pParam)
{
    //DEV_CMD_DevHardwareCleanup  *ptr = (DEV_CMD_DevHardwareCleanup*)pParam;

    //
    // TODO: Make operation
    //

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_subHardwareInit **********
//=*****************************************************
S32 BUNI2Driver::subHardwareInit( void *pParam)
{
    //DEV_CMD_SubHardwareInit *ptr = (DEV_CMD_SubHardwareInit*)pParam;

    //
    // TODO: Make operation
    //

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_subHardwareCleanup *******
//=*****************************************************
S32 BUNI2Driver::subHardwareCleanup(  void *pParam )
{
    //DEV_CMD_SubHardwareCleanup  *ptr = (DEV_CMD_SubHardwareCleanup*)pParam;

    //
    // TODO: Make operation
    //

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_getServList ***************
//=*****************************************************
S32 BUNI2Driver::getServList(  void *pParam )
{
    DEV_CMD_GetServList *ptr = (DEV_CMD_GetServList*)pParam;

    if( ptr==NULL )
        return DRVERR(BRDerr_BAD_PARAMETER);

	int len = _servList.get_length();

	if( ptr->item < len )
	{
		ptr->itemReal = len;
		return -1;
	}

	DEV_CMD_ServListItem	*pSLI = ptr->pSLI;

	for( int i=0; i<len;i++ )
	{
		BRDC_strcpy( pSLI[i].name , _servList[i]->name );
		pSLI[i].attr = _servList[i]->attr;
		pSLI[i].idxMain = i;
	}
	
	ptr->itemReal = len;

    return 0;//Serv_GetServList( this, ptr );
}

S32  BUNI2Driver::REGREADDIR       (  DEVS_CMD_Reg *pParam )
{
	U32 val;

	S32 err = CTP_Read( (CTP_DEV*)pCTPDev, 0, CTP_ADM_DIR, ENCODE_TETR_REG( pParam->tetr,pParam->reg), &val, sizeof(U32), -1  );

	if (err < 0)
		return BRDerr_ERROR;

	pParam->val = val;

	return BRDerr_OK;
}

S32  BUNI2Driver::REGREADSDIR       (  DEVS_CMD_Reg *pParam )
{


	CTP_Read( (CTP_DEV*)pCTPDev, 0, CTP_ADM_DIR, ENCODE_TETR_REG( pParam->tetr,pParam->reg), pParam->pBuf, pParam->bytes, -1  );


	

	return BRDerr_OK;
}

S32  BUNI2Driver::REGREADIND      (  DEVS_CMD_Reg *pParam )
{
	U32 val;

	S32 err = CTP_Read( (CTP_DEV*)pCTPDev, 0, CTP_ADM_IND, ENCODE_TETR_REG( pParam->tetr,pParam->reg), &val, sizeof(U32), -1  );

	if (err < 0)
		return BRDerr_ERROR;

	pParam->val = val;

	return BRDerr_OK;
}

S32  BUNI2Driver::REGREADSIND       (  DEVS_CMD_Reg *pParam )
{


	CTP_Read( (CTP_DEV*)pCTPDev, 0, CTP_ADM_IND, ENCODE_TETR_REG( pParam->tetr,pParam->reg), pParam->pBuf, pParam->bytes, -1  );


	

	return BRDerr_OK;
}

S32  BUNI2Driver::REGWRITEDIR       (  DEVS_CMD_Reg *pParam )
{
	U32 val = pParam->val;

	CTP_Write( (CTP_DEV*)pCTPDev, 0, CTP_ADM_DIR, ENCODE_TETR_REG( pParam->tetr,pParam->reg), &val, sizeof(U32), -1  );

	return DRVERR(BRDerr_OK);
}
S32  BUNI2Driver::REGWRITESDIR       (  DEVS_CMD_Reg *pParam )
{
	
	CTP_Write( (CTP_DEV*)pCTPDev, 0, CTP_ADM_DIR, ENCODE_TETR_REG( pParam->tetr,pParam->reg), pParam->pBuf, pParam->bytes, -1  );

		 return BRDerr_OK;
}
S32  BUNI2Driver::REGWRITEIND       (  DEVS_CMD_Reg *pParam )
{
		U32 val = pParam->val;

	CTP_Write( (CTP_DEV*)pCTPDev, 0, CTP_ADM_IND, ENCODE_TETR_REG( pParam->tetr,pParam->reg), &val, sizeof(U32), -1  );
	
	return BRDerr_OK;

}
S32  BUNI2Driver::REGWRITESIND       (  DEVS_CMD_Reg *pParam )
{
		CTP_Write( (CTP_DEV*)pCTPDev, 0, CTP_ADM_IND, ENCODE_TETR_REG( pParam->tetr,pParam->reg), pParam->pBuf, pParam->bytes, -1  );

		
		return BRDerr_OK;
}

//
// End of file
//
