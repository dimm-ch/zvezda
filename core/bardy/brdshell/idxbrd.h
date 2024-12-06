//=*******************************************************
//
// IDXBRD.H - Class CIdxBrdCollection
//
// BRD Initialization Helper functions definition.
//
// (C) Instrumental Systems 2002-2004
//
// Created: Ekkore Aug 2004
//
//=*******************************************************

//
//====== Types: CIdxBrdCollection
//

typedef struct
{
	BRDCHAR	name[32];
	int		brdIdx;
} CIdxBrdItem;

class CIdxBrdCollection
{
public:

	CIdxBrdItem			*paIdxBrdItem;
	int					maxsize;

	         CIdxBrdCollection( int aMaxsize = 64 );
	virtual ~CIdxBrdCollection(void);

	void	Clear(void);
	void	PutName( BRDCHAR *pName );
	int		GetIdxBrd( BRDCHAR *pName );
	void	IncrementIdxBrd( BRDCHAR *pName );

protected:
	int		GetItem( BRDCHAR *pName );
};


//
// End of File
//

