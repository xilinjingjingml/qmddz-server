#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_POKER_KIND 15
#define max_POKER_NUM 20
#define CONTROL_POKER_NUM 6
#define MAX_COMBO_NUM 20
typedef int BOOL;
typedef int INT32;
#define true 1
#define false 0

enum poker_type {
    P3, //0
    P4,
    P5,
    P6,
    P7,
    P8,
    P9,
    P10,
    Pj,
    Pq,
    Pk,
    Pa,
    P2,
    LIT_JOKER,
    BIG_JOKER,	 //14
};

typedef enum PLAYER_TYPE {
    LORD,
    UPFARMER,
    DOWNFARMER,
} PLAYE_TYPE;


//don't CHANGE value in the following!!
typedef enum PLAYER_POS {
    DOWN_PLAYER =0 ,
    UP_PLAYER =1,
    CUR_PLAYER=2,
}PLAYER_POS;


typedef struct POKERS
{
    char hands[MAX_POKER_KIND];
    char total;
    char begin;
    char end;
    char hun;
    char hun_number;	
    char control;
} POKERS;

typedef enum HANDS_TYPE {
    NOTHING,
    ROCKET=5,
    BOMB=4,
    SINGLE_SERIES=11,
    PAIRS_SERIES=22,
    THREE_SERIES=30,
    THREE =3,
    PAIRS =2,
    SINGLE =1,
    THREE_ONE = 31,
    THREE_TWO = 32,
    //warning , plane is from 3311..

} HANDS_TYPE;

typedef struct COMBO_OF_POKERS
{
    unsigned int type;
    //char number;
    char low; // the lowest poker
    char len; // length for the series
    char control; // is it a control
    char three_desc[5]; //for three and 411,422
                                 // if it's bomb,three_desc[0] : 0, 带混诈，1 不带混诈，2 纯混诈
} COMBO_OF_POKERS;

typedef struct
{
    char single;
    char pair;
    char series;
    char three;
    char bomb;
    char ctrl;
} CONTROLS;

typedef struct COMBOS_SUMMARY
{
    char extra_bomb_ctrl;
    char combo_total_num; //exclude biggest combos
    char real_total_num; // include  biggest combos

    char combo_typenum;
    char combo_with_2_controls;

    char control_num;
	CONTROLS ctrl;

    char singles_num;
    COMBO_OF_POKERS * singles[MAX_POKER_KIND];
    char pairs_num;
    COMBO_OF_POKERS * pairs[MAX_POKER_KIND];

    char three_num;
    COMBO_OF_POKERS * three[MAX_POKER_KIND];

    char series_num;
    COMBO_OF_POKERS * series[6];
    char series_detail[10];

    char bomb_num;
    COMBO_OF_POKERS * bomb[6];

    char three_one_num;
    COMBO_OF_POKERS * three_one[6];

    char three_two_num;
    COMBO_OF_POKERS * three_two[6];

    char threeseries_one_num;
    COMBO_OF_POKERS * threeseries_one[3];

    char threeseries_two_num;
    COMBO_OF_POKERS * threeseries_two[3];

    char threeseries_num;
    COMBO_OF_POKERS * threeseries[3];

    char pairs_series_num;
    COMBO_OF_POKERS * pairs_series[3];
    char biggest_num;
    COMBO_OF_POKERS * biggest[20];
    char not_biggest_num;
    COMBO_OF_POKERS * not_biggest[20];
    char four_one_num;
    COMBO_OF_POKERS * four_one[4];
    char four_two_num;
    COMBO_OF_POKERS * four_two[3];

    char combo_smallest_single;
} COMBOS_SUMMARY;

typedef struct {
    BOOL has_series;
    BOOL has_double_series_begin;
    BOOL has_three_series_begin;
    BOOL has_bomb; //donot contact rocket.
    BOOL has_three;
} HAS_COMBOS_SUMMARY;

typedef struct
{
    POKERS *h; // current poker in hands.
    POKERS *opps;
    char oppUp_num,oppDown_num; //number of opps hands
    char lower_control_poker;
    PLAYER_TYPE id; //
    char good_framer; //1:good
    BOOL need_research;
     int chuntian; //是否可能被对方春天
    COMBO_OF_POKERS *combos;
    COMBOS_SUMMARY* summary;
//    CONTROLS max_control; //max control for all perhaps combos
    int  computer; //1  1:com 0:player
    BOOL use_quick_win_check;
    char score;
    //POKERS* poker;
    //COMBO_OF_POKERS* history[20];
    //char currComb;
    COMBO_OF_POKERS *cur;
    COMBO_OF_POKERS curcomb;//to be deleted

    POKERS * Up;      //the poker of Up player has outputted
    POKERS * Down; //the poker of Down player has outputted
    POKERS * me;    //the poker of  cur player has outputted

    HAS_COMBOS_SUMMARY all; // to speed up..
    COMBO_OF_POKERS Perhaps[20];
    COMBO_OF_POKERS combos_store[MAX_COMBO_NUM];	
    COMBOS_SUMMARY combos_summary;	
    int card[max_POKER_NUM+1]; //card
    POKERS p;
    POKERS opp;   	
    int first_hint;
    COMBO_OF_POKERS hint_combo; 
} PLAYER;

typedef struct
{
    COMBO_OF_POKERS h;
    PLAYER_POS player;
} HISTORY_HAND;

#define CALL_MAX_TIME 10
typedef struct
{
	int player[CALL_MAX_TIME];
	int score[CALL_MAX_TIME];
	int cur_num;
	int double_times;
	int cur_lord;
}CALL_HISTORY;

typedef struct
{
    POKERS pot;
    POKERS p[3];
    HISTORY_HAND history[180];
    int score[3];	
    PLAYER_TYPE type[3];	
    int first_caller;	
	CALL_HISTORY call;
    int lord;
    int winner;
    int bombed;
    int now;	
} RECORD;

typedef struct {
    int computer[3];
    int dapaikongzhi_is_biggest;
    int cus;
    char filename[1000];
    FILE* input;
} GAME_SETTING;

typedef struct
{
    POKERS *pot;
    POKERS POT;	
    POKERS all;
    int pot_card[4];
    int  lowest_bigPoker; // the first 6 biggest poker.
    
    int computer[3];
    COMBO_OF_POKERS prepre,pre,cur;
    int prepre_playernum,pre_playernum;
    int cur_playernum;
    PLAYER * players[3]; // for three playes....
    PLAYER suits[3];  //

    COMBO_OF_POKERS tmp_combos[4][25];	
    COMBO_OF_POKERS c[5]; //use for internal...		
    COMBOS_SUMMARY tmp_summary[4];
    CALL_HISTORY call;
    RECORD rec;
    //global
    PLAYE_TYPE player_type; //lord or farmer;
    int init;
    int first_half;
    int known_others_poker;
    int refer_others_poker; //for calculate the biggest combos.
    int use_best_for_undefined_case;
    COMBO_OF_POKERS tmp_for_best;//to be removed
    int good_farmer_0;
    int good_farmer_1 ;
    int hun;

} GAME;

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#define CEIL_A(a) ( (a)>(Pa)?(Pa):(a))
//#define DBG(A) A
#define DBG(A) 

typedef struct Request
{
	int bomb_multiple;              // 炸弹导致的平均倍数
	int better_seat;				// 优势座位号：0,1,2   无优势：3
	int card_tidiness;				// 手牌的整齐度 0:不调整 1:调整
	int call_lord_first;			// 当局先叫地主的座位号：0,1,2
	int base_good;				    // 底牌关键张，针对先叫地主的座位手牌 0:不调整 1:调整
}Request;

typedef struct lordGame
{
	POKERS pot;
	POKERS bombs[3];
	POKERS other_pokers[4];
	POKERS all;
	int bomb_number[3];
	//COMBO_OF_POKERS combos[2][25];
	//COMBOS_SUMMARY summary[2];
	COMBO_OF_POKERS cs[3][20];
	COMBOS_SUMMARY smry[3];
	int hun;
} lordGame;














