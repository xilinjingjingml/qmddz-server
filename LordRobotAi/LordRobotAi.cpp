// LordRobotAi.cpp : 定义 DLL 应用程序的入口点。
//
#include "stdafx.h"
#include "ddz_interface.h"
#include <malloc.h>
//#include <iostream>

//**********************************************************************************************************************************************************************************************************************
//**********************************************************************************************************************************************************************************************************************
//**********************************************************************************************************************************************************************************************************************
//**********************************************************************************************************************************************************************************************************************
//**********************************************************************************************************************************************************************************************************************
//**********************************************************************************************************************************************************************************************************************
//**********************************************************************************************************************************************************************************************************************
//**********************************************************************************************************************************************************************************************************************
//#define DEBUG_ROBOT
#define LEVEL 0
#define KEY_INFO 0
#define VERBOSE 0

int is_game_first_half(GAME* game,int player);
int find_a_bigger_combo_in_hands(POKERS* h, COMBO_OF_POKERS * c, COMBO_OF_POKERS* a);
int Check_win_quick(GAME * gm,int firstplayer_num,BOOL cur, BOOL up, BOOL down);	//need optimization
int get_the_best_hand(GAME *game,  COMBO_OF_POKERS * cur,bool first);

FILE* logfile,*logfile2;
int level=0,log_brief=0; /* 0 for ouput basic */
int for_test=0;
int robot_id=0; //robot's point

#define FUNC_NAME PRINTF(LEVEL,"ENTER %s\n",__FUNCTION__);
#define IS_HUN(h,i)   (i == h->hun)
#define HUN_NUMBER(h)   (h->hun>=0?h->hands[h->hun]:0)
#define SET_HUN_TO_i(h,i)  do{\
	h->hands[h->hun]--;\
	h->hands[i]++;\
	sort_poker(h);\
}while(0)

#define SET_i_to_HUN(h,i)  do{\
	h->hands[i]--;\
	h->hands[h->hun]++;\
	sort_poker(h);\
}while(0)



void PRINTF(int a,const char* const b, ...) {
#ifndef DEBUG_ROBOT
	 return;
#endif
	char   buffer[1024] = {0};

	va_list args;
	/* start or argument list */
	va_start(args, b);
	/* add in only format portion */
	vsprintf(buffer, (char *) b, args);

	/* tidy up */
	va_end(args);
	if (a<=level) {
		printf("%s",buffer);
	}
	if (a<=level)
	{
		if (logfile!=NULL)
			fprintf(logfile,	"%s",buffer);
		fflush(logfile);
	}
	if (a<=level) {
		if (logfile2!=NULL && log_brief)
			fprintf(logfile2,	"%s",buffer);
	}
}

void PRINTF_ALWAYS(const char* const b, ...)
{
#ifndef DEBUG_ROBOT
	return;
#endif
	char   buffer[1024] = {0};
	va_list args;
	/* start or argument list */
	va_start(args, b);
	/* add in only format portion */
	vsprintf(buffer, (char *)b, args);

	/* tidy up */
	va_end(args);
	printf("%s",buffer);
	if (logfile!=NULL)
		fprintf(logfile,	"%s",buffer);

	if (logfile2!=NULL && log_brief)
		fprintf(logfile2,	"%s",buffer);
	//just for test
	if(for_test)
		while(1);
}

#define CONTROL_NUM(player) ((player)->summary->ctrl.single+(player)->summary->extra_bomb_ctrl*10)
#define CONTROL_SUB_COMBO_NUM(player)  ( (CONTROL_NUM(player))-(10*(player)->summary->combo_total_num)-  ((player)->summary->combo_with_2_controls*10))
#define CONTROL_SUB_COMBO_NUM_IS_GOOD(player) ( CONTROL_SUB_COMBO_NUM(player) > -20 )
#define HAS_ROCKET(h)  (h->hands[LIT_JOKER] && h->hands[BIG_JOKER] )
#define PRE_PLAYER(game)  ( game->players[game->pre_playernum]->id )
#define NOT_BOMB(c)  (c->type!=BOMB && c->type!=ROCKET)
#define SET_PLAYER_POINT(game) \
	PLAYER * pl= game->players[CUR_PLAYER];\
	PLAYER * downPl= game->players[UP_PLAYER];\
	PLAYER * upPl= game->players[DOWN_PLAYER];


COMBO_OF_POKERS* check_for_win_now(GAME* game,COMBO_OF_POKERS* pre);

//poker operations

void full_poker(POKERS *h)
{
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		h->hands[i]  = i>=LIT_JOKER?1:4;
	}
	h->end=BIG_JOKER;
	h->begin=P2;
	h->total=54;
}


int sort_poker(POKERS * h )
{
	int res=true;
	h->begin=-1;
	h->end=h->total=0;
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		if (h->hands[i]>0)
		{
			if (h->begin==-1) h->begin = i;
			h->end =i;
		}
		else if (h->hands[i]<0)
		{
			res=false;
			PRINTF_ALWAYS("ERR in sort_poker: fatal, poker %d is negative\n",i);
		}
		h->total+=h->hands[i];
	}
	if (h->begin==-1) h->begin = 0;
	return res;
	//if (h->end>12) h->end = 12;
}


void add_poker(POKERS* a, POKERS * b, POKERS * c ) // c=a+b
{
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		c->hands[i]  = a->hands[i] + b->hands[i];
	}
	c->total=a->total+b->total;
	c->end=max(a->end,b->end);
	c->begin=min(a->begin,b->begin);
	sort_poker(c);
}

void sub_poker(POKERS* a, POKERS * b, POKERS * c ) // c=a-b
{
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		c->hands[i]  = a->hands[i] - b->hands[i];
	}
	sort_poker(c);
}

int cmp_poker(POKERS* a, POKERS * b ) // return a!=b
{
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		if( a->hands[i] != b->hands[i])
			return 1;
	}
	return 0;
}

//return hun numbers
int sub_poker_with_hun(POKERS* a, POKERS * b, POKERS * c ) // c=a-b
{
	int hun_num=0;
	c->hun = a->hun;
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		c->hands[i]  = a->hands[i] - b->hands[i];
		if(c->hands[i] <0) //carry from hun
		{
			hun_num = -c->hands[i];
			c->hands[c->hun] +=c->hands[i];
			c->hands[i]=0;  

		}
	}
	sort_poker(c);
	return hun_num;
}

int convert_poker_to_hun(POKERS* src, POKERS * dst, POKERS* realhand)
{
	int hun_num=0;
	int hun = realhand->hun;
	dst->hands[hun] =0;
	for (int i=0; i<MAX_POKER_KIND; i++)
	{

		if( src->hands[i] > realhand->hands[i] )
		{
			dst->hands[i]= realhand->hands[i];
			dst->hands[hun] += src->hands[i] - realhand->hands[i];   
			hun_num +=src->hands[i] - realhand->hands[i];  
		}
		else if(i==hun)
			dst->hands[i]+=src->hands[i];		   
		else	
			dst->hands[i]=src->hands[i];		   
	}
	if(hun_num > realhand->hands[hun])	
	{
		PRINTF_ALWAYS("line %d, No enough hun in real hand\n", __LINE__);
	}
	sort_poker(dst);
	return hun_num;
}

int is_sub_poker(POKERS* a, POKERS * b ) // a in b ? 1 : 0
{
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		if ( a->hands[i] > b->hands[i]) return false;
	}
	return true;
}

//check Poker a is the subset of poker B
int is_subset_poker_hun(POKERS* a, POKERS * b ) // a in b ? 1 : 0
{
	int missed=0;
	for (int i=0; i<LIT_JOKER; i++)
	{
		if(a->hands[i] <=0)
			continue;
		if(IS_HUN(b, i) )
		{
			missed+=a->hands[i];
			continue;
		}			
		if ( a->hands[i] > b->hands[i]) 
			missed+=   a->hands[i] - b->hands[i] ;
	}
	if(missed>HUN_NUMBER(b)) return false;
	for (int i=LIT_JOKER; i<MAX_POKER_KIND; i++)
	{
		if ( a->hands[i] > b->hands[i]) return false;
	}
	return true;
}

int combo_2_poker(POKERS* hand, COMBO_OF_POKERS * h) // h must be contained by hand.
{
	for(int i=P3; i<=BIG_JOKER; i++)
		hand->hands[i]=0;
	switch (h->type)
	{
	case ROCKET :
		hand->hands[LIT_JOKER]=1;
		hand->hands[BIG_JOKER]=1;
		break;
	case SINGLE_SERIES:
		for (int i=h->low; i<h->low+h->len; i++) hand->hands[i]=1;
		break;
	case PAIRS_SERIES:
		for (int i=h->low; i<h->low+h->len; i++) {
			hand->hands[i]=2;
		}
		break;
	case THREE_SERIES:
		for (int i=h->low; i<h->low+h->len; i++) {
			hand->hands[i]=3;
		}
		break;
	case BOMB:
		hand->hands[h->low]=4;
		break;
	case THREE:
		hand->hands[h->low]=3;
		break;
	case PAIRS:
		hand->hands[h->low]=2;
		break;
	case SINGLE:
		hand->hands[h->low]=1;
		break;
	case 31: //31
		for (int i=h->low; i<h->low+1; i++) hand->hands[i]=3;
		hand->hands[h->three_desc[0]]=1;
		break;
	case 32: //32
		for (int i=h->low; i<h->low+1; i++) hand->hands[i]=3;
		hand->hands[h->three_desc[0]]=2;
		//hand->hands[h->three_desc[0]];
		break;
	case 3311: //3311
		for (int i=h->low; i<h->low+2; i++) hand->hands[i]=3;
		hand->hands[h->three_desc[0]]+=1;
		hand->hands[h->three_desc[1]]+=1;
		break;
	case 3322: //3322
		for (int i=h->low; i<h->low+2; i++) hand->hands[i]=3;
		hand->hands[h->three_desc[0]]+=1;
		hand->hands[h->three_desc[1]]+=1;
		hand->hands[h->three_desc[0]]+=1;
		hand->hands[h->three_desc[1]]+=1;
		break;
	case 333111: //333111
		for (int i=h->low; i<h->low+3; i++) hand->hands[i]=3;
		hand->hands[h->three_desc[0]]+=1;
		hand->hands[h->three_desc[1]]+=1;
		hand->hands[h->three_desc[2]]+=1;
		break;
	case 333222: //333222
		for (int i=h->low; i<h->low+3; i++) hand->hands[i]=3;
		hand->hands[h->three_desc[0]]+=2;
		hand->hands[h->three_desc[1]]+=2;
		hand->hands[h->three_desc[2]]+=2;
		break;
	case 33332222: //33332222
		for (int i=h->low; i<h->low+4; i++) hand->hands[i]=3;
		hand->hands[h->three_desc[0]]+=2;
		hand->hands[h->three_desc[1]]+=2;
		hand->hands[h->three_desc[2]]+=2;
		hand->hands[h->three_desc[3]]+=2;
		break;
	case 33331111: //33331111
		for (int i=h->low; i<h->low+4; i++) hand->hands[i]=3;
		hand->hands[h->three_desc[0]]+=1;
		hand->hands[h->three_desc[1]]+=1;
		hand->hands[h->three_desc[2]]+=1;
		hand->hands[h->three_desc[3]]+=1;
		break;
	case 531: //531
		for (int i=h->low; i<h->low+5; i++) hand->hands[i]=3;
		hand->hands[h->three_desc[0]]+=1;
		hand->hands[h->three_desc[1]]+=1;
		hand->hands[h->three_desc[2]]+=1;
		hand->hands[h->three_desc[3]]+=1;
		hand->hands[(h->three_desc[4])]+=1;
		break;
	case 411: //411
		hand->hands[h->low]=4;
		hand->hands[h->three_desc[0]]+=1;
		hand->hands[h->three_desc[1]]+=1;
		break;
	case 422: //422
		hand->hands[h->low]=4;
		hand->hands[h->three_desc[0]]+=2;
		hand->hands[h->three_desc[1]]+=2;
		break;		
	case NOTHING:
		//break;
	default:
		PRINTF_ALWAYS("line %d, ERR, remove failed, this combo type %d not supported\n", __LINE__,h->type);
		break;
	};

	return sort_poker(hand);

}

int remove_combo_poker(POKERS* hand, COMBO_OF_POKERS * h, COMBO_OF_POKERS *h1  ) // h must be contained by hand.
{
	POKERS t;
	int err= combo_2_poker(&t,h);
	if( hand->hun == -1 )
	{
		sub_poker(hand, &t, hand);
		return err;
	}
	else
		return sub_poker_with_hun(hand, &t, hand);
}

//todo: add a parma number in COMBO_OF_POKERS
int get_combo_number(COMBO_OF_POKERS *h)
{
	{
		switch (h->type)
		{
		case ROCKET :
			return 2;
		case SINGLE_SERIES:
			return h->len;
		case PAIRS_SERIES:
			return h->len*2;
		case THREE_SERIES:
			return h->len*3;
		case 31: //31
		case BOMB:
			return 4;
		case THREE:
			return 3;
		case PAIRS:
			return 2;
		case SINGLE:
			return 1;

		case 32: //32
			return 5;
		case 411:
			return 6;
		case 422:
		case 3311: //3311
			return 8;
		case 3322: //3322
			return 10;
		case 333111: //333111
			return 12;
		case 333222: //333222
			return 15;
		case 33331111: //333111
			return 16;
			//	case 333222: //333222
		case 531: //333111
		case 33332222: //333222
			return 20;

		default:
			return 0;
		}
		//PRINTF(LEVEL," ");
	}
}

int check_combo_a_Big_than_b(COMBO_OF_POKERS * a, COMBO_OF_POKERS *b)
{
	if(b->type <=ROCKET || b->type ==THREE_ONE || b->type ==THREE_TWO) 
		b->len=1; //double check
	if(a->type <=ROCKET || a->type ==THREE_ONE || a->type ==THREE_TWO)
		a->len=1; //double check

	if (  a->type == ROCKET)
		return true;
	if (  b->type == ROCKET)
		return false;
	if(NOT_BOMB(a) && (b->type != a->type || a->len!=b->len))
	{
		return false;
	}     	
	if ( b->type == a->type && a->len==b->len && a->type !=BOMB)
	{
		if (b->low< a->low)
			return true;
		else 
			return false;	
	}
	if (  a->type == BOMB && NOT_BOMB(b))
		return true;
	if ( (a->type == BOMB && b->type==BOMB))
	{
		if(a->three_desc[0]>b->three_desc[0])
			return true;
		else if (a->three_desc[0]<b->three_desc[0])	
			return false;
		else   if (b->low< a->low)
			return true;	 	
	}   	
	return false;
}

int  int_2_poker(int i)
{
	if (i>51)
		i-=39;
	else
		i=i%13;
	return i;
}

//io functions
char poker_to_char(int i)
{
	switch ( i) {
	case 0:
		return '3';
	case 1:
		return '4';
	case 2:
		return '5';
	case 3:
		return '6';
	case 4:
		return '7';
	case 5:
		return '8';
	case 6:
		return '9';
	case 7:
		return 'T';
	case 8:
		return 'J';
	case 9:
		return 'Q';
	case 10:
		return 'K';
	case 11:
		return 'A';
	case 12:
		return '2';
	case 13:
		return 'X';
	case 14:
		return 'D';
	default:
		return '?';
	}
}

int char_to_poker(char c)
{
	switch ( c) {
	default:
		return -1;
	case '3':
		return 0;

	case '4':
		return 1;

	case '5':
		return 2;

	case '6':
		return 3;

	case '7':
		return 4;

	case '8':
		return 5;

	case '9':
		return 6;

	case 't':
	case 'T':
		return 7;

	case 'j':
	case 'J':
		return 8;

	case 'q':
	case 'Q':
		return 9;

	case 'k':
	case 'K':
		return 10;

	case 'a':
	case 'A':
		return 11;

	case '2':
		return 12;

	case 'x':
	case 'X':
		return 13;

	case 'D':
	case 'd':
		return 14;
	}
}

void print_one_poker(int i)
{
	PRINTF(LEVEL,"%c", poker_to_char(i));
}

void print_hand_poker(POKERS * h )
{
	PRINTF(LEVEL,"===total %d      ",h->total);//,poker_to_char(h->begin),poker_to_char(h->end));
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		if (h->hands[i]<0)
			PRINTF(LEVEL," ERR ");
		for (int j=0 ; j<h->hands[i]; j++)
			print_one_poker(i);
	}
	PRINTF(LEVEL,"       ===\n");
}

void print_hand_poker_in_line(POKERS * h )
{
	//PRINTF(LEVEL,"total %d\n===       ",h->total);//,poker_to_char(h->begin),poker_to_char(h->end));
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		if (h->hands[i]<0)
			PRINTF(LEVEL," ERR ");
		for (int j=0 ; j<h->hands[i]; j++)
			print_one_poker(i);
	}
	//PRINTF(LEVEL,"       ===\n");
}

void print_combo_poker(COMBO_OF_POKERS * h /*, COMBO_OF_POKERS *h1 /*for three*/  )
{
	switch (h->type)
	{
	case ROCKET :
		PRINTF(LEVEL,"Rocket!");
		break;
	case SINGLE_SERIES:
		for (int i=h->low; i<h->low+h->len; i++) print_one_poker(i);
		break;
	case PAIRS_SERIES:
		for (int i=h->low; i<h->low+h->len; i++) {
			print_one_poker(i);
			print_one_poker(i);
		}
		break;
	case THREE_SERIES:
		for (int i=h->low; i<h->low+h->len; i++) {
			print_one_poker(i);
			print_one_poker(i);
			print_one_poker(i);
		}
		break;
	case BOMB:
		print_one_poker(h->low);
	case THREE:
		print_one_poker(h->low);
	case PAIRS:
		print_one_poker(h->low);
	case SINGLE:
		print_one_poker(h->low);
		break;
	case 31: //31
		for (int i=h->low; i<h->low+1; i++) {
			print_one_poker(i);
			print_one_poker(i);
			print_one_poker(i);
		}
		print_one_poker(h->three_desc[0]);
		break;
	case 32: //32
		for (int i=h->low; i<h->low+1; i++) {
			print_one_poker(i);
			print_one_poker(i);
			print_one_poker(i);
		}
		print_one_poker(h->three_desc[0]);
		print_one_poker(h->three_desc[0]);
		break;
	case 3311: //3311
		for (int i=h->low; i<h->low+2; i++) {
			print_one_poker(i);
			print_one_poker(i);
			print_one_poker(i);
		}
		print_one_poker(h->three_desc[0]);
		print_one_poker(h->three_desc[1]);
		break;
	case 3322: //3322
		for (int i=h->low; i<h->low+2; i++) {
			print_one_poker(i);
			print_one_poker(i);
			print_one_poker(i);
		}
		print_one_poker(h->three_desc[0]);
		print_one_poker(h->three_desc[0]);
		print_one_poker(h->three_desc[1]);
		print_one_poker(h->three_desc[1]);
		break;
	case 333111: //333111
		for (int i=h->low; i<h->low+3; i++) {
			print_one_poker(i);
			print_one_poker(i);
			print_one_poker(i);
		}
		print_one_poker(h->three_desc[0]);
		print_one_poker(h->three_desc[1]);
		print_one_poker(h->three_desc[2]);
		break;
	case 333222: //333222
		for (int i=h->low; i<h->low+3; i++) {
			print_one_poker(i);
			print_one_poker(i);
			print_one_poker(i);
		}
		print_one_poker(h->three_desc[0]);
		print_one_poker(h->three_desc[0]);
		print_one_poker(h->three_desc[1]);
		print_one_poker(h->three_desc[1]);
		print_one_poker(h->three_desc[2]);
		print_one_poker(h->three_desc[2]);
		break;
	case 33331111: //33331111
		for (int i=h->low; i<h->low+4; i++) {
			print_one_poker(i);
			print_one_poker(i);
			print_one_poker(i);
		}
		print_one_poker(h->three_desc[0]);
		print_one_poker(h->three_desc[1]);
		print_one_poker(h->three_desc[2]);
		print_one_poker(h->three_desc[3]&0xf);
		break;
	case 33332222: //33332222
		for (int i=h->low; i<h->low+4; i++) {
			print_one_poker(i);
			print_one_poker(i);
			print_one_poker(i);
		}
		print_one_poker(h->three_desc[0]);
		print_one_poker(h->three_desc[0]);
		print_one_poker(h->three_desc[1]);
		print_one_poker(h->three_desc[1]);
		print_one_poker(h->three_desc[2]);
		print_one_poker(h->three_desc[2]);
		print_one_poker(h->three_desc[3]);
		print_one_poker(h->three_desc[3]);
		break;
	case 531: //531
		for (int i=h->low; i<h->low+5; i++) {
			print_one_poker(i);
			print_one_poker(i);
			print_one_poker(i);
		}
		print_one_poker(h->three_desc[0]);
		print_one_poker(h->three_desc[1]);
		print_one_poker(h->three_desc[2]);
		print_one_poker(h->three_desc[3]);
		print_one_poker((h->three_desc[4]));
		break;
	case NOTHING:
		break;
	default:
		PRINTF(LEVEL,"ERR, this combo type %d not supported\n", h->type);
		break;
	}
	PRINTF(LEVEL," ");
}

void print_all_combos(COMBO_OF_POKERS *combos, int number)
{
	DBG(
		for (int k=0; k<number; k++)
		{
			print_combo_poker(&combos[k]);
		}
		PRINTF(LEVEL,"\n");
		);
}


void print_card_2(int * p,int num)
{
	//DBG(
	char t[4]= {'d','c','h','s'};
	PRINTF(LEVEL,"CARD: ");
	int i=0;
	while(i++<num)
	{
		if(*p==-1){
			p++;
			continue;
		}
		if(*p>53 || *p<0 ) {
			PRINTF_ALWAYS("fatal error(card number too big)!\n");
			return;
		}
		if(*p>51)
			PRINTF(LEVEL,"%c ",*p==52?'X':'D');
		else
			PRINTF(LEVEL,"%c%d ",t[*p/13],*p%13);
		p++;
	}
	PRINTF(LEVEL,"\n");
	//)
}

void print_card(int * p)
{
	//DBG(
	char t[4]= {'d','c','h','s'};
	PRINTF(LEVEL,"CARD: ");
	while(*p!=-1)
	{
		if(*p>53 || *p<0 ) {
			PRINTF_ALWAYS("fatal error(card number too big)!\n");
			return;
		}
		if(*p>51)
			PRINTF(LEVEL,"%c ",*p==52?'X':'D');
		else
			PRINTF(LEVEL,"%c%d ",t[*p/13],*p%13);
		p++;
	}
	PRINTF(LEVEL,"\n");
	//)
}


void print_summary( COMBOS_SUMMARY * s)
{
	PRINTF(LEVEL,"bomb_ctrl %d,",s->extra_bomb_ctrl);
	PRINTF(LEVEL,"combos %d,big %d,type %d total %d\n",s->combo_total_num,s->biggest_num,s->combo_typenum,s->real_total_num);
	//PRINTF(LEVEL,"single %d\n",s->singles_num);

	//PRINTF(LEVEL, "\nSingles: ");
	for (int k=0; k<s->singles_num; k++)
	{
		print_combo_poker(s->singles[k]);
	}
	//PRINTF(LEVEL, "\nPairs: ");
	for (int k=0; k<s->pairs_num; k++)
	{
		print_combo_poker(s->pairs[k]);
	}
	//PRINTF(LEVEL, "\nThrees: ");
	for (int k=0; k<s->three_num; k++)
	{
		print_combo_poker(s->three[k]);
	}
	for (int k=0; k<s->three_one_num; k++)
	{
		print_combo_poker(s->three_one[k]);
	}
	for (int k=0; k<s->three_two_num; k++)
	{
		print_combo_poker(s->three_two[k]);
	}
	//PRINTF(LEVEL, "\nSeries: ");
	for (int k=0; k<s->series_num; k++)
	{
		print_combo_poker(s->series[k]);
	}
	//PRINTF(LEVEL, "\nBombs: ");
	for (int k=0; k<s->bomb_num; k++)
	{
		print_combo_poker(s->bomb[k]);
	}
	PRINTF(LEVEL, "  big:");
	for (int k=0; k<s->biggest_num; k++)
	{
		print_combo_poker(s->biggest[k]);
	}
	PRINTF(LEVEL, "\n");
}

void print_suit(PLAYER * player)
{
	COMBOS_SUMMARY * s = player->summary;
	//    PRINTF(LEVEL+1,"\nctrl %d extra_ctrl %d,",player->max_control.single,s->extra_bomb_ctrl);
	PRINTF(LEVEL+1,"combos %d,big %d,type %d total %d\n",s->combo_total_num,s->biggest_num,s->combo_typenum,s->real_total_num);
	//PRINTF(LEVEL,"single %d\n",s->singles_num);

	//PRINTF(LEVEL, "\nSingles: ");
	for (int k=0; k<s->singles_num; k++)
	{
		print_combo_poker(s->singles[k]);
	}
	//PRINTF(LEVEL, "\nPairs: ");
	for (int k=0; k<s->pairs_num; k++)
	{
		print_combo_poker(s->pairs[k]);
	}
	//PRINTF(LEVEL, "\nThrees: ");
	for (int k=0; k<s->three_num; k++)
	{
		print_combo_poker(s->three[k]);
	}
	for (int k=0; k<s->three_one_num; k++)
	{
		print_combo_poker(s->three_one[k]);
	}
	for (int k=0; k<s->three_two_num; k++)
	{
		print_combo_poker(s->three_two[k]);
	}
	//PRINTF(LEVEL, "\nSeries: ");
	for (int k=0; k<s->series_num; k++)
	{
		print_combo_poker(s->series[k]);
	}
	//PRINTF(LEVEL, "\nBombs: ");
	for (int k=0; k<s->bomb_num; k++)
	{
		print_combo_poker(s->bomb[k]);
	}
	PRINTF(LEVEL, "  big:");
	for (int k=0; k<s->biggest_num; k++)
	{
		print_combo_poker(s->biggest[k]);
	}
	PRINTF(LEVEL, "\n");
}


void read_poker(char * buf, POKERS * h  )
{
	do {
		switch ( *buf) {
		case '3':
			h->hands[0]++;
			break;
		case '4':
			h->hands[1]++;
			break;
		case '5':
			h->hands[2]++;
			break;
		case '6':
			h->hands[3]++;
			break;
		case '7':
			h->hands[4]++;
			break;
		case '8':
			h->hands[5]++;
			break;
		case '9':
			h->hands[6]++;
			break;
		case 't':
		case 'T':
			h->hands[7]++;
			break;
		case 'j':
		case 'J':
			h->hands[8]++;
			break;
		case 'q':
		case 'Q':
			h->hands[9]++;
			break;
		case 'k':
		case 'K':
			h->hands[10]++;
			break;
		case 'a':
		case 'A':
			h->hands[11]++;
			break;
		case '2':
			h->hands[12]++;
			break;
		case 'x':
		case 'X':
			h->hands[13]++;
			break;
		case 'D':
		case 'd':
			h->hands[14]++;
			break;
		default:
			break;
		}
	}	while ( *buf++!='\0');


	//check more than four
	{
		int i;
		for (i=0; i<15; i++)
		{
			if (h->hands[i]>4) h->hands[i]=4;
		}
	}
	sort_poker(h);
}

int read_poker_int(int * buf, POKERS * h  )
{
	{
		int i;
		for (i=0; i<15; i++)
		{
			h->hands[i]=0;
		}
	}
	char count = 0;
	do {
		int a= *buf;
		if(a==-1) break;
		count++;
		if(a<52)
			a= a%13;
		else
			a=(a-39)%15;
		h->hands[a]++;
		buf++;
	}	while ( 1);


	//check more than four
	{
		int i;
		for (i=0; i<15; i++)
		{
			if (h->hands[i]>4) h->hands[i]=4;
		}
	}
	sort_poker(h);
	return count;
}

int read_poker_int_2(int * buf, int len,POKERS * h  )
{
	{
		int i;
		for (i=0; i<15; i++)
		{
			h->hands[i]=0;
		}
	}
	char count = 0,count1=0;
	do {
		int a= *buf;
		count1++;
		if(count1>len)
			break;
		if(a==-1){ buf++;continue;}
		count++;
		if(a<52)
			a= a%13;
		else
			a=(a-39)%15;
		h->hands[a]++;
		buf++;
	}	while ( 1);


	//check more than four
	{
		int i;
		for (i=0; i<15; i++)
		{
			if (h->hands[i]>4) h->hands[i]=4;
		}
	}
	sort_poker(h);
	return count;
}

int find_and_remove_int_in_card(int* card,int value) {
	for(int i=0; i<20; i++)
	{
		int t=-1;
		if(card[i]!=-1)
		{
			if(card[i]>51)
				t=card[i]-39;
			else
				t=card[i]%13;
		}
		if(t==value) {
			int tmp=card[i];
			card[i]=-1;
			return tmp;
		}
	}
	return -1;
}

void remove_poker_in_int_array(POKERS * h,int * p,int *card, int hun)
{
	int hun_num = 0;
	int tmp[1+20+4+1];
	int aim_carder[21],*aim=aim_carder;	
	int* p_bak =p;
	p =tmp;	
	for(int i=P3; i<=BIG_JOKER; i++)
	{
		while (h->hands[i]!=0)
		{
			*p= find_and_remove_int_in_card(card,i);
			if(*p==-1)
			{
				if(i>=LIT_JOKER)
					PRINTF_ALWAYS("FUCK in line %d, no card for %d or hun %d\n",__LINE__, i,hun);		
				//check hun again
				*p= find_and_remove_int_in_card(card,hun);    
				if(*p==-1)
				{
					PRINTF_ALWAYS("FUCK in line %d, no card for %d or hun %d\n",__LINE__, i,hun);			      
				}
				*aim++=i;
				hun_num++;
				p++;
				h->hands[i]--;			
			}
			else 	
			{
				if(i==hun)
				{
					hun_num++;
					*aim++=i;
				}
				p++;
				h->hands[i]--;
			}
		}
	}
	*p = -1;

	// re-org for hun
	if(1)// hun_num)
	{
		int huns[4];
		p =p_bak;
		*p++= hun_num;
		int j=0;
		for(int i=0; i<21;i++)
		{
			if(tmp[i]!=-1 )
			{
				if (tmp[i]>=52 || (tmp[i]%13)!=hun )
					*p++=tmp[i];
				else
					huns[j++]=tmp[i];
			}
			else 
				break;			 
		}
		for(int i=0;i<hun_num;i++)
		{
			*p++=aim_carder[i];//huns[i];
		}
		//just for internal testing...
		for(int i=0;i<hun_num;i++)
		{
			*p++=huns[i];
		}
		*p++ =-1;		
	}
	else
	{
		memcpy(p_bak, tmp,21*sizeof(int));
	}

}
void combo_to_int_array_hun(COMBO_OF_POKERS *cur,int * p,int *card, int hun)
{
	POKERS h1,h;
	combo_2_poker(&h,cur);
	//  convert_poker_to_hun(&h1,&h,realhand);
	int hun_num = 0;
	int tmp[1+20+4+1];
	int aim_carder[21],*aim=aim_carder;	
	int* p_bak =p;
	p =tmp;	
	for(int i=P3; i<=BIG_JOKER; i++)
	{
		while (h.hands[i]!=0)
		{
			*p= find_and_remove_int_in_card(card,i);
			if(*p==-1)
			{
				if(i>=LIT_JOKER)
					PRINTF_ALWAYS("FUCK in line %d, no card for %d or hun %d\n",__LINE__, i,hun);		
				//check hun again
				*p= find_and_remove_int_in_card(card,hun);    
				if(*p==-1)
				{
					PRINTF_ALWAYS("FUCK in line %d, no card for %d or hun %d\n",__LINE__, i,hun);			      
				}
				*aim++=i;
				hun_num++;
				p++;
				h.hands[i]--;			
			}
			else 	
			{
				if(i==hun)
				{
					hun_num++;
					*aim++=i;
				}
				p++;
				h.hands[i]--;
			}
		}
	}
	*p = -1;

	// re-org for hun
	if(1)// hun_num)
	{
		int huns[4];
		p =p_bak;
		*p++= hun_num;
		int j=0;
		for(int i=0; i<21;i++)
		{
			if(tmp[i]!=-1 )
			{
				if (tmp[i]>=52 || (tmp[i]%13)!=hun )
					*p++=tmp[i];
				else
					huns[j++]=tmp[i];
			}
			else 
				break;			 
		}
		for(int i=0;i<hun_num;i++)
		{
			*p++=aim_carder[i];//huns[i];
		}
		//just for internal testing...
		for(int i=0;i<hun_num;i++)
		{
			*p++=huns[i];
		}
		*p++ =-1;		
	}
	else
	{
		memcpy(p_bak, tmp,21*sizeof(int));
	}

}

void combo_to_int_array(COMBO_OF_POKERS *cur,int * p,int *card)
{
	POKERS h;
	combo_2_poker(&h,cur);
	for(int i=P3; i<=BIG_JOKER; i++)
	{
		while (h.hands[i]!=0)
		{
			*p= find_and_remove_int_in_card(card,i);
			if(*p==-1)
			{
				PRINTF_ALWAYS("FUCK in line %d\n",__LINE__);
			}
			p++;
			h.hands[i]--;
		}
	}
	*p = -1;
}



//check functions

int is_series(POKERS *h,int number)
{

	for (int i = h->begin; i<= CEIL_A(h->end); i++)
	{
		if ( h->hands[i]!=1) return false;
	}
	return true;
}

int is_doubleseries(POKERS *h)
{

	for (int i = h->begin; i<= CEIL_A(h->end); i++)
	{
		if ( h->hands[i]!=2) return false;
	}
	return true;
}

int is_threeseries(POKERS *h)
{

	for (int i = h->begin; i<= CEIL_A(h->end); i++)
	{
		if ( h->hands[i]!=3) return false;
	}
	return true;
}

int is_411(POKERS *h, COMBO_OF_POKERS *c )
{
	int j=0;
	c->type = -1;
	for (int i = h->begin; i<= h->end; i++)
	{
		if ( h->hands[i] ==4 )
		{
			c->type = 411;
			c->low= i;
			c->len =1;
			break;
		}
	}
	if (c->type != 411)
		return false;

	for (int i = h->begin; i<= (h->end); i++)
	{
		if ( h->hands[i] ==1  )
			c->three_desc[j++] = i;
		else if ( h->hands[i] ==2  )
		{
			c->three_desc[0] = i;
			c->three_desc[1] = i;
		}
	}
	if (c->type == 411)
		return true;
	else
		return false;
}


int is_422(POKERS *h, COMBO_OF_POKERS *c )
{
	int j=0;
	c->type = -1;
	for (int i = h->begin; i<= h->end; i++)
	{
		if ( h->hands[i] ==4 )
		{
			c->type = 422;
			c->len =1;
			c->low = i;
			break;
		}
	}

	if (c->type != 422)
		return false;
	
	for (int i = h->begin; i<= (h->end); i++)
	{
		if ( h->hands[i] ==2 )
			c->three_desc[j++] = i;
	}
	if ( j != 2)
		c->type = -1;
	if (c->type == 422)
		return true;
	else
		return false;
}


int is_31(POKERS *h, COMBO_OF_POKERS *c )
{
	int j=0;
	c->type = -1;
	for (int i = h->begin; i<= h->end; i++)
	{
		if ( h->hands[i] ==3  )
		{
			c->type = 31;
			c->len = 1;
			c->low = i;
			break;
		}
	}
	if (c->type != 31)
		return false;	
	for (int i = h->begin; i<= (h->end); i++)
	{
		if ( h->hands[i] ==1 )
		{
			c->three_desc[j++] = i;
			break;
		}
	}
	if (c->type == 31)
		return true;
	else
		return false;
}

int is_32(POKERS *h, COMBO_OF_POKERS *c )
{
	int j=0;
	c->type = -1;
	for (int i = h->begin; i<= h->end; i++)
	{
		if ( h->hands[i] ==3 )
		{
			c->type = 32;
			c->len = 1;
			c->low = i;
			break;
		}
	}
	if (c->type != 32)
		return false;	
	
	for (int i = h->begin; i<= (h->end); i++)
	{
		if ( h->hands[i] ==2 )
             {
			c->three_desc[j++] = i;
			break;
		}
	}
	if (j!=1)
		c->type = -1;
	if (c->type == 32)
		return true;
	else
		return false;
}



int is_3322(POKERS *h, COMBO_OF_POKERS *c )
{
	int j=0;
	c->type = -1;
	for (int i = h->begin; i<= CEIL_A(h->end)-1; i++)
	{
		if ( h->hands[i] ==3 && h->hands[i+1] ==3 )
		{
			c->type = 3322;
			c->low = i;
			c->len =2;
			break;
		}
	}

	if (c->type != 3322)
		return false;	
	
	for (int i = h->begin; i<= (h->end); i++)
	{
		/*
		if( h->hands[i]==4){
		c->three_desc[0] = i;
		c->three_desc[1] = i;
		break;
		}
		else */
		if ( h->hands[i] ==2 )
			c->three_desc[j++] = i;
		//double check
		if(j>=2)
			break;
	}
	if (j!=2)
		c->type = -1;
	if (c->type == 3322)
		return true;
	else
		return false;
}


int is_333222(POKERS *h, COMBO_OF_POKERS *c )
{
	int j=0;
	c->type = -1;
	for (int i = h->begin; i<= CEIL_A(h->end)-2; i++)
	{
		if ( h->hands[i] ==3 && h->hands[i+1] ==3 && h->hands[i+2] ==3 )
		{
			c->type = 333222;
			c->low =i;
			c->len =3;
			break;
		}
	}
	if (c->type != 333222)
		return false;		
	for (int i = h->begin; i<= (h->end); i++)
	{
		if ( h->hands[i] ==2 )
			c->three_desc[j++] = i;
		//double check
		if(j>=3)
			break;		
	}
	if ( j!=3)
		c->type = -1;
	if (c->type == 333222)
		return true;
	else
		return false;
}

int is_3311(POKERS *h, COMBO_OF_POKERS *c )
{
	int j=0;
	c->type = -1;
	for (int i = h->begin; i<= CEIL_A(h->end)-1; i++)
	{
		if ( h->hands[i] ==3 && h->hands[i+1] ==3 )
			//         if ( h->hands[i] >=3 && h->hands[i+1] >=3 )
		{
			c->type = 3311;
			c->low = i;
			c->len =2;
			break;
		}
	}
	if (c->type != 3311)
		return false;	
	
	for (int i = h->begin; i<= (h->end); i++)
	{
		if ( h->hands[i] ==2 )
		{
			c->three_desc[j++] = i;
			c->three_desc[j++] = i;
		}
		else  if ( h->hands[i] == 1 )
			c->three_desc[j++] = i;
		if(j>=2)
			break;
	}
	if (c->type == 3311)
		return true;
	else
		return false;
}

int is_333111(POKERS *h, COMBO_OF_POKERS *c )
{
	int j=0;
	c->type = -1;
	for (int i = h->begin; i<= CEIL_A(h->end)-2; i++)
	{
		if ( h->hands[i] ==3 && h->hands[i+1] ==3&& h->hands[i+2] ==3 )
		{
			c->type = 333111;
			c->low = i;
			c->len =3;
			h->hands[i]-=3;
			h->hands[i+1]-=3;
			h->hands[i+2]-=3;
			break;
		}
	}
	if (c->type != 333111)
		return false;		
	for (int i = h->begin; i<= (h->end); i++)
	{

		if ( h->hands[i]>=1 )
		{
			for (int k=0; k<h->hands[i]; k++)
				c->three_desc[j++] = i;
			if(j>=3)
				break;
		}
	}
			h->hands[c->low]+=3;
			h->hands[c->low+1]+=3;
			h->hands[c->low+2]+=3;
			
	if (c->type == 333111)
		return true;
	else
		return false;
}


int is_33331111(POKERS *h, COMBO_OF_POKERS *c )
{
	int j=0;
	c->type = -1;
	for (int i = h->begin; i<= CEIL_A(h->end)-3; i++)
	{
		if ( h->hands[i] ==3 && h->hands[i+1] ==3&& h->hands[i+2] ==3 && h->hands[i+3] ==3 )
		{
			c->type = 33331111;
			c->len =4;
			c->low =i;
			h->hands[i]-=3;
			h->hands[i+1]-=3;
			h->hands[i+2]-=3;
			h->hands[i+3]-=3;
			break;
		}
	}
	if (c->type != 33331111)
		return false;		
	for (int i = h->begin; i<= (h->end); i++)
	{
		if (  h->hands[i]>=1 )
		{
			for (int k=0; k<h->hands[i]; k++)
				c->three_desc[j++] = i;
		}

	}
			h->hands[c->low]+=3;
			h->hands[c->low+1]+=3;
			h->hands[c->low+2]+=3;
			h->hands[c->low+3]+=3;

	if (c->type == 33331111)
		return true;
	else
		return false;
}

int is_531(POKERS *h, COMBO_OF_POKERS *c )
{
	int j=0;
	c->type = -1;
	for (int i = h->begin; i<= CEIL_A(h->end)-3; i++)
	{
		if ( h->hands[i] ==3 && h->hands[i+1] ==3&& h->hands[i+2] ==3 && h->hands[i+3] ==3
			&& h->hands[i+4] ==3)
		{
			c->type = 531;
			c->len =5;
			c->low =i;
			h->hands[i]-=3;
			h->hands[i+1]-=3;
			h->hands[i+2]-=3;
			h->hands[i+3]-=3;
			h->hands[i+4]-=3;
			break;
		}
	}
	if (c->type != 531)
		return false;
	
	for (int i = h->begin; i<= (h->end); i++)
	{
		if (  h->hands[i]>=1 )
		{
			for (int k=0; k<h->hands[i]; k++)
				c->three_desc[j++] = i;
		}
	}
			h->hands[c->low]+=3;
			h->hands[c->low+1]+=3;
			h->hands[c->low+2]+=3;
			h->hands[c->low+3]+=3;
			h->hands[c->low+4]+=3;
	
	if (c->type == 531)
		return true;
	else
		return false;
}




int is_33332222(POKERS *h, COMBO_OF_POKERS *c )
{
	int j=0;
	c->type = -1;
	for (int i = h->begin; i<= CEIL_A(h->end)-3; i++)
	{
		if ( h->hands[i] ==3 && h->hands[i+1] ==3 && h->hands[i+2] ==3  && h->hands[i+3] ==3 )
		{
			c->type = 33332222;
			c->low = i;
			c->len = 4;
			break;
		}
	}
	if (c->type != 33332222)
		return false;	
	for (int i = h->begin; i<= (h->end); i++)
	{
		if ( h->hands[i] ==2 )
			c->three_desc[j++] = i;
		if(j>=4)
			break;

	}
	if ( j!=4)
		c->type = -1;
	if (c->type == 33332222)
		return true;
	else
		return false;
}

int is_combo(POKERS *src, COMBO_OF_POKERS * c)
{
	c->type = -1;
	c->control=0;
	POKERS t,*h=&t;
	*h=*src;	
	int number =0 ;	
	if ( h->total> 0 )
	{
		int number =h->total;
		c->len = h->total;
		c->low = h->begin ;
		if (number>4 && h->end <=Pa)
		{
			if (is_series(h,number) )
			{
				c->type=SINGLE_SERIES;
				return true;
			}
			else if (number%2==0 && is_doubleseries(h) )
			{
				c->type=PAIRS_SERIES;
				c->len/=2;
				return true;
			}
			else if (number%3==0 && is_threeseries(h) )
			{
				c->type=THREE_SERIES;
				c->len/=3;
				return true;
			}
		}
		switch (h->total)
		{
		case 1 :
			c->type =SINGLE;
			break;
		case 2 :
			if (h->hands[h->begin]==2) {
				c->type =PAIRS;
				c->len>>=1;
			}
			else if (h->begin == LIT_JOKER) c->type= ROCKET;
			break;
		case 3 :
			if (h->hands[h->begin]==3) c->type =THREE;
			break;
		case 4 :
			if (h->hands[h->begin]==4){ 
				c->type =BOMB;
				c->three_desc[0] = 0;
			}
			else if (h->hands[h->begin]==3 || h->hands[h->end]==3)
			{
				c->type = 31;
				c->len = 1;
				if (h->hands[h->begin]==3)
				{
					c->three_desc[0] =   h->end;
					c->low = h->begin;
				}
				else
				{
					c->three_desc[0] =   h->begin;
					c->low = h->end;
				}
			}
			break;
		case 5 :
			is_32(h,c);
			break;
		case 6 :
			is_411(h,c);
			break;
		case 7 :
			break;
		case 8 :
			if ( !is_3311(h,c))
				is_422(h,c);
			break; //3311
		case 9 :
			break;
		case 10 :
			is_3322(h,c);
			break;//3322
		case 11 :
			break;
		case 12 :
			is_333111(h,c);
			break; //333111
		case 13 :
			break;
		case 15 :
			is_333222(h,c);
			break;
		case 16 :
			is_33331111(h,c);
			break; //33331111
		case 20 :
			if (!is_33332222(h,c))
				is_531(h,c);
			break; //33332222
		default:
			break;

		}
		// return true;
	}
	if (c->type!=-1)
		return true;
	else
		return false;
}

//return 1 if c's priority is higher 
int check_prior_first( COMBO_OF_POKERS *c, COMBO_OF_POKERS *a)
{
	if(a->type == NOTHING)
		return 1; 

	if (c->type == a->type && c->low >a->low )
		return 1;	

	switch( get_combo_number(c))
	{
	case 1 :
	case 2 :
	case 3 :
		if (c->low >a->low)
			return 1;
		break;	
	case 4 :
		if (c->type ==BOMB && (c->low >a->low || a->type!=BOMB))
			return 1;	  		
		else if (c->type ==31 && c->low >a->low && a->type ==31 )
			return 1;
		break;
	case 5 :
		if (c->type ==32 && (a->type!=32))
			return 1;	  		
		break;
	case 6 :
		if (c->type ==PAIRS_SERIES && (a->type!=PAIRS_SERIES))
			return 1;	  		
		else if (c->type ==411 && (a->type!=411 && a->type!=PAIRS_SERIES))
			return 1;	  		
		else if (c->type ==THREE_SERIES && (a->type!=411 && a->type!=PAIRS_SERIES && 
			a->type!=THREE_SERIES  ))
			return 1;
		break;
	case 7 :
		break;
	case 8 :
		if (c->type ==422 && (a->type!=422))
			return 1;	  		
		else if (c->type ==3311 && (a->type!=422 && a->type!=3311))
			return 1;	  		
		else if (c->type ==PAIRS_SERIES && (a->type!=422 && a->type!=3311 && 
			a->type!=PAIRS_SERIES  ))
			return 1;
		break;
	case 9 :
		if (c->type ==THREE_SERIES && (a->type!=THREE_SERIES))
			return 1;	  		
		break;
	case 10 :
		if (c->type ==3322 && (a->type!=3322))
			return 1;	  		
		else if (c->type ==PAIRS_SERIES && (a->type!=3322 && a->type!=PAIRS_SERIES))
			return 1;	  		
		break;
	case 11 :
		break;
	case 12 :
		if (c->type ==THREE_SERIES && (a->type!=THREE_SERIES))
			return 1;	  		
		else if (c->type ==333111 && (a->type!=333111 && a->type!=THREE_SERIES))
			return 1;	
		else if (c->type ==PAIRS_SERIES && (a->type!=PAIRS_SERIES &&a->type!=333111 && a->type!=THREE_SERIES))
			return 1;	
	case 13 :
		break;
	case 15 :
		if (c->type ==THREE_SERIES && (a->type!=THREE_SERIES))
			return 1;	  		
		else if (c->type ==333222 && (a->type!=333222 && a->type!=THREE_SERIES))
			return 1;	
		break;
	case 16 :
		break; //33331111
	case 20 :
		if (c->type ==33332222 && (a->type!=33332222))
			return 1;	  		
		break;
	default:
		break;
	}
	return 0;
}


int check_combo_with_rules(POKERS *h, COMBO_OF_POKERS * out, int first, COMBO_OF_POKERS * pre)
{
	COMBO_OF_POKERS c1,*c=&c1;
	if(first)
		out->type = NOTHING;
	else
		*out=*pre;


	if( HUN_NUMBER(h)==h->total || h->begin == h->end)
	{
		// if(first)
		{
			out->low = h->begin;	   
			switch (h->total)
			{
			case 1 :
				out->type =SINGLE;
				break;
			case 2 :
				out->type =PAIRS;
				break;
			case 3 :
				out->type =THREE;
				break;
			case 4 :
				out->type =BOMB;
				out->three_desc[0] = h->begin==h->hun?2:1;

				break;			
			}
			if (first)
				return 0;
			else if(check_combo_a_Big_than_b(out,pre))
				return 0;
			else
				return -2;
		}	   	
	}

	if(HUN_NUMBER(h)>0)
	{
		int ben_hun = 0;
		//set first hun values  
		for( int i1=P3; i1<=P2; i1++ )
		{	    
			SET_HUN_TO_i(h, i1);
			PRINTF(VERBOSE, "==set first hun to %c\n", poker_to_char(i1));
			if(IS_HUN(h, i1)) ben_hun++;	
			if(HUN_NUMBER(h)>ben_hun)
			{
				//set second hun values  
				for( int i2=P3; i2<=P2; i2++ )
				{
					SET_HUN_TO_i(h, i2);
					PRINTF(VERBOSE, "\t==set second hun to %c\n", poker_to_char(i2));					
					if(IS_HUN(h, i2)) ben_hun++;				  
					if(HUN_NUMBER(h)>ben_hun)
					{
						//set 3rd hun values  
						for( int i3=P3; i3<=P2; i3++ )
						{
							SET_HUN_TO_i(h, i3);
							PRINTF(VERBOSE, "\t==set 3rd hun to %c\n", poker_to_char(i3));					
							if(IS_HUN(h, i3)) ben_hun++;				  
							if(HUN_NUMBER(h)>ben_hun)
							{
								//set 4th hun values  
								for( int i4=P3; i4<=P2; i4++ )
								{     
									SET_HUN_TO_i(h, i4);
									if( is_combo(h,c))
									{
										if(first)
										{
											if(check_prior_first(c,out))
												*out=*c;
										}
										else
										{
											if(check_combo_a_Big_than_b(c,out))
												*out=*c;
										}
									}
									SET_i_to_HUN(h, i4);			 
								}
							}
							else  if( is_combo(h,c)) 
							{
								if(first)
								{
									if(check_prior_first(c,out))
										*out=*c;
								}
								else
								{
									if(check_combo_a_Big_than_b(c,out))
										*out=*c;
								}
							}
							if(IS_HUN(h, i3)) ben_hun--;	
							SET_i_to_HUN(h, i3);
						}
					}
					else   if( is_combo(h,c)) 
					{
						if(first)
						{
							if(check_prior_first(c,out))
								*out=*c;
						}
						else
						{
							if(check_combo_a_Big_than_b(c,out))
								*out=*c;
						}
					}
					if(IS_HUN(h, i2)) ben_hun--;	
					SET_i_to_HUN(h, i2);										
				}		  
			}
			else  if( is_combo(h,c)) {
				if(first)
				{
					if(check_prior_first(c,out))
						*out=*c;
				}
				else
				{
					if(check_combo_a_Big_than_b(c,out))
						*out=*c;
				}
			}
			if(IS_HUN(h, i1)) ben_hun--;	
			SET_i_to_HUN(h, i1);
		}
	}
	else if( is_combo(h,c)) 
	{
		if(first)
		{
			return 0;
		}
		else
		{
			if(check_combo_a_Big_than_b(c,pre)){
				*out=*c;
				return 0;
			}else
				return -2;
		}
	}
	else
		return -1;

	//final check
	if(first)
	{
		if(out->type!=NOTHING)
			return 0;
		else 
			return -1;
	}
	else
	{
		if(check_combo_a_Big_than_b(out,pre))
			return 0;
		else
			return -2;
	}
}



int is_combo_hun(POKERS *h, COMBO_OF_POKERS * c)
{
	if(HUN_NUMBER(h)>0)
	{
		int ben_hun = 0;
		//set first hun values  
		for( int i1=P3; i1<=P2; i1++ )
		{	    
			SET_HUN_TO_i(h, i1);
			PRINTF(VERBOSE, "==set first hun to %c\n", poker_to_char(i1));
			if(IS_HUN(h, i1)) ben_hun++;	
			if(HUN_NUMBER(h)>ben_hun)
			{
				//set second hun values  
				for( int i2=P3; i2<=P2; i2++ )
				{
					SET_HUN_TO_i(h, i2);
					PRINTF(VERBOSE, "\t==set second hun to %c\n", poker_to_char(i2));					
					if(IS_HUN(h, i2)) ben_hun++;				  
					if(HUN_NUMBER(h)>ben_hun)
					{
						//set 3rd hun values  
						for( int i3=P3; i3<=P2; i3++ )
						{
							SET_HUN_TO_i(h, i3);
							PRINTF(VERBOSE, "\t==set 3rd hun to %c\n", poker_to_char(i3));					
							if(IS_HUN(h, i3)) ben_hun++;				  
							if(HUN_NUMBER(h)>ben_hun)
							{
								//set 4th hun values  
								for( int i4=P3; i4<=P2; i4++ )
								{     
									SET_HUN_TO_i(h, i4);
									if( is_combo(h,c)) return 1;
									SET_i_to_HUN(h, i4);			 
								}
							}
							else  if( is_combo(h,c)) return 1;	
							if(IS_HUN(h, i3)) ben_hun--;	
							SET_i_to_HUN(h, i3);
						}
					}
					else   if( is_combo(h,c)) return 1;	
					if(IS_HUN(h, i2)) ben_hun--;	
					SET_i_to_HUN(h, i2);										
				}		  
			}
			else  if( is_combo(h,c)) return 1;
			if(IS_HUN(h, i1)) ben_hun--;	
			SET_i_to_HUN(h, i1);
		}
	}
	else{
		return is_combo(h, c);	
	}

	return 0 ;
}


int is_input_combo(int * p , COMBO_OF_POKERS* c)
{
	POKERS a,b;
	int num =read_poker_int(p,&a);

	return  is_combo(&a,c)?num:0;
}






BOOL getBomb(POKERS* h, COMBO_OF_POKERS * p) //
{
	int total=h->total;
	p->len = 1;
	for (int i=h->begin; i<=h->end; i++)
	{
		if (h->hands[i] >= 4) //bomb got
		{
			p->type = BOMB;
			p->three_desc[0]=0;
			p->low = i;
			return true;
		}
	}

	if (h->hands[LIT_JOKER]==1 &&h->hands[BIG_JOKER]==1)
	{
		p->type = ROCKET;
		p->low = LIT_JOKER;
		return true;
	}
	return false;
}


BOOL getThree(POKERS* h, COMBO_OF_POKERS * p) //
{
	int total=h->total;
	for (int i=h->begin; i<=h->end; i++)
	{
		if (h->hands[i] == 3) //Got Three
		{
			p->type = THREE;
			p->low  = i;
			return true;
		}
	}
	return false;
}



BOOL getThreeSeries(POKERS* h, COMBO_OF_POKERS * p)
{
	for (int i = h->begin; i<= (CEIL_A(h->end)- 1);)
	{
		int j;
		for (j=i; j<=CEIL_A(h->end); j++) {
			if ( h->hands[j]<3) {
				break;
			}
		}
		if ( j > i+1 )
		{
			p->type = THREE_SERIES;
			p->low = i;
			p->len = j-i;
			return true;
		}

		i=j+1;

	}
	return false;
}

BOOL getDoubleSeries(POKERS* h, COMBO_OF_POKERS * p)
{
	for (int i = h->begin; i<= (CEIL_A(h->end) - 2);)
	{
		int j;
		for (j=i; j<=CEIL_A(h->end); j++) {
			if ( h->hands[j]<2) {
				break;
			}
		}
		if ( j >i+2  )
		{
			p->type = PAIRS_SERIES;
			p->low = i;
			p->len = j-i;
			return true;
		}
		i=j+1;
	}
	return false;
}

//双顺调优
BOOL updateDoubleSeries(POKERS* h, COMBO_OF_POKERS * p)
{
	//from head to tail.
	while ( p->len >3)
	{
		int end=p->len+p->low-1;
		if ( h->hands[end] > h->hands[p->low] )
		{   //remove tail
			h->hands[end]+=2;
			p->len--;
			//end--;
		}
		else if (h->hands[p->low] >0)
		{
			h->hands[p->low]+=2;
			p->low++;
			p->len--;
		}
		else
			break;
	}
	return false;
}

BOOL updateDoubleSeries1(POKERS* h, COMBO_OF_POKERS * p)
{
	if (p->len ==3)
	{
		if ( h->hands[p->low]==1 && h->hands[p->low+2]==1) // cl
			return true;
	}
	return false;
}

BOOL getSeries(POKERS* h, COMBO_OF_POKERS * p)
{
	for (int i = h->begin; i<= (h->end - 4);)
	{
		int j;
		for (j=i; j<=CEIL_A(h->end); j++) {
			if ( h->hands[j]<1) {
				break;
			}
		}
		if ( j > i+4  )
		{
			p->type = SINGLE_SERIES;
			p->low = i;
			p->len = j-i;
			//			p->number = len;
			return true;
		}

		i=j+1;
	}
	return false;
}

BOOL getBigBomb(POKERS* h, COMBO_OF_POKERS * p, COMBO_OF_POKERS * a) //
{
	int total=h->total;
	for (int i=a->low+1; i<=h->end; i++)
	{
		if (h->hands[i] == 4) //bomb got
		{
			p->type = BOMB;
			p->three_desc[0]=0;			
			p->low = i;
			return true;
		}
	}
	if (a->low<LIT_JOKER && h->hands[LIT_JOKER]==1 &&h->hands[BIG_JOKER]==1)
	{
		p->type = ROCKET;
		p->low = LIT_JOKER;
		return true;
	}
	return false;
}

BOOL getBigBomb_hun(POKERS* h, COMBO_OF_POKERS * p, COMBO_OF_POKERS * a) //
{
	switch (a->three_desc[0])
	{
	default:
	case 0: 
		if(HUN_NUMBER(h)){	
			for (int i=a->low+1; i<=min(h->end,P2); i++)
			{
				if(IS_HUN(h, i)) continue;
				if (h->hands[i] <4 &&h->hands[i]>0 &&
					h->hands[i] >= (4-HUN_NUMBER(h))) //got hun bomb
				{
					p->type = BOMB;
					p->three_desc[0]=0;
					p->low = i;
					return true;
				}
			}   	 
		}
	case 1:
		for (int i=a->three_desc[0]==1?a->low+1:0; i<=h->end; i++)
		{
			if( IS_HUN(h, i) ) //skip hun at first
				continue;
			if (h->hands[i] ==4 ) //got hun bomb
			{
				p->type = BOMB;
				p->three_desc[0]=1;
				p->low = i;
				return true;
			}
		}  
		if (h->hands[h->hun] ==4 ) //got pure hun bomb
		{
			p->type = BOMB;
			p->three_desc[0]=2;
			p->low = h->hun;
			return true;
		}				  
	case 2:
		if (a->low<LIT_JOKER && h->hands[LIT_JOKER]==1 &&h->hands[BIG_JOKER]==1)
		{
			p->type = ROCKET;
			p->low = LIT_JOKER;
			return true;
		}			
	}	
	return false;
}

BOOL getBomb_hun(POKERS* h, COMBO_OF_POKERS * p) //
{
	switch (0)
	{
	default:
	case 0: 
		if(HUN_NUMBER(h)){	
			for (int i=h->begin; i<=min(h->end,P2); i++)
			{
				if( IS_HUN(h, i) ) //skip hun at first
					continue;
				if (h->hands[i] <4 && h->hands[i] >0 &&
					h->hands[i] >= (4-HUN_NUMBER(h))) //got hun bomb
				{
					p->type = BOMB;
					p->three_desc[0]=0;
					p->low = i;
					return true;
				}
			}   	 
		}
	case 1:
		for (int i=0; i<=h->end; i++)
		{
			if( IS_HUN(h, i) ) //skip hun at first
				continue;
			if (h->hands[i] ==4 ) //got hun bomb
			{
				p->type = BOMB;
				p->three_desc[0]=1;
				p->low = i;
				return true;
			}
		}  
		if (h->hands[h->hun] ==4 ) //got pure hun bomb
		{
			p->type = BOMB;
			p->three_desc[0]=2;
			p->low = h->hun;
			return true;
		}				  
	case 2:
		if ( h->hands[LIT_JOKER]==1 &&h->hands[BIG_JOKER]==1)
		{
			p->type = ROCKET;
			p->low = LIT_JOKER;
			return true;
		}			
	}	
	return false;
}

int get_bomb_numbers_hun(POKERS* t)
{
	POKERS tmp=*t,*h=&tmp;
	int number =0;

	for (int i=h->begin; i<=min(h->end,P2); i++)
	{
		if( IS_HUN(h, i) ) //skip hun at first
			continue;
		if (h->hands[i] ==4 ) //got hun bomb
		{
			h->hands[i] =0;
			number++;
		}
	}  

	if ( h->hands[LIT_JOKER]==1 &&h->hands[BIG_JOKER]==1)
	{
		number++;
	}			

	if( HUN_NUMBER(h))
	{
		for (int i=h->begin; i<=min(h->end,P2); i++)				  	
		{
			if( IS_HUN(h, i) ) //skip hun at first
				continue;
			if (h->hands[i] ==3 && HUN_NUMBER(h) ) //got hun bomb
			{
				h->hands[i] =0;
				h->hands[h->hun] --;						  
				number++;
			}
		}  
	}	

	if( HUN_NUMBER(h)>=2)			
	{
		for (int i=h->begin; i<=min(h->end,P2); i++)
		{
			if( IS_HUN(h, i) ) //skip hun at first
				continue;
			if (h->hands[i] ==2 && HUN_NUMBER(h)>=2 ) //got hun bomb
			{
				h->hands[i] =0;
				h->hands[h->hun] -=2;						  
				number++;
			}
		}  
	}	

	if( HUN_NUMBER(h)>=3)
	{
		for (int i=h->begin; i<=min(h->end,P2); i++)
		{
			if( IS_HUN(h, i) ) //skip hun at first
				continue;
			if (h->hands[i] ==1 && HUN_NUMBER(h)>=3 ) //got hun bomb
			{
				number++;
				h->hands[h->hun] -=3;		  
				break;		  
			}
		}	            
	}
	if( HUN_NUMBER(h)>3)
		number++;

	return number;
}

int get_bomb_numbers(POKERS* t)
{
	POKERS tmp=*t,*h=&tmp;
	int number =0;

	for (int i=h->begin; i<=min(h->end,P2); i++)
	{
		if (h->hands[i] ==4 ) //got hun bomb
		{
			number++;
		}
	}  

	if ( h->hands[LIT_JOKER]==1 &&h->hands[BIG_JOKER]==1)
	{
		number++;
	}	
	return number;				 
}

//fixme: ?? for single??
BOOL getBigSingle_hun(POKERS* h, COMBO_OF_POKERS * p,int start, int end, int number )
{
	for (int i = start ; i<= end; i++)
	{
		if( IS_HUN(h, i) ) //skip hun at first
			continue;
		if (h->hands[i] >= number
			|| (h->hands[i] >= number-HUN_NUMBER(h) && h->hands[i] >0 ) 
			)
		{
			p->type = number==3 ? THREE: (number==2 ? PAIRS: SINGLE) ;
			p->low  = i;
			p->len = 1;
			return true;
		}
	}

	if(HUN_NUMBER(h)>=number && h->hun > start)	
	{
		p->type = number==3 ? THREE: (number==2 ? PAIRS: SINGLE) ;
		p->low  = h->hun;
		p->len = 1;    
		return true;
	}

	return false;
}

BOOL getBigThree(POKERS* h, COMBO_OF_POKERS * p,COMBO_OF_POKERS* a) //
{
	for (int i=a->low+1; i<=h->end; i++)
	{
		if (h->hands[i] == 3) //Got Three,doesn't check bomb..
		{
			p->type = THREE;
			p->low  = i;
			p->len = 1;
			return true;
		}
	}
	return false;
}

BOOL getBigSingle(POKERS* h, COMBO_OF_POKERS * p,int start, int end, int number )
{
	for (int i = start ; i<= end; i++)
	{
		if (h->hands[i] >= number) //Got Three
		{
			p->type = number==3 ? THREE: (number==2 ? PAIRS: SINGLE) ;
			p->low  = i;
			p->len = 1;
			return true;
		}
	}
	return false;
}

BOOL getBigSeries_hun(POKERS* h, COMBO_OF_POKERS * p,int start, int end, int number, int len ) //
{
	end =CEIL_A(end);
	POKERS t={0};   	
	for (int j=start; j<= start+len-1; j++) {
		t.hands[j]=number;
	}		
	for (int i = start ; i<= (end -len + 1);)
	{
		if(is_subset_poker_hun(&t, h))
		{
			p->type = number==3 ? THREE_SERIES: (number==2 ? PAIRS_SERIES: SINGLE_SERIES) ;
			p->low = i;
			p->len = len;	 
			return true;
		}
		t.hands[i]=0;
		t.hands[i+len]=number;	
		i++;	
	}
	return false;
}

BOOL getBigSeries(POKERS* h, COMBO_OF_POKERS * p,int start, int end, int number, int len ) //
{
	end =CEIL_A(end);
	for (int i = start ; i<= (end -len + 1);)
	{
		int j;
		for (j=i; j<= end; j++) {
#if 1
			if(number==3 && h->hands[j]!=3) break; //don't get bomb
#endif
			if ( h->hands[j]<number) break;
		}
		if ( j >= i+ len )
		{
			p->type = number==3 ? THREE_SERIES: (number==2 ? PAIRS_SERIES: SINGLE_SERIES) ;
			p->low = i;
			p->len = len;
			//			p->number = p->len * number;
			return true;
		}
		i=j+1;
	}
	return false;
}

BOOL getSingleSeries(POKERS* h, COMBO_OF_POKERS * p,int start, int end, int number ) //
{
	end =CEIL_A(end);
	for (int i = start ; i<= (end -4);)
	{
		int j;
		for (j=i; j<= end; j++) {
			if ( h->hands[j]<1) break;
			else if ( j-i >=4 && h->hands[j]>1)
			{
				j++;
				break;
			}
		}
		if ( j >= i+ 5 )
		{
			p->type =  SINGLE_SERIES;
			p->low = i;
			p->len = j-i;
			//  	    if( p->len > 5 )
			//			p->number = p->len * number;
			return true;
		}
		i=j+1;
	}
	return false;
}


//only used for known_other_pokers

BOOL getSmallSingle(POKERS* h, COMBO_OF_POKERS * p,int end, int number )
{
	for (int i = h->begin ; i< end; i++)
	{
		if (h->hands[i] >= number && h->hands[i]!=4)
		{
			p->type = number==3 ? THREE: (number==2 ? PAIRS: SINGLE) ;
			p->low  = i;
			p->len = 1;
			return true;
		}
	}
	return false;
}

BOOL getSmallSingle_hun(POKERS* h, COMBO_OF_POKERS * p,int end, int number )
{
	for (int i = h->begin ; i< end; i++)
	{
		if(IS_HUN(h, i))
			continue;
		if (h->hands[i] >= number)  
		{
			p->type = number==3 ? THREE: (number==2 ? PAIRS: SINGLE) ;
			p->low  = i;
			p->len = 1;
			return true;
		}
	}
	//check with hun
	for (int i = h->begin ; i< end; i++)
	{
		if(IS_HUN(h, i))
			continue;
		if (h->hands[i] >= number -HUN_NUMBER(h))  
		{
			p->type = number==3 ? THREE: (number==2 ? PAIRS: SINGLE) ;
			p->low  = i;
			p->len = 1;
			return true;
		}
	}	
	return false;
}


BOOL getSmallSeries(POKERS* h, COMBO_OF_POKERS * p,int end, int number, int len ) //
{
	end =CEIL_A(end);
	//    for (int i = h->begin ; i<=  (end -len + 1);)
	for (int i = h->begin ; i< end;)
	{
		int j;
		for (j=i; j< i+len; j++) {
			if ( h->hands[j]<number) break;
		}
		if ( j >= i+ len )
		{
			p->type = number==3 ? THREE_SERIES: (number==2 ? PAIRS_SERIES: SINGLE_SERIES) ;
			p->low = i;
			p->len = len;
			//			p->number = p->len * number;
			return true;
		}
		i=j+1;
	}
	return false;
}

BOOL getSmallSeries_hun(POKERS* h, COMBO_OF_POKERS * p,int end, int number, int len ) //
{
	end =CEIL_A(end);
	POKERS t={0};   	
	for (int j=0; j<= len-1; j++) {
		t.hands[j]=number;
	}		
	for (int i = 0 ; i<= end-1;)
	{
		if(is_subset_poker_hun(&t, h))
		{
			p->type = number==3 ? THREE_SERIES: (number==2 ? PAIRS_SERIES: SINGLE_SERIES) ;
			p->low = i;
			p->len = len;	 
			return true;
		}
		t.hands[i]=0;
		t.hands[i+len]=number;	
		i++;	 
	}
	return false;
}


int get_opp_bomb_number(GAME* g)
{
	if(g->known_others_poker)
	{
		int  (*func)(POKERS*); 
		if(g->hun == -1) {
			func = get_bomb_numbers;
		}else{
			func = get_bomb_numbers_hun;
		}

		COMBO_OF_POKERS *c,tmp;
		c=&tmp;
		if(g->players[CUR_PLAYER]->id == LORD)
		{
			return func(g->players[DOWN_PLAYER]->h)	+ func(g->players[UP_PLAYER]->h);
		}
		else if(g->players[CUR_PLAYER]->id == UPFARMER)
		{
			return func(g->players[DOWN_PLAYER]->h);
		}
		else
		{
			return func(g->players[UP_PLAYER]->h);
		}
	}
	else{
		return 0;	
	}
}
int opp_hasbomb(GAME * g)
{
	if(g->known_others_poker)
	{
		BOOL  (*func)(POKERS*,COMBO_OF_POKERS *); 
		if(g->hun == -1) 
			func = getBomb;
		else
			func = getBomb_hun;    

		COMBO_OF_POKERS *c,tmp;
		c=&tmp;
		if(g->players[CUR_PLAYER]->id == LORD)
		{
			if(func(g->players[DOWN_PLAYER]->h,c)
				||func(g->players[UP_PLAYER]->h,c) )
			{
				return true;
			}
			else
				return false;
		}
		else    if(g->players[CUR_PLAYER]->id == UPFARMER)
		{
			if(!func(g->players[DOWN_PLAYER]->h,c))
			{
				return false;
			}
			else
				return true;
		}
		else
		{
			if(!func(g->players[UP_PLAYER]->h,c))
			{
				return false;
			}
			else
				return true;
		}
	}
	else
	{
		COMBO_OF_POKERS *c,tmp;
		c=&tmp;
		PLAYER *pl = g->players[CUR_PLAYER];
		if(getBomb(g->players[CUR_PLAYER]->opps,c))
		{
			//double check
			int check_num =4;
			if(pl->opps->hands[BIG_JOKER]==1 &&
				pl->opps->hands[LIT_JOKER]==1 )
			{
				check_num=2;
			}		   		   	  

			if(g->players[CUR_PLAYER]->id == LORD &&
				pl->oppDown_num<check_num && pl->oppUp_num <check_num )
			{
				return false;
			}
			if(pl->id == UPFARMER && pl->oppDown_num<check_num )
			{
				return false;
			}              		   
			if(pl->id == DOWNFARMER && pl->oppUp_num<check_num )
			{
				return false;
			}              		

			return true;
		}
		else
			return false;				        
	}		
	return false;
}

BOOL  is_combo_biggest_sure(GAME * g, COMBO_OF_POKERS * a)
{
	COMBO_OF_POKERS tmp,*c=&tmp;
	PLAYER* pl=g->players[CUR_PLAYER];
	if(g->known_others_poker)
	{
		if(g->players[CUR_PLAYER]->id == LORD)
		{
			if(find_a_bigger_combo_in_hands(g->players[DOWN_PLAYER]->h,c,a)
				||find_a_bigger_combo_in_hands(g->players[UP_PLAYER]->h,c,a) )
			{
				return false;
			}
			else
				return true;
		}
		else    if(g->players[CUR_PLAYER]->id == UPFARMER)
		{
			if(find_a_bigger_combo_in_hands(g->players[DOWN_PLAYER]->h,c,a))
			{
				return false;
			}
			else
				return true;
		}
		else
		{
			if(find_a_bigger_combo_in_hands(g->players[UP_PLAYER]->h,c,a))
			{
				return false;
			}
			else
				return true;
		}
	}
	else
	{
		int num = get_combo_number(a);
		if(a->type!=BOMB &&a->type!=ROCKET && opp_hasbomb(g))
			return false;
		if(pl->id == LORD)
		{
			if(find_a_bigger_combo_in_hands(pl->opps,c,a))
			{
				//check again
				if(pl->oppDown_num < num && pl->oppUp_num < num )
					return true;
				return false;
			}
			else
				return true;
		}
		else    if(g->players[CUR_PLAYER]->id == UPFARMER)
		{
			if(find_a_bigger_combo_in_hands(pl->opps,c,a))
			{
				//check again
				if(pl->oppDown_num < num  )
					return true;
				return false;
			}
			else
				return true;
		}
		else
		{
			if(find_a_bigger_combo_in_hands(pl->opps,c,a))
			{
				//check again
				if(pl->oppUp_num < num  )
					return true;
				return false;
			}
			else
				return true;
		}
	}
}

BOOL  is_combo_biggest_sure_without_bomb(GAME * g, COMBO_OF_POKERS * a)
{
	COMBO_OF_POKERS tmp,*c=&tmp;
	PLAYER* pl=g->players[CUR_PLAYER];
	GAME* game=g;
	if(g->known_others_poker)
	{
		COMBO_OF_POKERS tmp;
		if(game->player_type == LORD)
		{
			if( find_a_bigger_combo_in_hands(game->players[UP_PLAYER]->h,&tmp, a) && tmp.type!=ROCKET && tmp.type!=BOMB)
				return false;
			else if ( find_a_bigger_combo_in_hands(game->players[DOWN_PLAYER]->h,&tmp, a)&& tmp.type!=ROCKET && tmp.type!=BOMB)
				return false;
			else
				return true;
		}
		else if(game->player_type == UPFARMER )
		{
			if ( find_a_bigger_combo_in_hands(game->players[DOWN_PLAYER]->h,&tmp, a)&& tmp.type!=ROCKET && tmp.type!=BOMB)
				return false;
			else
				return true;
		}
		else if(game->player_type == DOWNFARMER )
		{
			if ( find_a_bigger_combo_in_hands(game->players[UP_PLAYER]->h,&tmp, a)&& tmp.type!=ROCKET && tmp.type!=BOMB)
				return false;
			else
				return true;
		}
		else
			return false;
	}
	else
	{
		int num = get_combo_number(a);
		if(pl->id == LORD)
		{
			if(find_a_bigger_combo_in_hands(pl->opps,c,a)&&NOT_BOMB(c))
			{
				if(pl->oppDown_num < num && pl->oppUp_num < num )
					return true;
				return false;
			}
			else
				return true;
		}
		else    if(g->players[CUR_PLAYER]->id == UPFARMER)
		{
			if(find_a_bigger_combo_in_hands(pl->opps,c,a)&&NOT_BOMB(c))
			{
				if(pl->oppDown_num < num  )
					return true;
				return false;
			}
			else
				return true;
		}
		else
		{
			if(find_a_bigger_combo_in_hands(pl->opps,c,a)&&NOT_BOMB(c))
			{
				if(pl->oppUp_num < num  )
					return true;
				return false;
			}
			else
				return true;
		}
	}
}
//检查本手牌是否最大
//不考虑炸弹
BOOL is_combo_biggest(GAME* game,POKERS *opp, COMBO_OF_POKERS * c , int opp1_num, int opp2_num,int lower)
{
	BOOL res = true;
	static   char BigSNum[8]= {7,9,11,13,15,17,18,18};
	int num = get_combo_number(c);
	//check number
	opp1_num = game->players[CUR_PLAYER]->oppDown_num;
	opp2_num = game->players[CUR_PLAYER]->oppUp_num;
	if(game->player_type == UPFARMER )
		opp2_num =0;
	if(game->player_type == DOWNFARMER)
		opp1_num =0;
	if (opp1_num <num && opp2_num < num)
		return res;

	if (game->known_others_poker)
	{

		COMBO_OF_POKERS tmp;
		if(game->player_type == LORD)
		{
			if( find_a_bigger_combo_in_hands(game->players[UP_PLAYER]->h,&tmp, c) && tmp.type!=ROCKET && tmp.type!=BOMB)
				return false;
			else if ( find_a_bigger_combo_in_hands(game->players[DOWN_PLAYER]->h,&tmp, c)&& tmp.type!=ROCKET && tmp.type!=BOMB)
				return false;
			else
				return true;

		}
		else if(game->player_type == UPFARMER )
		{
			if ( find_a_bigger_combo_in_hands(game->players[DOWN_PLAYER]->h,&tmp, c)&& tmp.type!=ROCKET && tmp.type!=BOMB)
				return false;
			else
				return true;
		}
		else if(game->player_type == DOWNFARMER )
		{
			if ( find_a_bigger_combo_in_hands(game->players[UP_PLAYER]->h,&tmp, c)&& tmp.type!=ROCKET && tmp.type!=BOMB)
				return false;
			else
				return true;
		}
		else
			return false;
	}

	switch ( c->type )
	{
	case ROCKET:
	case BOMB:
	case PAIRS_SERIES:
	case THREE_SERIES:
	default:
		break;

	case SINGLE_SERIES:
		if ( opp1_num+opp2_num > BigSNum[c->len -5 ])
		{
			int start = c->low+1;
			int len=c->len;
			int end=min(Pa,opp->end);
			for (int i = start ; i<= (end -len + 1);)
			{
				int j,sum=0;
				for (j=i; j<= end; j++) {
					if ( opp->hands[j]<1) break;
					//else  if(opp->hands[j] ==1)
					//	sum++;
				}
				if ( j >= i+ len )
				{
					int loop = j-i-len;
					for ( int k=0; k<=loop ; k++)
					{
						sum = 0;
						for (int m=i+k; m<j ; m++)
							if (opp->hands[m] ==1)
								sum++;
						if (sum<4)
							res = false;
					}
				}
				i=j+1;
			}
		}
		break;

	case THREE_TWO:
	case THREE_ONE:
		if (c->len > 1) break;
	case THREE:
		for (int k=c->low+1; k<=min(P2,opp->end); k++)
		{
			if ( opp->hands[k]>=3 ) {
				res=false;
				break;
			}
		}
		break;
	case PAIRS :
		for (int k=c->low+1; k<=min(P2,opp->end); k++)
		{
			if ( opp->hands[k]>=2 ) {
				res=false;
				break;
			}
		}
		break;
	case SINGLE :
		for (int k=c->low+1; k<=opp->end; k++)
		{
			if ( opp->hands[k]>=1 ) {
				res=false;
				break;
			}
		}
		break;
	}
	return res;
}




int get_lowest_controls( POKERS *h,int number )
{
	int high = h->end;
	if ( h->total <= number)
		return h->begin;
	int sum=0,j=0,j1=0;
	int i;
	for ( i= high; i>0 && number>0; i--)
	{
		number -=h->hands[i];
		if (number<=0)
			return i;
	}
	return 0;
}


int calc_controls( POKERS *h, POKERS *opp ,int number )
{
	int high = opp->end > h->end ? opp->end : h->end;
	int sum=0,j=0,j1=0,HighPokers1[9], HighPoker[9];
	for ( int i= high; i>0 && (j+j1)< number ; i--)
	{
		for (int k=0; k<h->hands[i]; k++)
		{
			HighPoker[j++] = i;
		}
		for (int k=0; k<opp->hands[i]; k++)
		{
			HighPokers1[j1++] = i;
		}
	}
	number = j+j1;
	j--;
	j1--;

#if 0
	DBG(
		PRINTF(LEVEL,"\ncur  high:  ");
	for (int k=j; k>=0; k--)
		print_one_poker(HighPoker[k]);
	PRINTF(LEVEL,"\n opps  high: ");
	for (int k=j1; k>=0; k--)
		print_one_poker(HighPokers1[k]);

	//PRINTF(LEVEL,"\n round:\n ");
	)
#endif
		//j;j1;
		int M=0,N=0;
	//int bak_j=j,bak_j1=bak_j1;
	int last;
	for ( ; j >= 0 && j1>=0;)
	{
		// simulate round
		last = HighPoker[j];
		j--;
		//DBG(    print_one_poker(last);  PRINTF(LEVEL,"\n");      )
search_0:
		for (int k=j1; k>=0; k--)
			if ( HighPokers1[k]> last)
			{
				last =HighPokers1[k];
				// DBG(    print_one_poker(last);  PRINTF(LEVEL,"\n");      )
				for (int p=k; p<j1 ; p++) //remove poker
					HighPokers1[p]= HighPokers1[p+1];
				j1--;
				goto search_1;
			}
			N++;
			continue;
search_1:
			for (int k=j; k>=0; k--)
				if ( last < HighPoker[k])
				{
					last=HighPoker[k];
					// DBG(    print_one_poker(last);  PRINTF(LEVEL,"\n");      )
					for (int p=k; p<j ; p++) //remove poker
						HighPoker[p]= HighPoker[p+1];
					j--;
					goto search_0;
				}
				M++;
	}
	j=j1>=0?j:j+1;
	if (j<0) j++;
	//    PRINTF(LEVEL,"N %d M %d j %d\n", N,M,j);
	return ((N+j)*10 + M - number/2);
}


//search functions
int browse_pokers(POKERS *h, COMBO_OF_POKERS * pCombos)
{
	int num=0;
	for (int i=h->begin; i<=min(P2,h->end); i++)
	{
		if ( h->hands[i] == 0)
			continue;
		else if ( h->hands[i] == 1)
			pCombos->type = SINGLE;
		else if ( h->hands[i] == 2)
			pCombos->type = PAIRS;
		else if ( h->hands[i] == 3)
			pCombos->type = THREE;
		else if ( h->hands[i] >= 4)
			pCombos->type = BOMB;

		pCombos->low = i;
		pCombos->len = 1;
		pCombos->three_desc[0] = 0;
		//        pCombos->number = h->hands[i];
		pCombos++;
		num++;
		if ( h->hands[i] > 4)
		{
			h->hands[i]-=4;
			i--;		
		}		
	}
	//if ( h->end >= LIT_JOKER)
	//{
	if ( h->hands[LIT_JOKER]==1 && h->hands[BIG_JOKER]==1 ) {
		pCombos->type = ROCKET;
		pCombos->low= LIT_JOKER;
		pCombos->len = 1;
		pCombos++;
		num++;
	}
	else if (h->hands[LIT_JOKER]==1) {
		pCombos->type = SINGLE;
		pCombos->low = LIT_JOKER;
		pCombos->len = 1;
		pCombos++;
		num++;
	} else if (h->hands[BIG_JOKER]==1) {
		pCombos->type = SINGLE;
		pCombos->len = 1;
		pCombos->low = BIG_JOKER;
		pCombos++;
		num++;
	}
	return num;
}

int search222inSingleSeries(POKERS* h, COMBO_OF_POKERS * p, COMBO_OF_POKERS * s)
{
	int len = p->len -5;
	int res = 0;
	if (len>=3 )
	{
		if ( h->hands[p->low+1]>=1 && h->hands[p->low+2]>=1 && h->hands[p->low]>=1)
		{
			s->type = PAIRS_SERIES;
			s->len =3;
			s->low = p->low;
			p->len -=3;
			h->hands[p->low]--;
			h->hands[p->low+1]--;
			h->hands[p->low+2]--;
			p->low+=3;

			//found a double series
			if (len >4 && h->hands[p->low]>=1 )
			{
				s->len++;
				p->len--;

				h->hands[p->low]--;
				p->low++;
				if (h->hands[p->low-1]>0) {
					h->hands[p->low-1]--;
					p->low--;
					p->len++;
					return 1;
				}
			}
			else 			if (h->hands[p->low-1]>0) {
				h->hands[p->low-1]--;
				p->low--;
				p->len++;
				return 1;
			}

			return 1;
		}
		else 	   if ( h->hands[p->low+p->len-1-1]>=1 && h->hands[p->low+p->len-1-2]>=1 && h->hands[p->low+p->len-1]>=1)
		{
			s->type = PAIRS_SERIES;
			s->len =3;
			s->low = p->low+p->len-3;

			h->hands[p->low+p->len-1]--;
			h->hands[p->low+p->len-1-1]--;
			h->hands[p->low+p->len-1-2]--;
			p->len -=3;
			//found a double series
			if (len >4 && h->hands[p->low+p->len-1]>=1 )
			{
				s->len++;
				s->low--;
				h->hands[p->low+p->len-1]--;
				p->len--;
				if (h->hands[p->low+p->len]>0) {
					h->hands[p->low+p->len]--;
					p->len++;
					return 1;
				}
			}
			else 	  		     if (h->hands[p->low+p->len]>0) {
				h->hands[p->low+p->len]--;
				p->len++;
				return 1;
			}

			return 1;
		}

	}
	return res;
}

int  search234inSingleSeries(POKERS* h, COMBO_OF_POKERS * p, COMBO_OF_POKERS * s) //tobe optimized
{
	//from head to tail.
	while ( p->len >5)
	{
		int end=p->len+p->low-1;
		if ( h->hands[end] > h->hands[p->low] )
		{   //remove tail
			h->hands[end]++;
			p->len--;
			//end--;
		}
		else if (h->hands[p->low] >0)
		{
			h->hands[p->low]++;
			p->low++;
			p->len--;
		}
		else
			break;
	}
	//check_five_series:
	//todo:
	return 0;
}


int  searchMultiSingleSeries(POKERS* h, COMBO_OF_POKERS * p) //tobe optimized
{
	//    POKERS t,*hbak;
	//    memmove(&t,h,sizefo(POKERS));
	//    h=&t;
	COMBO_OF_POKERS * first = p;
	int num=0;

	// got all 5-series
	for (int i = h->begin; i<= (CEIL_A(h->end) - 4); i++)
	{
		//        if (getBigSeries(h, p, i ,  CEIL_A(h->end), 1, 5)) //could be optimized
		if (getSingleSeries(h, p, i ,  CEIL_A(h->end), 1)) //could be optimized
		{
			remove_combo_poker(h, p, NULL);
			p++;
			num++;
		}
	}
	p=first;
	for (int i=0; i<num; i++) {
		{
			while ( p->len+p->low<=Pa && h->hands[p->len+p->low])
			{
				h->hands[p->len+p->low]--;
				p->len++;
			}
			p++;
		}
	}

	p=first;
	int num_bak =num;
	for (int i=0; i<num; i++) {

		if (  i<num-1 && p->low+p->len == (p+1)->low ) //connect
		{
			num--;
			p->len+= (p+1)->len;
			i++;
			if ( num > i )
				memmove(p+1 ,p+2,sizeof(COMBO_OF_POKERS)*(num-i));
		}
		else  if (  i<num-2 && p->low+p->len == (p+2)->low ) //connect
		{

			p->len+= (p+2)->len;
			if ( num ==4 )
				memmove(p+2 ,p+3,sizeof(COMBO_OF_POKERS)*(1));
			num--;
		}
		else if (  i<num-3 && p->low+p->len == (p+3)->low ) //connect
		{

			p->len+= (p+3)->len;
			//              if ( num ==3 )
			//                memmove(p+2 ,p+3,sizeof(COMBO_OF_POKERS)*(num-2));
			num--;
		}
		p++;
	}

	for(int i=num; i<num_bak; i++)
	{
		first[i].type = NOTHING;
	}
	return num;
}

int search_general_1(POKERS* h , COMBO_OF_POKERS * pCombos,
	bool skip_bomb,bool skip_double, bool skip_three, bool skip_series)
{
	int combo_nums=0;
	int num =0;
	if (!skip_series) {

		num= searchMultiSingleSeries(h,pCombos);
		combo_nums+=num;
		pCombos+=num;
		COMBO_OF_POKERS * tmp=pCombos-num;
		for (int k=0; k<num; k++)
		{
			int res =  search222inSingleSeries(h,tmp+k,pCombos);
			if (res==1) {
				pCombos++;
				combo_nums++;
			}
			else
				search234inSingleSeries(h,tmp+k,pCombos);
			//     remove_combo_poker(h,pCombos,NULL);
		}
	}
	if (!skip_bomb)
		while ( getBomb(h,pCombos))
		{
			remove_combo_poker(h,pCombos,NULL);
			combo_nums++;
			pCombos++;
		}

		if (!skip_three)
			while ( getThreeSeries(h,pCombos))
			{
				remove_combo_poker(h,pCombos,NULL);
				combo_nums++;
				pCombos++;
			}

			if (!skip_double)
				while ( getDoubleSeries(h,pCombos)) //todo: check three in doubles.
				{
					remove_combo_poker(h,pCombos,NULL);
					//          updateDoubleSeries(h,pCombos);
					combo_nums++;
					pCombos++;
				}

				/*   	while( getThree(h,pCombos))
				{
				remove_combo_poker(h,pCombos,NULL);
				combo_nums++;
				pCombos++;
				}
				*/

				//todo search for three's in series head and tail.
				num = browse_pokers(h,pCombos);
				pCombos+=num;
				combo_nums+=num;


				//  print_all_combos(pCombos-combo_nums,combo_nums);

				return combo_nums;
}

int search_general_2(POKERS* h , COMBO_OF_POKERS * pCombos,
	bool skip_bomb,bool skip_double, bool skip_three, bool skip_series)
{
	int combo_nums=0;
	int num ;
	if (!skip_bomb)
		while ( getBomb(h,pCombos))
		{
			remove_combo_poker(h,pCombos,NULL);
			combo_nums++;
			pCombos++;
		}

		if (!skip_three)
			while ( getThreeSeries(h,pCombos))
			{
				remove_combo_poker(h,pCombos,NULL);
				combo_nums++;
				pCombos++;
			}
			if (!skip_series)	{

				num = searchMultiSingleSeries(h,pCombos);
				combo_nums+=num;
				pCombos+=num;
				COMBO_OF_POKERS * tmp=pCombos-num;
				for (int k=0; k<num; k++)
				{
					int res =  search222inSingleSeries(h,tmp+k,pCombos);
					if (res==1) {
						pCombos++;
						combo_nums++;
					}
					else
						search234inSingleSeries(h,tmp+k,pCombos);
					//     remove_combo_poker(h,pCombos,NULL);
				}
			}

			if (!skip_double)
				//may be removed.
				while ( getDoubleSeries(h,pCombos)) //todo: check three in doubles.
				{
					remove_combo_poker(h,pCombos,NULL);
					//         updateDoubleSeries(h,pCombos);
					combo_nums++;
					pCombos++;
				}

				//todo search for three's in series head and tail.
				num = browse_pokers(h,pCombos);
				pCombos+=num;
				combo_nums+=num;


				//   print_all_combos(pCombos-combo_nums,combo_nums);

				return combo_nums;
}

int search_general_3(POKERS* h , COMBO_OF_POKERS * pCombos,
	bool skip_bomb,bool skip_double, bool skip_three, bool skip_series)
{
	int combo_nums=0;
	int num;
	if (!skip_bomb)
		while ( getBomb(h,pCombos))
		{
			remove_combo_poker(h,pCombos,NULL);
			combo_nums++;
			pCombos++;
		}

		if (!skip_three)
			while ( getThreeSeries(h,pCombos))
			{
				remove_combo_poker(h,pCombos,NULL);
				combo_nums++;
				pCombos++;
			}

			if (!skip_double)
				while ( getDoubleSeries(h,pCombos))
				{
					remove_combo_poker(h,pCombos,NULL);
					//      updateDoubleSeries(h,pCombos);
					combo_nums++;
					pCombos++;
				}

				if (!skip_series)
				{
					num = searchMultiSingleSeries(h,pCombos);
					combo_nums+=num;
					pCombos+=num;
					COMBO_OF_POKERS * tmp=pCombos-num;
					for (int k=0; k<num; k++)
					{
						int res =  search222inSingleSeries(h,tmp+k,pCombos);
						if (res==1) {
							pCombos++;
							combo_nums++;
						}
						else
							search234inSingleSeries(h,tmp+k,pCombos);
						//     remove_combo_poker(h,pCombos,NULL);
					}

				}
				//todo search for three's in series head and tail.
				num = browse_pokers(h,pCombos);
				pCombos+=num;
				combo_nums+=num;

				//    print_all_combos(pCombos-combo_nums,combo_nums);

				return combo_nums;
}


//1. bomb 2. three 3. straight 4. else
int search_general_4(POKERS* h , COMBO_OF_POKERS * pCombos,
	bool skip_bomb,bool skip_double, bool skip_three, bool skip_series)
{
	int combo_nums=0;
	int num =0;

	if (!skip_bomb)
		while ( getBomb(h,pCombos))
		{
			remove_combo_poker(h,pCombos,NULL);
			combo_nums++;
			pCombos++;
		}
		while ( getThree(h,pCombos))
		{
			remove_combo_poker(h,pCombos,NULL);
			combo_nums++;
			pCombos++;
		}

		if (!skip_series) {

			num= searchMultiSingleSeries(h,pCombos);
			combo_nums+=num;
			pCombos+=num;
			COMBO_OF_POKERS * tmp=pCombos-num;
			for (int k=0; k<num; k++)
			{
				int res =  search222inSingleSeries(h,tmp+k,pCombos);
				if (res==1) {
					pCombos++;
					combo_nums++;
				}
				else
					search234inSingleSeries(h,tmp+k,pCombos);
				//     remove_combo_poker(h,pCombos,NULL);
			}
		}
		//todo search for three's in series head and tail.
		num = browse_pokers(h,pCombos);
		pCombos+=num;
		combo_nums+=num;


		//  print_all_combos(pCombos-combo_nums,combo_nums);

		return combo_nums;
}

COMBO_OF_POKERS*  find_max_len_in_combos(COMBO_OF_POKERS* combos, int total)
{
	COMBO_OF_POKERS* cur=NULL;
	int	max= 0;
	for (int k=0; k<total; k++)
	{
		if (combos[k].type!=NOTHING)
		{
			int tmp;
			(tmp=get_combo_number(&combos[k]));
			if (tmp>max) {
				max=tmp;
				cur= combos+k;
			}
			else if (tmp == max && max>1 && combos[k].low < cur->low)
			{
				max=tmp;
				cur= combos+k;
			}
			else if (tmp == max && max==1 && combos[k].low > cur->low)
			{
				max=tmp;
				cur= combos+k;
			}
		}
	}
	return cur;
}

COMBO_OF_POKERS*  find_biggest_in_combos(COMBO_OF_POKERS* combos, int total)
{
	COMBO_OF_POKERS* cur=NULL;
	int	max= 0;
	for (int k=0; k<total; k++)
	{
		if (combos[k].type!=NOTHING)
		{
			if (combos[k].low>=max) {
				max=combos[k].low;
				cur= combos+k;
			}
		}
	}
	return cur;
}

/*
COMBO_OF_POKERS*  find_smallest_in_combos(COMBO_OF_POKERS* combos, int total,PLAYER* player)
{
COMBO_OF_POKERS* cur=NULL;
int	max= 0;
for (int k=0; k<total; k++)
{
if (combos[k].type!=NOTHING &&)
{int tmp;
if(combos[k].low<max){
max=tmp;
cur= combos+k;
}
else if(combos[k].low == max && get_combo_number(combos[k])< get_combo_number(cur)){
max=tmp;
cur= combos+k;
}
}
}
return cur;
}
*/

//todo: you could sort not_biggest first...
COMBO_OF_POKERS*  find_smallest_in_combos(COMBO_OF_POKERS* com, int total,PLAYER* player ,BOOL not_search_single)
{
	COMBO_OF_POKERS* cur=NULL;

	COMBOS_SUMMARY * s= player->summary;
	//    COMBO_OF_POKERS* combos=&s->not_biggest[0];

	int	max=0;
	if ( !not_search_single) {
		if ( s->not_biggest_num >0) {
			cur= s->not_biggest[0];
			max= s->not_biggest[0]->low;
		}
		for (int k=1; k<s->not_biggest_num; k++)
		{
			if (s->not_biggest[k]->type!=NOTHING )
			{
				if (s->not_biggest[k]->low>max) {}
				else if (s->not_biggest[k]->low<max) {
					max=s->not_biggest[k]->low;
					cur= s->not_biggest[k];
				}
				else if ( get_combo_number(s->not_biggest[k])> get_combo_number(cur)) {
					max=s->not_biggest[k]->low;
					cur=s->not_biggest[k];
				}
			}
		}
		if ( cur == NULL) // search biggest
		{   //max len..
			cur= s->biggest[0];
			max= s->biggest[0]->low;
			if (s->biggest[0]->type==BOMB)
				max=LIT_JOKER;
			for (int k=0; k<s->biggest_num; k++)
			{
				if (s->biggest[k]->type!=NOTHING )
				{
					if (s->biggest[k]->low>max) {
					}
					else if (s->biggest[k]->low<max) {
						max=s->biggest[k]->low;
						cur= s->biggest[k];
					}
					else if ( get_combo_number(s->biggest[k])> get_combo_number(cur)) {
						max=s->biggest[k]->low;
						cur=s->biggest[k];
					}
				}
			}
		}
	}
	else
	{
		int k;
		for (k=0; k<s->not_biggest_num; k++)
		{
			if (s->not_biggest[k]->type!=NOTHING &&  s->not_biggest[k]->type!=SINGLE)
			{
				max=s->not_biggest[k]->low;
				cur= s->not_biggest[k];
				k++;
				break;
			}
		}
		for (; k<s->not_biggest_num; k++)
		{
			if (s->not_biggest[k]->type!=NOTHING &&  s->not_biggest[k]->type!=SINGLE)
			{
				if (s->not_biggest[k]->low>max) {}
				else if (s->not_biggest[k]->low<max) {
					max=s->not_biggest[k]->low;
					cur= s->not_biggest[k];
				}
				else if ( get_combo_number(s->not_biggest[k])< get_combo_number(cur)) {
					max=s->not_biggest[k]->low;
					cur=s->not_biggest[k];
				}
			}
		}
#if 0
		if ( cur == NULL) // search biggest
		{   //max len..
			cur= s->biggest[0];
			max= s->biggest[0]->low;
			if (s->biggest[0]->type==BOMB)
				max=LIT_JOKER;
			for (int k=0; k<s->biggest_num; k++)
			{
				if (s->biggest[k]->type!=NOTHING )
				{
					if (s->biggest[k]->low>max) {
					}
					else if (s->biggest[k]->low<max) {
						max=s->biggest[k]->low;
						cur= s->biggest[k];
					}
					else if ( get_combo_number(s->biggest[k])> get_combo_number(cur)) {
						max=s->biggest[k]->low;
						cur=s->biggest[k];
					}
				}
			}
		}
#endif
	}
	return cur;
}


int  find_combo_with_3_controls_in_combos(COMBO_OF_POKERS* com, int total,PLAYER* player)
{
	COMBO_OF_POKERS* cur=NULL;

	COMBOS_SUMMARY * s= player->summary;
	//    COMBO_OF_POKERS* combos=&s->not_biggest[0];

	int	max=0;
	cur= s->not_biggest[0];
	max= s->not_biggest[0]->low;
	for (int k=0; k<s->not_biggest_num; k++)
	{
		if (s->not_biggest[k]->type!=NOTHING )
		{
			if (s->not_biggest[k]->low>max) {}
			else if (s->not_biggest[k]->low<max) {
				max=s->not_biggest[k]->low;
				cur= s->not_biggest[k];
			}
			else if ( get_combo_number(s->not_biggest[k])< get_combo_number(cur)) {
				max=s->not_biggest[k]->low;
				cur=s->not_biggest[k];
			}
		}
	}
	return 0;
}

int get_control_poker_num_in_combo(COMBO_OF_POKERS* c, int lower)
{
	switch ( c->type )
	{
	case ROCKET:
		//s->singles[s->series_num]=&c[k];
		//s->rocket_num++;
		//break;
		return 0;
	case BOMB:
		//if ( c->low >=lower)
		//	return 4;
		return 0;
	case SINGLE_SERIES:
		if ( c->low + c->len - 1  >=lower)
			return c->low+c->len-lower;
	case PAIRS_SERIES:
		if ( c->low + c->len - 1  >=lower)
			return 2*(c->low+c->len-lower);
	case THREE_SERIES:
		if ( c->low + c->len - 1  >=lower)
			return 3*(c->low+c->len-lower);
	case 31:
		return 3*( c->low >=lower) + c->three_desc[0]>=lower ;
	case 32:
		return ( c->low >=lower)*3 + 2*(c->three_desc[0]>=lower);
#if 0
	case THREE:
		if ( c->low >=lower)
			return 3;
	case PAIRS :

		if ( c->low >=lower)
			return 2;
	case SINGLE :
		if ( c->low >=lower)
			return 1;
#else
	case THREE:  //donot check three,pairs,single
	case PAIRS :
	case SINGLE :
		return 0;
#endif
	case 3311:
	case 333111:
	case 33331111:
	case 531: {
		int 	num =0;
		if ( c->low + c->len - 1  >=lower)  //todo fixme...
			num = 3*(c->low+c->len-lower);
		for (int i=0; i<c->len; i++)
		{
			num += c->three_desc[i]>=lower;
		}
		return num;
			  }
	case 3322:
	case 333222:
	case 33332222:
		{
			int 	num =0;
			if ( c->low + c->len - 1  >=lower)  //todo fixme...
				num = 3*(c->low+c->len-lower);
			for (int i=0; i<c->len; i++)
			{
				num += 2*(c->three_desc[i]>=lower);
			}
			return num;
		}
	case 411:
		{
			int 	num =0;
			if ( c->low >=lower)  //todo fixme...
				num = 4;
			for (int i=0; i<2; i++)
			{
				num += (c->three_desc[0]>=lower);
			}
			return num;
		}
	case 422:
		{
			int 	num =0;
			if ( c->low >=lower)  //todo fixme...
				num = 4;
			for (int i=0; i<2; i++)
			{
				num += 2*(c->three_desc[0]>=lower);
			}
			return num;
		}
	}

	return 0;
}


//update summary of combos
void update_summary(GAME* game, COMBOS_SUMMARY *s, POKERS*h, POKERS * opp, COMBO_OF_POKERS * c,int total, int opp1,int opp2,int lower,PLAYER * player)
{
	//s->ctrl.single = calc_controls(h,opp,CONTROL_POKER_NUM);
	s->real_total_num = 0;
	s->biggest_num = 0;
	s->extra_bomb_ctrl =0;
	s->not_biggest_num =0;
	s->combo_with_2_controls = 0;
	for (int k=0; k<total; k++)
	{
		if (c[k].type!=NOTHING)
		{
			s->real_total_num++;
			if ( is_combo_biggest(game,opp,c+k,opp1,opp2,lower) )
			{
				c[k].control = 1;
				s->biggest[s->biggest_num++] = &c[k];
				if (c[k].type==BOMB)
					s->extra_bomb_ctrl ++;
			}
			else
			{
				c[k].control = 0;
				s->not_biggest[s->not_biggest_num++] = &c[k];

			}
			if ( get_control_poker_num_in_combo	(c+k,lower)>=2  &&c[k].control ==1)//&& c[k].type!=PAIRS)
				s->combo_with_2_controls ++;
		}
	}


	// s->combo_total_num = s->real_total_num - s->biggest_num + s->combo_with_2_controls;
	//  s->combo_total_num = s->real_total_num - s->biggest_num;
	//   if( player->max_control.single >0 )
	//     s->combo_total_num = s->real_total_num - s->biggest_num + s->combo_with_2_controls;
	// else
	s->combo_total_num = s->real_total_num - s->biggest_num;

	int series_type=0;
	if (s->series_num>0)
	{
		for (int k=0; k<10; k++)
			series_type+= s->series_detail[k]!=0;
	}
	s->combo_typenum = 	(s->pairs_num!=0)  +  series_type
		+ (s->singles_num!=0) + (s->three_num!=0) + (s->three_two_num!=0)
		+ (s->three_one_num!=0) + (s->pairs_series_num!=0) + (s->threeseries_num!=0)
		+ (s->threeseries_two_num!=0)+ (s->threeseries_one_num!=0)+(s->four_one_num!=0)
		+(s->four_two_num!=0)
		;
	s->combo_smallest_single = s->singles_num>0?s->singles[0]->low:0;
	PRINTF(LEVEL+1,"\nCombos summary: total %d type %d real_total %d biggest %d\n",s->combo_total_num,s->combo_typenum,s->real_total_num,s->biggest_num)	;
}

void update_summary_for_big_sure(GAME* game, COMBOS_SUMMARY *s,
	COMBO_OF_POKERS * c,int total,PLAYER * player)
{
	//s->ctrl.single = calc_controls(h,opp,CONTROL_POKER_NUM);
	s->real_total_num = 0;
	s->biggest_num = 0;
	s->extra_bomb_ctrl =0;
	s->not_biggest_num =0;
	s->combo_with_2_controls = 0;
	int lower = player->lower_control_poker;
	for (int k=0; k<total; k++)
	{
		if (c[k].type!=NOTHING)
		{
			s->real_total_num++;
			if ( is_combo_biggest_sure_without_bomb(game,c+k) )
			{
				c[k].control = 1;
				s->biggest[s->biggest_num++] = &c[k];
				if (c[k].type==BOMB)
					s->extra_bomb_ctrl ++;
			}
			else
			{
				c[k].control = 0;
				s->not_biggest[s->not_biggest_num++] = &c[k];

			}
			if ( get_control_poker_num_in_combo	(c+k,lower)>=2 &&c[k].control ==1)//&& c[k].type!=PAIRS)
				s->combo_with_2_controls ++;
		}
	}


	// s->combo_total_num = s->real_total_num - s->biggest_num + s->combo_with_2_controls;
	//  s->combo_total_num = s->real_total_num - s->biggest_num;
	//   if( player->max_control.single >0 )
	//     s->combo_total_num = s->real_total_num - s->biggest_num + s->combo_with_2_controls;
	// else
	s->combo_total_num = s->real_total_num - s->biggest_num;

	int series_type=0;
	if (s->series_num>0)
	{
		for (int k=0; k<10; k++)
			series_type+= s->series_detail[k]!=0;
	}
	s->combo_typenum = 	(s->pairs_num!=0)  +  series_type
		+ (s->singles_num!=0) + (s->three_num!=0) + (s->three_two_num!=0)
		+ (s->three_one_num!=0) + (s->pairs_series_num!=0) + (s->threeseries_num!=0)
		+ (s->threeseries_two_num!=0)+ (s->threeseries_one_num!=0)+(s->four_one_num!=0)
		+(s->four_two_num!=0)
		;
	s->combo_smallest_single = s->singles_num>0?s->singles[0]->low:0;
	PRINTF(LEVEL+1,"\nCombos summary: total %d type %d real_total %d biggest %d\n",s->combo_total_num,s->combo_typenum,s->real_total_num,s->biggest_num)	;

}


//to be optimized
//current for
void sort_all_combos(GAME* game, COMBO_OF_POKERS * c , int total, COMBOS_SUMMARY * s,
	POKERS *opp, int opp1, int opp2, /* for check biggest*/
	int lower,PLAYER * player
	)
{
	//   s->real_total_num = total;
	//    int total = s->combo_total_num;
	memset(s,0,sizeof(COMBOS_SUMMARY));//todo: optimized
	for (int k=0; k<total; k++)
	{
		switch (c[k].type)
		{
		case ROCKET:
			//     s->bomb[s->bomb_num]=&c[k];
			//            s->bomb_num++;
			//     break;
		case BOMB:
			//	{//check if it is a laizi bomb or pure bomb
			c[k].three_desc[0]=0;
			if(player->h->hands[c[k].low]==4)
			{
				c[k].three_desc[0]= c[k].low==game->hun?2:1;

			}
			if(game->hun==-1)
				c[k].three_desc[0]=0;

			{
				int pos=0;	
				for(int i=s->bomb_num-1; i>=0;i--)
				{
					if( check_combo_a_Big_than_b(&c[k],s->bomb[i] ))
					{  //insert bomb here.
						pos=i+1;
						break;
					}
				}
				for(int j=s->bomb_num-1; j>=pos;j--)
				{
					s->bomb[j+1]=s->bomb[j];
				}
				s->bomb[pos]= &c[k];  						  
				s->bomb_num++;					 
			}
			break;
		case SINGLE_SERIES:
			s->series[s->series_num]=&c[k];
			s->series_num++;
			s->series_detail[c[k].len -5]++;
			break;
		case PAIRS_SERIES:
			s->pairs_series[s->pairs_series_num]=&c[k];
			s->pairs_series_num++;
			break;
		case THREE_SERIES:
			s->threeseries[s->threeseries_num]=&c[k];
			s->threeseries_num++;
			break;
		case THREE:
			s->three[s->three_num]=&c[k];
			s->three_num++;
			break;
		case THREE_ONE:
			s->three_one[s->three_one_num]=&c[k];
			s->three_one_num++;
			break;
		case THREE_TWO:
			s->three_two[s->three_two_num]=&c[k];
			s->three_two_num++;
			break;
		case PAIRS :
			s->pairs[s->pairs_num]=&c[k];
			s->pairs_num++;
			break;
		case SINGLE :
			s->singles[s->singles_num]=&c[k];
			s->singles_num++;
			break;
		case 3311:
		case 333111:
		case 33331111:
		case 531:
			s->threeseries_one[s->threeseries_one_num++]=&c[k];
			break;
		case 3322:
		case 333222:
		case 33332222:
			s->threeseries_two[s->threeseries_two_num++]=&c[k];
			break;
		case 411:
			s->four_one[s->four_one_num++]=&c[k];
			break;
		case 422:
			s->four_two[s->four_two_num++]=&c[k];
			break;
		default:
			//nothing++;
			PRINTF_ALWAYS("!!ERR, line %d this type of combo not supported : %d\n",c[k].type,__LINE__);
			break;
		case NOTHING:
			break;
		}
	}

	// refine for three;
	int j,num=s->threeseries_num;
	//    int not_pad[j]
	for (j=0; j<num; j++) //todo: spilt a pair..
	{
		unsigned int type1[6]= {0,31,3311,333111,33331111,531};
		unsigned int type2[5]= {0,32,3322,333222,33332222};
		if(  s->threeseries[j]->len <= s->singles_num && s->threeseries[j]->len <= s->pairs_num)
		{
			//todo:refine
			if(  s->singles[s->threeseries[j]->len-1]->low > s->pairs[s->threeseries[j]->len-1]->low+1
				||(s->singles[s->threeseries[j]->len-1]->low > s->pairs[s->threeseries[j]->len-1]->low
				&& s->singles[s->threeseries[j]->len-1]->low >=player->lower_control_poker) )
			{
				goto _use_pairs;
			}
		}
		// for
		if ( s->threeseries[j]->len <= s->singles_num)
		{
			for (int k=0; k< s->threeseries[j]->len ; k++ )
			{
				s->threeseries[j]->three_desc[k] = s->singles[k]->low;

				s->singles[k]->type =NOTHING;
			}
			s->singles_num-=s->threeseries[j]->len ;
			memmove(s->singles,s->singles+s->threeseries[j]->len ,(s->singles_num)*sizeof(void*));
			s->threeseries_one[ s->threeseries_one_num ]= s->threeseries[j];
			s->threeseries[j]->type = type1[s->threeseries[j]->len];
			s->threeseries_one_num ++;
			s->threeseries_num --;
			s->threeseries[j] = NULL;
		}
		else if (s->threeseries[j]->len <= s->pairs_num)
		{
_use_pairs:
			for (int k=0; k< s->threeseries[j]->len ; k++ )
			{
				s->threeseries[j]->three_desc[k] = s->pairs[k]->low;
				s->pairs[k]->type =NOTHING;
			}
			s->pairs_num-=s->threeseries[j]->len ;
			memmove(s->pairs,s->pairs+s->threeseries[j]->len ,(s->pairs_num)*sizeof(void*));
			s->threeseries_two[s->threeseries_two_num ]= s->threeseries[j];
			s->threeseries[j]->type = type2[s->threeseries[j]->len];
			s->threeseries_two_num ++;
			//->biggest[s->biggest_num++]=  s->threeseries_two[j];
			s->threeseries_num --;
			s->threeseries[j] = NULL;
		}
		else if(s->threeseries[j]->len <= s->pairs_num*2 + s->singles_num )
		{
			/*
			if( s->singles_num == 1 && s->threeseries[j]->len ==3 ){
			s->threeseries[j]->three_desc[0] = s->singles[0]->low;
			s->singles[0]->type =NOTHING;
			s->singles_num = 0;
			s->threeseries[j]->three_desc[1] = s->pairs[0]->low;
			s->threeseries[j]->three_desc[2] = s->pairs[0]->low;
			s->pairs[0]->type =NOTHING;
			s->pairs_num --;
			memmove(s->pairs,s->pairs+1 ,(s->pairs_num)*sizeof(void*));
			}
			*/
			//int single=0,pair=0;
			//do not support len ==5;
			for (int k=0; k< s->threeseries[j]->len ;  )
			{
				if(s->singles_num>0 && s->pairs_num>0 )
				{
					if ( k== (s->threeseries[j]->len -1) ||s->singles[0]->low < s->pairs[0]->low )
					{
__pad_single:
						s->threeseries[j]->three_desc[k++] = s->singles[0]->low;
						s->singles[0]->type =NOTHING;
						s->singles_num-- ;
						memmove(s->singles,s->singles+1 ,(s->singles_num)*sizeof(void*));
					}
					else
					{
__pad_single_in_pair:
						s->threeseries[j]->three_desc[k++] = s->pairs[0]->low;
						if(k==s->threeseries[j]->len)
						{
							s->pairs[0]->type= SINGLE;
							s->pairs_num --;
							s->singles_num ++;
							for(int single=s->singles_num ; single>0; single--)
								s->singles[single]=s->singles[single-1];
							s->singles[0]=s->pairs[0];
							memmove(s->pairs,s->pairs+1 ,(s->pairs_num)*sizeof(void*));
							break;
						}
						s->threeseries[j]->three_desc[k++] = s->pairs[0]->low;
						s->pairs[0]->type =NOTHING;
						s->pairs_num-- ;
						memmove(s->pairs,s->pairs+1 ,(s->pairs_num)*sizeof(void*));
					}
				}
				else if (s->singles_num>0)
				{
					goto __pad_single;
				}
				else if(s->pairs_num>0)
				{
					goto __pad_single_in_pair;
				}

			}
			s->threeseries_one[s->threeseries_one_num]= s->threeseries[j];
			s->threeseries[j]->type = type1[s->threeseries[j]->len];
			//s->biggest[s->biggest_num++]=  s->threeseries_two[j];
			s->threeseries_one_num ++;
			s->threeseries_num --;
			s->threeseries[j] = NULL;
		}
		//todo refine
		//    break;
	}


	if (s->threeseries_num >0 ) //strange
	{
		int k=0;
		for(int j=0; j<3; j++)
		{
			if(s->threeseries[j]!=NULL)
			{
				s->threeseries[k++]=s->threeseries[j];
			}
		}
		if(k!=s->threeseries_num)
			PRINTF_ALWAYS("ERR: 3311 or 3322\n");
	}



	if(s->singles_num>=s->three_num && s->pairs_num>= s->three_num && s->three_num>0)
	{
		int use_pairs_for_three =0;
		if(  s->singles[s->three_num-1]->low > s->pairs[s->three_num-1]->low+1
			||(s->singles[s->three_num-1]->low > s->pairs[s->three_num-1]->low
			&& s->singles[s->three_num-1]->low >=player->lower_control_poker) )
		{
			use_pairs_for_three=1;
		}
		if( (game->player_type==UPFARMER && game->players[CUR_PLAYER]->oppDown_num ==1)
			||   	(game->player_type==DOWNFARMER && game->players[CUR_PLAYER]->oppUp_num ==1)
			|| (game->player_type==LORD &&
			( game->players[CUR_PLAYER]->oppDown_num ==1
			|| game->players[CUR_PLAYER]->oppUp_num==1 ) )
			)
		{
			use_pairs_for_three =0;
		}

		for (j=0; j<s->three_num; j++) //todo: spilt a pair..
		{
			// for
			if ( !use_pairs_for_three)
			{
				s->three_one[j]= s->three[j];
				s->three[j]->type = 31;
				s->three[j]->three_desc[0] = s->singles[j]->low;
				s->three_one_num ++;
				s->singles[j]->type =NOTHING;
			}
			else
			{
				//search single first
				s->three_two[j]= s->three[j];
				s->three[j]->type = 32;
				s->three[j]->three_desc[0] = s->pairs[j]->low;
				s->three_two_num = j-s->singles_num;
				s->pairs[j]->type =NOTHING;
			}
		}

		if ( !use_pairs_for_three )
		{
			memmove(s->singles,s->singles+s->three_num,(s->singles_num-s->three_num)*sizeof(void*));
			s->singles_num =(s->singles_num-s->three_num);
		}
		else
		{
			memmove(s->pairs,s->pairs+s->three_num,(s->pairs_num-s->three_num)*sizeof(void*));
			s->pairs_num -= s->three_num;
		}
		s->three_num = 0;
	}
	else
	{
		for (j=0; j<s->three_num; j++) //todo: spilt a pair..
		{
			// for
			if ( j<s->singles_num)
			{
				s->three_one[j]= s->three[j];
				s->three[j]->type = 31;
				s->three[j]->three_desc[0] = s->singles[j]->low;
				s->three_one_num ++;
				s->singles[j]->type =NOTHING;
			}
			else if (j-s->singles_num < s->pairs_num)
			{
				//search single first
				s->three_two[j-s->singles_num]= s->three[j];
				s->three[j]->type = 32;
				s->three[j]->three_desc[0] = s->pairs[j-s->singles_num]->low;
				s->three_two_num = j-s->singles_num+1;
				s->pairs[j-s->singles_num]->type =NOTHING;
			}
			else //todo refine
				break;
		}



		if (j<s->three_num)
		{
			memmove(s->three,s->three+j,(s->three_num-j)*sizeof(void*));
			s->three_num =(s->three_num-j);
			s->singles_num = 0;
			s->pairs_num = 0;
		}
		else if (s->three_num > 0 )
		{
			if ( s->three_num <= s->singles_num )
			{
				memmove(s->singles,s->singles+s->three_num,(s->singles_num-s->three_num)*sizeof(void*));
				s->singles_num =(s->singles_num-s->three_num);
			}
			else
			{

				int number = s->three_num - s->singles_num ;
				s->singles_num = 0;
				memmove(s->pairs,s->pairs+number,(s->pairs_num-number)*sizeof(void*));
				s->pairs_num -= number;
			}
			s->three_num = 0;
		}

	}

	update_summary(game,s,player->h,opp,c,total,opp1,opp2,lower,player);
}


int get_2nd_min_singles(COMBOS_SUMMARY *s )
{
	if (s->singles_num <=1 )
		return BIG_JOKER;
	else {
		return s->singles[1]->low;
	}
}

int cmp_summary_for_min_single_farmer( COMBOS_SUMMARY *cur,COMBOS_SUMMARY *result) //return 1 if a is better than b
{
	return 
		( get_2nd_min_singles(cur) > get_2nd_min_singles(result)  
		|| (cur->singles_num < result->singles_num
		&&  get_2nd_min_singles(cur) == get_2nd_min_singles(result) )
		);
}

/*
i.	控制 - 套的数目 最少。
ii.	套的种类最少
iii.	单牌数量最少
iv.	最小的单牌最大
*/
int cmp_summary( COMBOS_SUMMARY *a,COMBOS_SUMMARY *b) //return a>b
{
	if (	(a->combo_total_num-a->extra_bomb_ctrl< b->combo_total_num-b->extra_bomb_ctrl )
		)
		return true;
	else	if (	a->combo_total_num-a->extra_bomb_ctrl > b->combo_total_num-b->extra_bomb_ctrl
		)
		return false;
	else  if (a->combo_total_num-a->extra_bomb_ctrl == b->combo_total_num-b->extra_bomb_ctrl
		)
	{
		if(a->extra_bomb_ctrl>b->extra_bomb_ctrl)
			return true;
		if(a->extra_bomb_ctrl<b->extra_bomb_ctrl)
			return false;		
		if (a->combo_typenum < b->combo_typenum)
			return true;
		else if (a->combo_typenum > b->combo_typenum)
			return false;
		else {
			if (a->singles_num > b->singles_num )
				return false;
			else if (a->singles_num < b->singles_num )
				return true;
			else
				return a->combo_smallest_single > b->combo_smallest_single;
		}
	}
	return 0;
}

int search_combos_in_hands(GAME* game,POKERS* h , COMBO_OF_POKERS * pCombos, COMBOS_SUMMARY *pSummary,PLAYER * player)
{
	int combo_nums=0;
	/*
	//serach four
	if ( (h->end-h->begin < 2 || h->end-h->begin < 4 )&& h->total <6  )
	{
	combo_nums =  browse_pokers(h,pCombos);;
	return combo_nums;
	}
	else if ( h->end-h->begin < 4 )
	{
	POKERS t;
	memmove(&t,h,sizeof(POKERS));
	h= &t;
	while ( getBomb(h,pCombos))
	{
	remove_combo_poker(h,pCombos,NULL);
	combo_nums++;
	pCombos++;
	}

	if ( getThreeSeries(h,pCombos))
	{
	remove_combo_poker(h,pCombos,NULL);
	combo_nums++;
	pCombos++;
	}
	if ( getDoubleSeries(h,pCombos)) //todo: check three in doubles.
	{
	remove_combo_poker(h,pCombos,NULL);
	updateDoubleSeries(h,pCombos);
	combo_nums++;
	pCombos++;
	}

	int num=browse_pokers(h,pCombos);
	combo_nums+=num;
	pCombos+=num;

	sort_all_combos(game,pCombos-combo_nums,combo_nums,pSummary,
	player->opps,player->oppDown_num,player->oppUp_num, player->lower_control_poker,player);
	return combo_nums;
	}
	else
	*/
	{
		COMBO_OF_POKERS *presult,*pNow,*pTmp; //todo: use alloce
		COMBOS_SUMMARY *cur,*result,*tmp;
		presult= game->tmp_combos[0];//(COMBO_OF_POKERS*) malloc(sizeof(COMBO_OF_POKERS)*20); //comtmp[0][0];
		pNow=game->tmp_combos[1];//(COMBO_OF_POKERS*) malloc(sizeof(COMBO_OF_POKERS)*20);

		memset(presult,0, sizeof(COMBO_OF_POKERS)*20); //comtmp[0][0];
		memset(pNow,0 ,sizeof(COMBO_OF_POKERS)*20);


		result= &game->tmp_summary[0];//(COMBOS_SUMMARY *)malloc(sizeof(COMBOS_SUMMARY));
		cur=&game->tmp_summary[1];//(COMBOS_SUMMARY *)malloc(sizeof(COMBOS_SUMMARY));
		memset(cur,0,sizeof(COMBOS_SUMMARY));
		memset(result,0,sizeof(COMBOS_SUMMARY));

		int num=0,numRes=0;
		POKERS t;

		memmove(&t,h,sizeof(POKERS));
		numRes= result->combo_total_num  = search_general_1( &t,presult,0,0,0,0);
		sort_all_combos(game,presult,numRes, result,player->opps,player->oppDown_num,player->oppUp_num, player->lower_control_poker,player);
		//DBG(print_all_combos(presult,numRes));

		memmove(&t,h,sizeof(POKERS));
		num= cur->combo_total_num = search_general_2( &t,pNow,0,0,0,0);
		sort_all_combos(game,pNow,num, cur,player->opps,player->oppDown_num,player->oppUp_num, player->lower_control_poker,player);
		//DBG(print_all_combos(pNow,num));

		if ( cmp_summary(cur,result))
		{
			numRes = num;
			pTmp = presult;
			presult=pNow;
			pNow=pTmp;
			tmp=result;
			result= cur;
			cur=tmp;
		}

		memmove(&t,h,sizeof(POKERS));
		//        memset(cur,0,sizeof(COMBOS_SUMMARY));
		num = cur->combo_total_num = search_general_3( &t,pNow,0,0,0,0);
		sort_all_combos(game,pNow,num,cur,player->opps,player->oppDown_num,player->oppUp_num, player->lower_control_poker,player);
		// DBG(print_all_combos(pNow,num));


		if ( cmp_summary(cur,result))
		{
			numRes = num;
			pTmp = presult;
			presult=pNow;
			pNow=pTmp;

			tmp=result;
			result= cur;
			cur=tmp;
		}

		// for (int k=0; k<numRes; k++)
		memmove(pCombos,presult,numRes*sizeof(COMBO_OF_POKERS));
		sort_all_combos(game,pCombos,numRes,pSummary,player->opps,player->oppDown_num
			,player->oppUp_num,player->lower_control_poker,player);		
		//  print_all_combos(pCombos, numRes);
		return numRes;
	}
}

//地主报单农民组牌方式
int search_combos_in_suits_for_min_single_farmer_internal
	(GAME* game,POKERS* h , COMBO_OF_POKERS * pCombos, COMBOS_SUMMARY *pSummary,PLAYER * player)
{
	memset(pCombos,0, sizeof(COMBO_OF_POKERS)*20);

	COMBO_OF_POKERS *pNow;
	COMBOS_SUMMARY *cur,*result;
	pNow=game->tmp_combos[0];// (COMBO_OF_POKERS*) malloc(sizeof(COMBO_OF_POKERS)*20);
	memset(pNow,0, sizeof(COMBO_OF_POKERS)*20);

	result=player->summary;
	cur=&game->tmp_summary[0];//(COMBOS_SUMMARY *)malloc(sizeof(COMBOS_SUMMARY));
	memset(cur,0,sizeof(COMBOS_SUMMARY));
	//        memset(result,0,sizeof(COMBOS_SUMMARY));

	POKERS t;
	memmove(&t,h,sizeof(POKERS));
	int num  = search_general_1( &t,pCombos,0,1,0,0);
	sort_all_combos(game,pCombos,num, result,player->opps,player->oppDown_num,player->oppUp_num, player->lower_control_poker,player);

	//todo: add four_two
	memmove(&t,h,sizeof(POKERS));
	int num1 = search_general_4( &t,pNow,0,1,0,0);
	sort_all_combos(game,pNow,num1, cur,player->opps,player->oppDown_num,player->oppUp_num, player->lower_control_poker,player);

	if ( cur->singles_num < result->singles_num
		|| (cur->singles_num == result->singles_num
		&&  get_2nd_min_singles(cur) > get_2nd_min_singles(result) )
		)
	{
		memmove(pCombos,pNow,max(num1,num)*sizeof(COMBO_OF_POKERS));
		num =num1;
		sort_all_combos(game,pCombos,num, result,player->opps,player->oppDown_num,player->oppUp_num, player->lower_control_poker,player);
	}
	*pSummary = *player->summary;

finish_search:
	DBG(print_all_combos(pCombos, 20));
	return num;
}



#define CMP_AND_SWAP_SMY_SINGLE()\
	/*print_summary(result);*/\
	/*print_summary(cur);*/\
	if ( cmp_summary_for_min_single_farmer(cur,result))\
{\
	numRes = number;\
	pTmp = presult;\
	presult=pNow;\
	pNow=pTmp;\
	tmp=result;\
	result= cur;\
	cur=tmp;\
	/*print_all_combos(presult, number);*/\
}

/*
Get the
*/
int search_combos_in_suits_for_min_single_farmer
	(GAME* game, POKERS *h , COMBO_OF_POKERS *combos_tmp, PLAYER* player )
{
	POKERS *opp =player->opps;
	//player->lower_control_poker =
	//    player->max_control.single = calc_controls( h,opp, CONTROL_POKER_NUM );

	COMBO_OF_POKERS * combos=player->combos;
	COMBOS_SUMMARY *pcombo_summary, combo_summary;
	pcombo_summary = & combo_summary;
	for (int k=0; k<20; k++)
	{
		combos[k].type=NOTHING;;
		//PRINTF(LEVEL,"\n");
	}

	h->hun = game->hun;

	if(game->hun!=-1 && HUN_NUMBER(h)>0)
	{
		COMBO_OF_POKERS *presult,*pNow,*pTmp; 
		COMBOS_SUMMARY *cur,*result,*tmp;
		presult= game->tmp_combos[2];
		pNow=game->tmp_combos[3];

		memset(presult,0, sizeof(COMBO_OF_POKERS)*25);
		memset(pNow,0 ,sizeof(COMBO_OF_POKERS)*25);

		result= &game->tmp_summary[2];
		cur=&game->tmp_summary[3];
		memset(cur,0,sizeof(COMBOS_SUMMARY));
		memset(result,0,sizeof(COMBOS_SUMMARY));

		int numRes = search_combos_in_suits_for_min_single_farmer_internal
			(game, h , presult , result , player);
		int number;	

		int ben_hun = 0;
		//set first hun values  
		for( int i1=P3; i1<=P2; i1++ )
		{	    
			SET_HUN_TO_i(h, i1);
			//    PRINTF(VERBOSE, "==set first hun to %c\n", poker_to_char(i1));
			if(IS_HUN(h, i1)) ben_hun++;	
			if(HUN_NUMBER(h)>ben_hun)
			{
				//set second hun values  
				for( int i2=P3; i2<=P2; i2++ )
				{
					SET_HUN_TO_i(h, i2);
					//	          PRINTF(VERBOSE, "\t==set second hun to %c\n", poker_to_char(i2));					
					if(IS_HUN(h, i2)) ben_hun++;				  
					if(HUN_NUMBER(h)>ben_hun)
					{
						//set second hun values  
						for( int i3=P3; i3<=P2; i3++ )
						{
							SET_HUN_TO_i(h, i3);				               		       
							//                        PRINTF(VERBOSE, "\t\t==set third hun to %c\n", poker_to_char(i3));					 							
							number = search_combos_in_suits_for_min_single_farmer_internal
								(game, h , pNow,cur,player);	
							CMP_AND_SWAP_SMY_SINGLE()	
								SET_i_to_HUN(h, i3);
						}             
					}
					else
					{
						number = search_combos_in_suits_for_min_single_farmer_internal
							(game, h , pNow,cur,player);	
						CMP_AND_SWAP_SMY_SINGLE()			         
					}
					if(IS_HUN(h, i2)) ben_hun--;		
					SET_i_to_HUN(h, i2);
				}
			}
			else
			{
				number = search_combos_in_suits_for_min_single_farmer_internal
					(game, h , pNow,cur,player);	
				CMP_AND_SWAP_SMY_SINGLE()	       
			}
			if(IS_HUN(h, i1)) ben_hun--;		
			SET_i_to_HUN(h, i1);	
		}

		memmove(combos,presult,numRes*sizeof(COMBO_OF_POKERS)); 			
		sort_all_combos(game,player->combos,number,player->summary,player->opps,player->oppDown_num
			,player->oppUp_num,player->lower_control_poker,player);

	}
	else 
	{
		int number = search_combos_in_suits_for_min_single_farmer_internal
			(game, h , combos , pcombo_summary,player);
		sort_all_combos(game,player->combos,number,player->summary,player->opps,player->oppDown_num
			,player->oppUp_num,player->lower_control_poker,player);	   
	}

	if(level>=VERBOSE)
		print_suit(player);

	return true;
}


/*
Get the
*/
int search_combos_in_suit(GAME* game, POKERS *h , POKERS *opp, PLAYER* player )
{
	int ctrl_num = 	CONTROL_POKER_NUM;
	POKERS all;
	add_poker(h,opp,&all);
	if(all.total <=20 )
		ctrl_num = 4;
	else if(all.total <=10 )
		ctrl_num = 2;
	player->lower_control_poker = get_lowest_controls(&all,ctrl_num);
	int single = calc_controls( h,opp, ctrl_num );

	COMBO_OF_POKERS * combos=player->combos;
	COMBOS_SUMMARY *pcombo_summary, combo_summary;
	pcombo_summary = & combo_summary;
	for (int k=0; k<20; k++)
	{
		combos[k].type=NOTHING;
		combos[k].control=0;		
		combos[k].len=0;	
		//PRINTF(LEVEL,"\n");
	}

	h->hun = game->hun;

	if(game->hun!=-1 && HUN_NUMBER(h)>0)
	{
		COMBO_OF_POKERS *presult,*pNow,*pTmp; 
		COMBOS_SUMMARY *cur,*result,*tmp;
		presult= game->tmp_combos[2];
		pNow=game->tmp_combos[3];

		memset(presult,0, sizeof(COMBO_OF_POKERS)*25);
		memset(pNow,0 ,sizeof(COMBO_OF_POKERS)*25);

		result= &game->tmp_summary[2];
		cur=&game->tmp_summary[3];
		memset(cur,0,sizeof(COMBOS_SUMMARY));
		memset(result,0,sizeof(COMBOS_SUMMARY));

		int numRes = search_combos_in_hands(game, h , presult , result , player);
		int number;	

#define CMP_AND_SWAP_SMY()\
	/*print_summary(result);*/\
	/*print_summary(cur);*/\
	if ( cmp_summary(cur,result))\
		{\
		numRes = number;\
		pTmp = presult;\
		presult=pNow;\
		pNow=pTmp;\
		tmp=result;\
		result= cur;\
		cur=tmp;\
		/*print_all_combos(presult, number);*/\
		}

		int ben_hun = 0;
		//set first hun values  
		for( int i1=P3; i1<=P2; i1++ )
		{	    
			SET_HUN_TO_i(h, i1);
			//	    PRINTF(VERBOSE, "==set first hun to %c\n", poker_to_char(i1));
			if(IS_HUN(h, i1)) ben_hun++;	
			if(HUN_NUMBER(h)>ben_hun)
			{
				//set second hun values  
				for( int i2=P3; i2<=P2; i2++ )
				{
					SET_HUN_TO_i(h, i2);
					//  	          PRINTF(VERBOSE, "\t==set second hun to %c\n", poker_to_char(i2));					
					if(IS_HUN(h, i2)) ben_hun++;				  
					if(HUN_NUMBER(h)>ben_hun)
					{
						//set second hun values  
						for( int i3=P3; i3<=P2; i3++ )
						{
							SET_HUN_TO_i(h, i3);				               		       
							//                          PRINTF(VERBOSE, "\t\t==set third hun to %c\n", poker_to_char(i3));					 							
							number = search_combos_in_hands(game, h , pNow,cur,player);	
							CMP_AND_SWAP_SMY()	
								SET_i_to_HUN(h, i3);
						}             
					}
					else
					{
						number = search_combos_in_hands(game, h , pNow,cur,player);	
						CMP_AND_SWAP_SMY()			         
					}
					if(IS_HUN(h, i2)) ben_hun--;		
					SET_i_to_HUN(h, i2);
				}
			}
			else
			{
				number = search_combos_in_hands(game, h , pNow,cur,player);	
				CMP_AND_SWAP_SMY()	       
			}
			if(IS_HUN(h, i1)) ben_hun--;		
			SET_i_to_HUN(h, i1);	
		}

		memmove(combos,presult,numRes*sizeof(COMBO_OF_POKERS)); 			
		sort_all_combos(game,player->combos,number,player->summary,player->opps,player->oppDown_num
			,player->oppUp_num,player->lower_control_poker,player);

	}
	else 
	{
		int number = search_combos_in_hands(game, h , combos , pcombo_summary,player);
		sort_all_combos(game,player->combos,number,player->summary,player->opps,player->oppDown_num
			,player->oppUp_num,player->lower_control_poker,player);	   
	}
	player->summary->ctrl.single = single;  

	if(level>=VERBOSE)
		print_suit(player);
	//get_pairs(hands_combo);

	return true;
}


//get a single or pair for  three
//and update the summary
int search_1_2_for_a_three(POKERS*h, PLAYER* player,COMBO_OF_POKERS * c)
{
	COMBO_OF_POKERS * p  = c;
	player->need_research = 0;
	COMBOS_SUMMARY *s =player->summary;
	if (p->type == THREE_ONE )
	{
		if (player->summary->singles_num>0) {
			c->type=THREE_ONE;
			c->three_desc[0] = player->summary->singles[0]->low;
			remove_combo_poker(h,player->summary->singles[0],NULL);
			s->singles_num -- ;
			if (s->singles[0]->control!=1)
				s->combo_total_num --;
			else
				s->biggest_num --;
			s->real_total_num --;
			s->singles[0]->type=NOTHING;
			memmove(s->singles,s->singles+1,(s->singles_num)*sizeof(void*));
		}
		else if (player->summary->series_num>0 && s->series[0]->len>5)
		{
			c->type=THREE_ONE;
			c->three_desc[0]= s->series[0]->low;
			s->series[0]->low++;
			s->series[0]->len--;
		}
		else if (player->summary->pairs_num>0)
		{
			c->type=THREE_ONE;
			c->three_desc[0]= s->pairs[0]->low;
			s->pairs[0]->type=SINGLE;
			h->hands[ s->pairs[0]->low]-=1;
			player->need_research = 1;
		}
		else {
			for ( int i=h->begin; i<=h->end; i++)
				if ( h->hands[i]>0&& i!=c->low)
				{
					c->type=THREE_ONE;
					c->three_desc[0]= i;
					h->hands[i]--;
					player->need_research = 1;
					//search_combos_in_suit(game,h,player->opps,player);
					return 1;
				}
				// return 0;
		}
	}

	else if (p->type == THREE_TWO )
	{
		if (player->summary->pairs_num>0)
		{
			c->type=THREE_TWO;
			c->three_desc[0]= s->pairs[0]->low;
			s->pairs[0]->type=NOTHING;
			memmove(s->pairs,s->pairs+1,(s->pairs_num-1)*sizeof(void*));
		}
		else {
			for ( int i=h->begin; i<=max(P2,h->end); i++)
				if ( h->hands[i]>=2 && i!=c->low)
				{
					c->type=THREE_TWO;
					c->three_desc[0]= i;
					h->hands[i]-=2;
					player->need_research = 1;
					//search_combos_in_suit(game,h,player->opps,player);
					return 1;
				}
				return 0;
		}
	}

	else  if (p->type == 3322 )
	{
		if (player->summary->pairs_num>1)
		{
			c->type=3322;
			c->three_desc[0]= s->pairs[0]->low;
			c->three_desc[1]= s->pairs[1]->low;
			s->pairs[0]->type=NOTHING;
			s->pairs[1]->type=NOTHING;
			memmove(s->pairs,s->pairs+2,(s->pairs_num-2)*sizeof(void*));
		}
		else {
			int j=0;
			for ( int i=h->begin; i<=max(P2,h->end); i++) {
				if ( h->hands[i]>=2 && i!=c->low)
				{
					c->three_desc[j++]= i;
					// h->hands[i]-=2;
					if (j>=2)
					{
						c->type = 3322;
						player->need_research = 1;
						h->hands[c->three_desc[0]]  -=2;
						h->hands[c->three_desc[1]]  -=2;
						return 1;
					}
				}
			}

			return 0;
		}
	}

	if (p->type == 3311 )
	{
		if (player->summary->singles_num>1) {
			c->type=3311;
			c->three_desc[0]= player->summary->singles[0]->low;
			c->three_desc[1]= player->summary->singles[1]->low;

			remove_combo_poker(h,player->summary->singles[0],NULL);
			remove_combo_poker(h,player->summary->singles[1],NULL);
			s->singles_num -=2 ;
			if (s->singles[0]->control!=1)
				s->combo_total_num --;
			else
				s->biggest_num --;
			if (s->singles[1]->control!=1)

				s->combo_total_num --;
			else
				s->biggest_num --;

			s->real_total_num -=2;
			s->singles[0]->type=NOTHING;
			s->singles[1]->type=NOTHING;
			memmove(s->singles,s->singles+2,(s->singles_num)*sizeof(void*));
		}
		else if (player->summary->pairs_num>0)
		{
			c->type=3311;
			c->three_desc[0]= s->pairs[0]->low;
			c->three_desc[1]= s->pairs[0]->low;
			s->pairs[0]->type=NOTHING;
			memmove(s->pairs,s->pairs+1,(s->pairs_num-1)*sizeof(void*));
		}
		else {
			int j=0;
			for ( int i=h->begin; i<=h->end; i++)
				if (h->hands[i]>0)
				{
					c->type=3311;
					c->three_desc[j++]= i;
					h->hands[i]--;
					if (j==2)
						//			{
						break;
					//         	}
				}
				player->need_research = 1;
				//            search_combos_in_suit(game,h,player->opps,player);
				return 0;
		}
	}
	return 1;
}

int get_combo_num_for_player_win(PLAYER* pl)
{
	//todo:refine it
	return CONTROL_SUB_COMBO_NUM(pl);
}

int is_player_ready_win(GAME* game,PLAYER* pl)
{
	//todo:refine it
	return CONTROL_SUB_COMBO_NUM_IS_GOOD(pl);
}

int rand_a_poker(POKERS *all)
{
	int k;
	while (1)
	{
		k=rand()%MAX_POKER_KIND;
		if (all->hands[k]>0)
		{
			all->hands[k]--;
			return k;
		}
	}
}

void game_init(GAME* game)
{
	//        game->players[CUR_PLAYER]->id = game->player[CUR_PLAYER].type;
	//		game->players[UP_PLAYER]->id = game->player[UP_PLAYER].type;
	//		game->players[DOWN_PLAYER]->id = game->player[DOWN_PLAYER].type;

	game->players[CUR_PLAYER]->lower_control_poker= P2;//get_lowest_controls(&game->all, CONTROL_POKER_NUM);

	//	  game->player[CUR_PLAYER].cur = &game->player[CUR_PLAYER].curcomb;
	//		  game->player[UP_PLAYER].cur = &game->player[UP_PLAYER].curcomb;
	//	  game->player[DOWN_PLAYER].cur = &game->player[DOWN_PLAYER].curcomb;

	game->computer[0]=1;
	game->computer[1]=1;
	game->computer[2]=1;

	//search and initialize Suits
	search_combos_in_suit(game,game->players[CUR_PLAYER]->h,game->players[CUR_PLAYER]->opps,game->players[CUR_PLAYER]);
	if(game->known_others_poker)
	{
		search_combos_in_suit(game,game->players[DOWN_PLAYER]->h,game->players[DOWN_PLAYER]->opps,game->players[DOWN_PLAYER]);
		search_combos_in_suit(game,game->players[UP_PLAYER]->h,game->players[UP_PLAYER]->opps,game->players[UP_PLAYER]);
	}
	game->good_farmer_0 =50;
	game->good_farmer_1 =20;
}

//search in poker h to find a combo c bigger than a
int find_a_bigger_combo_in_hands_hun(POKERS* h, COMBO_OF_POKERS * c, COMBO_OF_POKERS* a)
{
	c->control=0;
	if (h->total<get_combo_number(a))
	{
		goto _find_bomb;
	}

	switch ( a->type )
	{
	case ROCKET:
		;
		return false;
	case BOMB:
		if (getBigBomb_hun(h,c,a))
		{
			return true;
		}
		return false;
	case SINGLE_SERIES:
		if (getBigSeries_hun(h,c,a->low+1,CEIL_A(h->end),1,a->len)) //todo: fix me
		{
			return true;
		}
		break;

	case PAIRS_SERIES:
		if (getBigSeries_hun(h,c,a->low+1,CEIL_A(h->end),2,a->len)) //todo: fix me
		{
			return true;
		}
		break;
	case THREE_SERIES:
		if (getBigSeries_hun(h,c,a->low+1,CEIL_A(h->end),3,a->len))//todo: fix me
		{
			return true;
		}
		break;
	case 3311: //todo fixme
		c->len =a->len;
		if(a->control==2) //search a big attach for three
			// fix me!
		{
			for (int i=a->three_desc[1]+1; i<=h->end; i++)
			{
				if (h->hands[i] >0 &&  i!=a->low && i!=a->low+1
					&& i!=h->hun)
				{
					c->type = 3311;
					c->low  = a->low;
					c->three_desc[0]=a->three_desc[0];
					c->three_desc[1] = i;
					return true;
				}
			}

			for (int i=a->three_desc[0]+1; i<=h->end; i++)
			{
				if (h->hands[i] >0 &&  i!=a->low && i!=a->low+1 && i!=h->hun)
				{
					c->type = 3311;
					c->low  = a->low;
					c->three_desc[0] = i;
					if (h->hands[i] >1)
					{
						c->three_desc[1] = i;
						return true;
					}

					for (int j=i+1; j<=h->end; j++)
					{

						if (h->hands[j] >0 &&  j!=a->low && j!=a->low+1 && j!=h->hun)
						{
							c->three_desc[1] = j;
							return true;
						}
					}

				}
			}

		}

		if (getBigSeries(h,c,a->low+1,CEIL_A(h->end),3,a->len))//todo: fix me
		{
			for (int i=h->begin; i<=h->end; i++)
			{
				if (h->hands[i] >0 &&  i!=c->low && i!=c->low+1 && i!=h->hun)
				{
					c->type = 3311;
					//c->low  = a->low;
					c->three_desc[0] = i;
					if (h->hands[i] >1)
					{
						c->three_desc[1] = i;
						return true;
					}

					for (int j=i+1; j<=h->end; j++)
					{

						if (h->hands[j] >0 &&  j!=c->low && j!=c->low+1 && j!=h->hun)
						{
							c->three_desc[1] = j;
							return true;
						}
					}

				}
			}
		}
		return false;
		break;
	case 333111:
	case 33331111:
	case 531:
		return false;
		if (getBigSeries_hun(h,c,a->low+1,CEIL_A(h->end),3,a->len))//todo: fix me
		{
			return true;
		}
		break;
	case 3322:
		c->len =a->len;
		c->type = 3322;
		c->low  = a->low;

		if(a->control==2) //search a big attach for three
		{
			for (int i=a->three_desc[1]+1; i<=h->end; i++)
			{
				if (h->hands[i] >1 &&  i!=a->low && i!=a->low+1 && i!=h->hun)
				{
					c->three_desc[0]=a->three_desc[0];
					c->three_desc[1] = i;
					return true;
				}
			}

			for (int i=a->three_desc[0]+1; i<=h->end; i++)
			{
				if (h->hands[i] >1 &&  i!=a->low && i!=a->low+1 && i!=h->hun)
				{
					c->three_desc[0] = i;
					for (int j=i+1; j<=h->end; j++)
					{

						if (h->hands[j] >1 &&  j!=a->low && j!=a->low+1 && j!=h->hun)
						{
							c->three_desc[1] = j;
							return true;
						}
					}

				}
			}

		}

		if (getBigSeries(h,c,a->low+1,CEIL_A(h->end),3,a->len))//todo: fix me
		{
			for (int i=h->begin; i<=h->end; i++)
			{
				if (h->hands[i] >1 &&  i!=c->low && i!=c->low+1 && i!=h->hun)
				{
					c->type = 3322;
					//                         c->low  = a->low;
					c->three_desc[0] = i;

					for (int j=i+1; j<=h->end; j++)
					{

						if (h->hands[j] >1 &&  j!=c->low && j!=c->low+1&& j!=h->hun)
						{
							c->three_desc[1] = j;
							return true;
						}
					}
				}
			}
		}
		return false;
	case 333222:
	case 33332222:
		return false;
		if (getBigSeries_hun(h,c,a->low+1,CEIL_A(h->end),3,a->len))//todo: fix me
		{
			return true;
		}
		break;
	case 31:
		{
			if(a->control==2) //search a big attach for three
			{
				for (int i=a->three_desc[0]+1; i<=h->end; i++)
				{
					if (h->hands[i] >0 &&  i!=a->low  && i!=h->hun)
					{
						c->type = THREE_ONE;
						c->low  = a->low;
						c->three_desc[0] = i;
						c->len = 1;
						return true;
					}
				}
			}

			if(getBigSingle_hun(h, c, a->low+1, P2, 3))
			{
				if(c->low==h->hun)
					return false;
				c->type = THREE_ONE;
				c->three_desc[0]=-1; 
				for (int j=h->begin ; j<=h->end; j++)
				{
					if(h->hands[j] >0 && j!=c->low && j!=h->hun) {
						c->three_desc[0] = j;
						c->len = 1;
						return true;
					}
				}		   
			}
			return false;
		}
		break;
	case 32:
		{
			if(a->control==2) //search a big attach for three
			{
				for (int i=a->three_desc[0]+1; i<=h->end; i++)
				{
					if (h->hands[i] >=2 &&  i!=a->low && i!=h->hun)//todo fixme!
					{
						c->type = 32;
						c->low  = a->low;
						c->three_desc[0] = i;
						c->len = 1;
						return true;
					}
				}
			}

			if(getBigSingle_hun(h, c, a->low+1, P2, 3))
			{
				if(c->low==h->hun)
					return false;
				c->type = THREE_TWO;
				c->three_desc[0]=-1; 
				for (int j=h->begin ; j<=h->end; j++)
				{
					if(h->hands[j] >=2 && j!=c->low && j!=h->hun) {
						c->three_desc[0] = j;
						c->len = 1;
						return true;
					}
				}		   
			}
			return false;
		}
		break;
	case THREE:
		if (getBigSingle_hun(h,c,a->low+1,min(h->end,P2),3)) //todo: fix me
		{
			return true;
		}
		break;
	case PAIRS :
		if (getBigSingle_hun(h,c,a->low+1,min(h->end,P2),2)) //todo: fix me
		{
			return true;
		}
		break;
	case SINGLE :
		if (getBigSingle_hun(h,c,a->low+1,h->end,1)) //todo: fix me
		{
			return true;
		}
	default:
		break;
	}

_find_bomb:	
	///*
	if(a->type!=BOMB)
	{
		return getBomb_hun(h, c);
	}
	//*/
	return false;
}
//search in poker h to find a combo c bigger than a
int find_a_bigger_combo_in_hands(POKERS* h, COMBO_OF_POKERS * c, COMBO_OF_POKERS* a)
{
	c->control=0;
	if( h->hun!=-1)
	{
		return find_a_bigger_combo_in_hands_hun(h,c,a);
	}
	//= player->h;
	if (h->total<get_combo_number(a))
	{
		goto _find_bomb;
	}

	switch ( a->type )
	{
	case ROCKET:
		;
		return false;
	case BOMB:
		if (getBigBomb(h,c,a))
		{
			return true;
		}
		return false;
	case SINGLE_SERIES:
		if (getBigSeries(h,c,a->low+1,CEIL_A(h->end),1,a->len)) //todo: fix me
		{
			return true;
		}
		break;

	case PAIRS_SERIES:
		if (getBigSeries(h,c,a->low+1,CEIL_A(h->end),2,a->len)) //todo: fix me
		{
			return true;
		}
		break;
	case THREE_SERIES:
		if (getBigSeries(h,c,a->low+1,CEIL_A(h->end),3,a->len))//todo: fix me
		{
			return true;
		}
		break;
	case 3311:
		c->len =a->len;
		if(h->hands[a->low]==3) //search a big attach for three
		{
			for (int i=a->three_desc[1]+1; i<=h->end; i++)
			{
				if (h->hands[i] >0 &&  i!=a->low && i!=a->low+1)
				{
					c->type = 3311;
					c->low  = a->low;
					c->three_desc[0]=a->three_desc[0];
					c->three_desc[1] = i;
					return true;
				}
			}

			for (int i=a->three_desc[0]+1; i<=h->end; i++)
			{
				if (h->hands[i] >0 &&  i!=a->low && i!=a->low+1)
				{
					c->type = 3311;
					c->low  = a->low;
					c->three_desc[0] = i;
					if (h->hands[i] >1)
					{
						c->three_desc[1] = i;
						return true;
					}

					for (int j=i+1; j<=h->end; j++)
					{

						if (h->hands[j] >0 &&  j!=a->low && j!=a->low+1)
						{
							c->three_desc[1] = j;
							return true;
						}
					}

				}
			}

		}

		if (getBigSeries(h,c,a->low+1,CEIL_A(h->end),3,a->len))//todo: fix me
		{
			for (int i=h->begin; i<=h->end; i++)
			{
				if (h->hands[i] >0 &&  i!=c->low && i!=c->low+1)
				{
					c->type = 3311;
					//c->low  = a->low;
					c->three_desc[0] = i;
					if (h->hands[i] >1)
					{
						c->three_desc[1] = i;
						return true;
					}

					for (int j=i+1; j<=h->end; j++)
					{

						if (h->hands[j] >0 &&  j!=c->low && j!=c->low+1)
						{
							c->three_desc[1] = j;
							return true;
						}
					}

				}
			}
		}
		return false;
		break;
	case 333111:
	case 33331111:
	case 531:
		return false;
		if (getBigSeries(h,c,a->low+1,CEIL_A(h->end),3,a->len))//todo: fix me
		{
			return true;
		}
		break;
	case 3322:
		c->len =a->len;
		c->type = 3322;
		c->low  = a->low;

		if(h->hands[a->low]==3) //search a big attach for three
		{
			for (int i=a->three_desc[1]+1; i<=h->end; i++)
			{
				if (h->hands[i] >1 &&  i!=a->low && i!=a->low+1)
				{
					c->three_desc[0]=a->three_desc[0];
					c->three_desc[1] = i;
					return true;
				}
			}

			for (int i=a->three_desc[0]+1; i<=h->end; i++)
			{
				if (h->hands[i] >1 &&  i!=a->low && i!=a->low+1)
				{
					c->three_desc[0] = i;
					for (int j=i+1; j<=h->end; j++)
					{

						if (h->hands[j] >1 &&  j!=a->low && j!=a->low+1)
						{
							c->three_desc[1] = j;
							return true;
						}
					}

				}
			}

		}

		if (getBigSeries(h,c,a->low+1,CEIL_A(h->end),3,a->len))//todo: fix me
		{
			for (int i=h->begin; i<=h->end; i++)
			{
				if (h->hands[i] >1 &&  i!=c->low && i!=c->low+1)
				{
					c->type = 3322;
					//                         c->low  = a->low;
					c->three_desc[0] = i;

					for (int j=i+1; j<=h->end; j++)
					{

						if (h->hands[j] >1 &&  j!=c->low && j!=c->low+1)
						{
							c->three_desc[1] = j;
							return true;
						}
					}
				}
			}
		}
		return false;
	case 333222:
	case 33332222:
		return false;
		if (getBigSeries(h,c,a->low+1,CEIL_A(h->end),3,a->len))//todo: fix me
		{
			return true;
		}
		break;
	case 31:
		{
			if(h->hands[a->low]==3) //search a big attach for three
			{
				for (int i=a->three_desc[0]+1; i<=h->end; i++)
				{
					if (h->hands[i] >0 &&  i!=a->low )
					{
						c->type = THREE_ONE;
						c->low  = a->low;
						c->three_desc[0] = i;
						c->len = 1;
						return true;
					}
				}
			}

			for (int i=a->low+1 ; i<=h->end; i++)
			{
				if (h->hands[i] == 3) //Got Three,doesn't check bomb..
				{
					c->type = THREE_ONE;
					c->low  = i;
					c->three_desc[0]=-1;
					for (int j=h->begin ; j<=h->end; j++)
					{
						if(h->hands[j] >0 && j!=c->low) {
							c->three_desc[0] = j;
							c->len = 1;
							return true;
						}
					}
				}
			}
			return false;
		}
		break;
	case 32:
		{
			if(h->hands[a->low]==3) //search a big attach for three
			{
				for (int i=a->three_desc[0]+1; i<=h->end; i++)
				{
					if (h->hands[i] >=2 &&  i!=a->low )
					{
						c->type = 32;
						c->low  = a->low;
						c->three_desc[0] = i;
						c->len = 1;
						return true;
					}
				}
			}

			for (int i=a->low+1 ; i<=h->end; i++)
			{
				if (h->hands[i] == 3) //Got Three,doesn't check bomb..
				{
					c->type = 32;
					c->low  = i;
					for (int j=h->begin ; j<=h->end; j++)
					{
						if(h->hands[j] >=2 && j!=c->low)
						{
							c->three_desc[0] = j;
							c->len = 1;
							return true;
						}
					}
				}
			}
			return false;
		}
		break;
	case THREE:
		if (getBigThree(h,c,a)) //todo: fix me
		{
			return true;
		}
		break;
	case PAIRS :
		if (getBigSingle(h,c,a->low+1,h->end,2)) //todo: fix me
		{
			return true;
		}
		break;
	case SINGLE :
		if (getBigSingle(h,c,a->low+1,h->end,1)) //todo: fix me
		{
			return true;
		}
	default:
		break;
	}

_find_bomb:	
	///*
	if(a->type!=BOMB)
	{
		for (int i=h->begin; i<=min(P2,h->end); i++)
		{
			if ( h->hands[i] == 4)
			{
				c->type= BOMB;
				c->low = i;
				return true;
			}
		}
		if( h->hands[BIG_JOKER] && h->hands[LIT_JOKER] )
		{
			c->type=ROCKET;
			c->low=LIT_JOKER;
			return true;
		}
	}
	//*/
	return false;
}

int has_combo_bigger_than_c_in_hands(POKERS * h , COMBO_OF_POKERS * c)
{
	COMBO_OF_POKERS t;
	return find_a_bigger_combo_in_hands(h, &t,c);
}

int find_a_smaller_combo_in_hands_hun(POKERS* h, COMBO_OF_POKERS * c, COMBO_OF_POKERS* a)
{
	//= player->h;
	if (h->total<get_combo_number(a))
		return false;
	switch ( a->type )
	{
	case ROCKET:
		;
		return false;
	case BOMB:
		return false;
	case SINGLE_SERIES:
		if (getSmallSeries_hun(h,c,a->low,1,a->len))
		{
			return true;
		}
		break;

	case PAIRS_SERIES:

	case THREE_SERIES:

	case 3311:
	case 333111:
	case 33331111:
	case 531:

	case 3322:
	case 333222:
	case 33332222:
		return false;
	case 31:
	case 32:
	case THREE:
		if (getSmallSingle_hun(h,c,a->low,3))
		{
			return true;
		}
		break;
	case PAIRS :
		if (getSmallSingle_hun(h,c,a->low,2))
		{
			return true;
		}
		break;
	case SINGLE :
		if (getSmallSingle_hun(h,c,a->low,1))
		{
			return true;
		}
	default:
		break;
	}
	return false;
}


//search in poker h to find a  combo c smaller than a
int find_a_smaller_combo_in_hands(POKERS* h, COMBO_OF_POKERS * c, COMBO_OF_POKERS* a)
{
	if(h->hun!=-1)
		return find_a_smaller_combo_in_hands_hun(h,c,a);
	//= player->h;
	if (h->total<get_combo_number(a))
		return false;
	switch ( a->type )
	{
	case ROCKET:
		;
		return false;
	case BOMB:
		return false;
	case SINGLE_SERIES:
		if (getSmallSeries(h,c,a->low,1,a->len))
		{
			return true;
		}
		break;

	case PAIRS_SERIES:

	case THREE_SERIES:

	case 3311:
	case 333111:
	case 33331111:
	case 531:

	case 3322:
	case 333222:
	case 33332222:
		return false;
	case 31:
	case 32:
	case THREE:
		if (getSmallSingle(h,c,a->low,3))
		{
			return true;
		}
		break;
	case PAIRS :
		if (getSmallSingle(h,c,a->low,2))
		{
			return true;
		}
		break;
	case SINGLE :
		if (getSmallSingle(h,c,a->low,1))
		{
			return true;
		}
	default:
		break;
	}
	return false;
}


//a stupid fucntion...
//select  a combo c from player->h, but not remove
//arrange the other poker in player->h to combos and save summary to player->summary
int	rearrange_suit(GAME* game,PLAYER *player,COMBO_OF_POKERS* c)
{
	POKERS t;
	DBG( PRINTF(LEVEL,"  Chaipai  \n"));
	memmove(&t,player->h,sizeof(POKERS));
	int res=remove_combo_poker(&t,c,NULL);
	search_combos_in_suit(game,&t,player->opps,player);
	return res;
}

// Remove combo a from the player s
// return true if success
int remove_combo_in_suit( COMBOS_SUMMARY *s , COMBO_OF_POKERS* a)
{
	//   COMBOS_SUMMARY *s =player->summary;
	switch ( a->type )
	{
	case ROCKET:
		//s->singles[s->series_num]=&c[k];
		//s->rocket_num++;
		//break;
	case BOMB:
		if (s->bomb_num>0)
		{
			for (int k=0; k<s->bomb_num; k++)
			{
				if ( s->bomb[k]->low == a->low) {
					//memmove(c,s->bomb[k],sizeof(COMBO_OF_POKERS));
					if (k!=s->bomb_num-1)
						memmove(&s->bomb[k], &s->bomb[k+1], (s->bomb_num-k)*sizeof(void*));
					s->bomb_num --;
					return true;

				}
			}
		}
		break;
	case SINGLE_SERIES:
		if (s->series_num>0)
		{
			for (int k=0; k<s->series_num; k++)
			{
				if ( s->series[k]->low == a->low && s->series[k]->len == a->len) {
					// memmove(c,s->series[k],sizeof(COMBO_OF_POKERS));
					if (k!=s->series_num-1)
						memmove(&s->series[k], &s->series[k+1], (s->series_num-k)*sizeof(void*));
					s->series_num --;
					s->series_detail[a->len-5]--;
					return true;

				}
			}
		}
		break;
	case PAIRS_SERIES:
		if (s->pairs_series_num>0)
		{
			for (int k=0; k<s->pairs_series_num; k++)
			{
				if ( s->pairs_series[k]->low == a->low && s->pairs_series[k]->len == a->len) {
					// memmove(c,s->series[k],sizeof(COMBO_OF_POKERS));
					if (k!=s->pairs_series_num-1)
						memmove(&s->pairs_series[k], &s->pairs_series[k+1], (s->pairs_series_num-k)*sizeof(void*));
					s->pairs_series_num --;
					//s->series_detail[a->len-5]--;
					return true;

				}
			}
		}
		break;

	case THREE_SERIES:
		if (s->threeseries_num>0)
		{
			for (int k=0; k<s->threeseries_num; k++)
			{
				if ( s->threeseries[k]->low == a->low && s->threeseries[k]->len == a->len) {
					// memmove(c,s->series[k],sizeof(COMBO_OF_POKERS));
					if (k!=s->threeseries_num-1)
						memmove(&s->threeseries[k], &s->threeseries[k+1], (s->threeseries_num-k)*sizeof(void*));
					s->threeseries_num --;
					//s->series_detail[a->len-5]--;
					return true;

				}
			}
		}
		break;
	case THREE:
		for (int k=0; k<s->three_num; k++)
		{
			if ( s->three[k]->low == a->low ) {
				// memmove(c,s->series[k],sizeof(COMBO_OF_POKERS));
				if (k!=s->three_num-1)
					memmove(&s->three[k], &s->three[k+1], (s->three_num-k)*sizeof(void*));
				s->three_num --;
				return true;

			}
		}
		break;

		break;
	case 31:
		if (s->three_one_num>0)
		{
			for (int k=0; k<s->three_one_num; k++)
			{
				if ( s->three_one[k]->low == a->low  ) {
					// memmove(c,s->three_one[k],sizeof(COMBO_OF_POKERS));
					if (k!=s->three_one_num-1)
						memmove(&s->three_one[k], &s->three_one[k+1], (s->three_one_num-k)*sizeof(void*));
					s->three_one_num --;
					return true;

				}
			}
		}
		break;
	case 32:
		if (s->three_two_num>0)
		{
			for (int k=0; k<s->three_two_num; k++)
			{
				if ( s->three_two[k]->low == a->low  ) {
					//  memmove(c,s->three_two[k],sizeof(COMBO_OF_POKERS));
					if (k!=s->three_two_num-1)
						memmove(&s->three_two[k], &s->three_two[k+1], (s->three_two_num-k)*sizeof(void*));
					s->three_two_num --;
					return true;

				}
			}
		}
		break;
	case PAIRS :
		if (s->pairs_num>0)
		{
			for (int k=0; k<s->pairs_num; k++)
			{
				if ( s->pairs[k]->low == a->low  ) {
					//  memmove(c,s->pairs[k],sizeof(COMBO_OF_POKERS));
					if (k!=s->pairs_num-1)
						memmove(&s->pairs[k], &s->pairs[k+1], (s->pairs_num-k)*sizeof(void*));
					s->pairs_num --;
					return true;
				}
			}
		}
		break;
	case SINGLE :
		if (s->singles_num>0)
		{
			for (int k=0; k<s->singles_num; k++)
			{
				if ( s->singles[k]->low == a->low  ) {
					//memmove(c,s->singles[k],sizeof(COMBO_OF_POKERS));
					if (k!=s->singles_num-1)
						memmove(&s->singles[k], &s->singles[k+1], (s->singles_num-k)*sizeof(void*));
					s->singles_num --;
					return true;
				}
			}
		}
		break;
	case 3311:
	case 333111:
	case 33331111:
	case 531:
		if (s->threeseries_one_num>0)
		{
			for (int k=0; k<s->threeseries_one_num; k++)
			{
				if ( s->threeseries_one[k]->low == a->low  ) {
					//memmove(c,s->singles[k],sizeof(COMBO_OF_POKERS));
					if (k!=s->threeseries_one_num-1)
						memmove(&s->threeseries_one[k], &s->threeseries_one[k+1], (s->threeseries_one_num-k)*sizeof(void*));
					s->threeseries_one_num --;
					return true;
				}
			}
		}
		break;
	case 3322:
	case 333222:
	case 33332222:
		if (s->threeseries_two_num>0)
		{
			for (int k=0; k<s->threeseries_two_num; k++)
			{
				if ( s->threeseries_two[k]->low == a->low  ) {
					//memmove(c,s->singles[k],sizeof(COMBO_OF_POKERS));
					if (k!=s->threeseries_two_num-1)
						memmove(&s->threeseries_two[k], &s->threeseries_two[k+1], (s->threeseries_two_num-k)*sizeof(void*));
					s->threeseries_two_num --;
					return true;
				}
			}
		}
		break;
	case 411:
		{
			for (int k=0; k<s->four_one_num; k++)
			{
				if ( s->four_one[k]->low == a->low  ) {
					//memmove(c,s->singles[k],sizeof(COMBO_OF_POKERS));
					if (k!=s->four_one_num-1)
						memmove(&s->four_one[k], &s->four_one[k+1], (s->four_one_num-k)*sizeof(void*));
					s->four_one_num --;
					return true;
				}
			}
		}
		break;

	case 422:
		{
			for (int k=0; k<s->four_two_num; k++)
			{
				if ( s->four_two[k]->low == a->low  ) {
					//memmove(c,s->singles[k],sizeof(COMBO_OF_POKERS));
					if (k!=s->four_one_num-1)
						memmove(&s->four_two[k], &s->four_two[k+1], (s->four_two_num-k)*sizeof(void*));
					s->four_two_num --;
					return true;
				}
			}
		}
		break;

	default:
		PRINTF_ALWAYS("!!ERR,in line %d :  no such combos in player\n",__LINE__);
	}
	return false;
}

//save all combos to in order..
COMBO_OF_POKERS** find_all_combo_bigger_than_combo_a( COMBOS_SUMMARY *s ,
	COMBO_OF_POKERS* a,int* number)//find all c > a in s
{
	//   COMBOS_SUMMARY *s =player->summary;
	*number = 0;
	switch ( a->type )
	{
	case ROCKET:
		//s->singles[s->series_num]=&c[k];
		//s->rocket_num++;
		//break;
		return NULL;
	case BOMB:
		//s->bomb[s->bomb_num++]=;
		if (s->bomb_num>0)
		{
			for (int k=0; k<s->bomb_num; k++)
			{
				if ( s->bomb[k]->low > a->low) {
					*number = s->bomb_num-k;
					return &s->bomb[k];
				}
			}
		}
		break;
	case SINGLE_SERIES:
		if (s->series_num>0)
		{
			for (int k=0; k<s->series_num; k++)
			{
				if ( s->series[k]->low > a->low && s->series[k]->len == a->len) {
					*number = s->series_num-k;
					return &s->series[k];
				}
			}
		}
		break;
	case PAIRS_SERIES:
		if (s->pairs_series_num>0)
		{
			for (int k=0; k<s->pairs_series_num; k++)
			{
				if ( s->pairs_series[k]->low > a->low && s->pairs_series[k]->len == a->len) {
					*number = s->pairs_series_num-k;
					return &s->pairs_series[k];
				}
			}
		}
		break;
	case THREE_SERIES:
		if (s->threeseries_num>0)
		{
			for (int k=0; k<s->threeseries_num; k++)
			{
				if ( s->threeseries[k]->low > a->low && s->threeseries[k]->len == a->len) {
					*number = s->threeseries_num-k;
					return &s->threeseries[k];
				}
			}
		}
		break;
		break;
	case THREE:
		if (s->three_num>0)
		{
			for (int k=0; k<s->three_num; k++)
			{
				if ( s->three[k]->low > a->low && s->three[k]->len == a->len) {
					*number = s->three_num-k;
					return &s->three[k];
				}
			}
		}
		break;
	case 31:
		if (s->three_one_num>0)
		{
			for (int k=0; k<s->three_one_num; k++)
			{
				if ( s->three_one[k]->low > a->low  ) {
					*number = s->three_one_num-k;
					return &s->three_one[k];
				}
			}
		}
		break;
	case 32:
		if (s->three_two_num>0)
		{
			for (int k=0; k<s->three_two_num; k++)
			{
				if ( s->three_two[k]->low > a->low  ) {
					*number = s->three_two_num-k;
					return &s->three_two[k];
				}
			}
		}
		break;
	case PAIRS :
		if (s->pairs_num>0)
		{
			for (int k=0; k<s->pairs_num; k++)
			{
				if ( s->pairs[k]->low > a->low  ) {
					*number = s->pairs_num-k;
					return &s->pairs[k];
				}
			}
		}
		break;
	case SINGLE :
		if (s->singles_num>0)
		{
			for (int k=0; k<s->singles_num; k++)
			{
				if ( s->singles[k]->low > a->low  ) {
					*number = s->singles_num-k;
					return &s->singles[k];
				}
			}
		}
		break;
	case 3311:
	case 333111:
	case 33331111:
	case 531:
		if (s->threeseries_one_num>0)
		{
			for (int k=0; k<s->threeseries_one_num; k++)
			{
				if ( s->threeseries_one[k]->low > a->low && s->threeseries_one[k]->len == a->len) {
					*number = s->threeseries_one_num-k;
					return &s->threeseries_one[k];
				}
			}
		}
		break;
	case 3322:
	case 333222:
	case 33332222:
		if (s->threeseries_one_num>0)
		{
			for (int k=0; k<s->threeseries_two_num; k++)
			{
				if ( s->threeseries_two[k]->low > a->low && s->threeseries_two[k]->len == a->len) {
					*number = s->threeseries_two_num-k;
					return &s->threeseries_two[k];
				}
			}
		}
		break;

	}
	/*
	if (a->type!=BOMB && s->bomb_num>0)
	{
	for (int k=0; k<s->bomb_num; k++)
	{
	*number=1;
	return &s->bomb[k];
	}
	}
	*/
	return NULL;
}

COMBO_OF_POKERS* find_combo( COMBOS_SUMMARY *s , COMBO_OF_POKERS* a)//find c > a in s and return c.
{
	//   COMBOS_SUMMARY *s =player->summary;
	switch ( a->type )
	{
	case ROCKET:
		//s->singles[s->series_num]=&c[k];
		//s->rocket_num++;
		//break;
		return NULL;
	case BOMB:
		//s->bomb[s->bomb_num++]=;
		if (s->bomb_num>0)
		{
			for (int k=0; k<s->bomb_num; k++)
			{
				if (check_combo_a_Big_than_b(s->bomb[k], a)) {
					return s->bomb[k];
				}
			}
		}
		break;
	case SINGLE_SERIES:
		if (s->series_num>0)
		{
			for (int k=0; k<s->series_num; k++)
			{
				if ( s->series[k]->low > a->low && s->series[k]->len == a->len) {
					return s->series[k];
				}
			}
		}
		break;
	case PAIRS_SERIES:
		if (s->pairs_series_num>0)
		{
			for (int k=0; k<s->pairs_series_num; k++)
			{
				if ( s->pairs_series[k]->low > a->low && s->pairs_series[k]->len == a->len) {
					return s->pairs_series[k];
				}
			}
		}
		break;
	case THREE_SERIES:
		if (s->threeseries_num>0)
		{
			for (int k=0; k<s->threeseries_num; k++)
			{
				if ( s->threeseries[k]->low > a->low && s->threeseries[k]->len == a->len) {
					return s->threeseries[k];
				}
			}
		}
		break;
		break;
	case THREE:
		if (s->three_num>0)
		{
			for (int k=0; k<s->three_num; k++)
			{
				if ( s->three[k]->low > a->low && s->three[k]->len == a->len) {
					return s->three[k];
				}
			}
		}
		break;
	case 31:
		if (s->three_one_num>0)
		{
			for (int k=0; k<s->three_one_num; k++)
			{
				if ( s->three_one[k]->low > a->low  ) {
					return s->three_one[k];
				}
			}
		}
		break;
	case 32:
		if (s->three_two_num>0)
		{
			for (int k=0; k<s->three_two_num; k++)
			{
				if ( s->three_two[k]->low > a->low  ) {
					return s->three_two[k];
				}
			}
		}
		break;
	case PAIRS :
		if (s->pairs_num>0)
		{
			for (int k=0; k<s->pairs_num; k++)
			{
				if ( s->pairs[k]->low > a->low  ) {
					return s->pairs[k];
				}
			}
		}
		break;
	case SINGLE :
		if (s->singles_num>0)
		{
			for (int k=0; k<s->singles_num; k++)
			{
				if ( s->singles[k]->low > a->low  ) {
					return s->singles[k];
				}
			}
		}
		break;
	case 3311:
	case 333111:
	case 33331111:
	case 531:
		if (s->threeseries_one_num>0)
		{
			for (int k=0; k<s->threeseries_one_num; k++)
			{
				if ( s->threeseries_one[k]->low > a->low && s->threeseries_one[k]->len == a->len) {
					return s->threeseries_one[k];
				}
			}
		}
		break;
	case 3322:
	case 333222:
	case 33332222:
		if (s->threeseries_one_num>0)
		{
			for (int k=0; k<s->threeseries_two_num; k++)
			{
				if ( s->threeseries_two[k]->low > a->low && s->threeseries_two[k]->len == a->len) {
					return s->threeseries_two[k];
				}
			}
		}
		break;

	}
	if (a->type!=BOMB && s->bomb_num>0)
	{
		for (int k=0; k<s->bomb_num; k++)
		{
			return s->bomb[k];
		}
	}
	return NULL;
}

int has_control_poker(COMBO_OF_POKERS* c, int lower)
{
	if (c->type == BOMB|| c->type ==ROCKET)
		return c->low;
	if(c->type == PAIRS || c->type == SINGLE)
		c->len =1;
	if (c->len ==0)
	{
		//  PRINTF_ALWAYS("ERR: len of type is zero..! type %d\n",c->type);
		c->len=1;
	}
	if ( c->low + c->len -1 >= lower ) //for theries , pairs, single
		return c->low + c->len -1;
	if ( c->type == THREE_ONE || c->type == THREE_TWO )
	{
		if ( c->three_desc[0] >=lower )
			return c->three_desc[0];
	}
	if ( c->type >=3311 ) //plane , trick..
	{
		if ( c->three_desc[0] >=lower ||  c->three_desc[1] >=lower )
			return max(c->three_desc[0],c->three_desc[1]);
	}
	return -1;
}

//remove 1 or 2 pokers from opps hand
void assume_remove_controls_in_opps(POKERS * opps, COMBO_OF_POKERS * c, int begin)
{
	//     ASSERT(begin==c->low);
	if (c->type==SINGLE)       {
		for (int k=begin+1; k<=opps->end; k++)
			if (  opps->hands[k]>0)
			{
				opps->hands[k]--;
				break;		
			}
	}
	else if ( c->type==PAIRS)
	{
		for (int k=begin+1; k<=opps->end; k++)
			if (  opps->hands[k]>=2)
			{
				opps->hands[k]-=2;
				break;		
			}
			//    if (  opps->hands[LIT_JOKER]==1 &&opps->hands[BIG_JOKER]==1 )
			//            opps->hands[LIT_JOKER]=opps->hands[BIG_JOKER]=0;

	}
}

int  find_the_biggest_combo_great_pre(GAME* game,COMBO_OF_POKERS* pre,COMBO_OF_POKERS * res)

{
	COMBOS_SUMMARY * summary = game->players[CUR_PLAYER]->summary;

	//todo: add more check here if other has bombs
	if(summary->bomb_num>0) //
	{
		int i=0;
		for(; i<summary->bomb_num; i++)
			if(check_combo_a_Big_than_b(summary->bomb[i],pre))
			{
				*res=*summary->bomb[i];
				return 1;
			}
	}
	else
	{

		COMBO_OF_POKERS * ret,*p,*c,tmp,tmp2;
		c=&tmp;
		p=&tmp2;
		*p=*pre;
		int find=0;
		while( find_a_bigger_combo_in_hands(game->players[CUR_PLAYER]->h,c,p))
		{
			*res=*c;
			*p=*c;
			find =1;
		}

		return find;
	}
	return 0;
}


int check_only_2_same_combo(GAME* game,COMBO_OF_POKERS* cur)
{
	SET_PLAYER_POINT(game);
	//    PLAYER *pl=game->players[CUR_PLAYER];
	COMBOS_SUMMARY * s = pl->summary;
	if(s->real_total_num ==2  &&  s->combo_typenum == 1)
	{
		//two single!
		if(s->singles_num == 2)
		{
			if((pl->id==UPFARMER && pl->oppDown_num ==1 )
				||(pl->id==DOWNFARMER && pl->oppUp_num ==1 ))		
			{
				*cur =*s->singles[1];
				return 1;  
			}

			if(( s->singles[1]->low > pl->opps->end)
				||(game->known_others_poker && pl->id==UPFARMER && s->singles[1]->low > downPl->opps->end)
				||(game->known_others_poker &&  pl->id==DOWNFARMER && s->singles[1]->low > upPl->opps->end)
				)
			{
				*cur =*s->singles[0];
				return 1;
			}
		}
		//two single!
		if(s->pairs_num == 2)
		{ 
			*cur=*s->pairs[0];
			if(s->pairs[1]->low==P2)
			{
				return 1;
			}
			int  res=1;
			for(int i=max(pl->opps->begin,s->pairs[1]->low + 1);i<=P2; i++)
			{
				if(pl->opps->hands[i] >=2)
					res =0;	
			}
			if(game->known_others_poker)
			{
				COMBO_OF_POKERS p=*s->pairs[1];
				p.low--;
				if(is_combo_biggest(game, pl->opps, 
					&p, 2,2,2))
				{
					res =1;
				}
			}           	
			return res;
		}		

		if(s->three_one_num == 2)
		{ 
			*cur=*s->three_one[0];
			if(game->known_others_poker)
			{
				COMBO_OF_POKERS p=*s->three_one[1];
				p.low--;
				if(is_combo_biggest(game, pl->opps, 
					&p, 4,4,4))
				{
					return 1;
				}
			}
			else
			{
				return 1;
			}
			return 0;
		}		

		if(s->three_two_num == 2)
		{ 
			*cur=*s->three_two[0];
			if(game->known_others_poker)
			{
				COMBO_OF_POKERS p=*s->three_two[1];
				p.low--;
				if(is_combo_biggest(game, pl->opps, 
					&p, 5,5,5))
				{
					return 1;
				}
			} 
			else
			{
				return 1;
			}
			return 0;
		}

		if(s->series_num == 2)
		{ 
			*cur=*s->series[0];
			if(game->known_others_poker)
			{
				COMBO_OF_POKERS p=*s->series[1];
				p.low--;
				if(is_combo_biggest(game, pl->opps, 
					&p, 13,13,13))
				{
					return 1;
				}
			}           	
			return 0;
		}

	}
	return 0;
}

//最简单必胜情况，只有一手小牌，其他牌都是最大
//
COMBO_OF_POKERS* check_for_win_now(GAME* game,COMBO_OF_POKERS* pre)
{
	COMBOS_SUMMARY * summary = game->players[CUR_PLAYER]->summary;
	PLAYER *pl=game->players[CUR_PLAYER];
	if(summary->real_total_num==1) //just one combo
	{
		COMBO_OF_POKERS * ret = find_combo(summary,pre);
		return ret;
	}
	else if(summary->combo_total_num <= 1)
	{
		if(summary->real_total_num==2 && summary->combo_total_num <= 1) //just one combo
		{
			if(summary->bomb_num==1 && is_combo_biggest_sure(game,summary->bomb[0]) ) //
			{
				int i=0;
				for(; i<summary->bomb_num; i++)
					if(check_combo_a_Big_than_b(summary->bomb[i],pre))
						return summary->bomb[i];
			}
		}   

		//farmer do not make bigger for another farmer player
		if(pl->id == DOWNFARMER)
		{
			if(game->players[game->pre_playernum]->id == UPFARMER
				&& game->players[game->pre_playernum]->summary->real_total_num == 1 )
			{
				//check bomb?
				return NULL;
			}
		}
		else if (0 && pl->id ==UPFARMER && (game->players[game->pre_playernum]->id == DOWNFARMER )
			&& game->players[game->pre_playernum]->summary->real_total_num == 1 )
		{
			//todo check
			//if(is_combo_biggest_for_player(game->players[DOWN_PLAYER],game->pre))
			return NULL;
		}
		if(NOT_BOMB(pre))
		{
			int number=0,num=0;
			COMBO_OF_POKERS ** ret = find_all_combo_bigger_than_combo_a(summary, pre, &number);

			if (number>=1)
			{
				num=number;
				{
					while (number>1)
					{
						if(ret[number-1]->low > ret[number-2]->low
							&&  is_combo_biggest_sure(game, ret[number-2]) )
							number--;
						else
							break;
					}
				}
				if(is_combo_biggest_sure(game, ret[number-1]))
					return *(ret+number-1);
				//  else
				//   return NULL;
			}
			// else
			//   return NULL;
		}
		if(summary->bomb_num>0 && !opp_hasbomb(game) ) //
		{
			int i=0;
			for(; i<summary->bomb_num; i++)
				if(check_combo_a_Big_than_b(summary->bomb[i],pre))
				{
					return summary->bomb[i];
				}
		}
		//else		
	}
	return NULL;
}


COMBO_OF_POKERS* get_combo_in_a_win_suit(GAME* game)
{
	PLAYER* player =game->players[CUR_PLAYER];
	COMBOS_SUMMARY* s = player->summary ;
	int num =s->combo_total_num;    
	if (num<=1+s->bomb_num  )//for bomb
	{
		int has_big=0;
		if(player->id==LORD)
		{
			for (int k=0; k<s->biggest_num; k++)
			{
				if (s->biggest[k]->type==SINGLE_SERIES ||
					s->biggest[k]->type==PAIRS_SERIES ||
					s->biggest[k]->type>=3311)
					has_big=1;					 
			}
		}
		if( !opp_hasbomb(game) || 
			player->id !=LORD 
			|| (s->bomb_num ==0 && (s->ctrl.single <10 || has_big==1))
			||(s->bomb_num +1 -num ) >= get_opp_bomb_number(game)
			|| ( (s->bomb_num + 2 -num ) >= get_opp_bomb_number(game)
			&& s->bomb_num >0 && is_combo_biggest_sure(game,
			s->bomb[s->bomb_num-1] )
			)			
			)
		{      	   	
			for (int k=0; k<s->biggest_num; k++)
			{
				if (s->biggest[k]->type!=BOMB && s->biggest[k]->type!=ROCKET)//先出冲锋套
				{
					for (int i=0; i<s->not_biggest_num; i++)
					{
						if(s->not_biggest[i]->type == s->biggest[k]->type
							&&(s->not_biggest[i]->len == s->biggest[k]->len ||( s->biggest[k]->type!=SINGLE_SERIES 
							&& s->biggest[k]->type!=PAIRS_SERIES && s->biggest[k]->type!=THREE_SERIES ) ) )
						{
							COMBO_OF_POKERS t= *s->biggest[k];
							t.low--;
							if(is_combo_biggest_sure_without_bomb(game,&t)) //todo check this?
							{  
								if(s->not_biggest_num>=1+s->bomb_num)
								{
									COMBO_OF_POKERS *c=&t;
									if(game->known_others_poker)
									{
										//查看农民是否刚好能跑
										if(game->players[CUR_PLAYER]->id!=DOWNFARMER &&
											game->players[DOWN_PLAYER]->h->total == get_combo_number(s->not_biggest[i])
											&& find_a_bigger_combo_in_hands(game->players[DOWN_PLAYER]->h,  c, s->not_biggest[i])
											&&NOT_BOMB(c))
											continue;
										if(game->players[CUR_PLAYER]->id!=UPFARMER &&
											game->players[UP_PLAYER]->h->total == get_combo_number(s->not_biggest[i])
											&& find_a_bigger_combo_in_hands(game->players[UP_PLAYER]->h,  c, s->not_biggest[i])
											&&NOT_BOMB(c))
											continue;
									}
									return s->not_biggest[i];
								}
							}					
						}
					}
				}
			}
			for (int k=0; k<s->biggest_num; k++)
			{
				if (s->biggest[k]->type!=BOMB && s->biggest[k]->type!=ROCKET
					&& is_combo_biggest_sure_without_bomb(game, s->biggest[k]))//先出冲锋套
					return s->biggest[k];					 
			}
			for (int k=0; k<s->biggest_num; k++)
			{
				if (s->biggest[k]->type!=BOMB && s->biggest[k]->type!=ROCKET)//先出冲锋套
					return s->biggest[k];    
			}

			if ( s->not_biggest_num>0 )
			{    //find the small comb
				COMBO_OF_POKERS * c1=NULL,t,*c=&t;
				//check c
				for(int i=0; i<s->not_biggest_num; i++)
				{
					if(game->known_others_poker)
					{
						//查看农民是否刚好能跑
						if(game->players[CUR_PLAYER]->id!=DOWNFARMER &&
							game->players[DOWN_PLAYER]->h->total == get_combo_number(s->not_biggest[i])
							&& find_a_bigger_combo_in_hands(game->players[DOWN_PLAYER]->h,  c, s->not_biggest[i])
							&&NOT_BOMB(c))
							continue;
						if(game->players[CUR_PLAYER]->id!=UPFARMER &&
							game->players[UP_PLAYER]->h->total == get_combo_number(s->not_biggest[i])
							&& find_a_bigger_combo_in_hands(game->players[UP_PLAYER]->h,  c, s->not_biggest[i])
							&&NOT_BOMB(c))
							continue;
					}else if((s->not_biggest[i]->type==SINGLE
						&& game->players[CUR_PLAYER]->id!=UPFARMER && player->oppUp_num ==1)
						||
						(s->not_biggest[i]->type==PAIRS
						&& game->players[CUR_PLAYER]->id!=UPFARMER && player->oppUp_num ==2)   
						||(s->not_biggest[i]->type==SINGLE
						&& game->players[CUR_PLAYER]->id!=DOWNFARMER&& player->oppDown_num==1)
						||
						(s->not_biggest[i]->type==PAIRS
						&& game->players[CUR_PLAYER]->id!=DOWNFARMER && player->oppDown_num ==2)
						)
					{ 
						continue;
					}
					if(c1==NULL)
						c1=s->not_biggest[i];
					if(s->not_biggest[i]->low < c1->low)
						c1=s->not_biggest[i];
				}        	               
				if(c1==NULL)
				{
					c1=s->not_biggest[0];
				}        	               
				return c1;	 
			}

			if ( s->bomb_num>0 )
			{
				return s->bomb[0];
			}
			return NULL;
		}
		else
		{
			if(s->not_biggest_num >0)
			{    //find the small comb
				COMBO_OF_POKERS * c1=NULL,t,*c=&t;
				//check c
				for(int i=0; i<s->not_biggest_num; i++)
				{
					if(game->known_others_poker)
					{
						//查看农民是否刚好能跑
						if(game->players[CUR_PLAYER]->id!=DOWNFARMER &&
							game->players[DOWN_PLAYER]->h->total == get_combo_number(s->not_biggest[i])
							&& find_a_bigger_combo_in_hands(game->players[DOWN_PLAYER]->h,  c, s->not_biggest[i])
							&&NOT_BOMB(c))
							continue;
						if(game->players[CUR_PLAYER]->id!=UPFARMER &&
							game->players[UP_PLAYER]->h->total == get_combo_number(s->not_biggest[i])
							&& find_a_bigger_combo_in_hands(game->players[UP_PLAYER]->h,  c, s->not_biggest[i])
							&&NOT_BOMB(c))
							continue;
					}
					for (int k=0; k<s->biggest_num; k++)
					{
						if (s->biggest[k]->type!=BOMB && s->biggest[k]->type!=ROCKET
							&& s->biggest[k]->type == s->not_biggest[i]->type
							&& (( s->biggest[k]->type!=SINGLE_SERIES 
							&& s->biggest[k]->type!=PAIRS_SERIES && s->biggest[k]->type!=THREE_SERIES ) ||
							s->biggest[k]->len == s->not_biggest[i]->len)
							)
						{
							if(c1==NULL || 
								s->not_biggest[i]->len > c1->len)
								c1=s->not_biggest[i];                                          	
						}
					}				
				}        	               
				if(c1==NULL)
				{    //find the small comb
					for(int i=0; i<s->not_biggest_num; i++)
					{
						if(game->known_others_poker)
						{
							//查看农民是否刚好能跑
							if(game->players[CUR_PLAYER]->id!=DOWNFARMER &&
								game->players[DOWN_PLAYER]->h->total == get_combo_number(s->not_biggest[i])
								&& find_a_bigger_combo_in_hands(game->players[DOWN_PLAYER]->h,  c, s->not_biggest[i])
								&&NOT_BOMB(c))
								continue;
							if(game->players[CUR_PLAYER]->id!=UPFARMER &&
								game->players[UP_PLAYER]->h->total == get_combo_number(s->not_biggest[i])
								&& find_a_bigger_combo_in_hands(game->players[UP_PLAYER]->h,  c, s->not_biggest[i])
								&&NOT_BOMB(c))
								continue;
						}
						if(c1==NULL || s->not_biggest[i]->len > c1->len)
							c1=s->not_biggest[i];                                          	
					}				
				}        	               
				if(c1==NULL)
				{
					c1=s->not_biggest[0];
				}
				return c1;	 
			}

			for (int k=0; k<s->biggest_num; k++)
			{
				if (s->biggest[k]->type!=BOMB && s->biggest[k]->type!=ROCKET
					&& is_combo_biggest_sure_without_bomb(game, s->biggest[k]))//先出冲锋套
					return s->biggest[k];					 
			}
			for (int k=0; k<s->biggest_num; k++)
			{
				if (s->biggest[k]->type!=BOMB && s->biggest[k]->type!=ROCKET)//先出冲锋套
					return s->biggest[k];    
			}				

			if ( s->bomb_num>0 )
			{
				return s->bomb[0];
			}
			return NULL;		    		

		}			
	}
	else
		return NULL;
}
// select a combo form player
COMBO_OF_POKERS* farm_select_combo_in_suit(GAME* game)
{
	PLAYER* player =game->players[CUR_PLAYER];
	COMBOS_SUMMARY* s = player->summary ;

	if(s->combo_total_num ==2)
	{
		int number=0;
		for (int k=0; k<s->not_biggest_num; k++)
		{
			if (s->not_biggest[k]->type==SINGLE ||s->not_biggest[k]->type==PAIRS)
			{ 
				number++;
			}
		}
		if(number==2 && s->bomb_num==0 && s->ctrl.single < 5 )
		{
			for (int k=0; k<s->biggest_num; k++)
			{
				return s->biggest[k];
			}	     
		}
	}
	if ( !is_player_ready_win(game,player) ) //不合手
	{
		for (int k=0; k<s->biggest_num; k++)
		{
			if (s->biggest[k]->type==SINGLE_SERIES  && s->biggest[k]->low<=P6)
			{ 
				return s->biggest[k];
			}
		} 
		//出所有套牌中标签最小的一套。（两套牌标签相同时，出张数多的一套）
		return  find_smallest_in_combos(player->combos,20,player,0);
	}
	///*
	// rocket or biggest bomb and other..
	// for chuntian
	if(s->combo_total_num == 1 && s->bomb_num == s->biggest_num &&s->bomb_num>0
		&& player->oppDown_num >=17 && player->oppUp_num >=17 )
	{
		//bomb is biggest
		if(s->bomb[0]->type ==ROCKET)
			return s->bomb[0];
		else
		{
			COMBO_OF_POKERS tmp;
			if(! getBigBomb(player->opps, &tmp, s->bomb[0]))
				return s->bomb[0];
		}
	}
	//*/
	COMBO_OF_POKERS * c = get_combo_in_a_win_suit(game);
	if(c!=NULL)
	{
		return c;
	}
	for (int k=0; k<s->biggest_num; k++)
	{
		if (s->biggest[k]->type==PAIRS_SERIES && s->biggest[k]->low<P9)
		{ 
			for (int i=0; i<s->not_biggest_num; i++)
			{
				if (s->not_biggest[i]->type==PAIRS_SERIES)
				{ 
					return s->not_biggest[i];
				}
			}            	
			return s->biggest[k];
		}
		if (s->biggest[k]->type==SINGLE_SERIES )
		{ 
			for (int i=0; i<s->not_biggest_num; i++)
			{
				if (s->not_biggest[i]->type==SINGLE_SERIES && s->not_biggest[i]->low<=s->biggest[k]->type)
				{ 
					return s->not_biggest[i];
				}
			}            	
			return s->biggest[k];
		}
		if (s->biggest[k]->type>=3311 )
		{          	            	
			return s->biggest[k];
		}			
	}

	//寻找牌组中，套数最多的一类牌
	int max=0,idx=0;
	if (s->threeseries_num+s->threeseries_one_num+s->threeseries_two_num >max) {
		max = s->threeseries_num+s->threeseries_one_num+s->threeseries_two_num ;
		idx=1;
	}
	if (s->pairs_series_num>max) {
		max=s->pairs_series_num;
		idx=2;
	}
	for (int i=0; i<10; i++)
		if (s->series_detail[i]>max) {
			max=s->series_detail[i];
			idx=6;
		}

		if (s->three_one_num+s->three_two_num+s->three_num>max) {
			max=s->three_one_num+s->three_two_num+s->three_num;
			idx=3;
		}
		if (s->pairs_num>max) {
			max=s->pairs_num;
			idx=4;
		}
		if (s->singles_num>max) {
			max=s->singles_num;
			idx=5;
		}

		if ( max > 1 ) //todo: check for controls..
		{
			if (idx==3) //to be refine
			{
				if (s->three_one_num>0) {
					return s->three_one[0];
				}
				else			if (s->three_two_num>0) {
					return s->three_two[0];
				}
				else
					return s->three[0];
			}
			else if (idx==1) {
				if (s->threeseries_one_num>0) {
					return s->threeseries_one[0];
				}
				else			if (s->threeseries_two_num>0) {
					return s->threeseries_two[0];
				}
				else
					return s->threeseries[0];
			}
			else if (idx==2) {
				return s->pairs_series[0];
			}
			else if (idx==4) {
				return    s->pairs[0];
			}
			else if (idx==5) {
				return    s->singles[0];
			}
			else if (idx==6) {
				return s->series[0]; //todo : fixme..
			}
		}
		else
		{
			//合手了，但是小牌大于一套，出标签较小那套
			return  find_smallest_in_combos(player->combos,20,player,0);
		}
		return NULL;
}


// select a combo form player
COMBO_OF_POKERS* lord_select_combo_in_suit(GAME* game)
{
	PLAYER* player =game->players[CUR_PLAYER];
	COMBOS_SUMMARY* s = player->summary ;

	if(s->combo_total_num ==2)
	{
		int number=0;
		for (int k=0; k<s->not_biggest_num; k++)
		{
			if (s->not_biggest[k]->type==SINGLE ||s->not_biggest[k]->type==PAIRS)
			{ 
				number++;
			}
		}
		if(number==2 && s->bomb_num==0 && s->ctrl.single < 5 )
		{
			for (int k=0; k<s->biggest_num; k++)
			{
				return s->biggest[k];
			}	     
		}
	}
	if ( !is_player_ready_win(game,player) ) //不合手
	{
		for (int k=0; k<s->biggest_num; k++)
		{
			if (s->biggest[k]->type==SINGLE_SERIES  && s->biggest[k]->low<=P6)
			{ 
				return s->biggest[k];
			}
			if (s->biggest[k]->type==PAIRS_SERIES && s->biggest[k]->low<=P8)
			{ 
				return s->biggest[k];
			}
			if (s->biggest[k]->type>=3311 && s->biggest[k]->low<=P7)
			{ 
				return s->biggest[k];
			}		
		}    
		//出所有套牌中标签最小的一套。（两套牌标签相同时，出张数多的一套）
		return  find_smallest_in_combos(player->combos,20,player,0);
	}
	///*
	// rocket or biggest bomb and other..
	// for chuntian
	if(s->combo_total_num == 1 && s->bomb_num == s->biggest_num &&s->bomb_num>0
		&& player->oppDown_num >=17 && player->oppUp_num >=17 )
	{
		//bomb is biggest
		if(s->bomb[0]->type ==ROCKET)
			return s->bomb[0];
		else
		{
			COMBO_OF_POKERS tmp;
			if(! getBigBomb(player->opps, &tmp, s->bomb[0]))
				return s->bomb[0];
		}
	}
	//*/
	COMBO_OF_POKERS * c = get_combo_in_a_win_suit(game);
	if(c!=NULL)
	{
		return c;
	}
	for (int k=0; k<s->biggest_num; k++)
	{
		if (s->biggest[k]->type==PAIRS_SERIES && s->biggest[k]->low<P9)
		{ 
			for (int i=0; i<s->not_biggest_num; i++)
			{
				if (s->not_biggest[i]->type==PAIRS_SERIES)
				{ 
					return s->not_biggest[i];
				}
			}            	
			return s->biggest[k];
		}
		if (s->biggest[k]->type==SINGLE_SERIES )
		{ 
			for (int i=0; i<s->not_biggest_num; i++)
			{
				if (s->not_biggest[i]->type==SINGLE_SERIES && s->not_biggest[i]->low<=s->biggest[k]->type)
				{ 
					return s->not_biggest[i];
				}
			}            	
			return s->biggest[k];
		}
		if (s->biggest[k]->type>=3311 )
		{          	            	
			return s->biggest[k];
		}			
	}

	//寻找牌组中，套数最多的一类牌
	int max=0,idx=0;
	if (s->threeseries_num+s->threeseries_one_num+s->threeseries_two_num >max) {
		max = s->threeseries_num+s->threeseries_one_num+s->threeseries_two_num ;
		idx=1;
	}
	if (s->pairs_series_num>max) {
		max=s->pairs_series_num;
		idx=2;
	}
	for (int i=0; i<10; i++)
		if (s->series_detail[i]>max) {
			max=s->series_detail[i];
			idx=6;
		}

		if (s->three_one_num+s->three_two_num+s->three_num>max) {
			max=s->three_one_num+s->three_two_num+s->three_num;
			idx=3;
		}
		if (s->pairs_num>max) {
			max=s->pairs_num;
			idx=4;
		}
		if (s->singles_num>max) {
			max=s->singles_num;
			idx=5;
		}

		if ( max > 1 ) //todo: check for controls..
		{
			if (idx==3) //to be refine
			{
				if (s->three_one_num>0) {
					return s->three_one[0];
				}
				else			if (s->three_two_num>0) {
					return s->three_two[0];
				}
				else
					return s->three[0];
			}
			else if (idx==1) {
				if (s->threeseries_one_num>0) {
					return s->threeseries_one[0];
				}
				else			if (s->threeseries_two_num>0) {
					return s->threeseries_two[0];
				}
				else
					return s->threeseries[0];
			}
			else if (idx==2) {
				return s->pairs_series[0];
			}
			else if (idx==4) {
				return    s->pairs[0];
			}
			else if (idx==5) {
				return    s->singles[0];
			}
			else if (idx==6) {
				return s->series[0]; //todo : fixme..
			}
		}
		else
		{
			//合手了，但是小牌大于一套，出标签较小那套
			return  find_smallest_in_combos(player->combos,20,player,0);
		}
		return NULL;
}

//check if lord win for current status
//地主强合手
//return 1: lord win
//return 0: no simple win for lord.
COMBO_OF_POKERS* is_lord_play_first_win(GAME * game)
{
	FUNC_NAME;
	PLAYER * player = game->players[CUR_PLAYER];
	COMBOS_SUMMARY *s =player->summary;
	COMBO_OF_POKERS* c= &game->c[0];
	if(game->known_others_poker)
	{

		//春天可能性检查
		//如果春天则春天
		if(s->combo_total_num == 1 && s->bomb_num == s->biggest_num &&s->bomb_num>0
			&& player->oppDown_num >=17 && player->oppUp_num >=17 )
		{
			//bomb is biggest
			if(s->bomb[0]->type ==ROCKET)
				return s->bomb[0];
			else
			{
				COMBO_OF_POKERS tmp;
				if(is_combo_biggest_sure(game, s->bomb[0]))
					return s->bomb[0];
			}
		}

		int num =s->combo_total_num;

		COMBO_OF_POKERS * c = get_combo_in_a_win_suit(game);
		if(c!=NULL)
		{
			return c;
		}

		if (num<=1+s->bomb_num)//for bomb
		{
			for (int k=0; k<s->biggest_num; k++)
			{
				if (s->biggest[k]->type!=BOMB && s->biggest[k]->type!=ROCKET)//先出冲锋套
				{
					return s->biggest[k];
				}
			}

			if ( s->not_biggest_num>0 )
			{
				//play the one
				for(int i=0; i<s->not_biggest_num; i++)
				{
					//查看农民是否刚好能跑
					if(game->players[DOWN_PLAYER]->h->total == get_combo_number(s->not_biggest[i])
						&& find_a_bigger_combo_in_hands(game->players[DOWN_PLAYER]->h,  c, s->not_biggest[i]))
						continue;
					if(game->players[UP_PLAYER]->h->total == get_combo_number(s->not_biggest[i])
						&& find_a_bigger_combo_in_hands(game->players[UP_PLAYER]->h,  c, s->not_biggest[i]))
						continue;
					return s->not_biggest[i];
				}
				if(s->not_biggest_num>=2)//lord would lose
				{
					return s->not_biggest[0];
				}
			}

			if ( s->bomb_num>0 )
			{
				return s->bomb[0];
			}
			return NULL;
		}
		return 0;
		//todo: 如果有小牌我们肯定能收回的
		for(int i=0; i<s->not_biggest_num; i++)
		{
			//nd_biggest_in_combos(COMBO_OF_POKERS * combos, int total)
		}
	}
	else
	{
		if(s->combo_total_num == 1 && s->bomb_num == s->biggest_num &&s->bomb_num>0
			&& player->oppDown_num >=17 && player->oppUp_num >=17 )
		{
			if(s->bomb[0]->type ==ROCKET)
				return s->bomb[0];
			else
			{
				COMBO_OF_POKERS tmp;
				if(is_combo_biggest_sure(game, s->bomb[0]))
					return s->bomb[0];
			}
		}
		COMBO_OF_POKERS * c = get_combo_in_a_win_suit(game);
		if(c!=NULL)
		{
			int good=1;
			if ( (player->oppDown_num == 1 || player->oppUp_num == 1 ) && c->type ==SINGLE 
				&& c->low < player->opps->end )
				good=0;
			if ( (player->oppDown_num == 2 || player->oppUp_num == 2 ) && c->type ==PAIRS
				&& !is_combo_biggest_sure(game,c))
			{
				good=0;
			}
			if(good){
				return c;
			}
		} 
	}
	return 0;
}

void lord_play_first(GAME * game ,COMBO_OF_POKERS* c)//select c from player
{
	PLAYER * player = game->players[CUR_PLAYER];
	COMBOS_SUMMARY *s =player->summary;
	COMBO_OF_POKERS* t =NULL ;
	FUNC_NAME;

	if(NULL!=(t=is_lord_play_first_win(game)))
	{
		PRINTF(LEVEL,"lord quick win found!\n");
		*c=*t;
		return;
	}

	if (  player->oppDown_num ==1 // game->players[(CUR_PLAYER+1)%3]->h->total == 1  //last play farmer has 1 only
		|| player->oppUp_num == 1 ) // the down player of last play farmer has 1 only
		// ||  game->players[(CUR_PLAYER+2)%3]->h->total == 1)  // the Up player of last play farmer has 1 only  and current is a single.
		// )

	{   //农民报单
		for (int k=0; k<s->biggest_num; k++)
		{
			if (s->biggest[k]->type==PAIRS_SERIES && s->biggest[k]->low<P9)
			{ 
				for (int i=0; i<s->not_biggest_num; i++)
				{
					if (s->not_biggest[i]->type==PAIRS_SERIES)
					{ 
						*c=*s->not_biggest[i];
						return	;
					}
				}            	
				*c=*s->biggest[k];
				return;
			}
			if (s->biggest[k]->type==SINGLE_SERIES )
			{ 
				for (int i=0; i<s->not_biggest_num; i++)
				{
					if (s->not_biggest[i]->type==SINGLE_SERIES && s->not_biggest[i]->low<=s->biggest[i]->type)
					{ 
						*c=*s->not_biggest[i];   return;
					}
				}            	
				*c= *s->biggest[k];return;
			}
			if (s->biggest[k]->type>=3311 )
			{          	            	
				*c=*s->biggest[k];return;
			}			
		}

		//如果手中有不含控制牌且不为单牌的套牌，则打出其中标签最小的一套
		t = find_smallest_in_combos(player->combos,20,player,1);

		if (t ==NULL)
		{
			//如果手中有两套以上不为绝对控制的单牌，打出第二小的
			if ( s->singles_num >=2 && s->singles[1]->low<player->opps->end)  //   || s->singles_num == s->real_total_num )
				t = s->singles[1];
			else
			{
				/*if(s->not_biggest_num>0)//打出含有控制牌的非冲锋套
				t= s->not_biggest[0];
				else*/
				if ( s->not_biggest_num >0 )
				{
					for(int k=0; k<s->not_biggest_num; k++)
					{
						if(s->not_biggest[k]->type!=SINGLE)
						{
							t=s->not_biggest[k];
							break;
						}
					}
					if(t==NULL) {
						goto __CHONGFENG;
					}
				}
				else {
__CHONGFENG:
					if (s->biggest_num>0 && s->biggest_num>s->bomb_num )//打出冲锋套
					{

						for(int k=0; k<s->biggest_num; k++)
						{
							if(s->biggest[k]->type!=BOMB&&s->biggest[k]->type!=ROCKET)
							{
								t=s->biggest[k];
								break;
							}
						}
					}
					else if( s->bomb_num >0 )//打出炸弹
						t= s->bomb[0];
					else if(s->singles_num>0) //打出剩余的一张单牌
						t=s->singles[0];
					else { //what happens...
						PRINTF_ALWAYS("strange error\n");
					}
				}
			}
		}
	}
	else
		t = lord_select_combo_in_suit(game);


	if (t==NULL)
	{
		PRINTF_ALWAYS("ERR: line %d lord play first: stupid error\n",__LINE__);
		return;
	}
	memmove(c,t,sizeof(COMBO_OF_POKERS));
	remove_combo_in_suit(s,t);
	t->type=NOTHING;
}

int Lord_Chaipai(GAME * game,COMBO_OF_POKERS* cur, COMBO_OF_POKERS* pre,int good_bomb)
{
	FUNC_NAME
		PLAYER * player =game->players[CUR_PLAYER];
	COMBO_OF_POKERS tmp1;
	COMBO_OF_POKERS* c= &tmp1;
	COMBO_OF_POKERS tmp,*p =&tmp;
	*p=*pre;
	int first_time =1;
	int mustPlay = 0;
	// COMBOS_SUMMARY *s =player->summary;

	//to be optimized by using new search methods..
	if(pre->type==SINGLE && player->summary->ctrl.single>=30)
	{
		good_bomb=1;
	}    
	while ( find_a_bigger_combo_in_hands(player->h,c,p)) //could not handle corner case for  BOMB in two combos.
	{
		if(!NOT_BOMB(c))
			break;
		if(c->type>=THREE_ONE)
		{
			c->control=2;//tmporary use
		}
		else 
			c->control=0;//tmporary use	

		PLAYER tmp_player;
		POKERS h,opps;
		COMBO_OF_POKERS combo[20]= {20*0};
		COMBOS_SUMMARY sum= {0},*s;
		tmp_player.combos=&combo[0];
		tmp_player.h = &h;
		tmp_player.opps = &opps;
		tmp_player.summary =s = &sum;
		tmp_player.lower_control_poker = player->lower_control_poker;
		tmp_player.oppDown_num = player->oppDown_num;
		tmp_player.oppUp_num = player->oppUp_num;
		memmove(&h,player->h,sizeof(POKERS));

		memmove(&opps,player->opps,sizeof(POKERS));
		remove_combo_poker(tmp_player.h,c,NULL);

		search_combos_in_suit(game,tmp_player.h,tmp_player.opps,&tmp_player);
		if(good_bomb)//不拆炸弹
		{


			if(tmp_player.summary->bomb_num< player->summary->bomb_num)
			{		    	     
				*p=*c;
				continue;
			}
		}
		int max_poker_in_c = has_control_poker(c, player->lower_control_poker);
		{
			if(c->type==SINGLE ||c->type==PAIRS)
			{
				if(c->low>=player->lower_control_poker)
					assume_remove_controls_in_opps(&opps, c,max_poker_in_c);
				int ctrl_num =	CONTROL_POKER_NUM;

				//update the lowest control poker and update the summary
				POKERS all;
				add_poker(tmp_player.h,&opps,&all);
				if(all.total <=20 )
					ctrl_num = 4;
				else if(all.total <=10 )
					ctrl_num = 2;		
				tmp_player.lower_control_poker = get_lowest_controls(&all,ctrl_num);							
				tmp_player.summary->ctrl.single = calc_controls( tmp_player.h,tmp_player.opps, ctrl_num );
			}		   
			if(tmp_player.lower_control_poker<player->lower_control_poker
				&& tmp_player.summary->ctrl.single > player->summary->ctrl.single )
			{//shit
				PRINTF(LEVEL,"pull back single control\n");
				tmp_player.summary->ctrl.single = player->summary->ctrl.single-1;
			}

			if ( CONTROL_SUB_COMBO_NUM_IS_GOOD(&tmp_player)
				||  CONTROL_SUB_COMBO_NUM(&tmp_player) >= CONTROL_SUB_COMBO_NUM(player)
				|| CONTROL_NUM(&tmp_player)>=30
				|| ( CONTROL_SUB_COMBO_NUM(&tmp_player)+10 >= CONTROL_SUB_COMBO_NUM(player)
				&& ( is_combo_biggest( game,player->opps,c,player->oppDown_num,player->oppUp_num,player->lower_control_poker)
				|| (c->type!=SINGLE && c->type!=PAIRS)
				)
				)
				|| mustPlay )
			{
				goto SAVE_CUR_RESULT;
			}
			else
			{
				*p=*c;
				continue;
			}

SAVE_CUR_RESULT:
			//update player in hands;
			PRINTF(LEVEL,"!DIZHU chaipai, control-combo %d, combo_num %d old( %d,%d)\n"
				,CONTROL_SUB_COMBO_NUM(&tmp_player),tmp_player.summary->combo_total_num,
				CONTROL_SUB_COMBO_NUM(player), player->summary->combo_total_num
				);
			if ( first_time
				||(CONTROL_SUB_COMBO_NUM(&tmp_player) > CONTROL_SUB_COMBO_NUM(player) )
				|| ( (CONTROL_SUB_COMBO_NUM(&tmp_player) == CONTROL_SUB_COMBO_NUM(player) )
				&& tmp_player.summary->combo_total_num < player->summary->combo_total_num)
				|| ( (CONTROL_SUB_COMBO_NUM(&tmp_player) == CONTROL_SUB_COMBO_NUM(player) )
				&& tmp_player.summary->combo_total_num == player->summary->combo_total_num
				&& tmp_player.summary->combo_typenum < player->summary->combo_typenum )
				||  ( (CONTROL_SUB_COMBO_NUM(&tmp_player) == CONTROL_SUB_COMBO_NUM(player) )
				&& tmp_player.summary->combo_total_num == player->summary->combo_total_num
				&& tmp_player.summary->combo_typenum == player->summary->combo_typenum
				&& tmp_player.summary->singles_num < player->summary->singles_num )
				/* ||   ( (CONTROL_SUB_COMBO_NUM(&tmp_player) == CONTROL_SUB_COMBO_NUM(player) )
				&& tmp_player.summary->combo_total_num == player->summary->combo_total_num
				&& tmp_player.summary->combo_typenum == player->summary->combo_typenum
				&& tmp_player.summary->singles_num == player->summary->singles_num )*/
				)
			{
				*player->summary = *tmp_player.summary;
				first_time = 0;
				*cur=*c;
			}
			*p = *c;
		}
	}
	cur->control = 0;
	return first_time;
}


int lord_play_2(GAME * game,COMBO_OF_POKERS* cur, COMBO_OF_POKERS* pre)//select a combo from player and save to cur.
{
	FUNC_NAME
		PLAYER * player  = game->players[CUR_PLAYER];
	int mustPlay = 0;
	int pass;
	COMBO_OF_POKERS* res;
	if( (res= check_for_win_now(game,pre))!=NULL ) //win immediately, play the biggest or bomb
	{
		remove_combo_in_suit(player->summary,res);
		memmove(cur,res,sizeof(COMBO_OF_POKERS));
		res->type = NOTHING;
		return 0;
	}

	COMBOS_SUMMARY *s =player->summary;
	COMBO_OF_POKERS *c;

	{
		if ( (player->oppDown_num==1 && game->pre_playernum == DOWN_PLAYER) // game->players[game->pre_playernum]->h->total == 1  //last play farmer has 1 only
			||(player->oppUp_num==1 && game->pre_playernum == UP_PLAYER)
			||(player->oppDown_num==1 && game->pre_playernum == UP_PLAYER && pre->type == SINGLE)
			||(player->oppUp_num==1 && game->pre_playernum == DOWN_PLAYER)
			//|| (game->players[(game->pre_playernum+1)%3]->h->total == 1 ) // the down player of last play farmer has 1 only
			//   || ( pre->type == SINGLE && game->players[(game->pre_playernum+2)%3]->h->total == 1)
			// the Up player of last play farmer has 1 only  and current is a single.
			)
		{   // lord must play this turn if he can
			mustPlay = 1;
			if (pre->type == SINGLE )
			{
				if (s->singles_num >= 2 )
				{
					/*todo: optimize if all farmer has single in hand only*/
					/*todo: research hands?*/

					//在当前组合中，如果有两套或以上单牌，则从第二小的开始，找出比pre大的单牌cur，
					for (int k=1; k<s->singles_num; k++)
					{
						if (s->singles[k]->low>pre->low) {
							memmove(cur,s->singles[k],sizeof(COMBO_OF_POKERS));
							s->singles[k]->type = NOTHING;
							remove_combo_in_suit(s,cur);
							return 0;
						}
					}
					goto __SEARCH_BOMB_FOR_SINGLE;
					//return 1;
				}
				else //play the biggest
				{
					if(game->use_best_for_undefined_case)
						return get_the_best_hand( game, cur, 0);
					//如果仅剩的一张单牌绝对大，则打出
					if (s->singles_num == 1
						&& s->singles[0]->low>pre->low
						&& s->singles[0]->low >= player->opps->end )
					{
						memmove(cur,s->singles[0],sizeof(COMBO_OF_POKERS));
						s->singles[0]->type = NOTHING;
						remove_combo_in_suit(s,cur);
						return 0;
					}
__SEARCH_BOMB_FOR_SINGLE:
					//如果有炸弹，且出完炸弹合手，则出炸弹
					if (s->bomb_num >0 &&
						CONTROL_SUB_COMBO_NUM_IS_GOOD(player)   
						&& get_2nd_min_singles(player->summary)>=player->opps->end ) //有炸弹，且合手
					{
						memmove(cur,s->bomb[0],sizeof(COMBO_OF_POKERS));
						s->bomb[0]->type = NOTHING;
						remove_combo_in_suit(s,cur);
						return 0;
					}
					//出最大的一张牌（可以从套里拆出）
					for ( int k= max(player->opps->end,pre->low+1) ; k<= player->h->end ; k ++)
					{
						if ( player->h->hands[k]>0 )
						{
							cur->type=SINGLE;
							cur->low = k;
							cur->len = 1;
							return 0;
						}
					}

					for(int k= player->h->end ; k>pre->low;k--)			
					{
						if ( player->h->hands[k]>0 )
						{
							cur->type=SINGLE;
							cur->low = k;
							cur->len = 1;
							//research needed.
							player->need_research = 1;
							return 0;
						}
					}
				}
				return 1;
			}
		}
	}

	//顺牌
	COMBO_OF_POKERS tmp,*p =&tmp;
	*p=*pre;
	int good_bomb=0;
	while ( (c =find_combo(s,p))!=NULL)
	{
		int max_poker_in_c;
		if ( c->type==SINGLE_SERIES || c->type==PAIRS_SERIES || c->type >=3311 ||
			((max_poker_in_c=has_control_poker(c,player->lower_control_poker))<0))
		{   //如果该牌型不包含当前大牌控制或者当前牌组是顺子,飞机，双顺，则出牌
			DBG( PRINTF(LEVEL,"ShunPai\n"));
			memmove(cur,c,sizeof(COMBO_OF_POKERS));
			return 0;

		}
		else {
			PLAYER tmp_player;
			POKERS h,opps;
			COMBO_OF_POKERS combo[20]= {20*0};
			COMBOS_SUMMARY sum= {0},*s;
			tmp_player.combos=&combo[0];
			tmp_player.h = &h;
			tmp_player.opps = &opps;
			tmp_player.summary =s = &sum;
			tmp_player.lower_control_poker = player->lower_control_poker;
			tmp_player.oppDown_num = player->oppDown_num;
			tmp_player.oppUp_num = player->oppUp_num;
			memmove(&h,player->h,sizeof(POKERS));
			memmove(&opps,player->opps,sizeof(POKERS));
			remove_combo_poker(&h,c,NULL);
			//remove one/two poker bigger than max poker in c in opps.
			search_combos_in_suit(game,tmp_player.h,tmp_player.opps,&tmp_player);

			if ( CONTROL_SUB_COMBO_NUM_IS_GOOD(&tmp_player)
				||  CONTROL_SUB_COMBO_NUM(&tmp_player) >= CONTROL_SUB_COMBO_NUM(player)
				|| CONTROL_NUM(&tmp_player)>=30
				|| ( CONTROL_SUB_COMBO_NUM(&tmp_player)+10 >= CONTROL_SUB_COMBO_NUM(player)
				&& ( is_combo_biggest( game,player->opps,c,player->oppDown_num,player->oppUp_num,player->lower_control_poker)
				|| (c->type!=SINGLE && c->type!=PAIRS) )
				&&NOT_BOMB(c)
				//|| mustPlay
				)
				|| mustPlay )
			{
				if(c->type==BOMB || c->type == ROCKET)
				{
					good_bomb=1;  
					//only for win sure!
					/*					
					if(player->oppDown_num >8 && player->oppUp_num>8 && 
					player->summary->combo_total_num>=3) //出炸弹晚一些
					{
					break;
					}
					/*
					if(pre->type==SINGLE && pre->low!=P2 && 
					((PRE_PLAYER(game)==UPFARMER && player->oppUp_num>3)//单排
					||(PRE_PLAYER(game)==DOWNFARMER && player->oppDown_num>6))
					&& player->summary->combo_total_num>=3
					)
					{
					break;
					}
					*/
					POKERS t,t1;
					int tmp_card[26],card[21];	
					if(game->hun == -1)
					{
						remove_combo_in_suit(s,c);
						remove_combo_poker(player->h, c,NULL);
					}
					else
					{
						combo_2_poker(&t,c);
						convert_poker_to_hun(&t, &t1, player->h);
						sub_poker(player->h,  &t1,player->h);
						memcpy(card,player->card, max_POKER_NUM*sizeof(int));			 
						// remove card 
						combo_to_int_array_hun(c, 
							tmp_card,player->card, game->hun);

					}
					COMBO_OF_POKERS pre=game->pre;
					int prepre=game->prepre_playernum;
					int prep=game->pre_playernum;
					game->prepre_playernum=game->pre_playernum;
					game->pre_playernum = CUR_PLAYER;
					game->pre= *c;
					game->players[CUR_PLAYER]->cur = c;
					if(         ( game->use_best_for_undefined_case) && !Check_win_quick(game, DOWN_PLAYER,0,0,0))
					{
						if(game->hun == -1)					
						{
							POKERS p;
							combo_2_poker(&p, c);
							add_poker(player->h,&p,player->h);
							search_combos_in_suit(game, player->h, player->opps, player);
						}
						else
						{
							add_poker(player->h,&t1,player->h);
							//                        search_combos_in_suit(game, player->h, player->opps, player);
							memcpy(player->card,card, max_POKER_NUM*sizeof(int));

						}
						game->pre= pre;
						game->prepre_playernum=	   prepre;
						game->pre_playernum=prep;
						break;
					}
					else {
						if(game->hun == -1)					
						{
							POKERS p;
							combo_2_poker(&p, c);
							add_poker(player->h,&p,player->h);
							//   search_combos_in_suit(game, player->h, player->opps, player);
						}
						else
						{
							add_poker(player->h,&t1,player->h);
							//                        search_combos_in_suit(game, player->h, player->opps, player);
							memcpy(player->card,card, max_POKER_NUM*sizeof(int));

						}
					}
					game->pre= pre;
					game->prepre_playernum=	   prepre;
					game->pre_playernum=prep;

				}
				PRINTF(LEVEL,"GuanPai \n");
				*cur=*c;
				return 0;
			}
			else
			{
				*p=*c;
				continue;
			}
		}
	}
	//Chai Pai
	if (!Lord_Chaipai(game, cur, pre,good_bomb))
		return 0;
	return 1;
}

//test if c is smaller than a
int is_smaller(COMBO_OF_POKERS* c, COMBO_OF_POKERS* a)
{
	if( c==NULL || a==NULL)
		return 0;
	if(c->type!=NOTHING && c->type==a->type && c->low <a->low )
	{
		if( c->type!=SINGLE_SERIES || c->len == a->len)
			return 1;
	}
	return 0;
}

//get number of combos in tobetested which is smaller than cur.
int get_num_of_smaller(COMBO_OF_POKERS* tobetested,int num, COMBO_OF_POKERS* cur)
{
	int t=0;
	for(int i=0; i<num; i++)
	{
		if(is_smaller(&tobetested[i],cur))
		{
			t++;
		}
	}
	return t;
}

//get number of combos in tobetested which is bigger than cur.
int get_num_of_bigger(COMBO_OF_POKERS* tobetested,int num, COMBO_OF_POKERS* cur)
{
	int t=0;
	for(int i=0; i<num; i++)
	{
		if(is_smaller(cur,&tobetested[i]))
		{
			t++;
		}
	}
	return t;
}

//get number of combos in s2 which there is a combo bigger in s1  than s2.
int get_num_of_bigger_in_combos(COMBO_OF_POKERS** s1,int n1, COMBO_OF_POKERS* s2)
{
	int t=0;
	// for(int i=0;i<n2;i++)
	{
		for(int j=0; j<n1; j++)
			if(is_smaller(s2,s1[j]))
			{
				t++;
				break;
			}
	}
	return t;
}

//get number of combos in s2 which there is a combo bigger in s1  than s2.
int get_num_of_smaller_in_combos(COMBO_OF_POKERS** s1,int n1, COMBO_OF_POKERS** s2, int n2)
{
	int t=0;
	for(int i=0; i<n2; i++)
	{
		for(int j=0; j<n1; j++)
			if(is_smaller(s1[j], s2[i]))
			{
				t++;
				break;
			}
	}
	return t;
}


void farmer_normal_play_first(GAME * game ,COMBO_OF_POKERS* c)//select c from player
{
	FUNC_NAME
		PLAYER * player = game->players[CUR_PLAYER];
	COMBOS_SUMMARY *s =player->summary;
	COMBO_OF_POKERS* t =NULL ;
	t = farm_select_combo_in_suit(game);

	if (t==NULL)
	{
		PRINTF_ALWAYS("ERR: line %d lord play first: stupid error\n",__LINE__);
		return;
	}
	*c=*t;
}

void downfarmer_good_play_first(GAME * game,COMBO_OF_POKERS* cur)//select cur from player
{
	FUNC_NAME
		PLAYER * player = game->players[CUR_PLAYER];
	COMBOS_SUMMARY *s =player->summary;
	COMBO_OF_POKERS* t =NULL ;
	//   {
	if (s->not_biggest_num == 2 && s->real_total_num == 2)
	{
		/*如果2种牌各剩1套，且没冲锋套时。如果有3张以上套，则优先出；如果只有单/对，则出号码小的那套，留号码大的套
		*/
		if ( player->summary->singles_num==1 &&  player->summary->pairs_num==1 )
		{
			/*
			for(int i=player->h->begin;i<player->h->end;i++)
			{
			cur->type=player->h->hands[i]==1?SINGLE:PAIRS;
			cur->low = i ;
			player->h->hands[i]=0;
			}t
			*/
			if ( player->summary->singles[0]->low > player->summary->pairs[0]->low )
			{
				memmove(cur, player->summary->pairs[0],sizeof(COMBO_OF_POKERS));
				player->summary->pairs[0]->type=NOTHING;
			}
			else {
				memmove(cur, player->summary->singles[0],sizeof(COMBO_OF_POKERS));
				player->summary->singles[0]->type=NOTHING;
			}
			remove_combo_in_suit(player->summary,cur);//to be optimized
		}
		else if ( player->summary->singles_num==2 )
		{
			memmove(cur, player->summary->singles[0],sizeof(COMBO_OF_POKERS));
			player->summary->singles[0]->type=NOTHING;
			remove_combo_in_suit(player->summary,cur);//to be optimized
		}
		else if ( player->summary->pairs_num==2 )
		{
			memmove(cur, player->summary->pairs[0],sizeof(COMBO_OF_POKERS));
			player->summary->pairs[0]->type=NOTHING;
			remove_combo_in_suit(player->summary,cur);//to be optimized
		}
		else
		{
			for (int k=20	; k>=0; k--)
			{
				if (get_combo_number(&player->combos[k])>=3)
				{
					memmove(cur, &player->combos[k],sizeof(COMBO_OF_POKERS));
					player->combos[k].type=NOTHING;
					remove_combo_in_suit(player->summary,cur);//to be optimized
					return;
				}
			}
		}
		return;
	}

	t = farm_select_combo_in_suit(game);

	if (t==NULL)
	{
		PRINTF_ALWAYS("ERR: line %d up farmaer play first: stupid error\n",__LINE__);
		return;
	}
	*cur=*t;	
}

void farmer_select_one_combo_for_other_farmer(GAME* game,PLAYER* pl,COMBO_OF_POKERS* cur,int isup)
{
	FUNC_NAME
		PLAYER* lord = game->player_type == UPFARMER ?
		game->players[DOWN_PLAYER] :
	game->players[UP_PLAYER];
	PLAYER* other= game->player_type == DOWNFARMER ?
		game->players[DOWN_PLAYER] :
	game->players[UP_PLAYER];

	search_combos_in_suit(game,
		game->players[DOWN_PLAYER]->h,
		game->players[DOWN_PLAYER]->opps,
		game->players[DOWN_PLAYER]);
	search_combos_in_suit(game,
		game->players[UP_PLAYER]->h,
		game->players[UP_PLAYER]->opps,
		game->players[UP_PLAYER]);

	PRINTF(LEVEL,"ENTER Fun farmer_select_one_combo_for_other_farmer: up %d\n"	,isup);

	if(isup)
	{
		/*
		if (is_player_ready_win(game,other))
		{
		//select a combo which other farmer needed.
		}
		*/
		//select a three one
		if(other->summary->three_one_num >=1 && lord->summary->three_one_num ==0 && lord->summary->three_two_num == 0)
		{
			if( find_a_smaller_combo_in_hands(pl->h,cur,
				other->summary->three_one[other->summary->three_one_num-1]))
			{
				POKERS t;
				cur->type = THREE;
				memmove(&t,pl->h,sizeof(POKERS));
				remove_combo_poker(&t,cur,NULL);
				search_combos_in_suit(game,&t,pl->opps,pl);
				cur->type =31;
				search_1_2_for_a_three(&t, pl,cur);
				pl->need_research = 1 ;
				return;
			}
		}
		//select a pairs
		if(other->summary->pairs_num>=1 &&
			(lord->summary->pairs_num==0 ||
			lord->summary->pairs[0]->low >= P2 ||
			lord->summary->pairs[(lord->summary->pairs_num-1)/2]->low < other->summary->pairs[(other->summary->pairs_num)/2]->low)
			)
		{
			do {
				if(lord->summary->pairs_num==0 )
				{
					int	finded=find_a_smaller_combo_in_hands(pl->h, cur,other->summary->pairs[other->summary->pairs_num-1]);
					if(!finded) break;
					else
					{
						pl->need_research = 1 ;
						return;
					}
				}
				else
				{
					int	finded=find_a_bigger_combo_in_hands(pl->h, cur,lord->summary->pairs[(lord->summary->pairs_num)/2]);
					if(!finded) break;
					else if(cur->low <  other->summary->pairs[(other->summary->pairs_num)/2]->low 
						&& cur->low<lord->lower_control_poker )
					{
						pl->need_research = 1 ;
						return;
					}
					else
						break;
				}
			} while(1);
		}
		//select a big single
		{
			//get the singles of lord..
			int single=char_to_poker('J') ;

			COMBOS_SUMMARY * s = game->players[DOWN_PLAYER]->summary;

			for(int i=s->singles_num-1; i>=0; i--)
			{
				if(s->singles[i]->low <=char_to_poker('K') )
				{
					single  = s->singles[i]->low;
					break;
				}
			}
			if(s->singles_num>=3)
			{
				if(single > s->singles[2]->low)
					single= s->singles[2]->low;
			}



			//   int pass = 1;
			for (int i=single; i<=char_to_poker('K'); i++ )
			{
				if ( pl->h->hands[i]==1 || pl->h->hands[i]==2)
				{
					cur->type=SINGLE;
					cur->low =i;
					cur->len = 1;
					pl->h->hands[i]=0;
					pl->need_research = 1;
					search_combos_in_suit(game,pl->h,pl->opps,pl);
					pl->h->hands[i]++;
					DBG( PRINTF(LEVEL,"!Bad Up Farmer Gepai\n"));
					//   pass = 0;
					return;
				}
			}
			//    if (pass == 1)
		}

		farmer_normal_play_first(game, cur);

	}
	else
	{
		COMBOS_SUMMARY * s= other->summary;
		//寻找牌组中，套数最多的一类牌
		int max=0,idx=0;
		if (s->threeseries_num+s->threeseries_one_num+s->threeseries_two_num >max) {
			max = s->threeseries_num+s->threeseries_one_num+s->threeseries_two_num ;
			idx=1;
		}
		if (s->pairs_series_num>max) {
			max=s->pairs_series_num;
			idx=2;
		}
		for (int i=0; i<10; i++)
			if (s->series_detail[i]>max) {
				max=s->series_detail[i];
				idx=6;
			}

			if (s->three_one_num+s->three_two_num+s->three_num>max) {
				max=s->three_one_num+s->three_two_num+s->three_num;
				idx=3;
			}
			if (s->pairs_num>max) {
				max=s->pairs_num;
				idx=4;
			}
			if (s->singles_num>max) {
				max=s->singles_num;
				idx=5;
			}

			if ( max >= 1 ) //todo: check for controls..
			{
				if (idx==4 && pl->summary->pairs_num>=1) {
					COMBO_OF_POKERS * t = pl->summary->pairs[0];
					memmove(cur,t,sizeof(COMBO_OF_POKERS));
					remove_combo_in_suit(pl->summary,t);
					t->type=NOTHING;
					return;
				}
				else if (idx==5 && pl->summary->singles_num>=1) {
					COMBO_OF_POKERS * t = pl->summary->singles[0];
					memmove(cur,t,sizeof(COMBO_OF_POKERS));
					remove_combo_in_suit(pl->summary,t);
					t->type=NOTHING;
					return;
				}

			}
			farmer_normal_play_first(game,cur);

	}

}
/*
void upfarmer_play_first_known_others(GAME* game,PLAYER* pl,COMBO_OF_POKERS* cur,int is_good)
{
//up_farmer is better than down_farmer
//int is_good = check_farmer_is_good(game);
if ( pl->oppDown_num == 1)
{
farmer_play_first_lord_has_1_poker_only(game,cur,is_good,cur,1);
return;
}

if(is_good)
{
upfarmer_good_play_first(game,cur);
}
else
{
farmer_select_one_combo_for_other_farmer(game,pl,cur,1);
}
}

void downfarmer_play_first_known_others(GAME* game,PLAYER* pl,COMBO_OF_POKERS* cur,int is_good)
{
if(is_good)
{
downfarmer_good_play_first(game, cur);
}
else
{
farmer_select_one_combo_for_other_farmer(game,pl,cur,0);
}
}

*/

int must_play_bigger_to_stop_lord(COMBO_OF_POKERS * cur, PLAYER* player,COMBO_OF_POKERS *pre,GAME * game,int isup, int isgood,int isfarmer)
{
	FUNC_NAME
		//printf("called\n");
		COMBO_OF_POKERS * c;
	int pass =1;	
	if ( pass==1) {
		COMBO_OF_POKERS tmp1,tmp,*p=&tmp;
		c= &tmp1;
		memmove(p,pre,sizeof(COMBO_OF_POKERS));
		int first_time =1;
		COMBOS_SUMMARY s;
		while ( find_a_bigger_combo_in_hands(player->h,c,p)) //could not handle corner case for  BOMB in two combos.
		{
			if(game->hun==-1)
			{
				POKERS t;
				remove_combo_poker(player->h,c,NULL);
				search_combos_in_suit(game,player->h,player->opps,player);           
				combo_2_poker(&t, c);
				add_poker(player->h,&t,player->h);
			}
			else
			{
				POKERS t,t1;
				combo_2_poker(&t,c);
				convert_poker_to_hun(&t,&t1,player->h);		 
				sub_poker(player->h, &t1, player->h);
				search_combos_in_suit(game,player->h,player->opps,player);           
				add_poker(player->h,&t1,player->h);
				if(c->type>=THREE_ONE)
				{

					c->control=2;//tmporary use
				}
				else 
					c->control=0;//tmporary use		   


			}
			if( c->type == ROCKET||c->type==BOMB)
			{
				if(player->summary->combo_total_num ==1
					&& !opp_hasbomb(game)) //to be refine
				{
					*cur=*c;
					return 0;		 	 
				}
				else  if(game->known_others_poker)
				{
					PLAYER *lord= player->id==UPFARMER?
						game->players[DOWN_PLAYER] :
					game->players[UP_PLAYER];
					if(lord->summary->control_num ==0 
						&& lord->summary->bomb_num ==0)
					{
						*cur=*c;
						return 0;		 	 	      	     	
					}			
					if(game->use_best_for_undefined_case)
					{ //todo: use check win
						return get_the_best_hand(game,cur,0);
					}
				}			

				if(first_time){
					*p = *c;
					continue;
				}
			}

			if( c->type == ROCKET)
			{
				if(player->summary->real_total_num ==1) //to be refine
				{
					*cur=*c;
					return 0;		 	 
				}
				if (first_time)
					return 1;
			}


			if ( first_time)
			{  
				s= *player->summary;       
				*cur=*c;
				first_time =0;
			}
			else if(cmp_summary(player->summary,&s)||!cmp_summary(&s,player->summary))
			{
				s=*player->summary; 
				*cur=*c;
			}

			*p = *c;
		}
		//}
		pass= first_time;
	}
	cur->control=0;
	return pass;
}

// search in player to get a bigger one than pre
// 1. search in summary first
// 2. search in hands, if find one, re-arrange hands and save the combo summary to player->summary
//save result to cur
int must_play_bigger(COMBO_OF_POKERS * cur, PLAYER* player,COMBO_OF_POKERS *pre,GAME * game,int isup, int isgood,int isfarmer)
{
	FUNC_NAME
		COMBO_OF_POKERS * c = find_combo(player->summary,pre);
	int pass =1 ;
	if ( c!=NULL && c->type!=BOMB && c->type!=ROCKET )// !(isup && !isgood && isfarmer))  //up bad farmer do not play bomb here.
	{
		remove_combo_in_suit(player->summary,c);
		memmove(cur,c,sizeof(COMBO_OF_POKERS));
		c->type=NOTHING;
		pass=0;
		DBG( PRINTF(LEVEL,"!Nongming Biguan: shunpai\n"));
		return pass;	
	}
	//if(c==NULL) //有炸弹不瞎拆
	{
		COMBO_OF_POKERS tmp1,tmp,*p=&tmp;
		c= &tmp1;
		memmove(p,pre,sizeof(COMBO_OF_POKERS));
		int first_time =1;
		PLAYER* suit1 = (PLAYER*)malloc(sizeof(PLAYER));
		POKERS h,opps;
		suit1->combos=suit1->combos_store;
		suit1->h = &suit1->p;
		suit1->opps = &suit1->opp;
		suit1->summary =&suit1->combos_summary;

		//int prob1 = get_prob(game);
		//int prob2 = get_prob_2(game);
		while ( find_a_bigger_combo_in_hands(player->h,&tmp1,p)) //could not handle corner case for  BOMB in two combos.
		{
			if(tmp1.type==BOMB || tmp1.type==ROCKET )
				break;
			if(c->type>=THREE_ONE)
			{

				c->control=2;//tmporary use
			}
			else 
				c->control=0;//tmporary use

			suit1->lower_control_poker = player->lower_control_poker;
			suit1->oppDown_num = player->oppDown_num;
			suit1->oppUp_num = player->oppUp_num;
			*suit1->h=*player->h;
			*suit1->opps=*player->opps;
			remove_combo_poker(suit1->h,&tmp1,NULL);//todo: fixme

			search_combos_in_suit(game,suit1->h,suit1->opps,suit1);
			if(suit1->lower_control_poker<player->lower_control_poker
				&& suit1->summary->ctrl.single > player->summary->ctrl.single )
			{//shit
				PRINTF(LEVEL,"pull back single control\n");
				suit1->summary->ctrl.single = player->summary->ctrl.single-1;
			}

			//update player in hands;
			PRINTF(LEVEL,"\n!farmer biguan, new_combo_num %d, new_ctrl %d pre_ctrl %d, pre_combo_num %d\n"
				,suit1->summary->combo_total_num ,CONTROL_NUM(suit1) ,CONTROL_NUM(player), player->summary->combo_total_num
				);

			if ( (first_time && suit1->summary->bomb_num >= player->summary->bomb_num)
				||(CONTROL_SUB_COMBO_NUM(suit1) > CONTROL_SUB_COMBO_NUM(player) )
				|| ( (CONTROL_SUB_COMBO_NUM(suit1) == CONTROL_SUB_COMBO_NUM(player) )
				&& suit1->summary->combo_total_num < player->summary->combo_total_num)
				|| ( (CONTROL_SUB_COMBO_NUM(suit1) == CONTROL_SUB_COMBO_NUM(player) )
				&& suit1->summary->combo_total_num == player->summary->combo_total_num
				&& suit1->summary->combo_typenum < player->summary->combo_typenum )
				|| ( (CONTROL_SUB_COMBO_NUM(suit1) == CONTROL_SUB_COMBO_NUM(player) )
				&& suit1->summary->combo_total_num == player->summary->combo_total_num
				&& (suit1->summary->singles_num< player->summary->singles_num||
				suit1->summary->real_total_num< player->summary->real_total_num)  
				)                         

				/*  ||  ( (CONTROL_SUB_COMBO_NUM(&suit1) >= (CONTROL_SUB_COMBO_NUM(player)+10) )
				&& get_prob1(game) )
				||   ( (CONTROL_SUB_COMBO_NUM(&suit1) >= (CONTROL_SUB_COMBO_NUM(player)+10) )
				&& get_prob1(game) )*/
				)
			{
				PRINTF(LEVEL,"got better\n");
				*player->summary =* suit1->summary;
				first_time = 0;
				*cur=tmp1;
			}
			*p = tmp1;
		}
		//}
		free(suit1);
		if(!first_time)	
			search_combos_in_suit(game,player->h,player->opps,player);	
		cur->control=0;
		return first_time;
	}
	return pass;
}


//地主报单农民必管逻辑
int must_play_bigger_for_single(COMBO_OF_POKERS * cur, PLAYER* player,COMBO_OF_POKERS *pre,GAME * game,int isup, int isgood,int isfarmer)
{
	FUNC_NAME
		COMBO_OF_POKERS * c = find_combo(player->summary,pre);
	int pass =1 ;
	if ( c==NULL) {
		//拆牌
		COMBO_OF_POKERS tmp1,tmp,*p=&tmp;
		c= &tmp1;
		memmove(p,pre,sizeof(COMBO_OF_POKERS));
		int first_time =1;
		PLAYER suit1;
		POKERS h,opps;
		COMBO_OF_POKERS combo[20]= {20*0};
		COMBOS_SUMMARY sum= {0},*s;
		suit1.combos=&combo[0];
		suit1.h = &h;
		suit1.opps = &opps;
		suit1.summary =s = &sum;
		while ( find_a_bigger_combo_in_hands(player->h,c,p))
		{
			suit1.lower_control_poker = player->lower_control_poker;
			suit1.oppDown_num = player->oppDown_num;
			suit1.oppUp_num = player->oppUp_num;
			memmove(&h,player->h,sizeof(POKERS));

			memmove(&opps,player->opps,sizeof(POKERS));
			remove_combo_poker(suit1.h,c,NULL);
			if(c->type>=THREE_ONE)
			{

				c->control=2;//tmporary use
			}
			else 
				c->control=0;//tmporary use
			search_combos_in_suits_for_min_single_farmer(game,suit1.h,suit1.combos,&suit1);

			//update player in hands;
			DBG( PRINTF(LEVEL,"\n!farmer biguan, new_combo_num %d, new_ctrl %d pre_ctrl %d, pre_combo_num %d\n"
				,suit1.summary->combo_total_num ,CONTROL_NUM(&suit1) ,CONTROL_NUM(player), player->summary->combo_total_num
				));
			if ( first_time
				||( isup && get_2nd_min_singles(suit1.summary) > get_2nd_min_singles(player->summary))//寻找第二小的单牌最大的组合
				||( !isup && suit1.summary->singles_num < player->summary->singles_num )//寻找单牌最少的组合
				)
			{
				*player->summary = *suit1.summary;
				first_time = 0;
				*cur=*c;
			}
			*p = *c;
		}
		cur->control =0;
		//}
		return first_time;
	}
	else if ( c->type!=BOMB || 1)// !(isup && !isgood && isfarmer)) //检查炸弹？
	{
		remove_combo_in_suit(player->summary,c);
		memmove(cur,c,sizeof(COMBO_OF_POKERS));
		c->type=NOTHING;
		pass=0;
		DBG( PRINTF(LEVEL,"!Nongming Biguan: shunpai\n"));
	}


	return pass;
}
/*
int farmer_shunpai(PLAYER * player ,COMBO_OF_POKERS* cur, COMBO_OF_POKERS* pre,GAME * game)
{

}
*/
int farmer_Chaipai(GAME*game,COMBO_OF_POKERS* cur, COMBO_OF_POKERS* pre)
{
	FUNC_NAME
		PLAYER * player =game->players[CUR_PLAYER];
	COMBO_OF_POKERS tmp1;
	COMBO_OF_POKERS* c= &tmp1;
	COMBO_OF_POKERS tmp,*p =&tmp;
	memmove(p,pre,sizeof(COMBO_OF_POKERS));
	int first_time =1;
	int mustPlay = 0;
	// COMBOS_SUMMARY *s =player->summary;

	//to be optimized by using new search methods..
	while ( find_a_bigger_combo_in_hands(player->h,c,p)) //could not handle corner case for  BOMB in two combos.
	{
		if(c->type==BOMB || c->type==ROCKET)
			break;
		if(c->type>=THREE_ONE)
		{
			c->control=2;//tmporary use
		}
		else 
			c->control=0;//tmporary use	
		PLAYER tmp_player;
		POKERS h,opps;
		COMBO_OF_POKERS combo[20]= {20*0};
		COMBOS_SUMMARY sum= {0},*s;
		tmp_player.combos=&combo[0];
		tmp_player.h = &h;
		tmp_player.opps = &opps;
		tmp_player.summary =s = &sum;
		tmp_player.lower_control_poker = player->lower_control_poker;
		tmp_player.oppDown_num = player->oppDown_num;
		tmp_player.oppUp_num = player->oppUp_num;
		memmove(&h,player->h,sizeof(POKERS));

		memmove(&opps,player->opps,sizeof(POKERS));
		remove_combo_poker(tmp_player.h,c,NULL);

		search_combos_in_suit(game,tmp_player.h,tmp_player.opps,&tmp_player);

		int max_poker_in_c = has_control_poker(c, player->lower_control_poker);

		if(c->type==SINGLE ||c->type==PAIRS)
		{
			if(c->low>=player->lower_control_poker)
				assume_remove_controls_in_opps(&opps, c,max_poker_in_c);
			int ctrl_num = 	CONTROL_POKER_NUM;
			POKERS all;
			add_poker(tmp_player.h,&opps,&all);
			if(all.total <=20 )
				ctrl_num = 4;
			else if(all.total <=10 )
				ctrl_num = 2;	    
			tmp_player.lower_control_poker = get_lowest_controls(&all,ctrl_num);				
			tmp_player.summary->ctrl.single = calc_controls( tmp_player.h,tmp_player.opps, ctrl_num );
		}
		if(tmp_player.lower_control_poker<player->lower_control_poker
			&& tmp_player.summary->ctrl.single > player->summary->ctrl.single )
		{
			PRINTF(LEVEL,"pull back single control\n");
			tmp_player.summary->ctrl.single = player->summary->ctrl.single-1;
		}


		//update the lowest control poker and update the summary

		if ( CONTROL_SUB_COMBO_NUM_IS_GOOD(&tmp_player)
			||  CONTROL_SUB_COMBO_NUM(&tmp_player) >= CONTROL_SUB_COMBO_NUM(player)
			|| CONTROL_NUM(&tmp_player)>=30
			|| tmp_player.summary->combo_total_num<=1 
			||(PRE_PLAYER(game)==LORD && 
			CONTROL_SUB_COMBO_NUM(&tmp_player)+10 >= CONTROL_SUB_COMBO_NUM(player)
			&& NOT_BOMB(c))
			)
		{
			goto SAVE_CUR_RESULT;
		}
		else
		{
			*p=*c;
			continue;
		}

SAVE_CUR_RESULT:
		//update player in hands;
		PRINTF(LEVEL,"!farmer chaipai, ctrl %d control-combo %d combo_num %d, old(%d,%d,%d)\n",
			tmp_player.summary->ctrl.single,CONTROL_SUB_COMBO_NUM(&tmp_player),tmp_player.summary->combo_total_num,player->summary->ctrl.single,
			CONTROL_SUB_COMBO_NUM(player), player->summary->combo_total_num
			);
		print_combo_poker(c);
		if ( first_time
			||(CONTROL_SUB_COMBO_NUM(&tmp_player) > CONTROL_SUB_COMBO_NUM(player) )
			|| ( (CONTROL_SUB_COMBO_NUM(&tmp_player) == CONTROL_SUB_COMBO_NUM(player) )
			&& tmp_player.summary->combo_total_num < player->summary->combo_total_num)
			|| ( (CONTROL_SUB_COMBO_NUM(&tmp_player) == CONTROL_SUB_COMBO_NUM(player) )
			&& tmp_player.summary->combo_total_num == player->summary->combo_total_num
			&& tmp_player.summary->combo_typenum < player->summary->combo_typenum )
			||  ( (CONTROL_SUB_COMBO_NUM(&tmp_player) == CONTROL_SUB_COMBO_NUM(player) )
			&& tmp_player.summary->combo_total_num == player->summary->combo_total_num
			&& tmp_player.summary->combo_typenum == player->summary->combo_typenum
			&& tmp_player.summary->singles_num < player->summary->singles_num )
			/* ||   ( (CONTROL_SUB_COMBO_NUM(&tmp_player) == CONTROL_SUB_COMBO_NUM(player) )
			&& tmp_player.summary->combo_total_num == player->summary->combo_total_num
			&& tmp_player.summary->combo_typenum == player->summary->combo_typenum
			&& tmp_player.summary->singles_num == player->summary->singles_num )*/
			)
		{
			*player->summary = *tmp_player.summary;
			first_time = 0;
			*cur=*c;
		}
		*p = *c;
	}
	cur->control = 0;//reset..
	return first_time;
}

int farmer_play_normal(GAME*game,COMBO_OF_POKERS* cur, COMBO_OF_POKERS* pre)
{
	FUNC_NAME
		PLAYER * player  = game->players[CUR_PLAYER];
	int mustPlay = 0;
	int pass;
	//    if(!farmer_shunpai(player, cur, pre, game))
	//		return 0;

	//Chai Pai
	if (!farmer_Chaipai(game, cur, pre))
		return 0;

	/* //use bomb..
	if( PRE_PLAYER(game)== LORD ) //double check!!
	{
	return get_the_best_hand(game, cur, 0);
	}
	*/
	return 1;
}

int farmer_play_when_lord_has_1_poker_only(COMBO_OF_POKERS* cur, COMBO_OF_POKERS* pre, PLAYER*player, GAME*game, int isup,int isgood)
{
	FUNC_NAME
		SET_PLAYER_POINT(game);
	if (game->players[game->pre_playernum]->id == LORD && pre->type != SINGLE)
	{
		//地主报单农民必管逻辑
		search_combos_in_suits_for_min_single_farmer(game,player->h,player->combos,player);
		return must_play_bigger_for_single(cur,player,pre,game,isup,isgood,1);
	}
	else
	{
		if ( pre->type == SINGLE)
		{
			if ( game->players[game->pre_playernum]->id == UPFARMER)
			{   //当前为地主下家农民
				goto _CHECK_GOOD_ENOUGH;
			}
			//current is downfarmer

			int lord_single=  player->opps->end;
			if(game->known_others_poker)
			{
				lord_single= downPl->h->end;
			}
			search_combos_in_suits_for_min_single_farmer(game,player->h,player->combos,player);											
			if(get_2nd_min_singles(player->summary)> lord_single)
			{
				COMBO_OF_POKERS *c;
				if(player->summary->singles_num>1
					&& player->summary->singles[player->summary->singles_num-1]->low
			> max(pre->low,lord_single)){
				*cur=*player->summary->singles[player->summary->singles_num-1];
				return 0;
				}				
				c=find_combo((player->summary), pre);
				if(c!=NULL)	   
				{
					*cur=*c;
					return 0;
				}
			}			
			for (int k=max(pre->low+1,lord_single); k<=player->h->end; k++ )
			{
				if (player->h->hands[k]>0)
				{
					cur->type=SINGLE;
					cur->low = k ;
					cur->len = 1;
					player->h->hands[k]--;		
					search_combos_in_suits_for_min_single_farmer(game,player->h,player->combos,player);
					player->h->hands[k]++;
					if(get_2nd_min_singles(player->summary)> lord_single)
					{		
						return 0;
					}
				}
			}
			//如果该牌为外面最大.. 则过牌？
			if ( game->players[game->pre_playernum]->id == DOWNFARMER
				&&( pre->low >= lord_single )
				)
			{
				return 1;
			}
			else
			{
				//如果手中有绝对大的单牌，则出
				for (int k=max(pre->low+1,lord_single); k<=player->h->end; k++ )
				{
					if (player->h->hands[k]==1)
					{
						cur->type=SINGLE;
						cur->low = k ;
						cur->len = 1;
						{		
							return 0;
						}
					}
				}
				for (int k=max(pre->low+1,lord_single); k<=player->h->end; k++ )
				{
					if (player->h->hands[k]>1)
					{
						cur->type=SINGLE;
						cur->low = k ;
						cur->len = 1;
						{		
							//player->h->hands[i]--;
							return 0;
						}
					}
				}

				if (player->summary->bomb_num!=0 )//
				{
					if ( isup)
					{
						memmove(cur,player->summary->bomb[0],sizeof(COMBO_OF_POKERS));
						player->summary->bomb[0]->type=NOTHING;
						remove_combo_in_suit(player->summary,cur);

						return 0;
					}
					else
					{
						POKERS t;
						memmove(&t,player->h,sizeof(POKERS));
						remove_combo_poker(&t,player->summary->bomb[0],NULL); //todo: for mulit bomb
						memmove(cur,player->summary->bomb[0],sizeof(COMBO_OF_POKERS));
						player->summary->bomb[0]->type=NOTHING;
						remove_combo_in_suit(player->summary,cur);
						search_combos_in_suits_for_min_single_farmer(game,&t,player->combos,player);
						if ( get_2nd_min_singles(player->summary) >= player->opps->end )
						{   //heshou
							return 0;
						}
						else
						{
							search_combos_in_suits_for_min_single_farmer(game,player->h,player->combos,player);
						}
						return 1;
					}
				}

				if ( isup)
				{
					if (player->h->end>pre->low)
					{
						cur->type=SINGLE;
						cur->low = player->h->end;
						cur->len = 1;
						player->need_research = 1;
						POKERS t;
						memmove(&t,player->h,sizeof(POKERS));
						remove_combo_poker(&t,cur,NULL);
						search_combos_in_suit(game,&t,player->opps,player);
						//player->h->hands[i]--;
						return 0;
					}
				}
				return 1;
			}
		}
		else //另外一个农民出的非单牌
		{
_CHECK_GOOD_ENOUGH:
			search_combos_in_suits_for_min_single_farmer(game,player->h,player->combos,player);
			if ( get_2nd_min_singles(player->summary) >= player->opps->end ) //合手
			{
				COMBO_OF_POKERS* c= find_combo(player->summary,pre);//
				if ( c==NULL)
				{
					COMBO_OF_POKERS tmp1,tmp,*p=&tmp;
					c= &tmp1;
					memmove(p,pre,sizeof(COMBO_OF_POKERS));
					int first_time =1;
					PLAYER suit1;
					POKERS h,opps;
					COMBO_OF_POKERS combo[20]= {20*0};
					COMBOS_SUMMARY sum= {0},*s;
					suit1.combos=&combo[0];
					suit1.h = &h;
					suit1.opps = &opps;
					suit1.summary =s = &sum;
					while ( find_a_bigger_combo_in_hands(player->h,c,p)) //could not handle corner case for  BOMB in two combos.
					{
						if(c->type>=THREE_ONE)
						{

							c->control=2;//tmporary use
						}
						else 
							c->control=0;//tmporary use

						suit1.lower_control_poker = player->lower_control_poker;
						suit1.oppDown_num = player->oppDown_num;
						suit1.oppUp_num = player->oppUp_num;
						memmove(&h,player->h,sizeof(POKERS));

						memmove(&opps,player->opps,sizeof(POKERS));
						remove_combo_poker(suit1.h,c,NULL);

						search_combos_in_suits_for_min_single_farmer(game,suit1.h,suit1.combos,&suit1);


						if ( get_2nd_min_singles(suit1.summary) >= player->opps->end )
						{   //update player in hands;
							DBG( PRINTF(LEVEL,"\n!farmer biguan, new_combo_num %d, new_ctrl %d pre_ctrl %d, pre_combo_num %d\n"
								,suit1.summary->combo_total_num ,CONTROL_NUM(&suit1) ,CONTROL_NUM(player), player->summary->combo_total_num
								));
							memmove(player->combos , suit1.combos ,  20* sizeof(COMBO_OF_POKERS)); //todo refine;
							memset(player->summary, 0, sizeof(COMBOS_SUMMARY));
							sort_all_combos(game,player->combos,20,player->summary,player->opps,player->oppDown_num
								,player->oppUp_num,player->lower_control_poker,player); //to remove this..
							first_time = 0;
							memmove(cur,c,sizeof(COMBO_OF_POKERS));
							break;
						}
						*p= *c;
					}
					//}
					cur->control=0;
					return first_time;
				}
				else if ( c->type!=BOMB || 1)// !(isup && !isgood && isfarmer))  //up bad farmer do not play bomb here.
				{
					remove_combo_in_suit(player->summary,c);
					memmove(cur,c,sizeof(COMBO_OF_POKERS));
					c->type=NOTHING;

					DBG( PRINTF(LEVEL,"!Nongming Biguan: shunpai\n"));
					return 0;
				}
			}
			else if (pre->type==SINGLE )//goto...
			{
				return 1;
			}
		}
	}
	return 1;
}

void farmer_play_first_lord_has_1_poker_only( GAME*game,COMBO_OF_POKERS* cur,int is_good_farmer, PLAYER*player, int isup)
{
	FUNC_NAME
		search_combos_in_suits_for_min_single_farmer(game,player->h,player->combos,player);

	int second_single= get_2nd_min_singles(player->summary);
	int lord_single = player->opps->end;

	if(game->known_others_poker)
	{
		PLAYER* lord = game->player_type == UPFARMER ?
			game->players[DOWN_PLAYER] :
		game->players[UP_PLAYER];
		lord_single = lord->h->end;
	}

	if( game->known_others_poker )
	{
		if (!isup && game->players[DOWN_PLAYER]->h->total==1) //down player is farmer ,and has only one poker..
		{
			if(game->players[DOWN_PLAYER]->h->end > player->h->begin)
			{
				//todo: check bombs
				cur->len=1;
				cur->type =SINGLE;
				cur->low = player->h->begin ;
				player->need_research = 1 ;
				return;
			}
		}

		if (player->summary->singles_num <=1
			|| second_single >= lord_single ) //self will win!
		{
			COMBO_OF_POKERS * res;
			if ( (res=find_max_len_in_combos(player->combos, 20))!=NULL)
			{
				memmove(cur,res,sizeof(COMBO_OF_POKERS));
				res->type=NOTHING;
				remove_combo_in_suit(player->summary,cur);
				return;
			}
			else
			{
				PRINTF_ALWAYS("line %d stupid error happens...\n",__LINE__);
			}
		}


		//search the other farmer's poker
		PLAYER* other_farmer = game->player_type == DOWNFARMER ?
			game->players[DOWN_PLAYER] :
		game->players[UP_PLAYER];
		search_combos_in_suits_for_min_single_farmer(game,other_farmer->h,other_farmer->combos,other_farmer);

		//the other farmer can win
		int the2ndsingle= get_2nd_min_singles(other_farmer->summary);
		if(the2ndsingle > lord_single)
		{
			//let the other farmer play..
			//check pairs,three_one,series..
			if(other_farmer->summary->pairs_num>=1 )
			{
				if( find_a_smaller_combo_in_hands(player->h,cur,
					other_farmer->summary->pairs[other_farmer->summary->pairs_num-1]))
				{
					player->need_research = 1 ;
					return;
				}
			}

			if(other_farmer->summary->series_num >=1 )
			{
				if( find_a_smaller_combo_in_hands(player->h,cur,
					other_farmer->summary->series[other_farmer->summary->series_num-1]))
				{
					player->need_research = 1 ;
					return;
				}
			}

			if(other_farmer->summary->three_one_num >=1 )
			{
				if( find_a_smaller_combo_in_hands(player->h,cur,
					other_farmer->summary->three_one[other_farmer->summary->three_one_num-1]))
				{
					POKERS t;
					cur->type = THREE;
					memmove(&t,player->h,sizeof(POKERS));
					remove_combo_poker(&t,cur,NULL);
					search_combos_in_suit(game,&t,player->opps,player);
					cur->type =31;
					search_1_2_for_a_three(&t, player,cur);
					player->need_research = 1 ;
					return;
				}
			}

		}

	}

	if (player->summary->singles_num <=1
		|| second_single >= lord_single ) //win!
	{
		COMBO_OF_POKERS * res;
		if ( (res=find_max_len_in_combos(player->combos, 20))!=NULL)
		{
			memmove(cur,res,sizeof(COMBO_OF_POKERS));
			res->type=NOTHING;
			remove_combo_in_suit(player->summary,cur);
			return;
		}
		else
		{
			PRINTF_ALWAYS("line %d stupid error happens...\n",__LINE__);
		}
	}
	else   if (!isup && game->players[CUR_PLAYER]->oppDown_num==1) //down player is farmer ,and has only one poker..
	{
		for (int i =player->h->begin; i<player->h->end ; i++)
		{
			if (player->h->hands[i]>=1)
			{
				//player->h->hands[i]-=2;
				cur->len=1;
				cur->type =SINGLE;
				cur->low = i ;
				player->need_research = 1 ;
				return;
				//goto combo_selected;
			}
		}
	}
	else
	{
		if (!isup && player->summary->singles_num ==2)
		{        
			if (player->summary->pairs_num >= 2)//todo:fixed me
			{
				*cur=*player->summary->pairs[0];
				return;
			}			
			if (player->h->total==2)
			{
				// 如果只有2套单牌，则打出最大的一套单牌，否则            
				*cur=*player->summary->singles[1];
				return;
			}
		}       
		//down farmer 出牌逻辑
		if(!isup)
		{
			COMBO_OF_POKERS* t = find_smallest_in_combos(player->combos,20,player,1);
			if(t!=NULL)
			{
				*cur=*t;
				return;
			}		   
		}

		for (int i =player->h->begin; i<=player->h->end ; i++)
		{
			if (player->h->hands[i]>=2)
			{
				//player->h->hands[i]-=2;
				cur->len=1;
				cur->type =PAIRS;
				cur->low = i ;
				//check if others has a big than this
				COMBO_OF_POKERS c;
				if(find_a_bigger_combo_in_hands(player->opps,&c,cur))
					return;
				//goto combo_selected;
			}
		}

		COMBO_OF_POKERS* t = find_smallest_in_combos(player->combos,20,player,1);
		if(t!=NULL)
		{
			*cur=*t;
			return;
		}


		//f(isup)
		{
			//player->h->hands[i]-=1;
			cur->len=1;
			cur->type =SINGLE;
			cur->low = isup? player->h->end : player->h->begin;
			player->need_research = 1;
			return;
			//goto combo_selected;
		}
	}
}

//todo: update this
COMBO_OF_POKERS* upfarmer_good_play_first(GAME * game,COMBO_OF_POKERS* cur )//select cur from play
{
	FUNC_NAME
		PLAYER* player =game->players[CUR_PLAYER];
	PLAYER * lord = game->players[DOWN_PLAYER];
	PLAYER * dnFm= game->players[UP_PLAYER];

	COMBOS_SUMMARY* s = player->summary ;

	if ( !is_player_ready_win(game,player) && game->known_others_poker ) //不合手
	{
		COMBO_OF_POKERS * list[MAX_COMBO_NUM];
		int num = 0;

		//先出 地主组合里没有的牌....
		for(int i=0; i<s->not_biggest_num; i++)
		{
			if(!get_num_of_bigger(lord->combos,MAX_COMBO_NUM,s->not_biggest[i]))
			{
				list[num++] = s->not_biggest[i];
			}
		}

		if(num>0)
		{
			//先出自己有回手的
			for(int i=0; i<num; i++)
			{
				if(get_num_of_bigger_in_combos(s->biggest,s->biggest_num,list[i]))
				{
					return list[i];
				}
			}
			return list[0];
		}


		if(s->biggest_num >0)
		{
			//出地主没有的牌, 且不含大牌控制的牌
			for(int i=0; i<s->biggest_num; i++)
			{
				if(!get_num_of_smaller(lord->combos,MAX_COMBO_NUM,s->biggest[i]))
				{
					if(s->biggest[i]->type == SINGLE_SERIES )
						return s->biggest[i];
					if(!has_control_poker(s->biggest[i], game->lowest_bigPoker))
						return s->biggest[i];
				}
			}
		}

		//出自己或同伴有回手的

		num =0;

		//出地主只有用大牌管的
		for(int i=0; i<s->not_biggest_num; i++)
		{
			if(!get_num_of_bigger_in_combos(lord->summary->not_biggest,
				lord->summary->not_biggest_num
				,s->not_biggest[i]))
			{
				//refine?
				if(!has_control_poker(s->not_biggest[i], player->lower_control_poker))
					return s->not_biggest[i];
			}
		} 

		if(game->use_best_for_undefined_case && lord->h->total<=2)
		{
			get_the_best_hand(game, &game->tmp_for_best, 1);
			return &game->tmp_for_best;
		}

		//出所有套牌中标签最小的一套。（两套牌标签相同时，出张数多的一套）
	}

	// rocket or biggest bomb and other..
	// for chuntian
	//bomb is biggest


	// win for sure....
	COMBO_OF_POKERS * t= farm_select_combo_in_suit(game);
	return  t;


	//寻找牌组中，套数最多的一类牌


}


void upfarmer_bad_play_first(GAME* game, COMBO_OF_POKERS* cur)
{
	FUNC_NAME
		PLAYER* curPlayer = game->players[CUR_PLAYER];
	PLAYER *downfarmer= game->players[UP_PLAYER];
	PLAYER * lord= game->players[DOWN_PLAYER];

	if (!game->known_others_poker && 
		game->players[(CUR_PLAYER)]->oppUp_num == 1 )
	{        
		cur->len=1;
		cur->type =SINGLE;
		cur->low = curPlayer->h->begin ;
		return;
	}

	if (!game->known_others_poker && 
		game->players[CUR_PLAYER]->oppUp_num == 2 )
	{
		for (int i = curPlayer->h->begin; i<=curPlayer->h->end; i++)
		{
			if (curPlayer->h->hands[i]>=2)
			{
				//curPlayer->h->hands[i]-=2;
				cur->len=1;
				cur->type =PAIRS;
				cur->low = i;
				if (is_combo_biggest(game,curPlayer->opps,cur,curPlayer->oppUp_num,curPlayer->oppDown_num,curPlayer->lower_control_poker))
					goto __play_single_upfarmer;
				return;
			}
		}
		goto __play_single_upfarmer;
	} 

	if(game->known_others_poker)
	{
		if(game->use_best_for_undefined_case && (lord->h->total <=2 || game->hun==-1))
		{
			get_the_best_hand( game,cur,1);
			//double check
			if(cur->type==PAIRS && lord->summary->real_total_num ==1 
				&& lord->h->total == 2 && lord->h->begin > cur->low)
				goto __play_single_upfarmer;
			return;
		}
	}


	if ( curPlayer->oppDown_num!= 2 )
	{
		/*优先出对牌，从88开始往小找*/
		for (int i = char_to_poker('8'); i>0; i--)
		{
			if (curPlayer->h->hands[i]==2)
			{
				//curPlayer->h->hands[i]-=2;
				cur->len=1;
				cur->type =PAIRS;
				cur->low = i ;
				return;
				//goto combo_selected;
			}
		}
	}
__play_single_upfarmer:
	/*没有对牌则出单牌，除去控制，从大往小出*/

	for (int i = curPlayer->lower_control_poker-1; i>=curPlayer->h->begin; i--)
	{
		if (curPlayer->h->hands[i]==1)
		{
			//curPlayer->h->hands[i]-=1;
			cur->len=1;
			cur->type =SINGLE;
			cur->low = i ;
			return;
			//goto combo_selected;
		}
	}

	COMBO_OF_POKERS * t= farm_select_combo_in_suit(game);
	*cur=*t;	
	return;
}

int upfarmer_play_when_down_farmer_has_only_1_poker(GAME *game, COMBO_OF_POKERS * cur)
{
	FUNC_NAME
		PLAYER* curPlayer = game->players[CUR_PLAYER];
	PLAYER *dnfm= game->players[UP_PLAYER];
	PLAYER * lord= game->players[DOWN_PLAYER];

	//todo..
	// 不拆炸弹，先诈炸弹。。。
	if (game->known_others_poker && dnfm->h->total == 1 && lord->h->total>1 && (dnfm->h->begin > lord->h->end) )
	{
		for(int i=0; i<curPlayer->summary->singles_num; i++)
		{
			if(curPlayer->summary->singles[i]->low < dnfm->h->begin )
			{
				*cur = *curPlayer->summary->singles[i];
				return 1;
			}
		}

		if( dnfm->h->begin > 	curPlayer->h->begin )
		{
			cur->len=1;
			cur->type =SINGLE;
			cur->low = curPlayer->h->begin ;
			return 1;
		}
	}

	if(game->known_others_poker && dnfm->summary->biggest_num > 0 )
	{
		if( find_a_smaller_combo_in_hands(curPlayer->h, cur, dnfm->summary->biggest[0]))
		{
			//todo: double check if lord has the same one...
			return 1;
		}
	}

	if(game->known_others_poker && dnfm->summary->not_biggest_num >0)
	{
		if( !get_num_of_bigger(lord->combos, MAX_COMBO_NUM, dnfm->summary->not_biggest[0]))
		{
			//if(get_num_of_smaller(curPlayer->combos, MAX_COMBO_NUM,dnfm->summary->not_biggest[0]))
			for(int i=0; i< MAX_COMBO_NUM; i++)
			{
				if(is_smaller(&curPlayer->combos[i], dnfm->summary->not_biggest[0]))
				{
					*cur = curPlayer->combos[i];
					return 1;
				}
			}
		}
	}
	return 0;	
}

//select a combo from hands, save to cur
void upfarmer_play_first(GAME* game, COMBO_OF_POKERS* cur)
{
	FUNC_NAME
		PLAYER* curPlayer = game->players[CUR_PLAYER];
	PLAYER *dnfm= game->players[UP_PLAYER];
	PLAYER * lord= game->players[DOWN_PLAYER];

	int is_good_farmer =curPlayer->good_framer;

	COMBO_OF_POKERS * c = get_combo_in_a_win_suit(game);
	if(c!=NULL)
	{
		int good=1;
		if ( curPlayer->oppDown_num == 1 && c->type ==SINGLE 
			&& c->low < curPlayer->opps->end )
			good=0;
		if ( curPlayer->oppDown_num == 2 && c->type ==PAIRS
			&& !is_combo_biggest_sure(game,c))
		{
			good=0;
		}
		if(good){
			*cur=*c;
			return;
		}
	}    
	if ( curPlayer->oppDown_num == 1)
	{
		return farmer_play_first_lord_has_1_poker_only(game,cur,is_good_farmer,curPlayer,1);
	}

	//check for down farmer win..
	if(game->known_others_poker)
	{
		if (dnfm->summary->real_total_num ==1){
			if(upfarmer_play_when_down_farmer_has_only_1_poker(game, cur))
				return;
		}		
	}

	if(game->known_others_poker && lord->h->total == 2 )
	{
		if(game->use_best_for_undefined_case)
		{
			get_the_best_hand( game,cur,1);
			if(cur->type==PAIRS && lord->summary->real_total_num ==1 
				&& lord->h->begin > cur->low)
			{
			}
			else
				return;
		}
	}

	if ( curPlayer->oppDown_num == 2)
	{
		if(curPlayer->h->end > curPlayer->opps->end) //use known_other_poker
		{
			if (curPlayer->summary->singles_num>0)
			{
				int found=0;
				for(int i=0;i<curPlayer->summary->singles_num;i++)
					if(curPlayer->summary->singles[i]->low<
						curPlayer->opps->end)
					{
						*cur=*curPlayer->summary->singles[i];
						found=1;
					}
					if (found) return;
			}  
			if(curPlayer->summary->pairs_num>0 &&
				(!is_combo_biggest_sure(game,curPlayer->summary->pairs[0])))
			{ //拆对!
				cur->low=curPlayer->summary->pairs[0]->low;
				cur->type=SINGLE;
				return;
			}			 
		}
		//check opps has pairs?
		int haspairs=0;
		for(int i=0;i<=P2;i++)
		{
			if(curPlayer->opps->hands[i]>=2)
			{
				haspairs=1;
				break;
			}
		}
		if(haspairs==1)
		{
			if(curPlayer->summary->pairs_num>0 &&
				(!is_combo_biggest_sure(game,curPlayer->summary->pairs[0])))
			{ 
				if (curPlayer->summary->singles_num>0)
				{
					int found=0;
					for(int i=0;i<curPlayer->summary->singles_num;i++)
						if(curPlayer->summary->singles[i]->low<
							curPlayer->opps->end)
						{
							*cur=*curPlayer->summary->singles[i];
							found=1;
						}
						if (found) return;
				}	         
				else if(curPlayer->summary->pairs_num>1)
				{
					//check the second pairs is the biggest?
					if(!is_combo_biggest_sure(game,curPlayer->summary->pairs[1]))
					{ //2e??!
						cur->low=curPlayer->summary->pairs[0]->low;
						cur->type=SINGLE;
						return;
					}
				}
			}
		}
	}



	if(check_only_2_same_combo(game,cur))
	{
		return;
	}	   

	if ( is_good_farmer) {
		COMBO_OF_POKERS * res = (upfarmer_good_play_first(game,cur));
		if(res==NULL)
		{
			PRINTF(LEVEL,"ERR stupid error @ line %d\n",__LINE__);
		}
		else
			*cur=*res;
	}
	else
		upfarmer_bad_play_first(game,cur);
	return;
}


void downfarmer_bad_play_first(GAME* game, COMBO_OF_POKERS* cur)
{
	FUNC_NAME
		PLAYER* curPlayer = game->players[CUR_PLAYER];
	PLAYER *lord= game->players[UP_PLAYER];
	PLAYER * upfm= game->players[DOWN_PLAYER];
	if(!game->known_others_poker)
	{
		if ( curPlayer->oppUp_num == 2 )
		{
			for (int i = curPlayer->h->begin; i<=curPlayer->h->end; i++)
			{
				if (curPlayer->h->hands[i]>=2)
				{
					//curPlayer->h->hands[i]-=2;
					cur->len=1;
					cur->type =PAIRS;
					cur->low = i;
					if (is_combo_biggest(game,curPlayer->opps,cur,curPlayer->oppUp_num,curPlayer->oppDown_num,curPlayer->lower_control_poker))
						return;
				}
			}
		}
		else
		{
			COMBO_OF_POKERS* t = find_smallest_in_combos(curPlayer->combos,20,curPlayer,0);
			*cur=*t;
		}
	}

	//todo: when up farmer is ready to win...

	//	PLAYER *lord= game->players[UP_PLAYER];
	//    PLAYER * upfm= game->players[DOWN_PLAYER];

	if( game->known_others_poker && upfm->summary->combo_total_num <=2)
	{
		//get the best hand
		if( game->use_best_for_undefined_case) {
			get_the_best_hand(game, cur,1);
			return;
		}

	}
	if( game->known_others_poker)
		return   farmer_select_one_combo_for_other_farmer(game,curPlayer,cur,0);

	COMBO_OF_POKERS* t= farm_select_combo_in_suit(game);
	*cur=*t;
}

void  downfarmer_play_first_when_upfarmer_has_only_1_pokers(GAME* game, COMBO_OF_POKERS* cur)
{
	FUNC_NAME
		PLAYER*player =game->players[CUR_PLAYER];
	PLAYER* upfm= 	game->players[DOWN_PLAYER];
	PLAYER* lord = game->players[UP_PLAYER];

	if(!game->known_others_poker)
	{
		// todo: add logic for bomb...
		for (int i=player->h->begin; i<=player->h->end; i++)
		{
			if (player->h->hands[i]>0)
			{
				cur->type=SINGLE;
				cur->low = i ;
				return;
			}
		}
	}
	else
	{
		//search bomb in player
		// play the smallest Bomb which
		if( getBomb(player->h, cur)) {
			do
			{
				if(!has_combo_bigger_than_c_in_hands(lord->h, cur))
				{
					//double check if there's still another poker less than down's
					for (int i=player->h->begin; i<=player->h->end; i++)
					{
						if( i!=cur->low && player->h->hands[i]>0 && i<upfm->h->begin)
							return;
					}
				}
			} while(getBigBomb(player->h, cur,cur));

		}
		for (int i=player->h->begin; i<=player->h->end; i++)
		{
			if (player->h->hands[i]>0)
			{
				cur->type=SINGLE;
				cur->low = i ;
				return;
			}
		}
	}
}

void  downfarmer_play_first_when_upfarmer_has_only_2_pokers(GAME* game, COMBO_OF_POKERS* cur)
{
	FUNC_NAME
		PLAYER*player =game->players[CUR_PLAYER];
	PLAYER* upfm= 	game->players[DOWN_PLAYER];
	PLAYER* lord = game->players[UP_PLAYER];

	//search bomb in player
	if( getBomb(player->h, cur)) {
		do                   	   	    {

			{
				if(!has_combo_bigger_than_c_in_hands(lord->h, cur)) // bomb is the biggest
				{
					if(upfm->h->begin == upfm->h->end) //it's a pair
						//double check if there's still another poker less than down's
						for (int i=player->h->begin; i<=player->h->end; i++)
						{
							if( i!=cur->low && player->h->hands[i]>1 && i<upfm->h->begin)
								return;
						}
				}
				else
				{
					int bigger =0, first_min=0;
					for (int i=player->h->begin; i<=player->h->end; i++)
					{
						if( i!=cur->low && player->h->hands[i]>0) {
							if( i<upfm->h->begin && bigger ==0 ) {
								bigger+=player->h->hands[i];
								first_min = i;
							}
							if( i<upfm->h->end && bigger >0 ) {
								bigger+=player->h->hands[i];
							}
							if(bigger>=2)
							{
								cur->type= SINGLE;
								cur->low = first_min;
								return;
							}
						}
					}
				}
			}
		} while(getBigBomb(player->h, cur,cur));

	}
	if(upfm->h->begin == upfm->h->end) //it's a pair
	{
		//double check if there's still another poker less than down's
		for (int i=player->h->begin; i<=player->h->end; i++)
		{
			if(  player->h->hands[i]>1 && i<upfm->h->begin) {
				cur->type= PAIRS;
				cur->low = i;
				return;
			}
		}
	}

	if (player->good_framer)
	{
		downfarmer_good_play_first(game,cur);
	}
	else //bad farmer
	{
		downfarmer_bad_play_first(game,cur);
	}

}

void downfarmer_play_first(GAME*game,COMBO_OF_POKERS* cur)
{
	PLAYER*player =game->players[CUR_PLAYER];
	PLAYER* upfm= 	game->players[DOWN_PLAYER];
	PLAYER* lord = game->players[UP_PLAYER];
	int is_good_farmer = player->good_framer;

	/*如果下家报单，则优先出最小的单牌*/
	if (player->oppDown_num ==1
		&&	(  game->known_others_poker==0
		||player->h->begin <upfm->h->begin ))
		return downfarmer_play_first_when_upfarmer_has_only_1_pokers(game,cur);

	// up farmer only have two cards
	if (player->oppDown_num ==2
		&&	(  game->known_others_poker==1))
		return  downfarmer_play_first_when_upfarmer_has_only_2_pokers(game,cur);

	COMBO_OF_POKERS * c = get_combo_in_a_win_suit(game);
	if(c!=NULL)
	{
		int good=1;
		if ( player->oppUp_num == 1 && c->type ==SINGLE 
			&& c->low < player->opps->end )
			good=0;
		if ( player->oppUp_num == 2 && c->type ==PAIRS
			&& !is_combo_biggest_sure(game,c))
		{
			good=0;
		}
		if(good){
			*cur=*c;
			return;
		}
	} 

	if ( player->oppUp_num == 1)
	{
		farmer_play_first_lord_has_1_poker_only(game,cur,is_good_farmer,player,0);
	}
	else
	{
		if(game->known_others_poker && lord->h->total == 2 )
		{
			if(game->use_best_for_undefined_case)
			{
				get_the_best_hand( game,cur,1);
				if(cur->type==PAIRS && lord->summary->real_total_num ==1 
					&& lord->h->begin > cur->low)
				{
				}
				else
					return;
			}
		}
		/**/
		if(player->oppUp_num==2 && player->oppDown_num !=2)
		{
			//check opps has pairs?
			int haspairs=0;
			for(int i=0;i<=P2;i++)
			{
				if(player->opps->hands[i]>=2)
				{
					haspairs=1;
					break;
				}
			}
			if(haspairs==1)
			{
				if(player->summary->pairs_num>0 &&
					(!is_combo_biggest_sure(game,player->summary->pairs[0])))
				{ 
					if (player->summary->singles_num>0)
					{
						int found=0;
						for(int i=0;i<player->summary->singles_num;i++)
							if(player->summary->singles[i]->low<
								player->opps->end)
							{
								*cur=*player->summary->singles[i];
								found=1;
							}
							if (found &&player->h->end>player->opps->end) return;
					}	         
					else if(player->summary->pairs_num>1 
						&& player->summary->pairs_num == player->summary->real_total_num)
					{
						//check the second pairs is the biggest?
						if(!is_combo_biggest_sure(game,player->summary->pairs[1]))
						{ //拆对!
							cur->low=player->summary->pairs[0]->low;
							cur->type=SINGLE;
							return;
						}
						else
						{
							*cur=*player->summary->pairs[1];
							return;	            
						}
					}
				}
			}
		}

		if(check_only_2_same_combo(game,cur))
		{
			return;
		}
__re_run:	 

		if (is_good_farmer)
		{
			downfarmer_good_play_first(game,cur);
		}
		else //bad farmer
		{
			downfarmer_bad_play_first(game,cur);
		}

	}
	//double check again
	if(!game->known_others_poker
		&& cur->type == PAIRS && player->oppUp_num ==2 && player->oppDown_num !=2 && player->h->total!=2)
	{
		int haspairs=0;
		for(int i=cur->low+1;i<=P2;i++)
		{
			if(player->opps->hands[i]>=2)
			{
				haspairs++;
			}
		}
		if(haspairs>1|| cur->low<player->opps->begin)
		{ 
			cur->type =SINGLE;	      
		}
	}
}

//find and remove a joker in combos, 31 3311... 411
int find_joker_in_combos(PLAYER* player, int joker)
{
	if (player->summary->singles_num>0 &&  player->summary->singles[player->summary->singles_num-1]->type==joker )
	{
		player->summary->singles_num--;
		player->summary->singles[player->summary->singles_num]->type=NOTHING;
	}
	else if (player->summary->three_one_num>1 && player->summary->three_one[player->summary->three_one_num-1]->three_desc[0] == joker)
	{
		player->summary->three_one_num--;
		player->summary->three[player->summary->three_num++]=player->summary->three_one[player->summary->three_one_num];
		player->summary->three_one[player->summary->three_one_num]->type = THREE;
	}
	else {
		return 0;
	}

	return 1;
}

int must_play_for_poker2(GAME * game,COMBO_OF_POKERS * cur, PLAYER* player,COMBO_OF_POKERS *pre, int check_opps_lit_joker)
{
	int	pass=1;
	if ( player->h->hands[LIT_JOKER]==1 )
	{
		cur->type=SINGLE;
		cur->low = LIT_JOKER;
		cur->len = 1;
		pass =0;
		if ( player->summary->bomb_num>0 && player->summary->bomb[player->summary->bomb_num-1]->low==LIT_JOKER)
		{
			if(player->good_framer  || player->id == DOWNFARMER ) //不拆大小王
				return 1;
			player->summary->bomb[player->summary->bomb_num-1]->type=SINGLE;
			player->summary->bomb[player->summary->bomb_num-1]->low=BIG_JOKER;
			player->summary->bomb[player->summary->bomb_num-1]->len=1;
			player->summary->singles_num++;
			player->summary->singles[player->summary->singles_num-1]=player->summary->bomb[player->summary->bomb_num-1];
			player->summary->bomb_num--;
		}
		else {
			//find joker in 31 3311.. 411..
			if ( !find_joker_in_combos(player,LIT_JOKER))
			{
				rearrange_suit(game,player,cur);
			}
		}
		//pass = 0;
	}
	else if ( ( player->h->hands[BIG_JOKER]==1 )
		&&(
		player->opps->hands[LIT_JOKER] == 0
		|| !check_opps_lit_joker
		)// Lit joker not seen, do not play for big_joker
		)
	{

		cur->type=SINGLE;
		cur->low = BIG_JOKER;
		cur->len = 1;
		pass =0;
		if ( !find_joker_in_combos(player,BIG_JOKER))
		{
			rearrange_suit(game,player,cur);
		}
	}
	else
		pass =1;
	return pass;
}

void update_players(GAME * game,COMBO_OF_POKERS * cur)
{
	if(cur!=NULL)
		remove_combo_poker(&game->all, cur,NULL);
	int ctrl_num = 	CONTROL_POKER_NUM;
	if(game->all.total <=20 )
		ctrl_num = 4;
	else if(game->all.total <=10 )
		ctrl_num = 2;

	// player->lower_control_poker=
	game->players[UP_PLAYER]->lower_control_poker =
		game->players[DOWN_PLAYER]->lower_control_poker =
		game->players[CUR_PLAYER]->lower_control_poker
		= get_lowest_controls(&game->all, ctrl_num);

	for(int i=2; i>0; i--) {
		game->players[i]->summary->ctrl.single=
			calc_controls(game->players[i]->h, game->players[i]->opps, ctrl_num);
		update_summary(game,game->players[i]->summary,game->players[i]->h,
			game->players[i]->opps,game->players[i]->combos,20,
			game->players[i]->oppDown_num,game->players[i]->oppUp_num,
			game->players[i]->lower_control_poker,game->players[i]);
	}
}

int is_upfarmer_must_play_bigger(GAME* game,int is_good_farmer,COMBO_OF_POKERS* pre)
{
	if(game->player_type == UPFARMER )
	{
		if(is_good_farmer)
		{
			if ( PRE_PLAYER(game) == LORD //pre player is lord.
				&&
				( (pre->type >= 3311) //飞机
				||(pre->type == PAIRS_SERIES)
				||(pre->type == SINGLE_SERIES && pre->len>8 )
				//||( pre->type == SINGLE )
				|| (pre->type == SINGLE && 
				pre->low<game->players[CUR_PLAYER]->lower_control_poker)

				) 
				)
			{   //必管..
				return true;
			}
		}
		else  //bad
		{
			if ( PRE_PLAYER(game)  == LORD
				&&
				( (pre->type!=SINGLE  && pre->type!=PAIRS )
				|| (pre->type == SINGLE && 
				pre->low<game->players[CUR_PLAYER]->lower_control_poker)
				//|| (pre->type == PAIRS && pre->low<game->lowest_bigPoker-2)
				)
				)
			{
				return true;
			}

		}
	}
	return false;
}

int upfarmer_play_normal(GAME *game, COMBO_OF_POKERS* cur,COMBO_OF_POKERS* pre)
{
	return farmer_play_normal(game, cur, pre);
}

int upfarmer_make_a_bigger_for_single_and_pair(GAME *game,COMBO_OF_POKERS* pre, COMBO_OF_POKERS* cur,int is_good)
{
	FUNC_NAME
		int pass = 1;
	PLAYER *player = game->players[CUR_PLAYER];

	int geying =  is_game_first_half(game,CUR_PLAYER)?  char_to_poker('Q'): char_to_poker('Q');

	if(game->known_others_poker && pre->type ==SINGLE)
	{
		//get the singles of lord..
		int single=geying;
		//play a bigger than more of the singles
		search_combos_in_suit(game,
			game->players[DOWN_PLAYER]->h,
			game->players[DOWN_PLAYER]->opps,
			game->players[DOWN_PLAYER]);
		COMBOS_SUMMARY * s = game->players[DOWN_PLAYER]->summary;

		for(int i=s->singles_num-1; i>=0; i--)
		{
			if(s->singles[i]->low <=char_to_poker('K') )
			{
				single  = s->singles[i]->low;
				break;
			}
		}
		if(s->singles_num>=3)
		{
			if(single > s->singles[2]->low)
				single= s->singles[2]->low;
		}
		geying= single;
	}

	if ( (pre->type == SINGLE ))
	{

		if (!is_good && pre->low < geying)
		{

			if ( game->players[game->pre_playernum]->id !=LORD  
				&& ( pre->low >= player->lower_control_poker|| pre->low >= geying ))
				return 1;

			pass = 1;
			for (int i=char_to_poker('A'); i>=geying; i-- )
			{
				if ( player->h->hands[i]==1)
				{
					cur->type=SINGLE;
					cur->low =i;
					cur->len = 1;
					DBG( PRINTF(LEVEL,"!Bad Up Farmer Gepai\n"));
					pass = 0;
					break;
				}
			}
			if (pass == 1)
			{
				for (int i=char_to_poker('A'); i>=geying; i-- ) //寻找>X的格应区的对子，拆最大的一个，否则
				{
					if ( player->h->hands[i]==2)
					{
						cur->type=SINGLE;
						cur->low =i;
						cur->len = 1;
						player->h->hands[i]=1;
						player->need_research = 1;
						search_combos_in_suit(game,player->h,player->opps,player);
						player->h->hands[i]++;
						DBG( PRINTF(LEVEL,"!Bad Up Farmer Geying\n"));
						pass = 0;
						break;
					}
				}
			}
			if (pass==1)// && (game->pre_playernum == game->lord-1 )) //only for preplayer is lord..
			{
				//for (int i=max(game->lowest_bigPoker,pre->low+1); i<=BIG_JOKER; i++ )
				if(PRE_PLAYER(game)==LORD
					|| pre->low <= P10)
				{
					if ( player->h->hands[P2]>=1)
					{
						cur->type=SINGLE;
						cur->low =P2;
						cur->len = 1;
						DBG( PRINTF(LEVEL,"!Bad Up Farmer select a control poker\n"));
						pass = 0;
						//   break;
					}
				}
			}

			if (pass==1)
			{   //rarely here..
				for (int i=geying; i>=pre->low+1; i-- )
				{
					if ( player->h->hands[i]>=1 && player->h->hands[i]!=4)
					{
						cur->type=SINGLE;
						cur->low =i;
						player->h->hands[i]--;
						cur->len = 1;
						search_combos_in_suit(game,player->h,player->opps,player);
						player->need_research =1 ;
						player->h->hands[i]++;
						DBG( PRINTF(LEVEL,"!Bad Up Farmer select other single poker\n"));
						pass = 0;
						break;
					}
				}
			}
			return pass;
		}
		else if (!is_good && PRE_PLAYER(game)== LORD && pre->low >=geying && pre->low <= Pa )
		{
			//寻找最小的能管住X的牌。如果不存在或为大小王，则过牌，否则打出
			pass = 1;
			for (int i=pre->low+1; i<=P2; i++ )
			{
				if ( player->h->hands[i]>=1)
				{
					cur->type=SINGLE;
					cur->low =i;
					cur->len = 1;
					player->h->hands[i]--;
					search_combos_in_suit(game,player->h,player->opps,player);
					player->need_research =1 ;
					player->h->hands[i]++;
					DBG( PRINTF(LEVEL,"!Bad Up Farmer select other single poker\n"));
					pass = 0;
					break;
				}
			}
		}
		else
		{
			pass =1;
			if(  pre->low < geying)
			{
				//find in pairs..
				//()
				COMBOS_SUMMARY *s = player->summary;
				if (s->singles_num>0)
				{
					for (int k=0; k<s->singles_num; k++)
					{
						if ( s->singles[k]->low > pre->low  && s->singles[k]->low >=geying
							&&s->singles[k]->low <=
							player->lower_control_poker) {
								memmove(cur, s->singles[k],sizeof(COMBO_OF_POKERS));
								return 0;
						}
					}
				}

				//todo: 拆牌

				/*
				if (s->pairs_num>0)
				{
				for (int k=0; k<s->pairs_num; k++)
				{
				if ( s->pairs[k]->low > pre->low  && s->singles[k]->low >=geying ) {

				{
				cur->type=SINGLE;
				cur->low =k;
				cur->len = 1;
				player->h->hands[k]-=1;
				player->need_research = 1;
				search_combos_in_suit(game,player->h,player->opps,player);
				player->h->hands[i]++;
				DBG( PRINTF(LEVEL,"!Good Up Farmer GePai\n"));
				pass = 0;
				break;
				}
				return s->pairs[k];
				}
				}
				}
				//     */
			}
			if(pass==1 && PRE_PLAYER(game)== LORD)
				pass=upfarmer_play_normal(game,cur,pre);
		}
	}
	else
	{
		if (!is_good && PRE_PLAYER(game) == LORD  )
			pass = must_play_bigger(cur, player,pre,game,1,0,1);
		else if(!is_good) {
			//todo
			if ( PRE_PLAYER(game)!=LORD )
			{
				if(pre->low >= P10)
					pass =1;
				else
					pass=upfarmer_play_normal(game,cur,pre);
			}
			if(pass==0)
			{//double check
				if(cur->low>=P10)
					pass =1;
			}
		}
		else
		{
			if ( PRE_PLAYER(game)!=LORD )
			{
				if(pre->low >= P10)
					pass =1;
				else
					pass=upfarmer_play_normal(game,cur,pre);
			}
			else
				pass=upfarmer_play_normal(game,cur,pre);
		}
	}
	return pass;
}

int downfarmer_play_when_upfarmer_has_only_1_poker
	(GAME* game, COMBO_OF_POKERS * cur, COMBO_OF_POKERS * pre)
{
	FUNC_NAME
		PLAYER *player=game->players[CUR_PLAYER];
	PLAYER * lord= game->players[UP_PLAYER];
	for (int i=player->h->begin; i<=player->h->end; i++)
	{
		if (player->h->hands[i] == 4) //bomb got
		{
			cur->type = BOMB;
			cur->low = i;
			if (is_combo_biggest_sure(game,cur)
				&& check_combo_a_Big_than_b(cur,pre))
				return 0;
		}
	}

	if(HAS_ROCKET(player->h))
	{
		cur->type = ROCKET;
		cur->low = LIT_JOKER;
		return 0;
	}

	if( PRE_PLAYER(game)!=LORD)
		return 1;
	else if ( pre->type==SINGLE && (pre->low < player->opps->begin ||
		(game->known_others_poker && pre->low < game->players[DOWN_PLAYER]->h->begin)))
	{
		return 1;
	}
	else
	{
		if (find_the_biggest_combo_great_pre(game,pre,cur))
		{
			COMBO_OF_POKERS t;
			if(!find_a_bigger_combo_in_hands(lord->h, &t,cur))
				return 0;
		}

		if(game->use_best_for_undefined_case)
			return  get_the_best_hand(game,cur, 0);
		else //play the biggest combo
		{
			if (find_the_biggest_combo_great_pre(game,pre,cur))
				return 0;
			else
				return 1;
		}
	}
}

int downfarmer_play_when_upfarmer_has_only_1_combo
	(GAME* game, COMBO_OF_POKERS * cur, COMBO_OF_POKERS * pre)
{
	FUNC_NAME
		PLAYER *player=game->players[CUR_PLAYER];
	PLAYER *upfm=game->players[DOWN_PLAYER];
	PLAYER * lord= game->players[UP_PLAYER];
	COMBO_OF_POKERS * upfm_only_combo;
	//get the only combo of upfarmer
	if(upfm->summary->not_biggest_num>0)
		upfm_only_combo = upfm->summary->not_biggest[0];
	else
		upfm_only_combo = upfm->summary->biggest[0];

	if(!find_a_smaller_combo_in_hands(player->h, cur,upfm_only_combo))
		return 1;

	if(upfm->h->total >2 ) //break for now
		return 1;

	// for a pair...
	for (int i=player->h->begin; i<=player->h->end; i++)
	{
		if (player->h->hands[i] == 4) //bomb got
		{
			cur->type = BOMB;
			cur->low = i;
			if (is_combo_biggest_sure(game,cur)
				&& check_combo_a_Big_than_b(cur,pre))
			{
				POKERS t;
				t=*player->h;
				t.hands[i]=0;
				COMBO_OF_POKERS c;
				if(find_a_smaller_combo_in_hands(&t, &c, upfm_only_combo))
					return 0;
			}
		}
	}

	if(HAS_ROCKET(player->h))
	{
		cur->type = ROCKET;
		cur->low = LIT_JOKER;
		COMBO_OF_POKERS c;
		if(find_a_smaller_combo_in_hands(player->h, &c, upfm_only_combo))
			return 0;
	}

	if( PRE_PLAYER(game)!=LORD)
		return 1;
	else if ( pre->type==upfm_only_combo->type && pre->low < game->players[DOWN_PLAYER]->h->begin)
	{
		return 1;
	}
	else
	{
		//todo: for sure
		/*
		if (find_the_biggest_combo_great_pre(game,pre,cur))
		{
		COMBO_OF_POKERS t;
		if(!find_a_bigger_combo_in_hands(lord->h, &t,cur))
		return 0;
		}
		*/
		if(game->use_best_for_undefined_case)
			return  get_the_best_hand(game,cur, 0);
		else //play the biggest combo
		{
			if (find_the_biggest_combo_great_pre(game,pre,cur))
				return 0;
			else
				return 1;
		}
	}
}



int downframer_play(GAME* game, COMBO_OF_POKERS * cur, COMBO_OF_POKERS *pre)
{
	FUNC_NAME
		PLAYER*player =game->players[CUR_PLAYER];
	PLAYER* upfm= game->players[DOWN_PLAYER];
	PLAYER* lord = game->players[UP_PLAYER];	
	int is_good_farmer = player->good_framer;
	int pass;
	COMBO_OF_POKERS* res;

	if( (res= check_for_win_now(game,pre))!=NULL ) //win immediately, play the biggest or bomb
	{
		remove_combo_in_suit(player->summary,res);
		memmove(cur,res,sizeof(COMBO_OF_POKERS));
		res->type = NOTHING;
		return 0;
	}
	// up farmer has only 1 poker
	if ( game->players[CUR_PLAYER]->oppDown_num == 1 )
	{
		if(game->known_others_poker  &&  (player->h->begin< game->players[DOWN_PLAYER]->h->begin ))
			return downfarmer_play_when_upfarmer_has_only_1_poker(game, cur,pre);
		else
		{
			if(PRE_PLAYER(game)==UPFARMER)
			{
				if(player->h->begin< game->players[DOWN_PLAYER]->h->begin )
				{
					for (int i=player->h->begin; i<=player->h->end; i++)
					{
						if (player->h->hands[i] == 4) //bomb got
						{
							cur->type = BOMB;
							cur->low = i;
							if (is_combo_biggest_sure(game,cur)
								&& check_combo_a_Big_than_b(cur,pre))
								return 0;
						}
					}
					if(HAS_ROCKET(player->h))
					{
						cur->type = ROCKET;
						cur->low = LIT_JOKER;
						return 0;
					}            	    	   
				}
				else		
					return 1;
			}
			else
			{
				if (find_the_biggest_combo_great_pre(game,pre,cur))
				{
					POKERS t=*player->h;
					remove_combo_poker(&t,cur,NULL);
					if(player->h->begin<player->opps->end )
					{
						if(pre->type!=SINGLE)
							return 0;
						else if(pre->type==SINGLE)
						{
							if( is_combo_biggest_sure(game,cur))
							{
								return 0;
							}
							else if(pre->low >=player->opps->end)
							{
								return 0;
							}
							else if(pre->low <player->opps->begin && NOT_BOMB(cur))
							{   
								return 1;
							}
							return 1;
						}
						return 0;
					}  
				}
				else
					return 1;
			}
		}
	}

	// up farmer has only 2 poker
	if (game->known_others_poker &&  game->players[CUR_PLAYER]->oppDown_num == 2
		&& game->players[DOWN_PLAYER]->summary->real_total_num == 1
		&&  (player->h->begin<game->players[DOWN_PLAYER]->h->begin ))
	{
		if(game->known_others_poker) {
			return downfarmer_play_when_upfarmer_has_only_1_combo(game, cur,pre);
		}
	}


	//pre player is lord.)
	if ( ( game->players[CUR_PLAYER]->oppUp_num == 1) )
	{
		pass= farmer_play_when_lord_has_1_poker_only(cur,pre,player,game,0,is_good_farmer);
		return pass;
	}//pre player is lord.)
	if( player->summary->real_total_num<=2 || 
		player->summary->combo_total_num<=1)	
	{
		if(game->use_best_for_undefined_case)
			return get_the_best_hand(game,cur,0);
	}//pre player is lord.)

	//biguan
	if (game->known_others_poker && PRE_PLAYER(game) == LORD
		&& ( (lord->summary->combo_total_num<=1 && player->oppUp_num <=10)
		/*||player->oppDown_num<=2*/	   
		|| ( (lord->summary->combo_total_num<=2 && player->oppUp_num <=10) 
		&& !find_a_bigger_combo_in_hands (upfm->h,cur, pre) )
		)	  
		)
	{

		COMBO_OF_POKERS *c;

		if((c=find_combo(player->summary,pre))!=NULL &&c->type!=BOMB && c->type!=ROCKET )
		{
			*cur=*c;
			return 0;
		}	   

		if ((c=find_combo(upfm->summary,pre))!=NULL &&c->type!=BOMB && c->type!=ROCKET )
		{
			return 1;
		}	   
		pass = must_play_bigger_to_stop_lord(cur, player,pre,game,1,is_good_farmer,1);
		return pass;
	}


	{
		if (is_good_farmer)
		{
			if (PRE_PLAYER(game)==LORD//pre player is lord.
				&&
				( (pre->type >= 3311) //飞机
				||(pre->type == PAIRS_SERIES)
				||(pre->type == SINGLE_SERIES && pre->len>8 )
				) //lord has only one poker
				)
			{
				if( game->known_others_poker )
				{
					if(!get_num_of_bigger(player->combos, MAX_COMBO_NUM,pre))
					{
						if( get_num_of_bigger(upfm->combos, MAX_COMBO_NUM,pre))
						{
							return 1;
						}
					}
				}
				//必管..
				pass = must_play_bigger(cur, player,pre,game,0,is_good_farmer,1);
			}
			else if (PRE_PLAYER(game)== LORD //pre player is lord.
				&& (pre->type == SINGLE && pre->low>=P2 && pre->low<BIG_JOKER))
			{
				pass=must_play_for_poker2(game,cur, player,pre,1);
				if ( pass ==1)
					pass=farmer_play_normal(game,cur,pre);
			}
			else
			{
				if (game->players[game->pre_playernum]->id != LORD   &&(
					has_control_poker(pre, player->lower_control_poker) >0
					|| game->players[CUR_PLAYER]->oppDown_num <=4 
					|| get_combo_number(pre)>=4
					|| (pre->low>=P9 && get_combo_number(pre)>=2) 
					|| pre->low>=Pj)
					)
				{
					if(game->known_others_poker)
					{

					}
					pass =1;
				}
				else{
					pass = farmer_play_normal(game,cur,pre);
					if(PRE_PLAYER(game)!=LORD &&
						((pre->low>=P9 && get_combo_number(pre)>=2) 
						|| pre->low>=Pk) 
						)
						pass=1;
				}
			}
		}
		else //bad down farmer
		{
			if ( (game->players[game->pre_playernum]->id == LORD  ) //pre player is lord.
				&& ( get_combo_number(pre)>=3 ) )
			{   //必管..
				pass=must_play_bigger(cur,player,pre,game,0,is_good_farmer,1);
			}
			else if ((game->players[game->pre_playernum]->id == LORD  ) //pre player is lord.
				&& (pre->type == SINGLE && pre->low>=P2 && pre->low<BIG_JOKER))
			{
				pass=must_play_for_poker2(game,cur,player,pre,0);
			}
			else if ((game->players[game->pre_playernum]->id == LORD  ) //pre player is lord. //pre's type is single or pair
				)
			{
				if (!is_game_first_half(game,CUR_PLAYER) &&pre->type != SINGLE && pre->type != PAIRS)
					pass=must_play_bigger(cur,player,pre,game,0,is_good_farmer,1);
				else
					pass=farmer_play_normal(game,cur,pre);
			}
			else //from the other farmer.
				pass = 1;
		}
	}
	return pass;
}

//play the biggest for win


int upframer_play(GAME* game, COMBO_OF_POKERS * cur, COMBO_OF_POKERS *pre)
{
	PLAYER*player =game->players[CUR_PLAYER];
	PLAYER* lord = game->players[DOWN_PLAYER];

	int is_good_farmer = player->good_framer;
	int pass;
	COMBO_OF_POKERS *res;
	if( (res= check_for_win_now(game,pre))!=NULL ) //win immediately, play the biggest or bomb
	{
		remove_combo_in_suit(player->summary,res);
		memmove(cur,res,sizeof(COMBO_OF_POKERS));
		res->type = NOTHING;
		return 0;
	}

	if ( game->players[CUR_PLAYER]->oppDown_num == 1  )
	{
		pass= farmer_play_when_lord_has_1_poker_only(cur,pre,player,game,1,is_good_farmer);
		return pass;
	}

	//是否需要上手
	if (game->known_others_poker && PRE_PLAYER(game) == LORD
		&& ((lord->summary->combo_total_num<=1 && player->oppDown_num <=10))
		)
		/*||player->oppDown_num<=2*/
	{
		pass = must_play_bigger_to_stop_lord(cur, player,pre,game,1,is_good_farmer,1);
		return pass;
	}

	if( player->summary->real_total_num<=2 || 
		player->summary->combo_total_num<=1)	
	{
		if(game->use_best_for_undefined_case)
			return get_the_best_hand(game,cur,0);
	}
	// biguan, pre player is LORD...
	if(is_upfarmer_must_play_bigger(game,is_good_farmer,pre))
	{
		pass = must_play_bigger(cur, player,pre,game,1,is_good_farmer,1);
		if(player->oppDown_num >=10 && !pass)
		{
			if ( get_control_poker_num_in_combo	(cur,player->lower_control_poker)>=2 )//&& c[k].type!=PAIRS)
			{
				if(pre->low+2>=cur->low){}
				else
					pass=1;
			}
		}
		return pass;
	}

	if (PRE_PLAYER(game) == LORD //pre player is lord.
		&& (pre->type == SINGLE && pre->low==P2))
	{
		pass=must_play_for_poker2(game,cur, player,pre,1);
		if ( pass ==1)
			pass=upfarmer_play_normal(game,cur,pre);
		return pass;
	}

	if (PRE_PLAYER(game) == LORD //pre player is lord.
		&& (pre->type == SINGLE && pre->low==LIT_JOKER) && player->h->hands[BIG_JOKER]==1)
	{
		cur->type=SINGLE;
		cur->low=BIG_JOKER;
		return 0;		
	}
	// pre is down_farmer
	if (PRE_PLAYER(game)!=LORD)
	{
		// check downfarmer is good or not...
		if(game->known_others_poker)
		{
			//todo :
		}
		if(has_control_poker(pre, player->lower_control_poker) >0
			//|| game->players[CUR_PLAYER]->oppUp_num<=4
			|| get_combo_number(pre)>=4 )
			return 1;
	}

	if(pre->type==SINGLE || pre->type == PAIRS)
	{
		pass=upfarmer_make_a_bigger_for_single_and_pair(game,pre,cur,is_good_farmer);
		return pass;
	}

	if( !player->good_framer && PRE_PLAYER(game)!=LORD)
	{
		return  1;
	}
	///*
	if(PRE_PLAYER(game) ==LORD) //must play again..
	{
		int prob=rand()%100;
		int play_prob = player->good_framer?40:80;
		if(player->oppDown_num <=4 ||
			(player->oppDown_num <=6 &&prob<play_prob+20)||
			prob<play_prob)
		{
			pass = must_play_bigger(cur, player,pre,game,1,is_good_farmer,1);
			return pass;			 	
		}
	}
	// */
	return upfarmer_play_normal(game,cur,pre);
}

int check_farmer_is_good(GAME* game)
{

	PLAYER * curPlayer= game->players[CUR_PLAYER];
	PLAYER * upPl= game->players[UP_PLAYER];
	PLAYER * dnPl= game->players[DOWN_PLAYER];

	COMBOS_SUMMARY * s= curPlayer->summary;
	int isup= game->player_type == UPFARMER ;
	int is_first_half_game = is_game_first_half(game,CUR_PLAYER);
	int is_good_farmer = CONTROL_SUB_COMBO_NUM(curPlayer)>=
		(is_first_half_game? -50: isup?-20:-30);

	if ( ( game->players[CUR_PLAYER]->oppUp_num == 1 &&  game->players[UP_PLAYER]->id  == LORD )
		||( game->players[CUR_PLAYER]->oppDown_num == 1 && game->players[DOWN_PLAYER]->id  == LORD ) )
	{   //refine for lords only has one poker..
		if (s->singles_num <= 1 || s->singles[1]->low >= curPlayer->opps->end)
			is_good_farmer = 1;
		else {
			search_combos_in_suits_for_min_single_farmer(game,curPlayer->h,curPlayer->combos,curPlayer); //should done in other place, since you should know it..
			if (s->singles_num <= 1 || s->singles[1]->low >= curPlayer->opps->end)
				is_good_farmer = 1;
			else
				is_good_farmer = 0;
		}
	}

	//double check
	if(isup&& !is_good_farmer)
	{
		if(curPlayer->summary->combo_total_num < upPl->summary->combo_total_num
			|| curPlayer->summary->combo_total_num <=3
			|| curPlayer->summary->combo_total_num<dnPl->summary->combo_total_num+1)
		{
			is_good_farmer = 1;
		}
	}
	else if(!isup&& !is_good_farmer)
	{
		if(curPlayer->summary->combo_total_num < dnPl->summary->combo_total_num
			|| curPlayer->summary->combo_total_num <=3
			|| curPlayer->summary->combo_total_num<upPl->summary->combo_total_num+1)
		{
			is_good_farmer = 1;
		}
	}

	curPlayer->good_framer = is_good_farmer;
	return is_good_farmer;
}



//void DUMP_GAME_FOR_PLAYER(LordRobot * r);

void update_copied_game(GAME* game,GAME * ref_g)
{
	game->pot = &game->POT;
	game->computer[0] = 1;
	int j;
	{

		for (j=0; j<3; j++)
		{
			game->suits[j].h = &game->suits[j].p;
			game->suits[j].opps =&game->suits[j].opp;
			game->suits[j].combos =&game->suits[j].combos_store[0];
			game->suits[j].summary =&game->suits[j].combos_summary;
			PLAYER * pl = &game->suits[j];
			sort_all_combos(game, pl->combos_store, max_POKER_NUM,
				pl->summary, pl->opps, pl->oppUp_num, pl->oppDown_num,
				pl->lower_control_poker, pl);
		}
	}
	for (j=0; j<3; j++){
		int i;
		for(i=0;i<3;i++){
			if(ref_g->players[j] == &ref_g->suits[i]){
				game->players[j]= &game->suits[i];
			}	  	
		}
	}
	game->rec.now =0; //clear record

}

void Robot_copy(LordRobot *dst,LordRobot *src)
{
	memcpy(dst,src,sizeof(LordRobot));
	// update game
	GAME *game=&dst->game;
	update_copied_game(game,&src->game);
}

void GAME_copy(GAME *dst,GAME *src)
{
	memcpy(dst,src,sizeof(GAME));
	// update game
	update_copied_game(dst,src);
}

//for internal use
void convert(int * outs, int * out1,int *out2)
{
	if(*outs==-1) //no output
	{
		*out1 = -1;
		*out2 = -1;
		return;
	}
	int hun_num = *outs++;
	if(hun_num >4 ||hun_num<0)
	{
		PRINTF_ALWAYS("sucks in line %d\n",__LINE__);
	}
	do
	{
		*out1++=*outs++;
	}
	while(*outs!=-1);
	out1[-hun_num*2]=-1;
	outs-=hun_num*2;
	// outs--;
	for(int i=0;i<hun_num;i++)
	{
		*out2++=*outs++;
	}
	*out2=-1;
}


//for internal use
void convert_to_new_out_format(int * outs, int * out1,int *out2)
{
	if(*outs==-1) //no output
	{
		*out1 = -1;
		*out2 = -1;
		return;
	}
	int hun_num = *outs++;
	if(hun_num >4 ||hun_num<0)
	{
		PRINTF_ALWAYS("sucks in line %d\n",__LINE__);
	}
	do
	{
		*out1++=*outs++;
	}
	while(*outs!=-1);
	//   out1[-hun_num]=-1;
	outs-=hun_num*2;
	out1-=hun_num*2;

	// outs--;
	for(int i=0;i<hun_num;i++)
	{
		*out2++=*outs++;
	}
	for(int i=0;i<hun_num;i++)
	{
		*out1++=*outs++;
	}
	*out2=-1;
	*out1=-1;
}

//todo: fix for 1,1,1
int Check_win_quick(GAME * gm,int firstplayer_num,BOOL cur, BOOL up, BOOL down)	//need optimization
{

	PRINTF(LEVEL,"\n===========BEGIN CHECK WIN===============\n");
	FILE * bak_logfile2 = logfile2;
	logfile2 = NULL;
#if 1
	LordRobot* robot[3];
	robot[0]=createRobot(1,0);
	robot[1]=createRobot(1,0);
	robot[2]=createRobot(1,0);
	int out1[26],out2[5];

	GAME_copy(&robot[0]->game,gm);
	GAME_copy(&robot[1]->game,gm);
	GAME_copy(&robot[2]->game,gm);


	PLAYER* tmp;

	/*
	player1's view
	CUR:

	*/
	/*
	search_combos_in_suit(&robot[0]->game, robot[0]->game.players[CUR_PLAYER] ->h,
	robot[0]->game.players[CUR_PLAYER] ->opps, robot[0]->game.players[CUR_PLAYER] );
	search_combos_in_suit(&robot[0]->game, robot[0]->game.players[DOWN_PLAYER] ->h,
	robot[0]->game.players[DOWN_PLAYER] ->opps, robot[0]->game.players[DOWN_PLAYER] );
	search_combos_in_suit(&robot[0]->game, robot[0]->game.players[UP_PLAYER] ->h,
	robot[0]->game.players[UP_PLAYER] ->opps, robot[0]->game.players[UP_PLAYER] );
	*/

	//update for robot[1]
	tmp = robot[1]->game.players[CUR_PLAYER];
	robot[1]->game.players[CUR_PLAYER] = robot[1]->game.players[DOWN_PLAYER];
	robot[1]->game.players[DOWN_PLAYER] = robot[1]->game.players[UP_PLAYER];
	robot[1]->game.players[UP_PLAYER] = tmp;
	robot[1]->game.player_type = robot[1]->game.players[CUR_PLAYER]->id;
	if(robot[0]->game.pre_playernum == CUR_PLAYER) //now always it is
	{
		robot[1]->game.pre_playernum = UP_PLAYER;
		robot[2]->game.pre_playernum = DOWN_PLAYER;
	}
	else if (robot[0]->game.pre_playernum == DOWN_PLAYER) //now always it is
	{
		robot[1]->game.pre_playernum = CUR_PLAYER;
		robot[2]->game.pre_playernum = UP_PLAYER;
	}
	else if (robot[0]->game.pre_playernum == UP_PLAYER) //now always it is
	{
		robot[1]->game.pre_playernum = DOWN_PLAYER;
		robot[2]->game.pre_playernum = CUR_PLAYER;
	}
	/*
	search_combos_in_suit(&robot[1]->game, robot[1]->game.players[CUR_PLAYER] ->h,
	robot[1]->game.players[CUR_PLAYER] ->opps, robot[1]->game.players[CUR_PLAYER] );
	search_combos_in_suit(&robot[1]->game, robot[1]->game.players[DOWN_PLAYER] ->h,
	robot[1]->game.players[DOWN_PLAYER] ->opps, robot[1]->game.players[DOWN_PLAYER] );
	search_combos_in_suit(&robot[1]->game, robot[1]->game.players[UP_PLAYER] ->h,
	robot[1]->game.players[UP_PLAYER] ->opps, robot[1]->game.players[UP_PLAYER] );
	*/

	//update for robot[2]
	tmp = robot[2]->game.players[CUR_PLAYER];
	robot[2]->game.players[CUR_PLAYER] = robot[2]->game.players[UP_PLAYER];
	robot[2]->game.players[UP_PLAYER] = robot[2]->game.players[DOWN_PLAYER];
	robot[2]->game.players[DOWN_PLAYER] = tmp;

	robot[2]->game.player_type = robot[2]->game.players[CUR_PLAYER]->id;
	/*
	search_combos_in_suit(&robot[2]->game, robot[2]->game.players[CUR_PLAYER] ->h,
	robot[2]->game.players[CUR_PLAYER] ->opps, robot[2]->game.players[CUR_PLAYER] );
	search_combos_in_suit(&robot[2]->game, robot[2]->game.players[DOWN_PLAYER] ->h,
	robot[2]->game.players[DOWN_PLAYER] ->opps, robot[2]->game.players[DOWN_PLAYER] );
	search_combos_in_suit(&robot[2]->game, robot[2]->game.players[UP_PLAYER] ->h,
	robot[2]->game.players[UP_PLAYER] ->opps, robot[2]->game.players[UP_PLAYER] );

	*/
	robot[0]->game.players[CUR_PLAYER]->use_quick_win_check = cur;
	robot[1]->game.players[CUR_PLAYER]->use_quick_win_check = down;
	robot[2]->game.players[CUR_PLAYER]->use_quick_win_check = up;
	robot[0]->game.use_best_for_undefined_case = cur;
	robot[1]->game.use_best_for_undefined_case = down;
	robot[2]->game.use_best_for_undefined_case = up;

	switch (firstplayer_num)
	{
	default:
	case CUR_PLAYER:
		goto game_turn_p1;
	case DOWN_PLAYER:
		goto game_turn_p2;
	case UP_PLAYER:
		goto game_turn_p3;

	}

	int pass1, winner;

	while (1)
	{
game_turn_p1:
		//DUMP_GAME_FOR_PLAYER(robot[0]);
		//		DUMP_GAME_FOR_PLAYER(robot[1]);
		//		DUMP_GAME_FOR_PLAYER(robot[2]);
		pass1=takeOut(robot[0],out1,out2);
		getPlayerTakeOutCards(robot[1],out1,out2,1);
		getPlayerTakeOutCards(robot[2],out1,out2,0);
		winner=CUR_PLAYER;
		if(robot[0]->game.players[2]->h->total==0) //game over
			break;
game_turn_p2:

		//DUMP_GAME_FOR_PLAYER(robot[1]);
		pass1=takeOut(robot[1],out1,out2);
		getPlayerTakeOutCards(robot[2],out1,out2,1);
		getPlayerTakeOutCards(robot[0],out1,out2,0);
		winner=DOWN_PLAYER;
		if(robot[1]->game.players[2]->h->total==0) //game over
			break;
game_turn_p3:
		//DUMP_GAME_FOR_PLAYER(robot[2]);
		pass1=takeOut(robot[2],out1,out2);
		getPlayerTakeOutCards(robot[0],out1,out2,1);
		getPlayerTakeOutCards(robot[1],out1,out2,0);
		winner=UP_PLAYER;
		if(robot[2]->game.players[2]->h->total==0) //game over
			break;
		//goto turn_p1;
	}

	PRINTF(LEVEL,"winner is player %d\n",winner);
	PRINTF(LEVEL,"===========END   CHECK WIN===============\n\n");
	int res =0;
	if(winner==CUR_PLAYER ||
		(robot[0]->game.players[CUR_PLAYER]->id!=LORD
		&& robot[0]->game.players[winner ]->id!=LORD)
		)
		res=1;
	destoryRobot(robot[0]);
	destoryRobot(robot[1]);
	destoryRobot(robot[2]);
	logfile2 =bak_logfile2;
#endif
	return res;

}

typedef struct {
	PLAYER_TYPE id;//current player
	POKERS p;
} POKER_AND_TYPE;
typedef struct CNODE CNODE;
typedef struct CNODE {
	COMBO_OF_POKERS c;
	POKER_AND_TYPE *h;
	POKER_AND_TYPE *h1;
	POKER_AND_TYPE *h2;
	int value;
	int max_value;
	int prop;
	int child_nums;
	CNODE * farther;
	CNODE* childs;
} CNODE;


void generate_perhaps_nodes(CNODE * t)
{
	//if it is the first time for run..
	int first =0;
	if(t->c.type == NOTHING && (t->farther ==NULL || t->farther->c.type == NOTHING))
		first =1;
	if(first)
	{
		COMBO_OF_POKERS combos[100];
		int num=0;
		POKERS tmp;
		tmp = t->h1->p;
		num+=search_general_1(&tmp, combos, 0,0,0,0);
		//	  tmp = t->h1->p;
		//	 num+=search_general_2(&tmp, combos+num, 0,0,0,0);
		//	  tmp = t->h1->p;
		//	 num+=search_general_3(&tmp, combos+num, 0,0,0,0);
		int max_value = 1;
		//todo: add others and remove duplicated combos.
		PRINTF(LEVEL, "test combos %d: \n",num );
		print_all_combos(combos, num);
		t->childs = (CNODE*)malloc(sizeof(CNODE) * num );
		for(int i=0; i<num; i++)
		{
			CNODE * n = &t->childs[t->child_nums++];
			n->c = combos[i];
			n->max_value = max_value;
			n->value=0;
			n->farther = t;
			n->child_nums = 0;
			n->h = t->h1;
			n->h1 = t->h2;
			n->h2 = t->h;
			n->prop = 1;
		}
	}
	else
	{
		COMBO_OF_POKERS c;
		COMBO_OF_POKERS *p;
		if(t->c.type !=NOTHING) p=&t->c;
		else p=&t->farther->c;
		int	max_value =1;
		t->childs = (CNODE*)malloc(sizeof(CNODE) * 100);
		while ( find_a_bigger_combo_in_hands(&t->h1->p,&c,p) )
		{
			CNODE *n = &t->childs[t->child_nums++] ;
			n->c = c;
			n->max_value = max_value;
			n->value=0;
			n->farther = t;
			n->child_nums = 0;
			n->h = t->h1;
			n->h1 = t->h2;
			n->h2 = t->h;
			n->prop = 1;

			p= &n->c;
		}

		CNODE *n = &t->childs[t->child_nums++] ;
		n->c.type = NOTHING;
		n->max_value = max_value;
		n->value=0;
		n->farther = t;
		n->child_nums = 0;
		n->h = t->h1;
		n->h1 = t->h2;
		n->h2 = t->h;
		n->prop = 1;
		p= &n->c;
	}

}

int get_node_value(CNODE * t)
{
	register POKERS tmp;

	// /*
	PRINTF(LEVEL,"TESTing NODE VALUE: %d, type %d Combo ",t->value,t->h->id);
	if(t->c.type!=NOTHING)
		print_combo_poker(&t->c);
	PRINTF(LEVEL,"\n");
	print_hand_poker(&t->h->p);
	print_hand_poker(&t->h1->p);
	print_hand_poker(&t->h2->p);
	//*/
	if(t->c.type!=NOTHING) {
		combo_2_poker(&tmp, &t->c);
		sub_poker(&t->h->p,&tmp, &t->h->p);
	}
	int val;
	if( 0 == t->h->p.total ) //done
	{
		PRINTF(LEVEL,"DONE, %d win\n",t->h->id);
		t->value = 1;
		if(t->c.type == BOMB || t->c.type == ROCKET )
		{
			t->value =2;
		}
		val = t->value;
	}
	else
	{
		generate_perhaps_nodes(t);
		//for( int i=0;i<t->child_nums;i++)
		int loop = t->child_nums;//>2?2:t->child_nums;
		for( int i=0; i<loop; i++)
		{
			int value= get_node_value(&t->childs[i]);
			if( t->h->id !=  DOWNFARMER ) //child is UPFARMER
				value = -value;
			if(value >= t->max_value) //reach the best, exit
			{
				t->value =value;
				val = value;
				break;
			}
			t->value =value;
		}
	}
	//free child nodes
	// for( int i=0;i<t->child_nums;i++)
	{
		free(t->childs);
	}
	// PRINTF(LEVEL,"NODE VALUE: %d, type %d Combo ",t->value,t->h->id);
	//   if(t->c.type!=NOTHING)
	//  print_combo_poker(&t->c);
	//   PRINTF(LEVEL,"\n");
	if(t->c.type!=NOTHING)
		add_poker(&t->h->p,&tmp, &t->h->p);
	return t->value;

}

int get_the_best_hand_2(GAME *game,  COMBO_OF_POKERS * cur,bool first)
{
	//generate the top NODE;
	CNODE top;
	register POKER_AND_TYPE p[3];
	PLAYER *pl= game->players[CUR_PLAYER];
	p[0].p = *pl->h;
	p[0].id = pl->id;
	p[1].p= *game->players[DOWN_PLAYER]->h;
	p[1].id= game->players[DOWN_PLAYER]->id;
	p[2].p= *game->players[UP_PLAYER]->h;
	p[2].id= game->players[UP_PLAYER]->id;

	top.farther = NULL;
	top.h = &p[2];
	top.h1 = &p[0];
	top.h2 = &p[1];
	if(first)
	{

		top.c.type = NOTHING;
	}
	else
	{
		top.c = game->pre;
	}

	top.max_value =1; //todo: check if there's bomb in our side;

	generate_perhaps_nodes(&top);
	int value;
	int max_value,max_value_item=0;
	if (top.child_nums>1)
	{
		for( int i=0; i<top.child_nums; i++)
		{
			value= get_node_value(&top.childs[i]);
			if(value>=top.max_value)
				break;
		}
		//get the best one
		max_value=top.childs[0].value;
		for( int i=1; i<top.child_nums; i++)
		{
			if(top.childs[i].value > max_value)
			{
				max_value_item = i;
			}
		}
	}

	*cur = top.childs[max_value_item].c;

	//for( int i=0;i<top.child_nums;i++)
	{
		free(top.childs);
	}
	if(cur->type !=NOTHING)
	{
		return 0;
	}
	else
	{
		if(first ==1) PRINTF_ALWAYS("line %d, fatal error, must play some cards..\n", __LINE__);
		return 1;
	}
}


void check_card(POKERS *h,int * card)
{
	POKERS t;
	read_poker_int_2(card,max_POKER_NUM,&t);
	if(0!=cmp_poker(&t,h))
	{
		PRINTF_ALWAYS("card not match poker\n");	 
	}
}


typedef struct {
	COMBO_OF_POKERS c;
	int val;
} COMBO_VALUE;

int generate_perhaps_combos(GAME *game,COMBO_VALUE* cv,int first)
{
	PLAYER *player=game->players[CUR_PLAYER];
	int i=0;
	if(first)
	{
		//just use combos in summary..
		for(int j=0; j<(player->summary->not_biggest_num); j++)
		{
			if(player->summary->not_biggest[j]->type!=SINGLE && 
				player->summary->not_biggest[j]->type!=PAIRS  	)
			{
				cv[i].c=*player->summary->not_biggest[j];
				cv[i++].val=0;
			}
			/*
			if(player->combos_store[j].type!=NOTHING)
			{
			cv[i].c=player->combos_store[j];
			cv[i++].val=0;
			}
			*/
			//if(player->summary->not_biggest_num)
		}
		for(int j=0; j<(player->summary->biggest_num); j++)
		{
			if(player->summary->biggest[j]->type!=SINGLE && 
				player->summary->biggest[j]->type!=PAIRS  	)
			{
				cv[i].c=*player->summary->biggest[j];
				cv[i++].val=0;
			}
		}
		//todo: test other combossss??
		for(int j=player->h->begin; j<=player->h->end; j++)
		{
			if(player->h->hands[j]>0)
			{
				cv[i].c.type=SINGLE;
				cv[i].c.low = j;
				cv[i++].val=0;
			}		  
			if(player->h->hands[j]>1)
			{
				cv[i].c.type=PAIRS;
				cv[i].c.low = j;
				cv[i++].val=0;
			}		  		   
		}
	}
	else
	{

		COMBO_OF_POKERS tmp,*p =&tmp;
		p=&game->pre;
		while ((p=find_combo(player->summary, p))!=NULL)
		{
			cv[i].c=*p;
			cv[i].val = 0;
			i++;
		}
		p=&game->pre;

		while ( find_a_bigger_combo_in_hands(player->h,&cv[i].c,p) )
		{
			p=&cv[i].c;
			cv[i].val = 0;
			i++;
		}
		cv[i].c.type=NOTHING;
		cv[i].val = 0;
		i++;
	}
	for(int j=0; j<i;j++)
	{
		cv[j].c.control=0;
	}	
	return i;
}


int get_the_best_hand(GAME *game,  COMBO_OF_POKERS * cur,bool first)
{
	PLAYER *pl= game->players[CUR_PLAYER];
	FUNC_NAME
		//get best hands...
		COMBO_VALUE * cv= (COMBO_VALUE *)malloc(1000*sizeof(COMBO_VALUE));
	int cv_num=generate_perhaps_combos(game, cv,first);
	COMBO_OF_POKERS pre=game->pre;
	POKERS tmp;	
	int card_bak[21],out[26];	
	for(int i=0; i<cv_num; i++)
	{
		if(cv[i].c.type!=NOTHING)
		{
			game->pre = cv[i].c;
			tmp= *pl->h;			
			memcpy(card_bak,pl->card, max_POKER_NUM*sizeof(int));
			remove_combo_poker(pl->h,&game->pre, NULL);
			if(game->hun!=-1)	
				combo_to_int_array_hun(&game->pre, &out[0], pl->card, pl->h->hun);
			pl->cur = &game->pre;
			PRINTF(LEVEL,"try output: ");
			print_combo_poker(&cv[i].c);
			PRINTF(LEVEL,"\n");
		}
		//                else
		//              break;

		int winner = 0;
		if(pl->h->total!=0)
			winner=Check_win_quick(game, DOWN_PLAYER,0,0,0);
		else
			winner = 1;

		if(winner)
		{
			cv[i].val = 100;
		}
		else if(pl->id != LORD && game->players[winner]->id!=LORD)
		{
			cv[i].val=100;
		}
		else
			cv[i].val=-100;
		if(cv[i].c.type==BOMB || cv[i].c.type==ROCKET )
		{
			cv[i].val*=2;
		}

		if(cv[i].c.type!=NOTHING)
		{
			game->pre = pre;
			*pl->h=tmp;
			memcpy(pl->card,card_bak, max_POKER_NUM*sizeof(int));	
		}
		if(cv[i].val>0)
			break;
	}
	int t=0,pass=1;
	for(int i=0; i<cv_num; i++)
	{
		if(cv[t].val<cv[i].val)
		{
			t=i;
		}
	}
	if(cv[t].c.type!=NOTHING)
	{
		pass = 0;
		*cur=cv[t].c;
	}

	free(cv);
	return pass;
}

//return 1 : passed
//return 0: output to cur.
//don't remove any thing...
int robot_play(LordRobot* rb,  COMBO_OF_POKERS * cur,bool first)
{
	GAME *game=&rb->game;
	PLAYER *pl= game->players[CUR_PLAYER];
	int pass =1 ;
	FUNC_NAME;

	//check for robot win now
	if(!first) {
		COMBO_OF_POKERS* res;
		if( (res= check_for_win_now(game,&game->pre))!=NULL ) //win immediately, play the biggest or bomb
		{
			remove_combo_in_suit(pl->summary,res);//tobe removed
			*cur=*res;
			res->type = NOTHING;
			return  0;
		}
	}

	//check for other player win...
	//todo..

	//int pass;
	/*
	if(0&&pl->use_quick_win_check
	&& (game->players[CUR_PLAYER]->h->total<=2
	||game->players[DOWN_PLAYER]->h->total<=2
	|| game->players[UP_PLAYER]->h->total<=2)
	&&
	(!first || pl->id!= LORD)
	)
	{
	get_the_best_hand(game, cur,first);
	}
	else //normal process
	*/
	{
		//   int pass;
		COMBO_OF_POKERS* pre =&game->pre;
		memset(cur,0,sizeof(COMBO_OF_POKERS));
		if ( 1 ) //current player is computer
		{
COMPUTER_PLAY :
			if (game->player_type == LORD )//is lord
			{
				if (first) //zhudong chupai
				{
					lord_play_first(game,cur);
					pass=0;
				}
				else  //beidong chupai
				{
					pass = lord_play_2(game,cur,pre);
				}

			}
			else// farmer's trun
			{
				check_farmer_is_good(game);
				int isup = game->player_type == UPFARMER;
				//       DBG( PRINTF(LEVEL,"game firsthalf %d ： %s %s farmer\n",game->is_first_half_game,is_good_farmer?"good":"bad",isup?"up":"down"));
				if (first) //zhudong chupai
				{
					if (isup )
					{
						upfarmer_play_first(game,cur);
					}
					else  // down farmer
					{
						downfarmer_play_first(game,cur);
					}
					pass=0;
					//game->players[CUR_PLAYER]->summary->combo_total_num -- ;
				}
				else  //beidong chupai
				{
					if (isup)
						pass=upframer_play(game, cur, pre);
					else
						pass=downframer_play(game, cur, pre);
				}
			}
		}

		//PRINTF(VERBOSE,"\n====       Current Hands      ====\n         ");//, UP_PLAYER );
		//log_brief =1;
		//log_brief=0;
		return pass;
	}

}

//just for debug
int check_poker_suit(POKERS*hand ,PLAYER * player)
{
	//   return 1; //don't check
	POKERS t,*h=&t;
	memmove(h,hand,sizeof(POKERS));
	for (int k=0; k<20; k++)
	{
		if (player->combos[k].type!=NOTHING) {
			if ( remove_combo_poker(h,&player->combos[k],NULL )==false)
				return 0;
		}
	}
	if (h->total ==0)
		return 1;
	else
		return 0;
}


int is_game_first_half(GAME* game,int player)
{
	if ( ( game->players[CUR_PLAYER]->oppUp_num <=10 && game->players[UP_PLAYER]->id == LORD )
		||( game->players[CUR_PLAYER]->oppDown_num <= 10 && game->players[DOWN_PLAYER]->id == LORD ) )
		return 0;
	else if ( game->players[CUR_PLAYER]->oppDown_num <8 ||
		game->players[CUR_PLAYER]->oppUp_num <8 )
		return 0;
	return 1;
}

//int test{}



//初始化机器人，返回机器人指针
LordRobot* createRobot(int type, int userid)
{
	LordRobot* tmp = (LordRobot*)malloc(sizeof(LordRobot));
	tmp->game.known_others_poker = type&1;
	GAME* game =&tmp->game;
	game->use_best_for_undefined_case = type&2?1:0;
	//    logfile2 = NULL;
	//   logfile = NULL;
	/*
	if(userid!=0) {
		char filename[100];
		sprintf(filename,"/tmp/lordai/%d.log",userid);
		printf("log to file: %s\n",filename);

		logfile= fopen(filename,"wb");
	}*/
	//init something....
	memset(tmp->card,0xFF,3*21*sizeof(int));
	memset(tmp->outs,0xFF,21*sizeof(int));
	memset(tmp->pot,0xFF,4*sizeof(int));
	memset(tmp->laizi,0xFF,5*sizeof(int));
	return tmp;
}

void initCard(LordRobot* robot, int* robot_card, int* up_player_card, int* down_player_card)
{
#if 1
	//todo: save some code...
	GAME *game=&robot->game;
	int t = game->known_others_poker;
	int t2=game->use_best_for_undefined_case; 
	memset(game, 0,sizeof(GAME));
	memset(robot->outs,0xFF,21*sizeof(int));
	game->known_others_poker = t;
	game->use_best_for_undefined_case =t2; 	

	game->pot = &game->POT;

	game->computer[0] = 1;
	{
		int j;
		for (j=0; j<3; j++)
		{
			game->suits[j].h = &game->suits[j].p;
			game->suits[j].opps =&game->suits[j].opp;
			game->suits[j].h->hun = -1;
			game->suits[j].opps->hun = -1;			
			game->suits[j].combos =&game->suits[j].combos_store[0];
			game->suits[j].summary =&game->suits[j].combos_summary;
			game->suits[j].oppDown_num = 17;
			game->suits[j].oppUp_num = 17;

		}
	}
	game->players[0]=&game->suits[0];
	game->players[1]=&game->suits[1];
	game->players[2]=&game->suits[2];
	game->players[CUR_PLAYER]->use_quick_win_check = game->known_others_poker;

	POKERS all;
	full_poker(&game->all);
	full_poker(&all);
	game->all.hun = -1;

	//get pot_card;
	int card_used[54]= {0};

	//fa pai
	//  if (!set->cus) //rand
	{
		int i,k;
		for (k=0,i=0; k<17; k++,i++)
		{
			game->players[CUR_PLAYER]->card[k]= robot_card[i];
			card_used[ robot_card[i] ] = 1;
			if(robot_card[i]<52)
				game->players[CUR_PLAYER]->h->hands[robot_card[i]%13]++;
			else if(robot_card[i]<54)
				game->players[CUR_PLAYER]->h->hands[robot_card[i]-39]++;
			else
				PRINTF(LEVEL,"SUCks\n");


			game->players[UP_PLAYER]->card[k]= up_player_card[i];
			card_used[ up_player_card[i] ] = 1;
			if(up_player_card[i]<52)
				game->players[UP_PLAYER]->h->hands[up_player_card[i]%13]++;
			else if(up_player_card[i]<54)
				game->players[UP_PLAYER]->h->hands[up_player_card[i]-39]++;
			else
				PRINTF(LEVEL,"SUCks\n");

			game->players[DOWN_PLAYER]->card[k]= down_player_card[i];
			card_used[ down_player_card[i] ] = 1;

			if(down_player_card[i]<52)
				game->players[DOWN_PLAYER]->h->hands[down_player_card[i]%13]++;
			else  if(down_player_card[i]<54)
				game->players[DOWN_PLAYER]->h->hands[down_player_card[i]-39]++;
			else
				PRINTF(LEVEL,"SUCks\n");
		}
		for (k=17; k<21; k++){
			game->players[DOWN_PLAYER]->card[k]=-1;
			game->players[CUR_PLAYER]->card[k]=-1;
			game->players[UP_PLAYER]->card[k]=-1;
		}		
	}


	sort_poker(game->players[2]->h);
	sort_poker(game->players[1]->h);
	sort_poker(game->players[0]->h);
	PRINTF(LEVEL,"\ncurrent PLAYER Hands: ");
	print_hand_poker(game->players[2]->h);
	print_card(game->players[2]->card);

	full_poker(&all);
	sub_poker(&all,game->players[CUR_PLAYER]->h,game->players[CUR_PLAYER]->opps);
	sort_poker(game->players[CUR_PLAYER]->opps);
	sub_poker(&all,game->players[UP_PLAYER]->h,game->players[UP_PLAYER]->opps);
	sort_poker(game->players[UP_PLAYER]->opps);
	sub_poker(&all,game->players[DOWN_PLAYER]->h,game->players[DOWN_PLAYER]->opps);
	sort_poker(game->players[DOWN_PLAYER]->opps);

	add_poker(game->players[DOWN_PLAYER]->h,game->players[CUR_PLAYER]->h,&game->all);
	add_poker(&game->all,game->players[UP_PLAYER]->h,&game->all);
	sub_poker(&all,&game->all, game->pot);
	sort_poker(game->pot);

	{
		int j=0;
		for(int i=0; i<54; i++)
		{
			if(card_used[i]==0)
				game->pot_card[j++] = i;
			if(j>3)
			{
				if(game->pot->total!=3)
					PRINTF_ALWAYS("line %d: FATAL ERROR in initCard\n",__LINE__);	 	
				//  fill_card_from_poker(game->pot_card, game->pot);
				break;
			}
		}
		if(j!=3)
		{
			PRINTF(LEVEL,"line %d: FATAL ERROR in initCard\n",__LINE__);
			j=3;
		}
		game->pot_card[j]=-1;
	}



	full_poker(&game->all);
	game->players[0]->cur=NULL;
	game->players[1]->cur=NULL;
	//    log_brief =0;

	game->rec.first_caller = -1;
	game->rec.p[CUR_PLAYER]=*game->players[CUR_PLAYER]->h;
	game->rec.pot = *game->pot;
	game->rec.p[DOWN_PLAYER]=*game->players[DOWN_PLAYER]->h;
	game->rec.p[UP_PLAYER]=*game->players[UP_PLAYER]->h;

	game->hun = -1;	
	return ;
#endif
}

void setLaizi(LordRobot* robot, int laizi)
{

	GAME *game=&robot->game;
	{
		int j;
		for (j=0; j<3; j++)
		{
			game->suits[j].h->hun = laizi;
			game->suits[j].opps->hun = laizi;			
		}
	}
	game->hun = laizi;		
	game->all.hun = laizi;

}

//传出参数为本机器人叫分
//-1 表示不叫
int callScore(LordRobot* robot)
{
	GAME *game=&robot->game;
	PLAYER* pl = game->players[CUR_PLAYER];
	if(game->rec.first_caller==-1)
		game->rec.first_caller = 	CUR_PLAYER;
	//check ourselves
	if(game->known_others_poker)
	{
		int score=0;


		LordRobot tR;
		Robot_copy(&tR,robot);
		beLord(&tR, tR.game.pot_card,CUR_PLAYER);
		int winner = Check_win_quick(&tR.game, CUR_PLAYER,0,0,0);
		if(winner )
		{
			PRINTF(LEVEL,"robot %x could win!!\n",robot);
			score = 3;
		}
		else
		{
			PRINTF(LEVEL,"robot %x would lose!!\n",robot);

			if(game->players[DOWN_PLAYER]->computer ==0
				&& game->players[UP_PLAYER]->computer == 1
				&& game->players[DOWN_PLAYER]->score == 0 /*Un called*/)
			{
				int score;
				//try be lord...
				int control= calc_controls(game->players[DOWN_PLAYER]->h,game->players[DOWN_PLAYER]->opps, 6 );
				if ( control > 20 )  //qiang dizhu!
					score= 3;
				if(control>10)
				{
					search_combos_in_suit(game, game->players[DOWN_PLAYER]->h,
						game->players[DOWN_PLAYER]->opps, game->players[DOWN_PLAYER]);
					if(CONTROL_SUB_COMBO_NUM(game->players[DOWN_PLAYER])>-30)
					{
						score= 3;
					}
				}
				PRINTF(LEVEL,"robot %x would lose, but should be lord since player will win!!\n",robot);
			} else  if(game->players[UP_PLAYER]->computer ==0
				&& game->players[DOWN_PLAYER]->computer == 1
				&& game->players[UP_PLAYER]->score == 0 /*Un called*/)
			{
				//try be lord...
				int control= calc_controls(game->players[UP_PLAYER]->h,game->players[UP_PLAYER]->opps, 6 );
				if ( control > 20 )  //qiang dizhu!
				{
					add_poker(game->players[DOWN_PLAYER]->h, game->pot, game->players[DOWN_PLAYER]->h);
					search_combos_in_suit(game, game->players[DOWN_PLAYER]->h,
						game->players[DOWN_PLAYER]->opps, game->players[DOWN_PLAYER]);
					if(CONTROL_SUB_COMBO_NUM(game->players[DOWN_PLAYER])+20<=
						CONTROL_SUB_COMBO_NUM(tR.game.players[CUR_PLAYER]))
					{
						score= 3;
					}
				}
				PRINTF(LEVEL,"robot %x would lose, but should be lord since player will win!!\n",robot);
				//return 3;
			}
		}
		game->rec.score[(int)CUR_PLAYER]=score;

		game->call.player[game->call.cur_num]=CUR_PLAYER;
		game->call.score[game->call.cur_num]=score;
		game->call.cur_num++; 

		if(score==3)
		{      
			game->call.cur_lord=CUR_PLAYER;		
			return score;			
		}
		else
			return -1;

	}

	//check human players...
	//

	//    GAME *game=&robot->game;
	int control= calc_controls(game->players[2]->h,game->players[2]->opps, 6 );
	int score = (control>10)?3:-1;
	game->rec.score[(int)CUR_PLAYER]=score;
	game->call.player[game->call.cur_num]=CUR_PLAYER;
	game->call.score[game->call.cur_num]=score;
	game->call.cur_num++;
	return score;
}

//be lord
/* 5、其他玩家叫分信息：
Player_number, 0: down player,
1: up player.
2: cur player
*/
void beLord(LordRobot* robot,int* robot_card,int player_number)
{
	robot->game.rec.lord = player_number;

	if(player_number == 2)
	{
		//printf("robot is lord!\n");
		GAME *game=&robot->game;

		for (int k=17,i=0; k<20; k++,i++)
		{
			game->players[CUR_PLAYER]->card[k]= robot_card[i];
			if(robot_card[i]<52)
				game->players[CUR_PLAYER]->h->hands[robot_card[i]%13]++;
			else
				game->players[CUR_PLAYER]->h->hands[robot_card[i]-39]++;
		}
		game->players[CUR_PLAYER]->card[20]=-1;
		sort_poker(game->players[CUR_PLAYER]->h);
		print_hand_poker(game->players[CUR_PLAYER]->h);
		POKERS all;
		full_poker(&all);
		sub_poker(&all,game->players[CUR_PLAYER]->h,game->players[CUR_PLAYER]->opps);
		sort_poker(game->players[CUR_PLAYER]->opps);

		//	game->lord=3;
		game->players[CUR_PLAYER]->id=LORD;
		game->players[DOWN_PLAYER]->id=DOWNFARMER;
		game->players[UP_PLAYER]->id=UPFARMER;
		game->player_type = (PLAYER_TYPE)game->players[CUR_PLAYER]->id;
		if(game->init==0)
		{
			game_init(game);
			game->init=1;
		}
		game->rec.type[CUR_PLAYER] = game->players[CUR_PLAYER]->id;
		game->rec.type[DOWN_PLAYER] = game->players[DOWN_PLAYER]->id;
		game->rec.type[UP_PLAYER] = game->players[UP_PLAYER]->id;
		return;
	}
	GAME *game=&robot->game;
	//game->player[player_number].type = LORD;
	if(player_number==UP_PLAYER)
	{
		game->players[CUR_PLAYER]->id  = DOWNFARMER;//robot
		game->players[DOWN_PLAYER]->id  = UPFARMER;
		game->players[UP_PLAYER]->id = LORD;
		game->players[CUR_PLAYER]->oppUp_num =20;
		add_poker(game->players[UP_PLAYER]->h, game->pot,game->players[UP_PLAYER]->h);
		sort_poker(game->players[UP_PLAYER]->h);
		sub_poker(game->players[UP_PLAYER]->opps, game->pot,game->players[UP_PLAYER]->opps);
		sort_poker(game->players[UP_PLAYER]->opps);

		for (int k=17,i=0; k<20; k++,i++)
		{
			game->players[UP_PLAYER]->card[k]= robot_card[i];
		}

	}
	else if	(player_number==DOWN_PLAYER)
	{
		game->players[DOWN_PLAYER]->id = LORD;
		game->players[UP_PLAYER]->id= DOWNFARMER;
		game->players[CUR_PLAYER]->id = UPFARMER;
		game->players[CUR_PLAYER]->oppDown_num =20;
		add_poker(game->players[DOWN_PLAYER]->h, game->pot,game->players[DOWN_PLAYER]->h);
		sort_poker(game->players[DOWN_PLAYER]->h);
		sub_poker(game->players[DOWN_PLAYER]->opps, game->pot,game->players[DOWN_PLAYER]->opps);
		sort_poker(game->players[DOWN_PLAYER]->opps);
		for (int k=17,i=0; k<20; k++,i++)
		{
			game->players[DOWN_PLAYER]->card[k]= robot_card[i];
		}
	}
	game->player_type = (PLAYER_TYPE)game->players[CUR_PLAYER]->id;
	//	game->lord=player_number+1;
	if(game->init==0)
	{
		game_init(game);
		game->init=1;
	}
	game->rec.type[CUR_PLAYER] = game->players[CUR_PLAYER]->id;
	game->rec.type[DOWN_PLAYER] = game->players[DOWN_PLAYER]->id;
	game->rec.type[UP_PLAYER] = game->players[UP_PLAYER]->id;

}


void getPlayerTakeOutCards_hun(LordRobot* robot,int* poker, int * aim_poker, int player_number)
{
	GAME *game=&robot->game;
	game->players[player_number]->cur= &game->players[player_number]->curcomb;
	if (poker[0]!=-1)// || aim_poker[0]!= -1)
	{

		POKERS a,b,c;
		int i,tmp_card[26];
		int num =read_poker_int(poker,&a);		
		int hun_num = read_poker_int(aim_poker,&b);

		if(hun_num!=a.hands[game->hun])
		{
			PRINTF_ALWAYS("[%s][%d]: godie! \n",__FUNCTION__,__LINE__);
		}
		a.hands[game->hun] = 0;

		if( hun_num)
		{
			add_poker(&a, &b, &c);
			if( !(i=is_combo(&c, game->players[player_number]->cur)))
			{
				PRINTF_ALWAYS("godie! error pokers from other poker\n");
			}
			memset(&b,0,sizeof(POKERS));
			b.begin=b.end=game->hun;
			b.hands[b.begin] = hun_num;

			add_poker(&a, &b, &c);   
			if(game->players[player_number]->cur->type == BOMB)
			{
				game->players[player_number]->cur->three_desc[0]= hun_num==4? 2: 0;
			}		
		}
		else
		{
			c=a;
			if( !(i=is_combo(&a, game->players[player_number]->cur)))
			{
				PRINTF_ALWAYS("godie! error pokers from other poker\n");
			}
			if(game->players[player_number]->cur->type == BOMB)
			{
				game->players[player_number]->cur->three_desc[0]= 1;
			}
		}

		sub_poker(game->players[CUR_PLAYER]->opps, &c,
			game->players[CUR_PLAYER]->opps);
		sub_poker(&game->all, &c,&game->all);

		memmove(&game->prepre,&game->pre,sizeof(COMBO_OF_POKERS));
		memmove(&game->pre,game->players[player_number]->cur,sizeof(COMBO_OF_POKERS));
		game->prepre_playernum = game->pre_playernum;
		game->pre_playernum = player_number;

		PRINTF(VERBOSE,"itr@game[%x] %d\n",game,game->rec.now);
		game->rec.history[game->rec.now].player= (PLAYER_POS)player_number;
		game->rec.history[game->rec.now++].h= game->pre;
		if(game->pre.type == BOMB ||game->pre.type == ROCKET)
		{
			game->rec.bombed++;
		}

		if (player_number==UP_PLAYER)
		{
			sub_poker(game->players[UP_PLAYER]->h, &c,game->players[UP_PLAYER]->h);
			// remove card 
			remove_poker_in_int_array(&c, 
				tmp_card,game->players[UP_PLAYER]->card, game->hun);
			check_card(game->players[UP_PLAYER]->h,game->players[UP_PLAYER]->card);
			game->players[CUR_PLAYER]->oppUp_num -= c.total;
			sub_poker(game->players[DOWN_PLAYER]->opps, &c,game->players[DOWN_PLAYER]->opps);
			game->players[DOWN_PLAYER]->oppDown_num -= c.total;
		}
		else if (player_number==DOWN_PLAYER)
		{
			sub_poker(game->players[DOWN_PLAYER]->h, &c,game->players[DOWN_PLAYER]->h);
			// remove card 
			remove_poker_in_int_array(&c, 
				tmp_card,game->players[DOWN_PLAYER]->card, game->hun);
			check_card(game->players[DOWN_PLAYER]->h,game->players[DOWN_PLAYER]->card);
			game->players[CUR_PLAYER]->oppDown_num -= c.total;
			sub_poker(game->players[UP_PLAYER]->opps, &c,game->players[UP_PLAYER]->opps);
			game->players[UP_PLAYER]->oppUp_num -= c.total;
		}

		if(!i)
			PRINTF_ALWAYS("fatal erro line %d!\n",__LINE__);

	}
	else
	{
		PRINTF(VERBOSE,"itr@game[%x] %d PASS\n",game,game->rec.now);
		game->players[player_number]->cur = NULL;
		game->rec.history[game->rec.now].player= (PLAYER_POS)player_number;
		game->rec.history[game->rec.now++].h.type= NOTHING;
	}

}



/*其他玩家出牌信息：
Player_number, 0: down player,
1: up  player.
参数同1、发牌，如果其他玩家未出牌，则传入null
*/
void getPlayerTakeOutCards(LordRobot* robot,int* poker, int * laizi, int player_number)
{
	GAME *game=&robot->game;

	if(game->hun !=-1)
	{
		return getPlayerTakeOutCards_hun(robot,poker,laizi,player_number);
	}

	game->players[player_number]->cur= &game->players[player_number]->curcomb;
	if (poker[0]!=-1)
	{
		int i=is_input_combo(poker, game->players[player_number]->cur);
		memmove(&game->prepre,&game->pre,sizeof(COMBO_OF_POKERS));
		memmove(&game->pre,game->players[player_number]->cur,sizeof(COMBO_OF_POKERS));
		game->prepre_playernum = game->pre_playernum;
		game->pre_playernum = player_number;
		remove_combo_poker(game->players[CUR_PLAYER]->opps, &game->pre,NULL);

		remove_combo_poker(&game->all, &game->pre,NULL);

		PRINTF(VERBOSE,"itr@game[%x] %d\n",game,game->rec.now);
		game->rec.history[game->rec.now].player= (PLAYER_POS)player_number;
		game->rec.history[game->rec.now++].h= game->pre;
		if(game->pre.type == BOMB ||game->pre.type == ROCKET)
		{
			game->rec.bombed++;
		}

		if (player_number==UP_PLAYER)
		{
			remove_combo_poker(game->players[UP_PLAYER]->h, &game->pre,NULL);
			game->players[CUR_PLAYER]->oppUp_num -= i;
			remove_combo_poker(game->players[DOWN_PLAYER]->opps, &game->pre,NULL);
			game->players[DOWN_PLAYER]->oppDown_num -= i;

			//remove card...
			/*
			for(int i=0;i<21;i++)
			{
			if(poker[i]==-1) break;
			for(int j=0;j<21;j++)
			if( game->players[UP_PLAYER]->card[j]==poker[i])
			game->players[UP_PLAYER]->card[j] = -1;
			}
			*/
			//if( game->known_others_poker) //check others
			{
				//            search_combos_in_suit(game,game->players[UP_PLAYER]->h,game->players[UP_PLAYER]->opps,game->players[UP_PLAYER]);
			}
		}
		else if (player_number==DOWN_PLAYER)
		{
			remove_combo_poker(game->players[DOWN_PLAYER]->h, &game->pre,NULL);
			game->players[CUR_PLAYER]->oppDown_num -= i;
			remove_combo_poker(game->players[UP_PLAYER]->opps, &game->pre,NULL);
			game->players[UP_PLAYER]->oppUp_num -= i;
		}

		game->players[CUR_PLAYER]->first_hint = 1;

		if(!i)
			PRINTF_ALWAYS("fatal erro line %d!\n",__LINE__);



	}
	else
	{
		PRINTF(VERBOSE,"itr@game[%x] %d PASS\n",game,game->rec.now);
		game->players[player_number]->cur = NULL;
		game->rec.history[game->rec.now].player= (PLAYER_POS)player_number;
		game->rec.history[game->rec.now++].h.type= NOTHING;
	}

}


/* 5、其他玩家叫分信息：
Player_number, 0: up player,
1: down player.
score,  分值, -1表示不叫
*/
void getPlayerCallScore(LordRobot* robot,int score, int player_number)
{
	GAME *game=&robot->game;
	game->players[player_number]->score = score;
	game->rec.score[player_number]=score;
	if(game->rec.first_caller==-1)
		game->rec.first_caller = 	player_number;
	game->call.player[game->call.cur_num]=player_number;
	game->call.score[game->call.cur_num]=score;
	game->call.cur_num++;
	if(score==4) //doubled game
	{ 
		game->call.double_times++;
	}
	if(score!=-1)
		game->call.cur_lord=player_number;
}

int  doubleGame(LordRobot* robot)
{
	GAME *game=&robot->game;
	int lord=game->call.cur_lord;
	if(0)//(game->players[lord]->computer)
	{
		if( lord == CUR_PLAYER) //double check
		{
			return 1;
		}
		else
		{  // 50% 概率 加倍
			return rand()%2;
		}
	}
	else
	{
		int player_pos =lord;		  
		if(game->known_others_poker)
		{	
			int score=0;
			LordRobot tR;
			Robot_copy(&tR,robot);		  
			PRINTF(LEVEL,"=============START DOUBLEGAME %d=============\n",player_pos);	
			if(player_pos==DOWN_PLAYER)
			{
				return 0;
				POKERS tmph= *tR.game.players[CUR_PLAYER]->h;
				POKERS tmpopps= *tR.game.players[CUR_PLAYER]->opps;
				*tR.game.players[CUR_PLAYER]->h = *tR.game.players[DOWN_PLAYER]->h;
				*tR.game.players[CUR_PLAYER]->opps = *tR.game.players[DOWN_PLAYER]->opps;
				*tR.game.players[DOWN_PLAYER]->h = *tR.game.players[UP_PLAYER]->h;
				*tR.game.players[DOWN_PLAYER]->opps = *tR.game.players[UP_PLAYER]->opps;
				*tR.game.players[UP_PLAYER]->h = tmph;
				*tR.game.players[UP_PLAYER]->opps = tmpopps;		 
			}
			else			
				if(player_pos==UP_PLAYER)
				{
					return 0;
					POKERS tmph= *tR.game.players[CUR_PLAYER]->h;
					POKERS tmpopps= *tR.game.players[CUR_PLAYER]->opps;
					*tR.game.players[CUR_PLAYER]->h = *tR.game.players[UP_PLAYER]->h;
					*tR.game.players[CUR_PLAYER]->opps = *tR.game.players[UP_PLAYER]->opps;
					*tR.game.players[UP_PLAYER]->h = *tR.game.players[DOWN_PLAYER]->h;
					*tR.game.players[UP_PLAYER]->opps = *tR.game.players[DOWN_PLAYER]->opps;
					*tR.game.players[DOWN_PLAYER]->h = tmph;
					*tR.game.players[DOWN_PLAYER]->opps = tmpopps;		 
				}
				beLord(&tR, tR.game.pot_card,CUR_PLAYER);
				int winner = Check_win_quick(&tR.game, CUR_PLAYER,0,0,0);
				if(winner )
				{
					return 0;			
				}
				else
				{
					return (rand()%100 )>75;
				}
		}
		else 
			return 0;          	 
	}
	return 0;
}

int showCard(LordRobot* robot)
{
	int ret =0;
	GAME *game=&robot->game;
	//check if i'm lord
	{
		if(game->known_others_poker)
		{	
			int score=0;
			LordRobot tR;
			Robot_copy(&tR,robot);		   
			beLord(&tR, tR.game.pot_card,CUR_PLAYER);
			PLAYER * pl= tR.game.players[CUR_PLAYER];
			search_combos_in_suit(&tR.game, pl ->h,pl ->opps,pl );		   
			int winner;
			if( (CONTROL_SUB_COMBO_NUM(pl)>-20 && (pl->summary->bomb_num>0 || opp_hasbomb(&tR.game)))
				|| (CONTROL_SUB_COMBO_NUM(pl)>-10)
				|| (HUN_NUMBER(tR.game.players[CUR_PLAYER]->h) >=3)
				)
			{
				winner=1;
			}
			else
				winner = Check_win_quick(&tR.game, CUR_PLAYER,0,0,0);
			if(winner )
			{
				PRINTF(LEVEL,"robot %x could win!!\n",robot);
				if ( CONTROL_SUB_COMBO_NUM(pl) > 5*(LORD_LEVEL_3-4))
					ret|= 0x3;			
				else if ( CONTROL_SUB_COMBO_NUM(pl) > 5*(LORD_LEVEL_2-8)  )
					ret|= 0x2;					
				else
				{
					ret |=1;		       
				} 	
			}

		}
	}
	//check up_farmer
	if(0)
	{
		if(game->known_others_poker)
		{	
			int score=0;
			LordRobot tR;
			Robot_copy(&tR,robot);		   
			PLAYER * pl= tR.game.players[CUR_PLAYER];
			PLAYER * dnpl= tR.game.players[DOWN_PLAYER];
			PLAYER * uppl= tR.game.players[UP_PLAYER];
			POKERS tmph= *tR.game.players[CUR_PLAYER]->h;
			POKERS tmpopps= *tR.game.players[CUR_PLAYER]->opps;
			*tR.game.players[CUR_PLAYER]->h = *tR.game.players[DOWN_PLAYER]->h;
			*tR.game.players[CUR_PLAYER]->opps = *tR.game.players[DOWN_PLAYER]->opps;
			*tR.game.players[DOWN_PLAYER]->h = *tR.game.players[UP_PLAYER]->h;
			*tR.game.players[DOWN_PLAYER]->opps = *tR.game.players[UP_PLAYER]->opps;
			*tR.game.players[UP_PLAYER]->h = tmph;
			*tR.game.players[UP_PLAYER]->opps = tmpopps;				   
			int tmpcard[21];
			memcpy(tmpcard, pl->card,sizeof(tmpcard));	
			memcpy(pl->card,dnpl->card,sizeof(tmpcard));	
			memcpy(dnpl->card,uppl->card,sizeof(tmpcard));	
			memcpy(uppl->card,tmpcard,sizeof(tmpcard));	
			beLord(&tR, tR.game.pot_card,CUR_PLAYER);
			int winner = Check_win_quick(&tR.game, CUR_PLAYER,0,0,0);
			if(!winner )
			{
				PRINTF(LEVEL,"robot %x could win!!\n",robot);
				PLAYER * pl= tR.game.players[CUR_PLAYER];
				search_combos_in_suit(&tR.game, pl ->h,pl ->opps,pl );
				if ( CONTROL_SUB_COMBO_NUM(pl) > 5*(UPFARMER_LEVEL_3-6) )
					ret|= (0x3<<8);			
				else if ( CONTROL_SUB_COMBO_NUM(pl) >5*(UPFARMER_LEVEL_2-10) )
					ret|= (0x2<<8);					
				else
				{
					ret |=(1<<8);		       
				} 						
			}
		}
	}	
	//check down_farmer
	if(0)
	{
		if(game->known_others_poker)
		{	
			int score=0;
			LordRobot tR;
			Robot_copy(&tR,robot);		   
			PLAYER * pl= tR.game.players[CUR_PLAYER];
			PLAYER * dnpl= tR.game.players[DOWN_PLAYER];
			PLAYER * uppl= tR.game.players[UP_PLAYER];	
			POKERS tmph= *tR.game.players[CUR_PLAYER]->h;
			POKERS tmpopps= *tR.game.players[CUR_PLAYER]->opps;
			*tR.game.players[CUR_PLAYER]->h = *tR.game.players[UP_PLAYER]->h;
			*tR.game.players[CUR_PLAYER]->opps = *tR.game.players[UP_PLAYER]->opps;
			*tR.game.players[UP_PLAYER]->h = *tR.game.players[DOWN_PLAYER]->h;
			*tR.game.players[UP_PLAYER]->opps = *tR.game.players[DOWN_PLAYER]->opps;
			*tR.game.players[DOWN_PLAYER]->h = tmph;
			*tR.game.players[DOWN_PLAYER]->opps = tmpopps;		 			   
			int tmpcard[21];
			memcpy(tmpcard, pl->card,sizeof(tmpcard));	
			memcpy(pl->card,uppl->card,sizeof(tmpcard));	
			memcpy(uppl->card,dnpl->card,sizeof(tmpcard));	
			memcpy(dnpl->card,tmpcard,sizeof(tmpcard));	
			beLord(&tR, tR.game.pot_card,CUR_PLAYER);
			int winner = Check_win_quick(&tR.game, CUR_PLAYER,0,0,0);
			if(!winner )
			{
				PRINTF(LEVEL,"robot %x could win!!\n",robot);
				PLAYER * pl= tR.game.players[CUR_PLAYER];
				search_combos_in_suit(&tR.game, pl ->h,pl ->opps,pl );
				if ( CONTROL_SUB_COMBO_NUM(pl) > 5*(DOWNFARMER_LEVEL_3-4) )
					ret|= (0x3<<16);			
				else if ( CONTROL_SUB_COMBO_NUM(pl) >5*(DOWNFARMER_LEVEL_2-8) )
					ret|= (0x2<<16);					
				else
				{
					ret |=(1<<16);		       
				} 				
			}
		}
	}	
	return ret;			
}

int forceLord(LordRobot* robot)
{
	GAME *game=&robot->game;
	int pos=1; //pos=1, first
	int up_done=0;
	int down_done=0;					
	//int cur_lord =0;
	int lord=game->call.cur_lord;
	return 0;
	for(int i=0;i<game->call.cur_num;i++)
	{
		if(game->call.score[i]== -1
			||game->call.score[i] ==4) //不叫或者不抢
		{
			if(game->call.player[i]==UP_PLAYER)		
			{
				up_done=1;
			}
			if(game->call.player[i]==DOWN_PLAYER)		
			{
				down_done=1;
			}		 
		}			 
	}
	pos += up_done +down_done;
	//  if(pos==3) //最后一次抢地主
	{
		if(game->known_others_poker)
		{	
			int score=0;
			LordRobot tR;
			Robot_copy(&tR,robot);		   
			beLord(&tR, tR.game.pot_card,CUR_PLAYER);
			int winner = Check_win_quick(&tR.game, CUR_PLAYER,0,0,0);
			if(winner )
			{
				PRINTF(LEVEL,"robot %x could win!!\n",robot);
				PLAYER * pl= tR.game.players[CUR_PLAYER];
				search_combos_in_suit(&tR.game, pl ->h,pl ->opps,pl );
				if ( CONTROL_SUB_COMBO_NUM(pl) >0 )
					return 1;			
				else if ( CONTROL_SUB_COMBO_NUM(pl) >-10 )
					return 1;			
				else
				{
					return rand()%2;							
				}
			}
			else
			{
				return 0;
			}
		}
		else 
			return rand()%2;
	}

	return 0;

	if(pos==2 || pos==1) //后面还有1 -2人可以抢地主
	{
		int other_player_type=0; //computer or human
		int player_pos =UP_PLAYER;
		if(up_done)
		{
			player_pos=DOWN_PLAYER;
		}
		other_player_type = game->players[player_pos]->computer;			 
		if(game->known_others_poker)
		{	
			int score=0;
			LordRobot tR;
			Robot_copy(&tR,robot);		  
			PRINTF(LEVEL,"=============START LORD %d=============\n",player_pos);		   
			if(player_pos==DOWN_PLAYER)
			{
				POKERS tmph= *tR.game.players[CUR_PLAYER]->h;
				POKERS tmpopps= *tR.game.players[CUR_PLAYER]->opps;
				*tR.game.players[CUR_PLAYER]->h = *tR.game.players[DOWN_PLAYER]->h;
				*tR.game.players[CUR_PLAYER]->opps = *tR.game.players[DOWN_PLAYER]->opps;
				*tR.game.players[DOWN_PLAYER]->h = *tR.game.players[UP_PLAYER]->h;
				*tR.game.players[DOWN_PLAYER]->opps = *tR.game.players[UP_PLAYER]->opps;
				*tR.game.players[UP_PLAYER]->h = tmph;
				*tR.game.players[UP_PLAYER]->opps = tmpopps;		 
			}
			else if(player_pos==UP_PLAYER)
			{
				POKERS tmph= *tR.game.players[CUR_PLAYER]->h;
				POKERS tmpopps= *tR.game.players[CUR_PLAYER]->opps;
				*tR.game.players[CUR_PLAYER]->h = *tR.game.players[UP_PLAYER]->h;
				*tR.game.players[CUR_PLAYER]->opps = *tR.game.players[UP_PLAYER]->opps;
				*tR.game.players[UP_PLAYER]->h = *tR.game.players[DOWN_PLAYER]->h;
				*tR.game.players[UP_PLAYER]->opps = *tR.game.players[DOWN_PLAYER]->opps;
				*tR.game.players[DOWN_PLAYER]->h = tmph;
				*tR.game.players[DOWN_PLAYER]->opps = tmpopps;		 
			}
			beLord(&tR, tR.game.pot_card,CUR_PLAYER);
			int winner = Check_win_quick(&tR.game, CUR_PLAYER,0,0,0);
			if(winner )
			{
				if(other_player_type)
				{
					PLAYER * pl= tR.game.players[CUR_PLAYER];
					search_combos_in_suit(&tR.game, pl ->h,pl ->opps,pl );
					if ( CONTROL_SUB_COMBO_NUM(pl) >0 )
						return 1;		          
				}
				else
					return 0;
			}

			if(pos==1) //check down player as well
			{
				LordRobot tR;
				Robot_copy(&tR,robot);		
				POKERS tmph= *tR.game.players[CUR_PLAYER]->h;
				POKERS tmpopps= *tR.game.players[CUR_PLAYER]->opps;
				*tR.game.players[CUR_PLAYER]->h = *tR.game.players[DOWN_PLAYER]->h;
				*tR.game.players[CUR_PLAYER]->opps = *tR.game.players[DOWN_PLAYER]->opps;
				*tR.game.players[DOWN_PLAYER]->h = *tR.game.players[UP_PLAYER]->h;
				*tR.game.players[DOWN_PLAYER]->opps = *tR.game.players[UP_PLAYER]->opps;
				*tR.game.players[UP_PLAYER]->h = tmph;
				*tR.game.players[UP_PLAYER]->opps = tmpopps;		 
				beLord(&tR, tR.game.pot_card,CUR_PLAYER);
				int winner = Check_win_quick(&tR.game, CUR_PLAYER,0,0,0);
				if(winner )
				{
					return 0;
				}			
			}

			//check self
			{	
				LordRobot tR;
				Robot_copy(&tR,robot);		   
				beLord(&tR, tR.game.pot_card,CUR_PLAYER);
				int winner = Check_win_quick(&tR.game, CUR_PLAYER,0,0,0);
				if(winner )
				{
					PRINTF(LEVEL,"robot %x could win!!\n",robot);
					PLAYER * pl= tR.game.players[CUR_PLAYER];
					search_combos_in_suit(&tR.game, pl ->h,pl ->opps,pl );
					if ( CONTROL_SUB_COMBO_NUM(pl) >0 )
						return 1;			
					else if ( CONTROL_SUB_COMBO_NUM(pl) >-10 )
						return rand()%2;			
					else
					{
						if (game->players[lord]->computer) 
							return 0;
						return (rand()%100 )>50;							
					}
				}
				else
				{
					return 0;
				}
			}

		}
		else 
			return 0;          	 
	}	 

	return 0;
}


/*6、出牌：传出参数同[发牌]的传入参数，如果机器人不出牌，则传出null*/
int takeOut(LordRobot* robot,int * p, int* laizi)
{
	GAME *game=&robot->game;
	COMBO_OF_POKERS *cur, combo;
	PLAYER * pl= game->players[CUR_PLAYER];
	PLAYER * downPl= game->players[UP_PLAYER];
	PLAYER * upPl= game->players[DOWN_PLAYER];

	cur=&combo;
	//	game->cur_playernum = 2;

	static int turn = 0;
	DBG(
		PRINTF(LEVEL,"==START_OF_ROBOT_PLAY TURN %d==\n",turn);
	)
		int pass=1,first= (game->players[UP_PLAYER]->cur==NULL && game->players[DOWN_PLAYER]->cur==NULL);

	search_combos_in_suit(game, game->players[CUR_PLAYER]->h,  game->players[CUR_PLAYER]->opps,game->players[CUR_PLAYER]);
	if(game->known_others_poker)
	{
		search_combos_in_suit(game, game->players[DOWN_PLAYER]->h,  game->players[DOWN_PLAYER]->opps,game->players[DOWN_PLAYER]);
		search_combos_in_suit(game, game->players[UP_PLAYER]->h,  game->players[UP_PLAYER]->opps,game->players[UP_PLAYER]);
	}

	update_players(game,NULL);

	if(pl->h->total !=0) //error check
	{
		if(first)
		{   //主动出牌
			//printf("zhudong chupai!\n");
			pass=robot_play(robot,cur,1);
			pass = 0;
		}
		else //被动出牌
		{
			//printf("beidong chupai!\n");
			pass=robot_play(robot,cur,0);
		}
	}

	if(!pass) {
		if(game->hun==-1)
		{

			//update pokers..
			remove_combo_poker(pl->h, cur, NULL);
			remove_combo_poker(upPl->opps, cur,NULL);
			upPl->oppDown_num -= get_combo_number(cur);
			remove_combo_poker(downPl->opps, cur,NULL);
			downPl->oppUp_num -=  get_combo_number(cur);

			combo_to_int_array(cur,p,game->players[CUR_PLAYER]->card);
			remove_combo_poker(&game->all, cur,NULL);	
			*laizi= -1;
		}
		else
		{
			POKERS a;  
			int tmp[26];
			//fixme: for the bomb case, KEEP BOMB OR KEEP hun?
			combo_to_int_array_hun(cur,tmp,pl->card,game->hun);
			convert_to_new_out_format(tmp,p,laizi);		   
			int num =read_poker_int(p,&a);	

			//update pokers..
			sub_poker(pl->h, &a,pl->h);//cur, NULL);
			//check cards and h
			check_card(pl->h,pl->card);
			sub_poker(upPl->opps, &a,upPl->opps);
			sub_poker(downPl->opps, &a,downPl->opps);	
			sub_poker(&game->all, &a,&game->all);

			upPl->oppDown_num -= get_combo_number(cur);    
			downPl->oppUp_num -=  get_combo_number(cur);

		}

		PRINTF(VERBOSE,"game[%x] ",game);
		print_card(p);
		print_card_2(game->players[CUR_PLAYER]->card,21);
		game->prepre_playernum = game->pre_playernum;
		game->pre_playernum = CUR_PLAYER;


		PRINTF(VERBOSE,"itr@game[%x] own %d\n",game,game->rec.now);
		game->rec.history[game->rec.now].player= CUR_PLAYER;
		game->rec.history[game->rec.now++].h= *cur;
		if(game->pre.type == BOMB ||game->pre.type == ROCKET)
		{
			game->rec.bombed++;
		}
		if(*p==-1)
			PRINTF_ALWAYS("ERR:  you play what??\n");

	}
	else
	{
		p[0]=-1;
		laizi[0]=-1;
		game->rec.history[game->rec.now].player= CUR_PLAYER;
		game->rec.history[game->rec.now++].h.type= NOTHING;
		if(first)
			PRINTF_ALWAYS("ERR:  you are the first player..please output something? OK?\n");
	}
	/*

	if( !check_poker_suit(game->players[CUR_PLAYER]->h,game->players[CUR_PLAYER]))
	{
	PRINTF_ALWAYS("own poker and combos doesn't match!\n");
	}
	/*
	if( game->known_others_poker) //check others
	{
	if( !check_poker_suit(game->players[UP_PLAYER]->h,game->players[UP_PLAYER]))
	{
	PRINTF_ALWAYS("up's poker and combos doesn't match!\n");
	}
	if( !check_poker_suit(game->players[DOWN_PLAYER]->h,game->players[DOWN_PLAYER]))
	{
	PRINTF_ALWAYS("down's poker and combos doesn't match!\n");
	}
	}
	//*/
	DBG(
		PRINTF(LEVEL,"==END_OF_ROBOT_PLAY TURN %d==\n\n",turn++);
	)

		return !pass;
}



/*提示 可以出的牌;*/
int Hint(LordRobot* robot,int * p)
{
	GAME *game=&robot->game;
	int first= (game->players[UP_PLAYER]->cur==NULL && game->players[DOWN_PLAYER]->cur==NULL);

	if(first)
	{
		*p=-1; 
		return 0;
	}
	else 
	{
		int tmp_card[21];   
		COMBO_OF_POKERS *cur, combo;
		PLAYER * pl= game->players[CUR_PLAYER];
		cur=&combo;			
		memcpy(&tmp_card, 	game->players[CUR_PLAYER]->card ,21*sizeof(int));

		if (!pl->first_hint && find_a_bigger_combo_in_hands(pl->h, cur, &pl->hint_combo) )
		{
			combo_to_int_array(cur,p,tmp_card);
			pl->hint_combo = *cur;	 
			return 1;		  
		}
		else if ( find_a_bigger_combo_in_hands(pl->h, cur, &game->pre) )
		{
			combo_to_int_array(cur,p,tmp_card);
			pl->hint_combo = *cur;	 
			pl->first_hint =0 ;
			return 1;	  
		}
		else{
			*p=-1; 
			return 0;	  	
		}
	}
}

void DebugString(const char* lpszFormat,...)
{
	#ifdef WIN32
	va_list args;
	char szText[1024];

	va_start(args, lpszFormat);
	vsprintf(szText, lpszFormat, args);
	OutputDebugStringA(szText);
	va_end(args);
	#endif //WIN32
}

/*
return:  -1 error 
0 good
*/
int userTakeOutAndTest(LordRobot* robot,int * p/*in*/,int * laizi /*out*/)
{
	GAME *game=&robot->game;
	COMBO_OF_POKERS *cur, combo;
	PLAYER * pl= game->players[CUR_PLAYER];
	PLAYER * downPl= game->players[UP_PLAYER];
	PLAYER * upPl= game->players[DOWN_PLAYER];

	cur=&combo;

	int pass=1,first= (game->players[UP_PLAYER]->cur==NULL && game->players[DOWN_PLAYER]->cur==NULL);

	if(*p!=-1) //error check
	{
		POKERS h;
		read_poker_int(p, &h);
		h.hun = game->hun;

		if(game->hun==-1)
		{
			if ( is_combo(&h,cur))
			{
				if ( first || check_combo_a_Big_than_b(cur,&game->pre) )
				{
					//  return 0;
				}else {	   
					return -2;
				}
			}
			else
			{ 
				return -1;
			}
		}
		else
		{
			if(*laizi==-1)
			{
				int res = check_combo_with_rules(&h,cur,first,&game->pre);
				if(res<0) return res;
				//fill laizi
				POKERS t;
				combo_2_poker(&t, cur);
				for(int i=t.begin; i<=t.end;i++)
				{
					printf("i=%d\n",i);
					//DebugString("i=%d\n",i);
					//std::cout<<"aaa "<<i <<std::endl;
					if(game->hun==i)
					{
						for(int j=0;j<t.hands[i];j++)
							*laizi++=int_2_poker(i);
					}

					// check h...
					if(h.hands[i]<t.hands[i])
					{
						//std::cout<<"aaa "<<h.hands[i]<<"  "<<t.hands[i]<<std::endl;
						printf("h[i] %d t[i] %d\n",h.hands[i],t.hands[i] );
						//DebugString("h[i] %d t[i] %d\n",h.hands[i],t.hands[i] );
						int miss= t.hands[i] - h.hands[i];
						for(int j=0;j<miss;j++)
							*laizi++=int_2_poker(i);			    		
					}

				}
				*laizi=-1;

			}
			else //user define..
			{
				POKERS a,b,c;
				int i,tmp_card[26];
				int num =read_poker_int(p,&a);		
				int hun_num = read_poker_int(laizi,&b);

				if(hun_num!=a.hands[game->hun])
				{
					PRINTF_ALWAYS("[%s][%d]: godie! \n",__FUNCTION__,__LINE__);
				}
				a.hands[game->hun] = 0;

				add_poker(&a, &b, &c);

				if ( is_combo(&c,cur))
				{
					if ( first || check_combo_a_Big_than_b(cur,&game->pre) )
					{
						//  return 0;
					}
					else 	   
						return -2;
				}
				else
				{ 
					return -1;
				}		

			}			
		}
	}
	else
		return -1;

	//update pokers..
	if(game->hun == -1)
	{
		remove_combo_poker(pl->h, cur, NULL);
		remove_combo_poker(upPl->opps, cur,NULL);
		remove_combo_poker(downPl->opps, cur,NULL);
		remove_combo_poker(&game->all, cur,NULL);		
	}
	upPl->oppDown_num -= get_combo_number(cur);

	downPl->oppUp_num -=  get_combo_number(cur);


	PRINTF(VERBOSE,"game[%x] ",game);
	print_card(p);

	while(*p!=-1)
	{
		int found=0;
		for(int i=0; i<21;i++)
		{ 
			if(game->players[CUR_PLAYER]->card[i]==*p)
			{
				int t=game->players[CUR_PLAYER]->card[i];
				t=int_2_poker(t);
				if(game->hun!=-1)
				{
					pl->h->hands[t]--;
					upPl->opps->hands[t]--;
					downPl->opps->hands[t]--;
					game->all.hands[t]--;
				}
				game->players[CUR_PLAYER]->card[i]= -1;
				found=1;
				break;
			}
		}
		if(!found)
			PRINTF_ALWAYS("[%s] fatal error!\n",__FUNCTION__);
		p++;
	}
	sort_poker(pl->h);
	sort_poker(upPl->opps);
	sort_poker(downPl->opps);
	sort_poker(&game->all);


	print_card_2(game->players[CUR_PLAYER]->card,21);
	game->prepre_playernum = game->pre_playernum;
	game->pre_playernum = CUR_PLAYER;



	PRINTF(VERBOSE,"itr@game[%x] own %d\n",game,game->rec.now);
	game->rec.history[game->rec.now].player= CUR_PLAYER;
	game->rec.history[game->rec.now++].h= *cur;
	if(game->pre.type == BOMB ||game->pre.type == ROCKET)
	{
		game->rec.bombed++;
	}
	return 0;
}


//测试用户出的牌是否合要求,返回合格和是否大于上家牌
int test_user_card(LordRobot* robot,int* p)
{
	GAME *game=&robot->game;
	COMBO_OF_POKERS *cur, combo;
	PLAYER * pl= game->players[CUR_PLAYER];
	PLAYER * downPl= game->players[UP_PLAYER];
	PLAYER * upPl= game->players[DOWN_PLAYER];

	cur=&combo;

	int pass=1,first= (game->players[UP_PLAYER]->cur==NULL && game->players[DOWN_PLAYER]->cur==NULL);

	if(*p!=-1) //error check
	{
		POKERS h;
		read_poker_int(p, &h);
		h.hun = game->hun;
		if ( is_combo_hun(&h,cur))
		{
			if(first) return 0;
			else if (  check_combo_a_Big_than_b(cur,&game->pre) )
			{
				return 0;
			}
			return -2;
		}
		else
		{ 
			return -1;
		}
	}
	return -1;	
}
int combo_2_user_combo_define( COMBO_OF_POKERS * cur,int* m_nTypeBomb, int* m_nTypeNum, int*m_nTypeValue )
{
	int num = -1;
	num = get_combo_number(cur);
	*m_nTypeValue = cur->low + 3;
	*m_nTypeBomb = NOT_BOMB(cur) ? 0 : 2;
	switch (cur->type)
	{
	case ROCKET :
		{
			*m_nTypeBomb = 2;
			*m_nTypeValue =16;	
			*m_nTypeNum = 4;
			break;
		}
	case SINGLE_SERIES:
		*m_nTypeNum = cur->len;
		break;
	case PAIRS_SERIES:
		{
			int n = num;
			if (n == 6) //6
			{
				*m_nTypeNum = 222;
			}else if (n == 8)
			{
				*m_nTypeNum = 2222;
			} 
			else if (n == 10)
			{
				*m_nTypeNum = 22222;
			} 
			else if(n == 12)
			{
				*m_nTypeNum = 222222;
			}
			else if(n == 14)
			{
				*m_nTypeNum = 2222222;
			}else if (n == 16)
			{
				*m_nTypeNum = 22222222;
			} 
			else if(n == 18)
			{
				*m_nTypeNum = 222222222;
			}else if( n == 20){
				*m_nTypeNum = 2000000000;
			}else{
				*m_nTypeNum = n;
			}	
			break;
		}

	case THREE_SERIES: 
		{
			int n = num;
			if (n == 6)
			{
				*m_nTypeNum = 33;
			} 
			else if (n == 9)
			{
				*m_nTypeNum = 333;
			} 
			else if(n == 12)
			{
				*m_nTypeNum = 3333;
			}
			else if(n == 15)
			{
				*m_nTypeNum = 33333;
			}if (n == 18)
			{
				*m_nTypeNum = 333333;
			} 
			else
			{
				*m_nTypeNum = n;
			}	
			break;
		}
	case BOMB:
		*m_nTypeNum =4;
		*m_nTypeBomb = cur->three_desc[0]==0 ? 1 : 2;
		if( cur->three_desc[0]==2)
			*m_nTypeNum =17;//pure laizi
		break;
	case THREE:
		*m_nTypeNum =3;
		break;
	case PAIRS:
		*m_nTypeNum=2;
		break;
	case SINGLE:
		*m_nTypeNum=1;
		break;
	case 31: //31
		*m_nTypeNum = 31;
		break;
	case 32: //32
		*m_nTypeNum = 32;
		break;
	case 3311: //3311
		*m_nTypeNum = 3311;
		break;
	case 3322: //3322
		*m_nTypeNum = 3322;
		break;
	case 333111: //333111
		*m_nTypeNum = 333111;
		break;
	case 333222: //333222
		*m_nTypeNum = 333222;
		break;
	case 33332222: //33332222
		*m_nTypeNum = 33332222;
		break;
	case 33331111: //33331111
		*m_nTypeNum = 33331111;
		break;
	case 531: //3333311111
		*m_nTypeNum = 5351;
		break;
	case 411: //411
		*m_nTypeNum =411 ;
		break;
	case 422: //422
		*m_nTypeNum =422;
		break;	
	case NOTHING:
	default:
		*m_nTypeNum = 0;
		break;
	}
	return num;
}


int test_card_value(LordRobot* robot,int* p,int * laizi,int* m_nTypeBomb, int* m_nTypeNum, int*m_nTypeValue)
	//测试出的牌值
{

	COMBO_OF_POKERS *cur, combo;
	cur=&combo;
	if(*p!=-1) //error check
	{
		POKERS h,h1,t;
		read_poker_int(p, &h);
		h.hun=robot->game.hun;
		t=h;
		if(h.hun!=-1)
		{
			h.hands[h.hun]=0;
			read_poker_int(laizi, &h1);
			add_poker(&h,&h1,&h);
		}

		if ( is_combo(&h,cur))
		{
			if(cur->type ==BOMB && t.hun!=-1)
			{
				if(t.hands[t.hun]==4)
					cur->three_desc[0]=2;
				else if (t.hands[t.hun]>0)
					cur->three_desc[0]=0;
				else
					cur->three_desc[0]=1;
			}
			return combo_2_user_combo_define( cur,m_nTypeBomb,m_nTypeNum, m_nTypeValue );
		}
		else
		{ 
			return -1;
		}
	}
	return -1;	
}


//7、销毁机器人
void destoryRobot(LordRobot* robot)
{
	free(robot);
	if(logfile!=NULL)
		fclose(logfile);
	logfile=NULL;
}
char TYPE[3][11]= {"LORD  ", "UPFARM","DNFARM"};
char POS[3][11]= {"CUR  ", "DOWN","UP   "};


void dump_game_record(RECORD *rec)
{
	POKERS p[3],pot;
	//todo: rebuild p[3]..

	PRINTF(LEVEL,"\n\n\n=========GAME RECORD=======\n");

	PRINTF(LEVEL,"===CUR    PLAYER===\n");
	print_hand_poker(&rec->p[CUR_PLAYER]);
	PRINTF(LEVEL,"===DOWN  PLAYER===\n");
	print_hand_poker(&rec->p[DOWN_PLAYER]);
	PRINTF(LEVEL,"===UP       PLAYER===\n");
	print_hand_poker(&rec->p[UP_PLAYER]);
	PRINTF(LEVEL,"===   POT            ===\n");
	print_hand_poker(&rec->pot);

	PRINTF(LEVEL,"===   LORD is  %s    ===\n", rec->lord==CUR_PLAYER? "CUR_PLAYER" :
		(rec->lord==DOWN_PLAYER? "DOWN_PLAYER" : "UP_PLAYER") );

	PRINTF(LEVEL,"===   CUR_PLAYER is  %s    ===\n",TYPE[ rec->type[CUR_PLAYER]]);


	add_poker(&rec->p[rec->lord],&rec->pot,&rec->p[rec->lord]);
	for(int i=0; i<rec->now; i++)
	{
		PLAYER_POS pos= rec->history[i].player;
		//     	    PRINTF(LEVEL,"[%s][%s] " , TYPE[rec->type[pos]], POS[pos]);
		PRINTF(LEVEL,"[%s] " , TYPE[rec->type[pos]]);

		print_combo_poker(&rec->history[i].h);
		PRINTF(LEVEL,"\t\tHANDS:");
		print_hand_poker_in_line(&rec->p[pos]);
		//        if(rec->history[i].h.type!=NOTHING)
		//      remove_combo_poker(&rec->p[pos],
		//                            &rec->history[i].h, NULL);
		PRINTF(LEVEL,"\n");
	}


}

//#if 1//ef BUILD_TEST

char VER[30]="0.99";
GAME_SETTING set;

void DUMP_GAME_FOR_PLAYER(LordRobot * r);

void change_game_setting(GAME * game)
{
	printf("player0 is [%s], 1: computer 0:human, ",game->computer[0]?"Computer":"Human   ");
	scanf("%d",& set.computer[0]);
	printf("player1 is [%s], 1: computer 0:human, ",game->computer[1]?"Computer":"Human   ");
	scanf("%d", &set.computer[1]);
	printf("player2 is [%s], 1: computer 0:human, ",game->computer[2]?"Computer":"Human   ");
	scanf("%d", &set.computer[2]);
	//    set.computer[0]=game->computer[0];
	//   set.computer[1]=game->computer[1];
	//  set.computer[2]=game->computer[2];
}

void test_suit()
{
	char buf[50];
	POKERS dz= {0};
	POKERS all= {0};
	POKERS opp= {0};
	COMBO_OF_POKERS testhands[20];
	COMBOS_SUMMARY s= {0};
	PLAYER hands_dz= {0};
	hands_dz.combos = testhands;
	hands_dz.h = &dz;
	hands_dz.opps = &opp;
	hands_dz.summary =&s;

	GAME *game,g;
	game=&g;


	int t = game->known_others_poker;
	int t2=game->use_best_for_undefined_case; 
	memset(game, 0,sizeof(GAME));
	game->known_others_poker = t;
	game->use_best_for_undefined_case =t2; 	

	game->pot = &game->POT;

	game->computer[0] = 1;
	{
		int j;
		for (j=0; j<3; j++)
		{
			game->suits[j].h = &game->suits[j].p;
			game->suits[j].opps =&game->suits[j].opp;
			game->suits[j].combos =&game->suits[j].combos_store[0];
			game->suits[j].summary =&game->suits[j].combos_summary;
			game->suits[j].oppDown_num = 17;
			game->suits[j].oppUp_num = 17;

		}
	}
	game->players[0]=&game->suits[0];
	game->players[1]=&game->suits[1];
	game->players[2]=&game->suits[2];

	hands_dz.oppDown_num
		=hands_dz.oppUp_num = 17;
	PRINTF(LEVEL,"\n\nput hun:\n");
	scanf("%d",&game->hun);

	PRINTF(LEVEL,"\n\nput pokers:\n");
	scanf("%s",buf);

	read_poker(buf,&dz);
	print_hand_poker(&dz);
	full_poker(&all);
	sub_poker(&all,&dz,&opp);
	sort_poker(&opp);
	PRINTF(LEVEL,"opps:\n");
	print_hand_poker(&opp);

	search_combos_in_suit(game, &dz, &opp, &hands_dz);
	print_suit(&hands_dz);
	printf("\nfor min single..:\n");
	search_combos_in_suits_for_min_single_farmer(game, &dz, hands_dz.combos, &hands_dz);
	print_suit(&hands_dz);
}

int rand_a_card(int *p)
{
	int t;
	do {
		t=rand()%54;
		if(p[t]>0) {
			p[t]--;
			return t;
		}
	} while(1);



}



//dump the game in player's view
void DUMP_GAME_FOR_PLAYER(LordRobot * r)
{
	GAME * game = &r->game;
	if(level>=VERBOSE)
	{
		//clrscr();
		PRINTF(LEVEL,"\n+-----------Current game------------+");
		//PRINTF(LEVEL,"control poker since %c",poker_to_char(game->lowest_bigPoker));
		PRINTF(LEVEL,"\n|CurPLAYER [%10s] [%s] \nHands: ", TYPE[game->player_type], game->computer[0]?"Computer":"Human   ");
		//PRINTF(LEVEL,"\33\33[42;7m test");
		print_hand_poker(game->players[CUR_PLAYER]->h);
		//print real card... for debug
		for(int i=0;i<21;i++)
		{
			if(game->players[CUR_PLAYER]->card[i]!=-1)
				PRINTF(LEVEL,"%d ", game->players[CUR_PLAYER]->card[i]);
		}
		PRINTF(LEVEL,"\n");

		if (1)//(game->computer[0])
			print_suit(game->players[CUR_PLAYER]);
		PRINTF(LEVEL,"\n OPPs: ");
		print_hand_poker(game->players[CUR_PLAYER]->opps);
		///*
		if (game->prepre.type!=NOTHING)
		{
			PRINTF(LEVEL,      "======Pre Pre hands from Player%d []:   ", game->prepre_playernum);
			print_combo_poker(&game->prepre);
			PRINTF(LEVEL,                                            "      ======\n");
		}

		if (game->pre.type!=NOTHING)
		{
			PRINTF(LEVEL,      "======    Pre hands from Player%d []:   ", game->pre_playernum);
			print_combo_poker(&game->pre);
			PRINTF(LEVEL,                                            "      ======\n");
		}
		//     PRINTF(LEVEL,"\nTurn of Player%d [%s]\n", CUR_PLAYER,game->lord==CUR_PLAYER+1?"lord  ":"farmer");

		// */
	}
}

int cards[3][20];
int all_cards[54]; //

void fill_card_from_poker(int * card, POKERS*p)
{
	for(int i=P3; i<=BIG_JOKER; i++)
	{
		for(int j=0;j<p->hands[i];j++)
		{
			if (i>=LIT_JOKER) {
				*card= 52+i-LIT_JOKER;
			}
			else
				*card =i;
			card++;
		}
	}

}

void init_cus_game(GAME * game, GAME_SETTING * set)
{
	for (int k=0; k<54; k++)
	{
		all_cards[k]=1;
	}

	int t = game->known_others_poker;
	int t2= game->use_best_for_undefined_case;
	memset(game, 0,sizeof(GAME));
	game->known_others_poker = t;
	game->use_best_for_undefined_case = t2;

	game->pot = &game->POT;

	game->computer[0] = 1;
	{
		int j;
		for (j=0; j<3; j++)
		{
			game->suits[j].h = &game->suits[j].p;
			game->suits[j].opps =&game->suits[j].opp;
			game->suits[j].combos =&game->suits[j].combos_store[0];
			game->suits[j].summary =&game->suits[j].combos_summary;
			game->suits[j].oppDown_num = 17;
			game->suits[j].oppUp_num = 17;

		}
	}
	game->players[0]=&game->suits[0];
	game->players[1]=&game->suits[1];
	game->players[2]=&game->suits[2];
	game->players[CUR_PLAYER]->use_quick_win_check = game->known_others_poker;

	POKERS all;
	full_poker(&game->all);
	full_poker(&all);

	if (set->cus)
	{
		char buf[50];
RE_INPUT:
		memset(game->players[0]->h,0,sizeof(POKERS));
		memset(game->players[1]->h,0,sizeof(POKERS));
		memset(game->players[2]->h,0,sizeof(POKERS));
		if ( set->cus==2)
		{
			fscanf(set->input,"%s",buf);
			read_poker(buf,game->players[CUR_PLAYER]->h);
			fscanf(set->input,"%s",buf);
			read_poker(buf,game->players[DOWN_PLAYER]->h);
			fscanf(set->input,"%s",buf);
			read_poker(buf,game->players[UP_PLAYER]->h);
		}
		else {
			printf("player0:");
			scanf("%s",buf);
			//memset(game->players[0]->h,0,sizeof(POKER));
			read_poker(buf,game->players[0]->h);
			printf("player1:");
			scanf("%s",buf);
			//memset(game->players[1]->h,0,sizeof(POKER));
			read_poker(buf,game->players[1]->h);
			printf("player2:");
			scanf("%s",buf);
			//memset(game->players[2]->h,0,sizeof(POKER));
			read_poker(buf,game->players[2]->h);
			memset(game->pot,0,sizeof(POKERS));
		}
		POKERS t={0};

		add_poker(game->players[0]->h,game->players[1]->h,&t);
		add_poker(game->players[2]->h,&t,&all);


		for (int i=0; i<=P2; i++)
			if ( all.hands[i]>4) {
				printf("error input\n");
				goto RE_INPUT;
			}
			for (int i=LIT_JOKER; i<=BIG_JOKER; i++)
				if ( all.hands[i]>1) {
					printf("error input\n");
					goto RE_INPUT;
				}
				sort_poker(&all);
	}

	sort_poker(game->players[0]->h);
	sort_poker(game->players[1]->h);
	sort_poker(game->players[2]->h);

	memset(game->players[CUR_PLAYER]->card, -1, sizeof(game->players[CUR_PLAYER]->card));
	memset(game->players[DOWN_PLAYER]->card, -1, sizeof(game->players[DOWN_PLAYER]->card));
	memset(game->players[UP_PLAYER]->card, -1, sizeof(game->players[UP_PLAYER]->card));
	fill_card_from_poker(game->players[CUR_PLAYER]->card,game->players[CUR_PLAYER]->h);
	fill_card_from_poker(game->players[DOWN_PLAYER]->card,game->players[DOWN_PLAYER]->h);
	fill_card_from_poker(game->players[UP_PLAYER]->card,game->players[UP_PLAYER]->h);
	for (int k=0; k<game->players[CUR_PLAYER]->h->total; k++)
	{
		//game->players[CUR_PLAYER]->card[k]= robot_card[i];
	}


	log_brief =1;

	if (set->cus)
	{
		memcpy(&game->all,&all,sizeof(POKERS));
	}
	else
		full_poker(&all);

	sub_poker(&all,game->players[0]->h,game->players[0]->opps);
	sub_poker(&all,game->players[1]->h,game->players[1]->opps);
	sub_poker(&all,game->players[2]->h,game->players[2]->opps);
	int lord=0;
	if (set->cus)
	{
		if (set->cus==2)
		{
			fscanf(set->input,"%d",&lord);
			lord+=2;
			lord %= 3;
			fscanf(set->input,"%d",&game->cur_playernum);
			if (game->cur_playernum >2 ) game->cur_playernum= 2;

			fscanf(set->input,"%d",&game->computer[DOWN_PLAYER]);
			fscanf(set->input,"%d",&game->computer[UP_PLAYER]);
			fscanf(set->input,"%d",&game->computer[CUR_PLAYER]);			
			set->computer[CUR_PLAYER] = game->computer[CUR_PLAYER];
			set->computer[DOWN_PLAYER] = game->computer[DOWN_PLAYER];
			set->computer[UP_PLAYER] = game->computer[UP_PLAYER];
			int laizi;
			fscanf(set->input,"%d",&laizi);
			game->hun=laizi;
		}
		else {
			printf("select lord:");
			scanf("%d",&lord);
			lord++;
			if (lord >3 ) lord= 3;
			printf("first player:");
			scanf("%d",&game->cur_playernum);
			if (game->cur_playernum >2 ) game->cur_playernum= 2;

			change_game_setting(game);
		}
		game->players[0]->oppDown_num = game->players[1]->h->total;
		game->players[1]->oppDown_num = game->players[2]->h->total;
		game->players[2]->oppDown_num = game->players[0]->h->total;
		game->players[2]->oppUp_num = game->players[1]->h->total;
		game->players[0]->oppUp_num = game->players[2]->h->total;
		game->players[1]->oppUp_num = game->players[0]->h->total;


	}

	if (lord==DOWN_PLAYER) {
		game->players[0]->id = LORD;
		game->players[1]->id = DOWNFARMER;
		game->players[2]->id = UPFARMER;
		//    game->players[1]->oppDown_num = 0;
		//   game->players[2]->oppUp_num = 0;
	}
	else if (lord==UP_PLAYER) {
		game->players[1]->id = LORD;
		game->players[2]->id = DOWNFARMER;
		game->players[0]->id = UPFARMER;
		//    game->players[2]->oppDown_num = 0;
		//    game->players[0]->oppUp_num = 0;

	}
	else if (lord==CUR_PLAYER) {
		game->players[2]->id = LORD;
		game->players[0]->id = DOWNFARMER;
		game->players[1]->id = UPFARMER;
		//     game->players[0]->oppDown_num = 0;
		//   game->players[1]->oppUp_num = 0;
	}

	game->players[1]->lower_control_poker=
		game->players[0]->lower_control_poker=
		game->players[2]->lower_control_poker=
		game->lowest_bigPoker = get_lowest_controls(&game->all, CONTROL_POKER_NUM);

	game->player_type = game->players[CUR_PLAYER]->id;

	//    return 1;
}


void read_poker2(char * buf, int * out  )
{
	do {
		switch ( *buf++) {
		case '3':
			*out++ = 0 + 13 * (*buf-48);
			break;
		case '4':
			*out++ = 1 + 13 * (*buf-48);
			break;
		case '5':
			*out++ = 2 + 13 * (*buf-48);
			break;
		case '6':
			*out++ = 3 + 13 * (*buf-48);
			break;
		case '7':
			*out++ = 4 + 13 * (*buf-48);
			break;
		case '8':
			*out++ = 5 + 13 * (*buf-48);
			break;
		case '9':
			*out++ = 6 + 13 * (*buf-48);
			break;
		case 't':
		case 'T':
			*out++ = 7 + 13 * (*buf-48);
			break;
		case 'j':
		case 'J':
			*out++ = 8 + 13 * (*buf-48);
			break;
		case 'q':
		case 'Q':
			*out++ = 9 + 13 * (*buf-48);
			break;
		case 'k':
		case 'K':
			*out++ = 10 + 13 * (*buf-48);
			break;
		case 'a':
		case 'A':
			*out++ = 11 + 13 * (*buf-48);
			break;
		case '2':
			*out++ = 12 + 13 * (*buf-48);
			break;
		case 'x':
		case 'X':
			*out++ = 52 + (*buf-48);
			break;
		default:
			break;
		}
		buf++;
	}	while ( *buf++!='\0');
}

int arrange_poker_from_file2(LordRobot * robot[3], int *pot, FILE* fp, int islaizi)
{
	int laizi = -1;
	char buf[100];
	for (int k=0; k<3; k++)
	{
		fscanf(fp,"%s", buf);
		read_poker2(buf, robot[0]->card[k]);
	}
	fscanf(fp,"%s", buf);
	read_poker2(buf, pot);
	fscanf(fp,"%s",buf);
	//处理地主编号和先出编号
	fscanf(fp,"%s",buf);
	//处理人工出牌还是机器出牌 
	fscanf(fp,"%s",buf);
	read_poker2(buf, &laizi);
	if (-1 == islaizi)
	{
		laizi = -1;
	}
	
	initCard(robot[0],robot[0]->card[0],robot[0]->card[2],robot[0]->card[1]);//robot 0
	initCard(robot[1],robot[0]->card[1],robot[0]->card[0],robot[0]->card[2]);//robot 1
	initCard(robot[2],robot[0]->card[2],robot[0]->card[1],robot[0]->card[0]);//robot 2

	setLaizi(robot[0],laizi);
	setLaizi(robot[1],laizi);
	setLaizi(robot[2],laizi);

	//fill_card_from_poker(robot[0]->game.pot_card, robot[0]->game.pot);
	//fill_card_from_poker(robot[1]->game.pot_card, robot[1]->game.pot);
	//fill_card_from_poker(robot[2]->game.pot_card, robot[2]->game.pot);	 

	//memcpy(robot[0]->card[0], robot[0]->game.players[2]->card, (max_POKER_NUM+1)*sizeof(int));
	//memcpy(robot[0]->card[1], robot[1]->game.players[2]->card, (max_POKER_NUM+1)*sizeof(int));
	//memcpy(robot[0]->card[2], robot[2]->game.players[2]->card, (max_POKER_NUM+1)*sizeof(int));
	//memcpy(pot, robot[0]->game.pot_card, 4*sizeof(int));

	return laizi;
}

int  arrange_poker(LordRobot * robot[3],int *pot, int laizi)
{
#ifdef WIN32
	FILE* fp = fopen("robot_cards.csv", "rt");
#else
	FILE* fp = fopen("/mnt/sdcard/robot_cards.csv", "rb");
#endif
	if(fp == NULL)
	{
		for (int k=0; k<54; k++)
		{
			all_cards[k]=1;
		}
		// if (!set.cus) //rand
		{
			for (int k=0; k<17; k++)
			{
				robot[0]->card[0][k]= rand_a_card(all_cards);
				robot[0]->card[1][k]= rand_a_card(all_cards);
				robot[0]->card[2][k]= rand_a_card(all_cards);
			}
			for (int k=17; k<21; k++)
			{
				robot[0]->card[0][k]= -1;
				robot[0]->card[1][k]= -1;
				robot[0]->card[2][k]= -1;
			}
			for (int k=0; k<3; k++)
			{
				pot[k]= rand_a_card(all_cards);
			}
			pot[3]= -1;
		}
		initCard(robot[0],robot[0]->card[0],robot[0]->card[2],robot[0]->card[1]);//robot 0
		initCard(robot[1],robot[0]->card[1],robot[0]->card[0],robot[0]->card[2]);//robot 1
		initCard(robot[2],robot[0]->card[2],robot[0]->card[1],robot[0]->card[0]);//robot 2
		setLaizi(robot[0],laizi);
		setLaizi(robot[1],laizi);
		setLaizi(robot[2],laizi);
	}else{

		laizi = arrange_poker_from_file2(robot, pot, fp, laizi);
		fclose(fp);
	}

	return laizi;
}



int arrange_poker_from_file(LordRobot * robot[3],int *pot)
{
	for (int k=0; k<54; k++)
	{
		all_cards[k]=1;
	}
	POKERS a,cur={0};
	int laizi=0;
	char buf[100];
	//if ( set->cus==2)
	full_poker(&a);
	for (int k=0; k<3; k++)
	{
		memset(&cur,0,sizeof(POKERS));
		fscanf(set.input,"%s",buf);
		read_poker(buf,&cur);
		sub_poker(&a, &cur, &a);
		fill_card_from_poker( cards[k],&cur);	
	}
	printf("laizi: \n");			
	scanf("%d",&laizi);

	fill_card_from_poker(pot,&a);

	initCard(robot[0],cards[0],cards[2],cards[1]);//robot 0
	initCard(robot[1],cards[1],cards[0],cards[2]);//robot 1
	initCard(robot[2],cards[2],cards[1],cards[0]);//robot 2

	setLaizi(robot[0],laizi);
	setLaizi(robot[1],laizi);
	setLaizi(robot[2],laizi);

	fill_card_from_poker(robot[0]->game.pot_card, robot[0]->game.pot);
	fill_card_from_poker(robot[1]->game.pot_card, robot[1]->game.pot);
	fill_card_from_poker(robot[2]->game.pot_card, robot[2]->game.pot);	  
	return laizi;
}
int determine_lord(LordRobot * robot[3])
{
	//call score
	int lord =-1;
	int score=0;
	//for(int k=0;k<3;k++)
	int m1=showCard(robot[0]);
	int m2=forceLord(robot[0]);
	int m3=doubleGame(robot[0]);

	//    int m2=showCard(robot[1]);
	//   int m3=showCard(robot[2]);
	printf("%d %d %d\n", m1, m1 ,m1);	
	{
		score = callScore(robot[0]);
		getPlayerCallScore(robot[1],score, 0);
		getPlayerCallScore(robot[2],score, 1);
		if(score>0)
			lord=0;
		if(score==3)
		{
			goto BE_LORD;
		}
	}

	{
		score = callScore(robot[1]);
		getPlayerCallScore(robot[2],score, 0);
		getPlayerCallScore(robot[0],score, 1);
		if(score>0)
			lord=1;
		if(score==3)
		{
			goto BE_LORD;
		}
	}

	{
		score = callScore(robot[2]);
		getPlayerCallScore(robot[0],score, 0);
		getPlayerCallScore(robot[1],score, 1);
		if(score>0)
			lord=2;
		if(score==3)
		{
			goto BE_LORD;
		}
	}
	if(lord==-1)
	{
		printf("no one call lord,player0 be lord...\n");
		lord=0;
	}
BE_LORD:
	return lord;
}


int user_play(LordRobot* robot,int * p,int * laizi)
{
#if 1
	GAME * game = &robot->game;
	char buf[100];
	POKERS tmp;
	PLAYER * pl= game->players[CUR_PLAYER];
	PLAYER * downPl= game->players[UP_PLAYER];
	PLAYER * upPl= game->players[DOWN_PLAYER];			 
	POKERS *h = pl->h;

	COMBO_OF_POKERS *combo,hand;
	combo=&hand;
	int pass=1 ,first;
	first = (game->players[UP_PLAYER]->cur==NULL && game->players[DOWN_PLAYER]->cur==NULL);
	while (1) {

		//pass =1;
		//break;
HUMAN_PLAY:
		printf("please select pokers (enter 0 for pass)(enter m for change setting):\n");
		scanf("%s",buf);
		if (*buf=='m')
		{
			change_game_setting(game);
			if (game->computer[CUR_PLAYER])
				return takeOut(robot, p,laizi);
			else
				goto HUMAN_PLAY;

		}
		memset(&tmp,0,sizeof(POKERS));
		read_poker(buf,&tmp);
		sort_poker(&tmp);
		tmp.hun =h->hun;
		if (tmp.total==0)
		{
			if (first) {
				PRINTF(LEVEL,"!!ERR: you cannot pass\n");
				continue;
			}
			pass =1;
			break;
		}
		if (is_sub_poker(&tmp,h))
		{
			if ( is_combo_hun(&tmp,combo))
			{
				if ( first || check_combo_a_Big_than_b(combo,&game->pre) )
				{
					pass=0;
					break;
				}
				else
				{
					PRINTF(LEVEL,"!!ERR:wrong input, too litte! \n");
					continue;
				}
			}
			else
			{
				PRINTF(LEVEL,"!!ERR: wrong input, not a hands\n");
				continue;
			}
		}
		else
		{
			PRINTF(LEVEL,"!!ERR: wrong input, poker in hands is not enough\n");
			continue;
		}
	}
#endif

	COMBO_OF_POKERS * cur = combo;
	if(!pass) {
		if(game->hun==-1)
		{
			//update pokers..
			remove_combo_poker(pl->h, cur, NULL);
			remove_combo_poker(upPl->opps, cur,NULL);
			upPl->oppDown_num -= get_combo_number(cur);
			remove_combo_poker(downPl->opps, cur,NULL);
			downPl->oppUp_num -=  get_combo_number(cur);

			combo_to_int_array(cur,p,game->players[CUR_PLAYER]->card);		    
			remove_combo_poker(&game->all, cur,NULL);		
			laizi[0]=-1;
		}
		else
		{
			POKERS a,b,c;  
			int tmp[26];
			//fixme: for the bomb case, KEEP BOMB OR KEEP hun?
			combo_to_int_array_hun(cur,tmp,pl->card,game->hun);
			convert_to_new_out_format(tmp,p,laizi);	

			int i;
			int num =read_poker_int(p,&a);	
			//update pokers..
			sub_poker(pl->h, &a,pl->h);//cur, NULL);
			//check cards and h
			check_card(pl->h,pl->card);
			sub_poker(upPl->opps, &a,upPl->opps);
			sub_poker(downPl->opps, &a,downPl->opps);	
			sub_poker(&game->all, &a,&game->all);

			upPl->oppDown_num -= get_combo_number(cur);    
			downPl->oppUp_num -=  get_combo_number(cur);



		}

		PRINTF(VERBOSE,"game[%x] ",game);
		print_card(p);
		game->prepre_playernum = game->pre_playernum;
		game->pre_playernum = CUR_PLAYER;
		//research here to get more robust
		/*
		search_combos_in_suit(game, game->players[CUR_PLAYER]->h,  game->players[CUR_PLAYER]->opps,game->players[CUR_PLAYER]);
		search_combos_in_suit(game, game->players[DOWN_PLAYER]->h,  game->players[DOWN_PLAYER]->opps,game->players[DOWN_PLAYER]);
		search_combos_in_suit(game, game->players[UP_PLAYER]->h,  game->players[UP_PLAYER]->opps,game->players[UP_PLAYER]);

		update_players(game,cur); */


		PRINTF(VERBOSE,"itr@game[%x] own %d\n",game,game->rec.now);
		game->rec.history[game->rec.now].player= CUR_PLAYER;
		game->rec.history[game->rec.now++].h= *cur;
		if(game->pre.type == BOMB ||game->pre.type == ROCKET)
		{
			game->rec.bombed++;
		}
		if(*p==-1)
			PRINTF_ALWAYS("ERR:  you play what??\n");

	}
	else
	{
		p[0]=-1;
		laizi[0]=-1;
		game->rec.history[game->rec.now].player= CUR_PLAYER;  
		game->rec.history[game->rec.now++].h.type= NOTHING;  		
	}
	return pass;

}

#ifdef WIN32
int main(int argc, char** argv)
{
	int out1[26],out2[5];
	//GAME game;
	int end=10000,game_number = 0;
	int seed =300000;
	char filename[50];
	set.computer[0]=1;
	set.computer[1]=0;
	set.computer[2]=0;


	int lord_win=0,farmer1_win=0,farmer2_win=0,game_played=0;
	int laizi =-1;
NEW_GAME:
	level=0;
	logfile = NULL;
	logfile2= NULL;
	for_test = 1;
	set.cus=0;
	printf("\n == DouDiZhu Algorithm Test (VER: %s) Powered By ZZF ==\n",VER);
	printf(" == press 0 for exit: \n");
	printf(" == press 1 for a rand new game \n");
	printf(" == press 2 for selecting a new game \n");
	printf(" == press 3 for replay game\n");
	printf(" == press 4 for test a player\n");
	printf(" == press 5 for setting game\n");
	printf(" == press 6 for series testing\n");
	printf(" == press 7 for init a game by costumor\n");
	printf(" == press 8 for read a game from file\n");

	printf("player 0 is %s\n",set.computer[0]?"COM":"HUMAN");
	printf("player 1 is %s\n",set.computer[1]?"COM":"HUMAN");
	printf("player 2 is %s\n\n\n",set.computer[2]?"COM":"HUMAN");

	fflush(stdin);
	if(argc>1)
		end=8;
	else {
		end = 0;
		scanf("%d",&end) ;
	}
	if (end==0)
	{
		exit( 0);
	}
	else if (end==2) {
		printf("input game id:\n");
		scanf("%d",&seed);
		printf("input laizi :\n");
		scanf("%d",&laizi);		
		//set.computer[0]=1;
	}
	else if (end==9) {
	}

	else if (end==1) {
		seed=rand();
		//set.computer[0]=1;
	}
	else if (end==3) {
		;
	}
	else if (end==4) {
		while (1)
			test_suit();

		goto NEW_GAME;
		//PRINTF(LEVEL,"not suported\n");
	}
	else if (end==5)
	{
		printf("player0 is [%s], 1: computer 0:human, ",set.computer[0]?"Computer":"Human   ");
		scanf("%d",& set.computer[0]);
		printf("player1 is [%s], 1: computer 0:human, ",set.computer[1]?"Computer":"Human   ");
		scanf("%d", &set.computer[1]);
		printf("player2 is [%s], 1: computer 0:human, ",set.computer[2]?"Computer":"Human   ");
		scanf("%d", &set.computer[2]);
#if 0
		//	printf("count single/pair with big poker? 1: don't count,0: count ");
		//   scanf("%d", &dapaikongzhi_is_biggest);
		printf("good farmer's number for first half: ");
		scanf("%d", &good_farmer_0);
		good_farmer_0*=10;
		printf("good farmer's number for second half: ");
		scanf("%d", &good_farmer_1);
		good_farmer_1*=10;
#endif
		goto NEW_GAME;
	}
	else if (end==6)
	{
		printf("test from game id: ");
		scanf("%d", &seed);
		printf("how much games to be test  : ");
		scanf("%d", &game_number);
		level = -1;
		printf("test laizi? 1 for laizi, 0 for non_laizi  : ");
		scanf("%d", &laizi);
		if (laizi==0) laizi=-1;	  	
	}
	else if (end==7)
	{
		set.cus=1;
		printf("log id:");
		scanf("%d", &seed);
	}
	else if (end==8)
	{
		set.cus=2;
		if(argc>1)
			strncpy(set.filename,argv[1],999);
		else
		{
			printf("input file:");
			scanf("%s", set.filename);
		}
		if ( (set.input= fopen(set.filename,"rb"))==NULL)
		{
			printf("file could not be open!\n");
			goto NEW_GAME;
		}
		sprintf(filename, "result_%s", set.filename);
		if ( (logfile= fopen(filename,"wb"))==NULL)
		{
			printf("could not open log %s", filename);
			// exit(0);
		}
		sprintf(filename, "brief_result_%s", set.filename);
		if ( (logfile2= fopen(filename,"wb"))==NULL)
		{
			printf("could not open log %s", filename);
			// exit(0);
		}

		goto INIT_GAME;
	}
	else goto NEW_GAME;

LOOP:
	//set.computer[0]=1;

	if(end!=6)
	{
		printf("Begin game %d ,log to %d.txt and brief_%d.txt \n",seed,seed,seed);
		sprintf(filename, "%d.txt", seed);
		if ( (logfile= fopen(filename,"wb"))==NULL)
		{
			printf("could not open log %s", filename);
			// exit(0);
		}
		sprintf(filename, "brief_%d.txt", seed);
		if ( (logfile2= fopen(filename,"wb"))==NULL)
		{
			printf("could not open log %s", filename);
			// exit(0);
		}
	}
	srand(seed);

INIT_GAME:

	log_brief=1;
	PRINTF(LEVEL,"MAIN: === GAME %d start (VER: %s)===\n",seed,VER);
	log_brief=0;


	if(logfile!=NULL)
		fclose(logfile);
	logfile=NULL;
	//set up robot
	LordRobot* robot[3];
	robot[0]=createRobot(3,0);
	robot[1]=createRobot(3,0);
	robot[2]=createRobot(3,seed);
	int lord;
	if (set.cus==2)
	{
		init_cus_game(&robot[0]->game,&set);
		lord = robot[0]->game.cur_playernum;

		DUMP_GAME_FOR_PLAYER(robot[0]);
		GAME_copy(&robot[1]->game,&robot[0]->game);//LordRobot * dst, LordRobot * src)

		PLAYER* tmp = robot[1]->game.players[CUR_PLAYER];
		robot[1]->game.players[CUR_PLAYER] = robot[1]->game.players[DOWN_PLAYER];
		robot[1]->game.players[DOWN_PLAYER] = robot[1]->game.players[UP_PLAYER];	 
		robot[1]->game.players[UP_PLAYER] = tmp;	 	 
		robot[1]->game.player_type = robot[1]->game.players[CUR_PLAYER]->id; 



		Robot_copy(robot[2],robot[0]);
		tmp = robot[2]->game.players[CUR_PLAYER];
		robot[2]->game.players[CUR_PLAYER] = robot[2]->game.players[UP_PLAYER];
		robot[2]->game.players[UP_PLAYER] = robot[2]->game.players[DOWN_PLAYER];	 
		robot[2]->game.players[DOWN_PLAYER] = tmp;	 	 

		robot[2]->game.player_type = robot[2]->game.players[CUR_PLAYER]->id;
		for(int i=0;i<2;i++){
			//  if(game->init==0)
			GAME * game =& robot[i]->game;
			{
				game_init(game);
				game->init=1;
			}
		}

	}
	else
	{
		//fapai
		int pot[10];
		if(end==9) 	{
			if(argc>1)
				strncpy(set.filename,argv[1],999);
			else
			{
				printf("input file:");
				scanf("%s", set.filename);
			}
			if ( (set.input= fopen(set.filename,"rb"))==NULL)
			{
				printf("file could not be open!\n");
				goto NEW_GAME;
			}

			laizi=arrange_poker_from_file(robot,pot);
			fclose(set.input);
		}
		else
		{
			if(laizi!=-1)
				laizi=rand()%13;
			printf("laizi is %d\n",laizi);
			arrange_poker(robot,pot,laizi);
		}

		//determine_lord
		lord = determine_lord(robot);

		PRINTF(LEVEL,"MAIN: === LORD is %d===\n",lord);

		//reservedCard(robot[lord],pot);
		beLord(robot[(lord+0)%3],pot,2);
		beLord(robot[(lord+1)%3],pot,1);
		beLord(robot[(lord+2)%3],pot,0);
	}
PLAY_GAME:
	int winner;	
	{
		COMBO_OF_POKERS *pre=NULL,*cur=NULL,Hands[3];
		//int last = game->lord - 1;
		int first =1;
		int pass1=1,pass2=1,pass3=1;
		pre=&Hands[1];
		cur=&Hands[2];
		if (lord== 0) {
			goto turn_p1;
		}
		if (lord== 1) goto turn_p2;
		if (lord== 2) goto turn_p3;

		while (1)
		{
turn_p1:
			//  PRINTF(LEVEL,"MAIN: === PLAYER0's TURN ===\n");

			//         DUMP_GAME_FOR_PLAYER(robot[0]);
			//		DUMP_GAME_FOR_PLAYER(robot[1]);
			//		DUMP_GAME_FOR_PLAYER(robot[2]);
			if(set.computer[0])
				pass1=takeOut(robot[0],out1,out2);
			else
				pass1 = user_play(robot[0],out1,out2);

			getPlayerTakeOutCards(robot[1],out1,out2,1);
			getPlayerTakeOutCards(robot[2],out1,out2,0);

			if(!pass1)
			{
				PRINTF(LEVEL,"MAIN: === PLAYER0 passed===\n");
			}
			else
			{
				PRINTF(LEVEL,"MAIN: === PLAYER0: " );
				print_combo_poker(&robot[1]->game.pre);
				PRINTF(LEVEL,"===\n" );

			}


			winner=0;
			if(robot[0]->game.players[2]->h->total==0) //game over
				break;
turn_p2:
			//   PRINTF(LEVEL,"MAIN: === PLAYER1's TURN ===\n");

			//         DUMP_GAME_FOR_PLAYER(robot[1]);
			if(set.computer[1])
				pass1=takeOut(robot[1],out1,out2);
			else
				pass1 = user_play(robot[1],out1,out2);

			getPlayerTakeOutCards(robot[2],out1,out2,1);
			getPlayerTakeOutCards(robot[0],out1,out2,0);
			if(!pass1)
			{
				PRINTF(LEVEL,"MAIN: === PLAYER1 passed===\n");
			}
			else
			{
				PRINTF(LEVEL,"MAIN: === PLAYER1: " );
				print_combo_poker(&robot[2]->game.pre);
				PRINTF(LEVEL,"===\n" );

			}				
			winner=1;
			if(robot[1]->game.players[2]->h->total==0) //game over
				break;
turn_p3:
			//  PRINTF(LEVEL,"MAIN: === PLAYER2's TURN ===\n");

			//            DUMP_GAME_FOR_PLAYER(robot[2]);
			if(set.computer[2])
				pass1=takeOut(robot[2],out1,out2);
			else
				pass1 = user_play(robot[2],out1,out2);
			getPlayerTakeOutCards(robot[0],out1,out2,1);
			getPlayerTakeOutCards(robot[1],out1,out2,0);
			if(!pass1)
			{
				PRINTF(LEVEL,"MAIN: === PLAYER2 passed===\n");
			}
			else
			{
				PRINTF(LEVEL,"MAIN: === PLAYER2: " );
				print_combo_poker(&robot[1]->game.pre);
				PRINTF(LEVEL,"===\n" );

			}			
			winner=2;
			if(robot[2]->game.players[2]->h->total==0) //game over
				break;
			//goto turn_p1;
		}
		log_brief =1;
	}

	dump_game_record(&robot[0]->game.rec);
	//destory robot
	destoryRobot(robot[0]);
	destoryRobot(robot[1]);
	destoryRobot(robot[2]);
	//   int res=play_game(&game);
	// if (res==1) goto NEW_GAME;
	lord_win += robot[winner]->game.player_type == LORD;

	game_played	++;
	printf("lord wins %d in %d games\n",lord_win,game_played);
	log_brief=1;
	PRINTF(LEVEL,"=== GAME %d Finished (Ver: %s)===\n",seed,VER);
	log_brief=0;

	fflush(logfile);
	fflush(logfile2);
	if ( (logfile)!=NULL)
		fclose(logfile);
	if ( logfile2!=NULL)
		fclose(logfile2);
	if (game_number >0)
	{
		seed++;
		game_number --;
		goto LOOP;
	}

	goto NEW_GAME;

}
#endif
//#endif
#ifdef _MANAGED
#pragma managed(pop)
#endif

