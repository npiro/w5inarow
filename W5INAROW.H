#ifndef GOMOKU_H
#define GOMOKU_H
#define		EMPTY						-2
#ifndef BOARDSIZE
#define		DEFAULT_SIZE				19
#else
#define		DEFAULT_SIZE				BOARDSIZE
#endif
#define		TOTALSQS					SIZE*SIZE
#ifndef DEFAULT_MAINWIN_WIDTH
#define		DEFAULT_MAINWIN_WIDTH	418
#endif
#define		TICK_SIZE      MAINWIN_WIDTH/SIZE
#ifndef DEFAULT_CONNECTS
#define		DEFAULT_CONNECTS			5
#endif

#define		ST_COORDX	  	50
#define     ST_COORDY		50

#define 		SIZE				size
#define		CONNECTS			connects
#define     MAINWIN_WIDTH	mainwin_width
#define		TOTAL_MAINWIN_WIDTH			540
#define		TOTAL_MAINWIN_HEIGHT		585
#define 	DEFAULT_WIN_TYPE			large
#define 	DEFAULT_FLASH_DELAY			100		/* Miliseconds between flashes */

#define		DEFAULT_SEARCH_TIME			10000L	/* Miliseconds per search */

#define		DEFAULT_INITIAL_CLOCK_TIME	180000L	/* Miliseconds in initial clock */

typedef struct {
	int lowestx;
	int lowesty;
	int highestx;
	int highesty;
} BoardSq_T;

typedef struct {
	short int post;
	short int edit;
	short int quit;
	short int havewinner;
	short int both;
	short int compmove;
	short int force;
	short int flash;
	short int flashing;
	short int standard_gomoku;
	short int sound;
	short int mididev;
	short int music;
	short int search_extensions;
	short int newticks;
	short int iterative_deepening;
	short int timed_game;
	short int ttable;
	short int savepv;
	short int opening_game;
} FLAGS;
#endif
