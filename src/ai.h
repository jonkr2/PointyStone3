// DEFINITIONS
// enum ePiece { EMPTY, WHITE, BLACK };

constexpr int PASS = -100000;
constexpr int TIMEOUT = -99999;
constexpr int EMPTYEND = 7;

constexpr int INVALID_SQ = 66;
constexpr int INVALID_MOVE = 127;
constexpr int INVALID_VALUE = -9999;
constexpr int UNKNOWN_VALUE = -1001;

const int Location[16] = { 0, 0, 9801, 9720, 9477, 8748, 6561, 0, 6561, 8748, 9477, 9720, 9801, 0, 0 };
const int Power3[19] = { 0, 1, 3, 9, 27, 81, 81*3, 81*9, 81*27, 81*81, 81*81*3, 81*81*9 };

// STRUCTURES
struct SingleMove
{
	uint8_t sq, direction; // square of the move, and the directions it flips
	int16_t eval;
};

struct Flipped
{
	char left, right;
};

struct Move
{
	uint8_t WMoves[4]; //  white/black
	uint8_t BMoves[4];
};

struct Move2
{
	Flipped WF[4]; // Left/Right marks of slices For flipping pieces
	Flipped BF[4]; 
};

struct SqMove
{
	 Flipped W[8]; // Left/Right marks of slices For flipping pieces
     Flipped B[8];
};

// The GameBoard structure.       
struct Board
{
	// pattern indexes
	uint16_t east[9];
	uint16_t south[9];
	uint16_t southeast[15];
	uint16_t southwest[15];
	uint16_t corners[4];

	uint32_t HashKey, HashCheck;
	int16_t eval;
	uint8_t nummarkers, numblack;
	int8_t color, extra;
	int8_t edge1, edge2, edge3, edge4;
	int8_t cornerThreat, cornerThreat2;
};

// For Conversions      
struct BoardLookup2
{
	int8_t east[10*8];
	int8_t south[10*8];
	int8_t southeast[16*8];
	int8_t southwest[16*8];
	int8_t corner[10 * 4];
};

// Extra info taken from the diagonal
struct DiagInfo
{
	short XSquare1, XSquare2, CornerTake1, CornerTake2;
};

struct EdgeInfo
{
	int32_t First5;
	int32_t Last5;
	int32_t Middle6;
	int32_t Middle4;
	int32_t Stable3;
	int32_t Stable6;
	int32_t X1;
	int32_t X2;
};

struct EvaluateP
{
	uint8_t* EdgeX2; 
	uint8_t* EdgeM2;
	uint8_t* Corner2;
	uint8_t* Corner25;

	uint8_t* Slice8CF1;
	uint8_t* Slice8DE1;
	uint8_t* Diag8;
	uint8_t* Diag7;
	uint8_t* Diag6;
	uint8_t* Diag5;
	uint8_t* Diag4;
	float parity, sideTM;
};

const int STACK_SIZE = 66;
const int BOOK_MAX = 15000;

const int gameStage[100] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,4,1,1,1,1,1,1,1,5,5,5,5,5,5,2,2,2,2,2,2,2,6,6,6,6,6,6,3,3,3,3,3,3,3,3,3,3,3,3,3,3};

const unsigned int SortDepth[64] = { 0, 0, 1, 1, 2, 3, 4, 5, 6, 6, 7, 7, 8, 9, 10, 10, 11, 12, 13, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 14};

const int cutLevel[65] = 
{ 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17, 17, 18, 18, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 28, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30};

// Switch from Middle Edge to Edge + 2X patterns
const int EdgeSwitch[64] = 
{1, 0, 0, 0, 0, 0, 0, 2,
 0, 1, 0, 0, 0, 0, 2, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 3, 0, 0, 0, 0, 4, 0,
 3, 0, 0, 0, 0, 0, 0, 4
};

// An edge move was played
const int EdgePlay[64] = 
{0, 1, 1, 1, 1, 1, 1, 0,
 3, 0, 0, 0, 0, 0, 0, 4,
 3, 0, 0, 0, 0, 0, 0, 4,
 3, 0, 0, 0, 0, 0, 0, 4,
 3, 0, 0, 0, 0, 0, 0, 4,
 3, 0, 0, 0, 0, 0, 0, 4,
 3, 0, 0, 0, 0, 0, 0, 4,
 0, 2, 2, 2, 2, 2, 2, 0
};


const int CSquares[64] = 
{0, 1, 0, 0, 0, 0, 1, 0,
 3, 0, 0, 0, 0, 0, 0, 4,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 3, 0, 0, 0, 0, 0, 0, 4,
 0, 2, 0, 0, 0, 0, 2, 0
};

// FUNCTIONS
void DisplayNodes(int move, int eval, int inbook, int ticks, int question );
void SavePosition(Board &CBoard, char *filename, unsigned char solved);
int AlphaBetaSearch(int color, unsigned int ahead, int alpha, int beta, int &bestmove);
void FindMoves(char slice[9], int slicenum, Move &Eval, Move2 &Eval2, SqMove &Sq, int length, char color, int offset);
void InitConversionTable();
void SliceEval(Move &Eval, Move2 &Eval2, char slice[9], int slicenum);
void FirstOnes();