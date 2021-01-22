
#define MENU_ABOUT		1
#define MENU_CMDSTR		2

enum eColor { EMPTY, WHITE, BLACK, NONE, BOTH, ANALYZE_MODE };
constexpr int NUM_STAGES = 7;

HINSTANCE       MyInstance;
static char     PointyClass[32]="PointyClass";
HBITMAP square_bmp, white_bmp, black_bmp;
HBITMAP board_bmp, msquare_bmp, skinbrush;
HBITMAP flipBitmaps[6];
HCURSOR bcursor, wcursor, waitCursor, arrowCursor;
HMENU  oldmenu;

char GameBoard[66];
char OldBoard[66];
char SearchDepth = 16;
char EndgameDepth = 1;
char oldDepth = 1;
int SetupBoard = 0, DontErase = 0;

volatile int stopThinking = 0;

enum eThinkType
{
	NOT_THINKING,
	MOVE,
	SUGGEST,
	ANALYZE,
	PERFECT,
	WINLOSS
};

eThinkType thinkingType = MOVE;

int g_suggestSquare = -1;

int OpeningsType = 0, ExtraHashBits = 3, TimePause = 5, PauseOnlyPass = 1;

int BlackTime = 0, WhiteTime = 0;
clock_t starttime, endtime ;
char TimeLimit = 15, ShowMoves = 0, SearchInfo, NoFlip = 1, NoCursor = 0, TimeDependent, TimeLimitHit = 0;
int RotateBoard = 0, flipi = -1;
bool bGameover = false;
int ComputerColor = WHITE;

int MoveNum = 1;
int lastMove;
int trannum = 1;
int experiment = 0;
bool g_bSaveTraining = false;
volatile int thinking = 0;
int filegame = 0;

int mousex = 0, mousey = 0;
HWND GameStats = nullptr;
HWND MainWnd = nullptr;

HGLOBAL clipTranscript;
char Transcript[180], CopyTranscript[5][180], UndoTranscript[180]; // Transcript 0 = Current Move #
char StartBoard[65], StandardBoard[65], CopyBoard[5][65], UndoBoard[65];
char WhitePlayer[201], BlackPlayer[201], HumanPlayer[201];
char DefaultName[201], Defaultip[66];
char Comment[64][1000];

char Oname[200][50];
char Oscript[200][50];

#define GET_HINST( hwnd )      (HINSTANCE)GetWindowLong( hwnd, GWL_HINSTANCE );

int Moves( );
void DisplayText ( char *str, bool delayed = false );
static void DrawBitmap( HDC hdc, HBITMAP bitmap, int x, int y );
void GetDiscs( int &Black, int &White );
void ReplayGame( char board[], char Transcript[] );
void DrawBoard( HDC hdc, char board[65] );
INT_PTR CALLBACK GameDlgProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );
                               
static BOOL FirstInstance( HINSTANCE );
static BOOL AnyInstance( HINSTANCE, int, LPSTR );

void AddTranscript (int x, int y, char InTranscript[]);
void DoTakeBack ( char Transcript[] );
void ReadTranscriptClipBoard( char *clip, int nNeededMoves );

LRESULT CALLBACK WindowProc( HWND, UINT, WPARAM, LPARAM );
void init_Board( char board[65], int start);
void DoMoveXY( int x, int y, char Board[], char InTranscript[], int bFlip);
void CheckGameOver (char Board[], int &bGameResult);
void ComputerTourney ( int startgame, int endgame, int step, char *filename, int Depth);
void send_move(char *move);
int wait_move( );

void StandardizeTranscript( char* transcript );
int Pieces( char board[65] );
void CopyGame( int num);
int DoMove( int x, int y, char board[65], char* pTranscript );
int FindMoves( char board[65], char color);
void psChangeCursor( );

void NumMarkers( char board[65], int &numwhitem, int &numblackm);
int GetStartBoard( int i, char Transcript[] );

int LoadAllCoeff( );
int uncompressFileFromArchive( char *filename, char *compFile, unsigned char *OutputTable);