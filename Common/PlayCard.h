/*
 CopyRight(c)2006 by Banding,Shanghai, All Right Reserved.

 @Date:	        2009/4/20
 @Created:	    10:37
 @Filename: 	Playcard.h
 @File path:	BDLeyoyoV2\DDZV2\Common 
 @Author:		leiliang
	
 @Description:	牌型数据结构,目前各地区规则不同，通过读规则表来选规则
				现有算法为较笨的穷举，简单易懂，正在考虑模式匹配算法改进
*/

#pragma once

/*	牌型及牌值定义

牌型(Cards_Type)						牌值(Cards_Value)           牌数:

单牌:		 1							CCard.Value(面值)           1
一对:		 2							CCard.Value(面值)			2
三张:		 3							CCard.Value(面值)			3

三带一:		 31							三张的Card.Value(面值)		4
四张:		 4							CCard.Value(面值)			4

单顺:		 5							最小牌的Card.Value(面值)    5
三带一对:	 32							三张的Card.Value(面值)		5		

单顺:		 6							最小牌的Card.Value(面值)    6
双顺:		 222						最小牌的Card.Value(面值)    6
三顺:	     33							最小牌的Card.Value(面值)    6
四带二单:	 411						四张的Card.Value(面值)		6	 

单顺:		 7							最小牌的Card.Value(面值)    7

单顺:		 8							最小牌的Card.Value(面值)    8
双顺		 2222						最小牌的Card.Value(面值)    8
三顺带二单:  3311						最小三张的Card.Value(面值)  8
四带二对:	 422						四张的Card.Value(面值)		8

单顺:		 9							最小牌的Card.Value(面值)    9
三顺:		 333						最小三张的Card.Value(面值)  9

单顺:		 10							最小牌的Card.Value(面值)    10
双顺:		 22222						最小牌的Card.Value(面值)    10
三顺带二对:  3322						最小三张的Card.Value(面值)  10

单顺:		 11							最小牌的Card.Value(面值)    11

单顺:		 12							最小牌的Card.Value(面值)    12
双顺:		 222222						最小对牌的Card.Value(面值)  12
三顺:		 3333						最小三张的Card.Value(面值)  12
三顺带三:	 333111						最小三张的Card.Value(面值)  12

双顺		 2222222					最小对牌的Card.Value(面值)  14

三顺带三对:  333222						最小三张的Card.Value(面值)  15
三顺:		 33333						最小三张的Card.Value(面值)  15

双顺		 22222222					最小对牌的Card.Value(面值)  16
三顺带四单:  33331111					最小三张的Card.Value(面值)  16

双顺		 222222222					最小对牌的Card.Value(面值)  18
三顺		 333333						最小三张的Card.Value(面值)  18

双顺		 2222222222					最小对牌的Card.Value(面值)  20
三顺带五单:	 3333311111					最小三张的Card.Value(面值)  20
三顺带四对:  33332222					最小三张的Card.Value(面值)  20

*/
#define MAXCARDSNUM		20
#define MAXTYPENUM		17
#define BAO				17

class CPlayCards
{
	typedef vector<CCard> VECCARD;
public:
	CPlayCards();
	~CPlayCards();

	VECCARD m_cCards;					//未出的牌;
	VECCARD m_cChoosingCards;			//选择要出的牌
	VECCARD m_cDiscarding;				//客户端用(刚出的牌)
	vector<VECCARD> m_vecTipsCards;		//牌型提示
	VECCARD	m_tempCard;

	CCardsType m_cDiscardingType;		//刚出牌的牌型
	int m_nDiscardingCardsTable[20];	//记录出牌类型的扫描表

	unsigned long m_dwRule;				//游戏规则
	int	m_nBaoValue;					//宝牌的值

public:
	void SetRule(unsigned long dwRule)
	{
		m_dwRule = dwRule;
	};

	int IsSoftBomb();	  //软炸弹 
	int IsHardBomb();	  //硬炸弹判断是否是四张(炸弹); 
	int Tips(CCardsType& cCardsType);	//提示
	int Tips();	//首家出牌提示
public:
	int Is2();			  //判断是否是一对牌; 
	int Is3();			  //判断是否是三张;
	int IsFeiJi();
	int IsSeries();		  //判断是否是单顺(拖拉机),此函数适合5-12张牌情况; 
	int IsDoubleSeries(); //判断是否是双顺;此函数适合6-20张牌情况;
	int IsThreeSeries();  //判断是否是三顺;此函数适合6-18张牌情况;

	int Is31();			  //判断是否是三带一单;
	int Is32();		      //判断是否是三带一对;
	int Is411();		  //判断是否是四带两单;
	int Is3311();		  //判断是否是三顺带两单;
	int Is422();		  //判断是否是四带两对;
	int Is3322();		  //判断是否是三顺带两对;
	int Is333111();		  //判断是否是三顺带三单;
	int Is333222();		  //判断是否是三顺带三对;
	int Is33331111();	  //判断是否是三顺带四单;
	int Is531();		  //判断是否是三顺带五单;
protected:
	int TipsSearch1(CCardsType& cCardsType);
	int TipsSearch2(CCardsType& cCardsType);
	int TipsSearch3(CCardsType& cCardsType);
	int TipsSearch4(CCardsType& cCardsType);
	int TipsSearch31(CCardsType& cCardsType);
	int TipsSearch5(CCardsType& cCardsType);
	int TipsSearch32(CCardsType& cCardsType);
	int TipsSearch6(CCardsType& cCardsType);
	int TipsSearch222(CCardsType& cCardsType);
	int TipsSearch33(CCardsType& cCardsType);
	int TipsSearch411(CCardsType& cCardsType);
	int TipsSearch7(CCardsType& cCardsType);
	int TipsSearch8(CCardsType& cCardsType);
	int TipsSearch2222(CCardsType& cCardsType);
	int TipsSearch3311(CCardsType& cCardsType);
	int TipsSearch422(CCardsType& cCardsType);
	int TipsSearch9(CCardsType& cCardsType);
	int TipsSearch333(CCardsType& cCardsType);
	int TipsSearch10(CCardsType& cCardsType);
	int TipsSearch22222(CCardsType& cCardsType);
	int TipsSearch3322(CCardsType& cCardsType);
	int TipsSearch11(CCardsType& cCardsType);
	int TipsSearch12(CCardsType& cCardsType);
	int TipsSearch222222(CCardsType& cCardsType);
	int TipsSearch3333(CCardsType& cCardsType);
	int TipsSearch333111(CCardsType& cCardsType);
	int TipsSearch2222222(CCardsType& cCardsType);
	int TipsSearch33333(CCardsType& cCardsType);
	int TipsSearch333222(CCardsType& cCardsType);
	int TipsSearch22222222(CCardsType& cCardsType);
	int TipsSearch33331111(CCardsType& cCardsType);
	int TipsSearch222222222(CCardsType& cCardsType);
	int TipsSearch333333(CCardsType& cCardsType);
	int TipsSoftBomb(CCardsType& cCardsType);
public:
	void New();	
	void ScandCardTable();
	void TipScanCardTable();
	void SetBaoValue(const CCard& card);		//设置宝牌
	void CleanUp();								//整理牌数据结构，从大到小排列
	void CleanUp2();							//整理牌数据结果，按牌的数量从多到少排列
	void SortChoosingCards();					//对选中的牌排序
	int CheckChoosing(int m_nCardType = 0);						//检查出牌的逻辑合法性;
	int CompareCards(CCardsType& cardType);		//比较大小
	bool CheckCards(VECCARD& vecCards);			//检查牌是否存在
	void EraseCards(VECCARD& vecCards);			//删除牌
	bool SelectCard(VECCARD& vecCards);			//把牌置为选中状态
	void RevertTigerValue();
	bool CheckTigerValue (int nTigerCount);
	int GetBomb();
	void RevertCardValue(int value,int num);
};
