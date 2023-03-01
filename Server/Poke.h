#pragma once
#include "ConfigManager.h"

struct seriesCardsTemp
{
	int cards_size;
	int cards_start;
	int cards_end;
	int cards_type;
};

enum seriesCardsEmu {
	SERIES_SHUNZI = 1,
	SERIES_LIANDUI,
	SERIES_FEIJI,
	SERIES_BOMB
};

class CPoke
{
public:
	CPoke();
	~CPoke();
public:
	void NewRound();
	void SetDefaultLord(int chairid = -1);
	void SetZhaDanCard(int chairid = -1);			//设置炸弹
	int  GetZhaDanValue();			//获取单个炸弹值
	void SetSendCardNum();			//设置发牌连续张数
	void SortLordCards();
	CCard& GetCard(int nPos);
	CCard& GetCard();
	CCard ConverCard(int card);
	int SetLaiziValue( CCard& card);
	int checkCardIsOk(int* card);
	int GetLordCardType();
	int checkCardHas34(int* card);
	int checkLowCardIsOk(int* card);

	void ShuffleDefaultCards();
	void ShuffleCards(int nBetterSeat, int nRobotSeat);
	bool ShuffleCardsFile();
	void ShuffleCardsForNewBie(int nBetterSeat);
	bool ShuffleCardsForNewBieFile(int nBetterSeat, vector<int>& vecIds);
	bool ShuffleCardsForNewBieFileOrder(int nBetterSeat, int nOrderIndex);
	void ShuffleCardsForBaiYuanMatchCard(int nBetterSeat, int nRobotSeat);
	bool ShuffleCardsForBaiYuanMustLose(int nBetterSeat, int nRobotSeat, bool bUseMaxConfig);
	void genPackOfCards(vector<CCard>& cards);
	void genSeriesCards(vector<CCard>& srcCards, vector<CCard>& dstCards, const structMustLoseCardConfig& sConfig, vector<bool>& vecUsedPoint);
	void genSeriesCards(vector<CCard>& srcCards, vector<CCard>& dstCards, seriesCardsTemp SeriesTemp);
public:
	vector<CCard> m_AllCards;			//全副牌54张
	vector<CCard> m_cLordCard;			//地主补的三张牌
	vector<int> m_zhadan;
	int m_nRanCardPos;					//随机牌的位置
	int m_nDefaultLord;					//默认地主
	int m_nCurrentLord;					//当前地主
	int m_nBombCounter;					//计算炸弹个数
	int m_nNextPlayer;					//下一个出牌玩家
	int m_nDecideLordRound;				//决定地主计数
	bool m_bSendCardOK[3];				//几个玩家是否发牌完毕
	bool m_bSpring;
	bool m_bReverseSpring;
	CCard m_cLaiziCard;					//随机牌
};
