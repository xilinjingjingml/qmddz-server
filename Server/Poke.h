#pragma once

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
	void ShuffleCards();
	void ShuffleCards(int send_card_type,int send_card_num,int n_double);
	// bool ShuffleCardsDB(int send_card_type,int send_card_num,int n_double);
	bool ShuffleCardsFile(int send_card_type,int send_card_num,int n_double);
	void ShuffleDefaultCards();
	void NewShuffleCards();
	void NewRound();
	void NewRound(int send_card_type,int send_card_num,int n_double);//发牌类型和一次发牌张数
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

	void NewShuffleCardsForNewBie();
	bool ShuffleCardsForNewBieFile();
	void genPackOfCards(vector<CCard>& cards);
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
	int m_BetterSeat;                   //优势座位号
	int m_robotSeat;
	bool m_bSendCardOK[3];				//几个玩家是否发牌完毕
	bool m_bSpring;
	bool m_bReverseSpring;
	CCard m_cLaiziCard;					//随机牌
	vector<int> m_vecNewbieCardsId;
};
