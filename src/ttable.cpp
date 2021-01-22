// ------------
//
// TRANSPOSITION TABLE
//
// ------------
struct HashEntry
{	
	uint32_t checksum;
	int16_t eval;
	uint8_t bestmove, depthFail;
}; // 8 bytes

struct HashPair
{
	HashEntry Near, Far;
};

HashPair* HashTable = nullptr;
HashPair *TTpair = nullptr;
int g_Age = 0;
int	HASHTABLE_SIZE2 = 0;

// ---------------------------------
// Initialize square values for the hash table
// ---------------------------------
void InitRandomHash( )
{
	srand( 1001 );

	for (int i = 0; i < 64; i++)
	{
		Boardindex [i * 16 + 10] = rand() * 16777216 + rand() * 65536 + rand() * 256 + rand();
		Boardindex [i * 16 + 11] = rand() * 16777216 + rand() * 65536 + rand() * 256 + rand(); // xor white
		Boardindex [i * 16 + 12] = rand() * 16777216 + rand() * 65536 + rand() * 256 + rand();
		Boardindex [i * 16 + 13] = rand() * 16777216 + rand() * 65536 + rand() * 256 + rand(); // xor disc placed
	}
}

// ---------------------------------
// Find the HashEntry Key and the Checksum from the slices of a board
// ---------------------------------
void inline HashBoard (const Board &CBoard, char color, unsigned int &HashKey, unsigned int &HashCheck)
{
	HashKey   = CBoard.HashKey;
	HashCheck = CBoard.HashCheck;

	if (color == BLACK) 
	{
		HashKey   += 314721;
		HashCheck += 3214521;
	}
}

//
// Increment age on hash entry
// 
void inline TT_AgeInc( )
{
	g_Age = (g_Age + 1) & 15;
}

void inline TT_Clear( )
{
	memset( HashTable, 0, HASHTABLE_SIZE2 * sizeof(HashPair) );
}

//-------------------------------
// Change the size of the HashEntry Table
//-------------------------------
void TT_Allocate( int ExtraHashBits, HWND hwnd)
{
	if (ExtraHashBits < 0) ExtraHashBits = 0;
	if (ExtraHashBits > 7) ExtraHashBits = 7;

	if (HashTable != NULL) free( HashTable );

	HASHTABLE_SIZE2 = (1 << (18 + ExtraHashBits) ) - 1; 
	HashTable = (HashPair *) malloc( (HASHTABLE_SIZE2 + 16) * sizeof(HashPair) );

	if (HashTable == NULL) 
	{
		MessageBox (hwnd, "Error Allocating Hash Table", "Error", MB_OK | MB_ICONERROR);
		ExtraHashBits = 0;
		HASHTABLE_SIZE2 = (1 << (18 + ExtraHashBits) ) - 1; 
		HashTable = (HashPair*) malloc( (HASHTABLE_SIZE2 + 16) * sizeof(HashPair) );
	}
	TT_Clear();
}

// ------------
// Lookup an Entry
// ------------
void inline TT_Read( HashEntry* TTentry, int &eval, int &bestmove, const unsigned int &depth, const int &alpha, const int &beta )
{
	 unsigned int Tfailtype = (TTentry->depthFail >> 6);
	 unsigned int Tdepth	= (TTentry->depthFail & 63);

	if (Tdepth >= depth )
	{
		int Teval = (TTentry->eval >> 5);
		if (Tfailtype == 0) eval = Teval;
		else if (Tfailtype == 2) {
			 if (Teval >= beta)
				 eval = Teval;
		}
		else if (Tfailtype == 1) {
			if (Teval <= alpha)
			eval = Teval;
		}
	}
	// Check the previous best move first if stored value not deep enough or not within the window
    if ( (final!=1 && Tdepth > 3) || Tdepth >=8 ) bestmove = TTentry->bestmove; 
}

void inline TT_Lookup( int &eval, int &bestmove, const int &alpha, const int &beta, const uint32_t &ahead, const int &color, unsigned int &hi, unsigned int &hashvalue )
{
	if (ahead >= (CurrentAhead - 1) || hashing != 1 ) return;

	HashBoard( BoardStack[ahead], color, hi, hashvalue );
	TTpair  = &HashTable[ (hi & HASHTABLE_SIZE2) ];

	if (TTpair->Near.checksum == hashvalue)	{
		TT_Read( &TTpair->Near, eval, bestmove, CurrentAhead-ahead, alpha, beta);
    }
	else if (TTpair->Far.checksum == hashvalue)	{
		TT_Read( &TTpair->Far, eval, bestmove, CurrentAhead-ahead, alpha, beta);
	}
}

// Used only for displaying human readable info
bool inline TT_GetInfo( const Board& CBoard, int &eval, char &failTypeChar, int &bestmove, int& depth )
{
	unsigned int hi, hashvalue;
	HashBoard( CBoard, CBoard.color^3, hi, hashvalue );
	TTpair  = &HashTable [ (hi & HASHTABLE_SIZE2) ];
	HashEntry* entry = NULL;

	if ( TTpair->Near.checksum == hashvalue )
		entry = &TTpair->Near;
	else if ( TTpair->Far.checksum == hashvalue)
		entry = &TTpair->Far;

	if ( !entry )
		return false;

	bestmove = entry->bestmove;
	eval = (entry->eval>>5);
	int failType = (entry->depthFail >> 6);
	if ( failType == 1 ) 
		failTypeChar = '<';
	else if ( failType == 2 )
		failTypeChar = '>';
	else 
		failTypeChar = ' ';

	depth = (entry->depthFail & 63);
	return true;
}

// ------------
// Store an Entry
// ------------
void inline TT_Store( HashEntry* TTentry, int &eval, int &bestmove, const unsigned int &depth, const int &alpha, const int &beta, const unsigned int &hashvalue )
{
	unsigned int Tfailtype;
	if (eval <= alpha) Tfailtype = 1;
	else if (eval >= beta) Tfailtype = 2;
	else Tfailtype = 0;

	TTentry->checksum = hashvalue;
	TTentry->eval = (eval<<5) + g_Age;
	TTentry->depthFail = (depth) + (Tfailtype << 6);
	TTentry->bestmove = bestmove;
}

void inline TT_Write( int &eval, int &bestmove, const int &alpha, const int &beta, const uint32_t &ahead, const unsigned int &hi, const unsigned int &hashvalue  )
{
	if (ahead >= (CurrentAhead - 1) || hashing != 1 ) return;

	TTpair  = &HashTable[ (hi & HASHTABLE_SIZE2) ];
	if ( uint32_t(TTpair->Near.depthFail&63) <= CurrentAhead-ahead || ((TTpair->Near.eval&15) != g_Age && TTpair->Near.checksum != hashvalue) )
	{
		TT_Store( &TTpair->Near, eval, bestmove, CurrentAhead - ahead, alpha, beta, hashvalue );
		return;
	}
	TT_Store( &TTpair->Far, eval, bestmove, CurrentAhead - ahead, alpha, beta, hashvalue );
}
