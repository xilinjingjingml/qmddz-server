// It's used for Horn Games
// The interfaces for DDZ ai.
#pragma once 

#include "ddz.h"

#define LORD_LEVEL_3   (0)
#define LORD_LEVEL_2     (0)
#define UPFARMER_LEVEL_3   (0)
#define UPFARMER_LEVEL_2     (0)
#define DOWNFARMER_LEVEL_3   (0)
#define DOWNFARMER_LEVEL_2     (0)
typedef struct
{
    GAME game;
    int  card[3][21];
    int  pot[4];
	int  outs[21];
	int  laizi[5];
	int  id;
} LordRobot;
typedef void (*PLOG)(const char* fmt, ...);

void print_history_content(LordRobot *robot, void (*pLog)(const char* fmt, ...));
void print_game_record(LordRobot *robot,void (*pLog)(const char* fmt, ...));
void print_history_content2(LordRobot *robot, void (*pLog)(const char* fmt, ...));
//初始化机器人，返回机器人指针
//参数 0不看其他玩家的牌  1看其他玩家的牌
LordRobot* createRobot(int,int);

int arrange_poker(LordRobot * robot[3], int *pot,int laizi);
int arrange_poker_special(LordRobot * robot[3],  int *pot, int laizi, int *card_list);
int arrange_poker_request(Request* req, int *card_list, int laizi);
/*
参数为int[] 数组 ，表示为发到机器人手中的牌。牌值如下：
	3	4	5	6	7	8	9	10	J	Q	K	A	2
方	0	1	2	3	4	5	6	7	8	9	10	11	12
梅	13	14	15	16	17	18	19	20	21	22	23	24	25
红	26	27	28	29	30	31	32	33	34	35	36	37	38
黑	39	40	41	42	43	44	45	46	47	48	49	50	51
*/
void initCard(LordRobot* robot, int* robot_card, int* up_player_card, int* down_player_card);
void  reinitCard(LordRobot * robot[3],int *pot, int laizi);
//传出参数为本机器人叫分
//-1 表示不叫
int callScore(LordRobot* robot);

// 1 表示加倍，0 表示不加倍
int doubleGame(LordRobot* robot);

// bit 0-7:    0 表示当地主赢不了， 3表示赢概率比较大， 2 次之， 1 更次
// bit 8-15:  0 表示当上家农民赢不了， 3表示赢概率比较大， 2 次之， 1 更次
// bit 16-23: 0 表示当下家农民赢不了， 3表示赢概率比较大， 2 次之， 1 更次

int showCard(LordRobot* robot);

void setLaizi(LordRobot* robot, int laizi);
int  setRobotLevel(LordRobot* robot,int level);
// 0: is OK, k:error card
int check_all_card_is_ok(LordRobot* robot);
//1 表示抢地主，0 表示不抢
int forceLord(LordRobot* robot);

/*其他玩家出牌信息：
Player_number, 0: down player,
               1: up player.
参数同1、发牌，如果其他玩家未出牌，则传入null
*/
void getPlayerTakeOutCards(LordRobot* robot,int* poker, int* laizi, int player_number) ;

/* 5、其他玩家叫分信息：
Player_number, 0: down player,
               1: up player.
score,  分值, -1表示没叫
*/
void getPlayerCallScore(LordRobot* robot,int score, int player_number) ;

void beLord(LordRobot* robot,int *pot,int player_number);

/*6、出牌：1表示pass，0表示出牌，牌组信息放在P里*/
int  takeOut(LordRobot* robot,int * p, int* laizi);

int userTakeOutAndTest(LordRobot* robot,int * p, int* laizi);

//7、销毁机器人
void destoryRobot(LordRobot* robot);
//提示 可以出的牌;
int Hint(LordRobot* robot,int * p);

int test_card_value(LordRobot* robot,int* p,int*laizi, int* m_nTypeBomb, int* m_nTypeNum, int*m_nTypeValue);

int test_user_card(LordRobot* robot,int* p);

bool  InitCardRandom(Request& r, int type , int *Card , int *Laizi);
bool GetRandomParams(Request* res,
					 double avg_bomb_multiple_config, //1-8
					 int better_seat_config,  //0: 不调整, 1 : 低 2:中 3 :高
					 int card_tidiness_config,  //0: 不调整, 1 : 低 2:中 3 :高
					 int base_good_config     //0: 不调整, 1 : 低 2:中 3 :高
					 );