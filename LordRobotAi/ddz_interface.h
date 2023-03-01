// It's used for Horn Games
// The interfaces for DDZ ai.
#include "ddz.h"

#ifdef LORDROBOTAI_EXPORTS
#define LORDROBOTAI_API __declspec(dllexport)
#else
#define LORDROBOTAI_API
//#define LORDROBOTAI_API __declspec(dllimport)
#endif


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
} LordRobot;


//初始化机器人，返回机器人指针
//参数 0不看其他玩家的牌  1看其他玩家的牌
LORDROBOTAI_API LordRobot* createRobot(int,int);


LORDROBOTAI_API void shuf_poker(LordRobot* robot,int *pot,int laizi);
LORDROBOTAI_API int  arrange_poker(LordRobot * robot[3],int *pot,int laizi);
/*
参数为int[] 数组 ，表示为发到机器人手中的牌。牌值如下：
	3	4	5	6	7	8	9	10	J	Q	K	A	2
方	0	1	2	3	4	5	6	7	8	9	10	11	12
梅	13	14	15	16	17	18	19	20	21	22	23	24	25
红	26	27	28	29	30	31	32	33	34	35	36	37	38
黑	39	40	41	42	43	44	45	46	47	48	49	50	51
*/
LORDROBOTAI_API void initCard(LordRobot* robot, int* robot_card, int* up_player_card, int* down_player_card);

//传出参数为本机器人叫分
//-1 表示不叫
LORDROBOTAI_API int callScore(LordRobot* robot);

// 1 表示加倍，0 表示不加倍
LORDROBOTAI_API int doubleGame(LordRobot* robot);

// bit 0-7:    0 表示当地主赢不了， 3表示赢概率比较大， 2 次之， 1 更次
// bit 8-15:  0 表示当上家农民赢不了， 3表示赢概率比较大， 2 次之， 1 更次
// bit 16-23: 0 表示当下家农民赢不了， 3表示赢概率比较大， 2 次之， 1 更次

LORDROBOTAI_API int showCard(LordRobot* robot);




//1 表示抢地主，0 表示不抢
LORDROBOTAI_API int forceLord(LordRobot* robot);

/*其他玩家出牌信息：
Player_number, 0: down player,
               1: up player.
参数同1、发牌，如果其他玩家未出牌，则传入null
*/
LORDROBOTAI_API void getPlayerTakeOutCards(LordRobot* robot,int* poker, int* laizi, int player_number) ;

/* 5、其他玩家叫分信息：
Player_number, 0: down player,
               1: up player.
score,  分值, -1表示没叫
*/
LORDROBOTAI_API void getPlayerCallScore(LordRobot* robot,int score, int player_number) ;

LORDROBOTAI_API void beLord(LordRobot* robot,int *pot,int player_number);

/*6、出牌：1表示pass，0表示出牌，牌组信息放在P里*/
LORDROBOTAI_API int  takeOut(LordRobot* robot,int * p, int* laizi);
LORDROBOTAI_API int userTakeOutAndTest(LordRobot* robot,int * p, int* laizi);
//7、销毁机器人
LORDROBOTAI_API void destoryRobot(LordRobot* robot);
//提示 可以出的牌;
LORDROBOTAI_API int Hint(LordRobot* robot,int * p);

LORDROBOTAI_API int test_card_value(LordRobot* robot,int* p, int* laizi, int* m_nTypeBomb, int* m_nTypeNum, int*m_nTypeValue);

LORDROBOTAI_API int test_user_card(LordRobot* robot,int* p);
