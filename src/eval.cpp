//-------------------
// Evaluation by non-Pointy Stone brains
//-------------------
int GreedyEvaluateBoard (Board &CBoard)
{
	int eval = Mobility [ CBoard.east[2] ] + Mobility[ CBoard.south[2] ] + Mobility [ CBoard.east[3] ] + Mobility[ CBoard.south[3] ]
			+ Mobility [ CBoard.east[4] ] + Mobility[ CBoard.south[4] ] + Mobility [ CBoard.east[5] ] + Mobility[ CBoard.south[5] ]
			+ Mobility [ CBoard.east[6] ] + Mobility[ CBoard.south[6] ] + Mobility [ CBoard.east[7] ] + Mobility[ CBoard.south[7] ]
			+ EdgeV [ CBoard.south[1] ] + EdgeV [ CBoard.south[8] ] + EdgeV [ CBoard.east[1] ] + EdgeV [ CBoard.east[8] ]
			+ DiagV [ CBoard.southeast[7] ] + DiagV [ CBoard.southwest[7] ];

	return eval;
}

int RandomEvaluateBoard (Board &CBoard)
{
	int eval = RandomV[ CBoard.east[2] ] + RandomV[ CBoard.east[3] ] + RandomV[ CBoard.east[4] ] + RandomV[ CBoard.east[5] ]
			+ RandomV[ CBoard.east[6] ] + RandomV[ CBoard.east[7] ]
			+ EdgeV[ CBoard.south[1] ] + EdgeV[ CBoard.south[8] ] + EdgeV[ CBoard.east[1] ] + EdgeV[ CBoard.east[8] ];

	eval += (rand () & 31 ) - 15;

	return eval;
}

//
// For Gradient Descent Learning
//
void SaveTrainingPosition( Board &CBoard, int &eval )
{
	eval += (rand()%128 - 64);
	int nRand = (rand () % 2800);
	if (nRand <= 11 && CBoard.nummarkers >= 12 && CBoard.nummarkers <= 24) SavePosition( CBoard, "newposB.pnt", 128-65 );
	if (nRand <=3 && CBoard.nummarkers >= 24 && CBoard.nummarkers < 36) SavePosition( CBoard, "newposMB.pnt", 128-65 );
	if (nRand <=4 && CBoard.nummarkers >= 36 && CBoard.nummarkers < 48) SavePosition( CBoard, "newposME.pnt", 128-65 );
	if (nRand <=11 && CBoard.nummarkers >= 48) SavePosition( CBoard, "newposE.pnt", 128-65 );
}

// Evaluate Board using char coefficents
int EvaluateBoard( Board &board )
{
	// Consider Eliminations
    if (board.numblack == 0) return 400;
    if (board.numblack == board.nummarkers) return -400;

	if (brainType != 0 && final == 0) 
	{
		if (brainType == 1) return GreedyEvaluateBoard(board);
		if (brainType == 2) return RandomEvaluateBoard(board);
	}

	// Set the coeffs to the gamestage
	CCoeffs = &Coeffs[ gameStage[board.nummarkers] ];
	int eval = 0;
	int eval2 = 0;
	int sub = 0;

	// Overall Parity
	if ((board.color == WHITE && (board.nummarkers & 1) == 0 ) || (board.color == BLACK && (board.nummarkers & 1) != 0))
              eval-=int(CCoeffs->parity * 16); else eval+=int(CCoeffs->parity * 16);

	// Side to move
	if (board.color == WHITE)
        eval+=int(CCoeffs->sideTM * 16);  else eval-=int(CCoeffs->sideTM * 16);

	// Slices
	eval += (CCoeffs->Slice8CF1[ board.east[3]  ] + CCoeffs->Slice8DE1[ board.east[4] ]
		   + CCoeffs->Slice8DE1[ board.east[5]  ] + CCoeffs->Slice8CF1[ board.east[6] ]
		   + CCoeffs->Slice8CF1[ board.south[3] ] + CCoeffs->Slice8DE1[ board.south[4] ]
		   + CCoeffs->Slice8DE1[ board.south[5] ] + CCoeffs->Slice8CF1[ board.south[6] ]);

	eval += ( CCoeffs->Diag7 [board.southeast[6] ] + CCoeffs->Diag7 [board.southwest[6] ] + CCoeffs->Diag7 [board.southeast[8] ] + CCoeffs->Diag7 [board.southwest[8] ]
		   +  CCoeffs->Diag6 [board.southeast[5] ] + CCoeffs->Diag6 [board.southwest[5] ] + CCoeffs->Diag6 [board.southeast[9] ] + CCoeffs->Diag6 [board.southwest[9] ]
		   +  CCoeffs->Diag5 [board.southeast[4] ] + CCoeffs->Diag5 [board.southwest[4] ] + CCoeffs->Diag5 [board.southeast[10] ] + CCoeffs->Diag5 [board.southwest[10] ] );
		   
	int d8se = board.southeast[7];
	int d8sw = board.southwest[7];
	int x1 = DiagI[ d8se ].XSquare1; 
	int x3 = DiagI[ d8se ].XSquare2; 
	int x2 = DiagI[ d8sw ].XSquare1; 
	int x4 = DiagI[ d8sw ].XSquare2;

	// Long Diagonals
	eval += CCoeffs->Diag8[ d8se ] + CCoeffs->Diag8[ d8sw ]; 

	// Corners
	eval2 += CCoeffs->Corner2[ board.corners[0] ] + CCoeffs->Corner2[ board.corners[1] ]
		  +  CCoeffs->Corner2[ board.corners[2] ] + CCoeffs->Corner2[ board.corners[3] ];

	//Edges & corner 2x5s
	if (board.edge1 == 0)
		eval2 += CCoeffs->EdgeM2[ Middle6[board.east[1] ] + Middle4[ board.east[2]] ];
	else 
	{
		eval2 += CCoeffs->EdgeX2 [board.east[1] + x1 + x4 * 3]
				+ CCoeffs->Corner25[ First5[board.east[1] ] + First5[board.east[2] ] * 243 ]
				+ CCoeffs->Corner25[ Last5[board.east[1] ]  + Last5[board.east[2] ]  * 243 ];
		sub += 64;
	}
	if (board.edge2 == 0)
		eval2+= CCoeffs->EdgeM2 [ Middle6[board.east[8] ] + Middle4 [board.east[7]] ];
	else {
		eval2+= CCoeffs->EdgeX2 [board.east[8] + x2  + x3 * 3]
			+ CCoeffs->Corner25[ First5[board.east[8] ] + First5[board.east[7] ] * 243 ]
			+ CCoeffs->Corner25[ Last5[board.east[8] ]  + Last5[board.east[7] ]  * 243 ];
		sub +=64;
	}
	if (board.edge3 == 0)
		eval2+= CCoeffs->EdgeM2 [ Middle6[board.south[1] ] + Middle4 [board.south[2]] ];
	else 
	{
		eval2+= CCoeffs->EdgeX2[ board.south[1] + x1 + x2 * 3 ]
				+ CCoeffs->Corner25[ First5[board.south[1] ] + First5[board.south[2] ] * 243 ]
				+ CCoeffs->Corner25[ Last5[board.south[1] ]  + Last5[board.south[2] ] * 243  ];
		sub +=64;
	}
	if (board.edge4 == 0)
		eval2+= CCoeffs->EdgeM2[ Middle6[ board.south[8] ] + Middle4[ board.south[7] ] ];
	else {
		eval2+= CCoeffs->EdgeX2 [board.south[8] + x4 + x3 * 3]
			+ CCoeffs->Corner25[ Last5[board.south[8] ]  + Last5[board.south[7] ] * 243 ]
			+ CCoeffs->Corner25[ First5[board.south[8] ] + First5[board.south[7] ] * 243 ];
		sub +=64;
	}

	// sum the eval (Different patterns have different resolutions, 1/16 for normal slices, 1/4 for edge and corner)
	eval = ((eval + 8) / 16) - 176  
		 + ((eval2 + 2) / 4) - 256 - sub; 

	if (g_bSaveTraining) SaveTrainingPosition( board, eval );

    return eval;
}

// -------------------
// Allocate memory for all the coefficents, then Load them
// -------------------
int LoadAllCharCoeff( char *filename )
{
	int total;
	uint8_t *coeffs;
	uint8_t *buffer;
	// FILE *FP = fopen (filename, "rb");
	// if (FP == NULL) return 0;

	total = 59100 * 3 + 19684 + 6562 * 3 + 2200 + 730 + 245 + 85 + 4 + 4;
	coeffs = (unsigned char *)calloc( 1, total * 8 + 16 );

	buffer = coeffs;
	/*fread (buffer, total * 4, 1, FP);*/

	if (uncompressFileFromArchive( "Pattern.jef", "newcoeff.dat",  buffer ) != 1) 
		MessageBox( MainWnd, "Error Loading Pattern Weights from Pattern.jef. Pointy will not play properly.", "Error", MB_OK );

	for (int i = 0; i < 7; i++)
	{
		Coeffs[i].EdgeX2   = buffer;  buffer += 59100;
		Coeffs[i].EdgeM2   = buffer;  buffer += 59100;
		Coeffs[i].Corner2  = buffer;  buffer += 19684;
		Coeffs[i].Corner25 = buffer;  buffer += 59100;
		Coeffs[i].Slice8CF1 = buffer; buffer += 6562;
		Coeffs[i].Slice8DE1 = buffer; buffer += 6562;
		Coeffs[i].Diag8 = buffer;     buffer += 6562;
		Coeffs[i].Diag7 = buffer;     buffer += 2200;
		Coeffs[i].Diag6 = buffer;     buffer += 730;
		Coeffs[i].Diag5 = buffer;     buffer += 245;
		Coeffs[i].Diag4 = buffer;     buffer += 85;
		memcpy (&Coeffs[i].parity, buffer, 4); buffer += 4;
		memcpy (&Coeffs[i].sideTM, buffer, 4); buffer += 4;
	}

	//fclose (FP);
	return 1;
}

// -------------------
int SaveAllCharCoeff (char *filename)
{
	FILE *FP = fopen (filename, "wb");

	for (int i = 0; i < 4; i++)
	{
		fwrite( Coeffs[i].EdgeX2, 59100, 1, FP);
		fwrite( Coeffs[i].EdgeM2, 59100, 1, FP);
		fwrite( Coeffs[i].Corner2, 19684, 1, FP);
		fwrite( Coeffs[i].Corner25, 59100, 1, FP);
 
		fwrite( Coeffs[i].Slice8CF1, 6562, 1, FP);
		fwrite( Coeffs[i].Slice8DE1, 6562, 1, FP);
		fwrite( Coeffs[i].Diag8    , 6562, 1, FP);
		fwrite( Coeffs[i].Diag7    , 2200, 1, FP);
		fwrite( Coeffs[i].Diag6    , 730, 1, FP);
		fwrite( Coeffs[i].Diag5    , 245, 1, FP);
		fwrite( Coeffs[i].Diag4    , 85, 1, FP);
		fwrite( &Coeffs[i].parity  , 4, 1, FP);
		fwrite( &Coeffs[i].sideTM  , 4, 1, FP);
	}

	fclose(FP);
	return 1;
}

// Blend between two stages (just take the average)
inline uint8_t BlendOneCo(uint8_t value1, uint8_t value2)
{
	int i;
	i = ((value1 + value2) >> 1);
	return (uint8_t)i;
}

void BlendCoeff( int stageout, int stage1, int stage2 )
{
	int i;
	CCoeffs = &Coeffs[ stageout ];

	for (i = 0; i < 6562 ; i++)	 CCoeffs->Slice8DE1[ i ] = BlendOneCo( Coeffs[stage1].Slice8DE1[ i ], Coeffs[stage2].Slice8DE1[i] );
	for (i = 0; i < 6562 ; i++)	 CCoeffs->Slice8CF1[ i ] = BlendOneCo( Coeffs[stage1].Slice8CF1[ i ], Coeffs[stage2].Slice8CF1[i] );
	for (i = 0; i < 6562 ; i++)	 CCoeffs->Diag8[ i ]     = BlendOneCo( Coeffs[stage1].Diag8[ i ],     Coeffs[stage2].Diag8[i] );
	for (i = 0; i < 2188 ; i++)	 CCoeffs->Diag7[ i ]     = BlendOneCo( Coeffs[stage1].Diag7[ i ],     Coeffs[stage2].Diag7[i] );
	for (i = 0; i < 729 ; i++)	 CCoeffs->Diag6[ i ]     = BlendOneCo( Coeffs[stage1].Diag6[ i ],     Coeffs[stage2].Diag6[i] );
	for (i = 0; i < 243 ; i++)	 CCoeffs->Diag5[ i ]     = BlendOneCo( Coeffs[stage1].Diag5[ i ],     Coeffs[stage2].Diag5[i] );

	for (i = 0; i < 59050 ; i++)  CCoeffs->Corner25[ i ] = BlendOneCo( Coeffs[stage1].Corner25[ i ], Coeffs[stage2].Corner25[i] );
	for (i = 0; i < 19684 ; i++)  CCoeffs->Corner2 [ i ] = BlendOneCo( Coeffs[stage1].Corner2 [ i ], Coeffs[stage2].Corner2[i]  );
	for (i = 0; i < 59050 ; i++)  CCoeffs->EdgeX2  [ i ] = BlendOneCo( Coeffs[stage1].EdgeX2  [ i ], Coeffs[stage2].EdgeX2[i]   );
	for (i = 0; i < 59050 ; i++)  CCoeffs->EdgeM2  [ i ] = BlendOneCo( Coeffs[stage1].EdgeM2  [ i ], Coeffs[stage2].EdgeM2[i]   );

	CCoeffs->parity = (Coeffs[stage1].parity + Coeffs[stage2].parity) * 0.5f;
	CCoeffs->sideTM = (Coeffs[stage1].sideTM + Coeffs[stage2].sideTM) * 0.5f;
}