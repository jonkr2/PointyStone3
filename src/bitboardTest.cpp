#include "bitboard.h"
#include <assert.h>
#include <string>
#include <algorithm>
#include <time.h>
#define NOMINMAX
#include <windows.h>
#include "gui.h"

void DisplayText(const char *String);


namespace Othello
{
	// TODO : Make search stack for othello
	uint64_t g_nodeCount;

	BitBoard TouchingCorner;

	struct Move
	{
		Move() {}
		Move(int _sq, uint64_t _capSqBb) : sq(_sq), capSqBb(_capSqBb) {}
		uint64_t capSqBb;
		int sq;
	};

	struct MoveList
	{
		static constexpr int kMaxMoves = 24;
		Move moves[kMaxMoves];
		int scores[kMaxMoves];
		int count = 0;

		void Add(const int sq, const uint64_t capSqBb) 
		{
			assert(count < kMaxMoves);
			moves[count].sq = sq;
			moves[count++].capSqBb = capSqBb;
		}

		void SortByParity(const struct oBoard& board);
	};

	struct SearchStack
	{
		MoveList moves;
	};

	constexpr int MAX_PLY = 66;
	SearchStack searchStack[MAX_PLY];

	struct oBoard
	{
		// How would othello bitboard move gen work?
		BitBoard discs[2];
		int		 discCount;
		eColor	 stm;

		void DoMove( const Move& move )
		{
			// Generate flipBb from capDiscBb
			BitBoard capDiscs = move.capSqBb;
			BitBoard flipBb;
			while (capDiscs)
			{
				flipBb |= MaskBetweenSquares[move.sq][capDiscs.PopLowSq()];
			}

			// Flip the discs
			discs[WHITE] ^= flipBb;
			discs[BLACK] ^= flipBb;

			// Add the new disc
			discs[stm].AddSq( move.sq );

			// Change side to move
			stm = Opp(stm);
			discCount++;
		}

		void GenMoves( MoveList& moveList )
		{
			moveList.count = 0;

			// need to find empty squares next to opponent's discs
			const eColor o = Opp(stm);
			const BitBoard occ = discs[o] | discs[stm];
			const BitBoard stopperBb = ~discs[o];

			// for each empty square need to check if any of the 8 directions can flip pieces
			// might be better to keep bitboard of squares that touch any disc and use that to step through squares... or maybe some non bitboard struct
			BitBoard emptySqs = ~occ;
			while ( emptySqs )
			{
				int sq = emptySqs.PopLowSq();
				const BitBoard touchingBb = touchingSqs[sq];
				if (touchingBb & discs[o])
				{
					const BitBoard possibleFlips = RookMagics[sq].GetMoves(stopperBb) | BishopMagics[sq].GetMoves(stopperBb);
					const BitBoard capDiscs = possibleFlips & discs[stm] & ~touchingBb; // capDiscs are friendly discs that have opponent discs between them and sq
					if (capDiscs) {
						// We know there is a move possible. Exactly which discs it flipped will be computed later using capDiscs bitboard
						moveList.Add( sq, capDiscs.bb );
						// TODO : we can add a different magic bitboard type that maps capDiscs to flipBb. could ignore touching squares.. and do horizontal and diagonal separate
					}
				}
			}
		}

		// returns true if a move is legal
		bool IsMoveLegal( int sq )
		{
			if ((discs[WHITE] | discs[BLACK]).Includes(sq)) return false;
			const BitBoard stopperBb = ~discs[ Opp(stm) ];
			BitBoard possibleFlips = RookMagics[sq].GetMoves(stopperBb) | BishopMagics[sq].GetMoves(stopperBb);
			BitBoard capDiscs = possibleFlips & discs[stm] & ~touchingSqs[sq];
			return capDiscs.bb;
		}

		void StartBoard()
		{
			// place the 4 discs in the center
			discs[WHITE].bb = 0;
			discs[WHITE].AddSq(SQ(3, 3));
			discs[WHITE].AddSq(SQ(4, 4));
			discs[BLACK].bb = 0;
			discs[BLACK].AddSq(SQ(3, 4));
			discs[BLACK].AddSq(SQ(4, 3));
			discCount = 4;
			stm = BLACK;
		}

		// returns disc difference for stm
		int Score()
		{
			return 2 * discs[stm].BitCountFull() - discCount;
		}

		eColor GetDiscColor(int sq)
		{
			if (discs[WHITE].Includes(sq)) return WHITE;
			if (discs[BLACK].Includes(sq)) return BLACK;
			return NONE;
		}

		// Set from X-O string
		void SetFromText(const char *text)
		{
			// Add the discs
			int stringLen = (int)strlen(text);
			int maxBoardLen = std::min(stringLen, 64 );
			for (int i = 0; i < maxBoardLen; i++)
			{
				if ( text[i] == 'O' ) discs[WHITE].AddSq(i);
				if ( text[i] == 'X' ) discs[BLACK].AddSq(i);
			}

			// stm is on next line?
			stm = BLACK;
			if (stringLen > maxBoardLen && strstr(&text[maxBoardLen], "WHITE") ) { stm = WHITE; }

			discCount = (discs[WHITE] | discs[BLACK]).BitCountFull();
		}

		// Set from transcript of moves assuming normal start position
		void SetFromTranscript(const char *text)
		{
			StartBoard();
			int stringLen = (int)strlen(text);
			MoveList moveList;
			for (int i = 0; i < stringLen; i += 2)
			{
				GenMoves(moveList);
				int x = text[i] - 'a';
				int y = text[i+1] - '1';
				int sq = SQ(x, y);
				for (int m = 0; m < moveList.count; m++) {
					if (moveList.moves[m].sq == sq) {
						DoMove(moveList.moves[m]);
					}
				}
			}
		}
	};

	void MoveList::SortByParity(const Othello::oBoard& board)
	{
		const BitBoard empty = ~(board.discs[WHITE] | board.discs[BLACK]);
		for (int i = 0; i < count; i++) {
			const int sq = moves[i].sq;
			bool singleEmpty = (touchingSqs[sq] & empty).bb == 0;
			scores[i] = singleEmpty ? 100 : 0;
			scores[i] += BoardCorners.Includes(sq) ? 50 : 0;
			scores[i] += TouchingCorner.Includes(sq) ? -20 : 0;
		}

		// Now that they are scored, actually sort them
		// Insertion Sort
		const int startIdx = 0;
		for (int d = startIdx + 1; d < count; d++)
		{
			int i = d;
			int iVal = scores[i];
			Move tMove = moves[i];
			while (i > startIdx && iVal > scores[i - 1])
			{
				moves[i] = moves[i - 1];
				scores[i] = scores[i - 1];
				i--;
			}
			moves[i] = tMove;
			scores[i] = iVal;
		}
	}

	void PlayAnyMove( oBoard& board )
	{
		MoveList moves;
		board.GenMoves(moves);
		for (int i = 0; i < moves.count; i++)
		{
			// choose which move to play
		}
		if (moves.count > 1) {
			board.DoMove(moves.moves[1]);
		}
	}

	// Play all remaining moves until solved. Just an initial test of move gen..
	int Solve(oBoard& board, int ply, int alpha, int beta, bool previousMovePass = false )
	{
		MoveList& moves = searchStack[ply].moves;
		board.GenMoves(moves);

		// No moves left
		if (moves.count == 0)
		{
			// 2 passes means game is over
			if (previousMovePass) return -board.Score();

			// Otherwise pass then call solve again
			board.stm = Opp(board.stm);
			return -Solve(board, ply+1, -beta, -alpha, true);
		}

		// Do some move ordering... this is okay for near end but earlier need real move ordering
		if (board.discCount <= 60)
		{
			moves.SortByParity( board );
		}

		// TODO : need to add transposition table
		oBoard undoBoard = board;
		for (int i = 0; i < moves.count; i++)
		{
			const Move& move = moves.moves[i];
			board.DoMove(move);
			g_nodeCount++;

			int score = (board.discCount == 64) ? -board.Score() : -Solve(board, ply+1, -beta, -alpha, false);
			if (score > alpha)
			{
				alpha = score;
			}
			if (score >= beta)
			{
				return score;
			}

			board = undoBoard;
		}

		return alpha;
	}

	void DrawOthelloBoard( oBoard& board )
	{
		int bitmapIndices[64];
		for (int square = 0; square < 64; square++)
		{
			eColor discColor = board.GetDiscColor(square);
			if (discColor == WHITE) bitmapIndices[square] = WROOK;
			else if (discColor == BLACK) bitmapIndices[square] = BROOK;
			else bitmapIndices[square] = EMPTY;
		}
		DrawBoardGeneric(bitmapIndices);
	}

	void TestSolve()
	{
		// TODO : should make an othello init function
		TouchingCorner.bb = 0;
		for (int sq = 0; sq < 64; sq++) {
			if (touchingSqs[sq] & BoardCorners) { TouchingCorner.AddSq(sq); }
		}

		oBoard board;
		board.SetFromText("O--OOOOX-OOOOOOXOOXXOOOXOOXOOOXXOOOOOOXX---OOOOX-OOOO--XOOOXXO--");
		board.SetFromTranscript("c4e3f6e6f5c5f4g6f7g5g4e7d6f3f8h3g3d3h5h4h2h7d7c6b6b3b4a6b5c3c7a4a5d8c8e8e2d2f2e1b7f1g2a7");
		board.SetFromTranscript("c4e3f6e6f5c5f4g6f7g5g4e7d6f3f8h3g3d3h5h4h2h7d7c6b6b3b4a6b5c3c7a4a5d8c8e8e2d2f2e1");
		//PlayAnyMove(board);
		DrawOthelloBoard(board);

		clock_t startTime = clock();
		g_nodeCount = 0;

		int value = Solve(board, 0, -64, 64 );

		uint32_t tm = (uint32_t)((clock() - startTime) * 100.0f);
		float seconds = tm / 100000.0f;
		float nps = 0;
		if (tm > 0) { nps = float((double)g_nodeCount*100.0f / (double)tm) / 1000.0f; }

		char buffer[256];
		snprintf(buffer, sizeof(buffer), "Solved at %d\n%.2f seconds\n%d nodes\n%.2f MNps", value, seconds, g_nodeCount, nps);
		DisplayText( buffer );
	}
}

