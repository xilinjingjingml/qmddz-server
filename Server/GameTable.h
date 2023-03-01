/*
 CopyRight(c)2006 by Banding,Shanghai, All Right Reserved.

 @Date:	        2009/4/15
 @Created:	    15:03
 @Filename: 	GameTable.h
 @File path:	BDLeyoyoV2\NewDDZ\Server
 @Author:		leiliang

 @Description:
 */

#pragma once
#include "Poke.h"
#include <set>
#include "ConfigManager.h"
#include "Player.h"

//class CPlayer;
class IPlayerDelegate;

struct play_ackopcode
{
public:
	int	time_;
	pt_cg_play_card_ack ack_;
	play_ackopcode(int time, pt_cg_play_card_ack& ack) : time_(time), ack_(ack){}
};

class CGameTable : public ITableDelegate
{
public:
	enum TIMER_EVENT
	{
		eTIMER_EVENT_NULL = 0,
		eDEALCARD_ANIMATE_EVENT = 1,
		eCALLSCORE_EVENT,
		eROBLORD_EVENT,
		ePLAYCARD_EVENT,
		eSHOWCARD_EVENT,
		eSHOWANI_EVENT,
		ePLAYCARD_AUTO_EVENT,

		ePLAYCARD_EVENT1,
		eDOUBLE_EVENT,
		eSTANDARD_DEFENDER_DOUBLE_EVENT,				// 标准模式下防守者加倍时间
		eSTANDARD_BANKER_DOUBLE_EVENT,					// 标准模式下庄家加倍时间
		ePAUSE_GAME_EVENT,//游戏暂停
		eREADY_EVENT,
		eREADY_PAUSE_EVENT,

		eANI_PERIOD = 3 * 1000,
		eCALL_PERIOD = 15*1000/*20 * 1000  10 * 1000*/,
		eROB_PERIOD = 10*1000/*20 * 1000 10 * 1000*/,
		eFIRST_PLAY_PERIOD = 22 * 1000/*30 * 1000 20 * 1000*/,
		ePLAY_PERIOD = 14 * 1000/*30 * 1000 20 * 1000*/,
		eSHOW_PERIOD = 10*1000/*20 * 1000 10 * 1000*/,
		eSHOWANI_PERIOD = 2 * 1000,
		eDOUBLE_PERIOD = 6*1000,
		ePLAYCARD1_PERIOD=25*1000,
		ePLAYCARD_AUTO_PERIOD= 1000,
		ePAUSE_GAME_PERIOD = 1 * 1000,
		eREADY_PERIOD = 15 * 1000,
		eSTANDARD_CALL_PERIOD = 25 * 1000,				// 标准模式下叫地主时间
		eSTANDARD_DEFENDER_DOUBLE_PERIOPD = 25 * 1000,	// 标准模式下防守者加倍时间
		eSTANDARD_BANKER_DOUBLE_PERIOPD = 25 * 1000,	// 标准模式下庄家加倍时间
	};
	enum 
	{
		//ePLY_NUM = 3,
		eALLUSER = 0,
		eONLYPLAYER = 1,
	};

	enum
	{
		eCHUJICHANG=0,
		eZHONGJICHANG,
		eGAOJICHANG,
		eBISAICHANG,
		eZHIZUNCHANGE,
	};
	enum RecordTpye//记录类型
	{
		eRECORD_CALL,  //叫地主
		eRECORD_LORD,  //当地主
		eRECORD_WIN,   //胜利
		eRECORD_ZHNAJI,//战绩
	};
public:
	CGameTable();
	~CGameTable();
	virtual void BindCoreTable2Table(ITable* pTable);
	virtual void Release();
	virtual void OnTimer(void* pParam);
	virtual void OnStartGame();
	virtual void OnEndGame();
	virtual void OnUserJoinVisitor(IPlayerDelegate* pPlayer);
	virtual void OnUserJoin(IPlayerDelegate* pPlayer);
	virtual void OnUserLeave(IPlayerDelegate* pPlayer);
	virtual void OnUserJoined(IPlayerDelegate* pPlayer);
	virtual void ProcessPacket(const char* pData, int nLength);
public:
	//通过指定的椅子号取得当前这个椅子上的玩家指针
	CPlayer* GetPlayer(int nChairID);

	//分配任务
	void DealingTask();

	//发送初始17张牌
	void Dealing(bool specific);

	//叫分流程
	void CallScore();

	//抢地主的流程
	void RobLord();

	//处理地主拿底牌
	void DealingLord();

	//服务器发送出牌请求
	void SvrPlayCardReq(int nChairID);

	//发送通用消息
	void SendCommonCmd(int nOp, int nChairID = -1, CPlayer* toPlayer = NULL);

	//发送倍数信息
	void SendDoubleInfo();
	void CallScoreAndSendDoubleInfo();

	//是否完成任务
	bool CompleteTask(CPlayer* pPlayer,int nScore);

	//是否完成成就
	void ComplteAchieve(CPlayer* pPlayer,int nScore,bool isCompleteTask,int n_times=0);
	
	// void SaveCardToLocal(int n_double);

	//游戏结束
	void RoundEnd(CPlayer* pPlayer);

	void IsSpring(CPlayer* pPlayer);

	void UpdateTask(CPlayer* pPlayer, int nScore[3]);

	void UpdateTaskH5(CPlayer* pPlayer, int nScore[3]);

	//开始游戏
	void NewRound();

	//获取游戏数据，旁观、断线重连用
	void GetCompleteData(CPlayer *pPlayer);
	/**
	   @brief 房间内广播消息
	   @param packet 数据包 
	   @param nType 0全房间广播, 1玩家广播, 2全体旁观广播
	   @param pExceptPlayer 排除不发玩家
	*/
	template<class T>
	void NotifyRoom(const T& packet, int nType = 0, CPlayer* pExceptPlayer = NULL, bool bSaveReplaydata = true)
	{
		COutputStream os;
		os << packet;

		const char* pData = os.GetData();
		int nLength = os.GetLength();
		if (m_pCoreTable == NULL)
		{
			SERVER_LOG("CGameTable::NotifyRoom m_pCoreTable == NULL");
			return;
		}

		if (nType == 0)//房间广播
		{
			for (int i = 0; i < GetPlayNum(); ++i)
			{
				CPlayer* pPlayer = GetPlayer(i);
				if (!pPlayer)
				{
					continue;
				}
				if (pExceptPlayer && pExceptPlayer == pPlayer)
				{
					continue;
				}
				pPlayer->SendPacket(packet);
			}
			m_pCoreTable->NotifyVisitor(-1, pData, nLength);
		}
		else if (nType == 1)//玩家广播
		{
			for (int i = 0; i < GetPlayNum(); ++i)
			{
				CPlayer* pPlayer = GetPlayer(i);
				if (!pPlayer)
					continue;
				if (pExceptPlayer && pExceptPlayer == pPlayer)
					continue;
				pPlayer->SendPacket(packet);
			}
		}
		else if (nType == 2)
		{
			m_pCoreTable->NotifyVisitor(-1, pData, nLength);
		}
		else if (nType == 3)				//整个服务器发送
		{
			m_pCoreTable->Send2World(pData, nLength);
		}

		if (g_nPrivateRoom == 1 && bSaveReplaydata)
		{
			m_pCoreTable->PushReplayPacket(os.GetData(), os.GetLength());
		}
	}
	template<class T>
	void UpdateToTable(const T& packet)
	{
		if (m_pCoreTable)
		{
			COutputStream os;
			os << packet;
			m_pCoreTable->UpdateToTable(os.GetData(), os.GetLength());
		}
	};
	//创建一个定时器
	void StartTimer(char cChair,int nEvent);
	//检查优势座位
	bool CheckBetterSeatForNewBie(int& nType, int& nDouble );

	//检查优势座位
	bool CheckBetterSeat(int& nType, int& nDouble );

	//检查优势座位
	bool CheckBetterSeatForCleverRobot(int& nType, int& nDouble);

	string GetUniqLabel();
private:

	//开始游戏广播
	void SvrStartGameNot();

	//设置底注
	void SetBaseMoney();
	void SendBaseMoney();

	void ClientTimerNot(char cChairID, int nPeriod);

	// void SaveSpecificCardToLocal();

	void refreshPlayerTokenMoney();
public:
	/**
	@brief	向客户端刷新某玩家牌面信息
	@param	pPalyer 牌面发生变化的玩家
	@param	pExceptPlayer 排除该玩家不发
	@param	bPlyShow 是否向游戏者发送明牌	
	@param	bShowAll 是否向旁观者发送明牌	
	*/
	void OnPacket(const pt_bg_change_basescore_not& noti);
	void RefreshCards(CPlayer* pPlayer, CPlayer* pExceptPlayer = NULL, bool bPlyShow = false, bool bShowAll = false, bool bSaveCard = true);
	void SaveAllCards();
	//玩家出的牌
	void SendPutCards(CPlayer* pPlayer, bool bAll = false);

	//获得下一个出牌玩家
	int GetNextPlayer();
private:
	pt_cg_play_card_ack ack_;					//存放出牌的操作
public:
	ITable* m_pCoreTable;						//系统接口
	CPoke m_Poke;								//牌
	int m_nBaseScore;							//底分
	int m_nTax;									//税
	int m_nWaitOpChairID;						//等待操作的用户位置号
	int m_nSerialID;							//消息序列，确保服务器与客户端消息同步
	bool m_bRacing;								//桌子是否是比赛状态
	bool m_bShowCard;							//判断地主是否亮牌打
	int m_nPutCardsPlayerID;					//当前桌面上出牌最大的玩家
	int m_nCallScore;							//玩家叫的最大分数
	int m_nRobDouble;							//抢地主倍数
	int m_nInitDouble;							//初始倍数
	int m_nDouble;								//当前倍数
	int m_nAddFan;								//加番倍数
	int m_nAddFan2;								//加减翻次数(特殊功能)
	CTask task_;
	int m_currentTime_;							//当前计时

	string			m_strGameLabel;
	bool m_bLordDeal;
	bool m_bStart;//是否开始游戏
	int m_nRobLord;//是否抢地主
	int m_nMaxDouble;//当局最大倍数
	//int	m_setMoney[ePLY_NUM];
public:
	int GetPlayNum() { return m_nPlay_Num; }//得到玩家数目
	int GetLetCardNum() { return m_nLetCardNum; }//得到让牌数目
	void AddLetCardNum();//让牌数目+1
	bool IsCanEndRound(CPlayer* player);//判断牌局是够可以结束
	bool LordCardIsSeries();//地主牌是顺子
	bool LordCardIsSameColor();//地主牌是同花
	bool LordCardIs3();//地主牌是三条
	bool LordCardIsSmall();//地主牌都是小于10
	void sendLordDoubleScore(int nChairID = -1);//通知玩家地主牌信息
	void sendDoubleScore(int nDouble);//玩家加倍
	void StandardSendDoubleScore(int nDouble , CPlayer* pPlayer);// 标准模式下玩家加倍

	//加倍流程
	void DoubleScore();
	// 标准模式下的加倍流程
	void StandardDoubleScore();
	void startPutCard();
	void setDailyCountVal();//设置每日局数对应的优势位权重
	void sendRobotRemainCard(vector<CCard> vecCCard);//向机器人发送没有发送的牌

	void AddRecordTimes(int nChairID, RecordTpye eType, int64 num = 1);//添加记录
	void OnPauseGame(bool bIsReturn = false);
	void PauseEndGame();//异常结束游戏
	void AbnormalEndGame();//异常结束游戏
	void OnPutCardEndPause();
	void SendJuCount(CPlayer* pPlayer = NULL);
	void OnReadyTimeOut();
	void SendReadyTimeOutNot();
	void NotiStartGame();
	void SetRoomConfig();
	void OnGetPrivateRoomResult(CPlayer* pPlayer);

	void NotifyGameModle(CPlayer* pPlayer = NULL);	// 通知比赛模式

	// 当前游戏模式是否为标准模式
	bool is_standard_game() { return g_nIsStandard == 1; }

	bool isTableStatusIn(int s);
	void CheckShowCardInReady();
	void SendShowCardReq(int type, CPlayer* toPlayer = NULL);
	void SendShowCardNot(CPlayer* fromPlayer, CPlayer* toPlayer = NULL);
	void SendDoubleDetail(CPlayer* toPlayer = NULL);

	int setMutiDoubleDetail(int type, int bet);
	int getDoubleDetail(int type = -1);
	int setPlayerDouble(int chairid, int bet);

	void SendLookLordCard(CPlayer* toPlayer = NULL);

	void AfterDealCard();
	
	void calcLordChairID();
private:
	int m_nPlay_Num;//当前牌桌的玩牌人数 斗地主3人 二人斗地主2人
	int m_nLetCardNum;//让牌数目-二人斗地主
	int m_nDoubleNum;//加倍决定人数
	int m_nNoCallNum;//都不要的次数
	map<int,int> m_mapDailyCountVal;

	int	m_startPauseTime;
	int m_nSettleType;//结算方式 默认游戏币结算 0游戏币场 1积分场
	vector<GameStatisc> m_sGameStatic;
	bool m_bHadStart;//游戏是否已经开始过了

	int m_nMaJiangJu;
	bool m_bRoomChecked; //是否已确认私人房规则

	int m_nStandardCallDoubleStep;	// 

	vector<int> m_vecDoubleDetail;	// 详细倍数
	vector<int> m_vecPlayerDouble;	// 玩家倍数
private:
	bool checkPlayersItem(); //检测玩家身上道具是否足够开始游戏
	void LeftCardDouble(CPlayer* pPlayer);
};
