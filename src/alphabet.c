/***************************************************************************
                          alphabet.c 
 -  description: Init SDL
                             -------------------
    begin                : Jan 6 2003
    copyright            : (C) 2003 by Jesse Andrews
    email                : jdandr2@tux4kids.net
 
    Modified for use in tuxmath by David Bruce - 2006.
    email                : <dbruce@tampabay.rr.com>grep
                           <tuxmath-devel@lists.sourceforge.net> 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "tuxmath.h"
#include "titlescreen.h"

/* the colors we use throughout the game */

SDL_Color black;
SDL_Color gray;
SDL_Color dark_blue;
SDL_Color red;
SDL_Color white;
SDL_Color yellow;

/* Used for word list functions (see below): */
static int WORD_qty;
unsigned char WORDS[MAX_NUM_WORDS][MAX_WORD_SIZE+1];

/* --- setup the alphabet --- */
void set_letters(unsigned char *t) {
	int i;

	ALPHABET_SIZE = 0;
	for (i=0; i<256; i++)
		ALPHABET[i]=0;

	for (i=0; i<strlen(t); i++)
		if (t[i]!=' ') {
			ALPHABET[(int)t[i]]=1;
			ALPHABET_SIZE++;
		}
}

void clear_keyboard( void ) {
	int i,j;

	ALPHABET_SIZE = 0;
	for (i=0; i<256; i++) {
		ALPHABET[i]=0;
		for (j=0; j<10; j++)
			FINGER[i][j]=0;
		KEYMAP[i]=i;
	}
}

void LoadKeyboard( void ) {
	unsigned char fn[PATH_MAX];
	int l;

	clear_keyboard();

	for (l=useEnglish; l<2; l++) {
		sprintf( fn , "%s/keyboard.lst", realPath[l]);
		if ( checkFile(fn) ) {
			unsigned char str[255];
			FILE *f;
			int i,j;

			f = fopen( fn, "r" );

			if (f == NULL)
				continue;

			do {
				fscanf( f, "%[^\n]\n", str);
				if (strlen(str) > 3) {

					/* format is: FINGER(s)|Char(s) Upper/Lower */

					/* advance past the fingers */

					for (i=0; i<strlen(str) && str[i] != '|'; i++);

					i++; // pass the '|'
					j = i; 
					ALPHABET[(int)str[j]] = 1;  // first character is default

					for (i++; i<strlen(str); i++)
						KEYMAP[(int)str[i]] = str[j];

					/* set the fingers for this letter */

					for (i=0; i<j-1; i++)
						if (str[i]>='0' && str[i]<='9')
							FINGER[str[j]][(int)(str[i]-'0')]=1;

					ALPHABET_SIZE++;
				}

			} while (!feof(f));

			fclose(f);

			return;
		}
	}

	fprintf( stderr, "Error finding file for keyboard setup!\n" );
}


/* black_outline() creates a surface containing text of the designated */
/* foreground color, surrounded by a black shadow, on a transparent    */
/* background.  The appearance can be tuned by adjusting the number of */
/* background copies and the offset where the foreground text is       */
/* finally written (see below).                                        */
SDL_Surface* black_outline(unsigned char *t, TTF_Font *font, SDL_Color *c)
{
  SDL_Surface* out = NULL;
  SDL_Surface* black_letters = NULL;
  SDL_Surface* white_letters = NULL;
  SDL_Surface* bg = NULL;
  SDL_Rect dstrect;
  Uint32 color_key;

  if (!t || !font || !c)
  {
    fprintf(stderr, "black_outline(): invalid ptr parameter, returning.");
    return NULL;
  }

#ifdef TUXMATH_DEBUG
  fprintf( stderr, "\nEntering black_outline(): \n");
  fprintf( stderr, "black_outline of \"%s\"\n", t );
#endif

  black_letters = TTF_RenderUTF8_Blended(font, t, black);

  if (!black_letters)
  {
    fprintf (stderr, "Warning - black_outline() could not create image for %s\n", t);
    return NULL;
  }

  bg = SDL_CreateRGBSurface(SDL_SWSURFACE,
                            (black_letters->w) + 5,
                            (black_letters->h) + 5,
                             32,
                             rmask, gmask, bmask, amask);
  /* Use color key for eventual transparency: */
  color_key = SDL_MapRGB(bg->format, 01, 01, 01);
  SDL_FillRect(bg, NULL, color_key);

  /* Now draw black outline/shadow 2 pixels on each side: */
  dstrect.w = black_letters->w;
  dstrect.h = black_letters->h;

  /* NOTE: can make the "shadow" more or less pronounced by */
  /* changing the parameters of these loops.                */
  for (dstrect.x = 1; dstrect.x < 4; dstrect.x++)
    for (dstrect.y = 1; dstrect.y < 3; dstrect.y++)
      SDL_BlitSurface(black_letters , NULL, bg, &dstrect );

  SDL_FreeSurface(black_letters);

  /* --- Put the color version of the text on top! --- */
  white_letters = TTF_RenderUTF8_Blended(font, t, *c);
  dstrect.x = 1;
  dstrect.y = 1;
  SDL_BlitSurface(white_letters, NULL, bg, &dstrect);
  SDL_FreeSurface(white_letters);

  /* --- Convert to the screen format for quicker blits --- */
  SDL_SetColorKey(bg, SDL_SRCCOLORKEY|SDL_RLEACCEL, color_key);
  out = SDL_DisplayFormatAlpha(bg);
  SDL_FreeSurface(bg);

#ifdef TUXMATH_DEBUG
  fprintf( stderr, "\nLeaving black_outline(): \n");
#endif

  return out;
}

void show_letters( void ) {
	int i, l=0;
	SDL_Surface *abit;
	SDL_Rect dst;
	int stop = 0;
	unsigned char t[255];

	for (i=0; i<256; i++)
		if (ALPHABET[i])
			t[l++]=i;

	t[l] = 0;

	abit = black_outline(t, default_font, &white);

	dst.x = 320 - (abit->w / 2);
	dst.y = 275;
	dst.w = abit->w;
	dst.h = abit->h;

	SDL_BlitSurface(abit, NULL, screen, &dst);

	SDL_FreeSurface(abit);

	abit = black_outline("Alphabet Set To:", default_font, &white);
	dst.x = 320 - (abit->w / 2);
	dst.y = 200;
	dst.w = abit->w;
	dst.h = abit->h;

	SDL_BlitSurface(abit, NULL, screen, &dst);

	SDL_UpdateRect(screen, 0, 0, 0 ,0);

	while (!stop) 
		while (SDL_PollEvent(&event)) 
			switch (event.type) {
				case SDL_QUIT:
					exit(0);
				case SDL_KEYDOWN:
				case SDL_MOUSEBUTTONDOWN:
					stop = 1;
			}

	SDL_FreeSurface(abit);
}

/* --- get a letter --- */
unsigned char get_letter(void) {
	static int last = -1; // we don't want to return same letter twice in a row
	int letter;
	do {
		letter = rand() % 255;
	} while ((letter == last && ALPHABET_SIZE > 1) || ALPHABET[letter] == 0);

	last = letter;

	return letter;
}

/******************************************************************************
*                           WORD FILE & DATA STRUCTURE                        *
******************************************************************************/



/* WORDS_init: clears the number of words
 */
void WORDS_init( void ) {
	WORD_qty = 0;
}

/* WORDS_use_alphabet: setups the WORDS so that it really
 * returns a LETTER when WORDS_get() is called
 */
void WORDS_use_alphabet( void ) {
	int i;

	LOG("Entering WORDS_use_alphabet()\n");

	WORD_qty = 0;
	/* This totally mucks up i18n abilities :( */
	for (i=65; i<90; i++) 
	{
		if (ALPHABET[i]) {
			WORDS[WORD_qty][0] = (unsigned char)i;
			WORDS[WORD_qty][1] = '\0';
			WORD_qty++;

			DEBUGCODE { fprintf(stderr, "Adding %c\n", (unsigned char)i); }
		}
	}
	/* Make sure list is terminated with null character */
	WORDS[WORD_qty][0] = '\0';

	DOUT(WORD_qty);
	LOG("Leaving WORDS_use_alphabet()\n");
}

/* WORDS_get: returns a random word that wasn't returned
 * the previous time (unless there is only 1 word!!!)
 */
unsigned char* WORDS_get( void ) {
	static int last_choice = -1;
	int choice;

	LOG("Entering WORDS_get()\n");
	DEBUGCODE { fprintf(stderr, "WORD_qty is: %d\n", WORD_qty); }

	/* Now count list to make sure WORD_qty is correct: */

	WORD_qty = 0;
	while (WORDS[WORD_qty][0] != '\0')
	{
	  WORD_qty++;
	}

	DEBUGCODE { fprintf(stderr, "After count, WORD_qty is: %d\n", WORD_qty); }

        if (0 == WORD_qty)
	{
	  LOG("No words in list\n");
          return NULL;
	}

        if (WORD_qty > MAX_NUM_WORDS)
	{
	  LOG("Error: WORD_qty greater than array size\n");
          return NULL;
	}

        if (WORD_qty < 0)
	{
	  LOG("Error: WORD_qty negative\n");
          return NULL;
	}

	do {
		choice = (rand() % WORD_qty);
	} while ((choice == last_choice) || (WORD_qty < 2));

	last_choice = choice;

	DEBUGCODE { fprintf(stderr, "Selected word is: %s\n", WORDS[choice]); }
	return WORDS[choice];
}



/* WORDS_use: adds the words from a given wordlist
 * it ignores any words too long or that has bad
 * character (such as #)
 */
void WORDS_use( char *wordFn ) {
	int j;
	unsigned char temp_word[PATH_MAX];
	FILE *wordFile=NULL;

	DEBUGCODE { fprintf(stderr, "Entering WORDS_use() for file: %s\n", wordFn); }

	WORD_qty = 0;

	/* --- open the file --- */

	wordFile = fopen( wordFn, "r" );

	if ( wordFile == NULL ) {
		fprintf(stderr, "ERROR: could not load wordlist: %s\n", wordFn );
		fprintf(stderr, "Using ALPHABET instead\n");
		WORDS_use_alphabet( );
		return;
	}

	
	/* --- load words for this curlevel ---  */


	DEBUGCODE { fprintf(stderr, "WORD FILE OPENNED @ %s\n", wordFn); }

	/* ignore the title */
	fscanf( wordFile, "%[^\n]\n", temp_word);

	while (!feof(wordFile) && (WORD_qty < MAX_NUM_WORDS)) {
		fscanf( wordFile, "%[^\n]\n", temp_word);

		for (j = 0; j < strlen(temp_word); j++)
			if (temp_word[j] == '\n' || temp_word[j] == '\r')
				temp_word[j] = '\0';

		/* --- check its size, if too big, ignore it --- */

		if (strlen(temp_word) >= 1 && strlen(temp_word) <= MAX_WORD_SIZE) {

			/* --- add word --- */
			if (WORD_qty < MAX_NUM_WORDS)
			{
				strcpy( WORDS[WORD_qty], temp_word );
				WORD_qty++;
			}
		}
	}
        
	/* Make sure list is terminated with null character */
	WORDS[WORD_qty][0] = '\0';

	DOUT(WORD_qty);

	if (WORD_qty == 0)
		WORDS_use_alphabet( );

	fclose(wordFile);

	LOG("Leaving WORDS_use()\n");
}