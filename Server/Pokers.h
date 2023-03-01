#pragma once

#include "ddz.h"

#define IS_HUN(h,i)   (i == h->hun)
#define HUN_NUMBER(h)   (h->hun>=0?h->hands[(int)(h->hun)]:0)
#define SET_HUN_TO_i(h,i)  do{\
	h->hands[(int)(h->hun)]--;\
	h->hands[i]++;\
	sort_poker(h);\
}while(0)

#define SET_i_to_HUN(h,i)  do{\
	h->hands[i]--;\
	h->hands[(int)(h->hun)]++;\
	sort_poker(h);\
}while(0)

void full_poker(POKERS* h);
int sort_poker(POKERS * h);
int combo_2_poker(POKERS* hand, COMBO_OF_POKERS * h) /* h must be contained by hand. */;
int cmp_summary( COMBOS_SUMMARY *a,COMBOS_SUMMARY *b) /*return a>b */;
void add_poker(POKERS* a, POKERS * b, POKERS * c ) /* c=a+b */;
int cmp_poker(POKERS* a, POKERS * b ) /* return a!=b */;
BOOL getBomb(POKERS* h, COMBO_OF_POKERS * p) /* */;
BOOL getThree(POKERS* h, COMBO_OF_POKERS * p) /* */;
BOOL getThreeSeries(POKERS* h, COMBO_OF_POKERS * p);
BOOL getDoubleSeries(POKERS* h, COMBO_OF_POKERS * p);
BOOL updateDoubleSeries(POKERS* h, COMBO_OF_POKERS * p);
BOOL updateDoubleSeries1(POKERS* h, COMBO_OF_POKERS * p);
BOOL getSeries(POKERS* h, COMBO_OF_POKERS * p);
BOOL getBigBomb(POKERS* h, COMBO_OF_POKERS * p, COMBO_OF_POKERS * a) /* */;
int browse_pokers(POKERS *h, COMBO_OF_POKERS * pCombos);
int get_combo_number(COMBO_OF_POKERS *h);
// BOOL getBigThree(POKERS* h, COMBO_OF_POKERS * p,COMBO_OF_POKERS* a) /* */;
// BOOL getBigSingle(POKERS* h, COMBO_OF_POKERS * p,int start, int end, int number );
BOOL getBigSeries(POKERS* h, COMBO_OF_POKERS * p,int start, int end, int number, int len ) /* */;
BOOL getSingleSeries(POKERS* h, COMBO_OF_POKERS * p,int start, int end, int number ) /* */;
int search222inSingleSeries(POKERS* h, COMBO_OF_POKERS * p, COMBO_OF_POKERS * s);
int search234inSingleSeries(POKERS* h, COMBO_OF_POKERS * p, COMBO_OF_POKERS * s) /*tobe optimized */;
int searchMultiSingleSeries(POKERS* h, COMBO_OF_POKERS * p) /*tobe optimized */;
int search_general_1(POKERS* h , COMBO_OF_POKERS * pCombos, bool skip_bomb,bool skip_double, bool skip_three, bool skip_series);
int search_general_2(POKERS* h , COMBO_OF_POKERS * pCombos, bool skip_bomb,bool skip_double, bool skip_three, bool skip_series);
int search_general_3(POKERS* h , COMBO_OF_POKERS * pCombos, bool skip_bomb,bool skip_double, bool skip_three, bool skip_series);
COMBO_OF_POKERS* find_max_len_in_combos(COMBO_OF_POKERS* combos, int total);
COMBO_OF_POKERS* find_biggest_in_combos(COMBO_OF_POKERS* combos, int total);
int get_control_poker_num_in_combo(COMBO_OF_POKERS* c, int lower);
void sub_poker(POKERS* a, POKERS * b, POKERS * c ) /* c=a-b */;
int is_sub_poker(POKERS* a, POKERS * b ) ;//a in b ? 1
int remove_combo_poker(POKERS* hand, COMBO_OF_POKERS * h, COMBO_OF_POKERS *h1 ) /* h must be contained by hand. */;
int int_2_poker(INT32 a);
int get_lowest_controls( POKERS *h,int number );
int calc_controls( POKERS *h, POKERS *opp ,int number );
int rand_a_poker(POKERS *all);
int get_pos_char(char *remain,int total);
char poker_to_char(int i);