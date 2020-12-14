//============================================================================
// Pointy Stone Othello 3, Version 1.5
//============================================================================
// Copyright Jonathan Kreuzer, 2002 - 2020

#include <cstdint>
#include <process.h>
#include <dos.h>
#include <stdlib.h>
#include <math.h>
#include "ai.h"

int8_t MoveBoard[65];
char* OBook = (char*)calloc(48, BOOK_MAX); // Opening Book
int32_t SortBook[ BOOK_MAX ]; // indices for sorting the book when positions are added

Move* Slice_Moves = (Move*)calloc(sizeof(Move), 81*125);
Move2* Slice_Moves2 = (Move2*)calloc(sizeof(Move2), 81*125);
SqMove* Square_Moves = (SqMove*)calloc(sizeof(SqMove), 81*125);
int32_t* quiesce = (int32_t*)calloc(sizeof(int32_t), 81*125);

uint32_t* Boardindex =  (uint32_t*)calloc(sizeof(uint32_t) * 65 * 16 + 1,1);
uint32_t* Boardindex3 = (uint32_t*)calloc(sizeof(uint32_t) * 65 * 8 + 1,1);
BoardLookup2 Boardindex2; 

SingleMove *MoveStack = (SingleMove*) calloc( sizeof(SingleMove), STACK_SIZE * 32);
Board *BoardStack   = (Board*) calloc( sizeof(Board), STACK_SIZE);

uint32_t CurrentAhead;
uint64_t nodes;
uint64_t nodesDisplay = 0;

int BookLen;
int hashing = 1;
int currentBest = INVALID_MOVE;

int SearchEval, final, winloss, EndGamePrune;
clock_t g_start,end;
float MaxThinkSec, ExtraSecs;

int8_t* FirstOne = (int8_t*) calloc(1, 125*81);

int32_t* First5   = (int32_t*) calloc(4, 6562);
int32_t* Last5    = (int32_t*) calloc(4, 6562);
int32_t* Middle6  = (int32_t*) calloc(4, 6562);
int32_t* Middle4  = (int32_t*) calloc(4, 6562);
int32_t* Bflipped = (int32_t*) calloc(4, 11000);
int32_t* Wflipped = (int32_t*) calloc(4, 11000);

int8_t* Mobility = (int8_t*) calloc(1, 6562);
int8_t* RandomV	= (int8_t*) calloc(1, 6562);
int8_t* EdgeV    = (int8_t*) calloc(1, 6562);
int8_t* DiagV    = (int8_t*) calloc(1, 6562);

EdgeInfo* EdgeI = (EdgeInfo *) calloc( 6562, sizeof(EdgeInfo) );
DiagInfo* DiagI = (DiagInfo *) calloc( 6562, sizeof(DiagInfo) );

EvaluateP Coeffs[NUM_STAGES];
EvaluateP* CCoeffs;

uint8_t EmptySquares[66];
int HistoryWhite[66];
int HistoryBlack[66];
int Killer[66];

int startMarkers, brainType = 0, checkMove = 0, checkNum = 0, EndPruneRange = 70;
int refuteMove[66];
// ----------------
// AI files
#include "book.cpp"
#include "ttable.cpp"
#include "movegen.cpp"
#include "eval.cpp"

//---------
// Find slice for lookup code
//---------
inline void SliceZ(int i, char slice[])
{ 
    slice[1] = (char)( i%3);
    slice[2] = (char)( (i/3)%3);
    slice[3] = (char)( (i/9)%3);
    slice[4] = (char)( (i/27)%3);
    slice[5] = (char)( (i/81)%3);
    slice[6] = (char)( (i/243)%3);
    slice[7] = (char)( (i/729)%3);
    slice[8] = (char)( (i/2187)%3);
}

char MarkerColor (int slice, int index)
{
    slice = slice / Power3[index];
    return char(slice%3);
}

//--------
// Clear Board
//-------
void inline ClearBoard(Board &CBoard)
{
    memset (&CBoard, 0 , sizeof (Board));
}

//===========================================================
// INITIALIZATION
//===========================================================
// Find the Moves for each slice of lengths 3 through 8
void InitMoveTable()
{
	int length, x, temp = 0, temp2 = 6561;
    char slice[9];
          
    for (length = 8; length > 2; length--)
	{
		for (x=0; x< temp2; x++)
		{
            SliceZ (x, slice);
            FindMoves (slice, x+ temp, Slice_Moves[x + temp], Slice_Moves2[x + temp], Square_Moves[x + temp], length, WHITE, temp);
            FindMoves (slice, x+ temp, Slice_Moves[x + temp], Slice_Moves2[x + temp], Square_Moves[x + temp], length, BLACK, temp);
        }
		temp+=temp2;
        temp2=temp2/3;
    }            
}

//-------------------------------
// This computes some flags, it might not be currently used.
void InitEvaluationTable()
{
	char slice[9]; 
    int x;
                
    for (x=0; x < 6561; x++)
    {
		SliceZ (x,slice);
        SliceEval (Slice_Moves[x], Slice_Moves2[x], slice, x);
	}
}

// ---------------------
// Initialize all the tables
// ---------------------      
void InitTables(void)
{
	InitMoveTable();
	FirstOnes();       
	InitRandomHash( );
	InitConversionTable();
	InitEvaluationTable();
	LoadCompBook();

	LoadAllCharCoeff ("newcoeff.dat");
	BlendCoeff (4, 0 , 1);
	BlendCoeff (5, 1 , 2);
	BlendCoeff (6, 2 , 3);
}

// ==================
// Find 
// (sq) -> Slice conversions
// and Slice -> (sq) 
// ==================
void InitConversionTable()
{
	for (int x=0; x < 64; x++)
	{
		uint8_t Movex = (x % 8) + 1;
		uint8_t Movey = (x / 8) + 1;

		// Corner Table
		Boardindex[(x<<4) + 8] = 0;
		Boardindex[(x<<4) + 9] = 0;

		if (Movex < 4 && Movey < 4)
		{
			Boardindex[(x<<4) + 8] = 0;
			Boardindex[(x<<4) + 9] = Power3[ Movex + Movey * 3 - 3 ];
		}

		if (Movex > 5 && Movey > 5)
		{	
			Boardindex[(x<<4) + 8] = 1; 
			Boardindex[(x<<4) + 9] = Power3[ (9-Movex) + (9-Movey) * 3 - 3 ];
		}

		if (Movex < 4 && Movey > 5)
		{	  
			Boardindex[(x<<4) + 8] = 2; 
			Boardindex[(x<<4) + 9] = Power3[ Movex + (9-Movey) * 3 - 3 ] ;
		}

  		if (Movex > 5 && Movey < 4)
		{	  
			Boardindex[(x<<4) + 8] = 3; 
			Boardindex[(x<<4) + 9] = Power3[ (9-Movex) + Movey * 3 - 3 ];
		}

		// South
		Boardindex[(x<<4) + 0]  = Movex; 
		Boardindex[(x<<4) + 1]  = Power3 [ Movey ];
		Boardindex3[(x<<3) + 1] = Movey;
	              
		Boardindex2.south[Movey + (Movex << 3)] = x;

		// East
		Boardindex[ (x<<4) + 2 ] = Movey; 
		Boardindex[ (x<<4) + 3 ] = Power3[ Movex ];
		Boardindex3[ (x<<3) + 3 ] = Movex;

		Boardindex2.east[Movex + (Movey << 3)] = x;
	             
		// SouthEast
		Boardindex[ (x<<4) + 4] = Movex - Movey +7;
	               
		if (Movey >Movex)
		{
			Boardindex [(x<<4) + 5] = Power3 [ Movex ];
			Boardindex3 [(x<<3) + 5]= Movex;
			Boardindex2.southeast[Movex + ((Movex -Movey +7) << 3)] = x;
		}
		else
		{
			Boardindex [(x<<4) + 5] = Power3 [ Movey ];
			Boardindex3 [(x<<3) + 5]= Movey;
			Boardindex2.southeast[Movey + ((Movex -Movey +7) << 3)] = x;
		}
	                                  
		// Southwest
		Boardindex [(x<<4) + 6] = Movey + Movex-2;
		if (Movey + Movex < 9)
		{
			Boardindex [(x<<4) + 7] = Power3 [ Movex ];
			Boardindex3 [(x<<3) + 7] = Movex;
			Boardindex2.southwest[Movex + ((Movex +Movey -2) << 3)] = x;
		}
		else
		{
			Boardindex [(x<<4) + 7] = Power3 [ 9-Movey ];
			Boardindex3 [(x<<3) + 7] = 9-Movey;
			Boardindex2.southwest[(9-Movey) + ((Movex +Movey -2) << 3)] = x;
		}                   
	} // end for x
}     

// ----------------
// Convert to different Board format
// (from char[64] used for GUI, to lookup slices used by AI)
// ----------------
void ConvertBoard( Board &board, char squareArray[] )
{
    ClearBoard( board );

	board.color = squareArray[64];
	board.cornerThreat  = 66;
	board.cornerThreat2 = 66;
         
    for (int i = 0; i < 64; i++)
	{
		const char piece = squareArray[i];
		if (piece != EMPTY)
		{
			board.nummarkers++;
			// Controls whether middle edge or 2X + edge patterns are used
			if (EdgeSwitch [ i ] == 1) { board.edge1 = 1; board.edge3 = 1;}
			else if (EdgeSwitch [ i ] == 2) { board.edge1 = 1; board.edge4 = 1;}
			else if (EdgeSwitch [ i ] == 3) { board.edge2 = 1; board.edge3 = 1;}
			else if (EdgeSwitch [ i ] == 4) { board.edge2 = 1; board.edge4 = 1;}
								
			if (piece == WHITE) UpdateIndicesW2(board, i); // White marker
						   else UpdateIndicesB2(board, i); // Black marker
        }
	}
}

// ==================
// For Finding Empty Squares
// and corner 2x5 and middle edge
// ==================
void FirstOnes( )
{
	// tables for empty squares
	for (int i = 0; i < 125 * 80; i++)
	{
		for ( int x = 1; x < 9; x++)
			if (MarkerColor(i, x) == EMPTY)
			{
				FirstOne[i] = x;
				x = 10;
			}
	}

	// Now tables for 2x5 corners
	for (int i = 0; i < 6562; i++)
	{
		for (int x = 1; x < 6; x++)
			First5 [i] += MarkerColor(i,x) * Power3[x];
		for (int x = 8; x > 3; x--)
			Last5 [i]  += MarkerColor(i,x) * Power3[9 - x];

		// and the 14 square, 4 empty edges
		for (int x = 2; x <= 7; x++)
			Middle6 [i] += MarkerColor(i,x) * Power3[x - 1];
		for (int x = 3; x <= 6; x++)
			Middle4 [i] += MarkerColor(i,x) * Power3[x - 2];

		Middle4[i] *=  Power3[6 + 1];

		// If an X square or corner is played, don't use the middle edge pattern.
		if (MarkerColor(i,2) != EMPTY || MarkerColor(i,7)!=EMPTY) Middle4[i] = -100000;
		if (MarkerColor(i,1) != EMPTY || MarkerColor(i,8)!=EMPTY) Middle6[i] = -100000;

		EdgeI[i].First5  = First5[i];
		EdgeI[i].Last5   = Last5[i];
		EdgeI[i].Middle4 = Middle4[i];
		EdgeI[i].Middle6 = Middle6[i];
		EdgeI[i].X1 = MarkerColor(i,2) * 6561 + MarkerColor(i,7) * 19683;
	}
}

//======================================
// Find (Precompute) moves possible for each of the 3^8 slices
//======================================
// Outputs the lookup value for the new list for placing a piece in any empty Square
// And fills in possible moves along the slice that a player may make.
void FindMoves( char slice[9], int slicenum, Move &Eval, Move2 &Eval2, SqMove &Sq, int length, char color, int offset )
{
	int8_t newslice[9];
    int move = 0, flipped;
    uint8_t i; // piece of the slice the program is checking.
    int x,y;
	uint8_t left, right;
	uint8_t ocolor = color^3; // opponents color
    
    for (i=0; i<3; i++)
        if (color==WHITE) 
			Eval.WMoves[i]= 0;
		else 
			Eval.BMoves[i] = 0;
 
	for (i=1; i <= length; i++)
	{
		int t;
		for (t = 1; t <= length; t++) { 
			newslice[t] = slice[t]; 
		}

		// Start at marker placed
		left = right = i; 
	          
		if (slice[i]==0)
		{
			x=0; y=0;
			newslice[i]=color;
	                
			// Check right
			if (i < length-1 && slice[i+1] == ocolor)
				for ( x=i+1; x<=length; x++)
				{
					if (slice[x]==color) {right=x-1; x=10; }
					else if (slice[x]==0) x=8;
				}
	                   
			if (x>=10) {
				t=i;
				while (slice[t]!=color)	{
					newslice[t]=color;
					t++;
					}
			}
	                           
			// Check left                    
			if (i > 2 && slice[i-1] == ocolor)                
				for (y=i-1; y>0; y--)
				{
					if (slice[y]==color) {left=y+1; y=-1; }
					else if (slice[y]==0) y = 1;
				}
	                                                          
			if (y<=-1)  
			{ // Can move there.
				t=i;
				while ( slice[t] != color )
				{
					newslice[t]=color;
					t--;
				}
			}   
	              
			if (x<10 && y>-1) 
				newslice[i] = color; // Just place a marker, nothing flipped.
				else
				{ 
					for (int t=0; t<3; t++)
					{
						if (color == WHITE && Eval.WMoves[t]==0)
						{
							Eval.WMoves[t] = i; // White can move here
							Eval2.WF[t].left  = left;
							Eval2.WF[t].right = right;
							Sq.W[i-1].left = left;
							Sq.W[i-1].right = right;

							flipped = 0;
							for (int tmp = left; tmp <= right; tmp++)
								if (tmp!=i) flipped++;
							t=4;
							Wflipped [slicenum] = flipped;
						}
					if (color == BLACK && Eval.BMoves[t]==0) 
					{
						Eval.BMoves[t] = i; 
						Eval2.BF[t].left  = left;
						Eval2.BF[t].right = right;
						Sq.B[i-1].left = left;
						Sq.B[i-1].right = right;

						flipped =0;
						for (int tmp = left; tmp <= right; tmp++)
							if (tmp!=i) flipped++;
							t=4;
							Bflipped [slicenum] = flipped;
						}
				}                  
			}                                                   
		} // end slice[i]==0
	} // end for i
} // end find moves

// ------------------
// Find the Flags
// change the function name
// -------------------
void SliceEval( Move &Eval, Move2 &Eval2, char slice[9], int slicenum)
{   
	int i;
	int X1 = EMPTY, X2 = EMPTY, Take1 = 0, Take2 = 0;

	// Greedy Evaluation
	Mobility[ slicenum ] = 0;
	EdgeV[ slicenum ] = 0;
	RandomV[ slicenum ] = 0;
	DiagV[ slicenum ] = 0;

	if (slice[1] == WHITE) EdgeV[ slicenum ] += 22;
	if (slice[8] == WHITE) EdgeV[ slicenum ] += 22;
	if (slice[1] == BLACK) EdgeV[ slicenum ] -= 22;
	if (slice[8] == BLACK) EdgeV[ slicenum ] -= 22;

	for (i = 1; i < 8; i++) 
	{
		if (slice[i] == WHITE) {
			EdgeV[ slicenum ] += 3;
			RandomV[ slicenum ] += 3;
		}
		if (slice[i] == BLACK) {
			EdgeV[ slicenum ] -= 3;
			RandomV[ slicenum ] -= 3;
		}
	}

	if (slice[1] == EMPTY)
	{
		if (slice[2] == WHITE) {EdgeV[slicenum] -=4; DiagV[ slicenum ] -=10;}
		if (slice[2] == BLACK) {EdgeV[slicenum] +=4; DiagV[ slicenum ] +=10;}
	}
	if (slice[8] == EMPTY)
	{
		if (slice[7] == WHITE) {EdgeV[slicenum] -=4; DiagV[ slicenum ] -=10;}
		if (slice[7] == BLACK) {EdgeV[slicenum] +=4; DiagV[ slicenum ] +=10;}
	}

	for (i = 0; i < 3; i++)
	{
		if ( Eval.WMoves[i]!=0 ) Mobility[ slicenum ] += 5;
		if ( Eval.BMoves[i]!=0 ) Mobility[ slicenum ] -= 5;
	}

	//  X Squares Flags on length 8 diagonals
	//-------------
    i = 1;             
	if (slice[i+1]==BLACK ) if (Eval.WMoves[0] == i || Eval.WMoves[1] == i)	{X1 = BLACK; Take1 = 1;}
																	  else  {X1 = BLACK; }

    if  (slice[i+1]==WHITE) if (Eval.BMoves[0] == i || Eval.BMoves[1] == i) {X1 = WHITE; Take1 = 1;}
																	  else  {X1 = WHITE; }
                                        
	i = 8;
	if (slice[i-1]==WHITE) if (Eval.BMoves[0] == i || Eval.BMoves[1] == i)  {X2 = WHITE; Take2 = 1;}
																	   else {X2 = WHITE; }

    if (slice[i-1]==BLACK) if (Eval.WMoves[0] == i || Eval.WMoves[1] == i)	{X2 = BLACK; Take2 = 1;}
																	  else  {X2 = BLACK; }
 							
	DiagI[ slicenum ].XSquare1    = X1 * 6561;
	DiagI[ slicenum ].CornerTake1 = Take1 * 19683;
	DiagI[ slicenum ].XSquare2    = X2 * 6561;
	DiagI[ slicenum ].CornerTake2 = Take2 * 19683;

	quiesce[ slicenum ] = 0;
	if (X1 != EMPTY && Take1==0 && slice[1] == EMPTY) quiesce[ slicenum ] += 1;
	if (X2 != EMPTY && Take2==0 && slice[8] == EMPTY) quiesce[ slicenum ] += 2;
}

// --------------------------
// Sort a Movelist
// --------------------------
inline void ShellSort( SingleMove *Moves, int start, int total )
{
   SingleMove temp;

   for (int i = 3 + start; i <= total; i++)
   {
		temp = Moves[i];
		int j = i;
		while (j>=(3 + start) && Moves[j-3].eval < temp.eval)
		{
			Moves[j] = Moves[j-3];
			j-=3;
		}
		Moves[j]=temp;
	}

	for (int i = 1 + start; i <= total; i++)
	{
		temp = Moves[i];
		int j = i;
		while (j>=(1 + start) && Moves[j-1].eval < temp.eval)
		{
			Moves[j] = Moves[j-1];
			j-=1;
		}
		Moves[j]=temp;
	}
}

// Called from the move sorting
int ShallowSearch(char color, int ahead, int &bestmove, int alpha, int beta, int depth)
{
	if (depth == 0) return EvaluateBoard(BoardStack[ahead-1]);
	int oldAhead, eval;
        
	BoardStack[46] = BoardStack[ahead-1];
	oldAhead = CurrentAhead;
	CurrentAhead = 46 + depth;

	int oldFinal = final;
	final = 3;
	eval = AlphaBetaSearch(color, 46, alpha - 80, beta + 80, bestmove);
	final = oldFinal;
	CurrentAhead = oldAhead;

	return eval;
}

//---------------------
// Sort a movelist Best to Worst using 1 ply search
//---------------------
int inline SortMoves( SingleMove *Moves, char color, int total, int ahead, int bestmove, int alpha, int beta )
{
	int start = (bestmove == INVALID_MOVE) ? 1 : 2;

    for (int i = start; i <= total; i++)
	{
		BoardStack[ahead] = BoardStack[ahead-1];
        DoMove(BoardStack[ahead], Moves[i], color);

		if (color == WHITE) {
			Moves[i].eval = EvaluateBoard( BoardStack[ahead] );
			HistoryWhite[ Moves[i].sq ] = Moves[i].eval;
		} else {
			Moves[i].eval = -EvaluateBoard( BoardStack[ahead] );
			HistoryBlack[ Moves[i].sq ] = Moves[i].eval;
		}
     }

	ShellSort( Moves, start, total );

	return 0;
}

// -------------------------------
void inline SortMovesHistory( SingleMove *Moves, char color, int bestmove, int total )
{
	const int start = (bestmove == 127) ? 1 : 2;
	const int* History = (color == BLACK) ? HistoryBlack : HistoryWhite;

	for (int i = start; i <= total; i++)
		Moves[i].eval = History[ Moves[i].sq ];

	ShellSort( Moves, start, total );
}

//
// Same as the the normal SortMoves, but using enhanced transposition table cuts.
// This function also sorts according to the fastest first heuristic
// -------------------------------
int SortMovesETC(SingleMove *Moves, int8_t color, int numMoves, int ahead, int &bestmove, int alpha, int beta)
{
	uint32_t hi = 0, hashvalue = 0;
	
	if (hashing == 0) return SortMoves(Moves, color, numMoves, ahead, bestmove, alpha, beta);

	char ocolor = color ^ 3;
	int start = (bestmove == INVALID_MOVE) ? 1 : 2;
	int searchVal = (color == WHITE) ? beta : -alpha;
	bool fastestFirst = (BoardStack[ahead - 1].nummarkers <= 54);
	int tempbest = INVALID_MOVE;

   for (int i = start; i <= numMoves; i++)
   {
		BoardStack[ahead] = BoardStack[ahead-1];
		DoMove(BoardStack[ahead], Moves[i], color);

		int eval = INVALID_VALUE;
		TT_Lookup( eval, tempbest, alpha, beta, ahead, color, hi, hashvalue );
		if (eval != INVALID_VALUE)
		{
			if (color == WHITE && eval >= beta) 
			{
				bestmove = Moves[i].sq;
				return 1;
			}
			if (color == BLACK && eval <= alpha) 
			{
				bestmove = Moves[i].sq; 
				return -1;
			}
		}

		int depth = 0;
		if (BoardStack[ahead].nummarkers  > 46 || (start == 2 && ahead > 1) ) depth = 0;
			else if (BoardStack[ahead].nummarkers  > 44 || final!=1) depth = 1;
			else if (BoardStack[ahead].nummarkers  > 45 ) depth = 2;
			else depth = 3;

		if (color == WHITE) {
			Moves[i].eval = ShallowSearch( ocolor, ahead+1, tempbest, alpha, beta, depth);
			HistoryWhite[ Moves[i].sq ] = Moves[i].eval;
		} else {
			Moves[i].eval = -ShallowSearch( ocolor, ahead+1, tempbest, alpha, beta, depth);
			HistoryBlack[ Moves[i].sq ] = Moves[i].eval;
		}

		eval = Moves[i].eval - searchVal + 3;

		// fastest first
		if (fastestFirst == 1 && (eval > -14)) {
			int nextNumMoves = 0;
			GenerateMoves( BoardStack[ahead], ocolor, &MoveStack[54 * 32], nextNumMoves, bestmove );
			Moves[i].eval += (17- nextNumMoves) * ((eval+30)>>1);
		}
	} // end for

	ShellSort( Moves, start, numMoves );

	return 0;
}

// ------------ Sort Moves --------------
// Sort moves by their 1 ply Evaluation on depths up to SearchDepth - ?
// after that use History Heuristic
// For endgames use ETCcut and fastest first
//
int inline SortAllMoves( const int &color, const int &moveNum, unsigned int &ahead, int &bestmove, int &alpha, int &beta, int &sorted)
{
	int ETCcut = 0;

	if ( final == 0 || (final>=2 && ahead > 4)) 
	{   
		// Midgame
		if (ahead <= SortDepth[CurrentAhead]) 
		{
			SortMoves(&MoveStack[ahead<<5], color, moveNum, ahead, bestmove, alpha, beta);
			sorted = 1;
		}
		else if (ahead < CurrentAhead) {
			SortMovesHistory(&MoveStack[ahead << 5], color, bestmove, moveNum);
		}
	}
	else if (ahead < (CurrentAhead - 1))
	{
		// Endgame
		if ( BoardStack[ ahead-1 ].nummarkers <= 55 ) 
		{
			ETCcut = SortMovesETC(&MoveStack[ahead<<5], color, moveNum, ahead, bestmove, alpha, beta);
			sorted = 1;
		}
		else if (bestmove == INVALID_MOVE) {
			SortMovesHistory(&MoveStack[ahead << 5], color, bestmove, moveNum);
		}
	}

	return ETCcut;
}

// ------------
// Display thinking info & check for time running out
// ------------
int inline CheckTime( float ExtraSecs )
{
	if ( checkNum == 1 ) ExtraSecs = 0;
	if ( MaxThinkSec > 0)
		if ((clock() - g_start) > CLOCKS_PER_SEC * (MaxThinkSec + ExtraSecs)) return TIMEOUT;
	return 0;
}

int inline CheckTimeAndDisplayInfo( float ExtraSecs )
{
	if (nodes <= nodesDisplay) return 0;
	if (final == 0) 
		nodesDisplay = nodes + 200000; 
	else 
		nodesDisplay = nodes + 421320;
	
	end = clock();	
	DisplayNodes( currentBest, SearchEval, 0, end-g_start, 1 );
	return CheckTime( ExtraSecs );
}

// ---------------------------------------
// Is an important diagonal contested? This can be used to control search pruning decisions
// Is this useful? Somewhat.
int Diagonals( Board &board, int color, int nChange)
{
	int q1, q2, change, x1, x2, x3, x4;

	if (board.nummarkers < 35 ) return 0;

	q1 = quiesce [board.southeast[7] ];
	q2 = quiesce [board.southwest[7] ];

	if (q1 == 0 && q2 == 0) return 0;

	x1 = DiagI[ board.southeast[7] ].XSquare1;
	x3 = DiagI[ board.southeast[7] ].XSquare2;
	x2 = DiagI[ board.southwest[7] ].XSquare1;
	x4 = DiagI[ board.southwest[7] ].XSquare2;
	CCoeffs = &Coeffs[ gameStage[board.nummarkers] ];

	if ((q1&1) == 1 && x1 != color * 6561) 
	{
		 change = Coeffs->EdgeX2[board.east[1] + x1 ] - Coeffs->EdgeX2[board.east[1] ]
			    + Coeffs->EdgeX2[board.south[1] + x1 ] - Coeffs->EdgeX2[board.south[1] ];
		 if (abs (change) > nChange) return 1;
	}

	if ((q2&1) == 1 && x2 != color * 6561) 
	{
		change = Coeffs->EdgeX2[ board.east[8] + x2  ] - Coeffs->EdgeX2[ board.east[8] ]
			    + Coeffs->EdgeX2[ board.south[1] + x2 * 3 ] - Coeffs->EdgeX2[ board.south[1] ];
		 if (abs (change) > nChange) return 1;
	}

	if ( q1 >= 2 && x3 != color * 6561)
	{
		change = Coeffs->EdgeX2[board.east[8] + x3 * 3] - Coeffs->EdgeX2[ board.east[8] ]
			    + Coeffs->EdgeX2[board.south[8] + x3 * 3] - Coeffs->EdgeX2[ board.south[8] ];
		 if (abs (change) > nChange) return 1;
	}

	if ( q2 >= 2 && x4 != color * 6561)
	{
		change = Coeffs->EdgeX2[board.east[1] + x4 * 3] - Coeffs->EdgeX2[board.east[1] ]
			    + Coeffs->EdgeX2[board.south[8] + x4 ] - Coeffs->EdgeX2[board.south[8] ];
		 if (abs (change) > nChange) return 1;
	}	

	return 0;
}

// --------------------------------
// Verify Value:
// checks to make sure the value is unlikely to change greatly to help pruning decisions 
// --------------------------------
inline int VerifyValue( Board &board, int value, int nChange )
{
	if (value == 1) // positive value, good for white
	{
		if (board.cornerThreat2 != INVALID_SQ && board.color == BLACK) return 0;
		if (Diagonals(board, WHITE, nChange) == 1 ) return 0; // check if black may be holding a key diagonal
	}
	else
	{
		if (board.cornerThreat2 != INVALID_SQ && board.color == WHITE) return 0;
		if (Diagonals(board, BLACK, nChange) == 1) return 0; // check if white holds a key diagonal
	}

	return 1;
}

// ------------
// PRUNING ( sometimes prunes wrong, but a good improvement for overall play )
//
// If the evaluation for this board and the board before it (or before that too for endgame)
// are a certain level above beta, or below alpha, then alpha or beta is returned without 
// further searching.
// Verify Value is called to make sure no important diagonal/corner is contested
// ------------
void inline DoPruning( int &eval, const int &alpha, const int &beta, const unsigned int &ahead )
{
	// EndGame Forward Pruning if it's on for this iteration (for win/loss search only)
	if (EndGamePrune == 1 && final == 1 && winloss == 1 && BoardStack[ahead].nummarkers < 56 ) 
	{
		BoardStack[ahead].eval = EvaluateBoard (BoardStack[ahead]);
		if (ahead < 3) return;
		if (BoardStack[ahead].eval <-EndPruneRange && BoardStack[ahead-1].eval < -EndPruneRange && BoardStack[ahead-2].eval - 5 < -EndPruneRange
			&& VerifyValue( BoardStack[ahead], -1, 68) == 1 && Diagonals(BoardStack[ahead-1], BLACK, 68) == 0) eval =- 8;
			else if (BoardStack[ahead].eval > EndPruneRange  && BoardStack[ahead-1].eval > EndPruneRange && BoardStack[ahead-2].eval + 5 > EndPruneRange
			&& VerifyValue( BoardStack[ahead], 1, 68) == 1 && Diagonals(BoardStack[ahead-1], WHITE, 68) == 0 ) eval = 8; 
	}

	// Midgame Forward Pruning
	if ((final==0 || final == 2) && ahead > 6 && CurrentAhead > 8 )
	{
		if (ahead == 7) BoardStack[ahead].eval = EvaluateBoard(BoardStack[ahead]);
		else
		{
			eval = BoardStack[ahead].eval = EvaluateBoard(BoardStack[ahead]);
			int eval2 = BoardStack[ahead-1].eval; 
			int temp = cutLevel[ BoardStack[ahead].nummarkers ];
			if (ahead < 10) temp+=2;

			if ((eval - temp) >= beta && (eval2 - temp + 3) >= beta && VerifyValue (BoardStack[ahead], 1, 42) == 1) eval = beta; 
			else if ((eval + temp)  <= alpha && (eval2 + temp - 3) <= alpha && VerifyValue (BoardStack[ahead], -1, 42) == 1) eval = alpha;
			else eval = -9999;
		}
	}
}

//
// If a corner is under threat, see if taking it improves the eval
//
void inline LeafEval( Board& board, int &eval, int color )
{
	eval = EvaluateBoard(board);
	if (board.cornerThreat2 != INVALID_SQ )
	{
		if ( DoOneSquareMove(board, board.cornerThreat2, color^3) )
		{ 
			int temp = EvaluateBoard(board);
			nodes++;
			if (color == WHITE && temp < eval) eval = temp;
			if (color == BLACK && temp > eval) eval = temp;
		}
	 }
}

//===========================================================
// Alpha Beta
//===========================================================
int AlphaBetaSearch(int color, unsigned int ahead, int alpha, int beta, int &bestmove)
{
	int Max, sorted = 0;
	uint32_t hi, hashvalue;
	int cuteval = -2000;
	int eval = INVALID_VALUE;

	if ( ahead == 0 )
		currentBest = bestmove;

	if ( stopThinking == 1) return TIMEOUT;
	if ( CheckTimeAndDisplayInfo( ExtraSecs ) == TIMEOUT ) return TIMEOUT;

	ahead++;

	// Maybe we can avoid generating all the moves if the Killer Move works
	if (final==0 && ahead > 2 && ahead >= CurrentAhead )
	{
		BoardStack[ahead] = BoardStack[ahead-1];
		nodes++;

		if ( DoOneSquareMove( BoardStack[ahead], Killer[ahead], color) )
		{
			LeafEval( BoardStack[ahead], cuteval, color );
			if (color == WHITE && cuteval >=beta ) {bestmove = Killer[ahead]; return beta;}
			if (color == BLACK && cuteval <=alpha) {bestmove = Killer[ahead]; return alpha;}
		}
	}

	// Find possible moves for color
	int numMoves = 0;
	if (GenerateMoves(BoardStack[ahead-1], color, &MoveStack[ahead<<5], numMoves, bestmove) == 0)
		bestmove = INVALID_MOVE;
	
	// Pass?
	if (numMoves == 0)
	{
		color ^= 3; // switch color
		nodes++;

		GenerateMoves(BoardStack[ahead-1], color, &MoveStack[ahead<<5], numMoves, bestmove);

		if (numMoves == 0)
		{	// 2 passes, game over
			if (BoardStack[ahead-1].nummarkers + (CurrentAhead - ahead) >= 64)
				return FinalEvalBoard( BoardStack[ahead-1].nummarkers, BoardStack[ahead-1].numblack );
			else 
			{
				if ( (2* BoardStack[ahead-1].numblack) > BoardStack[ahead-1].nummarkers) 
			 		return -390;
				else 
					return 390;					
			}
		}
	}

	// Sort Moves
	int ETCcut = SortAllMoves( color, numMoves, ahead, bestmove, alpha, beta, sorted);
	if (ETCcut == 1) {
		nodes++; 
		return beta;
	}
	if (ETCcut == -1) {
		nodes++; 
		return alpha;
	}

	// ----------------------
	Max = (color == WHITE) ? -999999 : 999999;

	// Play and Evaluate the Moves
	for (int i = 1; i <= numMoves; i++)
	{
		SingleMove move = MoveStack[i + (ahead<<5)];
		// Check to see if this move was tried as a killer, 
		if (ahead == CurrentAhead && cuteval != -2000 && move.sq == Killer[ahead]) {
			eval = cuteval;
		}
		else
		{
			BoardStack[ahead] = BoardStack[ahead-1];
            DoMove(BoardStack[ahead], move, color);
                 
			nodes++; // Increment Nodes Searched

			if (ahead==1)  // set info for display
			{
				checkMove = MoveStack[i + 32].sq; 
			    checkNum = i;
				if ( abs(Max) < 90000) {
					SearchEval = Max; 
					currentBest = bestmove;
				}
				if ( i > 1 && CheckTime( 0 ) == TIMEOUT ) return TIMEOUT;
			}
			else
			{
				// eliminations?
				if (BoardStack[ahead].numblack == 0) {
					if (final == 1) return 64 * 3; else return 400;
				}
				if (BoardStack[ahead].numblack == BoardStack[ahead].nummarkers) {
					if (final == 1) return -64 * 3; else return -400;
				}
			}
				
			if (BoardStack[ahead].nummarkers == 64) // Board filled, game is over
			{
                bestmove = move.sq;
				return FinalEvalBoard( BoardStack[ahead].nummarkers, BoardStack[ahead].numblack );
            }      
				
            // If this is the max depth, evaluate the board position
            if ( ahead == CurrentAhead )
			{
				LeafEval( BoardStack[ahead], eval, color ); 
			}
            // If not look ahead to all possible move
            else  
			{
				refuteMove[ahead] = move.sq;
				int nextbest = 127;
				eval = INVALID_VALUE;

				TT_Lookup( eval, nextbest, alpha, beta, ahead, color, hi, hashvalue );

				if (eval == -9999) DoPruning( eval, alpha, beta, ahead );

                if (eval == -9999)
				{
					 if (i == 1) 
						 Killer[ ahead + 2] = MoveStack[2 + (ahead<<5)].sq;
					 else  
						 Killer[ ahead + 2] = MoveStack[1 + (ahead<<5)].sq;

					// Quicker endgame for 7 empty
					if (final == 1 && BoardStack[ahead].nummarkers >= (64 - EMPTYEND) ) 
					{
						FindEmptySquares( BoardStack[ ahead ], EmptySquares, nextbest );
						eval = SquareDoMoves( BoardStack[ ahead+1], ahead, alpha, beta, color^3, EmptySquares, 0, nextbest);
					}
					else
					{	// Recurse
						eval = AlphaBetaSearch( color^3, ahead , alpha, beta, nextbest);
                        if (eval == TIMEOUT) return TIMEOUT;
					}
                        
					TT_Write( eval, nextbest, alpha, beta, ahead, hi, hashvalue );
				}
			}
		}

		// Keep Track of Best Move and Alpha-Beta Prune
		if (color==WHITE && eval > Max )
		{
			Max = eval;
			if (ahead == 1)	bestmove = move.sq;
			if (eval > alpha) {
				bestmove = move.sq;
				if ( eval >= beta ) return beta;
				alpha = eval;
			}
		}
		if (color==BLACK && eval < Max )
		{
			Max = eval;
			if (ahead == 1)	bestmove = move.sq;
			if (eval < beta){
				bestmove = move.sq;
				if (eval <= alpha ) return alpha;
				beta = eval;
			}
		}
	} // end for

	return Max;
}

// ---------------------------------
// Perft
// ---------------------------------
uint64_t Perft( Board board, int ahead, int maxLevel )
{
	int numMoves = 0;
	int bestmove = 0;
	GenerateMoves( board, board.color, &MoveStack[ahead<<5], numMoves, bestmove );
	if ( ahead == maxLevel )
		return (numMoves > 0) ? numMoves : 1;

	if ( numMoves == 0 )
	{
		Board newBoard = board;
		newBoard.color ^= 3;
		return Perft( newBoard, ahead+1, maxLevel );
	}

	uint64_t numMoves2 = 0;
	for ( int i = 1; i <= numMoves; i++ )
	{
		SingleMove MMove = MoveStack[i + (ahead<<5)];
		Board newBoard = board;
		DoMove( newBoard, MoveStack[i+(ahead<<5)], newBoard.color );
		numMoves2 += Perft( newBoard, ahead+1, maxLevel );
	}
	return numMoves2;
}

// ---------------------------------
// Perft
// ---------------------------------
void StartPerft(char board[64])
{
	char buffer2[8192];
	Board CBoard;
	ConvertBoard (CBoard, board);

	int j = 0;
	for ( int level = 1; level <= 7; level++ )
	{
		unsigned long long numMoves = Perft( CBoard, 1, level );
		j+=sprintf( buffer2+j, "(%d) Num Leaf Nodes: %llu\015\012", level, numMoves );
	}

	DisplayText( buffer2, true );
}

// ---------------------------------
// Display Transposition Table entry
// ---------------------------------
void DisplayTT( char board[64] )
{
	int eval;
	char failType;
	int bestmove;
	int depth;

	Board CBoard;
	ConvertBoard (CBoard, board);

	if ( !TT_GetInfo( CBoard, eval, failType, bestmove, depth ) )
		return;

	char buffer2[512];
	int j = 0;
	j+= sprintf( buffer2+j, "Evaluation: %c%.2f\015\012", failType, (float)eval/ 3.2f);
	j+= sprintf (buffer2+j, "Move: %c%c\015\012", (bestmove & 7 ) + 'a', (bestmove >> 3) + '1' );
	j+= sprintf( buffer2+j, "Depth: %d\015\012", depth );
	DisplayText( buffer2, true );
}

// ---------------------------------
// Display Board Evaluation
// ---------------------------------
int DisplayBoardEval(char board[64])
{
	char buffer2[512];
	Board CBoard;

	ConvertBoard (CBoard, board);
	int eval = EvaluateBoard (CBoard);
	sprintf( buffer2, "Evaluation: %.2f", (float)eval/ 3.2f);

	DisplayText( buffer2, true );

	return 1;
}

// ==========================================================
// Display Search Info
// ==========================================================
void DisplayNodes( int move, int Eval, int inbook, int ticks, int question )
{
    if (SearchInfo != 1 && thinkingType != ANALYZE) 
		return;

	char buffer[160];
	char buffer2[80];
	int depth, j, color = GameBoard[64];
	int blackNum, whiteNum;

	buffer[0] = NULL;

	if ( CurrentAhead + BoardStack[0].nummarkers > 64 )
		depth = 64 - BoardStack[0].nummarkers; 
	else 
		depth = CurrentAhead;

	if ( move != 127 )
	{
		// Best move found so far
		j = sprintf (buffer, "Move: %c%c", (move & 7 ) + 'a', (move >> 3) + '1' );

		// Highlight the best move found so far
		if ( move != g_suggestSquare )
		{
			g_suggestSquare = move;
			if ( flipi == -1 )
				DrawBoard( NULL, GameBoard );
		}
		
		// Variation being searched
		if (checkMove != INVALID_SQ) {
			j += sprintf(buffer + j, " (%c%c", (checkMove & 7) + 'a', (checkMove >> 3) + '1');
		}
		if (refuteMove[2] != 66 && refuteMove[2] != 127) {
			if (depth >= 23 || (final == 0 && depth >= 12))
			{
				if (depth > 24)
				{
					j += sprintf(buffer + j, "%c%c%c%c...", (refuteMove[2] & 7) + 'a', (refuteMove[2] >> 3) + '1', (refuteMove[3] & 7) + 'a', (refuteMove[3] >> 3) + '1');
				}
				else j += sprintf(buffer + j, "%c%c...", (refuteMove[2] & 7) + 'a', (refuteMove[2] >> 3) + '1');
			}
		}
		strcat (buffer, ")\015\012");
	}

	if (winloss && abs (Eval) < 10000 && (final == 1 || final == 3))
	{
		if (Eval != -1001)
		{
			if (Eval == 0) 
				Eval = 500; 
			else if ((Eval <= -1 && color == BLACK) || (Eval >= 1 && color == WHITE))  
				Eval = 1000; 
			else if ((Eval >=  1 && color == BLACK) || (Eval <= -1 && color == WHITE)) 
				Eval = -1000;
		}
	}
    if (color == BLACK && Eval < 1000 && Eval > -1000 && Eval!=500) 
		Eval = - Eval;

    if ((final == 1 || final == 3)) 
		strcat (buffer, "Final Eval: " );
    else 
		strcat (buffer, "Eval: " );
        
	if (abs(Eval) > 10000 || abs(Eval) == 1001) strcat (buffer, "?? " );
    else if (Eval == 1000) strcat (buffer, "WIN " );
    else if (Eval == -1000) strcat (buffer, "LOSS " );
    else if (Eval == 500) strcat (buffer, "DRAW " ); 
    else 
	{
		if ((final ==1 || final ==3) && question == 1)  
		{
			if (Eval < 0) strcat (buffer, " <= ");
			else strcat (buffer, " >= ");
		}
		if ((final==1 || final == 3) || inbook != 0) 
		{
			if (inbook==0 ) Eval/=3; 
			_ltoa (Eval, buffer2, 10);

			if (inbook == 0) 
			{
				if (color == BLACK) Eval = -Eval;
				blackNum = 32 - Eval/2;
				whiteNum = 32 + Eval/2;
				if ( (Eval % 2) != 0) 
				if ( Eval >0) blackNum--;
						else  whiteNum--;
				if (color == BLACK) Eval = -Eval;
				sprintf (buffer2, "%d   ( %d - %d )", Eval, blackNum , whiteNum);
			}
		}
		else sprintf (buffer2, "%.1f", float (Eval / 3.2f));

			if (Eval > 0) strcat (buffer, "+");
			strcat (buffer, buffer2);
			if (inbook != 0) strcat (buffer, " (book) " );
		}
		if ((final == 1 || final==3 ) && (Eval == 1000 || Eval == -1000 || Eval == 500)) 
		{
			if ( EndGamePrune == 0 && (checkNum > 1 || move == 127 ) ) {

			} else if ( EndGamePrune == 0 || (EndPruneRange >= 40 && (checkNum > 1 || move == 127))  )
				strcat( buffer, "?" ); // light win/loss pruning
			else 
				strcat( buffer, "??" ); // heavy win/loss pruning
		}

	strcat( buffer, " \015\012" );

	// Depth/Speed, etc.
	if ( !inbook )
	{
		sprintf( buffer2, "Depth: %d \015\012", depth );
		strcat( buffer, buffer2 );

		if (nodes > 1000000 ) {
			sprintf(buffer2, "Nodes: %.2f M \015\012", (nodes/ 1000000.0f));
            strcat(buffer, buffer2);
		}
		else if (nodes > 1000) {
			sprintf( buffer2, "Nodes: %I64d K \015\012", (nodes/ 1000));
            strcat( buffer, buffer2);
        } else {
			sprintf (buffer2, "Nodes: %I64d \015\012", nodes);
            strcat (buffer, buffer2);
        }

		if (ticks > 4) {
			sprintf( buffer2, "Speed: %I64d Kn/s \015\012", (CLOCKS_PER_SEC * (nodes / 1000) / ticks));
			strcat( buffer, buffer2);
			sprintf( buffer2, "Time: %.1f s \015\012", float (ticks) / CLOCKS_PER_SEC);
			strcat( buffer, buffer2);
		}
	}
		
    DisplayText( buffer, true );
}

// ---------------------------------
// Add the board postion CBoard to a saved file
// ---------------------------------
void SavePosition( Board &CBoard, char *filename, unsigned char solved )
{
	FILE *FP = fopen( filename, "ab" );
	if (FP)
	{
		fwrite(&CBoard.east[1], sizeof(short), 8, FP);
		fwrite(&CBoard.nummarkers, 1, 1, FP);
		fwrite(&CBoard.color, 1, 1, FP);
		fwrite(&solved, 1, 1, FP);
		fclose(FP);
	}
}

// ---------------------------------
int AddGameLog( char CompColor )
{
	FILE* fp = fopen( "gamelog.txt", "at" );
	if (fp)
	{
		if (CompColor == WHITE) fprintf(fp, "W ");
		if (CompColor == BLACK) fprintf(fp, "B ");
		if (CompColor == BOTH)  fprintf(fp, "S ");

		int WhiteM, BlackM;
		NumMarkers(GameBoard, WhiteM, BlackM);
		if (WhiteM == BlackM) fprintf(fp, " (draw) ");
		else if (WhiteM > BlackM && CompColor == WHITE) fprintf(fp, " (win) ");
		else if (WhiteM < BlackM && CompColor == BLACK) fprintf(fp, " (win) ");
		else fprintf(fp, " (loss) ");

		fprintf(fp, "%d - %d \n", BlackM, WhiteM);

		fwrite(&Transcript[2], 120, 1, fp);
		fprintf(fp, "\n\n");

		fclose(fp);

		DisplayText("Added Game to gamelog.txt");
	}

	return 1;
}
//
// Set Max Think Sec from time remaining on clock
//
void SetThinkTime( char CompColor, float &MaxThinkSec, float &ExtraSecs, int bFinal, bool bAnalyze )
{
	unsigned int  ComputerTime;
	if (CompColor == BLACK) ComputerTime = TimeLimit * 60 - BlackTime - 2;
	if (CompColor == WHITE) ComputerTime = TimeLimit * 60 - WhiteTime - 2;

	if (ComputerTime > 100000 || ComputerTime < 2) MaxThinkSec = 2;
	else if (startMarkers >= 48 || TimeLimit == 0) 
	{
		MaxThinkSec = (float)ComputerTime;
		if (MaxThinkSec <= 0) MaxThinkSec = 2;
	}
	else 
	{
		if (bFinal) MaxThinkSec = (float)ComputerTime / 2;
		else MaxThinkSec = 2 * (float)ComputerTime / (53 - startMarkers);
	}
 
	ExtraSecs = 0;
	if (TimeDependent == 0 || bAnalyze ) MaxThinkSec = 0;
	else if (MaxThinkSec <= 0 || MaxThinkSec > 100000) MaxThinkSec = 2;
	else if ((ComputerTime - MaxThinkSec * 1.8) > 2) ExtraSecs = MaxThinkSec *.8f;
}

// ---------------------------------
// Computer Thinks and returns a move....
// ---------------------------------
int ComputerThink( char CompColor, char board[64], bool analyzeMode = false )
{
	int inBook;
	int LastEval = 0, Scope = 1440, oldEval = -1001;
	int bestmove, mid, question = 0;
	unsigned int MaxAhead;
	short MoveX, MoveY;
	Board TempBoard;
	static int LastWinLoss = 0;
	const int incPly = 2;
	 
	bestmove = 127;
	MaxAhead = SearchDepth;                    
	nodes = 0; nodesDisplay = nodes + 100000;
	final = 0; winloss = 0; checkMove = 66; mid = -99;
	EndGamePrune = 0;

	ConvertBoard( BoardStack[0], board );
	TT_AgeInc( );

	startMarkers = BoardStack[0].nummarkers;
	if (startMarkers == 64) 
		return 0;// Game is already over

	SetThinkTime( CompColor, MaxThinkSec, ExtraSecs, 0, analyzeMode );
	g_start = clock();
	// Set Lookahead Based on Difficulty Level
	CurrentAhead = MaxAhead;

	if (brainType !=2)	
	{
		if ((brainType == 0 && EndgameDepth+2 >= (64-startMarkers)) || thinkingType == WINLOSS) {
			CurrentAhead = MaxAhead; final = 1; winloss = 1;
		}
		if (EndgameDepth >= (64-startMarkers) || thinkingType == PERFECT ) {
			CurrentAhead = 22; LastEval = 32; Scope = 440; final = 1; winloss = 0;
		}
	}
	if (final !=1 && TimeDependent && (LastWinLoss == startMarkers-2 || LastWinLoss == startMarkers-1) ) 
	{
		SetThinkTime( CompColor, MaxThinkSec, ExtraSecs, 1, analyzeMode ); 
		final = 1; winloss = 1;
	}
	LastWinLoss = 0;
	if ( thinkingType == WINLOSS || thinkingType == PERFECT ) MaxThinkSec = 0;

	// Play in Opening Book?
	inBook = 0;

	int randombook = 0;
	if (CurrentAhead <= 8) randombook = 1;

	MoveX = 0;
	if ( thinkingType != WINLOSS && thinkingType != PERFECT )
		if (memcmp(StartBoard, StandardBoard, 65) == 0) // Only use the book on the normal startboard.
		{
			LastEval = PlayBook(CompColor, MoveX, MoveY, inBook, randombook);

			if ( LastEval != -5)
			{
				// Don't use book much for 1 or 2 ply
				if ( OpeningsType >= 3 && BoardStack[0].nummarkers > 12) inBook = 0;
				else if ( OpeningsType == 0 && (BoardStack[0].nummarkers > 12 || brainType == 2) && brainType != 0) inBook = 0;
				else if  (MaxAhead > 2 || BoardStack[0].nummarkers <= 8 || OpeningsType!=0) {bestmove = MoveX + 8*MoveY;}
				else inBook = 0;
			}
		}

	if (CurrentAhead + BoardStack[0].nummarkers > 64) 
	{
		CurrentAhead = 65-BoardStack[0].nummarkers;
		final = 1;
	}

	if (inBook == 0) // Computer must do an alpha beta Search
	{
		if (final == 0)  
		{
			// iterative deepening for midgame evaluation
			MaxAhead = CurrentAhead;
			if (MaxAhead > 6 ) 
				CurrentAhead = 6;
			else if (MaxAhead <= 4) 
				CurrentAhead = MaxAhead;
			else 
				CurrentAhead = MaxAhead - 2;

			bestmove = 127;
			TempBoard = BoardStack[0];

			if ( (TimeDependent == 1 || analyzeMode ) && !g_bSaveTraining) 
				MaxAhead = 28;

			while ( CurrentAhead <= MaxAhead && LastEval != TIMEOUT)
			{
				oldEval = LastEval;

				LastEval = AlphaBetaSearch( CompColor, 0, -399, 399, bestmove );

				CurrentAhead += incPly;
				if ( (int)(CurrentAhead+7) > (64-startMarkers) && (TimeDependent == 1 || analyzeMode ) && LastEval != TIMEOUT ) 
				{
					SetThinkTime( CompColor, MaxThinkSec, ExtraSecs, 1, analyzeMode ); 
					final = 1; 
					winloss = 1; 
					break;
				}
				if (TimeDependent && !analyzeMode && (clock() - g_start) > CLOCKS_PER_SEC * (MaxThinkSec * .68f) ) break;
			}

			CurrentAhead-=incPly;
			if (LastEval == TIMEOUT)
			{
				LastEval = oldEval;
				if (abs (SearchEval) < 10000) LastEval = SearchEval;
				if (checkNum == 1) CurrentAhead -=incPly;
			}
		}

		if (final == 1) // Search for final result
		{
			LastWinLoss = startMarkers;
			if (winloss == 1)  // Win/Loss/Draw
			{
				// pre search
				CurrentAhead = 8;
				final = 2;
				LastEval = AlphaBetaSearch( CompColor, 0, -1000, 1000,  bestmove );

				final = 1;
				TT_AgeInc();

				oldEval = -1001;

				// selective win/loss search
				SearchEval = -999999;
				EndGamePrune = 1;
				EndPruneRange = 35;
				CurrentAhead = 64 - BoardStack[0].nummarkers;
				LastEval =  AlphaBetaSearch(CompColor, 0, -6, 6,  bestmove);

				if (LastEval != TIMEOUT )
				{
					// more accurate selected win/loss search
					oldEval = LastEval; SearchEval = LastEval;
					EndPruneRange = 53;
					CurrentAhead = 65 - BoardStack[0].nummarkers;
					LastEval = AlphaBetaSearch( CompColor, 0, -6, 6,  bestmove );
					if (LastEval != TIMEOUT) {
						oldEval = LastEval; 
						SearchEval = LastEval; 
					}
					else 
					{
						if (abs(SearchEval) < 10) oldEval = SearchEval;
					}
				}

				if (BoardStack[0].nummarkers <= 40 && MaxThinkSec !=0) 
				{
					end = clock();
					if (BoardStack[0].nummarkers == 40)
					{
						if ((end - g_start) > (CLOCKS_PER_SEC * (MaxThinkSec - 9) )) 
							LastEval = TIMEOUT;
					}
					else LastEval = TIMEOUT; 
				}

				// win/loss search
				if (LastEval != TIMEOUT)
				{
					EndGamePrune = 0;
					CurrentAhead = 66 - BoardStack[0].nummarkers;
					LastEval = AlphaBetaSearch (CompColor, 0, -6, 6,  bestmove);
				}

				if ( analyzeMode )
				{
					winloss = 0;
					EndGamePrune = 0;
				}
				else if (TimeDependent == 1 && LastEval != TIMEOUT && thinkingType != WINLOSS && BoardStack[0].nummarkers >= 40) 
				{
					winloss = 0;
					EndGamePrune = 0;
				}
				else if (LastEval == TIMEOUT) {
					LastEval = oldEval; 
				}
							
				if (LastEval != -1001)
				if (TimeDependent == 1 || analyzeMode) oldEval = LastEval;
			}// end winloss

			// Perfect Endgame, using small search windows
			if (BoardStack[0].nummarkers < 50 && winloss == 0)
			{
				Scope = 6;

				CurrentAhead = 8;
				if (BoardStack[0].nummarkers >= 48) 
					CurrentAhead = 6;
				
				// Get a best move to start with from a midgame search
				final = 2;
				if (bestmove == 127) 
					LastEval = AlphaBetaSearch (CompColor, 0, -1000, 1000,  bestmove);

				final = 1;

				SearchEval = -999999;

				mid = 0;
				if ( (oldEval == -1000 && CompColor == WHITE) ||  (oldEval == 1000 && CompColor == BLACK) ) 
					{mid -=12; SearchEval = -6;}
				if ( (oldEval == 1000 && CompColor == WHITE) ||  (oldEval == -1000 && CompColor == BLACK) ) 
					{mid = 6; SearchEval = 6;}
						 					 
 			 	CurrentAhead = 66 - BoardStack[0].nummarkers;
				LastEval = AlphaBetaSearch (CompColor, 0, mid, mid+Scope,  bestmove);
							
				if (LastEval != TIMEOUT)
				{
					if (LastEval > mid ) // White Wins, find out by how much 
					{
						oldEval = LastEval - Scope;
						while (LastEval == oldEval + Scope && LastEval < 64 * 3) 
						{
							oldEval = LastEval; 
							SearchEval = oldEval;
							LastEval = AlphaBetaSearch (CompColor, 0, LastEval, LastEval + Scope,  bestmove);
						}
					}
					else if (LastEval <= mid) // Black Wins, find out by how much
					{
						oldEval = LastEval + Scope;
						while (LastEval == oldEval - Scope && LastEval > -64 * 3) 
						{
							oldEval = LastEval;
							SearchEval = oldEval;
							LastEval = AlphaBetaSearch (CompColor, 0, LastEval - Scope, LastEval,  bestmove);
						}
					}
				}

				if (LastEval == TIMEOUT) {LastEval = oldEval; question = 1;}
			}
			else if (winloss == 0) // Perfect Endgame
			{ 
				LastEval = AlphaBetaSearch (CompColor, 0, -200, 200,  bestmove);
			}
		}// end final
	} // end inbook

	end = clock();	
	DisplayNodes( 127, LastEval, inBook, end-g_start, question );

	return bestmove;
}