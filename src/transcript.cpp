// Pointy Stone 3 Othello 
// by Jonathan Kreuzer
// jkreuzer@3dkingdoms.com

// Create a standard transcipt, ie. rotate the moves of the transcipt as if blacks first move was c4
// (Also account for transposition in Tiger)
void StandardizeTranscript( char *sTranscript )
{
	int x;
	char temp;

	for (x = 0; x< 122; x++)
		sTranscript[x] = NULL; // For terminated character on string for opening book.
	                
	if (Transcript [2] == 'f')
		for (x = 1; x <= Transcript[0]; x++)
			{sTranscript[ x * 2 ]    = char (7 - (Transcript [x * 2]     - 'a') + 'a');
			sTranscript[ x * 2 + 1] = char (7 - (Transcript [x * 2 + 1] - '1') + '1');
			}
	        
	if (Transcript [2] == 'd')
		for (x = 1; x <= Transcript[0]; x++)
			{
			sTranscript[ x * 2 ]    =  char ( (Transcript [x * 2 + 1] - '1')  + 'a');
			sTranscript[ x * 2 + 1] =  char ( (Transcript [x * 2 ]    - 'a')  + '1');
			}
	        
	if (Transcript [2] == 'e')
		for (x = 1; x <= Transcript[0]; x++) 
			{sTranscript[ x * 2 ]    = char ( 7 -  (Transcript [x * 2 + 1] - '1') + 'a');
			sTranscript[ x * 2 + 1] = char ( 7 -  (Transcript [x * 2  ] - 'a') + '1');
			}
	                
	if (Transcript [2] == 'c')
		for (x = 1; x <= Transcript[0]; x++)
			{sTranscript[ x * 2 ]    = Transcript [x * 2];
			sTranscript[ x * 2 + 1] = Transcript [x * 2 + 1];
			}

	// Tiger Transposition
	if (sTranscript [ 6 ] == 'f' && sTranscript[10] == 'f')
			if (sTranscript [ 11 ] == '6' && sTranscript[7] == '5')
					{sTranscript [ 11 ] = '5';
					sTranscript [7] = '6';
					}

	// Diagonal Transposition
	if (sTranscript [ 4 ] == 'c' && sTranscript[ 5 ] == '3' && sTranscript[ 6 ] == 'd' && sTranscript[ 7 ] == '3' && sTranscript [ 8 ] == 'c' && sTranscript[ 9 ] == '5')
		{
		for (x = 4; x <= Transcript[0]; x++)
			{temp = sTranscript [x * 2];
			sTranscript[ x * 2 ]     = char ( (sTranscript [x * 2 + 1] - '1')  + 'a');
			sTranscript[ x * 2 + 1]  = char ( (temp  - 'a')  + '1');
			}
		sTranscript[ 8 ] = 'e';
		sTranscript[ 9 ] = '3';
		sTranscript[121] = '5';
		}

	sTranscript[0] = Transcript[0];
}

// Add a move to the Transcript of the game
void AddTranscript (int x, int y, char InTranscript[])
{     
	InTranscript[0]+=1;
	int GameTime = InTranscript[0];
	InTranscript[GameTime * 2 ]	   = char ('a' + x);
	InTranscript[GameTime * 2 + 1] = char ('1' + y);
	// No Redos on new moves
	InTranscript[GameTime * 2 + 2] = 0;
}

// ------------------
int Moves( char Transcript[] )
{
	for (int x = 0; x <64; x++)
			{if (Transcript [x*2+2] == 0) return x;}
	return -1;
}

int Moves( )
{
	for (int x = 0; x <64; x++)
			{if (Transcript [x*2+2] == 0) return x;}
	return -1;
}
//

void DoTakeBack ( char Transcript[] )
{
int nMoves = Moves (Transcript);
if (nMoves > 0)
	{
	Transcript[0] = nMoves-1;
	Transcript[Transcript[0] * 2 + 2] = 0;
	}
}

// =================================================
// Transcript Importing( & Exporting) Functions
// =================================================
// --------------------
// Check to see if a Text line appears to be from a transcript of a4b4c6 etc.
// --------------------
int TranlineMoves (char textline[])
{
 unsigned int x;
 char tempTran[120];
 int moves = 0, i;

 for (x = 0; x < strlen (textline); x++)
	{
	if ( textline[x] <= 'H' && textline[x] >= 'A' )
		textline[x] += 'a' - 'A';

	if ( textline[x] <= 'h' && textline[x] >= 'a' && textline[x+1] <= '8' && textline[x+1] >= '1')
		{
		 // if starting move, check validity
		 if (moves!=0 || (textline[x] == 'f' && textline[x + 1] == '5') || (textline[x] == 'e' && textline[x + 1] == '6') 
			 || (textline[x] == 'c' && textline[x + 1] == '4') || (textline[x] == 'd' && textline[x + 1] == '3'))
			{tempTran[ moves * 2] = textline[x];
			tempTran[ moves * 2 + 1] = textline[x + 1];
			moves++; x++;
			}
		}
	}

if (moves > 16) {
	for (i = 0; i < (moves*2); i+=2)
			{Transcript [ i + 2 ] = tempTran[ i ];
			 Transcript [ i + 3]  = tempTran[ i + 1 ];
			}
		 Transcript [i + 2] = 0;
		 return 1;
		}
return 0;
}

// -------------------
// Accumulate Moves
// -------------------
int AccumulateTranlineMoves (char textline[], int &moves, int add)
{
 unsigned int x;
 static char tempTran[120];
 int i;

 // put moves in transcript
 if (add == 1)
	{
	for (i = 0; i < (moves*2); i+=2)
					{Transcript [ i + 2 ] = tempTran[ i ];
					 Transcript [ i + 3]  = tempTran[ i + 1 ];
					}
	 Transcript [i + 2] = 0;
	 return 1;
	}

 // look for moves
 for (x = 0; x < strlen (textline); x++)
        {
		if ( textline[x] <= 'h' && textline[x] >= 'a' && textline[x+1] <= '8' && textline[x+1] >= '1')
			{if (moves!=0 || (textline[x] == 'f' && textline[x + 1] == '5') || (textline[x] == 'e' && textline[x + 1] == '6') 
				 || (textline[x] == 'c' && textline[x + 1] == '4') || (textline[x] == 'd' && textline[x + 1] == '3'))
				{tempTran[ moves * 2] = textline[x];
				tempTran[ moves * 2 + 1] = textline[x + 1];
				moves++; x++;
				}
			}
        }


return 0;
}

// --------------------
// Check to see if a Text line appears to be from a board transcript
// --------------------
int Tranline (char textline[], int startline, int spacing)
{
 int x;
 int test = 0, test2 = 0;

 if (strlen(textline) + 1 < unsigned (startline + spacing * 7) ) return 0;

 for (x = startline; x < startline + spacing * 8; x+= spacing)
        {
          if ( textline[x] >= '1' && textline[x] <='9') test++;
          if ( textline[x+1] >= '1' && textline[x+1] <='9') test2++;
        }

if (test > 5 && test2 > 3) return 1;

      return 0;
}

// --------------------
// Just Check for any nymbers on a line
// --------------------
int Numbers (char textline[])
{
 for (unsigned int x = 0; x < strlen(textline); x+= 1)
        if ( textline[x] >= '1' && textline[x] <='9') return 1;
       
 return 0;
}

// ===========================
// Read Transcript from Disk  
// ===========================
void ReadTranscript (char fname[])
{
	FILE *input;
	char textline[1100];
	int board[500];
	char tBoard[65];

	int index = 0;
	int spacing = 0, startline=-1;
	unsigned int x, y = 0;

	for (x = 0; x<64; x++)	board[x] = 0;
					        
	input = fopen (fname, "rt");
	if (input == NULL) return;
  
	for (x = 0; x<65; x++) board[x] = EMPTY;
	        
	// Find Transcript in File

	startline  = -1; spacing = 0;

	while ( startline == -1 && !feof(input))
       {
       fgets ( textline, 1000 , input);
                 
		for (x = 0; x < strlen(textline); x++)
			{
			if ( (textline[x] > '1' && textline[x] <='9') || textline[x] == '-' )
				{  
				if (startline == -1) 
					{startline = x;
					for (y = x+2; y< strlen(textline); y++)
					if ( (textline[y] >= '0' && textline[y] <='9') )
						{spacing = y-x; y = 10000;}
					if (Tranline( textline, startline, spacing) == 0) startline = -1;       
					}
				}
			}
		} // end while
                

	while (!feof(input) && index <64 && startline!=-1)
    {
		 if ( index!=0 ) fgets ( textline, 1000 , input);

		if (strlen(textline) > unsigned (startline + spacing*7) )  
			for (x = startline; x< unsigned (startline + spacing*8); x+=spacing)
			{
				if ( textline[x] >= '0' && textline[x] <='9')
							board [ index ] = textline[x] -'0';
							else board[index] = 0;
		                      
				if ( textline[x+1] >= '0' && textline[x+1] <='9' )
				{
					board[ index ] *= 10;
					board[ index ] += textline[x+1] -'0';
				}
		              
				index ++;
			} // end for	           
   }

  fclose(input);

// Search for transcript in c4e3d4 etc... format on one line
	if (index <= 63)
	{
		input = fopen (fname, "rt");

		while (!feof(input) && index!=66)
		{
		   fgets ( textline, 1000 , input);

		   if (TranlineMoves ( textline ) == 1) 
				{index = 66;
			}
	}
	fclose (input);

	// See if moves from everywhere can be made into a transcript
	if (index!=66)
	{
		input = fopen (fname, "rt");

		index = 0;
		while (!feof(input) && index!=66) {
		   fgets ( textline, 1000 , input);
		   AccumulateTranlineMoves (textline, index, 0);
		}
		if (index> 16)  {
			AccumulateTranlineMoves (textline, index, 1);
			index = 66;
		}
		fclose (input);
	}


	}
    // Setup Transcript in c4e3d4 etc... format   
	else      
       {for (x = 0; x<65; x++) Transcript [x*2] = 0;

        for (x = 0; x <64; x++)
                { StartBoard[x] = EMPTY;
                if (board[x] > 0 && board[x] < 61) {Transcript[ board[x] * 2] = char ((x % 8) + 'a');
                                   Transcript[ board[x] * 2 + 1] = char  ((x / 8) + '1');
                                   }
                }
		}
 
	if (index > 63)
		{	
        Transcript [0] = 0; // Start at beginning of game
        Transcript [1] = 0;

    // Set StartBoard to Othello Standard
        
        StartBoard[ 3*8 + 3] = WHITE;
        StartBoard[ 3*8 + 4] = BLACK;
        StartBoard[ 4*8 + 3] = BLACK;
        StartBoard[ 4*8 + 4] = WHITE;
        StartBoard[64] = BLACK;
    
		memcpy (tBoard, StartBoard, 65);

		int errors = 0;
		for ( index = 1; index < 61 && FindMoves(tBoard, tBoard[64]) != 0; index++)
			if ( DoMove( Transcript[index * 2] -'a', Transcript[index * 2 + 1] -'1', tBoard , 0) == 0)
				errors = 1;
				
	     if (errors == 1) SetDlgItemText ( GameStats, 112, "Transcript Incomplete or has Errors.");
					 else SetDlgItemText ( GameStats, 112, "Transcript Read Successful.");
       
       }
       else
        {SetDlgItemText ( GameStats, 112, "No Transcripts Found");
        }

  }       

// --------------------
// Get a Line
// --------------------
void GetLineClip(int &index2, int length, char textline[], char cliptext[])
	{
	int y = 0;
	            
	do	{
		textline[y] = cliptext[index2];
		index2++;
		y++;
		}
	while (cliptext[index2-1] !='\n' && y < 1100 && index2<=length);
	                        
	textline[y] = NULL;
	}

// ===========================
// Read Transcript from ClipBoard 
// ===========================
void ReadTranscriptClipBoard (char *clip, int nNeededMoves)
	{
	char textline[1100];
	char buffer[80];
	char tBoard[65];
	int board[100];         
	char cliptext[20000];
	        
	int index = 0, index2 = 0;
	int sides = 0, length = 0;
	int startline = 0, spacing = 0;
	int transcriptnum = 0;
	unsigned int x, y = 0;
	      
	x = 0;
	while (clip[x]!=NULL && length < 20000)
		{length++;
		cliptext[x]=clip[x];
		x++;}

	for (x = 0; x<65; x++) board[x] = EMPTY;

	//  Find and Read Transcript from ClipBoard
	startline  = -1; spacing = 0;

	while ( startline == -1 && index2<length)
		{
		transcriptnum = 0;
		GetLineClip (index2, length, textline, cliptext);
	            
		for (x = 0; x < strlen(textline); x++)
				{
				if ( (textline[x] > '1' && textline[x] <='9') || textline[x] == '-' || textline[x] == '.' )
					{  
					if (startline == -1) {startline = x;
										for (y = x+2; y< strlen(textline); y++)
											if ( (textline[y] >= '0' && textline[y] <='9') || textline[y] == '.' )
												{spacing = y-x; y = 10000;
												}
										if (Tranline( textline, startline, spacing) == 0)
												startline = -1;
	                                                     
											else
											{ transcriptnum++;   
											if (transcriptnum != trannum) 
													{
													startline = -1;
													x+=  spacing*7;
													}
											}
	                                              
										} // end if.
					}
			}
		} // end while
	                

		while (index2 <length && index <64 && startline!=-1)
			{
			if ( index!=0 ) GetLineClip (index2, length, textline, cliptext);

			if (strlen(textline) > unsigned (startline + spacing*7) && Numbers(textline) == 1)  
				for (x = startline; x< unsigned (startline + spacing*8); x+=spacing)
					{
					if ( textline[x] >= '0' && textline[x] <='9')
								board [ index ] = textline[x] -'0';
								else board[index] = 0;
	                      
					if ( textline[x+1] >= '0' && textline[x+1] <='9' )
						{
						board[ index ] *= 10;
						board[ index ] += textline[x+1] -'0';
						}
	               
				index ++;
				} // end for	           
			}

	if (index <= 63)
	{// Now check for a line transcript of e3d2 etc. format
		index2 = 0;
		
		if (trannum<=1)
		{while (index2<length)
		{
	       
		GetLineClip (index2, length, textline, cliptext);
		if (TranlineMoves ( textline ) == 1) 
				{index = 66;
				index2 = length+1;
				}
		}
		// See if any transcript can be made from the moves found throughout the transcript.
		if (index!=66)
			{index = 0;
			index2 = 0;
			while (index2<length)
				{
				GetLineClip (index2, length, textline, cliptext);
				AccumulateTranlineMoves (textline, index, 0);
				}
			if (index> nNeededMoves)  
					{AccumulateTranlineMoves (textline, index, 1);
					index = 66;
					index2 = length+1;
					}
			}
		}
	}
	// Setup Transcript in c4e3d4 etc... format   
	else      
		{
		for (x = 0; x<65; x++) Transcript [x*2] = 0;
			for (x = 0; x <64; x++)
			{
				StartBoard[x] = EMPTY;
					if (board[x] > 0 && board[x] < 61) {
						Transcript[ board[x] * 2] = char ((x % 8) + 'a');
						Transcript[ board[x] * 2 + 1] = char  ((x / 8) + '1');
					}
			}
		}
	 
	if (index > 63)
			{	
			Transcript [0] = 0; // Start at beginning of game
			Transcript [1] = 0;

		// Set StartBoard to Othello Standard
			for ( int b = 0; b < 64; b++ )
				StartBoard[b] = EMPTY;

			StartBoard[ 3*8 + 3] = WHITE;
			StartBoard[ 3*8 + 4] = BLACK;
			StartBoard[ 4*8 + 3] = BLACK;
			StartBoard[ 4*8 + 4] = WHITE;
			StartBoard[64] = BLACK;
	    
			memcpy (tBoard, StartBoard, 65);

			int errors = 0;
			for ( index = 1; index < 61 && FindMoves(tBoard, tBoard[64]) != 0; index++)
				if ( DoMove( Transcript[index * 2] -'a', Transcript[index * 2 + 1] -'1', tBoard , 0) == 0)
					errors = 1;
					
			if (errors == 1)
				strcpy (textline, "Found Errors or Incomplete #");
			else
				strcpy (textline, "Transcript Read Successful. #");

			_ltoa (trannum, buffer, 10);
			strcat (textline, buffer);
			DisplayText( textline );
	        
		}
		else
			{strcpy (textline, "No Transcript Found. ");
			_ltoa (trannum, buffer, 10);
			strcat (textline, buffer);
			DisplayText( textline );
		}
	}        

// =============================
// Write Transcript to Disk
// =============================
void WriteTranscript (char fname[])
{
	 FILE *output;
	 char textline[180];
	 int tboard[65];
	 char buffer[15];
	 int i, x, y;
	 int Black = 0, White = 0;
	          
	 output = fopen (fname, "wt");

	 strcpy ( textline , "Pointy Stone 3 Othello Transcript \n\n");
	 fwrite ( textline , strlen(textline), 1 , output);

	 memset (tboard, 0, 65 * sizeof (int) );
	 for (i = 1; i < 65; i++)
	 {
		tboard[ (Transcript[i*2] - 'a') + (Transcript[i*2+1] - '1') * 8 ] = i;
		if (Transcript[i*2 + 2] == 0) break;
	 }

	 for (y = 0; y < 8; y++)
	 {
			 strcpy (textline, "  ");
	         
			 for (x = 0; x < 8; x++)
					 {
					 _itoa (tboard [x + y * 8], buffer, 10);
					 strcat (textline, " ");
	    
					 if ( ( x == 4 && y == 3 ) ||  ( x == 3 && y == 4 ) )
							strcat (textline, "B ");
					 else if  ( ( x == 3 && y == 3 ) ||  ( x == 4 && y == 4 ) )
							strcat (textline, "W ");
					 else
						  {if (tboard [x + y * 8] == 0) strcat (textline, "--");
						   else
							 {if (tboard [x + y * 8] < 10)  strcat (textline, "0");
								 strcat (textline, buffer) ;
							 }
						  }
					 strcat (textline, " ");
					 }

			 strcat (textline, "\n");
	                               
			 fwrite (textline, strlen(textline), 1, output);
	}

	GetDiscs ( Black, White);

	strcpy ( textline, "\n\n Black: ");
	strcat ( textline, BlackPlayer);
	strcat ( textline, " ");
	_itoa ( Black, buffer, 10);
	strcat ( textline, buffer);
	strcat ( textline, "\n White: ");
	strcat ( textline, WhitePlayer);
	strcat ( textline, " ");
	_itoa ( White, buffer, 10);
	strcat ( textline, buffer);
	strcat ( textline, "\n ");
	fputs  ( textline , output);

	fputs ( "\nMoves : \n", output);
	fputs(&Transcript[2], output);

	fclose (output);
}