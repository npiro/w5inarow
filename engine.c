#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef GETTIMEOFDAY
#ifdef FTIME
#include <time.h>
#include <sys\timeb.h>
#else
#include <time.h>
#include "gettime.h"
#endif
#else
#include <time.h>
#endif
#include <math.h>

#include "w5inarow.h"
#include "5inarow2.h"
#include "macro.h"

#ifdef DEFAULT_MAX_TREEWIDTH
#undef 		MAX_TREEWIDTH
#define		MAX_TREEWIDTH	max_treewidth
#endif

int max_treewidth;

extern int size, connects, score_shift_connections;
extern int move_num;
extern long search_time, opp_remaining_time, com_remaining_time;
extern long game_time;
extern FLAGS flag;
static int states_allocated=0, tdepth, /*winnum,*/ same_score_moves;
static Game_state state_stack[MAX_DEPTH+30];
static Game_state *current_state;
static TIMED_MOVE game_table[19*19];
static NODE moveorder[DEFAULT_SIZE*DEFAULT_SIZE];
static int searchdepth, quit;
static Boolean ***map;
static SQ_DATA *bd_index;
static long nodes, quiesce_nodes;
static int winnum, onetowinnum, twotowinnum;
static int win_places_array_size;
int wehaveawinner, lastx, lasty, lastxall, lastyall;
int wins;
int MAINWIN_WIDTH;
#ifdef TTABLE
TABLE_ENTRY *ptrtoTT[2];
BITBOARD xrandom[19*19], orandom[19*19], xrandom2[19*19], orandom2[19*19], hash_mask;
long tt_probes, tt_hits, tt_cutoff, transposition_ordered;
#endif
/* Default settings. You can modify them */

void InitGame()
{
	int i;

	InitWinArray();
	quit = 0;
	searchdepth=DEFAULT_DEPTH;
	opp_remaining_time = game_time;
	com_remaining_time = game_time;
	if (flag.timed_game && !flag.edit) {
		StartTimer();
		SetClockTime(opp_remaining_time,OPP);
		SetClockTime(com_remaining_time,COM);
	}
	wehaveawinner=0;
	max_treewidth = DEFAULT_MAX_TREEWIDTH;
	if (max_treewidth > TOTALSQS) max_treewidth = TOTALSQS;

	lastx=-100; lasty=-100;
	lastxall=-100; lastyall=-100;

	current_state = &state_stack[0];
	winnum = 1 << score_shift_connections*CONNECTS;
	onetowinnum = winnum >> score_shift_connections;
	twotowinnum = onetowinnum >> score_shift_connections;
	same_score_moves = 0;
	win_places_array_size = wins * sizeof(short int);
	current_state->board = (short int *) malloc(TOTALSQS * sizeof(short int));
    for (i=0; i<TOTALSQS; i++)
        current_state->board[i] = EMPTY;

        /* Set up the score array */

	current_state->score_array[0] = (short int *) malloc(wins * sizeof(short int));
	current_state->score_array[1] = (short int *) malloc(wins * sizeof(short int));
	for (i=0; i<wins; i++) {
		current_state->score_array[0][i] = 1;
		current_state->score_array[1][i] = 1;
	}
	/*current_state->isolated = (short int *) malloc(TOTALSQS*sizeof(short int));
	for (i=0;i<TOTALSQS;i++) current_state->isolated = 0;*/
#ifdef QUIESCE
	/* Allocate connections array */

	current_state->connections[0] = (short int *) malloc(wins * sizeof(short
 int));
	current_state->connections[1] = (short int *) malloc(wins * sizeof(short
 int));
	for (i=0; i<wins; i++) {
		current_state->connections[0][i] = 0;
		current_state->connections[1][i] = 0;
	}
#endif


	current_state->score[0] = current_state->score[1] = wins;
	current_state->winner = EMPTY;
	current_state->num_of_pieces = 0;
#ifdef QUIESCE
	current_state->max_connections[0] = 0;
	current_state->max_connections[1] = 0;
#endif
	states_allocated = 1;
#ifdef TTABLE
	InitTTable();
#endif


	/* Initialize the move order array for better alpha-beta pruning */
	/* First Initialize moveorder array for the first move Search */
	for (i=0;i<DEFAULT_SIZE*DEFAULT_SIZE;i++) {
		moveorder[i].move=i;
		moveorder[i].score=0;
	}

	move_num=0;
}

void EndGame()
{
        int i, j;
    /* Free up the memory of all the states used. */

    for (i=0; i<states_allocated; i++) {
        free(state_stack[i].board);
        free(state_stack[i].score_array[0]);
        free(state_stack[i].score_array[1]);
#ifdef QUIESCE
		free(state_stack[i].connections[0]);
		free(state_stack[i].connections[1]);
#endif
		/*free(state_stack[i].isolated);*/
	}
    states_allocated = 0;

	StopTimer();
    for (i=0; i<SIZE; i++) {
        for (j=0; j<SIZE; j++)
            free(map[i][j]);
        free(map[i]);
    }
    free(map);

	for (i=0;i<TOTALSQS;i++)
		free(bd_index[i].win_index);
	free(bd_index);
#ifdef TTABLE
    free(ptrtoTT[0]);
	free(ptrtoTT[1]);
#endif

}

void Order(int player)
{
	int i;
	for (i=0;i<TOTALSQS;i++) {
		moveorder[i].move=i;
		push_state();
		update_score(Other(player),i);
		if (Winner != EMPTY && Board[i] == EMPTY) {
			moveorder[i].score=CWSCORE;
			pop_state();
			continue;
		}
		else {
			moveorder[i].score=-3*Evaluate(player);
		}
		pop_state();
		push_state();
		update_score(player,i);
		if (Board[i] != EMPTY) {
			moveorder[i].score=-500;
			pop_state();
			continue;
		}

		if (Winner != EMPTY)
			moveorder[i].score=CWSCORE;
		else {
			moveorder[i].score+=Evaluate(player)-Evaluate(Other(player));
			if (!Isolated(i) || Centered(i)) {
				moveorder[i].score+=10;
				pop_state();
				continue;
			}
		}

		pop_state();
	}

	qsort((void *)moveorder,TOTALSQS,sizeof(moveorder[0]),(int (*)(const void *,const void *))Comp);

}

int num_of_win_places(int x, int y, int n)
{
	 if (x < n && y < n)
		  return 0;
	 else if (x < n)
        return x * ((y-n)+1);
    else if (y < n)
        return y * ((x-n)+1);
    else
        return 4*x*y - 3*x*n - 3*y*n + 3*x + 3*y - 4*n + 2*n*n + 2;
}

void InitWinArray(void)
{
	int i, j, k, count, num_to_connect=CONNECTS;
        wins=num_of_win_places(SIZE,SIZE,num_to_connect);

    	map = (Boolean ***) malloc(SIZE * sizeof(Boolean **));
    	for (i=0; i<SIZE; i++) {
        	map[i] = (Boolean **) malloc(SIZE * sizeof(Boolean *));
        	for (j=0; j<SIZE; j++) {
            		map[i][j] = (Boolean *) malloc(wins * sizeof(Boolean));
            		for (k=0; k<wins; k++)
                	map[i][j][k] = 0;
        	}
    	}

        count = 0;

        /* Fill in the horizontal win positions */
        for (i=0; i<SIZE; i++)
                for (j=0; j<SIZE-num_to_connect+1; j++) {
                        for (k=0; k<num_to_connect; k++)
                                map[j+k][i][count] = 1;
                        count++;
                }
        /* Fill in the vertical win positions */
        for (i=0; i<SIZE; i++)
                for (j=0; j<SIZE-num_to_connect+1; j++) {
                        for (k=0; k<num_to_connect; k++)
                                map[i][j+k][count] = 1;
                        count++;
                }

        /* Fill in the forward diagonal win positions */
        for (i=0; i<SIZE-num_to_connect+1; i++)
                for (j=0; j<SIZE-num_to_connect+1; j++) {
                        for (k=0; k<num_to_connect; k++)
                                map[j+k][i+k][count] = 1;
                        count++;
                }

        /* Fill in the backward diagonal win positions */
        for (i=0; i<SIZE-num_to_connect+1; i++)
                for (j=SIZE-1; j>=num_to_connect-1; j--) {
                        for (k=0; k<num_to_connect; k++)
                                map[j-k][i+k][count] = 1;
                        count++;
                }

	bd_index = (SQ_DATA *) malloc(TOTALSQS*sizeof(SQ_DATA));
	for (i=0;i<TOTALSQS;i++) {
		bd_index[i].win_index = (int *) malloc(num_to_connect*4*sizeof(int));
	}

	for (i=0;i<size;i++) for (j=0;j<size;j++) {
		count = 0;
		for (k=0;k<wins;k++) {
			if (map[i][j][k]) {
				if (count >= 4*num_to_connect) {
					ShowMessage("Error: Too many places");
					return;
				}
				bd_index[j*size+i].win_index[count] = k;
				count++;
			}
		}
		bd_index[j*size+i].places = count;
	}
}
void update_score(int player, int move)
{
	register int i, index;
	int this_difference = 0, other_difference = 0;
#ifdef QUIESCE
#ifdef MAP_CHECK
	int current_max_connections=0;
#else
	int current_max_connections=current_state->max_connections[player];
#endif
#endif

	short int **current_score_array = current_state->score_array;
	int other_player = Other(player);

#ifdef MAP_CHECK
	for (i=0; i<wins; i++) {
		if (map[MOVEX(move)][MOVEY(move)][i]) {
			this_difference += current_score_array[player][i];
			other_difference += current_score_array[other_player][i];
			current_score_array[player][i] <<= score_shift_connections;
			current_score_array[other_player][i] = 0;
#ifdef QUIESCE
			/*current_state->connections[player][i]++;
			current_state->connections[other_player][i] = 0;*/
#endif

			this_difference -= current_score_array[player][i];

			if (current_score_array[player][i] == winnum) {
				if (current_state->winner == EMPTY) {
					current_state->winner = player;
				}
			}
		}
#ifdef QUIESCE
		if (flag.search_extensions) {
			current_max_connections=Max(current_max_connections,current_score_array[player][i]);
		}
#endif
    }
#else
	for (i=0;i<bd_index[move].places;i++) {
		index = bd_index[move].win_index[i];
		this_difference += current_score_array[player][index];
		other_difference += current_score_array[other_player][index];
		current_score_array[player][index] <<= score_shift_connections;
		current_score_array[other_player][index] = 0;
#ifdef QUIESCE
			/*current_state->connections[player][i]++;
			current_state->connections[other_player][i] = 0;*/
#endif

		this_difference -= current_score_array[player][index];

		if (current_score_array[player][index] == winnum) {
			if (current_state->winner == EMPTY) {
				current_state->winner = player;
			}
		}
	}
#ifdef QUIESCE
	if (flag.search_extensions) {
		current_max_connections=Max(current_max_connections,current_score_array[player][index]);
	}
#endif

#endif
#ifdef QUIESCE
	if (flag.search_extensions)
		current_state->max_connections[player] = current_max_connections;
#endif
	current_state->score[player] -= this_difference;
	current_state->score[other_player] -= other_difference;
}


int TestWin()
{
	if (current_state->winner == OPP) return OPPWINS;
	if (current_state->winner == COM) return COMWINS;
	if (PieceNbr == TOTALSQS) return DRAW;
	return NOWIN;

}
int Comp(const NODE *a, const NODE *b) {
		  if (a->score<b->score) return (1);
		  if (a->score>b->score) return (-1);
		  if (a->score==b->score) return (0);
		  return 0;
}
int Isolated(int move)
{
#ifdef NEWISOLATED
	int i, j, count, x = MOVEX(move), y = MOVEY(move);
	count=0;
	for (i=-1;i<=1;i++) for (j=-1;j<=1;j++)
		if (i != 0 || j != 0) {
			if (x+i>-1 && y+j>-1 && x+i <= size && y+j <= size) {
				if (Board[move+(i)+size*(j)] == EMPTY)
					count++;
			}
			else count++;
		}
	if (count == 8) return (1);
	else return (0);

	return (1);

#else
	int x, y;
	x = MOVEX(move);
	y = MOVEY(move);
        if (x != 0 && x != SIZE-1 && y != 0 && y != SIZE-1) {
                if (Board[move+1] == EMPTY && Board[move-1] == EMPTY && Board[move+size] == EMPTY && Board[move-size] == EMPTY && Board[move+size+1] == EMPTY && Board[move-size+1] == EMPTY && Board[move+size-1] == EMPTY && Board[move-size-1] == EMPTY)
                        return (1);
                else
                        return (0);
        }
        else if (x == 0 && y != 0 && y != SIZE-1) {
                if (Board[move+1] == EMPTY && Board[move+size+1] == EMPTY && Board[move-size+1] == EMPTY && Board[move-size] == EMPTY && Board[move+size] == EMPTY)
                        return (1);
                else
                        return (0);
        }
        else if (x == SIZE-1 && y != 0 && y != SIZE-1) {
                if (Board[move-1] == EMPTY && Board[move+size-1] == EMPTY && Board[move-size-1] == EMPTY && Board[move-size] == EMPTY && Board[move+size] == EMPTY)
                        return (1);
                else
                        return (0);
        }
        else if (y == 0 && x != 0 && x != SIZE-1) {
                if (Board[move+1] == EMPTY && Board[move+size+1] == EMPTY && Board[move+size] == EMPTY && Board[move+size-1] == EMPTY && Board[move-1] == EMPTY)
                        return (1);
                else
                        return (0);
        }
        else if (y == SIZE-1 && x != 0 && x != SIZE-1) {
                if (Board[move+1] == EMPTY && Board[move-size+1] == EMPTY && Board[move-size] == EMPTY && Board[move-size-1] == EMPTY && Board[move-1] == EMPTY)
                        return (1);
                else
                        return (0);
        }
        else if (x == 0 && y == 0) {
                if (Board[move+1] == EMPTY && Board[move+size+1] == EMPTY && Board[move+size] == EMPTY)
                        return (1);
                else
                        return (0);
        }
        else if (x == SIZE-1 && y == 0) {
                if (Board[move-1] == EMPTY && Board[move+size-1] == EMPTY && Board[move+size] == EMPTY)
                        return (1);
                else
                        return (0);
        }
        else if (x == 0 && y == SIZE-1) {
                if (Board[move+1] == EMPTY && Board[move-size+1] == EMPTY && Board[move-size] == EMPTY)
                        return (1);
                else
                        return (0);
        }
        else if (x == SIZE-1 && y == SIZE-1) {
                if (Board[move-1] == EMPTY && Board[move-size-1] == EMPTY && Board[move-size] == EMPTY)
                        return (1);
                else
                        return (0);
		  }
		  return (1);
#endif
}

int IsDraw()
{
        return (current_state->num_of_pieces == TOTALSQS);
}

MOVE CompMove(int depth, int player)
{
	MOVE mv;
	int score;
	float nps, elapsed;
	char str[30];
	float st, end;

	nodes = 0;
	quiesce_nodes = 0;

#ifdef TTABLE
	tt_probes = 0;
	tt_hits = 0;
	tt_cutoff = 0;
	transposition_ordered=0;
#endif

	st = GetTime();
	Order(player);
	if (flag.iterative_deepening) {
		if (flag.timed_game)
			score = IterativeDeepening(&mv,30,player,ComputeSearchTime(player == COM ? com_remaining_time : opp_remaining_time));
		else
			score = IterativeDeepening(&mv,30,player,search_time);
	}
	else
		score = Search(&mv,depth,player);

	end = GetTime();
	elapsed = end - st;
	if (elapsed == 0.0) elapsed = 0.001;
	nps = ((float)nodes)/elapsed;
	/*sprintf(msg,"elapsed: %.5f",elapsed);
	ShowMessage(msg);*/

	printf("Quiesce nodes: %ld\n",quiesce_nodes);
/*#ifdef TTABLE
	printf("TT probes: %ld\n",tt_probes);
	printf("TT hits: %ld\n",tt_hits);
	if (flag.ttable) {
		sprintf(str,"TT probes: %ld",tt_probes);
		ShowMessage(str);
		sprintf(str,"TT hits: %ld",tt_hits);
		ShowMessage(str);
	}
#endif*/
	ShowScore(score);
	ShowNodeCnt(nodes, nps);
	ShowQuiesceNodeCnt(quiesce_nodes);
	return mv;
}

int IterativeDeepening(MOVE *mv, int total_depth, int player, long int max_time)
{
	int i, depth, nbr_of_moves, score, best=-INFINITY, temp, alpha=-INFINITY, beta=INFINITY;
	int j, moves, level = total_depth;
	NODE movearr[361];
	long start;

	PVMOVE best_line[MAX_PVSIZE], next_line[MAX_PVSIZE];

	start = GetTickCount();
	for (i=0;i<19*19;i++) {
		movearr[i].move = -1;
		movearr[i].score = -INFINITY;
	}
	if (flag.savepv)
		for (i=0;i<MAX_PVSIZE;i++)
			best_line[i].move = -1;

	nbr_of_moves = GenerateMoves(movearr);
	if (PieceNbr == 0 && flag.standard_gomoku) {
		mv->move = 180;
		ShowBestSearchedMove(0,9,9);
		return (0);
	}

	if (level > TOTALSQS-PieceNbr) level = TOTALSQS-PieceNbr;
	for (depth=1;depth<=level;depth++) {
		best = -INFINITY;
		moves = 0;
		if (flag.savepv)
			for (i=0;i<depth+2;i++) {
				next_line[i].move = -1;
			}

#ifdef RANDMOVE
		same_score_moves=0;
#endif
		for (i=0;i<nbr_of_moves;i++) {
			moves++;
			ShowSearchedMove(i+1,MOVEX(movearr[i].move),MOVEY(movearr[i].move),nbr_of_moves,depth);
			push_state();
			Board[movearr[i].move] = player;
			update_score(player,movearr[i].move);
			PieceNbr++;
#ifdef TTABLE
			if (flag.ttable) {
				HashKey = Xor(HashKey,(player == OPP ? xrandom[i]:orandom[i]));
				BD = Xor(BD,(player == OPP ? xrandom2[i]:orandom2[i]));
			}
#endif
			if (Winner == player)
				movearr[i].score = CWSCORE-1;
			else if (PieceNbr == TOTALSQS)
				movearr[i].score = 0;
			else
				movearr[i].score = -NegaScout(depth,Other(player),-INFINITY,INFINITY,next_line);

			if (movearr[i].score > best /*|| (i == 0 && depth > 1)*/) {
				if (flag.savepv) {
					for (j=tdepth;j<depth+2;j++)
						best_line[j] = next_line[j];
					best_line[tdepth].move = movearr[i].move;
					ShowPV(depth,level,best_line);
				}
				best = movearr[i].score;
				same_score_moves=1;
				if (best == CWSCORE-1) {
					alpha = CWSCORE-2;
					beta = CWSCORE;
				}
				else {
					alpha = best-100;
					beta = best+100;
				}
				ShowScore(best);

				if (!flag.savepv)
					ShowBestSearchedMove(i,MOVEX(movearr[i].move),MOVEY(movearr[i].move));

			}

#ifdef RANDMOVE
			else if (movearr[i].score == best) {
				same_score_moves++;
				if (rand()%10000 < ((float)1/(float)same_score_moves) * 10000) {
					movearr[i].score++;
					if (flag.savepv) {
						for (j=tdepth;j<depth+2;j++)
							best_line[j] = next_line[j];
						best_line[tdepth].move = movearr[i].move;
						ShowPV(depth,level,best_line);
					}
				}
				else movearr[i].score--;
			}
#endif
			pop_state();
#ifdef TTABLE
			if (flag.ttable) {
				HashKey = Xor(HashKey,(player == OPP ? xrandom[i]:orandom[i]));
				BD = Xor(BD,(player == OPP ? xrandom2[i]:orandom2[i]));
			}
#endif

			if (flag.force || flag.quit) {
				depth = level+1;
				break;
			}
			if (GetTickCount()-start > max_time) {
				depth = level+1;
				break;
			}

		}
		qsort((void *)movearr,moves,sizeof(movearr[0]),(int (*)(const void *,const void *))Comp);
		if (movearr[0].score < -9000 || movearr[0].score > 9000) break;
	}
#ifdef TTABLE
		if (flag.ttable)
			StoreInTT(move_num+level-tdepth,best_line[tdepth],best,0,0,normal_move,player);
#endif
	mv->move = movearr[0].move;
	score = movearr[0].score;
	return (score);
}

int GenerateMoves(NODE *movearr)
{
 	int i, moves = 0;
	for (i=0;i<TOTALSQS;i++) {
		if (PieceNbr == 0) {
			if (Centered(i) && LegalMove(i)) {
				movearr[moves].move = i;
				moves++;
			}
		}
		else
			if (Board[i] == EMPTY && (!Isolated(i) || (PieceNbr == 2 && flag.standard_gomoku)) && LegalMove(i)) {
				movearr[moves].move = i;
				moves++;
			}
	}
	return moves;
}

int Search(MOVE *mv, int level, int player)
{
	register int m;
	int i;
	register unsigned short int p;

	PVMOVE next_line[MAX_PVSIZE], best_line[MAX_PVSIZE];

	int value, best = -INFINITY;
	MOVE bestmove;
	short int moves=0;

	for (i=0;i<level+2;i++)
		next_line[i].move = -1;

	for (m=0;m<TOTALSQS;m++) {
		i=moveorder[m].move;
		if (Board[i] == EMPTY && (!Isolated(i) || Centered(i) ||
		(PieceNbr == 2 && flag.standard_gomoku)) && LegalMove(i)) {
			moves++;
			ShowSearchedMove(moves,MOVEX(i),MOVEY(i),-1,level);
			push_state();
			Board[i] = player;
			PieceNbr++;
			update_score(player,i);
#ifdef TTABLE
			if (flag.ttable) {
				HashKey = Xor(HashKey,(player == OPP ? xrandom[i]:orandom[i]));
				BD = Xor(BD,(player == OPP ? xrandom2[i]:orandom2[i]));
			}
#endif
			if (Winner == player) {
				value = CWSCORE-tdepth;
			}
			else if (PieceNbr == TOTALSQS) value = 0;
			else
				value = -NegaScout(level,Other(player),-INFINITY,INFINITY,next_line);

			if (value > best) {
				best = value;
				ShowScore(best);
				if (!flag.savepv)
	 				ShowBestSearchedMove(m,MOVEX(moveorder[m].move),MOVEY(moveorder[m].move));
				bestmove.move = i;
				same_score_moves = 1;
				if (flag.savepv) {
					for (p=tdepth;p<level+2;p++) best_line[p] = next_line[p];
					best_line[tdepth].move = i;
					ShowPV(level,100,best_line);
				}

			}
#ifdef RANDMOVE
			else if (value == best) {
				same_score_moves++;
				if (rand()%10000 < ((float)1/(float)same_score_moves) * 10000) {
					bestmove.move = i;
					if (!flag.savepv)
						ShowBestSearchedMove(m,MOVEX(moveorder[m].move),MOVEY(moveorder[m].move));
					if (flag.savepv) {
						for (p=tdepth;p<level+2;p++) best_line[p] = next_line[p];
						best_line[tdepth].move = i;
						ShowPV(level,100,best_line);
					}
				}
			}

#endif
			pop_state();
#ifdef TTABLE
			if (flag.ttable) {
				HashKey = Xor(HashKey,(player == OPP ? xrandom[i]:orandom[i]));
				BD = Xor(BD,(player == OPP ? xrandom2[i]:orandom2[i]));
			}
#endif
			if (flag.force || flag.quit) {
				mv->move=bestmove.move;
				return (best);
			}
        }
	}
#ifdef TTABLE
	if (flag.ttable)
		StoreInTT(move_num+level-tdepth,best_line[tdepth],best,0,0,normal_move,player);
#endif
	mv->move=bestmove.move;
	return (best);
}

int NegaScout(int level, int player, int alpha, int beta, PVMOVE best_line[])
{
	register int m;
	int i;
#ifdef TTABLE
	int ttDepth, ttScore, ttAlpha, ttBeta;
	unsigned char ttHaveMove=0;
	PVMOVE ttMove;
	int ttType;
#endif
	int a=alpha, b=beta, value, moves=0;
	int best=-INFINITY;
	register unsigned short int p;
	PVMOVE next_line[MAX_PVSIZE];
	best_line[tdepth+1].move = -1;
	nodes++;

#ifdef TTABLE
	if (flag.ttable && LookupTT(&ttDepth,&ttMove,&ttScore,&ttAlpha,&ttBeta,&ttType,player)) {
		tt_probes++;
		if (ttDepth >= move_num+level-tdepth) {
			tt_hits++;
			switch (ttType) {
				case normal_move:
					return ttScore;
					break;
				case fail_high:
					if (ttBeta >= beta) return ttBeta;
					break;
				case fail_low:
					if (ttAlpha <= alpha) return ttAlpha;
					break;
			}

		}
	}


#endif

	if (tdepth == level) {
#ifdef QUIESCE
		if (flag.search_extensions)
			return (Quiesce(level,Other(player),Evaluate(player),next_line));
		else return (Evaluate(player));
#else
		return (Evaluate(player));
#endif
	}

	if (nodes%NODES_BETWEEN_CHECKMESSAGE < level+2 )
		CheckMessage();
	if (nodes%NODES_BETWEEN_PRINTNODES < level+2)
		ShowNodeCnt(nodes,-1);

	for (m=0;m<MAX_TREEWIDTH;m++) {
		i=moveorder[m].move;
		if (Board[i] == EMPTY && !Isolated(i)) {
			moves++;
			push_state();
			Board[i] = player;
			PieceNbr++;
			update_score(player,i);

#ifdef TTABLE
			if (flag.ttable) {
				HashKey = Xor(HashKey,(player == OPP ? xrandom[i]:orandom[i]));
				BD = Xor(BD,(player == OPP ? xrandom2[i]:orandom2[i]));
			}
#endif


			if (Winner == player) {
				value = CWSCORE-tdepth;
				if (flag.savepv) {
					best_line[tdepth].move = i;
					best_line[tdepth+1].move = -1;
				}
				pop_state();

				return (value);
			}
			else if (PieceNbr == TOTALSQS) {
				if (flag.savepv) {
					best_line[tdepth].move = i;
					best_line[tdepth+1].move = -1;
				}
				pop_state();
#ifdef TTABLE
				if (flag.ttable) {
					HashKey = Xor(HashKey,(player == OPP ? xrandom[i]:orandom[i]));
					BD = Xor(BD,(player == OPP ? xrandom2[i]:orandom2[i]));
				}
#endif
				return (0);
			}
			else
				value = -NegaScout(level,Other(player),-b,-a,next_line);

			if (value > a && value < beta && moves > 1)
				a = -NegaScout(level,Other(player),-beta,-value,next_line);

			a = Max(a,value);

			if (flag.savepv)
				if (a > best) {
					best = a;
					for (p=tdepth;p<level+2;p++)
						best_line[p] = next_line[p];
					best_line[tdepth].move = i;
				}
			pop_state();
#ifdef TTABLE
			if (flag.ttable) {
				HashKey = Xor(HashKey,(player == OPP ? xrandom[i]:orandom[i]));
				BD = Xor(BD,(player == OPP ? xrandom2[i]:orandom2[i]));
			}
#endif



			if (a >= beta) {
#ifdef TTABLE
				if (flag.ttable)
					StoreInTT(move_num+level-tdepth,best_line[tdepth],best,a,b,fail_high,player);
#endif
				return (a);
			}

			b=a+1;

			if (flag.force || flag.quit) return (a);
        }
	}
#ifdef TTABLE
	if (flag.ttable)
		StoreInTT(move_num+level-tdepth,best_line[tdepth],best,a,b,(alpha==a)?fail_low:normal_move,player);
#endif
	return (a);
}
#ifdef QUIESCE
int QuiesceGenerate(int player, NODE *mvarr)
{

	register int i;
	int moves=0;

	for (i=0;i<TOTALSQS;i++)
		if (Board[i] == EMPTY && !Isolated(i)) {
			push_state();
			update_score(player, i);
			if (Winner == player) {
				mvarr[moves].move=i;
				moves++;
			}
			pop_state();
        }

        return (moves);
}

int Quiesce(int level,int player,int score, PVMOVE best_line[])
{
	register int i;
	int value, best=-INFINITY;
	PVMOVE next_line[MAX_PVSIZE];

	if ((current_state->max_connections[0] < onetowinnum  &&
		current_state->max_connections[1] < onetowinnum) || tdepth >= level+MAX_QUIESCE_DEPTH) {
		if (tdepth == level) /*return (Evaluate(Other(player)))*/ return(score);
		else return (Evaluate(player));
	}



	for (i=0;i<size;i++) {
		if (Board[i] == EMPTY && !Isolated(i) && IsThreat(player,i)) {
			nodes++;
			quiesce_nodes++;
			push_state();
			next_line[tdepth+1].move = -1;
			Board[i] = player;
			PieceNbr++;
			update_score(player,i);
			if (Winner == player) {
				if (tdepth != level+1)
					value = tdepth-CWSCORE;
				else
					value = CWSCORE-tdepth;
				pop_state();
				return (value);
			}
			else if (Winner == Other(player))
				value = -CWSCORE+tdepth;
			else if (PieceNbr == TOTALSQS) value = 0;
			else
				value = -Quiesce(level,Other(player),-score,next_line);

			if (value > best) {
				/*if (flag.savepv) {
					for (p=tdepth;p<level+MAX_QUIESCE_DEPTH+2;p++)
						best_line[p] = next_line[p];
					best_line[tdepth].movex = i;
					best_line[tdepth].movey = j;
				}*/
				best = value;
			}
			pop_state();
		}
	}
	if (tdepth == level) return (score);
	else return (best);

}

int IsThreat(int player, int move)
{
	int i;

#ifdef MAP_CHECK
	for (i=0;i<wins;i++) {
		if (map[MOVEX(move)][MOVEY(move)][i]) {
			if (current_state->score_array[player][i] >= onetowinnum /*||
				current_state->score_array[Other(player)][i] >= onetowinnum*/)
				return (1);
		}
	}
#else
	for (i=0;i<bd_index[move].places;i++) {
		if (current_state->score_array[player][bd_index[move].win_index[i]] >= onetowinnum)
			return (1);
	}
#endif
	return (0);
}

#endif

short int *Curr_Board(void)
{
    return Board;
}

void push_state(void)
{
	Game_state *old_state, *new_state;

	old_state = &state_stack[tdepth++];
	new_state = &state_stack[tdepth];

	if (tdepth == states_allocated) {
		  /* Allocate space for the board */
		new_state->board = (short int *) malloc(TOTALSQS * sizeof(short int));
		  /* Allocate space for the score array */
		new_state->score_array[0] = (short int *) malloc(win_places_array_size);
		new_state->score_array[1] = (short int *) malloc(win_places_array_size);
		/*new_state->isolated = (short int *) malloc(TOTALSQS*sizeof(short int));*/
#ifdef QUIESCE
	/*new_state->connections[0] = (short int *) malloc(win_places_array_size);
	new_state->connections[1] = (short int *) malloc(win_places_array_size);*/
#endif

		states_allocated++;
	}
	 /* Copy the board */
	memcpy(new_state->board, old_state->board, sizeof(short int)*TOTALSQS);

	 /* Copy the score array */
	memcpy(new_state->score_array[0], old_state->score_array[0],
			  win_places_array_size);
	memcpy(new_state->score_array[1], old_state->score_array[1],
			  win_places_array_size);
	/*memcpy(new_state->isolated,old_state->isolated,TOTALSQS*sizeof(short int));*/
	 /* Copy the connections array */

#ifdef QUIESCE
	 /*memcpy(new_state->connections[0], old_state->connections[0],
		win_places_array_size);
	 memcpy(new_state->connections[1], old_state->connections[1],
			  win_places_array_size);*/
#endif

	 new_state->score[0] = old_state->score[0];
	 new_state->score[1] = old_state->score[1];
	 new_state->winner = old_state->winner;
#ifdef QUIESCE
	if (flag.search_extensions) {
		new_state->max_connections[0] = old_state->max_connections[0];
	 	new_state->max_connections[1] = old_state->max_connections[1];
	}
#endif
	 new_state->num_of_pieces=old_state->num_of_pieces;
#ifdef TTABLE
	 new_state->hash_key=old_state->hash_key;
	 new_state->bd=old_state->bd;
#endif

	 current_state = new_state;
}


#ifdef TTABLE
void InitTTable(void)
{
	int i, log2tsize;
	ptrtoTT[0] = (TABLE_ENTRY *)malloc(sizeof(TABLE_ENTRY)*TTSIZE);
	ptrtoTT[1] = (TABLE_ENTRY *)malloc(sizeof(TABLE_ENTRY)*TTSIZE);
	ClearTT();
	InitRandomHash();
	HashKey=0LL;
	BD=0LL;
	for  (i=0;i<19*19;i++) {
		HashKey = Xor(HashKey,xrandom[i]);
		HashKey = Xor(HashKey,orandom[i]);
	}
	log2tsize = (int) (log(TTSIZE)/log(2));
	hash_mask = (BITBOARD)Shiftl(1,log2tsize)-1;
}

void InitRandomHash(void)
{
	int i;
	for (i=0;i<TOTALSQS;i++) {
		xrandom[i] = Random64();
		orandom[i] = Random64();
		xrandom2[i] = Random64();
		orandom2[i] = Random64();
	}
}

BITBOARD Random64(void)
{
	BITBOARD result;
	unsigned int r1, r2;

	r1=Random32();
	r2=Random32();
	result=Or(r1,Shiftl((BITBOARD) r2,32));
	return (result);
}

unsigned int Random32(void)
{
	/*
	random numbers from Mathematica 2.0.
	SeedRandom = 1;
	Table[Random[Integer, {0, 2^32 - 1}]
 	*/
	static unsigned long x[55] = {
		1410651636UL, 3012776752UL, 3497475623UL, 2892145026UL, 1571949714UL,
		3253082284UL, 3489895018UL, 387949491UL, 2597396737UL, 1981903553UL,
		3160251843UL, 129444464UL, 1851443344UL, 4156445905UL, 224604922UL,
		1455067070UL, 3953493484UL, 1460937157UL, 2528362617UL, 317430674UL,
		3229354360UL, 117491133UL, 832845075UL, 1961600170UL, 1321557429UL,
		747750121UL, 545747446UL, 810476036UL, 503334515UL, 4088144633UL,
		2824216555UL, 3738252341UL, 3493754131UL, 3672533954UL, 29494241UL,
		1180928407UL, 4213624418UL, 33062851UL, 3221315737UL, 1145213552UL,
		2957984897UL, 4078668503UL, 2262661702UL, 65478801UL, 2527208841UL,
		1960622036UL, 315685891UL, 1196037864UL, 804614524UL, 1421733266UL,
		2017105031UL, 3882325900UL, 810735053UL, 384606609UL, 2393861397UL };
	static int init = 1;
	static unsigned long y[55];
	static int j, k;
	unsigned long ul;

	if (init) {
		int i;
		init = 0;
		for (i = 0; i < 55; i++) y[i] = x[i];
		j = 24 - 1;
		k = 55 - 1;
	}
  	ul = (y[k] += y[j]);
	if (--j < 0) j = 55 - 1;
	if (--k < 0) k = 55 - 1;
	return((unsigned int)ul);
}

int LookupTT(int *depth, PVMOVE *move, int *score,int *upperbound, int *lowerbound, int *type, int player)
{

	TABLE_ENTRY *temp;
	temp = ptrtoTT[player]+(HashKey & hash_mask);
	if (temp->flag) {
		/*while (temp->bd != BD) {
			if ((temp = temp->next) == NULL) return 0;
		}*/
		/*if (temp->bd != BD || temp->hash_key != HashKey) return 0;*/
		/*tt_hits++;*/
		if (temp->bd == BD && temp->hash_key == HashKey) {
			*depth = temp->depth;
			*score = temp->score;
			*upperbound = temp->upperbound;
			*lowerbound = temp->lowerbound;
			*type = temp->type;
			move->move = temp->move;
			return TRUE;
		}
		else return FALSE;
		/*for (i=0;i<SIZE;i++) {
			for (j=0;j<SIZE;j++) {
			printf("%d ",(Board[j][i]==EMPTY?5:Board[j][i]));
			}
			printf("\n");
		}
		printf("move: %d %d\n",move->movex,move->movey);
		printf("score: %d\n",*score);
		printf("position: %ld\n",HashKey & hash_mask);
		getchar();*/
		/*sprintf(str,"move:%d %d, tdepth:%u, score:%d",move->movex,move->movey,*depth,*score);
		ShowMessage(str);*/
	}
	return (FALSE);
}

int StoreInTT(int depth, PVMOVE move, int score,int upperbound, int lowerbound, int type, int player)
{
	TABLE_ENTRY *temp;

	temp = ptrtoTT[player]+(HashKey & hash_mask);
	/*while (temp != NULL) {
		if (temp->bd == BD) {
			temp->depth = depth;
			temp->from = move.from;
			temp->to = move.to;
			temp->score = score;
			return true;
		}
		if ( (temp->next = (TABLE_ENTRY *)malloc(sizeof(TABLE_ENTRY))) == NULL)
			return false;
		temp = temp->next;
	}*/


	temp->depth = depth;
	temp->move = move.move;
	temp->score = score;
	temp->upperbound = upperbound;
	temp->lowerbound = lowerbound;
	temp->bd = BD;
	temp->hash_key = HashKey;
	temp->flag = TRUE;
	temp->type = type;
	return TRUE;

}

void ClearTT(void)
{
	int i;
	printf("Clearing transposition table\n");
	for (i=0;i<TTSIZE;i++) {
		(ptrtoTT[OPP]+i)->flag=0;
		(ptrtoTT[COM]+i)->flag=0;
		/*(ptrtoTT[0]+i)->depth=0;
		(ptrtoTT[0]+i)->score=0;
		(ptrtoTT[0]+i)->move=0;
		(ptrtoTT[0]+i)->flag=0;
		(ptrtoTT[0]+i)->bd=0;
		(ptrtoTT[1]+i)->depth=0;
		(ptrtoTT[1]+i)->score=0;
		(ptrtoTT[1]+i)->move=0;
		(ptrtoTT[1]+i)->flag=0;
		(ptrtoTT[1]+i)->bd=0;
		(ptrtoTT[0]+i)->hash_key=0;
		(ptrtoTT[1]+1)->hash_key=0;*/
		/*(ptrtoTT+i)->depth=0;
		(ptrtoTT+i)->score=0;*/
		/*(ptrtoTT+i)->upperbound=0;
		(ptrtoTT+i)->lowerbound=0;*/
		/*(ptrtoTT+i)->move=0;
		(ptrtoTT+i)->flag=0;
		(ptrtoTT+i)->bd = 0;*/
	}
	printf("done\n");
}

void UpdateHashKeys(int move,int player)
{

	if (flag.ttable) {
		HashKey = Xor(HashKey,(player == OPP ? xrandom[move]:orandom[move]));
		BD = Xor(BD,(player == OPP ? xrandom2[move]:orandom2[move]));
	}
}

#endif

/* Functions for MS-Windows version */

void MakeMove(int move, int turn)
{
	Board[move] = turn;
	update_score(turn, move);
	PieceNbr++;
	game_table[PieceNbr-1].move = move;
	game_table[PieceNbr-1].piece = turn;
	game_table[PieceNbr-1].opp_time = opp_remaining_time;
	game_table[PieceNbr-1].com_time = com_remaining_time;

}

void UnmakeMove(void)
{
	int i, temp;
	int move, turn;
	temp = PieceNbr;
	if (temp>1) {
		EndGame();
		InitGame();
		for (i=0;i<temp-2;i++) {
			move = game_table[i].move;
			turn = game_table[i].piece;
			Board[move] = turn;
			opp_remaining_time = game_table[i].opp_time;
			com_remaining_time = game_table[i].com_time;
			update_score(turn,move);
			PieceNbr++;
		}
	}
}
int IsEmpty(int sq)
{
	return (Board[sq] == EMPTY);
}

int GetSquare(int sq)
{
	/*return (Board[x][y]);*/
	return (state_stack[0].board[sq]);
}

int LegalMove(int sq)
{
	if (flag.standard_gomoku) {
		if (PieceNbr == 0 && (sq != 180)) return (0);
		if (PieceNbr == 2 && (abs(MOVEX(sq)-9) < 3 && abs(MOVEY(sq)-9) < 3)) return (0);
	}
	return (1);
}

int GetPieceNbr(void)
{
	return PieceNbr;
}

void GetWinCoords(int *x1, int *y1, int *x2, int *y2)
{
    register int i, j;
    int winner, win_pos = 0;
    Boolean found;

    winner = current_state->winner;

    while (current_state->score_array[winner][win_pos] != winnum)
        win_pos++;

    /* Find the lower-left piece of the winning connection. */

    found = 0;
    for (j=0; j<size && !found; j++)
        for (i=0; i<size; i++)
            if (map[i][j][win_pos]) {
                *x1 = i;
                *y1 = j;
                found = 1;
                break;
            }

    /* Find the upper-right piece of the winning connection. */

    found = 0;
    for (j=size-1; j>=0 && !found; j--)
        for (i=size-1; i>=0; i--)
            if (map[i][j][win_pos]) {
                *x2 = i;
                *y2 = j;
                found = 1;
                break;
            }
}

long int ComputeSearchTime(long int time_left)
{
	long int searchtime;

	searchtime = ((float)time_left/72.0);
	return (searchtime);
}

void Flag(int player)
{
	Winner = player;
	flag.havewinner = 1;
}
