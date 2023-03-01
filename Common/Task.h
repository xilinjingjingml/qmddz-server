/*
CopyRight(c)2006 by Banding,Shanghai, All Right Reserved.

@Date:	        2009/7/1
@Created:	    10:37
@Filename: 	    Task.h
@File path:	    BDLeyoyoV2\DDZV2\Common 
@Author:		leiliang
@Description:	任务系统
*/

#pragma once
class CPlayer;
class CTask
{
public:
	CTask();
	~CTask();
	
	bool				Init(const char* m_szConfigFile);
	bool				InitTaskCSV(const char* m_szConfigFile);
	//取得任务, 0:没有任务
	void		 		GetTask(TaskItem& task_item);

	TaskItem			GetTask(int task_id_);
	bool				CheckTaskComplete(CPlayer* pPlayer,int nScore);
	int					current_task_id(){return current_task_id_;}
	TaskItem			current_task(){return vecTask_[current_task_id_];}
	bool				CheckLord(CPlayer* pPlayer);
private:

	int					current_task_id_;						//当前任务ID
	vector<TaskItem>	vecTask_;

protected:
	inline double		GetRandNum(int nBase = 1000, double nCount = 10.0);
	
	bool CheckHandCard(CPlayer* pPlayer, int nCard, int nMaxCount,int nScore);
	bool CheckRocket(CPlayer* pPlayer);
	bool CheckRocketWin(CPlayer* pPlayer,int nSCore);
	bool CheckBomb(CPlayer* pPlayer);
	bool CheckBombWin(CPlayer* pPlayer,int nScore);
	bool CheckBombJ(CPlayer* pPlayer);
	bool CheckBombQ(CPlayer* pPlayer,int nScore);
	bool CheckLordCard(CPlayer* pPlayer);
	bool CheckLordCardW(CPlayer* pPlayer);
	bool CheckLordCardWinW(CPlayer* pPlayer,int nScore);
	bool CheckLordCardTwo(CPlayer* pPlayer);
	bool CheckLordCardRocket(CPlayer* pPlayer,int nScore);
	bool CheckLordCardTT(CPlayer* pPlayer);
	bool CheckLordCardShunZi(CPlayer* pPlayer);
	bool CheckDoubleFour(CPlayer* pPlayer,int nScore);
	/************************************************************************/
	/* V0.1.1   新增加5个任务                                               */
	/************************************************************************/
	bool CheckDingTianShun(CPlayer* pPlayer);		//task_id 15 一次打出12张单顺
	bool Check96Times(CPlayer* pPlayer,int nScore);	//task_id 16 倍数在96倍以上获胜
	bool CheckShunZi(CPlayer* pPlayer,int nSCore);	//task_id 17 打出顺子并获胜
	bool CheckLianDui(CPlayer* pPlayer,int nScore);	//task_id 18 打出连对并获胜
	bool CheckSpring(CPlayer* pPlayer,int nScore);	//task_id 19 打出春天牌局


	/************************************************************************/
	/* V0.1.2   新增加6个任务                                               */
	/************************************************************************/
	bool Check4LaiZi(CPlayer* pPlayer,int nSCore);				//手牌抓4张癞子
	bool Check2LaiZiWin(CPlayer* pPlayer,int nSCore);			//手牌抓2张以上癞子并获胜
	bool Check2LaiZi2Win(CPlayer* pPlayer,int nSCore);			//手牌抓2张癞子牌2并获胜
	bool Check4LaiZiWin(CPlayer* pPlayer,int nScore);			//打出纯癞子炸弹并获胜
	bool Check5SoftBombWin(CPlayer* pPlayer,int nScore);		//打出5软炸弹并获胜
	bool CheckLordLaiZiWin(CPlayer* pPlayer,int nSCore);		//底牌抓癞子牌并获胜
};