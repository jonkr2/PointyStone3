// ------------------------------
// learn.cpp 
// Pattern Learning
// for Pointy Stone 3
// 
// by Jonathan Kreuzer
// ------------------------------
struct Learn
{
	int total;
	int game;
	short location;
	double discs, moves, Bmoves;
};

struct ErrorVal
{
	void resetError() 
	{
		error = 0.0;
		total = 6;
	}
	float value;
	double error;
	int total;
};

struct Mirror
{
	int32_t value1, value2, value3;
};

ErrorVal* ECorner = nullptr;
ErrorVal* ECorner25 = nullptr;
ErrorVal* EEdgeX = nullptr;
ErrorVal* EEdgeMid = nullptr;
ErrorVal* EDiag8 = nullptr;
ErrorVal* EDiag7 = nullptr;
ErrorVal* EDiag6 = nullptr;
ErrorVal* EDiag5 = nullptr;
ErrorVal* ESlice8DE = nullptr;
ErrorVal* ESlice8CF = nullptr;
ErrorVal parity, sideTM;

Mirror* MirCorner = nullptr;
Mirror* MirEdgeX = nullptr;
Mirror* MirEdgeM = nullptr;
Mirror* MirSlice = nullptr;
Mirror* MirSliceL[8];
int32_t* Mir25Corner = nullptr;

int addAll = 1;
int GetValues( char transcript[], char tBoard[], char cornersM[], char cornersS[], char edgeXM[], char edgeXS[], char corner25M[], char corner25S[], char edgeM[], char sliceDE[], char sliceCF[], char sliceD8[], char sliceD7[], char sliceD6[], char sliceD5[]);

// .CODE

void AllocateLearningData( )
{
	if (ECorner25 != NULL) return;

	ECorner25 = (ErrorVal*) calloc ( sizeof(ErrorVal), 59050 * NUM_STAGES);
	ECorner   = (ErrorVal*) calloc ( sizeof(ErrorVal), 19684 * NUM_STAGES);
	EEdgeX    = (ErrorVal*) calloc ( sizeof(ErrorVal), 59050 * NUM_STAGES);
	EEdgeMid  = (ErrorVal*) calloc ( sizeof(ErrorVal), 59050 * NUM_STAGES);
	ESlice8DE = (ErrorVal*) calloc ( sizeof(ErrorVal), 6562 * NUM_STAGES);
	ESlice8CF = (ErrorVal*) calloc ( sizeof(ErrorVal), 6562 * NUM_STAGES);
	EDiag8    = (ErrorVal*) calloc ( sizeof(ErrorVal), 6562 * NUM_STAGES);
	EDiag7    = (ErrorVal*) calloc ( sizeof(ErrorVal), 2188 * NUM_STAGES);
	EDiag6    = (ErrorVal*) calloc ( sizeof(ErrorVal), 729 * NUM_STAGES);
	EDiag5    = (ErrorVal*) calloc ( sizeof(ErrorVal), 243 * NUM_STAGES);

	MirCorner = (Mirror*) calloc ( sizeof(Mirror), 19684 );
	MirEdgeX  = (Mirror*) calloc ( sizeof(Mirror), 59050 );
	MirEdgeM  = (Mirror*) calloc ( sizeof(Mirror), 59050 );
	MirSlice  = (Mirror*) calloc ( sizeof(Mirror), 6562 );
	Mir25Corner = (int32_t *) calloc ( sizeof(int32_t), 59050 );

	for (int i = 1; i < 8; i++)
		MirSliceL[i] = (Mirror*) calloc ( sizeof(Mirror) , Power3[ i + 1] + 2 );
}


// -----------------------
// MIRROR FUNCTIONS
//
// mirrors a slice by line of symmetry, and also by color
// -----------------------

// ----------------
// 3x3 Corner
// ----------------
char MirrorC[9] = {0, 3, 6, 1, 4, 7, 2, 5, 8};

// 0 1 2
// 3 4 5
// 6 7 8

void MirrorCorner ( int slice, int &value1, int &value2, int &value3 )
{
	if (slice == 0)
	{
		value1 = 0;
		value2 = 0;
		value3 = 0;
		return;
	}

	// Lookup
	if (MirCorner [slice].value1 !=0)
	{
		value1 = MirCorner [slice].value1;
		value2 = MirCorner [slice].value2;
		value3 = MirCorner [slice].value3;
		return;
	}

	// If no lookup, calculate the mirror
	static int C[9];
	int i;

	for ( i = 0; i < 9; i++)
		C[i] = MarkerColor (slice, i+1);

	value1 = 0;
	for ( i = 0; i < 9; i++)
		value1 += C[ MirrorC[i] ] * Power3[i + 1];

	for ( i = 0; i < 9; i++)
		{if (C[i] == BLACK) C[i] = WHITE;
		 else if (C[i] == WHITE) C[i] = BLACK;
		}

	value2 = 0;
	for ( i = 0; i < 9; i++)
		value2 += C[ i ] * Power3[i + 1];

	value3 = 0;
	for ( i = 0; i < 9; i++)
		value3 += C[  MirrorC[i] ] * Power3[i + 1];

	// Store lookup for later
	MirCorner [slice].value1 = value1;
	MirCorner [slice].value2 = value2;
	MirCorner [slice].value3 = value3;
}

// ----------------
// 2x5 Corner has no line of symmetry. It occurs in 8 places on the board though.
// ----------------
void Mirror25Corner ( int slice, int &value1 )
{
	if (slice == 0)
	{
		value1 = 0;
		return;
	}

	// Lookup
	if (Mir25Corner [slice] !=0)
	{
		value1 = Mir25Corner [slice];
		return;
	}
		
	// If no lookup, calculate the mirror
	static int C[10];
	int i;

	for ( i = 0; i < 10; i++)
		C[i] = MarkerColor (slice, i+1);

	for ( i = 0; i < 10; i++)
	{
		if (C[i] == BLACK) C[i] = WHITE;
		else if (C[i] == WHITE) C[i] = BLACK;
	}

	value1 = 0;
	for ( i = 0; i < 10; i++)
		value1 += C[ i ] * Power3[i + 1];

	// Store lookup for later
	Mir25Corner[slice] = value1;
}

// -----------------------
//    0, 1, 2, 3, 4 , 5
//       6, 7, 8, 9

char MirrorM[10] = {5,4,3,2,1,0,9,8,7,6};

void MirrorEdgeMid( int slice, int &value1, int &value2, int &value3 )
{
	if (slice == 0)
	{
		value1 = 0;
		value2 = 0;
		value3 = 0;
		return;
	}
	// Lookup
	if (MirEdgeM [slice].value1 !=0)
	{
		value1 = MirEdgeM [slice].value1;
		value2 = MirEdgeM [slice].value2;
		value3 = MirEdgeM [slice].value3;
		return;
	}

	static int C[10];
	int i;

	for ( i = 0; i < 10; i++)
		C[i] = MarkerColor (slice, i+1);

	value1 = 0;
	for ( i = 0; i < 10; i++)
		value1 += C[ MirrorM[i] ] * Power3[i + 1];

	for ( i = 0; i < 10; i++)
	{
		if (C[i] == BLACK) C[i] = WHITE;
		else if (C[i] == WHITE) C[i] = BLACK;
	}

	value2 = 0;
	for ( i = 0; i < 10; i++)
		value2 += C[ i ] * Power3[i + 1];

	value3 = 0;
	for ( i = 0; i < 10; i++)
		value3 += C[  MirrorM[i] ] * Power3[i + 1];

	// Store lookup for later
	MirEdgeM[slice].value1 = value1;
	MirEdgeM[slice].value2 = value2;
	MirEdgeM[slice].value3 = value3;

	return;
}

// ----------------
// Edge + 2X
// -----------------
// 0, 1, 2, 3, 4, 5, 6, 7
//    8              9
char MirrorE[10] = {7, 6, 5, 4, 3, 2, 1, 0, 9, 8};

void MirrorEdgeX( int slice, int &value1, int &value2, int &value3 )
{
	if (slice == 0)
	{
		value1 = 0;
		value2 = 0;
		value3 = 0;
		return;
	}
	// Lookup
	if (MirEdgeX[slice].value1 !=0)
	{
		value1 = MirEdgeX[slice].value1;
		value2 = MirEdgeX[slice].value2;
		value3 = MirEdgeX[slice].value3;
		return;
	}

	static int C[10];
	int i;

	for ( i = 0; i < 10; i++)
		C[i] = MarkerColor(slice, i+1);

	value1 = 0;
	for ( i = 0; i < 10; i++)
		value1 += C[ MirrorE[i] ] * Power3[i + 1];

	for ( i = 0; i < 10; i++)
	{
		if (C[i] == BLACK) C[i] = WHITE;
		else if (C[i] == WHITE) C[i] = BLACK;
	}

	value2 = 0;
	for ( i = 0; i < 10; i++)
		value2 += C[ i ] * Power3[i + 1];

	value3 = 0;
	for ( i = 0; i < 10; i++)
		value3 += C[ MirrorE[i] ] * Power3[i + 1];

	// Store lookup for later
	MirEdgeX[slice].value1 = value1;
	MirEdgeX[slice].value2 = value2;
	MirEdgeX[slice].value3 = value3;

	return;
}

// --------------
void MirrorSlice( int slice, int &value1, int &value2, int &value3 )
{
	static int retSlice;
	static int C[10];

	if (slice == 0)
	{
		value1 = 0;
		value2 = 0;
		value3 = 0;
		return;
	}

	// Lookup
	if (MirSlice [slice].value1 !=0)
	{
		value1 = MirSlice[slice].value1;
		value2 = MirSlice[slice].value2;
		value3 = MirSlice[slice].value3;
		return;
	}

	int i;
	for ( i = 0; i < 8; i++)
		C[i] = MarkerColor(slice, i+1);

	value1 = 0;
	for ( i = 0; i < 8; i++)
		value1 += C[ MirrorE[i] ] * Power3[i + 1];

	for ( i = 0; i < 8; i++)
	{
		if (C[i] == BLACK) C[i] = WHITE;
		else if (C[i] == WHITE) C[i] = BLACK;
	}

	value2 = 0;
	for ( i = 0; i < 8; i++)
		value2 += C[ i ] * Power3[i + 1];

	value3 = 0;
	for ( i = 0; i < 8; i++)
		value3 += C[  MirrorE[i] ] * Power3[i + 1];

	// Store Lookup for Later
	MirSlice[slice].value1 = value1;
	MirSlice[slice].value2 = value2;
	MirSlice[slice].value3 = value3;
	return;
}

// --------------

void MirrorSliceL( int sliceLength, int slice, int &value1, int &value2, int &value3 )
{
	static int retSlice;
	static int C[10];

	// Lookup
	if (MirSlice[slice].value1 !=0)
	{
		value1 = MirSliceL[sliceLength][slice].value1;
		value2 = MirSliceL[sliceLength][slice].value2;
		value3 = MirSliceL[sliceLength][slice].value3;
		return;
	}

	int i;
	for ( i = 0; i < sliceLength; i++)
		C[i] = MarkerColor(slice, i+1);

	value1 = 0;
	for ( i = 0; i < sliceLength; i++)
		value1 += C[ sliceLength - i - 1 ] * Power3[i + 1];

	for ( i = 0; i < sliceLength; i++)
	{
		if (C[i] == BLACK) C[i] = WHITE;
		else if (C[i] == WHITE) C[i] = BLACK;
	}

	value2 = 0;
	for ( i = 0; i < sliceLength; i++)
		value2 += C[ i ] * Power3[i + 1];

	value3 = 0;
	for ( i = 0; i < sliceLength; i++)
		value3 += C[ sliceLength - i - 1 ] * Power3[i + 1];

	// Store Lookup for Later
	MirSliceL[sliceLength][slice].value1 = value1;
	MirSliceL[sliceLength][slice].value2 = value2;
	MirSliceL[sliceLength][slice].value3 = value3;
	return;
}

// ===================================================
// Gradient Descent Functions
// ===================================================
// Set Initial Values for Error calculation
void SetErrorValues( )
{
	int i;

	for (i = 0; i < 59050 ; i++) ECorner25[i].value = (float)(CCoeffs->Corner25 [ i ] - 128) / 4.0f;
	for (i = 0; i < 19684 ; i++) ECorner[i].value = (float)(CCoeffs->Corner2 [ i ]  - 128) / 4.0f;
	for (i = 0; i < 59050 ; i++) EEdgeX[i].value = (float)(CCoeffs->EdgeX2 [ i ]  - 128) / 4.0f;
	for (i = 0; i < 59050 ; i++) EEdgeMid[i].value = (float)(CCoeffs->EdgeM2 [ i ]  - 128) / 4.0f;
	for (i = 0; i < 6562 ; i++)	 ESlice8DE[i].value = (float)(CCoeffs->Slice8DE1 [ i ]  - 128) / 16.0f;
	for (i = 0; i < 6562 ; i++)	 ESlice8CF[i].value = (float)(CCoeffs->Slice8CF1 [ i ]  - 128) / 16.0f;
	for (i = 0; i < 6562 ; i++)	 EDiag8[i].value = (float)(CCoeffs->Diag8[i]  - 128) / 16.0f;
	for (i = 0; i < 2188 ; i++)	 EDiag7[i].value = (float)(CCoeffs->Diag7[i]  - 128) / 16.0f;
	for (i = 0; i < 729 ; i++)	 EDiag6[i].value = (float)(CCoeffs->Diag6[i]  - 128) / 16.0f;
	for (i = 0; i < 243 ; i++)	 EDiag5[i].value = (float)(CCoeffs->Diag5[i]  - 128) / 16.0f;
	parity.value = CCoeffs->parity;
	sideTM.value = CCoeffs->sideTM;
}

void SetErrorValuesZero( )
{
	int i;

	for (i = 0; i < 59050 ; i++) ECorner25[i].value = 0;
	for (i = 0; i < 19684 ; i++) ECorner[i].value = 0;
	for (i = 0; i < 59050 ; i++) EEdgeX[i].value = 0;
	for (i = 0; i < 59050 ; i++) EEdgeMid[i].value = 0;
	for (i = 0; i < 6562 ; i++)	 ESlice8DE[i].value = 0;
	for (i = 0; i < 6562 ; i++)	 ESlice8CF[i].value = 0;
	for (i = 0; i < 6562 ; i++)	 EDiag8[i].value = 0;
	for (i = 0; i < 2188 ; i++)	 EDiag7[i].value = 0;
	for (i = 0; i < 729 ; i++)	 EDiag6[i].value = 0;
	for (i = 0; i < 243 ; i++)	 EDiag5[i].value = 0;
	parity.value = 0.0f;
	sideTM.value = 0.0f;
}

inline void SetOneCo( unsigned char &value, ErrorVal Error, float mult)
{
	int eval = int (Error.value * mult + 128.5);
	if (eval > 255) eval = 255;
	if (eval < 0) eval = 0;
	value = eval;
}

// Set Coefficient Values from Error calculation
void SetCoeffFromError ( int stage )
{
	if (stage < 0 || stage> NUM_STAGES) return;
	CCoeffs = &Coeffs[ stage ];

	int i;
	for (i = 0; i < 6562 ; i++)	SetOneCo( CCoeffs->Slice8DE1[ i ], ESlice8DE[i], 16);
	for (i = 0; i < 6562 ; i++)	SetOneCo( Coeffs->Slice8CF1[ i ], ESlice8CF[i], 16);
	for (i = 0; i < 6562 ; i++)	SetOneCo( CCoeffs->Diag8[ i ] , EDiag8[i], 16);
	for (i = 0; i < 2188 ; i++)	SetOneCo( CCoeffs->Diag7[ i ] , EDiag7[i], 16);
	for (i = 0; i < 729 ; i++)	SetOneCo( CCoeffs->Diag6[ i ] , EDiag6[i], 16);
	for (i = 0; i < 243 ; i++)	SetOneCo( CCoeffs->Diag5[ i ] , EDiag5[i], 16);

	for (i = 0; i < 59050 ; i++) SetOneCo(CCoeffs->Corner25[ i ], ECorner25[i] , 4);
	for (i = 0; i < 19684 ; i++) SetOneCo(CCoeffs->Corner2[ i ], ECorner[i]   , 4);
	for (i = 0; i < 59050 ; i++) SetOneCo(CCoeffs->EdgeX2[ i ], EEdgeX[i]    , 4);
	for (i = 0; i < 59050 ; i++) SetOneCo(CCoeffs->EdgeM2[ i ], EEdgeMid[i]  , 4);

	CCoeffs->parity = parity.value;
	CCoeffs->sideTM = sideTM.value;
}

// --------------------------------
// Update the Values from the total error they accumlate. (clamped at +-10)
// --------------------------------
inline void UpdateOneErrorC10 (ErrorVal &Error, float rate)
{
	if (Error.total <= 12) return; // not enough info
	if (Error.total <= 22) rate *= .3f; // don't move too fast for small sample sets 
	Error.value -= float (rate * Error.error / Error.total);
	if (Error.value > 31) Error.value = 31;
	if (Error.value < -31) Error.value = - 31;
}

void UpdateAllErrorValues( )
{
	int i;
	float rate = .075f;

	for (i = 0; i < 59050 ; i++) UpdateOneErrorC10 (ECorner25[i], rate);
	for (i = 0; i < 19684 ; i++) UpdateOneErrorC10 (ECorner[i], rate);
	for (i = 0; i < 59050 ; i++) UpdateOneErrorC10 (EEdgeX[i], rate); 
	for (i = 0; i < 59050 ; i++) UpdateOneErrorC10 (EEdgeMid[i], rate); 
	for (i = 0; i < 6562 ; i++)	 UpdateOneErrorC10 (ESlice8DE[i], rate *.5f);
	for (i = 0; i < 6562 ; i++)	 UpdateOneErrorC10 (ESlice8CF[i], rate *.5f); 
	for (i = 0; i < 6562 ; i++)	 UpdateOneErrorC10 (EDiag8[i], rate *.5f); 
	for (i = 0; i < 2188 ; i++)	 UpdateOneErrorC10 (EDiag7[i], rate *.5f);
	for (i = 0; i < 729 ; i++)	 UpdateOneErrorC10 (EDiag6[i], rate *.5f); 
	for (i = 0; i < 243 ; i++)	 UpdateOneErrorC10 (EDiag5[i], rate *.5f);
	UpdateOneErrorC10 (parity, rate);
	UpdateOneErrorC10 (sideTM, rate);
}

// -------------
void ClearAllError( )
{
unsigned int i;

	for (i = 0; i < 59050; i++)	{ECorner25[i].resetError(); }
	for (i = 0; i < 19684; i++)	{ECorner[i].resetError();}
	for (i = 0; i < 59050 ; i++) {EEdgeX[i].resetError();}
	for (i = 0; i < 59050 ; i++) {EEdgeMid[i].resetError();}
	for (i = 0; i < 6562 ; i++)	 {ESlice8DE[i].resetError();}
	for (i = 0; i < 6562 ; i++)	{ESlice8CF[i].resetError();}
	for (i = 0; i < 6562 ; i++)		 {EDiag8[i].resetError();}
	for (i = 0; i < 2188 ; i++)		 {EDiag7[i].resetError();}
	for (i = 0; i < 729 ; i++)		 {EDiag6[i].resetError(); }
	for (i = 0; i < 243 ; i++)		 {EDiag5[i].resetError(); }
	parity.total = 8;
	sideTM.total = 8;
}

// --------------
void SaveAllValues( char *filename )
{
	int i;
	FILE *FP = fopen (filename, "wb");

	for (i = 0; i < 59050 ; i++) fwrite(&ECorner25[i].value, 1, sizeof(float), FP);
	for (i = 0; i < 19684 ; i++) fwrite(&ECorner[i].value, 1, sizeof(float), FP);
	for (i = 0; i < 59050 ; i++) fwrite(&EEdgeX[i].value, 1, sizeof(float), FP);
	for (i = 0; i < 59050 ; i++) fwrite(&EEdgeMid[i].value, 1, sizeof(float), FP);
	for (i = 0; i < 6562 ; i++) fwrite(&ESlice8DE[i].value, 1, sizeof(float), FP);
	for (i = 0; i < 6562 ; i++) fwrite(&ESlice8CF[i].value, 1, sizeof(float), FP);
	for (i = 0; i < 6562 ; i++) fwrite(&EDiag8[i].value, 1, sizeof(float), FP);
	for (i = 0; i < 2188 ; i++) fwrite(&EDiag7[i].value, 1, sizeof(float), FP);
	for (i = 0; i < 729 ; i++)  fwrite(&EDiag6[i].value, 1, sizeof(float), FP);
	for (i = 0; i < 243 ; i++)  fwrite(&EDiag5[i].value, 1, sizeof(float), FP);
	fwrite( &parity.value, 1, sizeof(float), FP);
	fwrite( &sideTM.value, 1, sizeof(float), FP);

	fclose (FP);
}

// --------------
void LoadAllValues( char *filename )
{
	int i;
	FILE *FP = fopen (filename, "rb");

	for (i = 0; i < 59050 ; i++) fread (&ECorner25[i].value,  sizeof(float), 1, FP);
	for (i = 0; i < 19684 ; i++) fread (&ECorner[i].value,  sizeof(float), 1, FP);
	for (i = 0; i < 59050 ; i++) fread (&EEdgeX[i].value,  sizeof(float), 1, FP);
	for (i = 0; i < 59050 ; i++) fread (&EEdgeMid[i].value,  sizeof(float), 1, FP);
	for (i = 0; i < 6562 ; i++) fread (&ESlice8DE[i].value,  sizeof(float), 1, FP);
	for (i = 0; i < 6562 ; i++) fread (&ESlice8CF[i].value,  sizeof(float), 1, FP);
	for (i = 0; i < 6562 ; i++) fread (&EDiag8[i].value,  sizeof(float), 1, FP);
	for (i = 0; i < 2188 ; i++) fread (&EDiag7[i].value,  sizeof(float), 1, FP);
	for (i = 0; i < 729 ; i++)  fread (&EDiag6[i].value,  sizeof(float), 1, FP);
	for (i = 0; i < 243 ; i++)  fread (&EDiag5[i].value,  sizeof(float), 1, FP);
	fread (&parity.value, sizeof (float), 1 , FP);
	if (fread (&sideTM.value, sizeof (float),1 ,  FP) == 0) sideTM.value = 0;

	fclose (FP);
}

// -------------
void UpdateOne( ErrorVal &Element, float Error )
{
	Element.total++;
	Element.error += Error;
}

// ---------------------------
// This function is really long
// ---------------------------
void StoreError( float Error, Board &cBoard)
{
	int i;
	int Value, Value2, Value3, Value4;
	int top = 0, left = 0, right = 0, bottom = 0;

	UpdateOne ( parity, Error);

	// Diagonals (Diag 5)
	for ( i = 0; i < 4; i++)
	{
		if (i == 0) Value = cBoard.southeast[4]; 
		if (i == 1) Value = cBoard.southeast[10];
		if (i == 2) Value = cBoard.southwest[4];
		if (i == 3) Value = cBoard.southwest[10];

		MirrorSliceL(5, Value, Value2, Value3, Value4);

		UpdateOne( EDiag5 [ Value ] , Error);    UpdateOne( EDiag5 [ Value2 ] , Error);
		UpdateOne( EDiag5 [ Value3 ] , -Error);  UpdateOne( EDiag5 [ Value4 ] , -Error);
	}

	// Diag6
	for ( i = 0; i < 4; i++)
	{
		if (i == 0) Value = cBoard.southeast[5]; 
		if (i == 1) Value = cBoard.southeast[9];
		if (i == 2) Value = cBoard.southwest[5];
		if (i == 3) Value = cBoard.southwest[9];
		MirrorSliceL (6, Value, Value2, Value3, Value4);

		UpdateOne ( EDiag6 [ Value ] , Error);    UpdateOne ( EDiag6 [ Value2 ] , Error);
		UpdateOne ( EDiag6 [ Value3 ] , -Error);  UpdateOne ( EDiag6 [ Value4 ] , -Error);
	}

	// Diag7
	for ( i = 0; i < 4; i++)
	{
		if (i == 0) Value = cBoard.southeast[6]; 
		if (i == 1) Value = cBoard.southeast[8];
		if (i == 2) Value = cBoard.southwest[6];
		if (i == 3) Value = cBoard.southwest[8];
		MirrorSliceL (7, Value, Value2, Value3, Value4);

		UpdateOne ( EDiag7 [ Value ] , Error);    UpdateOne ( EDiag7 [ Value2 ] , Error);
		UpdateOne ( EDiag7 [ Value3 ] , -Error);  UpdateOne ( EDiag7 [ Value4 ] , -Error);
	}

	// Diag8
	for ( i = 0; i < 2; i++)
	{
		if (i == 0) Value = cBoard.southeast[7]; 
		if (i == 1) Value = cBoard.southwest[7];
		MirrorSlice (Value, Value2, Value3, Value4);

		UpdateOne ( EDiag8 [ Value ] , Error);    UpdateOne ( EDiag8 [ Value2 ] , Error);
		UpdateOne ( EDiag8 [ Value3 ] , -Error); UpdateOne ( EDiag8 [ Value4 ] , -Error);
	}

	// SliceDE
	for ( i = 0; i < 4; i++)
	{
		if (i == 0) Value = cBoard.east[4]; 
		if (i == 1) Value = cBoard.east[5];
		if (i == 2) Value = cBoard.south[4];
		if (i == 3) Value = cBoard.south[5];
		MirrorSlice (Value, Value2, Value3, Value4);
		UpdateOne ( ESlice8DE [ Value ] , Error);   UpdateOne ( ESlice8DE [ Value2 ] , Error);
		UpdateOne ( ESlice8DE [ Value3 ] , -Error); UpdateOne ( ESlice8DE [ Value4 ] , -Error);
	}

	// SliceCF
	for ( i = 0; i < 4; i++)
	{
		if (i == 0) Value = cBoard.east[3]; 
		if (i == 1) Value = cBoard.east[6];
		if (i == 2) Value = cBoard.south[3];
		if (i == 3) Value = cBoard.south[6];
		MirrorSlice (Value, Value2, Value3, Value4);
		UpdateOne ( ESlice8CF [ Value ] , Error);   UpdateOne ( ESlice8CF [ Value2 ] , Error);
		UpdateOne ( ESlice8CF [ Value3 ] , -Error); UpdateOne ( ESlice8CF [ Value4 ] , -Error);
	}

	// Edge + Mid
	Value = Middle6[ cBoard.east[1] ] + Middle4 [ cBoard.east[2]];
	if (Value >= 0)
	{
		MirrorEdgeMid (Value, Value2, Value3, Value4);
		UpdateOne ( EEdgeMid [ Value ] , Error);   UpdateOne ( EEdgeMid [ Value2 ] , Error);
		UpdateOne ( EEdgeMid [ Value3 ] , -Error); UpdateOne ( EEdgeMid [ Value4 ] , -Error);
		top = 1;
	}

	Value = Middle6[ cBoard.east[8] ] + Middle4 [ cBoard.east[7]];
	if (Value >= 0)
	{
		MirrorEdgeMid (Value, Value2, Value3, Value4);
		UpdateOne ( EEdgeMid [ Value ] , Error);   UpdateOne ( EEdgeMid [ Value2 ] , Error);
		UpdateOne ( EEdgeMid [ Value3 ] , -Error); UpdateOne ( EEdgeMid [ Value4 ] , -Error);
		bottom = 1;
	}

	Value = Middle6[ cBoard.south[1] ] + Middle4 [ cBoard.south[2]];

	if (Value >= 0)
	{
		MirrorEdgeMid (Value, Value2, Value3, Value4);
		UpdateOne ( EEdgeMid [ Value ] , Error);   UpdateOne ( EEdgeMid [ Value2 ] , Error);
		UpdateOne ( EEdgeMid [ Value3 ] , -Error); UpdateOne ( EEdgeMid [ Value4 ] , -Error);
		left = 1;
	}

	Value = Middle6[ cBoard.south[8] ] + Middle4 [ cBoard.south[7]];

	if (Value >= 0)
	{
		MirrorEdgeMid (Value, Value2, Value3, Value4);
		UpdateOne ( EEdgeMid [ Value ] , Error);   UpdateOne ( EEdgeMid [ Value2 ] , Error);
		UpdateOne ( EEdgeMid [ Value3 ] , -Error); UpdateOne ( EEdgeMid [ Value4 ] , -Error);
		right = 1;
	}

	// Edge + 2X
	int x1, x2, x3, x4, d8se, d8sw;

	d8se = cBoard.southeast[7];
	d8sw = cBoard.southwest[7];
		
	x1 = DiagI[ d8se ].XSquare1; 
	x3 = DiagI[ d8se ].XSquare2; 
	x2 = DiagI[ d8sw ].XSquare1; 
	x4 = DiagI[ d8sw ].XSquare2;

	if (top == 0)
	{
		Value = cBoard.east[1]  + x1 + x4 * 3;
		MirrorEdgeX (Value, Value2, Value3, Value4);

		UpdateOne ( EEdgeX [ Value ] , Error);   UpdateOne ( EEdgeX [ Value2 ] , Error);
		UpdateOne ( EEdgeX [ Value3 ] , -Error); UpdateOne ( EEdgeX [ Value4 ] , -Error);
	}

	if (bottom == 0)
	{
		Value = cBoard.east[8] + x2  + x3 * 3;
		MirrorEdgeX (Value, Value2, Value3, Value4);

		UpdateOne ( EEdgeX [ Value ] , Error);	 UpdateOne ( EEdgeX [ Value2 ] , Error);
		UpdateOne ( EEdgeX [ Value3 ] , -Error); UpdateOne ( EEdgeX [ Value4 ] , -Error);
	}

	if (left == 0)
	{
		Value = cBoard.south[1] + x1 + x2 * 3;
		MirrorEdgeX (Value, Value2, Value3, Value4);

		UpdateOne ( EEdgeX [ Value ] , Error);   UpdateOne ( EEdgeX [ Value2 ] , Error);
		UpdateOne ( EEdgeX [ Value3 ] , -Error); UpdateOne ( EEdgeX [ Value4 ] , -Error);
	}

	if (right == 0)
	{
		Value = cBoard.south[8] + x4 + x3 * 3;
		MirrorEdgeX (Value, Value2, Value3, Value4);

		UpdateOne ( EEdgeX [ Value ] , Error);	 UpdateOne ( EEdgeX [ Value2 ] , Error);
		UpdateOne ( EEdgeX [ Value3 ] , -Error); UpdateOne ( EEdgeX [ Value4 ] , -Error);
	}

	// 2x5 Corners
	// Top
	if (top == 0)
	{
		Value = First5[ cBoard.east[1] ] + First5[ cBoard.east[2] ] * 243;
		Mirror25Corner (Value, Value2);
		UpdateOne (ECorner25 [ Value ],  Error); 
		UpdateOne (ECorner25 [ Value2 ], -Error);

		Value = Last5[ cBoard.east[1] ] + Last5[ cBoard.east[2] ] * 243;
		Mirror25Corner (Value, Value2);
		UpdateOne (ECorner25 [ Value ],  Error); 
		UpdateOne (ECorner25 [ Value2 ], -Error);
	}

	// Left
	if (left == 0)
	{
		Value = First5[ cBoard.south[1] ] + First5[ cBoard.south[2] ] * 243;
		Mirror25Corner (Value, Value2);
		UpdateOne (ECorner25 [ Value ],  Error); 
		UpdateOne (ECorner25 [ Value2 ], -Error);

		Value = Last5[ cBoard.south[1] ] + Last5[ cBoard.south[2] ] * 243;
		Mirror25Corner (Value, Value2);
		UpdateOne (ECorner25 [ Value ],  Error); 
		UpdateOne (ECorner25 [ Value2 ], -Error);
	}

	// Bottom
	if (bottom == 0)
	{
		Value = First5[ cBoard.east[8] ] + First5[ cBoard.east[7] ] * 243;
		Mirror25Corner (Value, Value2);
		UpdateOne (ECorner25 [ Value ],  Error); 
		UpdateOne (ECorner25 [ Value2], -Error);

		Value = Last5[ cBoard.east[8] ] + Last5[ cBoard.east[7] ] * 243;
		Mirror25Corner (Value, Value2);
		UpdateOne (ECorner25 [ Value ],  Error); 
		UpdateOne (ECorner25 [ Value2], -Error);
	}

	if (right == 0)
	// Right
	{
		Value = Last5[ cBoard.south[8] ] + Last5[ cBoard.south[7] ] * 243;
		Mirror25Corner (Value, Value2);
		UpdateOne (ECorner25 [ Value ],  Error); 
		UpdateOne (ECorner25 [ Value2], -Error);

		Value = First5[ cBoard.south[8] ] + First5[ cBoard.south[7] ] * 243;
		Mirror25Corner (Value, Value2);
		UpdateOne (ECorner25 [ Value ],  Error); 
		UpdateOne (ECorner25 [ Value2], -Error);
	}

	// 3x3 Corners
	// 1 4
	// 3 2
	for (i = 0; i < 4; i++)
	{
		Value = cBoard.corners[i];
		MirrorCorner (Value, Value2, Value3, Value4);

		UpdateOne ( ECorner [ Value ], Error );	
		UpdateOne ( ECorner [ Value2 ], Error );
		UpdateOne ( ECorner [ Value3], -Error );	
		UpdateOne ( ECorner [ Value4 ], -Error );
	}

	// Parity
	if (( (cBoard.nummarkers & 1) == 0 && cBoard.color == WHITE) || ((cBoard.nummarkers & 1) != 0 && cBoard.color == BLACK))
		UpdateOne (parity, -Error); else UpdateOne (parity, Error); 

	if ( cBoard.color == WHITE)
		UpdateOne (sideTM, Error); else UpdateOne (sideTM, -Error); 
}

// ==================================
// Evaluate Board from Error.value
// ==================================
double EvaluateBoardError (Board &cBoard)
{
	// Consider Eliminations
	if (cBoard.numblack == 0) return 400;
	if (cBoard.numblack == cBoard.nummarkers) return -400;

	// Slices
	double eval = 0;

	eval += (ESlice8CF[ cBoard.east[3]  ].value + ESlice8DE[ cBoard.east[4] ].value 
		   + ESlice8DE[ cBoard.east[5]  ].value + ESlice8CF[ cBoard.east[6] ].value
		   + ESlice8CF[ cBoard.south[3] ].value + ESlice8DE[ cBoard.south[4] ].value 
		   + ESlice8DE[ cBoard.south[5] ].value + ESlice8CF[ cBoard.south[6] ].value );

	eval += ( EDiag7[ cBoard.southeast[6] ].value + EDiag7[ cBoard.southwest[6] ].value + EDiag7[ cBoard.southeast[8] ].value  + EDiag7[ cBoard.southwest[8] ].value
		   +  EDiag6[ cBoard.southeast[5] ].value + EDiag6[ cBoard.southwest[5] ].value + EDiag6[ cBoard.southeast[9] ].value  + EDiag6[ cBoard.southwest[9] ].value
		   +  EDiag5[ cBoard.southeast[4] ].value + EDiag5[ cBoard.southwest[4] ].value + EDiag5[ cBoard.southeast[10] ].value + EDiag5[ cBoard.southwest[10] ].value 
	  		//+  EDiag4 [ cBoard.southeast[3] ] + EDiag4 [ cBoard.southwest[3] ] + EDiag4 [ cBoard.southeast[11] ] + EDiag4 [ cBoard.southwest[11] ] 
		   );

	int d8se = cBoard.southeast[7];
	int d8sw = cBoard.southwest[7];
	
	eval += EDiag8 [ d8se ].value + EDiag8 [ d8sw ].value;
			                     
	int x1 = DiagI[ d8se ].XSquare1; 
	int x3 = DiagI[ d8se ].XSquare2; 
	int x2 = DiagI[ d8sw ].XSquare1;
	int x4 = DiagI[ d8sw ].XSquare2;

///edges / corner 2x5s
	if (cBoard.edge1 == 0) 
			eval+= EEdgeMid [ Middle6[ cBoard.east[1] ] + Middle4 [ cBoard.east[2]] ].value;
	else 
	{
		eval+= EEdgeX [cBoard.east[1] + x1 + x4 * 3].value;
		eval+= ECorner25[ First5[ cBoard.east[1] ] + First5[ cBoard.east[2] ] ].value
				+ ECorner25[ Last5[ cBoard.east[1] ] + Last5[ cBoard.east[2] ] ].value;
	}
	if (cBoard.edge2 == 0) 
			eval+= EEdgeMid [ Middle6[ cBoard.east[8] ] + Middle4 [ cBoard.east[7]] ].value;
	else 
	{
		eval+= EEdgeX [cBoard.east[8] + x2 + x3 * 3].value;
		eval+= ECorner25[ First5[ cBoard.east[8] ] + First5[ cBoard.east[7] ] ].value
				+ ECorner25[ Last5[ cBoard.east[8] ]  + Last5[ cBoard.east[7] ] ].value;
	}
	if (cBoard.edge3 == 0) 
		eval+= EEdgeMid [ Middle6[ cBoard.south[1] ] + Middle4 [ cBoard.south[2]] ].value;
	else 
	{
		eval+= EEdgeX [cBoard.south[1] + x1 + x2 * 3].value;
		eval+= ECorner25[ First5[ cBoard.south[1] ] + First5[ cBoard.south[2] ] ].value
			+ ECorner25[ Last5[ cBoard.south[1] ]  + Last5[ cBoard.south[2] ] ].value;
	}
	if (cBoard.edge4 == 0) 
		eval+= EEdgeMid [ Middle6[ cBoard.south[8] ] + Middle4 [ cBoard.south[7]] ].value;
	else 
	{
		eval+= EEdgeX [cBoard.south[8] + x4 + x3 * 3].value;
		eval+= ECorner25[ Last5[ cBoard.south[8] ]  + Last5[ cBoard.south[7] ] ].value
				+ ECorner25[ First5[ cBoard.south[8] ] + First5[ cBoard.south[7] ] ].value;
	}

	// Corners
	eval += ECorner[ cBoard.corners[0] ].value + ECorner[ cBoard.corners[1] ].value 
		  + ECorner[ cBoard.corners[2] ].value + ECorner[ cBoard.corners[3] ].value;

	// Overall Parity
	if (( (cBoard.nummarkers & 1) == 0 && cBoard.color == WHITE) || ((cBoard.nummarkers & 1) != 0 && cBoard.color == BLACK))
              eval-=parity.value; else eval+=parity.value;

	// Side to move
	if ( cBoard.color == WHITE)
        eval+=sideTM.value; else eval-=sideTM.value;

    return eval;
}
// ---------------------------------
// Code for Gradient Descent Learning
// and generate training positions
// ---------------------------------
void SaveOutput( char *filename, int iteration, int low, int high, float value, float avgError, float percent, float percent2, int MaxError, int MaxNum, int total)
{
	FILE *FP = fopen( filename, "ab" );

	avgError /= 3.2f;
	value /=3.2f;

	fprintf (FP, "%d: \tAvg Val: %.03f \tAvg.Err %.03f W/L (0ply) %%= %.03f Max Error: %d (%d) Sample Size: %d \n", iteration, value, avgError, percent, MaxError, MaxNum, total);

	fclose (FP);
}

//---------------------------
// Read Through positions and check the error and other statistics
// --------------------------
void CheckPosition( char cBoard[], char *filename, int score, int iteration)
{
	int i, numright, numright2, totalE, total,  RealEval, z = 0,bestmove = 127;
	float AvgEval = 0, AvgError = 0, Error;
	float Eval, oldEval;
	short east[10];
	uint8_t nummarkers, solved;
	Board Board2;
	char TempBoard[66];

	numright = 0;
	numright2 = 0;
	total = 0;
	totalE = 0;
	int MaxError = 0, MaxNum = 0;

	ClearAllError();

	FILE *FP = fopen( filename, "rb" );
	if (FP == NULL) return;

	while (!feof(FP) )
	{
		 fread(east, sizeof(short), 8, FP);
		 fread(&nummarkers, 1, 1, FP);
		 fread(&TempBoard[64], 1, 1, FP);
		 fread(&solved, 1, 1, FP);
		 RealEval = solved - 128;
		 z++;

		if (/*RealEval != 0 &&*/ abs(RealEval) <= 42)
		{
			for (i = 0; i < 8; i++)
				SliceZ( east[i], &TempBoard[ i*8 - 1 ]);

			ConvertBoard (Board2, TempBoard);

			total++;

			Eval = (float)EvaluateBoardError(Board2);

			oldEval = Eval;

		/*	BoardStack[0] = board2;
			CurrentAhead = 2;
			hashing = 0;
			final = 0;
			bestmove = INVALID_MOVE;
			eval = AlphaBetaSearch (board2.color, 0, -3000, 3000,  bestmove, 0, 65);
			hashing = 1;
			if (fabs (eval) > 380) eval = oldEval;*/

			if (RealEval !=0)
				{if ((Eval <0 && RealEval <0) || (Eval >0 && RealEval >0)) numright++;
				totalE ++;
				}
		
			Error = (.5f + Eval - ((float)RealEval * 3.2f) ); 

			//if (abs(RealEval) > 12) Error *= (12.0f / float (abs(RealEval)) );

			StoreError (Error, Board2);

			Error = (float) fabs (Error);

			if (Error > MaxError) 
			{
				MaxError = (int) Error;
				MaxNum = z;
				memcpy (StartBoard, TempBoard, 65);
				memcpy (cBoard, TempBoard, 65);
			}

			if (RealEval !=0)
			{
				if ((Eval <=0 && RealEval <0) || (Eval >=0 && RealEval >0)) numright2++;
			}

 			AvgError = float(AvgError * (total-1) + Error ) / float(total);

			if (RealEval < 0) Eval = -Eval;

			AvgEval = float(AvgEval * (total-1) + Eval) / float(total);
		}
	}

	fclose (FP);

	UpdateAllErrorValues();

	char buffer[100];
	sprintf (buffer, "%d out of %d (%.2f%%)\015\012Max Error: %d at %d \015\012 Average Err: %f", numright, total, (float)(100*numright)/(float)totalE, MaxError, MaxNum, AvgError);
	SetDlgItemText(GameStats, 112, buffer);
	SaveOutput ("stats.txt", iteration, score-2, score+2, AvgEval, AvgError, (float)(100*numright)/(float)totalE, (float)(100*numright2)/(float)totalE, MaxError, MaxNum, total);
}

// ---------------------------------
// Just read a position from the file
// ---------------------------------
void ReadPosition( char cBoard[], char *filename, int z )
{
	short east[10];
	unsigned char nummarkers, solved;
	Board Board2;

	FILE* FP = fopen(filename, "rb");
	if (FP == NULL) return;
	if (z <= 0) return;

	for (int i = 0; i < z; i++) {
		fread(east, sizeof(short), 8, FP);
		fread(&nummarkers, 1, 1, FP);
		fread(&cBoard[64], 1, 1, FP);
		fread(&solved, 1, 1, FP);
	}

	for (int i = 0; i < 8; i++)
		SliceZ( east[i], &cBoard[ i * 8-1 ]);

	memcpy(StartBoard, cBoard, 65);
	ConvertBoard(Board2, cBoard);
	int Eval = (int)EvaluateBoardError(Board2);

	char buffer[100];
	sprintf( buffer, "#%d (%d e) Solved at %d \015\012 Eval: %.2f", z, 64-nummarkers, (solved-128), (float)Eval / 3.2f );
	SetDlgItemText(GameStats, 112, buffer);

	fclose(FP);
}

// -----------------------------------
// Read positions from a file, solve them, and output them with the solved value
// -----------------------------------
void ReadSolvePosition (char cBoard[], char *infile, char *outfile)
{
	int z = 0;
	int i, bestmove, eval, solvT;
	short east[10];
	uint8_t nummarkers, solved;
	Board board2;

	FILE* FP = fopen(infile, "rb");
	if (FP)
	{
		//fseek (FP, z * 19, SEEK_SET);
		fread(east, sizeof(short), 8, FP);

		//for (int temp = 0; temp < z; temp++)
		while (!feof(FP))
		{
			fread(&nummarkers, 1, 1, FP);
			fread(&cBoard[64], 1, 1, FP);
			fread(&solved, 1, 1, FP);
			z++;

			for (i = 0; i < 8; i++)
				SliceZ(east[i], &cBoard[i * 8 - 1]);

			ConvertBoard(board2, cBoard);
			bestmove = INVALID_MOVE;

			BoardStack[0] = board2;
			CurrentAhead = 6;
			final = 0;
			if (board2.nummarkers >= 48) {
				CurrentAhead = 65 - BoardStack[0].nummarkers;
				final = 1;
			}
			eval = AlphaBetaSearch(board2.color, 0, -3000, 3000, bestmove);

			solvT = int(128 + (eval / 3));
			if (solvT < 1) solvT = 1;
			if (solvT > 254) solvT = 254;
			solved = solvT;

			SavePosition(board2, outfile, solved);
			DoOneSquareMove(board2, bestmove, board2.color);
			SavePosition(board2, outfile, solved);

			fread(east, sizeof(short), 8, FP);
		}

		fclose(FP);
	}

	char buffer[100];
	snprintf(buffer, sizeof(buffer), "#%d Solved at %d \n ",z, (solved-128));
	SetDlgItemText(GameStats, 112, buffer);
}