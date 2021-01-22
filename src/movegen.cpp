//
// Pointy Stone Othello
// Functions for generating and performing moves
//
// -----------------
// Update Board lists affected by flipped pieces in other directions
// ----------------
// EMPTY to Black
#include <initializer_list>
#include <assert.h>

void inline Update4IndicesB2( Board &CBoard, int sq )
{  
	const uint32_t *src = &Boardindex[ (sq << 4) ];
	CBoard.numblack++;
	CBoard.south[ *(src ) ]         += (*(src + 1) << 1);
	CBoard.east[ *(src + 2) ]       += (*(src + 3) << 1);
	CBoard.southeast [ *(src + 4) ] += (*(src + 5) << 1);
	CBoard.southwest [ *(src + 6) ] += (*(src + 7) << 1);
}

// Flip Black or White
void inline Update4Indices( Board &CBoard, int sq, const int color )
{  
	const uint32_t *src = &Boardindex[ (sq << 4) ];
	if (color == WHITE)
	{
		CBoard.numblack--;
		CBoard.south[ *(src ) ]        -= *(src + 1);
		CBoard.east[ *(src + 2) ]      -= *(src + 3);
		CBoard.southeast[ *(src + 4) ] -= *(src + 5);
		CBoard.southwest[ *(src + 6) ] -= *(src + 7);
	}
	else
	{
		CBoard.numblack++;
		CBoard.south[ *(src ) ]        += *(src + 1);
		CBoard.east[ *(src + 2) ]      += *(src + 3);
		CBoard.southeast[ *(src + 4) ] += *(src + 5);
		CBoard.southwest[ *(src + 6) ] += *(src + 7); 
	}
}

// Update 5 indices (each slice + corner)
// EMPTY to Black
void inline UpdateIndicesB2( Board& board, int sq)
{  
	const uint32_t *src = &Boardindex[ (sq << 4) ];
	board.numblack++;
	board.south[ *(src ) ]        += (*(src + 1) << 1);
	board.east[ *(src + 2) ]      += (*(src + 3) << 1);
	board.southeast[ *(src + 4) ] += (*(src + 5) << 1);
	board.southwest[ *(src + 6) ] += (*(src + 7) << 1);
	board.corners[  *(src + 8) ]  += (*(src + 9) << 1);
	board.HashKey ^= *(src + 12);
	board.HashCheck ^= *(src + 13);
}

// Empty to White
void inline UpdateIndicesW2( Board& board, int sq)
{  
	const uint32_t *src = &Boardindex[ (sq << 4) ];
	board.south[ *(src ) ]        += *(src + 1);
	board.east[ *(src + 2) ]      += *(src + 3);
	board.southeast[ *(src + 4) ] += *(src + 5);
	board.southwest[ *(src + 6) ] += *(src + 7);
	board.corners[  *(src + 8) ]  += *(src + 9);
	board.HashKey ^= *(src + 10);
	board.HashCheck ^= *(src + 11);
	board.HashKey ^= *(src + 12);
	board.HashCheck ^= *(src + 13);
}

// Flip Stones
void inline UpdateIndices( Board &CBoard, int sq, const int color )
{  
	const uint32_t* src = &Boardindex[ (sq << 4) ];
	if (color == BLACK)
	{
		CBoard.numblack++;
		CBoard.south[ *(src ) ]        += *(src + 1);
		CBoard.east[ *(src + 2) ]      += *(src + 3);
		CBoard.southeast[ *(src + 4) ] += *(src + 5);
		CBoard.southwest[ *(src + 6) ] += *(src + 7); 
		CBoard.corners[  *(src + 8) ]  += *(src + 9);
	}
	else
	{
		CBoard.numblack--;
		CBoard.south[ *(src ) ]        -= *(src + 1);
		CBoard.east[ *(src + 2) ]      -= *(src + 3);
		CBoard.southeast[ *(src + 4) ] -= *(src + 5);
		CBoard.southwest[ *(src + 6) ] -= *(src + 7);
		CBoard.corners[ *(src + 8) ]   -= *(src + 9);
	}
	CBoard.HashKey ^= *(src + 10);
	CBoard.HashCheck ^= *(src + 11);
}

// ------------------------------------------------------
inline void FlipStones( Board &CBoard, const int delta, int tempsq, const int end, const int sq, const int color, const int bCorners )
{
	if (bCorners) {
		for (; tempsq< sq; tempsq+=delta) UpdateIndices (CBoard,tempsq, color);
		for (tempsq+=delta; tempsq<= end; tempsq+=delta) UpdateIndices(CBoard,tempsq, color);
		return;
	}
	for (; tempsq< sq; tempsq+=delta) Update4Indices (CBoard,tempsq, color);
	for (tempsq+=delta; tempsq<= end; tempsq+=delta) Update4Indices(CBoard,tempsq, color);
}

inline void FlipEast( Board& board, const Flipped &B, int sq, int mx, int my, const int color, const int bCorners)
{
	FlipStones(board,  1,  B.left - 9 + (my<<3),  B.right - 9 + (my<<3),  sq,  color, bCorners);
}

inline void FlipSouth( Board& board, const Flipped &B, int sq, int mx, int my, const int color, const int bCorners)
{
	FlipStones(board, 8, B.left*8 + mx - 9, B.right * 8 + mx - 9, sq, color, bCorners );
}

inline void FlipSoutheast( Board& board, const Flipped &B, int sq, int mx, int my, const int color, const int bCorners)
{                               
	int temp = mx - my + 7;
	FlipStones (board, 9, Boardindex2.southeast[ B.left + temp*8], Boardindex2.southeast[ B.right + temp*8], sq, color, bCorners);
}

inline void FlipSouthwest( Board& board, const Flipped &B, int sq, int mx, int my, const int color, const int bCorners)                 
{
	int temp = mx + my -2;
	FlipStones(board, 7, Boardindex2.southwest[ B.right + temp*8], Boardindex2.southwest[ B.left + temp*8],  sq, color, bCorners);
}

// Count discs for perfect endgame
// The 3* makes it correlate better with the midgame evals
int inline FinalEvalBoard (int nummarkers, int numblack)
{  
	int numwhite = nummarkers - numblack;
	assert(numwhite >= 0 && numwhite <= nummarkers);
	if ( numblack > numwhite) return 3*((numwhite<<1)-64);
	else if (numblack < numwhite) return 3*(64-(numblack<<1));
	else return 0;
}

//=================================
// Count pieces gained by a move to the empty square
// (for the final move in the endgame search)
//=================================
int PieceCount( Board &board, uint8_t colorToMove, uint8_t squares[] )
{
	int oldBlack = board.numblack;
	int numblack = board.numblack;
	nodes++;

	// Find the empty Square
	int mx = 0, my = 0;
	for (int x = 0; x < EMPTYEND; x++)
	{
		if (squares[x] != 66)
		{
			mx = (squares[x] & 7) + 1;
			my = (squares[x] >> 3) + 1;
			break;
		}
	}

	// Now count pieces for moving there
	for ( uint8_t color : { colorToMove, uint8_t(colorToMove ^ 3) } ) // the while is there to check opponent's move after a pass
	{
		if (color == WHITE) 
		{
			  numblack -= Wflipped[ board.east[my] ];
			  numblack -= Wflipped[ board.south[mx] ];
			  numblack -= Wflipped[ board.southeast[mx - my + 7] + Location[mx - my + 7] ];
			  numblack -= Wflipped[ board.southwest[mx + my - 2] + Location[mx + my - 2] ];

			  if (numblack != oldBlack)  
				  return FinalEvalBoard(board.nummarkers+1, numblack);
		}
		else 
		{
			  numblack += Bflipped[ board.east[my] ];
			  numblack += Bflipped[ board.south[mx] ];
			  numblack += Bflipped[ board.southeast[mx - my + 7] + Location[mx - my + 7]  ];
			  numblack += Bflipped[ board.southwest[mx + my - 2] + Location[mx + my - 2]  ];

			  if (numblack != oldBlack) 
				  return FinalEvalBoard(board.nummarkers+1, numblack+1);
		}
	}
	return FinalEvalBoard(board.nummarkers, board.numblack);  // two passes just return value
}

// ------------------------------
// Set Corner Threat variable for possible quiescence search (WHITE AddC==0, BLACK AddC==8)
// ------------------------------
void inline SetCornerThreatVar( Board &board, int sq, int8_t &cornerThreat, const int AddC )
{
	switch (EdgePlay[ sq ] )
	{
		case 1:
			if (Square_Moves[board.east[1] ].W[ 0 + AddC ].left ) cornerThreat = 0;
			if (Square_Moves[board.east[1] ].W[ 7 + AddC ].left ) cornerThreat = 7;
			break;
		case 2:
			if (Square_Moves[board.east[8] ].W[ 0 + AddC ].left ) cornerThreat = 56;
			if (Square_Moves[board.east[8] ].W[ 7 + AddC ].left ) cornerThreat = 63;
			break;
		case 3:
			if (Square_Moves[board.south[1] ].W[ 0 + AddC ].left ) cornerThreat = 0;
			if (Square_Moves[board.south[1] ].W[ 7 + AddC ].left ) cornerThreat = 56;
			break;
		case 4:
			if (Square_Moves[board.south[8] ].W[ 0 + AddC ].left ) cornerThreat = 7;
			if (Square_Moves[board.south[8] ].W[ 7 + AddC ].left ) cornerThreat = 63;
			break;
	}
}

void inline SetCornerThreat( Board &CBoard, int sq, const char color)
{
	CBoard.cornerThreat2 = CBoard.cornerThreat;
	CBoard.cornerThreat = 66;
	if (EdgePlay [sq] == 0 ) return;

	// Your move threatend a corner?
	SetCornerThreatVar( CBoard, sq, CBoard.cornerThreat, color == WHITE ? 0 : 8 );

	// You played next to a corner?
	if (CBoard.cornerThreat2 == 66 && CSquares[ sq ] !=0)	
		SetCornerThreatVar( CBoard, sq, CBoard.cornerThreat2, color == WHITE ? 8 : 0  );
}

void inline SetEdgeFlags( Board & board, const int sq)
{
	if ( !EdgeSwitch[ sq ] ) return;
	if (EdgeSwitch[ sq ] == 1) { board.edge1 = 1; board.edge3 = 1;}
	if (EdgeSwitch[ sq ] == 2) { board.edge1 = 1; board.edge4 = 1;}
	if (EdgeSwitch[ sq ] == 3) { board.edge2 = 1; board.edge3 = 1;}
	if (EdgeSwitch[ sq ] == 4) { board.edge2 = 1; board.edge4 = 1;}
}

// -------------------------------------------------------------------
// return TRUE if a move to sq is possible, and do the move
// unlike the usual DoMove function, this function doesn't require a movelist.
bool DoOneSquareMove( Board &board, int sq, char color)
{
	if (sq > 63 || sq < 0) return false;

	bool moved = false;
	Flipped B;
	int mx = (sq&7) + 1;
	int my = (sq>>3) + 1;

	if (color == BLACK)
	{
		// Find/Do Move (check the 4 directions)
		B = Square_Moves[ board.east[my] ].B[mx - 1];
		if ( B.left ) {
			FlipEast( board, B, sq, mx, my, color, 1); moved = true;
		}

		B = Square_Moves[ board.south[mx] ].B[my - 1];
		if ( B.left ) {
			FlipSouth( board, B, sq, mx, my, color, 1); moved = true;
		}

		B = Square_Moves[ board.southeast[mx - my + 7] ].B[ Boardindex3[ sq * 8 + 5 ] - 1];
		if ( B.left ) {
			FlipSoutheast( board, B, sq, mx, my, color, 1); moved = true;
		}

		B = Square_Moves[ board.southwest[mx + my - 2] ].B[ Boardindex3[ sq * 8 + 7 ] - 1];
		if ( B.left ) {
			FlipSouthwest (board, B, sq, mx, my, color, 1); moved = true;
		}
	}

	if (color == WHITE)
	{
		B = Square_Moves[ board.east[my] ].W[mx - 1];
		if ( B.left )  
			{FlipEast (board, B, sq, mx, my, color, 1); moved = true;}

		B = Square_Moves[ board.south[mx] ].W[my - 1];
		if ( B.left ) 
			{FlipSouth (board, B, sq, mx, my, color, 1); moved = true;}

		B = Square_Moves[ board.southeast[mx - my + 7] ].W[ Boardindex3[ sq * 8 + 5 ] - 1];
		if ( B.left )
			{FlipSoutheast (board, B, sq, mx, my, color, 1); moved = true;}

		B = Square_Moves[ board.southwest[mx + my - 2] ].W[ Boardindex3[ sq * 8 + 7 ] - 1];
		if ( B.left )
			{FlipSouthwest (board, B, sq, mx, my, color, 1); moved = true;}
	}

	if (moved)
	{
		if (color == WHITE) UpdateIndicesW2(board, sq); else UpdateIndicesB2(board, sq);
		SetCornerThreat( board, sq, color);
		SetEdgeFlags( board, sq);
		board.nummarkers++;
		board.color^= 3;
	}

	return moved;
}

// -----------------
// Do Moves from a list of empty squares
// this is used in the endgame
// -----------------
int SquareDoMoves( Board &board, int ahead, int alpha, int beta,char color, uint8_t squares[], int lastpass, int &bestmove)
{
	int sq, mx, my, eval, newbest = bestmove;
	bool moved = true;
	uint64_t oldnodes = nodes;
	Flipped B;

	ahead++;

	// If it's Blacks turn
	if (color == BLACK)
	{
		for (int i = 0; i < EMPTYEND; i++)
		  if (squares[i] != 66)
			{
			if (moved) board = BoardStack[ahead-1];
			moved = false;
			sq = squares[i];
			mx = (sq&7) + 1;
			my = (sq>>3)+ 1;

			// Find/Do Move (check the 4 directions)
			B = Square_Moves[board.east [my] ].B[mx - 1];
			if ( B.left )  
				{FlipEast(board, B, sq, mx, my, BLACK, 0); moved = true;}

			B = Square_Moves[board.south[mx] ].B[my - 1];
			if ( B.left ) 
				{FlipSouth(board, B, sq, mx, my, BLACK, 0); moved = true;}

			B = Square_Moves[board.southeast[mx - my + 7]  ].B[ Boardindex3 [ sq * 8 + 5 ] - 1];
			if ( B.left )
				{FlipSoutheast( board, B, sq, mx, my, BLACK, 0); moved = true;}

			B = Square_Moves[ board.southwest[mx + my - 2]  ].B[ Boardindex3 [ sq * 8 + 7 ] - 1];
			if ( B.left )
				{FlipSouthwest(board, B, sq, mx, my, BLACK, 0); moved = true;}

			// Get Evaluation of move
			if (moved)
			{
				nodes++;
				board.nummarkers++;
				Update4IndicesB2(board, sq);

				squares[i] = 66; // Square is now filled.
				if (board.nummarkers == 63) 
				{
					// Board 1 from filled, count pieces after move
					eval = PieceCount(board, WHITE, squares);
				} else {
					// Recursive lookahead
					eval = SquareDoMoves(BoardStack[ahead + 1], ahead, alpha, beta, WHITE, squares, 0, bestmove);
				}
				squares[i] = sq;

				// AB prune
				if (eval < beta)
				{
					if (eval <= alpha) { bestmove = sq; return alpha; }
					newbest = sq;
					beta = eval;
				}
			}
		}

		if (oldnodes == nodes) // no moves possible
		{
			if (lastpass == 0)
				beta = SquareDoMoves( BoardStack[ ahead +1], ahead, alpha, beta, WHITE, squares, 1, bestmove);
			else 
				return FinalEvalBoard( BoardStack[ahead-1].nummarkers, BoardStack[ahead-1].numblack);
		}
	
		bestmove = newbest;
		return beta;
	}

// -------- if it's white's turn do the same for white ---------
	for (int i = 0; i < EMPTYEND; i++)
	  if (squares[i] != 66)
		{
		if (moved) board = BoardStack[ahead-1];
		moved = false;
		sq = squares[i];
		mx = (sq&7) + 1;
		my = (sq>>3)+ 1;

		// Find/Do Move (check the 4 directions)
		B = Square_Moves[board.east [my] ].W[mx - 1];
		if ( B.left )  
			{FlipEast(board, B, sq, mx, my, WHITE, 0); moved = true;}

		B = Square_Moves[board.south[mx] ].W[my - 1];
		if ( B.left ) 
			{FlipSouth(board, B, sq, mx, my, WHITE, 0); moved = true;}

		B = Square_Moves[board.southeast[mx - my + 7]  ].W[ Boardindex3 [ sq * 8 + 5 ] - 1];
		if ( B.left )
			{FlipSoutheast(board, B, sq, mx, my, WHITE, 0); moved = true;}

		B = Square_Moves[board.southwest[mx + my - 2]  ].W[ Boardindex3 [ sq * 8 + 7 ] - 1];
		if ( B.left )
			{FlipSouthwest(board, B, sq, mx, my, WHITE, 0); moved = true;}

		// Get Evaluation of move
		if (moved)
		{
			nodes++;
			board.nummarkers++;
			board.numblack--;
			Update4Indices(board, sq, BLACK);
			
			squares[i] = 66; // Square is now filled.
			if (board.nummarkers == 63 ) // Board 1 from filled, count pieces after move
			{
				eval = PieceCount(board, BLACK, squares);
			} else {
				// Recursive lookahead
				eval = SquareDoMoves( BoardStack[ ahead +1], ahead, alpha, beta, BLACK, squares, 0, bestmove);
			}
			squares[i] = sq;

			// AB prune
			if (eval > alpha) 
			{
				if ( eval >= beta ) { bestmove = sq; return beta; }
				newbest = sq;
				alpha = eval;
			}
		}
	}
		
	if (oldnodes == nodes) // no moves possible
	{
		if (lastpass == 0)
			alpha = SquareDoMoves( BoardStack[ ahead +1], ahead, alpha, beta, BLACK, squares, 1, bestmove);
		else return FinalEvalBoard( BoardStack[ahead-1].nummarkers, BoardStack[ahead-1].numblack);
	}

	bestmove = newbest;
	return alpha;
}

// ------------------------------
// Do Move
// ------------------------------
int DoMove( Board &board, SingleMove &Move, char color )
{    
    int east = (Move.direction & 3) - 1;
	int south = ((Move.direction>>2) & 3) - 1;
    int southeast = ((Move.direction>>4) & 3) - 1;
	int southwest = ((Move.direction>>6) & 3) - 1;
	int mx = (Move.sq&7) + 1, my = (Move.sq>>3) + 1;

	SetEdgeFlags(board, Move.sq );
	board.nummarkers++; // 1 move = 1 more marker

	// Change the lookups of 4 directions of the board for pieces flipped
    if (color == WHITE)
	{
        if (east >= 0)	FlipEast(board, Slice_Moves2[board.east[my]].WF[east], Move.sq, mx, my, WHITE, 1);
        if (south >= 0) FlipSouth(board, Slice_Moves2[board.south[mx]].WF[south], Move.sq, mx, my, WHITE, 1 );
        if (southeast >= 0) FlipSoutheast(board, Slice_Moves2[board.southeast[mx - my + 7]].WF[southeast], Move.sq, mx, my, WHITE, 1);
        if (southwest >= 0) FlipSouthwest(board, Slice_Moves2[board.southwest [mx+my-2]].WF[southwest], Move.sq, mx, my, WHITE, 1);

		board.color = BLACK;
		UpdateIndicesW2(board, Move.sq);
		SetCornerThreat(board, Move.sq, color);
        return 1;
    }                 
	else // BLACK
	{
	    if (east >= 0) FlipEast(board, Slice_Moves2[board.east[my]].BF[east], Move.sq, mx, my, BLACK, 1);
        if (south >= 0) FlipSouth(board, Slice_Moves2[board.south[mx]].BF[south], Move.sq, mx, my, BLACK, 1 );
        if (southeast >= 0) FlipSoutheast(board, Slice_Moves2[board.southeast[mx - my + 7]].BF[southeast], Move.sq, mx, my, BLACK, 1);
		if (southwest >= 0) FlipSouthwest(board, Slice_Moves2[board.southwest [mx+my-2]].BF[southwest], Move.sq, mx, my, BLACK, 1);

		board.color = WHITE;
		UpdateIndicesB2(board, Move.sq);
		SetCornerThreat(board, Move.sq, color);
	}
	return 1;
}

//---------------------
// For better move ordering
//---------------------
void inline swapMoves( SingleMove *Moves, int Move1, int Move2)
{
	if (Move1 == Move2) return;

	MoveBoard[ Moves[Move1].sq ] = Move2;
	MoveBoard[ Moves[Move2].sq ] = Move1;
	SingleMove temp = Moves[ Move1 ];
	Moves[ Move1 ] = Moves[ Move2 ];
	Moves[ Move2 ] = temp;
}

void inline BestCornerFirst( SingleMove *Moves, unsigned int bestSquare )
{
	int swapper = 1;
	// best from hash
	if (bestSquare < 64 && MoveBoard[ bestSquare ] !=0)	{
		 swapMoves( Moves, swapper++, MoveBoard[ bestSquare ]);
		 MoveBoard[ bestSquare ] = 0;
	}
	// corners
	if (MoveBoard[0]  ) swapMoves( Moves, swapper++, MoveBoard[0]);
	if (MoveBoard[7]  ) swapMoves( Moves, swapper++, MoveBoard[7]);
	if (MoveBoard[56] ) swapMoves( Moves, swapper++, MoveBoard[56]);
	if (MoveBoard[63] ) swapMoves( Moves, swapper++, MoveBoard[63]);
}

//---------------------
// Add Move to movelist 
//---------------------
void inline AddMove( const int sq, uint8_t direction, SingleMove *Moves, int &total)
{
	if (MoveBoard[sq] )
		Moves[ MoveBoard[sq] ].direction |= direction;
	else
	{
		 MoveBoard[sq] = ++total;
		 Moves[total].sq = sq;
		 Moves[total].direction = direction;
	}
}

//=================================
// Find all the moves on board CBoard for color,
// and store them in movelist Moves
//=================================
int GenerateMoves( Board& board, char color, SingleMove *Moves, int &total, unsigned int bestsquare )
{
	unsigned char *LMoves;
	memset( MoveBoard, 0, 64 );
	total = 0;
	const int nAdd = (color == WHITE) ? 0 : 4;

	for (int x=1; x<=8; x++)
	{
		LMoves = Slice_Moves[board.east[x]].WMoves + nAdd;
		if (LMoves[0]) {
			AddMove(LMoves[0] + (x << 3) - 9, 1, Moves, total);
			if (LMoves[1]) {
				AddMove(LMoves[1] + (x << 3) - 9, 2, Moves, total);
				if (LMoves[2])  AddMove(LMoves[2] + (x << 3) - 9, 3, Moves, total);
			}
		}
					
		LMoves = Slice_Moves[board.south[x]].WMoves + nAdd;
		if (LMoves[0]) {
			AddMove( (LMoves[0] << 3) + x - 9, 4, Moves, total);
			if (LMoves[1]) {
				AddMove((LMoves[1] << 3) + x - 9, 8, Moves, total);
				if (LMoves[2])  AddMove((LMoves[2] << 3) + x - 9, 12, Moves, total);
			}
		}
	}
	         
	for (int x=2; x<=12; x++)
	{ 
		int temp = Location[x];  // Diagonals must check proper slice length
		LMoves = Slice_Moves[ board.southeast[x] + temp].WMoves + nAdd;
		if ( LMoves[0] ) {AddMove( Boardindex2.southeast[LMoves[0] + (x<<3) ], 16, Moves, total);
			if ( LMoves[1] ) {AddMove( Boardindex2.southeast[LMoves[1] + (x<<3) ], 32, Moves, total);
				if ( (LMoves[2])  ) AddMove( Boardindex2.southeast[LMoves[2] + (x<<3) ], 48, Moves, total); 
			}
		}
		 
		LMoves = Slice_Moves[ board.southwest[x] + temp].WMoves + nAdd;
		if ( LMoves[0] ) {AddMove( Boardindex2.southwest[LMoves[0] + (x<<3) ], 64, Moves, total);
			if ( LMoves[1] ) {AddMove( Boardindex2.southwest[LMoves[1] + (x<<3) ], 128, Moves, total);
				if ( (LMoves[2])  )  AddMove( Boardindex2.southwest[LMoves[2] + (x<<3) ], 192, Moves, total); 
			}
		}
	}

	// re-arrange moves to put best square first (then corners)
	int retVal = (bestsquare < 64 && MoveBoard[bestsquare] == 0) ? 0 : 1;
	BestCornerFirst( Moves, bestsquare );

	return retVal;
}

//=================================
// EndGame Move Generation & Search Functions
//=================================
int SB[100];
// Store the numbers of the empty squares in a list
void FindEmptySquares( Board &board, uint8_t squares[], int bestmove )
{
	int y, index = 0, mx, i, sq, sorted = 0;
	unsigned short slice[9];
	static int sqV = 0;
	sqV++;

	memset(squares, INVALID_SQ, EMPTYEND);

	// Find Empty Squares
	for (y = 1; y <= 8; y++)
		slice[y] = board.east[y];

	// helps move ordering slightly
	if (FirstOne[slice[1]] == 1) {squares[ index++ ] = 0; slice[1] += Power3[1];}
	if (FirstOne[slice[1]] == 8) {squares[ index++ ] = 7; slice[1] += Power3[8];}
	if (FirstOne[slice[8]] == 1) {squares[ index++ ] = 56; slice[8] += Power3[1];}
	if (FirstOne[slice[8]] == 8) {squares[ index++ ] = 63; slice[8] += Power3[8];}
		
	// find the empties
	for (y = 1; y <= 8; y++)
	{
		while ( FirstOne[ slice[y] ] ) 
		{
			mx = FirstOne[ slice[y] ];
 			squares[ index++ ] = (mx + (y <<3) -9); 
			SB[ mx + y  * 10 ] = sqV; // how could this be right?
			slice[y] += Power3[ mx ]; // fill it up, Joe
		}
	}

	// Move ordering
	for (i = 0; i < EMPTYEND; i++)
	{
		if (squares[i] == bestmove)
		{
			y = squares[0];
			squares[0] = squares[i];
			squares[i] = y;
			sorted++;
		}
	}

	// parity (order lone empty squares first)
	for (i = sorted; i < EMPTYEND; i++)
	{
		sq = (squares[i] >> 3) * 10 + (squares[i] & 7) + 11;
		if ( (SB[ sq + 1 ] != sqV && SB[ sq - 1 ] != sqV && SB[ sq + 10 ] != sqV && SB[ sq - 10 ] != sqV  &&
			SB[ sq + 9 ] != sqV && SB[ sq - 9 ] != sqV && SB[ sq + 11 ] != sqV && SB[ sq - 11 ] != sqV) )
		{
			y = squares[ sorted ];
			squares[ sorted ] = squares[i];
			squares[i] = y;
			sorted++;
		}
	}
}