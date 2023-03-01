#pragma once
#include "ddz.h"

//#define __INIT_DEBUG
#define VERBOSE 0
#define LEVEL 0

#ifdef WIN32 
//#define	scanf scanf_s
//#define fscanf fscanf_s
//#define vsprintf vsprintf_s
//#define sprintf sprintf_s
#else 
#endif
//#define __INIT_DEBUG 
//#define _INIT_TEST
#define DBG(A)

#ifdef __INIT_DEBUG
extern FILE* logfile,*logfile2;
extern int level,log_brief;
extern int for_test;
#define KEY_INFO 0
#define DEBUG

void PRINTF(int a,const char* const b, ...);
void PRINTF_ALWAYS(const char* const b, ...);
#else

#define PRINTF(a,b,...) 
#define PRINTF_ALWAYS(b,...) 
#endif

#define FUNC_NAME PRINTF(LEVEL,"ENTER %s\n",__FUNCTION__);

#ifdef __INIT_DEBUG
void print_hand_poker_in_line(POKERS * h );
void print_one_poker(int i);
void print_summary( COMBOS_SUMMARY * s);
void print_suit(PLAYER * player);
void print_combo_poker(COMBO_OF_POKERS * h /*, COMBO_OF_POKERS *h1 for three*/  );
void print_all_combos(COMBO_OF_POKERS *combos, int number);
void print_hand_poker(POKERS * h );
void print_card(int * p);
void print_card_2(int * p,int num);
void DUMP_GAME_FOR_PLAYER(GAME * game);
void dump_game_record(RECORD *rec);
void dumpGame(lordGame *g);
#else
inline void print_hand_poker_in_line(POKERS * h ) {}
inline void print_one_poker(int i) {}
inline void print_summary( COMBOS_SUMMARY * s) {}
inline void print_suit(PLAYER * player){}
inline void print_combo_poker(COMBO_OF_POKERS * h /*, COMBO_OF_POKERS *h1 for three*/  ){}
inline void print_all_combos(COMBO_OF_POKERS *combos, int number){}
inline void print_hand_poker(POKERS * h ){}
inline void print_card(int * p){}
inline void print_card_2(int * p,int num){}
inline void DUMP_GAME_FOR_PLAYER(GAME * game){}
inline void dump_game_record(RECORD *rec){}
inline void dumpGame(lordGame *g){}
#endif

char poker_to_char(int i);
