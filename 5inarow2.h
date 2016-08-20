
#ifndef INAROW2_H
#define INAROW2_H

#define		FULL		1

#define 	OPP 		0
#define 	COM 		1

#define		OPPWINS		1
#define		COMWINS		2
#define		DRAW		3
#define		NOWIN		0

#define		BOARDX		10
#define		BOARDY		10

#define		CONNECTED	4
#define		GOODSQUARE	3
#define		BADSQUARE	1
#define		INFINITY	10000

#define		CWSCORE		10000
#define		OWSCORE		-10000

#define		WINS		195
#define		WINNUM		32

#define		MAX_DEPTH	9
#define		MAX_QUIESCE_DEPTH	2
#define		DEFAULT_SCORE_SHIFT_CONNECTIONS 2

#ifndef	MAX_TREEWIDTH
#define		MAX_TREEWIDTH	TOTALSQS
#endif

#define		MAX_PVSIZE	30		/* Make it MAX_DEPTH + a few more entries */

#define		ALFAERROR	-INFINITY
#define		BETAERROR	+INFINITY

#define		DECREASE	1
#define		INCREASE	2

#define		DEFAULT_DEPTH	4

#define 	CENTER1		(SIZE/2-1)
#define		CENTER2		(SIZE/2)

#ifdef TTABLE
#define normal_move 1
#define fail_high	2
#define fail_low	3
#define         TTSIZE          131072
#endif

#define 		NODES_BETWEEN_CHECKMESSAGE	500
#define			NODES_BETWEEN_PRINTNODES	17487



typedef struct {
	int move;
	int score;
} NODE;

typedef char Boolean;

typedef int BOARD[DEFAULT_SIZE][DEFAULT_SIZE];

struct MVS {
	int mvs;
	int moveid;
};

#ifdef TTABLE
typedef __int64 BITBOARD;
typedef struct _TABLE_ENTRY {
	int depth;
	int move;
	int score;
	int upperbound, lowerbound;
	unsigned short flag;
	int type;
	BITBOARD bd;			/* Position secondary hash key
							   to avoid collition */
	BITBOARD hash_key;		/* Table entry hash key */
} TABLE_ENTRY;
#endif

typedef struct {
	int *win_index;
	int places;
} SQ_DATA;
typedef struct {
	int move;
	int piece;
} MOVE;

typedef struct {
	int move;
} PVMOVE;

typedef struct {
	int move;
	int piece;
	int opp_time;
	int com_time;
} TIMED_MOVE;
int PlaceMove(MOVE,int), WhoStarts(), Comp(const NODE *a, const NODE *b);
int Search(MOVE *,int,int);
int NegaScout(int,int,int,int,PVMOVE best_line[]);
int IterativeDeepening(MOVE *,int,int,long int);
long int ComputeSearchTime(long int);
int GenerateMoves(NODE *);
int TestWin(void), Connects(BOARD,int);
int Isolated(int), num_of_win_places(int,int,int);
int IsDraw(void);
int IsEmpty(int);
int GetPieceNbr(void);
void GetWinCoords(int *x1, int *y1, int *x2, int *y2);
int LegalMove(int sq);
int GetSquare(int sq);
void ShowNodeCnt (long int NodeCnt, float nps);
void ShowQuiesceNodeCnt(long int NodeCnt);
void ShowSearchedMove(int,int,int,int,int);
void ShowBestSearchedMove(int,int,int);
void ShowPV(int depth, int level, PVMOVE prvar[]);
void ShowScore (short int score);
void ShowMessage (char *s);
void CheckMessage(void);
void CheckAndLeaveMessage(void);
void WaitForMessage(void);
void InitSqArray(void);
void StartTimer(void);
void StopTimer(void);
void SetClockTime(long int time_num, int player);
void Flag(int player);
short int *Curr_Board(void);
void Clear(), MainMenu(), StartGame(), Help(), PrintXY(), PrintBoard(), End(void);
void Getstr(void), PrintResult(void), InitWinArray(void), update_score(int,int), push_state(void);
void TwoCompGame(void), InitGame(void), EndGame(void), Sort(int, MOVE *, int *, int);
MOVE OppMove(BOARD), CompMove(int,int);
MOVE MakeMoves(BOARD,BOARD);
void MakeMove(int, int);
void UnmakeMove(void);
void Order(int);
#ifdef TTABLE
void InitRandomHash(void);
void InitTTable(void);
BITBOARD Random64(void);
unsigned int Random32(void);

int StoreInTT(int, PVMOVE, int,int, int, int, int);
int LookupTT(int *, PVMOVE *, int *,int *, int *, int *, int);
void ClearTT(void);
void UpdateHashKeys(int,int);
#endif
#ifdef QUIESCE
int Quiesce(int level,int player,int score,PVMOVE best_line[]);
int QuiesceGenerate(int player, NODE *mvarr);
int IsThreat(int player, int move);
#endif

typedef struct {
	 short int *board;
	 short int *(score_array[2]);
	 short int score[2];
	 short int winner;
	 short int num_of_pieces;
#ifdef TTABLE
	 BITBOARD hash_key;
	 BITBOARD bd;
#endif
#ifdef QUIESCE
	 short int max_connections[2];
	 short int *(connections[2]);
#endif

} Game_state;
#endif
