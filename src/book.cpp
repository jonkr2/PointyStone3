//============================
// Opening Book Functions
//============================
// Pointy Stone 3 Othello 
// by Jonathan Kreuzer

//
// This opening book uses move lists to store openings and thus
// doesn't support transpositions (except special code for a couple very common ones.)
//

// ----------
// Find the Computer's Move to Play in the Opening Book
//
// returns -5 if the move isn't in the book
// otherwise returns the eval of the move
// ---------
int PlayBook( char color, short &x, short &y, int &total, int random)
{
	int i=0, value, temp, move, bi, sq;

	char tempString[200];
	short mx[15], my[15], mvalue[15];
	total = 0;
	        
	if (OpeningsType == 1) random = 0;
	if (OpeningsType == 2 || OpeningsType == 3 || (OpeningsType = 0 && brainType == BT_GREEDY) ) random = 1;

	clock_t randtime;
	randtime = clock();
	srand(randtime);

	// Convert transcript to the standard orientation ( c4 )
	char sTranscript[180];
	StandardizeTranscript( sTranscript );
	sTranscript[0]+='f';

	// Convert transcript to single byte moves
	for (i = 0; i < 42; i++)
		tempString[i] = 7 - (sTranscript[i*2+5] - '1') + (7 - (sTranscript[i*2+4] - 'a')) * 8 + 'A';

	// binary search.
	// tempString [ Transcript [0] ] = '\n';
	// tempString [ Transcript [0] + 1] = NULL;
	// low = 0; 
	// high = BookLen;
	// index = (low + high) /2; 
	// value = memcmp (tempString,  &OBook [bi + 3],  Transcript[0] - 1);
	// if (value == 0) sq;
	// if (value < 0) high = value;
	// if (value > 0) low = value;
	// bleh

	if (color == WHITE) value = -1;
	if (color == BLACK) value = 1;

	i = 0;

	while (i < BookLen)
	{
		bi = i * 48;

		if (OBook[ bi +2 ] == sTranscript[0])
		   if ( memcmp( &OBook[bi+3], tempString, Transcript[0] - 1 ) == 0)
			{           
				if (color==WHITE) 
				{
					if (OBook[bi] - '2' >= value || (random == 1 && OBook[bi]-'2' >=-1))
					{
						 sq = OBook[bi + Transcript[0] + 2] - 'A';
						 if (sq < 0) sq+=256;
						 if (OBook[bi] - '2' > value && random == 0) total = 0;
						 if (OBook[bi] - '2' > value) value = OBook[bi] - '2';
						 mx[total] = 7 -char (sq >> 3 );
						 my[total] = 7 -char (sq & 7 );
						 mvalue[total] = OBook[bi] - '2' ;
						 total++;
					}
				}
				else  if (OBook[bi] - '2' <= value ||  (random == 1 && OBook[bi]-'2' <=1))
				{
					sq = OBook[bi + Transcript[0] + 2] - 'A';
					if (sq < 0) sq+=256;
					if (OBook[bi] - '2' < value && random == 0) total = 0;
					if (OBook[bi] - '2' < value) value = OBook[bi] - '2';
					mx[total] = 7 -char (sq >> 3);
					my[total] = 7 -char (sq & 7 );
					mvalue[total] = OBook[bi] - '2' ;
					total++;
				}
			}

		i++;
	}

	if (Transcript[0] == 0)// If this is the first move of the game, play any of the four moves.
	{
		total = 4; 
		mx[0] = 3; my[0] = 2; 
		mx[1] = 2; my[1] = 3; 
		mx[2] = 4; my[2] = 5; 
		mx[3] = 5; my[3] = 4; 
		mvalue[0] = 0; mvalue[1] = 0; mvalue[2] = 0; mvalue[3] = 0;
	}

	/*char buffer[1024];
	sprintf( buffer, "Transcript : %s, Color : %d BookMoves : %d value : %d randombook : %d time : %d", &sTranscript[2], color, total, value, random, Transcript[0]  );
	MessageBox( MainWnd, buffer, &sTranscript[0], MB_OK );*/

	if (total == 0) return -5; // position not in book

	// randomly select a move from ones with the same value, or within 1 of the best for random.
	move = rand()%total; 
	i = 0;
	while (abs (mvalue[move] - value) > 1 && i < 40) {move = rand()%total; i++; }
	x = mx[ move ];
	y = my[ move ];
	value = mvalue[move];

	// Compensate for board orientation (if c4 isn't the first move.)

	if (Transcript [2] == 'f') {
		x = (7 - x);
		y = (7 - y);
	}

	if (Transcript [2] == 'd') {
		temp = x;
		x = (y);
		y = (temp);
	}

	if (Transcript [2] == 'e') {
		temp =  x;
		x = (7- y);
		y = (7 - temp);
	}                

	if (sTranscript[121] == '5') // This is for a transposed diagonal opening
	{
		temp = x;
		x = (y);
		y = (temp);
	}
	 
	sTranscript[0]-='f';

	return value;
}

// ------------------------------------------
//  Save the opening book.
//  the save file is compressed by not saving redundant information
// ------------------------------------------
void CompressSaveOBook()
{
	// insertion sort (store sorted indices in SortBook)
	for (int i = 0; i < BookLen; i++)
	{
		int x = i;
		while (x > 0 && strcmp(&OBook[ i * 48 + 3], &OBook[SortBook[x-1] * 48 + 3]) < 0) x--;

		for (int t = i; t > x; t--) SortBook[t] = SortBook[t-1];
		SortBook[x] = i;
	}

	// save it
	FILE* fp = fopen ("bookn3.pnt", "wb");
	if (fp)
	{
		int lastvalue = 0;
		unsigned char line[80];

		for (int x = 0; x < BookLen; x++)
		{
			int i = SortBook[x] * 48;
			int same = 0;
			int len = (OBook[i + 2] - 'f');
			if (x > 0)
			{
				while (OBook[i + 3 + same] == OBook[SortBook[x - 1] * 48 + 3 + same] && same < len) same++;
			}

			line[0] = OBook[i + 2] - 'f' + 132;
			line[1] = OBook[i];
			int size = 2 + line[0] - 132 - same;
			if (lastvalue == line[1]) {
				memcpy(&line[1], &OBook[i + 3 + same], size); size--;
			}
			else {
				memcpy(&line[2], &OBook[i + 3 + same], size);
			}

			fwrite(line, 1, size, fp);
			lastvalue = OBook[i];
		}

		fclose(fp);
	}
}

// -------------------------------
// Load the the opening book from the disk into
// an expanded form in memory for simpler searching.
// -------------------------------

void LoadCompBook()
{
	FILE *OpenBook;
	int x = 0;
	int length, pos, i;
	unsigned char TempString[128], buffer[128], nextPos;

	OpenBook = fopen ("bookn3.pnt", "rb");

	if (OpenBook == NULL) // Can't open book file.
	{
		MessageBox (MainWnd, "Can't find file 'bookn2.pnt', Pointy may not play properly", "Error", MB_OK);
		return; 
	}

	memset (buffer, 0, 120);
	nextPos = fgetc (OpenBook) +'f' - 132;
	TempString[1] = '-';

	while (!feof(OpenBook) && x < BOOK_MAX)
	{
		TempString[2] = nextPos;
		i = 3;
		while ( !feof(OpenBook) && buffer[i-1] < 132)
		{
			buffer[i++] = fgetc(OpenBook);
			// Get current evaluation value
			if (buffer[i-1] >= '0' && buffer[i-1] <='9') 
			{
				TempString[0] = buffer[i-1];
				i--;
			}
		}
		nextPos = buffer[i-1] +'f' - 132;
		length = i-1; 
		pos = TempString[2]-'f' - length + 3;
		memcpy (&TempString[ 3 + pos], &buffer[3], length);
		TempString[ pos + length ] = '\n';
		TempString[ pos + length + 1] = NULL;
		memcpy (&OBook[x*48], TempString,47);
		x++;
	}

	fclose (OpenBook);
	BookLen = x;

	clock_t randtime;
	randtime = clock();
	srand(randtime);
}

// ----------
// Remove a position from opening book
// ----------
void RemovePosition()
{
	char tempString2[200];
	char buffer2[300];
	int i, removed = 0, x;

	if (Transcript[0] >= 40) return;

	char sTranscript[180];
	StandardizeTranscript( sTranscript );
	for (i = 0; i < 44; i++)
		tempString2[i+1] = 7 - (sTranscript[i*2+5] - '1') + (7 - (sTranscript[i*2+4] - 'a')) * 8 + 'A';
	tempString2[0] = sTranscript[0] + 'e';
	tempString2[Transcript[0]] = NULL;

	i = 0;

	while (i < BookLen)
	{
	if ( memcmp( &OBook[i*48+2], tempString2, Transcript[0]) == 0)
			{
			sprintf (buffer2, " Position %d removed.", i + 1);
			SetDlgItemText ( GameStats, 112, buffer2 );
			removed++ ;

			for (x = i; x < BookLen-1 ; x++) memcpy ( &OBook[(x)*48], &OBook[(x+1)*48], 46 );
			BookLen--;
			}
	i++;
	}

	if (removed == 0) SetDlgItemText ( GameStats, 112, "Position not Found." );
	else CompressSaveOBook ();
}        

//--------
// Add position to the opening book
//--------
void AddtoBook(char input)
{
	char addbook[200];
	char temptran[140];
	char buffer2[300];
	int x, i;
	addbook[0] = input;
	addbook[1] = '-';
	addbook[2] = NULL;

	if (Transcript[0] >= 40)
		{SetDlgItemText ( GameStats, 112, "Can't add positions in the ending stage." );
		 return;
		}
	if (BookLen >= BOOK_MAX-1) 
		{SetDlgItemText ( GameStats, 112, "The opening book has reached the defined Max Size, position cannot be added." );
		 return;
		}

	char sTranscript[180];
	StandardizeTranscript( sTranscript );
	for (x = 0; x < 42; x++)
	  temptran[x+1] = 7 - (sTranscript[x*2+5] - '1') + (7 - (sTranscript[x*2+4] - 'a')) * 8 + 'A';
	 
	temptran[Transcript[0]] = NULL;

	temptran[0] = sTranscript[0] + 'e';

	strcat (addbook, temptran);
	strcat (addbook, "\n");

	// if position is already in there don't add, just change value
	i = 0;
	while (i < BookLen)
	{if (memcmp( &OBook[i*48+2], temptran, Transcript[0]) == 0)
		{
		sprintf (buffer2, " Position %d was %d, now %d", i + 1, OBook[i * 48] -'2', input - '2');
		OBook[i * 48] = input;
		SetDlgItemText ( GameStats, 112, buffer2 );
		CompressSaveOBook();
		return;
		}
	i++;
	}

	// add the position
	memcpy (&OBook[BookLen*48], addbook, 46);
	BookLen++;

	sprintf (buffer2, " # %d added at %d", BookLen, input -'2');
	SetDlgItemText ( GameStats, 112, buffer2 );

	CompressSaveOBook();
}

//
//  LOAD OPENING NAMES
//
void LoadOpeningNames( )
{
	FILE *input;
	int i = 0, x;

	input = fopen ("openings.txt" , "rt");
	if (input == NULL) return;

	fread (Oscript[i], 41 , 1, input);
	while ( !feof (input) )
			{                          
			x = 0;
			do 
				{
				Oname[i][x] = (char)fgetc(input);
				x++;
				}
			while (Oname[i][x-1] != '\n' && x < 120);

			Oname [i][x-1] = NULL;
			i++;
			fread (Oscript[i], 41 , 1, input);
			}

	fclose (input); 
}

int GetOpeningNumber( int GameTime )
{
	int OpenName = 500, x;
	char sTranscript[180];
	StandardizeTranscript( sTranscript );

	for (int i = 0; i < 105; i++)
		{
		for (x = 0; x < GameTime; x++)
			{if (Oscript[i][0 + x*2] == ' ') x = 150;
			 if (Oscript[i][0 + x*2] != sTranscript[ x*2 + 2 ] || Oscript[i][1 + x*2] != sTranscript[ x*2 + 3 ]) 	x = 150;
			}
		if (x<150 && Oscript[i][GameTime*2] == ' ') 
			{OpenName = i;
			 i = 500;}
		}
	return OpenName;
}
