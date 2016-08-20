#ifdef GCC
#include "engine.h"

int Comp(t_Move *,t_Move *);
int IsInCheck(t_Board ,t_Turn);
int IsKingAttacked(t_Board,t_Turn);
int LookupTT(int *, t_Move *, int *,int *, int *, int *, int);
int StoreInTT(int, t_Move, int,int,int,int,int);
void ClearTT(void);
void GeneratePawnMoves(t_Board,t_SquareIndex,t_Move *,int *,t_Turn,int,int);
void GeneratePawnCaptures(t_Board,t_SquareIndex,t_Move *,int *,t_Turn,int,int);
void GenerateEnPassantCaptures(t_Board,t_SquareIndex,t_Move *,int *,t_Turn,int,int);
void GenerateKnightMoves(t_Board,t_SquareIndex,t_Move *,int *,t_Turn,int,int);
void GenerateKnightCaptures(t_Board,t_SquareIndex,t_Move *,int *,t_Turn,int,int);
void GenerateBishopMoves(t_Board,t_SquareIndex,t_Move *,int *,t_Turn,int,int);
void GenerateBishopCaptures(t_Board,t_SquareIndex,t_Move *,int *,t_Turn,int,int);
void GenerateRookMoves(t_Board,t_SquareIndex,t_Move *,int *,t_Turn,int,int);
void GenerateRookCaptures(t_Board,t_SquareIndex,t_Move *,int *,t_Turn,int,int);
void GenerateKingMoves(t_Board,t_SquareIndex,t_Move *,int *,t_Turn ,int,int);
void GenerateKingCaptures(t_Board,t_SquareIndex,t_Move *,int *,t_Turn,int,int);
void GenerateCastleMoves(t_Board,t_SquareIndex,t_Move *,int *,t_Turn,int,int);
int GenerateMoves(t_Board,t_Turn,t_Move *,int,int);
int GenerateCaptures(t_Board,t_Turn,int,t_Move *);
void ClearKiller(void);
int Evaluate(t_Board,t_Turn);
int QuiescenseSearch(t_Board,t_Turn,int,int,int,int,t_Move best_line[]);
int AlphaBetaWithMemory(t_Board,t_Turn,int,int,int,int,t_Move best_line[]);
int MTDF(t_Board pos,t_Turn turn,int ply,int depth,int f,t_Move best_line[]);
int NegaScout (t_Board pos,t_Turn turn,int ply,int depth,int alpha,int beta,int do_null_move,t_Move best_line[]);
int AlphaBeta(t_Board pos,t_Turn turn,int ply,int depth,int alpha,int beta,int do_null_move,t_Move best_line[]);
int SearchMove(t_Board pos,t_Move *move,int max_depth,t_Turn turn);
unsigned int Random32(void);
BITBOARD Random64(void);
void InitRandomHash(void);
void InitTTable(void);
void FreeTTable(void);
void Initialize(t_Board pos,t_Turn *turn);
void Finish(void);
void MakeMove(t_Board pos,t_Move *move,int depth,t_Turn turn);
void UnMakeMove(t_Board pos,t_Move *move,int depth,t_Turn turn);
int IsLegalMove(t_Board pos,t_Move move,t_Turn last_turn);
int ParseMove(char *s,t_Move *move,t_Board pos,t_Turn turn);
void PrintBoard(t_Board pos, t_Turn side);









#endif
