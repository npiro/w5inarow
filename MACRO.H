/* Macros */
#define 	Other(pl) 	(pl ^ 1)
#define 	Centered(i) 	(MOVEX(i) >= CENTER1 && MOVEX(i) <= CENTER2 && MOVEY(i) >= CENTER1 && MOVEY(i) <= CENTER2)
#define 	Evaluate(player) (current_state->score[player] - current_state->score[Other(player)])
#define 	pop_state() 	(current_state = &state_stack[--tdepth])

#define 	Max(a,b) 	(a > b ? a : b)
#define		Min(a,b)	(a < b ? a : b)

#define		And(a,b)        (a & b)
#define		Or(a,b)         (a | b)
#define		Xor(a,b)        (a ^ b)
#define		Shiftl(a,b)     (a << b)
#define		Shiftr(a,b)     (a >> b)

#define		MOVEX(n)	(n%SIZE)
#define		MOVEY(n)	((int)n/SIZE)
#define		MOVEVAL(x,y)	(y*size+x)
#define		Board		current_state->board
#define 	HashKey 	current_state->hash_key
#define		Winner		current_state->winner
#define		PieceNbr	current_state->num_of_pieces
#define		BD			current_state->bd
