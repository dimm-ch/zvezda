//=*******************************************************
//
// CAPTOR.CPP - Class CCaptor
//
// BRD Initialization Helper functions definition.
//
// (C) Instrumental Systems 2002-2003
//
// Created: Ekkore Feb 2003
//
//=*******************************************************

//
//====== Types: CFineDestructor, CKillerCollection, CCaptor
//


//
//====== CFineDestructor, CKillerCollection, CCaptor
//
class CFineDestructor
{
public:
	         CFineDestructor(void){};
	virtual ~CFineDestructor(void){};
};

//
//====== CCaptorCollection
//
class CCaptorCollection
{
	//
	//=== Variables
	//
	CFineDestructor		**m_coll;	// Array of Pointers
	int					m_size;		// Current size of m_coll[]
	int					m_delta;	// Adder for size

public:

	//
	//=== Methods
	//
	CCaptorCollection( int size=20, int delta=10 )
	{
		m_size = size;
		m_delta= delta;
		m_coll = (CFineDestructor**)calloc(sizeof(CFineDestructor*),m_size);
	};

	~CCaptorCollection()
	{ 
		for(int i=0;i<m_size;i++) 
			if( m_coll[i]!=NULL )
				delete m_coll[i];
		free( m_coll );
	};
	int reg( CFineDestructor *ptr )
	{ 
		for(int i=0;i<m_size;i++) 
			if( m_coll[i]==NULL )
			{
				m_coll[i] = ptr;
				return 0;
			}
		//
		// If Collection is Full, Increase It
		//
		CFineDestructor		**collTmp;	// Temporary Array of Pointers

		collTmp = (CFineDestructor**)calloc(sizeof(CFineDestructor*),m_size+m_delta);
		if( collTmp==NULL )
			return -1;
		memcpy( (void*)collTmp, (void*)m_coll, sizeof(CFineDestructor*)*m_size );
		free( (void*)m_coll );
		m_coll = collTmp;
		m_coll[m_size] = ptr;
		m_size += m_delta;

		return 0;
	};
	int unreg( CFineDestructor *ptr )
	{ 
		for(int i=0;i<m_size;i++) 
			if( m_coll[i]==ptr )
			{
				m_coll[i] = NULL;
				delete ptr;
				return 0;
			}
		return -1;
	};
};

extern CCaptorCollection	g_captorCollection;

//
//====== CCaptor
//
class CCaptor : public CFineDestructor
{
public:

	//
	//=== Types
	//
	typedef struct
	{
		long		m_nState;		// Resource State (0 - UNUSED, +1...+N - SHARED; -1 - EXCLUSIVE )
	} SHAREDINFO;

	//
	//=== Variables
	//
	int				m_nCounter;		// Counter of Release()'s
	long			m_nState;		// Resource State (0 - UNUSED, +1...+N - SHARED; -1 - EXCLUSIVE )
	IPC_handle		m_hMutex;		// Protection Data Mutex
	IPC_handle		m_hEvent;		// Resource UNUSED State Event
	IPC_handle		m_hFM;			// File Mapping Handle to Share SHAREDINFO Structure
	SHAREDINFO*		m_pSI;			// Shared Structure Pointer


	//
	//=== Methods
	//
	CCaptor( const BRDCHAR *devName, const BRDCHAR *resName );
	~CCaptor( void );

	int		WaitForShared( DWORD dwMilliseconds );
	int		WaitForExclusive( DWORD dwMilliseconds );
	int		Release( void );
	BOOL	IsCapturedHandle( void );
	BOOL	IsValid( void );
	
	//=********************* CCaptor::GetCaptureMode ********************
	//=******************************************************************
	int		GetCaptureMode()
	{
		int		tmp = m_nState;

		return	(tmp==0) ? 2 : // SPY
				(tmp >0) ? 1 : // SHARED
						   0;  // EXCLUSIVE
	}
	
	//=************************ CCaptor::GetCounter ********************
	//=******************************************************************
	int		GetCounter()
	{
		return m_nCounter;
	}
};

//
// End of File
//

