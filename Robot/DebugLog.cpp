#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "ddz.h"
#include "DebugLog.h"


#ifdef __INIT_DEBUG

#ifdef DEBUG

FILE* logfile,*logfile2;
int level=0,log_brief=0; /* 0 for ouput basic */
int for_test=0;

#define FUNC_NAME PRINTF(	,"ENTER %s\n",__FUNCTION__);

void PRINTF(int a,const char* const b, ...) {

    char   buffer[1024] = {0};

    va_list args;
    /* start or argument list */
    va_start(args, b);
    /* add in only format portion */
    vsprintf(&(buffer[strlen(buffer)]), (char *) b, args);

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

    char   buffer[1024] = {0};
    va_list args;
    /* start or argument list */
    va_start(args, b);
    /* add in only format portion */
    vsprintf(&(buffer[strlen(buffer)]), (char *) b, args);

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
#else

void PRINTF(int a,const char* const b, ...) {}
void PRINTF_ALWAYS(const char* const b, ...) {
    while(1);
}
#endif //DEBUG
#endif //__INIT_DEBUG

#ifdef __INIT_DEBUG
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
    for (int i=0; i<MAX_POKER_KIND; i++)
    {
        if (h->hands[i]<0)
            PRINTF(LEVEL," ERR ");
        for (int j=0 ; j<h->hands[i]; j++)
            print_one_poker(i);
    }
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

    for (int k=0; k<number; k++)
    {
        print_combo_poker(&combos[k]);
    }
    PRINTF(LEVEL,"\n");
    ;
}
void print_card_2(int * p,int num)
{
    char t[4]= {'d','c','h','s'};
    PRINTF(LEVEL,"CARD: ");
    int i=0;
    while(i++<num)
    {
        if(*p==-1) {
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
    for (int k=0; k<s->singles_num; k++)
    {
        print_combo_poker(s->singles[k]);
    }
    for (int k=0; k<s->pairs_num; k++)
    {
        print_combo_poker(s->pairs[k]);
    }
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
    for (int k=0; k<s->series_num; k++)
    {
        print_combo_poker(s->series[k]);
    }
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

char TYPE[3][11]= {"LORD  ", "UPFARM","DNFARM"};
char POS[3][11]= {"CUR  ", "DOWN","UP   "};


void dump_game_record(RECORD *rec)
{
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


	//add_poker(&rec->p[rec->lord],&rec->pot,&rec->p[rec->lord]);
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


void dumpGame(lordGame *g)
{
	PRINTF(LEVEL,"+++++++++++++\n");

	for(int i=0; i< 3 ; i++) {
		PRINTF(LEVEL,"player%d: ",i);
		print_hand_poker_in_line(&g->bombs[i]);
		PRINTF(LEVEL," ");
		print_hand_poker_in_line(&g->other_pokers[i]);
		PRINTF(LEVEL,"\n");
	}
	PRINTF(LEVEL,"pot: ");
	print_hand_poker_in_line(&g->other_pokers[3]);
	PRINTF(LEVEL,"\n+++++++++++++\n");
}
//dump the game in player's view
void DUMP_GAME_FOR_PLAYER(GAME * game)
{
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
			if(game->players[CUR_PLAYER]->card[i]!=-1){
				PRINTF(LEVEL,"%d ", game->players[CUR_PLAYER]->card[i]);
			}
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
#endif



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



