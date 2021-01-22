// ------------------------------------\
// Pointy Stone 3 Windows GUI
//
// by Jonathan Kreuzer
// ------------------------------------/
#define _CRT_SECURE_NO_DEPRECATE
#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <process.h>
#include "resource.h"
#include "gui.h"

enum class eGameResult { WHITEWINS = 1, BLACKWINS = 2, NONE = 3, DRAW = 4 };

eGameResult g_GameResult = eGameResult::NONE;

const char* versionName = "Pointy Stone 3.5 Othello";

// ------ AI Functions -----------
#include "ai.cpp"

// ----- Learning Functions -----
#include "learn.cpp"

// Transcript importing/exporting functions
#include "transcript.cpp"

void ToggleAnalyzeMode();

char displayString[4192];
bool stringToDisplay = false;

bool ComputerIsThinking( bool stopIfPossible = false)
{
	if ( ComputerColor == ANALYZE_MODE )
	{
		if ( stopIfPossible )
			stopThinking = 1;

		return false;
	}
	return ( thinking == 1 );
}

/* ------------------------------------
 * WinMain - initialization, message loop
 * ------------------------------------- */
int WINAPI WinMain( HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow )
{
	MSG msg;

	MyInstance = this_inst;
	if( !FirstInstance( this_inst ) ) return( FALSE );
	if( !AnyInstance( this_inst, cmdshow, cmdline ) ) return( FALSE );

	while( GetMessage( &msg, NULL, NULL, NULL ) ) {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
	}

    return (int)( msg.wParam );
} 

void SetTitle( const char *str, const char *subTitle = nullptr )
{
	char tempName[1024];
	if (subTitle) {
		snprintf(tempName, sizeof(tempName), "%s - %s ", str, subTitle);
		SetWindowText(MainWnd, tempName);
	} else {
		SetWindowText(MainWnd, str);
	}
}

void DisplayText( char *str, bool delayed )
{
	if ( delayed )
	{
		stringToDisplay = true;
		strncpy( displayString, str, sizeof( displayString) );
	}
	else
	{
		SetDlgItemText(GameStats, 112, str);
	}
}

//
void EraseComments()
{
	memset( Comment, 0, 64*1000 );
}

int Pieces( char board[65] )
{
    int p = 0;

	for (int x = 0; x < 64; x++) {
		if (board[x] != EMPTY) p++;
	}

    return p;
}

void SaveDefaults()
{ 
	FILE* input = fopen("defaults.dat", "wt");
	if (input)
	{
		fwrite(&SearchDepth, 1, 1, input);
		fwrite(&TimeLimit, 1, 1, input);
		fwrite(&ShowMoves, 1, 1, input);
		fwrite(&SearchInfo, 1, 1, input);
		fwrite(&NoFlip, 1, 1, input);
		fwrite(&ExtraHashBits, 1, 4, input);
		fwrite(&OpeningsType, 1, 4, input);
		fwrite(&brainType, 1, 4, input);
		fwrite(&TimePause, 1, 4, input);
		fwrite(&PauseOnlyPass, 1, 4, input);

		int length = (int)strlen(DefaultName);
		fwrite(&length, 1, 4, input);
		fwrite(DefaultName, 1, length, input);

		fclose(input);
	}
}

void LoadDefaults( )
{	
	// TODO : make this a txt format 
	FILE* input = fopen( "defaults.dat" , "rt" );          
	if (input)
	{
		fread(&SearchDepth, 1, 1, input);
		fread(&TimeLimit, 1, 1, input);
		fread(&ShowMoves, 1, 1, input);
		fread(&SearchInfo, 1, 1, input);
		fread(&NoFlip, 1, 1, input);

		fread(&ExtraHashBits, 1, 4, input);
		fread(&OpeningsType, 1, 4, input);
		fread(&brainType, 1, 4, input);

		fread(&TimePause, 1, 4, input);
		fread(&PauseOnlyPass, 1, 4, input);

		int length = 0;
		fread(&length, 1, 4, input);
		fread(DefaultName, 1, length, input);

		strcpy(HumanPlayer, DefaultName);

		fclose(input);
	}
}

void SaveGame( FILE *output )
{ 
    fwrite(Transcript, 124, 1, output);
    fwrite(StartBoard, 65, 1, output);
    strcat(BlackPlayer, "\n");
    strcat(WhitePlayer, "\n");
         
    fputs(BlackPlayer, output);
    fputs(WhitePlayer, output);

	int16_t index[2];
	for (index[0] = 0; index[0] < 63; index[0]++)
	{
		if (Comment[index[0]][0] != NULL)
		{
			index[1] = (short)strlen(Comment[index[0]]);
			fwrite(&index[0], 4, 1, output);
			fwrite(Comment[index[0]], index[1], 1, output);
		}
	}
}
//
// Load Game

void LoadGame( FILE *output )
{
    char tempPlayer[200];

	SetupBoard = 0;
    EraseComments();
    strcpy (tempPlayer, HumanPlayer);
             
    fread(Transcript, 124, 1, output);
    fread(StartBoard, 65, 1, output);
    if (Pieces(StartBoard) == 4) {
		StartBoard[64] = BLACK;
        StartBoard[3*8 + 3] = WHITE; StartBoard[3*8 + 4] = BLACK;
        StartBoard[4*8 + 4] = WHITE; StartBoard[4*8 + 3] = BLACK;
	}
    fgets(BlackPlayer, 200, output);
    fgets(WhitePlayer, 200, output);
                                 
    BlackPlayer[strlen(BlackPlayer)-1] = NULL;
    WhitePlayer[strlen(WhitePlayer)-1] = NULL; // Get rid of extra character.

	int16_t index[2];
    while (!feof (output))
	{
		fread (&index[0], 4, 1, output);
        if (!feof (output) && index[0] < 63 && index[0] >=0 )
			fread (Comment[index[0]], index[1], 1, output);
	}

    if (GameStats!= NULL)
	{
		SetDlgItemText( GameStats, 121, WhitePlayer );
        SetDlgItemText( GameStats, 122, BlackPlayer );
    }

    strcpy( HumanPlayer, tempPlayer );
	BlackTime = 0;
	WhiteTime = 0;
	TimeLimitHit = 0;
		 
	if (Comment[ Transcript[0] ][0]!=NULL)
        DisplayText( Comment[ Transcript[0] ] );
    else    
		DisplayText( "Game Loaded" );

    CopyGame( 1 );        
}                    

// -------------
// Get StartBoards
// -------------
int GetStartBoard(int i, char Transcript[])
{
	FILE* input = fopen("outBoards.txt", "rt");
	if (input)
	{
		memset(Transcript, 0, 120);
		init_Board(GameBoard, WHITE);

		for (int n = 0; n < i; n++)
			fread(&Transcript[2], 1, 12 * 2, input);

		fclose(input);

		Transcript[0] = 12;
		return 1;
	}
	return 0;
}

// --------------------
// Read an X-O format text position
// --------------------
void ReadTextPosition(char *text)
{
	for (int i = 0; i < 64; i++)
	{
		if (text[i] == 'O') GameBoard[i] = WHITE;
		else if (text[i] == 'X') GameBoard[i] = BLACK;
		else GameBoard[i] = EMPTY;
	}
	if (text[65] == 'O') GameBoard [64] = WHITE; else GameBoard[64] = BLACK;

	memcpy (StartBoard, GameBoard, 65);
	Transcript[0] = 0;
	Transcript[2] = 0;

	DisplayText("Position Read.");
}

/*
 * FirstInstance - register window class for the application,
 *                 and do any other application initialization
 */
static BOOL FirstInstance( HINSTANCE this_inst )
{
    WNDCLASS    wc;
    BOOL        rc;
    
    //  set up and register window class     
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc =  WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof( DWORD );
    wc.hInstance = this_inst;
    wc.hIcon = LoadIcon( this_inst, "PointyIcon" );
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH) GetStockObject( WHITE_BRUSH );
    wc.lpszMenuName = "PointyMenu";
    wc.lpszClassName = PointyClass;
    rc = RegisterClass( &wc );
    return( rc );
} 

/*
 * AnyInstance - do work required for every instance of the application:
 *        +        create the window, initialize data
 */
static BOOL AnyInstance( HINSTANCE this_inst, int cmdshow, LPSTR cmdline )
{
	// Create Main Window
	HWND hwnd = CreateWindow( PointyClass, versionName,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,    
        CW_USEDEFAULT, CW_USEDEFAULT, 680, 580,          
        NULL, NULL, this_inst, NULL );
                    
    if( !hwnd ) return( FALSE );
	MainWnd = hwnd;

    EraseComments();
    UndoTranscript[0] = 100; // Set this so it knows it can't Paste/Undo Gamestate yet
    CopyTranscript[0][0] = 100;
	CopyTranscript[1][0] = 100;
	CopyTranscript[2][0] = 100;

    // Load all the bitmaps needed  (board. markers, side window skin)
    white_bmp = LoadBitmap( this_inst, "whiteb" );
    black_bmp = LoadBitmap( this_inst, "blackb" );
    square_bmp = LoadBitmap( this_inst, "squareb" );
    msquare_bmp = LoadBitmap( this_inst, "msquareb" );
    board_bmp = LoadBitmap( this_inst, "boardb" );
    // skinbrush = LoadBitmap( this_inst, "skin" );

	char* flipbm[6] = {"flip1", "flip2", "flip3", "flip4", "flip5", "flip6" };
	for (int i = 0; i < 6; i++) {
		flipBitmaps[i] = LoadBitmap(this_inst, flipbm[i]);
	}

    // Cursors
    wcursor = LoadCursor( this_inst , "wcursor");
    bcursor = LoadCursor( this_inst , "bcursor");
    waitCursor = LoadCursor( NULL, IDC_WAIT );
    arrowCursor = LoadCursor( NULL, IDC_ARROW );

    SetCursor( arrowCursor );

    LoadOpeningNames();
    LoadDefaults();   
  
	// Allocate HashEntry Table
	TT_Allocate( ExtraHashBits, hwnd );

	// Check Menus from defaults
	if (SearchDepth == 1)  {CheckMenuItem( GetMenu(hwnd), 51, MF_CHECKED); EndgameDepth = 4;}
	if (SearchDepth == 2)  {CheckMenuItem( GetMenu(hwnd), 52, MF_CHECKED); EndgameDepth = 6;}
	if (SearchDepth == 3)  {CheckMenuItem( GetMenu(hwnd), 53, MF_CHECKED); EndgameDepth = 8;}
	if (SearchDepth == 4)  {CheckMenuItem( GetMenu(hwnd), 54, MF_CHECKED); EndgameDepth = 10;}
	if (SearchDepth == 6)  {CheckMenuItem( GetMenu(hwnd), 55, MF_CHECKED); EndgameDepth = 12;}
	if (SearchDepth == 8)  {CheckMenuItem( GetMenu(hwnd), 56, MF_CHECKED); EndgameDepth = 16;}
	if (SearchDepth == 10) {CheckMenuItem( GetMenu(hwnd), 57, MF_CHECKED); EndgameDepth = 18;}
	if (SearchDepth == 12) {CheckMenuItem( GetMenu(hwnd), 58, MF_CHECKED); EndgameDepth = 20;}
	if (SearchDepth == 16) {CheckMenuItem( GetMenu(hwnd), 59, MF_CHECKED); EndgameDepth = 22;}
	if (SearchDepth == 18) {CheckMenuItem( GetMenu(hwnd), 60, MF_CHECKED); TimeDependent = 1;}
       
	CheckMenuItem (GetMenu(hwnd), 40 , MF_CHECKED);
    if (ShowMoves == 1) CheckMenuItem (GetMenu(hwnd), 71 , MF_CHECKED);
    CheckMenuItem (GetMenu(hwnd), 675 + NoFlip , MF_CHECKED);
    if (SearchInfo == 1) CheckMenuItem( GetMenu(hwnd), 73 , MF_CHECKED);
    if (TimeLimit == 15) CheckMenuItem( GetMenu(hwnd), 64, MF_CHECKED);
    if (TimeLimit == 1) CheckMenuItem( GetMenu(hwnd), 61, MF_CHECKED);
    if (TimeLimit == 2) CheckMenuItem( GetMenu(hwnd), 62, MF_CHECKED);
    if (TimeLimit == 3) CheckMenuItem( GetMenu(hwnd), 67, MF_CHECKED);
    if (TimeLimit == 5) CheckMenuItem( GetMenu(hwnd), 63, MF_CHECKED);
    if (TimeLimit == 10) CheckMenuItem( GetMenu(hwnd), 68, MF_CHECKED);
    if (TimeLimit == 30) CheckMenuItem( GetMenu(hwnd), 65, MF_CHECKED);
    if (TimeLimit == 0) CheckMenuItem( GetMenu(hwnd), 66, MF_CHECKED);

	CheckMenuItem (GetMenu(MainWnd), 1200 + OpeningsType, MF_CHECKED);
	CheckMenuItem (GetMenu(MainWnd), 1210 + ExtraHashBits, MF_CHECKED);
	CheckMenuItem (GetMenu(MainWnd), 1220 + brainType, MF_CHECKED);

	// Create Side Window & Set player names
	if (GameStats == NULL)
	{  
		GameStats = CreateDialog( this_inst, "CLIENT", hwnd, (GameDlgProc) );
		SetDlgItemText( GameStats, 121, WhitePlayer );
		SetDlgItemText( GameStats, 122, BlackPlayer );
		MoveWindow(GameStats, 380, 0, 300, 600, true);
	}

	init_Board( GameBoard, WHITE );

	// Game was sent in command line??
	if ( cmdline != NULL && cmdline[0]!= NULL )
	{
		char filename[1000];
		strcpy( filename, &cmdline[1]);
		filename[ strlen (filename)-1] = NULL;

		FILE* input = fopen( filename, "rb" );	   
		if (input!= nullptr)
		{
            LoadGame( input );
               
            ReplayGame(GameBoard, Transcript);
            DrawBoard (NULL, GameBoard);

		    fclose( input );
        }
	}

	CheckMenuItem( GetMenu(hwnd), 111, MF_CHECKED );

    SetTimer( hwnd, 1, 40 , NULL );

    ShowWindow( hwnd, cmdshow );
    UpdateWindow( hwnd );

    InitTables();
           
    return( TRUE );                        
} 

char OldPath[500];
// ------------------
// GetFileName - get a file name using common dialog (*.ps3)
static BOOL GetFileName( HWND hwnd, BOOL save, char *fname )
{
    static char         filterList[] = "Pointy Stone Othello (*.ps3)" \
                                        "\0" \
                                        "*.ps3" \
                                        "\0\0";
    OPENFILENAME        of;
    int                 rc;

	GetCurrentDirectory(500, OldPath);

	strcpy (fname, "*.ps3");
    memset( &of, 0, sizeof( OPENFILENAME ) );
    of.lStructSize = sizeof( OPENFILENAME );
    of.hwndOwner = hwnd;
    of.lpstrFilter = (LPSTR) filterList;
    of.lpstrDefExt = "";
    of.nFilterIndex = 1L;
    of.lpstrFile = fname;
    of.nMaxFile = _MAX_PATH;
    of.lpstrTitle = NULL;
    of.Flags = OFN_HIDEREADONLY;
    if( save ) {
        rc = GetSaveFileName( &of );
    } else {
        rc = GetOpenFileName( &of );
    }

	SetCurrentDirectory( OldPath );
    return( rc );
} 

// GetFileName - get a file name using common dialog (*.txt)
static BOOL GetFileName2(  HWND hwnd, BOOL save, char *fname )
{
    static char         filterList[] = "Text Transcript" \
                                        "\0"\
                                        "*.txt" \
                                        "\0\0";
    OPENFILENAME        of;
    int                 rc;
	GetCurrentDirectory(500, OldPath);

	strcpy (fname, "*.txt");
    memset( &of, 0, sizeof( OPENFILENAME ) );
    of.lStructSize = sizeof( OPENFILENAME );
    of.hwndOwner = hwnd;
    of.lpstrFilter = (LPSTR) filterList;
    of.lpstrDefExt = "";
    of.nFilterIndex = 1L;
    of.lpstrFile = fname;
    of.nMaxFile = _MAX_PATH;
    of.lpstrTitle = NULL;
    of.Flags = OFN_HIDEREADONLY;
    if( save ) {
        rc = GetSaveFileName( &of );
    } else {
        rc = GetOpenFileName( &of );
    }

	SetCurrentDirectory( OldPath);
    return( rc );
} 

// ==========================================
//   Othello Board Functions
// ==========================================
void SetToComputerName( char *sName )
{
	char buffer[40];
	if (brainType == 0) strcpy(sName, "Pointy Stone (");
	else if (brainType == 1) strcpy(sName, "Greedy (");
	else if (brainType == 2) strcpy(sName, "Random Max (");
	else strcpy(sName, "Load Error (");
	if (TimeDependent == 0)
	{
		_itoa(SearchDepth, buffer, 10);
		strcat(sName,  buffer );
		strcat(sName, " ply)");
	}
	else strcat(sName, "Timed)");
}

void UpdateNames( )
{
	if (HumanPlayer[0] == NULL) strcpy (HumanPlayer, "Human Player");

	if (ComputerColor == BLACK || ComputerColor == NONE) strcpy( WhitePlayer, HumanPlayer);
	if (ComputerColor == WHITE || ComputerColor == NONE) strcpy( BlackPlayer, HumanPlayer);
	                                        
	if (ComputerColor == WHITE || ComputerColor == BOTH) SetToComputerName( WhitePlayer );
	if (ComputerColor == BLACK || ComputerColor == BOTH) SetToComputerName( BlackPlayer );

	SetDlgItemText( GameStats, 121, WhitePlayer );
	SetDlgItemText( GameStats, 122, BlackPlayer );        
}

// ------------------
void init_Board( char board[65], int start )
{
	int x = 0;
	EraseComments();
         
    for (x = 0; x < 65; x++) {
		board[x] = EMPTY;
	}
	board[3*8 + 3] = WHITE;
	board[3*8 + 4] = BLACK;
	if (start == BLACK) board[4*8 + 4] = BLACK; else board[4*8 + 4] = WHITE;
	board[4*8 + 3] = BLACK;
	if (start == BLACK)  board[5*8+4] = BLACK;
	if (start == BLACK)  board[64] = WHITE;
		 else  board[64] = BLACK;

	for (x = 0; x < 65; x++) {
		StartBoard[x] = GameBoard[x];
	}

	memcpy( StandardBoard, StartBoard, 65 );
               
	// Times
	BlackTime = 0;
	WhiteTime = 0;
	TimeLimitHit = 0;
	starttime = clock();

	Transcript[0] = 0;
	Transcript[2] = 0;

	if (ComputerColor == BOTH) 	{
		ComputerColor = WHITE;
		for (x = 40; x<=43; x++) CheckMenuItem (GetMenu(MainWnd), x, MF_UNCHECKED);         
		CheckMenuItem (GetMenu(MainWnd), 40, MF_CHECKED);
	}

	UpdateNames();
	bGameover = false;
	SetupBoard = 0;
	g_suggestSquare = -1;

	SetTitle(versionName);
}

// Count the number of white and black markers on the board
void NumMarkers( char board[65], int &numwhitem, int &numblackm )
{
    numwhitem = 0;
    numblackm = 0;
    
    for (int x = 0; x< 64; x++)
	{
        if (board[x] == WHITE) numwhitem++;
        if (board[x] == BLACK) numblackm++;
	}
}

void GetRotatedCoords( int &x, int &y )
{
	int temp;
    if (RotateBoard == 2) {
		x = 7-x;
        y = 7-y;
	}
    if (RotateBoard == 1) {
		temp = x;
        x = y;
        y = temp;
	}
    if (RotateBoard == 3) {
		temp = x;
        x = 7- y;
        y = 7- temp;
	}
}
        
// Flip markers 
// (x,y) start square
// (xd, yd) x delta and y delta
// if (test == 1) just return the number flipped, don't change the board
// returns 0 if none can be flipped
int FlipDirection( int x, int y, int xd, int yd, char board[65], char color, char test )
{
	int flip = 0;
	char ocolor;
	if (color == WHITE) ocolor = BLACK; else ocolor = WHITE;
	if (x +xd < 0 || y +yd < 0 || x +xd > 7 || y +yd > 7) return 0;
	if (board [(x + xd) + (y + yd)*8] != ocolor) return 0;
	x += xd;
	y += yd;
         
	while (x >= 0 && y >= 0 && x <= 7 && y <= 7)
	{
		flip++;

		if (board[x + (y<<3)] == EMPTY) return 0;
		if (board[x + (y<<3)] == color && flip > 1)
		{
			if (test == 1) return flip;
			for (int i = 0; i <flip; i++)
			board[(x + (i * xd * -1)) + (y + (i * yd * -1) )*8] = color;
			return flip;
		}
               
		x += xd;
		y += yd;                        
	}

	return 0;
}

// Can color move at (x,y). Returns number of flipped markers
int CanMove( int x, int y, char board[65], char color )
{
	if (board[x + y * 8] != EMPTY) return 0;

	int flippedDiscs = 0;
	for (int xd = -1; xd <= 1; xd++ )
		for (int yd = -1; yd <= 1; yd++ )
			if (!(yd == 0 && xd == 0) ) flippedDiscs += FlipDirection( x, y, xd, yd, board, color, 1 );

	return flippedDiscs;
}

// Returns the number of moves on board[65] for color
int FindMoves( char board[65], char color )
{
	int nummoves = 0;            
	for (int x = 0; x< 64; x++)
	{
		int flippedDiscs = 0;
		if (board[x] == EMPTY)
		{
			for (int xd = -1; xd< 2; xd ++)
				for (int yd = -1; yd< 2; yd ++)
					if (!(yd == 0 && xd == 0) ) flippedDiscs += FlipDirection(x%8 , x/8, xd, yd, board, color, 1);
		}

		if (flippedDiscs !=0) {
			nummoves++; 
			flippedDiscs = 0;
		}
	}
	return nummoves;
}

 // ----------------------
int DoMove( int x, int y, char board[], char *pTranscript)
{
	int flip = 0, xd, yd;
	char color = board[64];
	  
	if (x < 0 || y < 0 || x>7 || y>7 || board[x + y*8] !=EMPTY ) return 0;

	for (xd = -1; xd< 2; xd ++)
	for (yd = -1; yd< 2; yd ++)
		if (!(yd == 0 && xd == 0) ) flip += FlipDirection( x , y, xd, yd, board, color, 0 );
                
	if (flip == 0) return 0;
       
	board[x + (y<<3)] = board[64];

	if (pTranscript!=NULL) AddTranscript( x,  y, pTranscript );
		
	lastMove = 0;
	g_suggestSquare = -1;

	if (board[64] == WHITE) board[64] = BLACK;
		else board[64] = WHITE;

	if (FindMoves (board, board[64]) == 0)
		if (board[64] == WHITE) board[64] = BLACK;
		else board[64] = WHITE;

	return 1;
}

//
// If a move is possible in this square, do the move
//
void DoMoveXY( int x, int y, char Board[], char InTranscript[], bool bFlip)
{
	if (CanMove( x, y, Board, Board[64] ) != 0)
	{
		if (!bFlip) 
		{
			DoMove( x, y, Board, InTranscript );
			return;
		}

		for (int temp = 0; temp < 65; temp ++)
			OldBoard[temp] = Board[temp];
	                                                       
		if (DoMove(x, y, Board, InTranscript) == 1) 
		{
			flipi = 0;
			starttime = clock() - (CLOCKS_PER_SEC / 3);
		}
	}
}

// -------------------------------
// Highlight the suggested square
// -------------------------------
void HighlightSquare( int square, unsigned long color)
{
	HDC		hdc = GetDC(MainWnd);
    HBRUSH	brush = CreateSolidBrush ( color );
    
    if (square >= 0 && square <=63)
	{
		// Fixme : account for RotateBoard
		int x = square %8;
		int y = square /8;
		GetRotatedCoords( x, y );

		RECT rect;
		rect.left = x*32 + 64;
		rect.right = (x+1)*32 + 64;
		rect.top = y * 32 + 54;
		rect.bottom = (y+1)*32 + 54;

		FrameRect(hdc, &rect, brush);
		/*
		if (border == 2)
		{
			rect.left+=1; rect.right -=1; rect.top+=1; rect.bottom-=1;
			FrameRect(hdc, &rect, brush);
		}
		*/
	}

    DeleteObject( brush );
	ReleaseDC(MainWnd, hdc );  
}

// Draw the Board
void DrawBack( HDC hdc )
{
    DrawBitmap( hdc, board_bmp , 22, 13 );
}

// -------------------------------
// Draw Board  (no flipping animation in progress)
// -------------------------------
void DrawBoard( HDC hdc, char board[65] )
{
	bool bRelease = false;
	if (hdc == NULL) 
	{
		hdc = GetDC( MainWnd );
		bRelease = true;
	}
       
    for (int x = 0; x < 8; x++)
		for (int y = 0; y < 8; y++)
		{
			int  index = 0;
			if (RotateBoard == 0) index = x + y * 8;
			if (RotateBoard == 2) index = 7-x + (7-y) * 8;
			if (RotateBoard == 1) index = y + x * 8;
			if (RotateBoard == 3) index = (7-y) + (7-x) * 8;
                    
			if (board[ index ] == WHITE)
				DrawBitmap( hdc,  white_bmp, x*32+64, y*32+54);
			if (board[ index ] == BLACK)
				DrawBitmap( hdc, black_bmp , x*32+64, y*32+54);
			if (board[ index ] == EMPTY)
				if (ShowMoves == 0 ||  CanMove( index%8, index/8, board, board[64]) == 0)
					DrawBitmap( hdc, square_bmp , x*32+64, y*32+54);
				else
					DrawBitmap( hdc, msquare_bmp , x*32+64, y*32+54);                        
		}

	if ( g_suggestSquare >= 0 )
		HighlightSquare( g_suggestSquare, 0xFFFFFF );

	if (bRelease) 
		ReleaseDC( MainWnd, hdc ); 
}

// ---------------------
// Draw the Othello Board (with flipping animation in progress)
//
// board2 contains the board before the move was made, board the board after it was made
// ---------------------
void DrawFlip(HDC hdc, char board[65], char board2[65])
{
	int x, y, index = 0;
	bool bRelease = false;
	static int incFlip = 0;
	if (hdc == NULL) {
		hdc = GetDC( MainWnd );
		bRelease = true;
	}

	// Check to see if flipping is done
	if (flipi == 6 || NoFlip == 0 || flipi == -1) 
	{
		DrawBoard (hdc, board);
        flipi = -1;
		psChangeCursor( );
        return;
	}
        
	// Draw the board
    for (x = 0; x < 8; x++)
		for (y = 0; y < 8; y++)
		{
			if (RotateBoard == 0) index = x + y * 8;
			if (RotateBoard == 2) index = 7-x + (7-y) * 8;
			if (RotateBoard == 1) index = y + x * 8;
			if (RotateBoard == 3) index = (7-y) + (7-x) * 8;
                    
			if (board [ index ] == WHITE )
				if (board2[index] == WHITE || board2[index] == EMPTY) DrawBitmap( hdc, white_bmp, x*32+64, y*32+54);
				else DrawBitmap( hdc, flipBitmaps[5-flipi], x*32+64, y*32+54 );
			if (board [ index ] == BLACK)
				if (board2[index] == BLACK || board2[index] == EMPTY ) DrawBitmap( hdc, black_bmp , x*32+64, y*32+54);
				else DrawBitmap( hdc, flipBitmaps[flipi], x*32+64, y*32+54 );
			if (board [ index ] == EMPTY)
				DrawBitmap( hdc, square_bmp , x*32+64, y*32+54 );
		}

	if (NoFlip == 2)
	{
		incFlip++;
		if (incFlip > 1) {flipi++; incFlip = 0;}
	}
	else flipi++;

	if (bRelease) 
		ReleaseDC( MainWnd, hdc );
}
    
// ------------------
// replays the game up to the move given in Transcript[0]
void ReplayGame( char board[], char Transcript[] )
{
	stopThinking = 1;

    int numMoves = Transcript[0];
	memcpy( board, StartBoard, 65 );
    
    for (int i = 1; i <= numMoves; i++)
		DoMove( Transcript[i*2]-'a' , Transcript[i*2 + 1]-'1', board , 0);
}

// COPY GAME STATE -------------
void CopyGame( int num )
{        
    for (int x = 0; x < 65; x++)
		CopyBoard[ num ][x] = StartBoard[x];
                
	for (int x = 0; x < 122; x++)
		CopyTranscript[ num ][x] = Transcript[x];      
}

// PASTE GAME STATE ------------
void PasteGame( int num )
{
    if (CopyTranscript[ num ][0] == 100)
	{
		DisplayText( "No Game State to Paste." );
        return;
	}

	int x;
    for (x = 0; x < 65; x++)
	{
		UndoBoard[x] = StartBoard[x];
        StartBoard[x] = CopyBoard[ num ][x];
	}
                                            
    for (x = 0; x < 122; x++)
	{
		UndoTranscript[x] = Transcript[x];
		Transcript[x] = CopyTranscript[ num ][x];
	}

    ReplayGame( GameBoard, Transcript );

    DrawBoard( NULL, GameBoard);
}

// UNDO PASTE GAME STATE ------------
void UndoPasteGame()
{
	if (UndoTranscript[0] == 100)
	{
		DisplayText( "No Paste to Undo." );
        return;
    }

	for (int x = 0; x < 65; x++)
		StartBoard[x] = UndoBoard[x];
                                                
    for (int x = 0; x < 122; x++)
		Transcript[x] = UndoTranscript[x];

	ReplayGame( GameBoard, Transcript);
    DrawBoard( NULL, GameBoard);
}             

// ------------------
void BackMove( )
{
    if ( Transcript[0] < 1 ) return;
	if ( ComputerIsThinking( true ) ) return;

    Transcript[0]--;
    ReplayGame( GameBoard, Transcript);

    flipi = -1;
    DrawBoard ( NULL, GameBoard);

    if (Comment[ 0 ][0]!=NULL && Transcript[0] == 0)
		DisplayText ( Comment[ Transcript[0] ] );
}

// ------------------
void RedoMove( )
{
	if ( ComputerIsThinking( true ) ) return;

	for (int i = 0; i < 65; i++)
		OldBoard[i] = GameBoard[i];
	                            
	if (Transcript[ Transcript[0] * 2 + 2] == 0) return;
	    
	Transcript[0]++;
	ReplayGame( GameBoard, Transcript);

	flipi = 0;
}

// ------------------
void StartMove( )
{
	if ( ComputerIsThinking( true ) ) return;

	Transcript[0] = 0;
	ReplayGame( GameBoard, Transcript);

	DrawBoard( NULL, GameBoard);

	if (Comment[0][0] != NULL)
		DisplayText( Comment[ Transcript[0] ] );
}

// ------------------
void LastMove( )
{
	if ( ComputerIsThinking( true ) ) return;
	       
	Transcript[0] = 0;

	while (Transcript[ Transcript[0]*2 + 2 ] !=0)
		Transcript[0]++;
	        
	ReplayGame( GameBoard, Transcript);

	DrawBoard( NULL, GameBoard);
}
      
// ------------------
void GetDiscs( int &Black, int &White )
{
	int x, temp;
	char board[65];

	temp = Transcript[0];

	Transcript[0] = 60;
	ReplayGame( board, Transcript);

	for (x = 0; x<64; x++)
		if (board[ x ] == BLACK) Black++;
	else 
		if (board[ x ] == WHITE) White++;

	Transcript[0] = temp;
}

// ------------------
void ShowMovesToggle( )
{        
	if (ShowMoves == 0) 
	{
		ShowMoves = 1;
		CheckMenuItem( GetMenu(MainWnd), 71, MF_CHECKED);
	} else {
		ShowMoves = 0;
		CheckMenuItem( GetMenu(MainWnd), 71, MF_UNCHECKED);
	}

	DrawBoard( NULL, GameBoard );
}
        
void DrawBitmap( HDC hdc, HBITMAP bitmap, int x, int y )
{
    BITMAP      bitmapbuff;
    HDC         memorydc;
    POINT       origin;
    POINT       size;
    
    memorydc = CreateCompatibleDC( hdc );
    SelectObject( memorydc, bitmap );
    SetMapMode( memorydc, GetMapMode( hdc ) );
    GetObject( bitmap, sizeof( BITMAP ), (LPSTR) &bitmapbuff );

    origin.x = (short) x;
    origin.y = (short) y;
    size.x = bitmapbuff.bmWidth;
    size.y = bitmapbuff.bmHeight;

    DPtoLP( hdc, &origin, 1 );
    DPtoLP( memorydc, &size, 1 );

    BitBlt( hdc, origin.x, origin.y, size.x, size.y, memorydc, 0, 0, SRCCOPY);
    DeleteDC( memorydc );
}

/* =============================================
 * AboutDlgProc - processes messages for the about dialog.
   ============================================== */
INT_PTR CALLBACK  AboutDlgProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM /*lparam*/ )
{
    switch( msg ) {
    case WM_INITDIALOG:
        return( TRUE );

    case WM_COMMAND:
        if( LOWORD( wparam ) == IDOK ) {
            EndDialog( hwnd, TRUE );
            return( TRUE );
        }
        break;
    }
    return( FALSE );
}

// ==============================================================
//   Player Info
// ==============================================================
BOOL CALLBACK PlayerDlgProc( HWND hwnd, UINT msg,  WPARAM wparam, LPARAM lparam )
{
 lparam = lparam;                    /* turn off warning */

 switch( msg ) {
    case WM_INITDIALOG:
		SetDlgItemText ( hwnd, 100, BlackPlayer);
		SetDlgItemText ( hwnd, 101, WhitePlayer);
		return( TRUE );

    case WM_COMMAND:
 
		if( LOWORD( wparam) == 3)
		{
			GetDlgItemText ( hwnd, 100, DefaultName , 39 );
			strcpy (HumanPlayer, DefaultName);
			SaveDefaults();
		}
		if( LOWORD( wparam) == 4)
		{
			GetDlgItemText ( hwnd, 101, DefaultName , 39 );
			strcpy (HumanPlayer, DefaultName);
			SaveDefaults();
		}

        if( LOWORD( wparam ) == 1 )  
        {
			GetDlgItemText( hwnd, 100, BlackPlayer , 39 );
			GetDlgItemText( hwnd, 101, WhitePlayer , 39 );
			if (ComputerColor == BLACK) strcpy (HumanPlayer, WhitePlayer);
			if (ComputerColor == WHITE) strcpy (HumanPlayer, BlackPlayer);
			SetDlgItemText( GameStats, 121, WhitePlayer );
			SetDlgItemText( GameStats, 122, BlackPlayer );
        }

        if( LOWORD( wparam ) == 2 ||  LOWORD( wparam ) == 1) {
            EndDialog( hwnd, TRUE );
            return( TRUE );
        }
          
        break;
 }
return (FALSE);
}

// =============================================
// TIME Dialog
// =============================================
INT_PTR CALLBACK TimeDlgProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM /*lparam*/ )
{               
	static int pause2, pause3;

 switch( msg ) {
    case WM_INITDIALOG:
        SetDlgItemInt( hwnd, 100, TimePause, FALSE );
		pause2 = TimePause;
		pause3 = PauseOnlyPass;
		CheckDlgButton( hwnd, 102, pause3 );
        return( TRUE );

    case WM_COMMAND:
 
        if( LOWORD( wparam ) == IDOK )  
			{TimePause = pause2;
			 PauseOnlyPass = pause3;
			 SaveDefaults();
			}

        if( LOWORD( wparam ) == 100 )
			pause2 = GetDlgItemInt ( hwnd, 100, NULL, FALSE );

		if( LOWORD( wparam ) == 102 )
			{pause3 = ((pause3 + 1) & 1);
			 CheckDlgButton ( hwnd, 102, pause3 );
			}
       
        if( LOWORD( wparam ) == IDCANCEL ||  LOWORD( wparam ) == IDOK) {
            EndDialog( hwnd, TRUE );
            return( TRUE );
          }
          
        break;
 }
return (FALSE);
}

//
// If the computer is currently thinking, stop thinking  (and play the current move)
//
void ComputerStop()
{
	stopThinking = 1;
	filegame = 0;
	if (ComputerColor == BOTH) 
		ComputerColor = NONE;
}

void SetCompColor( int color )
{
	if ( ComputerColor == ANALYZE_MODE )
	{
		stopThinking = 1;
		g_suggestSquare = -1;
		DrawBoard( NULL, GameBoard );
	}
	if ( ComputerColor == ANALYZE_MODE || color == ANALYZE_MODE )
		InvalidateRect( GetDlgItem( GameStats, ID_ANALYZE), NULL, TRUE); 

    ComputerColor = color;
    UpdateNames();

	for (int x = 40; x<=44; x++) 
		CheckMenuItem (GetMenu(MainWnd), x, MF_UNCHECKED);

	if (color == BLACK)			CheckMenuItem (GetMenu(MainWnd), 41, MF_CHECKED);
	if (color == WHITE)			CheckMenuItem (GetMenu(MainWnd), 40, MF_CHECKED);
    if (color == NONE)			CheckMenuItem (GetMenu(MainWnd), 42, MF_CHECKED);
	if (color == BOTH )			CheckMenuItem (GetMenu(MainWnd), 43, MF_CHECKED);
	if (color == ANALYZE_MODE )	CheckMenuItem (GetMenu(MainWnd), 44, MF_CHECKED);
        
	if (color != NONE && color != ANALYZE_MODE)
		Transcript[ Transcript[0] * 2 + 2] = 0; // Start playing if it's the computers move
}

void CheckGameOver( char Board[], eGameResult &bGameResult )
{
	// Game is over if no one can move
	if ( FindMoves(GameBoard, GameBoard[64]) == 0 ) 
	{
		int nWhite = 0;
		int nBlack = 0;
		NumMarkers( GameBoard, nWhite, nBlack );
		if (nWhite > nBlack) bGameResult = eGameResult::WHITEWINS;
		if (nBlack > nWhite) bGameResult = eGameResult::BLACKWINS;
		if (nBlack == nWhite) bGameResult = eGameResult::DRAW;
	}
}

//
// UPDATING CLOCK DISPLAY
// 
void UpdateTimeDisplay( HWND hwnd, int WhiteTime, int BlackTime )
{
	static char buffer[250];
	static char text[180];

	if (TimeLimit !=0 && TimeLimitHit == 0) 
	{
		WhiteTime = TimeLimit*60 - WhiteTime;
		BlackTime = TimeLimit*60 - BlackTime;
    }      

    _itoa( (WhiteTime/60), buffer, 10);
    if (TimeLimit !=0 && TimeLimitHit == 0) strcpy (text, "Time- "); else   strcpy (text, "Time+ ");
    strcat (text, buffer);
    if ( (WhiteTime%60) <10) strcat (text, ":0"); else strcat (text, ":");
    _itoa ( (WhiteTime%60) , buffer, 10);
    strcat (text, buffer);
    SetDlgItemText( hwnd, 105, text);

    _itoa ((BlackTime/60) , buffer, 10);
    if (TimeLimit !=0 && TimeLimitHit == 0) strcpy (text, "Time- "); else   strcpy (text, "Time+ ");
    strcat (text, buffer);
    if ( (BlackTime%60) <10) strcat (text, ":0"); else strcat (text, ":");
    _itoa( (BlackTime%60), buffer, 10);
    strcat (text, buffer);
    SetDlgItemText( hwnd, 108, text);
}

//
// RUNNING THE CLOCKS
// 
void UpdateClocks( HWND hwnd )
{
    endtime = clock();
	if (Transcript[0] != Moves() || bGameover ) starttime = endtime; // don't run clock when not playing

    if ((endtime - starttime) / CLOCKS_PER_SEC > 0 && !bGameover && Transcript[0] == Moves() && SetupBoard == 0)
	{
        if (GameBoard[64] == WHITE ) WhiteTime += (endtime - starttime) / CLOCKS_PER_SEC;
        if (GameBoard[64] == BLACK ) BlackTime += (endtime - starttime) / CLOCKS_PER_SEC;
                    
		 // RUN OUT OF TIME?
         if (WhiteTime >= TimeLimit * 60 && TimeLimitHit == 0 && TimeLimit != 0)
         {
				TimeLimitHit = 1; 
				MessageBeep( MB_OK ); 
				DisplayText (" BLACK WINS on time ");
         }
                         
         if (BlackTime >= TimeLimit * 60 && TimeLimitHit == 0 && TimeLimit != 0)
         {
			TimeLimitHit = 1; 
			MessageBeep( MB_OK ); 
			DisplayText(" WHITE WINS on time ");
		 }
	}

   if (((endtime - starttime) / CLOCKS_PER_SEC > 0 && !bGameover && Transcript[0] == Moves() && SetupBoard == 0) )
	{
         if ((endtime - starttime) / CLOCKS_PER_SEC > 0) 
			 starttime += ((endtime - starttime) / CLOCKS_PER_SEC) * CLOCKS_PER_SEC ;

		 UpdateTimeDisplay( hwnd, WhiteTime, BlackTime);
	}
}

//
// Draw the background of a button
//
void DrawButtonBack( bool bPressed, RECT rc, HDC hDC)
{
	int Color;
	if (bPressed) {
		// yellow background when pressed
		DrawEdge(hDC, &rc, EDGE_SUNKEN, BF_RECT );
		Color = 0x99CCCC;
	}
	else {
		DrawEdge(hDC, &rc, EDGE_RAISED, BF_RECT );
		Color = GetSysColor( COLOR_BTNFACE );
	}
	SetBkColor( hDC,  Color );
	rc.top += 2; rc.left += 2; rc.bottom -= 2; rc.right -= 2; 
	HBRUSH Htest = CreateSolidBrush( Color  ); 
	FillRect( hDC, &rc, Htest );
	DeleteObject( Htest );
	SetTextColor( hDC, 0x000000);
}

/* =======================
 * GAME WINDOW Procedure - processes messages for the about dialog.
   ======================= */
INT_PTR CALLBACK GameDlgProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{  
	static char buffer[250];
    static char text[180];
    static int NumBlack = 0, NumWhite = 0, Total;
    int NewBlack, NewWhite, x;
    int openingName;
     
    switch( msg ) 
	{
    case WM_INITDIALOG:
        return( TRUE );

   case WM_PAINT:
	   /*
	   *HDC                 hdc;
		PAINTSTRUCT         ps;
		if ( GetUpdateRect( hwnd, NULL, FALSE ) !=0 )
		{
			hdc = BeginPaint( hwnd, &ps );
                
			DrawBitmap( hdc, skinbrush, 0, 0);
      
			EndPaint(hwnd, &ps);
		}
		*/
   break;

	case WM_DRAWITEM:
	{
		LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lparam;
		RECT rc = lpdis->rcItem;

		if ( wparam == ID_ANALYZE )
		{
			DrawButtonBack( ComputerColor == ANALYZE_MODE, rc, lpdis->hDC );
			ExtTextOut( lpdis->hDC, 5, 5, 0, &rc, "An", 2, NULL );
		}

	   break;
	}
              
   case WM_COMMAND:

	// Back/Forward Buttons
	if (SetupBoard == 0)   
	{
		if ( LOWORD( wparam ) == 112 )
		{
			if (Transcript[0] >= 0 && Transcript[0] < 64)
				GetDlgItemText( hwnd, 112, Comment[ Transcript[0] ], 999 );
		}

		if ( LOWORD( wparam ) == ID_FIRSTMOVE ) {
			StartMove();
			SetFocus(MainWnd);
		}
		if ( LOWORD( wparam ) == ID_LASTMOVE ) {
			LastMove(); 
			SetFocus(MainWnd);
		}
		if ( LOWORD( wparam ) == ID_PREVMOVE ) {
			BackMove(); 
			SetFocus(MainWnd);
		}
		if ( LOWORD( wparam ) == ID_NEXTMOVE ) {
			RedoMove(); 
			SetFocus(MainWnd);
		}

		GetDlgItemText( GameStats, ID_FIRSTMOVE, buffer, 5 );
		if (buffer[0] == 'B')
		{
			SetDlgItemText( GameStats, ID_FIRSTMOVE, "<<");
			SetDlgItemText( GameStats, ID_LASTMOVE, ">>");
		}

		if (LOWORD( wparam ) == ID_MOVENOW ) // Stop/Start thinking
		{
			if ( ComputerIsThinking(true) ) 
				ComputerStop();
			else
				SetCompColor( GameBoard[64] );
		}
		if (LOWORD( wparam ) == ID_ANALYZE ) // Analyze Mode
		{
			ToggleAnalyzeMode();
		}
	}
	else // End Board Setup
    {
		if ( LOWORD( wparam ) == ID_FIRSTMOVE ) 
		{
			SetupBoard = -1;
			GameBoard[64] = BLACK;
			for (x = 0; x < 65; x++)
			StartBoard[x] = GameBoard[x];
			Transcript[0] = 0;
			Transcript[2] = 0;
		}
		if ( LOWORD( wparam ) == ID_LASTMOVE ) 
		{
			SetupBoard = -1;
			GameBoard[64] = WHITE;
			for (x = 0; x < 65; x++)
			StartBoard[x] = GameBoard[x];
			Transcript[0] = 0;
			Transcript[2] = 0;
		}
     }                          
                      
	NumMarkers( GameBoard, NewWhite, NewBlack );

	bGameover = FindMoves(GameBoard, GameBoard[64]) == 0;

	UpdateClocks( hwnd );
 
	// Display Comments
    if ( (NumBlack + NumWhite) != (NewWhite + NewBlack) && SetupBoard == 0 && DontErase ==0 && (NewBlack + NewWhite !=4) )
    { 
		if (Comment[ Transcript[0] ][0]!=NULL)
            DisplayText( Comment[ Transcript[0] ] );
        else if (thinking == 0 && lastMove != 2 /*|| Transcript[0] != Moves()*/) 
			DisplayText("");
    }
	DontErase = 0;

	// Board Setup Messages
	if (SetupBoard == 1) 
	{
		SetupBoard = 2;
		strcpy (text, "Board SETUP in progress   \015\012 LEFT  : White \015\012 LEFT + Shift : Black \015\012 RIGHT: erase \015\012\015\012 End Setup (with buttons below): \015\012 BL : Black's turn\015\012 WH : White's turn");
		text[25] = 13;
		DisplayText( text );
	}

	if (SetupBoard == -1) 
	{
		SetupBoard = 0;
		strcpy (text, "Game On! \015\012");
		DisplayText( text );
		WhiteTime = 0; 
		BlackTime = 0;
		starttime = clock();
	}                     
                 
	// Move Number
	if ( (NumBlack + NumWhite) != (NewWhite + NewBlack) || Moves() != Total)
	{
		_itoa ( Transcript[0] , buffer, 10);
		strcpy (text, "Move ");
		strcat (text, buffer);
      
		Total = Moves( );
		strcat (text, " of ");
		_itoa ( (Total), buffer, 10);
		strcat (text,buffer);
              
		SetDlgItemText( hwnd, 120, text );

		bGameover = FindMoves(GameBoard, WHITE) == 0 && FindMoves(GameBoard, BLACK) == 0 && SetupBoard == 0;

		// Display Game Over Messages
		if (bGameover && Comment[ Transcript[0] ][0] == NULL)
		{
			if (NewBlack > NewWhite)
			strcpy (text, "BLACK WINS " );
			else if (NewBlack < NewWhite)
			strcpy (text, "WHITE WINS " );
			else
			strcpy (text, "DRAWN GAME " );
		  
			DisplayText( text );
		}

		if (NewBlack + NewWhite == 4 && Moves() == 0 && SetupBoard==0)
			DisplayText( "The Game Begins." );

		// Name Opening
		if (NewBlack + NewWhite > 5 && SetupBoard == 0 && Comment[ Transcript[0] ][0] == NULL && Transcript[0] == Moves() )
		{
			openingName = GetOpeningNumber( NewBlack + NewWhite - 4 );
			if (openingName != 500)
			{
				GetDlgItemText(hwnd, 112, text, 100);
				if (strstr (text, "Eval") == NULL) text[0] = NULL;
				if (strstr (text, "Opening") ==NULL)
				{
					strcat (text, " \015\012Opening: \015\012 ");
					strcat (text, Oname[openingName] );
					DisplayText( text );
				}
			}
		}               
		}

		// Update Disc count display
		if (NumWhite != NewWhite)
		{     
			NumWhite = NewWhite;
        
			_itoa( NewWhite, buffer, 10 );
			strcpy( text, "Discs : " );
			strcat( text, buffer);
         
			SetDlgItemText( hwnd, 106, text );
		}

		if (NumBlack != NewBlack)
		{
			NumBlack = NewBlack;
			_itoa (NewBlack , buffer, 10);
			strcpy (text, "Discs : ");
			strcat (text, buffer);
        
			SetDlgItemText ( hwnd, 107, text ); 
		}              
	}

	return( FALSE );
}

// -----
// Gray or Ungray Menu items depening on whether it's a Net Game or not
// -----
void BoardSetupToggle( )
{
	if (SetupBoard == 0) 
	{
		SetupBoard = 1;
		SetDlgItemText( GameStats, ID_FIRSTMOVE, "BL");
		SetDlgItemText( GameStats, ID_LASTMOVE, "WH");
	}
    else 
	{
		SetupBoard = -1;
        for (int x = 0; x < 65; x++)
			StartBoard[x] = GameBoard[x];
		Transcript[0] = 0;
        Transcript[2] = 0;
	}
}

// ------------------
//
// Choose Cursor
//
// ------------------
void psChangeCursor( )
{	
	// if (mousey <8  || mousex > 154 + 32*8 ||  mousey> 206 +32*8 ) return;
	/*	if (thinking == 0 && GetCursor ()==waitCursor ) SetCursor ( arrowCursor );
	if (thinking == 1 && GetCursor ()!=waitCursor ) SetCursor( waitCursor );*/

	if (flipi != -1 ) return;
		
	if ( ComputerIsThinking() ||
		( GameBoard[64] == ComputerColor || ComputerColor == BOTH )
		&& ( SetupBoard == 0 && Transcript[0] == Moves() && !bGameover )
		)
		{
			SetCursor( waitCursor );
		}
	else if (bGameover || SetupBoard !=0) {
		SetCursor( arrowCursor );                                           
	}
	else if (mousey <56 || mousex<64 || mousex>64 + 32*8 ||  mousey>56 +32*8 ) {
		SetCursor ( arrowCursor );
	} else  {
		if (SetupBoard == 2) SetCursor( arrowCursor );
		if (SetupBoard != 2) 
			if (GameBoard[64] == WHITE) 
				SetCursor( wcursor);
			else 
				SetCursor( bcursor);
	}
}

// -----------------------
// For Threaded Thinking
// -----------------------
void FAR ThreadFunc( void FAR* /*parm*/) 
{
	int waitTime;
	char color = 55;

	thinking = 1; 
	stopThinking = 0;

	memcpy( OldBoard, GameBoard, 65 );

	starttime = clock();

	// Get best move
	int bestmove = ComputerThink( GameBoard[64], GameBoard, thinkingType == ANALYZE );

	int thinkTime = clock()-starttime;
                 
	starttime = clock() - (CLOCKS_PER_SEC / 3);
      
	if ( thinkingType == ANALYZE )
	{
		g_suggestSquare = bestmove;
		DrawBoard( NULL, GameBoard );
		// don't stop until requested
		while ( stopThinking == 0 )
		{
			Sleep( 1 );
		}
	}
	else if ( thinkingType == SUGGEST ) 
	{
		char buffer2[200];
		sprintf( buffer2, "I suggest %c%c", (bestmove & 7)+'a', (bestmove >> 3)+'1' );
		DisplayText( buffer2 );
		g_suggestSquare = bestmove;
	}
	else if ( thinkingType == MOVE || thinkingType == PERFECT || thinkingType == WINLOSS )
	{
		// wait for a certain amount of time if requested
		waitTime = 100 * TimePause - (thinkTime * 1000 / CLOCKS_PER_SEC);
		if ( PauseOnlyPass == 0 && Pieces(GameBoard) < 64 && waitTime > 0) 
			Sleep( waitTime ); 

		// do the move
		color = GameBoard[64];
		DoMove( (bestmove & 7), (bestmove >> 3), GameBoard, Transcript );
		flipi = 0; 
	}
		             
	lastMove = 2;

	if (GameBoard[64] == color && PauseOnlyPass == 1 && Pieces (GameBoard) < 64  && waitTime > 0) 
		Sleep (100 * TimePause);

	if (filegame == 1) 
	{
		send_move( &Transcript[Transcript[0] * 2] );
		DrawBoard( NULL, GameBoard );
	}

	thinking = 0;
		  
	_endthread();
} 

//
// StartThinkingThread
//
void StartThinkingThread( eThinkType thinkType )
{
	if ( thinking != 1 )
	{
		thinkingType = thinkType;
		thinking = 1;
		_beginthread( ThreadFunc, NULL, NULL );
	}
}

//
// Think, then suggest a move
//
void SuggestMove( )
{
	if ( thinking == 0 && SetupBoard == 0 )
	{
		CopyGame(4);

		StartThinkingThread( SUGGEST );
	}
}

void ToggleAnalyzeMode()
{
	if ( ComputerColor != ANALYZE_MODE )
	{
		SetCompColor( ANALYZE_MODE );
	}
	else
	{
		SetCompColor( NONE );
	}
}

void BlinkLastMove( )
{
	if (Transcript[0] != 0)
	{
        DrawBoard( NULL, OldBoard );
        Sleep(500);
        flipi = 0;
    }
}

void ReadNextPosition()
{
	static int x = 0;
	AllocateLearningData( );
	x++;
	ReadPosition( GameBoard, "newSolvE.pnt", x );
	DontErase = 1;
	DrawBoard (NULL, GameBoard);
}

void PlayPV( char board[66] )
{
	oldDepth = SearchDepth;
	ComputerColor = NONE;

	for (; SearchDepth >= 1; SearchDepth--)
	{
		int bestmove = ComputerThink(board[64], board);
		starttime = clock() - (CLOCKS_PER_SEC / 3);
		DoMove((bestmove & 7), (bestmove >> 3), board, Transcript);
		flipi = 0;
	}
	SearchDepth = oldDepth;
}

void DoEndgameSearch( eThinkType searchType )
{
	if (thinking == 0 && SetupBoard == 0)
	{
		if (Transcript[0] != Moves()) {
			CopyGame(2);
		}

		SetCompColor(GameBoard[64]);
		StartThinkingThread(searchType);
	}
}

//
// File I/O functions. Load/Save Import/Export
//
void SaveGameAs()
{
	GetDlgItemText(GameStats, 121, WhitePlayer, 200);
	GetDlgItemText(GameStats, 122, BlackPlayer, 200);

	static char tempname[3000], fname[3000];
	if (GetFileName(MainWnd, 1, tempname) == TRUE)
	{
		strcpy(fname, tempname);
		FILE* output = fopen(fname, "wb");
		if (output)
		{
			SaveGame(output);
			fclose(output);

			SetTitle(versionName, fname); 	// Add filename to title of window
		}
	}
}

void LoadGame()
{
	static char tempname[3000], fname[3000];
	if (GetFileName(MainWnd, 0, tempname) == TRUE)
	{
		strcpy(fname, tempname);
		FILE* output = fopen(fname, "rb");
		if (output != NULL)
		{
			LoadGame(output);
			fclose(output);

			ReplayGame(GameBoard, Transcript);
			DrawBoard(NULL, GameBoard);
			SetTitle(tempname, fname);
		}
	}
}

void CopyTranscriptToClipboard()
{
	if (OpenClipboard(MainWnd))
	{
		EmptyClipboard();
		clipTranscript = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, 128);
		char* bufferPtr = (char*)GlobalLock(clipTranscript);
		memcpy(bufferPtr, &Transcript[2], 122);
		GlobalUnlock(clipTranscript);
		SetClipboardData(CF_TEXT, clipTranscript);
		CloseClipboard();
	}
}

void PasteTranscriptFromClipboard()
{
	trannum = 1;

	if (OpenClipboard(MainWnd))
	{
		if (IsClipboardFormatAvailable(CF_TEXT))
		{
			ReadTranscriptClipBoard((char*)GetClipboardData(CF_TEXT), 6);
			//init_Board(GameBoard, WHITE);
			ReplayGame(GameBoard, Transcript);
			DrawBoard(NULL, GameBoard);
		}
		else
		{
			DisplayText("No text data found on Clipboard");
		}

		CloseClipboard();
	}
}

void ExportTranscript()
{
	static char tempname[3000], fname[3000];
	// ---  Export Transcript (Get Filename)
	GetDlgItemText(GameStats, 121, WhitePlayer, 200);
	GetDlgItemText(GameStats, 122, BlackPlayer, 200);

	if (GetFileName2(MainWnd, 1, tempname) == TRUE)
	{
		strcpy(fname, tempname);
		WriteTranscript(fname);
	}
}

void ImportTranscript()
{
	static char tempname[3000], fname[3000];
	if (GetFileName2(MainWnd, 0, tempname) == TRUE)
	{
		strcpy(fname, tempname);

		ReadTranscript(fname);
		init_Board(GameBoard, WHITE);
		ReplayGame(GameBoard, Transcript);

		DrawBoard(NULL, GameBoard);
	}
}

// --------------------------------------------------------------
//  Process Menu Commands
// --------------------------------------------------------------
void MenuCommands( HWND hwnd, LPARAM lparam, WPARAM wparam )
{
	static int x = 0;
	static char buffer[180];
    static int numSet, SB;

	switch( LOWORD( wparam ) ) 
	{
		case 255:
			stopThinking = 1;
			filegame = 0;
			if (ComputerColor == BOTH || ComputerColor == ANALYZE_MODE) 
				SetCompColor( NONE );
			break;

		case 256:
			stopThinking = 1;
			SetCompColor( NONE );
			break;

	   case 679: // Temporization
		DialogBox(MyInstance  ,"TimeBox", hwnd,  TimeDlgProc);
		break;

	   case 78: // Play PV on board
		   PlayPV(GameBoard);
	   break;

	   case 89:
			DisplayBoardEval(GameBoard);
	   break;

	   case 2956:
		   StartPerft( GameBoard );
	   break;

	   case 2977:
		   DisplayTT( GameBoard );
	   break;

	   case 2958:
		// File System Game
		if (filegame == 1) {
			DisplayText( "Ended File System Game" );
			filegame = 0;
		}
		else
		{	
			filegame = 1;
			init_Board( GameBoard, WHITE );
			DrawBoard( NULL, GameBoard );
			DisplayText( "Starting a file game.\015\012Write to: outmoves.txt.\015\012Read from: inmoves.txt" );
		}
	   break;

       case 48:
			SuggestMove( );
       break;
       
       case 76: // Board Orientation
        RotateBoard++;
        if (RotateBoard > 3) RotateBoard = 0;
        DrawBoard( NULL, GameBoard);
       break;

	   case 431:
		   DoEndgameSearch(PERFECT);
		   break;

		case 432:
			DoEndgameSearch(WINLOSS);      
			break;

	   case 60: 
			TimeDependent = 1;
			SearchDepth = 18;	
			EndgameDepth = 22;
			for (x = 51; x <= 60; x++) {
				CheckMenuItem(GetMenu(hwnd), x, MF_UNCHECKED);
			}
	      
			CheckMenuItem( GetMenu(hwnd), LOWORD( wparam ), MF_CHECKED );
			UpdateNames();
	   break;

	   // Set difficulty level
	   case 59: {SearchDepth = 16; EndgameDepth = 22;}
	   case 58: if (LOWORD( wparam ) == 58) {SearchDepth = 12; EndgameDepth = 20;}
	   case 57: if (LOWORD( wparam ) == 57) {SearchDepth = 10; EndgameDepth = 18;}
	   case 56: if (LOWORD( wparam ) == 56) {SearchDepth = 8; EndgameDepth = 16;}
	   case 55: if (LOWORD( wparam ) == 55) {SearchDepth = 6; EndgameDepth = 12;}
	   case 54: if (LOWORD( wparam ) == 54) {SearchDepth = 4; EndgameDepth = 10;}
	   case 53: if (LOWORD( wparam ) == 53) {SearchDepth = 3; EndgameDepth = 8;}
	   case 52: if (LOWORD( wparam ) == 52) {SearchDepth = 2; EndgameDepth = 6;}
	   case 51: if (LOWORD( wparam ) == 51) {SearchDepth = 1; EndgameDepth = 4;}
       for (x = 51; x <= 60; x++)
                CheckMenuItem (GetMenu(hwnd), x, MF_UNCHECKED);
      
       CheckMenuItem (GetMenu(hwnd), LOWORD( wparam ), MF_CHECKED);
	   TimeDependent = 0;

		if (ComputerColor == WHITE || ComputerColor == BLACK)
			UpdateNames();
       
       break;

	   // Reset Timer
	   case 69:
		   WhiteTime = 0;
		   BlackTime = 0;
		   TimeLimitHit = 0;
	   break;

		// TIME CHECK MARKS       
		case 61:  if (LOWORD( wparam ) == 61) TimeLimit = 1;
		case 62:  if (LOWORD( wparam ) == 62) TimeLimit = 2;
		case 63:  if (LOWORD( wparam ) == 63) TimeLimit = 5;
		case 64:  if (LOWORD( wparam ) == 64) TimeLimit = 15;
		case 65:  if (LOWORD( wparam ) == 65) TimeLimit = 30;
		case 66:  if (LOWORD( wparam ) == 66) TimeLimit = 0;
		case 67:  if (LOWORD( wparam ) == 67) TimeLimit = 3;
		case 68:  if (LOWORD( wparam ) == 68) TimeLimit = 10;

		for (x = 61; x <= 68; x++)
			CheckMenuItem (GetMenu(hwnd), x, MF_UNCHECKED);
        
		CheckMenuItem (GetMenu(hwnd), LOWORD( wparam ), MF_CHECKED);
      
		break;

		// Exit
		case 24:
			PostMessage( hwnd, WM_CLOSE, 1,1 );       
			break;

		// Restart
		case 21:

			if ( ComputerColor == ANALYZE_MODE )
			SetCompColor( NONE );

			init_Board( GameBoard, WHITE);
			Transcript[2] = 0;
			DrawBoard( NULL, GameBoard);
		break;

		case 23:
			SaveGameAs();
			break;

		case 22:
			LoadGame();
			break;
        
       case 25:
			ExportTranscript();
			break;
        
        case 26:
			ImportTranscript();
			break;

	    // Clipboard Paste Game Position
        case 2955:
			trannum = 1;
      
			OpenClipboard( hwnd);

			if (IsClipboardFormatAvailable (CF_TEXT) )
				ReadTextPosition ((char*) GetClipboardData (CF_TEXT) );
			else 
				DisplayText( "No text data found on Clipboard" );

			CloseClipboard();

			DrawBoard(NULL, GameBoard);
			break;


		case 523:
			CopyTranscriptToClipboard();
			break;

        // CLIPBOARD PASTE TRANSCRIPT
		case 927:
        case 27:
			PasteTranscriptFromClipboard();
			break;

		case 928:
        case 28:

        trannum +=1;
		OpenClipboard(hwnd);

		if (IsClipboardFormatAvailable(CF_TEXT) )	
		{
	   		ReadTranscriptClipBoard((char*) GetClipboardData (CF_TEXT), 10);
		}
		else 
		{
			DisplayText("No text data found on Clipboard");
		}

		CloseClipboard();

        ReplayGame( GameBoard, Transcript);

        DrawBoard(NULL, GameBoard);

		break;
	   
		case 30: // Get an even start board
			SB++;

			GetStartBoard( SB, Transcript );
			ReplayGame( GameBoard, Transcript);

			DrawBoard (NULL, GameBoard);

			break;

		// ------ generate training positions -------
		case 223:  // Self Play a tournament
			// ComputerTourney( 1, 2, 1, "tourney.txt", 8);
			// ComputerTourney( 240, 480, 1, "tourney.txt", 8);
			g_bSaveTraining = true;
			ComputerTourney( 1, 820, 1, "tourney.txt", 6);
			g_bSaveTraining = false;
			break;

		// ------ score positions -------
	    case 345:

		/*	CCoeffs = &Coeffs[ 0 ];
			LoadAllValues ("gradienB.val");
			SetCoeffFromError ( 0 );*/

		 /*	x = rand () %12480 + 1;*/
			DisplayText("Solving Positions...");	
			ReadSolvePosition ( GameBoard , "newposE.pnt", "newSolvE.pnt");
			/*ReadSolvePosition ( GameBoard , "newposME.pnt", "newSolvME.pnt");
			ReadSolvePosition ( GameBoard , "newposMB.pnt", "newSolvMB.pnt");
			ReadSolvePosition ( GameBoard , "newposB.pnt", "newSolvB.pnt");*/

			ComputerColor = NONE;
			DontErase = 1;
			Transcript[0] = 0;
			memcpy (StartBoard, GameBoard, 65);
			DrawBoard (NULL, GameBoard);
			 
			break;

		// ------ gradient descent -------
		case 344:

			DisplayText("Gradient Descent Learning in Progress...");
			AllocateLearningData( );

			//LoadAllValues ("gradienE.val");
			SetErrorValuesZero();
			for (x = 0; x < 1; x++)
				CheckPosition( GameBoard , "NewSolvE.pnt", 0, x);

			SaveAllValues ("gradienE.val");

			ComputerColor = NONE;
			DontErase = 1;
			DrawBoard( NULL, GameBoard );

			break;

		// ------ set coeffecients -------
		case 346:
			AllocateLearningData ( );
			/*LoadAllValues ("gradienB.val");
			SetCoeffFromError ( 0 );
			LoadAllValues ("gradienMB.val");
			SetCoeffFromError ( 1 );
			LoadAllValues ("gradienME.val");
			SetCoeffFromError ( 2 );*/
			LoadAllValues ("gradienE.val");
			SetCoeffFromError ( 3 );

			SaveAllCharCoeff ("newcoeff.dat");
			//LoadAllCharCoeff ("newcoeff.dat");
			BlendCoeff( 4, 0 , 1 );
			BlendCoeff( 5, 1 , 2 );
			BlendCoeff( 6, 2 , 3 );

			break;
	
        // Computer Plays?
        case 41: 
			SetCompColor(BLACK);
        break;
        
        case 40:
			SetCompColor(WHITE);
        break;

        case 42:
			SetCompColor( NONE );
        break;

		case 44:
			SetCompColor( ANALYZE_MODE );
		break;

        case 43:
			SetCompColor( BOTH );
        break;
		
		// ---------------
		case 1200:
		case 1201:
		case 1202:
		case 1203:
			CheckMenuItem (GetMenu(MainWnd), 1200 + OpeningsType, MF_UNCHECKED);
			OpeningsType = LOWORD (wparam) - 1200;
			CheckMenuItem (GetMenu(MainWnd), 1200 + OpeningsType, MF_CHECKED);
		break;

		case 1210:
		case 1211:
		case 1212:
		case 1213:
		case 1214:
		case 1215:
		case 1216:
		case 1217:
		case 1218:

			CheckMenuItem (GetMenu(MainWnd), 1210 + ExtraHashBits, MF_UNCHECKED);
			ExtraHashBits = LOWORD (wparam) - 1210;
			CheckMenuItem (GetMenu(MainWnd), 1210 + ExtraHashBits, MF_CHECKED);
			TT_Allocate ( ExtraHashBits, MainWnd );

			break;

		case 1219:
			TT_Clear();
			break;

		case 1220:
		case 1221:
		case 1222:
			CheckMenuItem(GetMenu(MainWnd), 1220 + brainType, MF_UNCHECKED);
			brainType = (eBrainType)(LOWORD(wparam) - 1220);
			CheckMenuItem(GetMenu(MainWnd), 1220 + brainType, MF_CHECKED);
			TT_Clear();
			UpdateNames();
			break;

		// --------------------------------------------------------------------------
        // Setup Board
        case 46:
			BoardSetupToggle();		
			break;

        case 71:
			ShowMovesToggle();                           
			break;
              
		// no cursor
		case 6721:
			if (NoCursor == 0) {
				NoCursor = 1;
				CheckMenuItem (GetMenu(MainWnd), 6721, MF_CHECKED);
			} else {
				NoCursor = 0;
				CheckMenuItem (GetMenu(MainWnd), 6721, MF_UNCHECKED);
			}
			break;

       // Toggle Flip Animation
        case 675:
			for (x = 675; x <= 677; x++)	CheckMenuItem (GetMenu(MainWnd), x, MF_UNCHECKED);

			NoFlip = 0;
			CheckMenuItem (GetMenu(MainWnd), 675, MF_CHECKED);
			break;

		case 676:
			for (x = 675; x <= 677; x++) {
				CheckMenuItem(GetMenu(MainWnd), x, MF_UNCHECKED);
			}

			NoFlip = 1;
			CheckMenuItem (GetMenu(MainWnd), 676, MF_CHECKED);
			break;

        case 677:
		for (x = 675; x <= 677; x++)	CheckMenuItem (GetMenu(MainWnd), x, MF_UNCHECKED);

        NoFlip = 2;
        CheckMenuItem (GetMenu(MainWnd), 677, MF_CHECKED);
        break;

        // Display Search Info
        case 73:
			if (SearchInfo == 0) {
				SearchInfo = 1;
				CheckMenuItem (GetMenu(MainWnd), 73, MF_CHECKED);
			} else {
				SearchInfo = 0;
				CheckMenuItem (GetMenu(MainWnd), 73, MF_UNCHECKED);
			}

        break;
        
        case 77:
			BlinkLastMove( );
        break;

		case 94:
		{
			char tempName[1024];
			strcpy(tempName, WhitePlayer);
			strcpy(WhitePlayer, BlackPlayer);
			strcpy(BlackPlayer, tempName);
			SetDlgItemText(GameStats, 121, WhitePlayer);
			SetDlgItemText(GameStats, 122, BlackPlayer);
		}
		break;

		case 95:
			strcpy( HumanPlayer, DefaultName );
			UpdateNames();
		break;

		case ID_NAMES_PLAYERINFO:
			DialogBox((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), "PLAYER", hwnd, (DLGPROC) PlayerDlgProc);
		break;

        case 101:
			CopyGame( 0 );
			break;

        case 102:
			PasteGame( 0 );
			break;

        case 103:
			UndoPasteGame( );
			break;

        case 104:
			PasteGame( 1 );
			break;

		case 109: 
			PasteGame( 2 );
			break;

		case 112:
        if (experiment == 0) {
			experiment = 1;
            CheckMenuItem( GetMenu(MainWnd), 112, MF_CHECKED);
			TT_Clear();
        } else {
			experiment = 0;
            CheckMenuItem( GetMenu(MainWnd), 112, MF_UNCHECKED);
			TT_Clear();
        }
		break;
                    
		case 111:
        if (hashing == 0) {
			hashing = 1;
            CheckMenuItem( GetMenu(MainWnd), 111, MF_CHECKED);
        } else {
			hashing = 0;
            CheckMenuItem( GetMenu(MainWnd), 111, MF_UNCHECKED);
		}
                    
		break;

        case MENU_ABOUT:
			DialogBox( MyInstance ,"AboutBox", hwnd, AboutDlgProc);
        break;
        }
}


void ToBoardCoords( int &x, int &y)
{
    x = (x - 64)/32;
    y = (y - 56)/32;
	GetRotatedCoords( x, y );
}

//
// For Board Setup, Place a marker on the board
//
void DropMarker( int x, int y, int rButton ) 
{
	ToBoardCoords( x, y );
	if (x >= 0 && y>=0 && y<=7 && x<=7)
	{
		if (rButton == 1) 
			GameBoard[x + y*8] = EMPTY;
		else if (GetKeyState(VK_SHIFT) < 0 )
			GameBoard[x + y*8] = BLACK;
		else 
			GameBoard[x + y*8] = WHITE;
		DrawBoard (NULL, GameBoard);
	}
}

//
// For making a move
//
void PlayerMove( int x, int y )
{
	ToBoardCoords( x, y );

	// If this is a valid move and we're analyzing, stop the thinking thread
	if ( CanMove( x, y, GameBoard, GameBoard[64] ) )
		if ( thinkingType == ANALYZE || thinkingType == SUGGEST )
		{
			stopThinking = 1;
			while ( thinking != 0 )
			{
				Sleep(1);
			}	
		}

	if (thinking == 0 )
	{
		int temp = (Transcript[0] != Moves() ) ? 1 : 0;
	        
		if ( CanMove( x, y, GameBoard, GameBoard[64] ) )
		{
			if (temp == 1) 
				CopyGame( 2 );

			memcpy( OldBoard, GameBoard, 65 );
		}
	                                                        
		if ( DoMove( x, y, GameBoard, Transcript ) == 1 )
		{
			flipi = 0; // Update Board via flip
			starttime = clock() - (CLOCKS_PER_SEC / 3);
		}
	}
}

//
// Play a file system game with another program
// 
void TickFilegame()
{
	if (FindMoves(GameBoard, GameBoard[64]) == 0) // GameOver
	{
		AddGameLog(ComputerColor);
		init_Board(GameBoard, WHITE);
		ComputerColor = (ComputerColor == WHITE) ? BLACK : WHITE;
		UpdateNames();
	}
	else if (thinking == 0)
	{
		if (wait_move() == 1 || (GameBoard[64] == ComputerColor && Transcript[0] == 0))
		{
			DrawBoard(NULL, GameBoard);

			if (FindMoves(GameBoard, GameBoard[64]) != 0) // If game is not over
			{
				if (ComputerColor != GameBoard[64]) {
					send_move("pass");
				} else {
					StartThinkingThread(MOVE);
				}
			}
		}
	}
}

// Keyboard Input
void ProcessKeydown( int key )
{
	if (key == '1') AddtoBook('0');
	if (key == '2') AddtoBook('1');
	if (key == '3') AddtoBook('2');
	if (key == '4') AddtoBook('3');
	if (key == '5') AddtoBook('4');
	if (key == '6') RemovePosition();
	if (key == '9') AddGameLog(ComputerColor);

	// Computer plays player to move
	if (key == 'Z')
	{
		if (Transcript[0] != Moves())
			CopyGame(2);

		SetCompColor(GameBoard[64]); // Computer now plays the side to move
	}

	if (GetKeyState(VK_CONTROL) < 0)
	{
		if (key == 'C') CopyTranscriptToClipboard();
		if (key == 'V') PasteTranscriptFromClipboard();
	}
	else
	{
		if (key == 'B') BoardSetupToggle();
		if (key == 'S') ComputerStop();
		if (key == 'M') SuggestMove();
		if (key == 'R') BlinkLastMove();
		if (key == 'V') PasteGame(2);
		if (key == 'E') DisplayBoardEval(GameBoard);
		if (key == 'T') DisplayTT(GameBoard);
		if (key == 'N') ReadNextPosition();
	}


}

/* ===============================================================
 * WindowProc - handle messages for the main application window
 ================================================================ */ 
LRESULT CALLBACK WindowProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	static char  buffer[180], buffer2[80];
	HDC                 hdc;
	PAINTSTRUCT         ps;
	HBRUSH              brush;
     
	int x, y;
	static int lButton = 0, rButton = 0;  

	switch( msg ) 
	{
	case WM_KEYDOWN:

		ProcessKeydown(int(wparam));
		return 1;
     
	case WM_LBUTTONDOWN:
		lButton = 1;
		SetFocus(hwnd);

		x = (int)(short)LOWORD(lparam );
		y = (int)(short)HIWORD(lparam );
		if (SetupBoard == 2) 
		{ 
			DropMarker( x, y, 0 ); 
			break;  
		}

		PlayerMove( (int)(short)LOWORD(lparam ), (int)(short)HIWORD(lparam ) );

		break;

	case WM_LBUTTONUP:
		lButton = 0;
		break;

	case WM_RBUTTONUP:
		rButton = 0;
		break;

    case WM_RBUTTONDOWN:
		rButton = 1;
		SetFocus(hwnd);

		x = (int)(short)LOWORD(lparam );
		y = (int)(short)HIWORD(lparam );

		if (SetupBoard == 2) 
		{
			DropMarker( x, y, 1);  
			break;  
		}
            
		break;

	case WM_MBUTTONDOWN:

		 x = (int)(short)LOWORD(lparam );
		 y = (int)(short)HIWORD(lparam );
		 ToBoardCoords (x, y);       

		if (SetupBoard == 0) 
			ShowMovesToggle();           

		break;

	case WM_MOUSEMOVE:
    
		mousex = (int)(short)LOWORD(lparam );
		mousey = (int)(short)HIWORD(lparam );
		psChangeCursor( );

		if (SetupBoard == 2 && (lButton || rButton)) 
		{ 
			DropMarker( mousex, mousey, rButton); 
			break;  
		}
	return 1;  

	case WM_TIMER:

		if ( stringToDisplay )
		{
			stringToDisplay = false;
			DisplayText( displayString );
		}

	 // Playing against a file on the computer
		if (filegame == 1)
		{
			TickFilegame();
		}
		// otherwise
		else if (flipi != -1) 
		{
            DrawFlip( NULL, GameBoard, OldBoard);
        } 
		else // Computer decides on a move
		{
           if ( SetupBoard == 0 && (Transcript[0] == Moves() || ComputerColor == ANALYZE_MODE) && !bGameover)
			 if ( (GameBoard[64] == ComputerColor || ComputerColor == BOTH || ComputerColor == ANALYZE_MODE ) )
			   if ( thinking == 0 )
				{ 
					if ( ComputerColor == ANALYZE_MODE )
					{
						 StartThinkingThread( ANALYZE );
					}
					else
					{
						SetCursor( waitCursor );
						StartThinkingThread( MOVE );
					}
				}
		}

        if (GameStats != NULL)
           PostMessage( GameStats, WM_COMMAND, 29,29 );
        
      	return 1;
		
	case WM_SETCURSOR:
		psChangeCursor( );                    
    break;
            
    case WM_COMMAND:
		MenuCommands (hwnd, lparam, wparam); 
    break;
 
	case WM_ERASEBKGND:
		return TRUE;
	break;

    case WM_PAINT:
    
        hdc = BeginPaint (hwnd, &ps);

        brush = CreateSolidBrush( 0x444444 );
        FillRect( hdc, &ps.rcPaint, brush );
        DeleteObject( brush );

        DrawBack (hdc);
        DrawBoard (hdc, GameBoard);
  
        EndPaint(hwnd, &ps);
          
    break;       

    case WM_DESTROY:

        SaveDefaults();
        PostQuitMessage( 0 );
        break;

    default:
        return( DefWindowProc( hwnd, msg, wparam, lparam ) );

    } // end switch message
   
  return( 0L );

}

// ------------------------------------------
// Playing games using File I/O
// ------------------------------------------
void send_move(char *move)
{
	FILE *f = nullptr;
	int overwrite = 0;
	
	// File should be read and delete by oponnent right away, but we need to check to make sure
	while( overwrite < 50 && (f = fopen("outmoves.txt", "r")) != NULL )
	{
		fclose(f);
		Sleep(100);
		overwrite++;
	}

	while ( true )
	{
		f = fopen("outmoves.txt", "w");
		if( f != NULL )
		{
			if (move[0] == 'p') fwrite( move, 4, 1, f);
			else fwrite( move, 2, 1, f);
			fclose(f);
			break;
		}	
	}
}
//------------------
// keep checking for "inmoves.txt" where the opponent will write its move
//------------------
int wait_move( )
{
	char move[80];
	FILE *fp = fopen("inmoves.txt", "r");
	Sleep(25);

	if(fp != NULL)
	{
		fgets(move, 80, fp);
		fclose(fp);
	
		remove("inmoves.txt");

		if (move[0]!='p')
		{
			DoMove (move[0] - 'a', move[1] - '1', GameBoard, Transcript );
		}
		return 1;
	}

	return 0;
}

// -----------
// Computer Tourney
// -----------
void ComputerTourney( int startgame, int endgame, int step, char *filename, int depth )
{
	// SetLevel( Depth, Depth+2, 0);
	SearchDepth = depth;
	EndgameDepth = depth;
	TimeDependent = 0;
	int P1Wins = 0, P2Wins = 0, BlackM, WhiteM, TotalP1 = 0, TotalP2 = 0, OldBlack, OldWhite, Distinct = 0;
	int OldDepth = SearchDepth, oldHashing = hashing, j = 0, P1Nodes = 0, P2Nodes = 0, Temp1, Temp2;
	char buffer2[2400];

	FILE *FP = fopen (filename, "wt");

	if (startgame < 1) startgame = 1;
	if (endgame > 833) endgame = 833;
	if (startgame > 833) return;

	for (int SB = startgame; SB <= endgame; SB+=step )
	{
		fclose (FP);
		FP = fopen (filename, "at");

		fprintf (FP, "Game # %d \n", SB - startgame + 1);

		Temp1 = 0;
		Temp2 = 0;

		if ( !GetStartBoard(SB, Transcript) ) 
			return;

		ReplayGame( GameBoard, Transcript );

		while ( FindMoves(GameBoard, GameBoard[64]) != 0 )
		{
			/*if (GameBoard [64] == BLACK) {hashing = 0; SearchDepth = 6; brainType = 0;}
			if (GameBoard [64] == WHITE) {hashing = 0; SearchDepth = 6; brainType = 1;}*/

			int bestmove = ComputerThink (GameBoard[64], GameBoard);
			DoMove( (bestmove & 7), (bestmove >> 3), GameBoard, Transcript );

			/* if (brainType == 0) P1Nodes += nodes;
							else P2Nodes += nodes;*/
		}
		fwrite (&Transcript[2], 120, 1, FP);
		fprintf (FP, "\n");

		NumMarkers ( GameBoard, WhiteM, BlackM);
		TotalP1 += BlackM;
		TotalP2 += WhiteM;

		OldBlack = BlackM; 
		OldWhite = WhiteM;

		fprintf (FP, "P1Black: %d P2White: %d \n", BlackM, WhiteM);
		
		if (BlackM > WhiteM)
			{P1Wins++; Temp1++;}
		else if (BlackM < WhiteM)
			{P2Wins++; Temp2++;}

		GetStartBoard (SB, Transcript);
		ReplayGame(GameBoard, Transcript);

		while ( FindMoves(GameBoard, GameBoard[64]) != 0 )
		{
			/*if (GameBoard [64] == WHITE) {hashing = 0; SearchDepth = 6; brainType = 0;}
			if (GameBoard [64] == BLACK) {hashing = 0; SearchDepth = 6; brainType = 0;}*/
			 ComputerThink( GameBoard[64], GameBoard );
			 DoMove( Transcript[Transcript[0] * 2] - 'a', Transcript[Transcript[0] * 2 + 1] -'1', GameBoard, 0 );

			/* if (GameBoard [64] == BLACK) P1Nodes += nodes;
							else P2Nodes += nodes;*/
		}

		fwrite( &Transcript[2], 120, 1, FP );
		fprintf( FP, "\n" );

		NumMarkers( GameBoard, WhiteM, BlackM);
		TotalP1 += WhiteM;
		TotalP2 += BlackM;

		if (BlackM > WhiteM)
		{	
			P2Wins++;
			Temp2++;
		}
		else if (BlackM < WhiteM)
		{
			P1Wins++;
			Temp1++;
		}

		if ( OldBlack != BlackM || OldWhite != WhiteM ) 
		{
			Distinct++;
			fprintf (FP, "P1White: %d P2Black: %d  (Distinct %d-%d)\n", WhiteM, BlackM, Temp1, Temp2);
		}
		else 
			fprintf (FP, "P1White: %d P2Black: %d  \n", WhiteM, BlackM);

		fprintf (FP, "\n");

		_ltoa( SB, buffer2, 10);
		SetDlgItemText( GameStats, 120, buffer2 );
		}

	fclose (FP);

	FP = fopen (filename, "at");
	fprintf (FP, "-------- \n");
	fclose (FP);

	FP = fopen (filename, "at");
	fprintf (FP, "\nP1 Nodes: %d P2 Nodes: %d", P1Nodes, P2Nodes);
	fprintf (FP, "\n\nDistinct Pairs: %d ", Distinct);
	fprintf (FP, "\nTotal Discs P1: %d Total Discs P2: %d \n", TotalP1, TotalP2);
	fprintf (FP, "P1Wins: %d P2Wins: %d Ties: %d \n", P1Wins, P2Wins, (endgame - startgame + step) * 2 / step - P1Wins - P2Wins);
	fclose (FP);

	memset (Transcript, 0, 120);
	init_Board (GameBoard, WHITE);

	j = sprintf (buffer2, "Total Pairs: %d ", (endgame - startgame + step) / step);
	j+= sprintf (buffer2 +j, "Distinct Pairs: %d ", Distinct);
	j+= sprintf (buffer2 +j, "\15\12Discs P1: %d Discs P2: %d ", TotalP1, TotalP2);
	j+= sprintf (buffer2 +j, "\15\12P1Wins: %d P2Wins: %d Ties: %d ", P1Wins, P2Wins, (endgame - startgame + step) * 2 / step - P1Wins - P2Wins);

	DisplayText( buffer2);
	//spawnl (P_NOWAIT, "tourney.txt", "notepad",  NULL  , NULL );
	SearchDepth = OldDepth;
	hashing = oldHashing;
	brainType = BT_POINTY;
}

