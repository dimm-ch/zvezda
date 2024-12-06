//=********************************************************
// BSERV.H - BARDY Service Support Declaration Header File
//
// Class Definitions:
//    SERV_Collection
//    SERV_ServiceCollection
//=********************************************************

#ifndef	__BSERV_H_
#define	__BSERV_H_

#include	"brdapi.h"
#include    "gipcy.h"

//=******************** SERV_Collection *******************
//=********************************************************
class SERV_Collection
{
	S32		m_itemSize;		// Size of Item (bytes)
	S32		m_size;			// Number of Filled Items
	S32		m_maxSize;		// Total Number of Items
	S32		m_delta;		// Delta
	void	*m_ptr;			// Array Pointer

public:

	SERV_Collection( S32 itemSize, S32 maxSize=25, S32 delta=10 );
	~SERV_Collection(void);

    PVOID operator new(size_t Size)   {return IPC_heapAlloc(Size);};
    VOID operator delete(PVOID pVoid) {if(pVoid) IPC_heapFree(pVoid); };
	
	
	S32		IsValid( void ) { return (m_ptr) ? 0 : -1; };
	void*	GetArray(void)  { return m_ptr; };
	S32		GetSize(void)   { return m_size; };
	S32		Include( void *pItem );
};
typedef SERV_Collection *PSERV_Collection;

//=********** SERV_ServiceCollection typedefs ************
//=********************************************************
typedef S32 SERV_CmdEntry( 
			BRD_Handle	handle,		// Service Index into Main Service List and Capture Mode
			void		*pSide,		// InfoSM or InfoBM
			void		*pServData,	// Specific Service Data
			void		*pContext,	// Specific Data to send to TECH Driver (for SIDE driver only)
			U32			cmd,		// Command Code (from BRD_ctrl())
			void		*args 		// Command Arguments (from BRD_ctrl())
		);

typedef struct
{
	U32			cmd;			// Command Code
	U32			isSpy:1,		// 0-Control Cmd, 1-Monitoring Cmd
				isCapture:1,	// 0-don't capture the Service, 1-capture the Service if need
				isRelease:1;	// 0-don't release the Service, 1-release the Service if need
	SERV_CmdEntry	*pEntry;	// Command Support Entry Point
} SERV_CmdListItem, *PSERV_CmdListItem;


typedef struct
{
	BRDCHAR			name[16];		// Service name with Number
	U32				attr;			// Attributes of Service (Look BRDserv_ATTR_XXX constants)
	S32				idxMain;		// Index of this Service into TECH/DUPLEX Main Service List
	void			*pServData;		// Specific Service Data
	SERV_CmdListItem	*pCmd;		// Service Command List
} SERV_ServiceListItem, *PSERV_ServiceListItem;


//=**************** SERV_ServiceCollection ***************
//=********************************************************
class SERV_ServiceCollection : public SERV_Collection
{
public:
	SERV_ServiceCollection(S32 maxSize=10, S32 delta=5);
	~SERV_ServiceCollection(void);

    PVOID operator new(size_t Size)   {return IPC_heapAlloc(Size);};
    VOID operator delete(PVOID pVoid) {if(pVoid) IPC_heapFree(pVoid); };
	
	S32		IncludeService( const BRDCHAR *pName, U32 attr, S32 idxMain, void *pServData, SERV_CmdListItem *pCmd, S32 cmdSize );
	S32		GetServiceList( SIDE_CMD_GetServList *pGSL );
	S32		SetServiceList( SIDE_CMD_SetServList *pSSL );
	S32		Ctrl( BRD_Handle handle, void *pSide, void *pContext, U32 cmd, void *args );

protected:
	SERV_ServiceListItem	*GetItem( S32 itemNo )
	{ return ((SERV_ServiceListItem*)GetArray()) + itemNo; };

};
typedef SERV_ServiceCollection *PSERV_ServiceCollection;

#endif //__BSERV_H_

//
// End of File
//


