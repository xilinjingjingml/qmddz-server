/*
CopyRight(c)2006 by Banding,Shanghai, All Right Reserved.

@Date:	        2009/4/20
@Created:	    10:37
@Filename: 		PlayCard.cpp
@File path:		BDLeyoyoV2\DDZV2\Common 
@Author:		leiliang

@Description:	牌型数据结构,目前各地区规则不同，通过读规则表来选规则
现有算法为较笨的穷举，简单易懂，正在考虑模式匹配算法改进
*/

#include "stdafx.h"
#include "PlayCard.h"
#include "Rule.h"

CPlayCards::CPlayCards()
{
	New();
}

CPlayCards::~CPlayCards()
{
}

void CPlayCards::CleanUp()
{
	for(size_t i = 0; i < m_cCards.size(); i++)
	{
		for(size_t j = i+1; j < m_cCards.size(); j++)
		{
			int nIValue = m_cCards[i].m_nValue;
			if (nIValue == m_nBaoValue)
			{
				nIValue = 20;
			}
			int nJValue = m_cCards[j].m_nValue;
			if (nJValue == m_nBaoValue)
			{
				nJValue = 20;
			}
			if( (nIValue * 4 + m_cCards[i].m_nColor) < (nJValue * 4 + m_cCards[j].m_nColor))
			{
				std::swap(m_cCards[i], m_cCards[j]);
			}
		}
	}
}

void CPlayCards::SortChoosingCards()
{
	for(size_t i = 0; i < m_cChoosingCards.size(); i++)
	{
		for(size_t j = i+1; j < m_cChoosingCards.size(); j++)
		{
			int nIValue = m_cChoosingCards[i].m_nValue;
			if (nIValue == m_nBaoValue)
			{
				nIValue = 20;
			}
			int nJValue = m_cChoosingCards[j].m_nValue;
			if (nJValue == m_nBaoValue)
			{
				nJValue = 20;
			}
			if( (nIValue * 4 + m_cChoosingCards[i].m_nColor) < (nJValue * 4 + m_cChoosingCards[j].m_nColor))
			{
				std::swap(m_cChoosingCards[i], m_cChoosingCards[j]);
			}
		}
	}
}

// 客户端用来洗牌，按4，3，2，1排
void CPlayCards::CleanUp2()
{	
	int nScanTable[MAXTYPENUM];
	memset(nScanTable, 0, sizeof(nScanTable));
	for(size_t i = 0; i < m_cCards.size(); i++)
	{
		if(m_cCards[i].m_nValue > 0 && m_cCards[i].m_nValue < 17)
		{
			nScanTable[(int)(m_cCards[i].m_nValue)]++;
		}
	}

	VECCARD vecCard;
	//如果大小鬼是一对,先抽出来
	if(nScanTable[16] == 2)
	{
		for(size_t i = 0; i < m_cCards.size(); i++)
		{
			if(m_cCards[i].m_nValue == 16)
			{
				vecCard.push_back(m_cCards[i]);
			}
		}
		nScanTable[16] = 0;
	}

	//依次抽取4,3,2,1张牌
	for(int k = 4; k > 0; k--)
	{
		for(int i = 16; i >= 3; i--)
		{
			if(nScanTable[i] == k)		//如果牌数量相等
			{
				for(size_t j = 0; j < m_cCards.size(); j++)
				{
					if(m_cCards[j].m_nValue == i)
					{
						vecCard.push_back(m_cCards[j]);
					}
				}
				nScanTable[i] = 0;
			}
		}
	}

	m_cCards = vecCard;
}

void CPlayCards::EraseCards(VECCARD& vecCards)
{
	for(size_t i = 0; i < vecCards.size(); i++)
	{
		for(size_t j = 0; j < m_cCards.size(); j++)
		{
			if((m_cCards[j].m_nColor == vecCards[i].m_nColor) && (m_cCards[j].m_nValue == vecCards[i].m_nValue))
			{
				m_cCards.erase(m_cCards.begin()+j);
				break;
			}
		}
	}
}

//判断是否是单顺(拖拉机),此函数适合5-12张牌情况; 
int CPlayCards::IsSeries()
{
	if(m_dwRule & RULESERIES)
		return 0;

	int nCardsCounter = 0;
	int	nMinValue = 17;
	int nTypeValue = 0;		//单牌的起始大小;
	int nCounter = 0;		//连续单牌的个数;

	// 2,大小王不能参加到顺
	if(m_nBaoValue != 15)
	{
		if(m_nDiscardingCardsTable[15] != 0)
		{
			return 0;
		}
	}

	for(int i = 16; i < 17; i++)
	{
		if(m_nDiscardingCardsTable[i] != 0)
		{
			return 0;
		}
	}	

	for(int i = 14; i > 2; i--)
	{
		nCardsCounter += m_nDiscardingCardsTable[i];
		if(m_nDiscardingCardsTable[i] > 0)
		{
			nMinValue = i;
		}
	}
	nCardsCounter += m_nDiscardingCardsTable[BAO];
	if(nCardsCounter <= (15 - nMinValue))
	{
		for(int i = 3; i < 15; i++)
		{
			nCounter = 0;
			if(m_nDiscardingCardsTable[i] == 1)		 //遇到第一张单牌,接下去扫描;
			{
				nTypeValue = i;
				for(int j = i; j < 15; j++)				//从第i张牌开始扫描;
				{				
					if(m_nDiscardingCardsTable[j] == 1)     //如果接下来的牌为单牌,则给连续单牌数加一;
					{					
						nCounter++;
					}
					else if(m_nDiscardingCardsTable[j] == 0)	//如果遇到不为单牌;
					{
						if(CheckTigerValue(1))
						{
							RevertCardValue(j,1);
							nCounter++;
						}
						else
						{
							for(int k = j; k < 15; k++)   //如果之后遇到非空,则返回0;
							{
								if(m_nDiscardingCardsTable[k] != 0)
								{							
									return 0;
								}
							}

							if(nCounter >= 5)   //如果连续对牌>=3,则为双顺;
							{				
								return nTypeValue;
							}	
							else
							{
								return 0;
							}
						}					
					}
					else      //不为空也不为单牌,返回0;
					{					
						return 0;
					}
				}
				break;
			}
			else if(m_nDiscardingCardsTable[i] != 0)
			{
				return 0;
			}
		}	

		if(nCounter >= 5)   //如果连续单牌>=5,则为单顺;
		{			
			return nTypeValue;
		}
		else				//不为单牌,返回0
		{			
			return 0;
		}
	}
	else if(nCardsCounter > (15 - nMinValue) && (15 + m_nDiscardingCardsTable[BAO] - nCardsCounter - nMinValue >= 0))
	{
		for(int i = nMinValue; i < 15; i++)
		{
			if(m_nDiscardingCardsTable[i] > 1)
			{
				return 0;
			}
		}
		return (15 - nCardsCounter);
	}
	else
	{
		return 0;
	}
}

//判断是否是双顺;此函数适合6-20张牌情况;
int CPlayCards::IsDoubleSeries()
{
	if(m_dwRule & RULEDOUBLESERIES)
		return 0;

	int nTypeValue = 0;		//对牌的起始大小;
	int nCounter = 0;		//连续对牌的个数;

	if(m_nBaoValue != 15)
	{
		if(m_nDiscardingCardsTable[15] != 0)
		{
			return 0;
		}
	}

	for(int i = 16; i < 17; i++)
	{
		if(m_nDiscardingCardsTable[i] != 0)
		{
			return 0;
		}
	}

	for(int i = 3; i < 15; i++)
	{
		nCounter = 0;
		if(m_nDiscardingCardsTable[i] == 2 || (m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(1)))
		{
			if(m_nDiscardingCardsTable[i] == 1)
				RevertCardValue(i,1);
			nCounter++;
			nTypeValue = i;
			for(int j = i + 1; j < 15; j++)    //从第i张牌开始扫描;
			{
				if(m_nDiscardingCardsTable[j] == 2)   //如果接下来的牌为对牌,则给连续对牌数加一;
				{
					nCounter++;					
				}
				else if(m_nDiscardingCardsTable[j] == 0)	//如果遇到不为对牌;
				{
					if(CheckTigerValue(2))
					{
						RevertCardValue(j,2);
						nCounter++;						
					}
					else
					{
						for(int k = j + 1; k < 15; k++)   //如果之后遇到非空,则返回0;
						{
							if(m_nDiscardingCardsTable[k] != 0)
							{					
								return 0;
							}
						}

						if(nCounter >= 3)   //如果连续对牌>=3,则为双顺;
						{				
							return nTypeValue;
						}	
						else
						{
							return 0;
						}
					}										
				}
				else if(m_nDiscardingCardsTable[j] == 1)
				{
					if(CheckTigerValue(1))
					{
						RevertCardValue(j,1);
						nCounter++;						
					}
					else
					{
						return 0;
					}
				}
				else      //不为空也不为对牌,返回0;
				{			
					return 0;
				}
			}
			break;
		}
		else if(m_nDiscardingCardsTable[i] != 0)
		{
			return 0;
		}		
	}	
	if(nCounter >= 3)   //如果连续单牌>=5,则为单顺;
	{			
		return nTypeValue;
	}
	else				//不为单牌,返回0
	{			
		return 0;
	}
}

//判断是否是三顺;此函数适合6-18张牌情况;
int CPlayCards::IsThreeSeries()
{
	if(m_dwRule & RULETHREESERIES)
		return 0;

	int nTypeValue = 0;	//三顺的起始大小;
	int nCounter = 0;	//连续三张的个数;

	if(m_nBaoValue != 15)
	{
		if(m_nDiscardingCardsTable[15] != 0)
		{
			return 0;
		}
	}

	for(int i = 15; i < 17; i++)
	{
		if(m_nDiscardingCardsTable[i] != 0)
		{
			return 0;
		}
	}

	for(int i = 3; i < 15; i++)
	{
		nCounter = 0;
		if(m_nDiscardingCardsTable[i] == 3 ||
			(m_nDiscardingCardsTable[i] == 2 && CheckTigerValue(1)) ||
			(m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(2)))
		{
			nCounter++;
			nTypeValue = i;
			for(int j = i + 1; j < 15; j++)					//从表第i项开始扫描;
			{
				if(m_nDiscardingCardsTable[j] == 3)			//如果接下来的牌为三张,则给连续对牌数加一;
				{
					nCounter++;					
				}
				else if(m_nDiscardingCardsTable[j] == 0)	//如果遇到不为三张,
				{
					if(CheckTigerValue(3))
					{
						nCounter++;
					}
					else
					{
						for(int k = j + 1; k < 15; k++)   //如果之后遇到非空,则返回0;
						{
							if(m_nDiscardingCardsTable[k] != 0)
							{							
								return 0;
							}
						}

						if(nCounter >= 1)   //如果连续对牌>=3,则为双顺;
						{				
							return nTypeValue;
						}	
						else
						{
							return 0;
						}
					}													
				}
				else if(m_nDiscardingCardsTable[j] == 2)
				{
					if(CheckTigerValue(1))
					{
						nCounter++;
					}
					else
					{
						return 0;
					}
				}
				else if(m_nDiscardingCardsTable[j] == 1)
				{
					if(CheckTigerValue(2))
					{
						nCounter++;
					}
					else
					{
						return 0;
					}
				}
				else      //不为空也不为对牌,返回0;
				{					
					return 0;
				}
			}
			break;
		}
		else if(m_nDiscardingCardsTable[i] != 0)
		{
			return 0;
		}
	}	
	if(nCounter >= 1)   //如果连续单牌>=5,则为单顺;
	{
		return nTypeValue;
	}
	else				//不为单牌,返回0
	{			
		return 0;
	}
}
int CPlayCards::IsFeiJi()
{
	if (m_nDiscardingCardsTable[16] == 2)
	{
		return 16;
	}
	return 0;
}
//判断是否是一对牌;
int CPlayCards::Is2()
{
	if(m_dwRule & RULE2)
		return 0;

	if (m_nDiscardingCardsTable[BAO] == 2)
	{
		return m_nBaoValue;
	}

	if (m_nDiscardingCardsTable[16] == 2)
	{
		return 16;
	}

	for(int i = 15; i >= 3; i--)
	{
		if(m_nDiscardingCardsTable[i] == 2)
		{
			return i;		
		}
		else if(m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(1))
		{
			RevertCardValue(i,1);
			return i;
		}		
	}
	return 0;    //出错,为空牌表;
}

//判断是否是三张;
int CPlayCards::Is3()
{
	if(m_dwRule & RULE3)
		return 0;
/*
	for(int i = 17; i >= 3; i--)
	{
		RevertTigerValue();
		if(m_nDiscardingCardsTable[i] == 3)
		{
			if(i == 17)
			{
				return m_nBaoValue;
			}
			else
			{
				return i;
			}			
		}
		else if(m_nDiscardingCardsTable[i] == 2 && CheckTigerValue(1) && i < 16)
		{
			return i;
		}	
		else if(m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(2) && i < 16)
		{
			return i;
		}
	}*/
	//change by JL for efficiency
	for(int i = 15; i >= 3; i--)
	{
		if(m_nDiscardingCardsTable[i] == 3)
		{
			return i;
		}
		else if(m_nDiscardingCardsTable[i] == 2 && m_nDiscardingCardsTable[BAO] >= 1)
		{
			RevertCardValue(i,1);
			return i;
		}	
		else if(m_nDiscardingCardsTable[i] == 1 && m_nDiscardingCardsTable[BAO] >= 2)
		{
			RevertCardValue(i,2);
			return i;
		}
	}
	if(m_nDiscardingCardsTable[BAO] == 3)
	{
		return m_nBaoValue;
	}
	return 0;
}

//判断是否是四张(软炸弹); 
int CPlayCards::IsSoftBomb()
{
	if(m_dwRule & RULE4)
		return 0;

	for(int i = 3; i < 16; i++)
	{
		/*
		if(m_nDiscardingCardsTable[i] == 3 && CheckTigerValue(1))
		{
			return i;
		}
		else if(m_nDiscardingCardsTable[i] == 2 && CheckTigerValue(2))
		{
			return i;
		}
		else if(m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(3))
		{
			return i;
		}*/		
		//change by JL for efficiency
		if(m_nDiscardingCardsTable[i] == 3 && m_nDiscardingCardsTable[BAO] >= 1)
		{
			RevertCardValue(i,1);
			return i;
		}
		else if(m_nDiscardingCardsTable[i] == 2 && m_nDiscardingCardsTable[BAO] >= 2)
		{
			RevertCardValue(i,2);
			return i;
		}
		else if(m_nDiscardingCardsTable[i] == 1 &&m_nDiscardingCardsTable[BAO] >= 3)
		{
			RevertCardValue(i,3);
			return i;
		}
	}
	return 0;
}

//判断是否是四张(炸弹); 
int CPlayCards::IsHardBomb()
{
	if(m_dwRule & RULE4)
		return 0;

	if (m_nDiscardingCardsTable[BAO] == 4)
	{
		return BAO;
	}

	for(int i = 3; i < 16; i++)
	{
		if(m_nDiscardingCardsTable[i] == 4)
		{
			return i;			
		}
		else if(m_nDiscardingCardsTable[i] != 0)
		{
			return 0;
		}
	}
	return 0;
}

//判断是否是三带一单;
int CPlayCards::Is31()
{
	if(m_dwRule & RULE31)
		return 0;
	//三赖子情况是软炸弹
	for(int i = 15; i >= 3; i--)
	{
		if(m_nDiscardingCardsTable[i] == 3)
		{
			return i;
		}
		else if(m_nDiscardingCardsTable[i] == 2 && CheckTigerValue(1))
		{
			RevertCardValue(i,1);
			return i;
		}
		else if(m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(2))
		{
			RevertCardValue(i,2);
			return i;
		}
	}
	return 0;
}

//判断是否是三带一对;
int CPlayCards::Is32()
{
	if(m_dwRule & RULE32)
		return 0;

	int nTypeValue = 0;
	//int Pair = 0;
	//四赖子情况是四带一
	if(m_nDiscardingCardsTable[BAO] == 3 )
	{
		nTypeValue = m_nBaoValue;
		for(int j = 3; j < 16; j++)
		{
			if (m_nDiscardingCardsTable[j] == 2)				
			{
				return nTypeValue>j ? nTypeValue : j;
			}	
		}
	}		
	for(int i = 15; i >= 3; i--)
	{
		if(m_nDiscardingCardsTable[i] == 3 ||
			(m_nDiscardingCardsTable[i] == 2 && CheckTigerValue(1)) ||
			(m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(2)))
		{
			if(m_nDiscardingCardsTable[i] == 2)
				RevertCardValue(i,1);
			if(m_nDiscardingCardsTable[i] == 1)
				RevertCardValue(i,2);

			nTypeValue = i;
			//for(int j = 3; j < 17; j++)//j < 17 ,333+大小王呢
			for(int j = 3; j < 16; j++)
			{
				if (i == j)//不带自己本身
				{
					continue;
				}
				if(m_nDiscardingCardsTable[j] == 2 )
				{
					return nTypeValue;
				}
				else if(m_nDiscardingCardsTable[j] == 1 && CheckTigerValue(1))
				{
					//if(i != j )//不带自己本身
					{
						RevertCardValue(j,1);
						return nTypeValue;
					}
				}
			}
			if(CheckTigerValue(2))
			{
				//Pair = j ;
				//if(i != Pair)//333+laizi时呢
				{
					//RevertCardValue(j,2);
					return nTypeValue;
				}
			}
		}
	}
	return 0;
}

//判断是否是四带两单;
int CPlayCards::Is411()
{
	if(m_dwRule & RULE411)
		return 0;

	int nTypeValue = 0;
	//int nCounter = 0;

	if(m_nDiscardingCardsTable[BAO] == 4)
	{
		nTypeValue = m_nBaoValue;
		int nNum = 0;
		for(int j = 3; j < 17; j++)
		{
			if(m_nDiscardingCardsTable[j] == 1)
			{
				nNum++;
				if(nTypeValue<j)
					nTypeValue = j;
			}else if(m_nDiscardingCardsTable[j] == 1)
			{
				nNum +=2;
				if(nTypeValue<j)
					nTypeValue = j;
			}
		}
		if (nTypeValue !=  m_nBaoValue)
		{
			if(m_nDiscardingCardsTable[nTypeValue] == 2)
				RevertCardValue(nTypeValue,2);
			if(m_nDiscardingCardsTable[nTypeValue] == 1)
				RevertCardValue(nTypeValue,3);
		}
		if(nNum == 2)
		{
			return nTypeValue;
		}
	}

	for(int i = 15; i >= 3; i--)
	{
		if(m_nDiscardingCardsTable[i] == 4 ||
			(m_nDiscardingCardsTable[i] == 3 && CheckTigerValue(1)) ||
			(m_nDiscardingCardsTable[i] == 2 && CheckTigerValue(2)) ||
			(m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(3)))
		{
			if(m_nDiscardingCardsTable[i] == 3)
				RevertCardValue(i,1);
			if(m_nDiscardingCardsTable[i] == 2)
				RevertCardValue(i,2);
			if(m_nDiscardingCardsTable[i] == 1)
				RevertCardValue(i,3);

			nTypeValue = i;
			int nNum = 0;
			if(CheckTigerValue(1))
				nNum += 1;
			if(CheckTigerValue(2))
				nNum += 2;
			for(int j = 3; j < 17; j++)
			{
				if (i == j)
				{
					continue;
				}
				if(m_nDiscardingCardsTable[j] == 1 || m_nDiscardingCardsTable[j] == 2)
				{
					//if(i != j)
					{
						nNum += m_nDiscardingCardsTable[j];
					}
				}
			}
			if(nNum == 2)
			{
				return nTypeValue;
			}
		}
	}
	return 0;
}

//判断是否是三顺带两单;
int CPlayCards::Is3311()
{
	if(m_dwRule & RULE3311)
		return 0;

	int nTypeValue = 0;
	for(int i = 3; i < 14; i++)
	{
		if(m_nDiscardingCardsTable[i] == 3 ||
			(m_nDiscardingCardsTable[i] == 2 && CheckTigerValue(1)) ||
			(m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(2)) || 
			CheckTigerValue(3))
		{
			nTypeValue = i;
			if(m_nDiscardingCardsTable[i+1] == 3 ||
				(m_nDiscardingCardsTable[i+1] == 2 && CheckTigerValue(1)) ||
				(m_nDiscardingCardsTable[i+1] == 1 && CheckTigerValue(2)) ||
				CheckTigerValue(3))
			{

				
#ifdef _BAMIN
				return nTypeValue;
#else
				int nNum = 0;
				//增加33 2 可以分解成3311牌型
				if(m_nDiscardingCardsTable[i] == 0)
					RevertCardValue(i,3);
				if(m_nDiscardingCardsTable[i] == 2)
					RevertCardValue(i,1);
				if(m_nDiscardingCardsTable[i] == 1)
					RevertCardValue(i,2);
				if(m_nDiscardingCardsTable[i+1] == 0)
					RevertCardValue(i+1,3);
				if(m_nDiscardingCardsTable[i+1] == 2)
					RevertCardValue(i+1,1);
				if(m_nDiscardingCardsTable[i+1] == 1)
					RevertCardValue(i+1,2);

				if(CheckTigerValue(1))
					nNum+=1;
				if(CheckTigerValue(2))
					nNum+=2;
				for(int j = 3; j < 17; j++)
				{
					if(j == i)//不带自己本身
					{
						j++; //i+1
						continue;
					}
					if(m_nDiscardingCardsTable[j] >= 1)
					{
						//if(j != i && (j != i+1))
						{
							nNum += m_nDiscardingCardsTable[j];
						}
					}
				}
				if(nNum == 2)
				{
					return nTypeValue;
				}
				
				/*
				for(int j = 3; j < 17; j++)
				{
					if(CheckTigerValue(1))
						nNum+=1;
					if(CheckTigerValue(2))
						nNum+=2;
					if(m_nDiscardingCardsTable[j] == 1 || m_nDiscardingCardsTable[j] == 2)
					{
						if(j != i && (j != i+1))
						{
							nNum += m_nDiscardingCardsTable[j];
						}
					}
				}
				if(nNum == 2)
				{
					return nTypeValue;
				}*/
#endif

			}
		}
		RevertTigerValue();
	}	

	return 0;
}

//判断是否是四带两对;
int CPlayCards::Is422()
{
	if(m_dwRule & RULE422)
		return 0;

	int nTypeValue = 0;

	if(m_nDiscardingCardsTable[BAO] == 4)
	{
		nTypeValue = m_nBaoValue;
		int nNum = 0;
		for(int j = 3; j < 16; j++)
		{
			if(m_nDiscardingCardsTable[j] == 2)
			{
				nNum++;
				if(nTypeValue<j)
					nTypeValue = j;
			}else if (m_nDiscardingCardsTable[j] == 4)
			{
				nNum +=2;
				if(nTypeValue<j)
					nTypeValue = j;
			}
		}
		if (nTypeValue !=  m_nBaoValue)
		{
			if(m_nDiscardingCardsTable[nTypeValue] == 2)
				RevertCardValue(nTypeValue,2);
		}
		if(nNum == 2)
		{
			return nTypeValue;
		}
	}		

	for(int i = 3; i < 16;i++)
	{
		if(m_nDiscardingCardsTable[i] == 4 ||
			(m_nDiscardingCardsTable[i] == 3 && CheckTigerValue(1)) ||
			(m_nDiscardingCardsTable[i] == 2 && CheckTigerValue(2)) ||
			(m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(3)))
		{

			if(m_nDiscardingCardsTable[i] == 3)
				RevertCardValue(i,1);
			if(m_nDiscardingCardsTable[i] == 2)
				RevertCardValue(i,2);
			if(m_nDiscardingCardsTable[i] == 1)
				RevertCardValue(i,3);

			nTypeValue = i;
			int nNum = 0;
			for(int j = 3; j < 16; j++)
			{
				if(i == j ) {
					continue;
				}
				if(m_nDiscardingCardsTable[j] == 2 )
				{
					//if(i != j)
					{
						nNum++;
					}
				}
				else if (m_nDiscardingCardsTable[j] == 1 && CheckTigerValue(1))
				{
					RevertCardValue(j,1);
					nNum++;
				}
			}
			if(CheckTigerValue(2))
			{
				nNum++;
			}
			if(CheckTigerValue(2))
			{
				nNum++;
			}
			if(nNum == 2)
			{
				return nTypeValue;
			}
		}		
	}	
	return 0;
}

//判断是否是三顺带两对;	
int CPlayCards::Is3322()
{
	if(m_dwRule & RULE3322)
		return 0;

	int nTypeValue = 0;

	for (int i = 3; i < 14; i++)
	{
		if(m_nDiscardingCardsTable[i] == 3 ||
			(m_nDiscardingCardsTable[i] == 2 && CheckTigerValue(1)) ||
			(m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(2)) ||
			 CheckTigerValue(3))
		{
			nTypeValue = i;
			if(m_nDiscardingCardsTable[i+1] == 3 ||
				(m_nDiscardingCardsTable[i+1] == 2 && CheckTigerValue(1)) ||
				(m_nDiscardingCardsTable[i+1] == 1 && CheckTigerValue(2)) ||
				 CheckTigerValue(3))
			{
				if(m_nDiscardingCardsTable[i] == 0)
					RevertCardValue(i,3);
				if(m_nDiscardingCardsTable[i] == 2)
					RevertCardValue(i,1);
				if(m_nDiscardingCardsTable[i] == 1)
					RevertCardValue(i,2);
				if(m_nDiscardingCardsTable[i+1] == 0)
					RevertCardValue(i+1,3);
				if(m_nDiscardingCardsTable[i+1] == 2)
					RevertCardValue(i+1,1);
				if(m_nDiscardingCardsTable[i+1] == 1)
					RevertCardValue(i+1,2);

				int nNum = 0;
				for(int j = 3; j < 16; j++)
				{
					if (i == j )
					{
						j++;
						continue;
					}
					if(m_nDiscardingCardsTable[j] == 2 )
					{
						//if(j != i && (j != i+1))
						{
							nNum++;
						}
					}
					else if (m_nDiscardingCardsTable[j] == 1 && CheckTigerValue(1))
					{
						RevertCardValue(j,1);
						nNum++;
					}
				}
				if(CheckTigerValue(2))
				{
					nNum++;
				}
				if(CheckTigerValue(2))
				{
					nNum++;
				}
				if(nNum == 2)
				{
					return nTypeValue;
				}
			}
		}
		RevertTigerValue();
	}	

	return 0;
}

//判断是否是三顺带三单;
int CPlayCards::Is333111()
{
	if(m_dwRule & RULE333111)
		return 0;

	int nTypeValue = 0;

	for (int i = 3; i < 13; i++)
	{
		if(m_nDiscardingCardsTable[i] == 3 ||
		  (m_nDiscardingCardsTable[i] == 2 && CheckTigerValue(1)) ||
		  (m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(2)) ||
		   CheckTigerValue(3))
		{
			nTypeValue = i;
			if(m_nDiscardingCardsTable[i+1] == 3 ||
			  (m_nDiscardingCardsTable[i+1] == 2 && CheckTigerValue(1)) ||
			  (m_nDiscardingCardsTable[i+1] == 1 && CheckTigerValue(2)) || 
			   CheckTigerValue(3))
			{
				if(m_nDiscardingCardsTable[i+2] == 3 ||
				  (m_nDiscardingCardsTable[i+2] == 2 && CheckTigerValue(1)) ||
				  (m_nDiscardingCardsTable[i+2] == 1 && CheckTigerValue(2)) ||
				   CheckTigerValue(3))
				{
#ifdef _BAMIN
					return nTypeValue;
#else 
					if(m_nDiscardingCardsTable[i] == 0)
						RevertCardValue(i,3);
					if(m_nDiscardingCardsTable[i] == 2)
						RevertCardValue(i,1);
					if(m_nDiscardingCardsTable[i] == 1)
						RevertCardValue(i,2);
					if(m_nDiscardingCardsTable[i+1] == 0)
						RevertCardValue(i+1,3);
					if(m_nDiscardingCardsTable[i+1] == 2)
						RevertCardValue(i+1,1);
					if(m_nDiscardingCardsTable[i+1] == 1)
						RevertCardValue(i+1,2);
					if(m_nDiscardingCardsTable[i+2] == 0)
						RevertCardValue(i+2,3);
					if(m_nDiscardingCardsTable[i+2] == 2)
						RevertCardValue(i+2,1);
					if(m_nDiscardingCardsTable[i+2] == 1)
						RevertCardValue(i+2,2);
					int nNum = 0;

					for(int j = 3; j < 18; j++)
					{
						if (i == j)
						{
							j +=2;
							continue;
						}
						if(m_nDiscardingCardsTable[j] >= 1)
						{
							//if(j != i && (j != i+1) && (j != i+2))
							{
								nNum += m_nDiscardingCardsTable[j];
							}
						}
					}
					if(nNum == 3)
					{
						return nTypeValue;
					}
					else
					{

					}
#endif

				}	
			}
		}
		RevertTigerValue();
	}	
	return 0;
}

//判断是否是三顺带三对;
int CPlayCards::Is333222()	
{
	if(m_dwRule & RULE333222)
		return 0;

	int nTypeValue = 0;
	
	for (int i = 3; i < 13; i++)
	{
		if(m_nDiscardingCardsTable[i] == 3 ||
		  (m_nDiscardingCardsTable[i] == 2 && CheckTigerValue(1)) ||
		  (m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(2)) ||
		   CheckTigerValue(3))
		{
			nTypeValue = i;
			if(m_nDiscardingCardsTable[i+1] == 3 ||
			  (m_nDiscardingCardsTable[i+1] == 2 && CheckTigerValue(1)) ||
			  (m_nDiscardingCardsTable[i+1] == 1 && CheckTigerValue(2)) ||
			   CheckTigerValue(3))
			{
				if(m_nDiscardingCardsTable[i+2] == 3 ||
				  (m_nDiscardingCardsTable[i+2] == 2 && CheckTigerValue(1)) ||
				  (m_nDiscardingCardsTable[i+2] == 1 && CheckTigerValue(2)) ||
				   CheckTigerValue(3))
				{

					if(m_nDiscardingCardsTable[i] == 0)
						RevertCardValue(i,3);
					if(m_nDiscardingCardsTable[i] == 2)
						RevertCardValue(i,1);
					if(m_nDiscardingCardsTable[i] == 1)
						RevertCardValue(i,2);
					if(m_nDiscardingCardsTable[i+1] == 0)
						RevertCardValue(i+1,3);
					if(m_nDiscardingCardsTable[i+1] == 2)
						RevertCardValue(i+1,1);
					if(m_nDiscardingCardsTable[i+1] == 1)
						RevertCardValue(i+1,2);
					if(m_nDiscardingCardsTable[i+2] == 0)
						RevertCardValue(i+2,3);
					if(m_nDiscardingCardsTable[i+2] == 2)
						RevertCardValue(i+2,1);
					if(m_nDiscardingCardsTable[i+2] == 1)
						RevertCardValue(i+2,2);

					int nNum = 0;
					for(int j = 3; j < 16; j++)
					{
						if (i == j)
						{
							j += 2;
							continue;
						}
						if(m_nDiscardingCardsTable[j] == 2 )
						{
							//if(j != i && (j != i+1) && (j != i+2))
							{
								nNum++;
							}
						}
						else if(m_nDiscardingCardsTable[j] == 1 && CheckTigerValue(1))
						{
							nNum++;
							RevertCardValue(j,1);
						}
					}
					if (CheckTigerValue(2))
					{
						nNum++;
					}
					if(CheckTigerValue(2))
					{
						nNum++;
					}
					if(nNum == 3)
					{
						return nTypeValue;
					}
				}	
			}
		}
		RevertTigerValue();
	}	

	return 0;
}

//判断是否是三顺带四单;
int CPlayCards::Is33331111()
{
	if(m_dwRule & RULE33331111)
		return 0;

	int nTypeValue = 0;

	for(int i = 3; i < 12; i++)
	{
		if(m_nDiscardingCardsTable[i] == 3 ||
		  (m_nDiscardingCardsTable[i] == 2 && CheckTigerValue(1)) ||
		  (m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(2)) ||
		  CheckTigerValue(3))
		{
			nTypeValue = i;
			if(m_nDiscardingCardsTable[i+1] == 3 ||
			  (m_nDiscardingCardsTable[i+1] == 2 && CheckTigerValue(1)) ||
			  (m_nDiscardingCardsTable[i+1] == 1 && CheckTigerValue(2)) ||
			  CheckTigerValue(3))
			{
				if(m_nDiscardingCardsTable[i+2] == 3 ||
				  (m_nDiscardingCardsTable[i+2] == 2 && CheckTigerValue(1)) ||
				  (m_nDiscardingCardsTable[i+2] == 1 && CheckTigerValue(2)) ||
				  CheckTigerValue(3))
				{
					if(m_nDiscardingCardsTable[i+3] == 3 ||
					  (m_nDiscardingCardsTable[i+3] == 2 && CheckTigerValue(1)) ||
					  (m_nDiscardingCardsTable[i+3] == 1 && CheckTigerValue(2)) ||
					  CheckTigerValue(3))
					{
						if(m_nDiscardingCardsTable[i] == 0)
							RevertCardValue(i,3);
						if(m_nDiscardingCardsTable[i] == 2)
							RevertCardValue(i,1);
						if(m_nDiscardingCardsTable[i] == 1)
							RevertCardValue(i,2);
						if(m_nDiscardingCardsTable[i+1] == 0)
							RevertCardValue(i+1,3);
						if(m_nDiscardingCardsTable[i+1] == 2)
							RevertCardValue(i+1,1);
						if(m_nDiscardingCardsTable[i+1] == 1)
							RevertCardValue(i+1,2);
						if(m_nDiscardingCardsTable[i+2] == 0)
							RevertCardValue(i+2,3);
						if(m_nDiscardingCardsTable[i+2] == 2)
							RevertCardValue(i+2,1);
						if(m_nDiscardingCardsTable[i+2] == 1)
							RevertCardValue(i+2,2);
						if(m_nDiscardingCardsTable[i+3] == 0)
							RevertCardValue(i+3,3);
						if(m_nDiscardingCardsTable[i+3] == 2)
							RevertCardValue(i+3,1);
						if(m_nDiscardingCardsTable[i+3] == 1)
							RevertCardValue(i+3,2);

						int nNum = 0;

						for(int j = 3; j < 18; j++)
						{
							if (i == j )
							{
								j += 3;
								continue;
							}
							if(m_nDiscardingCardsTable[j] >= 1)
							{
								//if(j != i && (j != i+1) && (j != i+2) && (j != i+3))
								{
									nNum += m_nDiscardingCardsTable[j];
								}
							}
						}
						if(nNum == 4)
						{
							return nTypeValue;
						}
					}					
				}				
			}
		}
		RevertTigerValue();
	}	
	return 0;
}

//判断是否是三顺带五单;
int CPlayCards::Is531()
{
	if (m_dwRule & RULE531)
	{
		return 0;
	}

	int nTypeValue = 0;
	for (int i = 3; i < 11; i++)
	{
		if (m_nDiscardingCardsTable[i] == 3 ||
			(m_nDiscardingCardsTable[i] == 2 && CheckTigerValue(1)) ||
			(m_nDiscardingCardsTable[i] == 1 && CheckTigerValue(2)) ||
			CheckTigerValue(3))
		{
			nTypeValue = i;
			if (m_nDiscardingCardsTable[i + 1] == 3 ||
				(m_nDiscardingCardsTable[i + 1] == 2 && CheckTigerValue(1)) ||
				(m_nDiscardingCardsTable[i + 1] == 1 && CheckTigerValue(2)) ||
				CheckTigerValue(3))
			{
				if (m_nDiscardingCardsTable[i + 2] == 3 ||
					(m_nDiscardingCardsTable[i + 2] == 2 && CheckTigerValue(1)) ||
					(m_nDiscardingCardsTable[i + 2] == 1 && CheckTigerValue(2)) ||
					CheckTigerValue(3))
				{
					if (m_nDiscardingCardsTable[i + 3] == 3 ||
						(m_nDiscardingCardsTable[i + 3] == 2 && CheckTigerValue(1)) ||
						(m_nDiscardingCardsTable[i + 3] == 1 && CheckTigerValue(2)) ||
						CheckTigerValue(3))
					{
						if (m_nDiscardingCardsTable[i + 4] == 3 ||
							(m_nDiscardingCardsTable[i + 4] == 2 && CheckTigerValue(1)) ||
							(m_nDiscardingCardsTable[i + 4] == 1 && CheckTigerValue(2)) ||
							CheckTigerValue(3))
						{
							if (m_nDiscardingCardsTable[i] == 0)
								RevertCardValue(i, 3);
							if (m_nDiscardingCardsTable[i] == 2)
								RevertCardValue(i, 1);
							if (m_nDiscardingCardsTable[i] == 1)
								RevertCardValue(i, 2);
							if (m_nDiscardingCardsTable[i + 1] == 0)
								RevertCardValue(i + 1, 3);
							if (m_nDiscardingCardsTable[i + 1] == 2)
								RevertCardValue(i + 1, 1);
							if (m_nDiscardingCardsTable[i + 1] == 1)
								RevertCardValue(i + 1, 2);
							if (m_nDiscardingCardsTable[i + 2] == 0)
								RevertCardValue(i + 2, 3);
							if (m_nDiscardingCardsTable[i + 2] == 2)
								RevertCardValue(i + 2, 1);
							if (m_nDiscardingCardsTable[i + 2] == 1)
								RevertCardValue(i + 2, 2);
							if (m_nDiscardingCardsTable[i + 3] == 0)
								RevertCardValue(i + 3, 3);
							if (m_nDiscardingCardsTable[i + 3] == 2)
								RevertCardValue(i + 3, 1);
							if (m_nDiscardingCardsTable[i + 3] == 1)
								RevertCardValue(i + 3, 2);
							if (m_nDiscardingCardsTable[i + 4] == 0)
								RevertCardValue(i + 4, 3);
							if (m_nDiscardingCardsTable[i + 4] == 2)
								RevertCardValue(i + 4, 1);
							if (m_nDiscardingCardsTable[i + 4] == 1)
								RevertCardValue(i + 4, 2);

							int nNum = 0;
							for (int j = 3; j < 16; j++)
							{
								if (i == j)
								{
									j += 4;
									continue;
								}
								if (m_nDiscardingCardsTable[j] >= 1)
								{
									//if(j != i && (j != i+1) && (j != i+2) && (j != i+3))
									{
										nNum += m_nDiscardingCardsTable[j];
									}
								}
							}
							if (nNum == 5)
							{
								return nTypeValue;
							}
							
						}
					}
				}
			}
		}
		RevertTigerValue();
	}
	return 0;
}

void CPlayCards::New()
{
	m_nBaoValue = 0;
	m_dwRule = 0/*RULE32|RULE422*/;
	memset(&m_cDiscardingType, 0, sizeof(m_cDiscardingType));
	memset(m_nDiscardingCardsTable, 0, sizeof(m_nDiscardingCardsTable));
	m_cCards.clear();
	m_cChoosingCards.clear();
	m_cDiscarding.clear();
	m_vecTipsCards.clear();
}

/*
* 函数介绍：检查出牌的逻辑合法性;
* 返回值 ： 匹配成功返回1,不成功返回0;
*/
int CPlayCards::CheckChoosing(int m_nCardType /*= 0*/)
{
	//int i = 0;
	ScandCardTable();
	int Table_Index;

	switch(m_cChoosingCards.size())
	{
	case 0:
		m_cDiscardingType.SetValue(0,0,0);
		return 0;
	case 1:
		if( m_cChoosingCards[0].m_nValue == 16 )
			m_cDiscardingType.SetValue(0,1,m_cChoosingCards[0].m_nValue+m_cChoosingCards[0].m_nColor);
		else
			m_cDiscardingType.SetValue(0,1,m_cChoosingCards[0].m_nValue);
		return 1;
	case 2://是否是一对牌;
		if( (Table_Index = Is2()) != 0 )
		{
			//是大王,则为炸弹;
			if(Table_Index == 16)
				m_cDiscardingType.SetValue(2,4,Table_Index);	//类型为2表示炸弹;
			else
				m_cDiscardingType.SetValue(0,2,Table_Index);
			return 1;
		}
		return 0;
	case 3:
		if( (Table_Index = Is3()) != 0 )
		{
			m_cDiscardingType.SetValue(0,3,Table_Index);
			return 1;
		}
		return 0;
	case 4:
		if( (Table_Index = IsHardBomb()) != 0 )
		{
			m_cDiscardingType.SetValue(2,4,Table_Index);
			return 1;
		}
		RevertTigerValue();
		if( (Table_Index = IsSoftBomb()) != 0 )
		{
			m_cDiscardingType.SetValue(1,4,Table_Index);
			return 1;
		}
		RevertTigerValue();
		if( (Table_Index = Is31()) != 0 )
		{
			m_cDiscardingType.SetValue(0,31,Table_Index);
			return 1;
		}
		return 0;
	case 5:/*
		if( (Table_Index = IsSeries()) != 0 )
		{
			m_cDiscardingType.SetValue(0,5,Table_Index);
			return 1;
		}	
		// not 41 type
		RevertTigerValue();
		if( (Table_Index = Is32()) != 0 )
		{
			m_cDiscardingType.SetValue(0,32,Table_Index);
			return 1;
		}	*/
		if(m_nCardType > 0)				//指定牌型做判断 针对癞子
		{
			if (m_nCardType == 32 && (Table_Index = Is32()) != 0)
			{
				m_cDiscardingType.SetValue(0,32,Table_Index);
				return 1;
			}		
			RevertTigerValue();
			if (m_nCardType == 5 && (Table_Index = IsSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0,5,Table_Index);
				return 1;
			}	
		}else{
			if ((Table_Index = IsSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0, 5, Table_Index);
				return 1;
			}
			RevertTigerValue();
			if ((Table_Index = Is32()) != 0)
			{
				m_cDiscardingType.SetValue(0, 32, Table_Index);
				return 1;
			}
		}
		return 0;
	case 6:
		if(m_nCardType > 0)				//指定牌型做判断 针对癞子
		{
			if (m_nCardType == 222 && (Table_Index = IsDoubleSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0,222,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if (m_nCardType == 411 && (Table_Index = Is411()) != 0)
			{
				m_cDiscardingType.SetValue(0,411,Table_Index);
				return 1;
			}	 
			RevertTigerValue();
			if (m_nCardType == 33 && (Table_Index = IsThreeSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0,33,Table_Index);
				return 1;
			}
			RevertTigerValue();		
			if (m_nCardType == 6 && (Table_Index = IsSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0,6,Table_Index);
				return 1;
			}		
		}
		else							//默认判断
		{
			if ((Table_Index = IsSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0, 6, Table_Index);
				return 1;
			}
			RevertTigerValue();
			if ((Table_Index = IsDoubleSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0,222,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if( (Table_Index = Is411()) != 0 )
			{
				m_cDiscardingType.SetValue(0,411,Table_Index);
				return 1;
			}	 
			RevertTigerValue();
			if( (Table_Index = IsThreeSeries()) != 0 )
			{
				m_cDiscardingType.SetValue(0,33,Table_Index);
				return 1;
			}
		}
		return 0;
	case 7:	//单顺
		if( (Table_Index = IsSeries()) != 0 )
		{
			m_cDiscardingType.SetValue(0,7,Table_Index);
			return 1;
		}
		return 0;
	case 8:
		if(m_nCardType > 0)				//指定牌型做判断 针对癞子
		{
			if (m_nCardType == 422 && (Table_Index = Is422()) != 0)
			{
				m_cDiscardingType.SetValue(0,422,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if (m_nCardType == 3311 && (Table_Index = Is3311()) != 0)
			{
				m_cDiscardingType.SetValue(0,3311,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if (m_nCardType == 2222 && (Table_Index = IsDoubleSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0,2222,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if (m_nCardType == 8 && (Table_Index = IsSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0,8,Table_Index);
				return 1;
			}		
		}
		else
		{
			if ((Table_Index = IsSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0, 8, Table_Index);
				return 1;
			}
			RevertTigerValue();
			if ((Table_Index = Is422()) != 0)
			{
				m_cDiscardingType.SetValue(0,422,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if( (Table_Index = Is3311()) != 0 )
			{
				m_cDiscardingType.SetValue(0,3311,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if( (Table_Index = IsDoubleSeries()) != 0 )
			{
				m_cDiscardingType.SetValue(0,2222,Table_Index);
				return 1;
			}
		}
		
		return 0;
	case 9:
		if ((Table_Index = IsSeries()) != 0)
		{
			m_cDiscardingType.SetValue(0, 9, Table_Index);
			return 1;
		}
		RevertTigerValue();
		if ((Table_Index = IsThreeSeries()) != 0)
		{
			m_cDiscardingType.SetValue(0,333,Table_Index);
			return 1;
		}
		return 0;
	case 10:
		if(m_nCardType > 0)				//指定牌型做判断 针对癞子
		{
			if (m_nCardType == 3322 && (Table_Index = Is3322()) != 0)
			{
				m_cDiscardingType.SetValue(0,3322,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if (m_nCardType == 22222 && (Table_Index = IsDoubleSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0,22222,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if (m_nCardType == 10 && (Table_Index = IsSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0,10,Table_Index);
				return 1;
			}		
		}
		else
		{		
			if( (Table_Index = Is3322()) != 0 )
			{
				m_cDiscardingType.SetValue(0,3322,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if( (Table_Index = IsDoubleSeries()) != 0 )
			{
				m_cDiscardingType.SetValue(0,22222,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if( (Table_Index = IsSeries()) != 0 )
			{
				m_cDiscardingType.SetValue(0,10,Table_Index);
				return 1;
			}		
		}
		return 0;
	case 11:		
		if( (Table_Index = IsSeries()) != 0 )
		{
			m_cDiscardingType.SetValue(0,11,Table_Index);
			return 1;
		}
		return 0;
	case 12:
		if(m_nCardType > 0)				//指定牌型做判断 针对癞子
		{
			if (m_nCardType == 3333 && (Table_Index = IsThreeSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0,3333,Table_Index);
				return 1;
			}
			RevertTigerValue();	
			if (m_nCardType == 333111 && (Table_Index = Is333111()) != 0)
			{
				m_cDiscardingType.SetValue(0,333111,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if (m_nCardType == 222222 && (Table_Index = IsDoubleSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0,222222,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if (m_nCardType == 12 && (Table_Index = IsSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0,12,Table_Index);
				return 1;
			}
		}
		else
		{
			if( (Table_Index = IsThreeSeries()) != 0 )
			{
				m_cDiscardingType.SetValue(0,3333,Table_Index);
				return 1;
			}
			RevertTigerValue();	
			if( (Table_Index = Is333111()) != 0 )
			{
				m_cDiscardingType.SetValue(0,333111,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if( (Table_Index = IsDoubleSeries()) != 0 )
			{
				m_cDiscardingType.SetValue(0,222222,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if( (Table_Index = IsSeries()) != 0 )
			{
				m_cDiscardingType.SetValue(0,12,Table_Index);
				return 1;
			}
		}
		return 0;
	case 13:
		return 0;
	case 14:		
		if( (Table_Index = IsDoubleSeries()) != 0 )
		{
			m_cDiscardingType.SetValue(0,2222222,Table_Index);
			return 1;
		}
		return 0;
	case 15:	
		if(m_nCardType > 0)				//指定牌型做判断 针对癞子
		{
			if (m_nCardType == 333222 && (Table_Index = Is333222()) != 0)
			{
				m_cDiscardingType.SetValue(0,333222,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if (m_nCardType == 33333 && (Table_Index = IsThreeSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0,33333,Table_Index);
				return 1;
			}	
		}else{
			if( (Table_Index = Is333222()) != 0 )
			{
				m_cDiscardingType.SetValue(0,333222,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if( (Table_Index = IsThreeSeries()) != 0 )
			{
				m_cDiscardingType.SetValue(0,33333,Table_Index);
				return 1;
			}	
		}
			
		return 0;
	case 16:	
		if(m_nCardType > 0)				//指定牌型做判断 针对癞子
		{
			if (m_nCardType == 33331111 && (Table_Index = Is33331111()) != 0)
			{
				m_cDiscardingType.SetValue(0,33331111,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if (m_nCardType == 22222222 && (Table_Index = IsDoubleSeries()) != 0)
			{
				m_cDiscardingType.SetValue(0,22222222,Table_Index);
				return 1;
			}
		}else{
			if( (Table_Index = Is33331111()) != 0 )
			{
				m_cDiscardingType.SetValue(0,33331111,Table_Index);
				return 1;
			}
			RevertTigerValue();
			if( (Table_Index = IsDoubleSeries()) != 0 )
			{
				m_cDiscardingType.SetValue(0,22222222,Table_Index);
				return 1;
			}
		}
		return 0;
	case 17:
		return 0;
	case 18:
		if( (Table_Index = IsThreeSeries()) != 0 )
		{
			m_cDiscardingType.SetValue(0,333333,Table_Index);
			return 1;
		}
		RevertTigerValue();
		if( (Table_Index = IsDoubleSeries()) != 0 )
		{
			m_cDiscardingType.SetValue(0,222222222,Table_Index);
			return 1;
		}		
		return 0;
	case 20:
		if ((Table_Index = Is531()) != 0)
		{
			m_cDiscardingType.SetValue(0, 531, Table_Index);
			return 1;
		}
		RevertTigerValue();
		if ((Table_Index = IsDoubleSeries()) != 0)
		{
			m_cDiscardingType.SetValue(0,2000000000,Table_Index);
			return 1;
		}
		return 0;
	}
	return 0;
}

int CPlayCards::CompareCards(CCardsType& cardType)
{
	if( m_cDiscardingType.m_nTypeNum == 0 )
		return 1;

	//大小判断;
	if( cardType.m_nTypeNum != 0 )
	{
		if(m_cDiscardingType.m_nTypeBomb == 2 && m_cDiscardingType.m_nTypeValue == 16)
			return 1;
		if(cardType.m_nTypeBomb ==2 && cardType.m_nTypeValue == 16)
			return 0;
		if( m_cDiscardingType.m_nTypeBomb > cardType.m_nTypeBomb )
		{
			return 1;
		}
		else if( m_cDiscardingType.m_nTypeBomb < cardType.m_nTypeBomb )
		{
			return 0;
		}
		else
		{
			if( m_cDiscardingType.m_nTypeNum != cardType.m_nTypeNum )
			{
				return 0;
			}
			else
			{	
				//如果牌型相同;
				if( m_cDiscardingType.m_nTypeValue >  cardType.m_nTypeValue )
				{
					return 1;  //合法;
				}
				else
				{
					return 0; //不合法;
				}
			}
		}
	}
	return 1;
}

void CPlayCards::ScandCardTable()
{
	//初始化扫描表
	memset(m_nDiscardingCardsTable, 0, sizeof(m_nDiscardingCardsTable));
	//扫描进表中
	for(size_t i = 0; i < m_cChoosingCards.size(); i++)
	{
		m_nDiscardingCardsTable[(int)m_cChoosingCards[i].m_nValue]++;
	}
	m_nDiscardingCardsTable[BAO] = m_nDiscardingCardsTable[m_nBaoValue];
	m_nDiscardingCardsTable[m_nBaoValue] = 0;
}

int CPlayCards::GetBomb()
{
	int nBomb = 0;
	TipScanCardTable();

	for(int i = 0; i < 16; i++)
	{
		if( m_nDiscardingCardsTable[i] == 4 )
			nBomb++;
	}
	if( m_nDiscardingCardsTable[16] == 2 )
		nBomb++;
	return nBomb;
}

void CPlayCards::TipScanCardTable()
{
	//初始化扫描表
	memset(m_nDiscardingCardsTable, 0, sizeof(m_nDiscardingCardsTable));
	//扫描进表中;
	for(size_t i = 0; i < m_cCards.size(); i++)
	{
		m_nDiscardingCardsTable[(int)m_cCards[i].m_nValue]++;
	}
}

int CPlayCards::Tips()
{
	//初始化扫描表
	m_vecTipsCards.clear();
	TipScanCardTable();
	//从单张牌开始提示
	for(int i = 1; i <= 4; i++)
	{
		for(int j = 0; j <= 16; j++)
		{
			if( m_nDiscardingCardsTable[j] == i || (j == 16 && m_nDiscardingCardsTable[j] == i && i == 1) )
			{
				VECCARD vecCard;
				CCard cCard(0,j);
				for(int n = 0; n < i; n++)
				{
					vecCard.push_back(cCard);
				}
				m_vecTipsCards.push_back(vecCard);
			}
		}
	}
	//2个王
	if( m_nDiscardingCardsTable[16] == 2 )
	{
		CCard cCard(0,16);
		VECCARD vecCard;
		vecCard.push_back(cCard);
		vecCard.push_back(cCard);
		m_vecTipsCards.push_back(vecCard);
	}
	return 0;
}

int CPlayCards::Tips(CCardsType& cCardsType)
{
	int nRet = 0;
	m_vecTipsCards.clear();
	TipScanCardTable();

	switch(cCardsType.m_nTypeNum) 
	{
	case 1: nRet = TipsSearch1(cCardsType); break;
	case 2: nRet = TipsSearch2(cCardsType); break;
	case 3: nRet = TipsSearch3(cCardsType); break;
	case 4: nRet = TipsSearch4(cCardsType); break;
	case 5: nRet = TipsSearch5(cCardsType); break;
	case 6: nRet = TipsSearch6(cCardsType); break;
	case 7: nRet = TipsSearch7(cCardsType); break;
	case 8: nRet = TipsSearch8(cCardsType); break;
	case 9: nRet = TipsSearch9(cCardsType); break;
	case 10: nRet = TipsSearch10(cCardsType); break;
	case 11: nRet = TipsSearch11(cCardsType); break;
	case 12: nRet = TipsSearch12(cCardsType); break;
	case 31: nRet = TipsSearch31(cCardsType); break;
	case 32: nRet = TipsSearch32(cCardsType); break;
	case 33: nRet = TipsSearch33(cCardsType); break;
	case 222: nRet = TipsSearch222(cCardsType); break;
	case 333: nRet = TipsSearch333(cCardsType); break;
	case 411: nRet = TipsSearch411(cCardsType); break;
	case 422: nRet = TipsSearch422(cCardsType); break;
	case 2222: nRet = TipsSearch2222(cCardsType); break;
	case 3322: nRet = TipsSearch3322(cCardsType); break;
	case 3333: nRet = TipsSearch3333(cCardsType); break;
	case 3311: nRet = TipsSearch3311(cCardsType); break;
	case 22222: nRet = TipsSearch22222(cCardsType); break;
	case 33333: nRet = TipsSearch33333(cCardsType); break;
	case 222222: nRet = TipsSearch222222(cCardsType); break;
	case 333111: nRet = TipsSearch333111(cCardsType); break;
	case 333222: nRet = TipsSearch333222(cCardsType); break;
	case 333333: nRet = TipsSearch333333(cCardsType); break;
	case 2222222: nRet = TipsSearch2222222(cCardsType); break;
	case 22222222: nRet = TipsSearch22222222(cCardsType); break;
	case 33331111: nRet = TipsSearch33331111(cCardsType); break;
	case 222222222: nRet = TipsSearch222222222(cCardsType); break;
	default:
		break;
	}

	if( cCardsType.m_nTypeNum != 4 || (cCardsType.m_nTypeNum == 2 && cCardsType.m_nTypeValue != 16) )
	{
		//炸弹
		for(int i = 0; i < 16; i++)
		{
			if(m_nDiscardingCardsTable[i] >= 4)
			{
				CCard card;
				VECCARD vecCard;
				card.m_nValue = i;
				vecCard.push_back(card);
				vecCard.push_back(card);
				vecCard.push_back(card);
				vecCard.push_back(card);
				m_vecTipsCards.push_back(vecCard);
			}
		}
		//对鬼
		if(m_nDiscardingCardsTable[16] == 2)
		{
			CCard card;
			VECCARD vecCard;
			card.m_nValue = 16;
			vecCard.push_back(card);
			vecCard.push_back(card);
			m_vecTipsCards.push_back(vecCard);
		}
	}
	return nRet;
}

// 在客户端使用，保存符合规则的各种组合
int CPlayCards::TipsSearch1(CCardsType& cCardsType)
{	
	int nBegin = cCardsType.m_nTypeValue + 1;	
	
	if(cCardsType.m_nTypeValue == 16)
	{
		for(size_t i = 0; i < m_cCards.size(); i++)
		{
			if(m_cCards[i].m_nValue == 16 && m_cCards[i].m_nColor == 1)
			{
				CCard card;
				VECCARD vecCard;
				card.m_nValue = 16;
				vecCard.push_back(card);
				m_vecTipsCards.push_back(vecCard);
				return 0;
			}
		}
	}

	for(int i = nBegin; i < 17; i++)
	{
		if(m_nDiscardingCardsTable[i] == 1)
		{
			CCard card;
			VECCARD vecCard;
			card.m_nValue = i;
			vecCard.push_back(card);
			m_vecTipsCards.push_back(vecCard);
		}
	} 

	for(int i = nBegin; i < 17; i++)
	{
		if(m_nDiscardingCardsTable[i] > 1)
		{
			CCard card;
			VECCARD vecCard;
			card.m_nValue = i;
			vecCard.push_back(card);
			m_vecTipsCards.push_back(vecCard);
		}
	}
	return 0;
}

int CPlayCards::TipsSearch2(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;

	for(int i = nBegin; i < 16; i++)
	{
		int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
		if(i == m_nBaoValue)
		{
			nBaoCount = 0;
		}
		if(m_nDiscardingCardsTable[i] + nBaoCount >= 2)
		{
			CCard card;	
			VECCARD vecCard;
			card.m_nValue = i;
			for(int j = 0; j < min(2,m_nDiscardingCardsTable[i]); j++)
			{
				vecCard.push_back(card);
			}
			if(m_nDiscardingCardsTable[i] == 1)
			{
				int nUsedCatCount = min(nBaoCount, 2 - m_nDiscardingCardsTable[i]);
				for(int k = 0; k < nUsedCatCount; k++)
				{
					card.m_nValue = m_nBaoValue;
					vecCard.push_back(card);
				}					
			}
			if(m_nDiscardingCardsTable[i] != 0)
			{
				m_vecTipsCards.push_back(vecCard);
			}			
		}		
	}
	return 0;
}

int CPlayCards::TipsSearch3(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;

	for(int i = nBegin; i < MAXTYPENUM; i++)
	{
		int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
		if(i == m_nBaoValue)
		{
			nBaoCount = 0;
		}
		if(m_nDiscardingCardsTable[i] + nBaoCount >= 3)
		{
			CCard card;		
			VECCARD vecCard;
			card.m_nValue = i;
			for(int j = 0; j < min(3,m_nDiscardingCardsTable[i]); j++)
			{
				vecCard.push_back(card);
			}
			int nUsedCatCount = min(nBaoCount, 3 - m_nDiscardingCardsTable[i]);
			for(int k = 0; k < nUsedCatCount; k++)
			{
				card.m_nValue = m_nBaoValue;
				vecCard.push_back(card);
			}
			if(m_nDiscardingCardsTable[i] != 0)
			{
				m_vecTipsCards.push_back(vecCard);
			}			
		}		
	}
	return 0;
}

int CPlayCards::TipsSearch4(CCardsType& cCardsType)
{   
	int nBegin = cCardsType.m_nTypeValue + 1;
	if(cCardsType.m_nTypeNum == 4)
	{		
		if(cCardsType.m_nTypeBomb == 1)
		{
			for(int i = 3; i < nBegin; i++)
			{
				if(m_nDiscardingCardsTable[i] >= 4)
				{
					CCard card;
					VECCARD vecCard;
					card.m_nValue = i;
					vecCard.push_back(card);
					vecCard.push_back(card);
					vecCard.push_back(card);
					vecCard.push_back(card);
					m_vecTipsCards.push_back(vecCard);
				}
			}

			for(int i = nBegin; i < 16; i++)
			{
				int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
				if(i == m_nBaoValue)
				{
					nBaoCount = 0;
				}
				if(m_nDiscardingCardsTable[i] + nBaoCount >= 4)
				{
					CCard card;
					VECCARD vecCard;
					card.m_nValue = i;
					for(int j = 0; j < min(4,m_nDiscardingCardsTable[i]); j++)
					{
						vecCard.push_back(card);
					}
					int nUsedCatCount = min(nBaoCount, 4 - m_nDiscardingCardsTable[i]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}				
					if(m_nDiscardingCardsTable[i] != 0)
					{
						m_vecTipsCards.push_back(vecCard);
					}			
				}		
			}
		}
		else if(cCardsType.m_nTypeBomb == 2)
		{
			for(int i = nBegin; i < 16; i++)
			{
				if(m_nDiscardingCardsTable[i] >= 4)
				{
					CCard card;
					VECCARD vecCard;
					card.m_nValue = i;
					vecCard.push_back(card);
					vecCard.push_back(card);
					vecCard.push_back(card);
					vecCard.push_back(card);
					m_vecTipsCards.push_back(vecCard);
				}
			}
		}
	}
	else
	{
		for(int i = 3; i < 16; i++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if(i == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i] + nBaoCount >= 4)
			{
				CCard card;		
				VECCARD vecCard;
				card.m_nValue = i;
				for(int j = 0; j < min(4,m_nDiscardingCardsTable[i]); j++)
				{
					vecCard.push_back(card);
				}
				int nUsedCatCount = min(nBaoCount, 4 - m_nDiscardingCardsTable[i]);
				for(int k = 0; k < nUsedCatCount; k++)
				{
					card.m_nValue = m_nBaoValue;
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i] != 0)
				{
					m_vecTipsCards.push_back(vecCard);		
				}
			}
		}
	}

	//对鬼
	if(m_nDiscardingCardsTable[16] == 2)
	{
		CCard card;
		VECCARD vecCard;
		card.m_nValue = 16;
		vecCard.push_back(card);
		vecCard.push_back(card);
		m_vecTipsCards.push_back(vecCard);
	}

	return 0;
}

//三带一
int CPlayCards::TipsSearch31(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 16; i++)
	{
		int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
		if(i == m_nBaoValue)
		{
			nBaoCount = 0;
		}
		if(m_nDiscardingCardsTable[i] + nBaoCount >= 3)
		{
			CCard card;		
			VECCARD vecCard;
			card.m_nValue = i;
			for(int j = 0; j < min(3,m_nDiscardingCardsTable[i]); j++)
			{
				vecCard.push_back(card);
			}
			int nUsedCatCount = min(nBaoCount, 3 - m_nDiscardingCardsTable[i]);
			for (int k = 0; k < nUsedCatCount; k++)
			{
				card.m_nValue = m_nBaoValue;
				vecCard.push_back(card);
			}
			bool bFind = false;
			//寻找正好是单张的牌
			for(int j = 3; j < 17; j++)
			{
				if(m_nDiscardingCardsTable[j] == 1 && (i!=j) && (j != m_nBaoValue))
				{
					CCard card;					
					card.m_nValue = j;
					vecCard.push_back(card);
					if(m_nDiscardingCardsTable[i] != 0)
					{
						bFind = true;
						m_vecTipsCards.push_back(vecCard);
						break;
					}
				}
			}
			//没有正好是单张的牌
			if( !bFind )
			{
				for(int j = 3; j < 17; j++)
				{
					if(m_nDiscardingCardsTable[j] > 1 && (i!=j) && (j != m_nBaoValue))
					{
						CCard card;
						card.m_nValue = j;
						vecCard.push_back(card);
						if(m_nDiscardingCardsTable[i] != 0)
						{
							m_vecTipsCards.push_back(vecCard);
							break;
						}
					}
				}
			}
		}
	}
	return 0;
}
/*
//五顺子
int CPlayCards::TipsSearch5(CCardsType& cCardsType)
{
	int nCount = 0;
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 11; i++)
	{
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		if(m_nDiscardingCardsTable[i] >= 1)
		{
			nCount = 0;
			CCard card;
			VECCARD vecCard;
			for(int j = 0; j < 5; j++)
			{
				int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
				if((i + j) == m_nBaoValue)
				{
					nBaoCount = 0;
				}
				if(m_nDiscardingCardsTable[i + j] + nBaoCount >= 1)
				{
					card.m_nValue = i + j;
					for(int k = 0; k < min(1,m_nDiscardingCardsTable[i + j]); k++)
					{
						vecCard.push_back(card);
					}
					if(m_nDiscardingCardsTable[i + j] < 1)
					{
						int nUsedCatCount = min(nBaoCount, 1 - m_nDiscardingCardsTable[i + j]);
						for(int k = 0; k < nUsedCatCount; k++)
						{
							card.m_nValue = m_nBaoValue;
							vecCard.push_back(card);
						}
						m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;
					}
					nCount++;
				}
			}
			if(nCount == 5)
			{
				m_vecTipsCards.push_back(vecCard);
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;
	}
	TipsSoftBomb(cCardsType);
	return 0;
}*/

//Change by JL for robot
//五顺子
int CPlayCards::TipsSearch5(CCardsType& cCardsType)
{
	int nCount = 0;
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 11; i++)
	{
		if(m_nDiscardingCardsTable[i] >= 1)
		{
			nCount = 0;
			CCard card;
			VECCARD vecCard;
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			for(int j = i; j < i+5; j++)
			{				
				if(j == m_nBaoValue)
				{
					nBaoCount--;
				}
				if(m_nDiscardingCardsTable[j] + nBaoCount >= 1)
				{
					card.m_nValue = j;
					if(m_nDiscardingCardsTable[j] >= 1)
					{
						vecCard.push_back(card);
					}
					else if(nBaoCount > 0)
					{
						nBaoCount--;
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);

					}else{
						break;
					}
					nCount++;
				}else{
					break;
				}
			}
			if(nCount == 5)
			{
				m_vecTipsCards.push_back(vecCard);
			}
		}
	}
	TipsSoftBomb(cCardsType);
	return 0;
}

//三带二
int CPlayCards::TipsSearch32(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 16; i++)
	{
		int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
		if(i == m_nBaoValue)
		{
			nBaoCount = 0;
		}
		if(m_nDiscardingCardsTable[i] + nBaoCount >= 3)
		{
			CCard card;
			VECCARD vecCard;
			card.m_nValue = i;
			for(int j = 0; j < min(3,m_nDiscardingCardsTable[i]); j++)
			{
				vecCard.push_back(card);
			}
			int nUsedCatCount = min(nBaoCount, 3 - m_nDiscardingCardsTable[i]);
			for(int k = 0; k < nUsedCatCount; k++)
			{
				card.m_nValue = m_nBaoValue;
				vecCard.push_back(card);
			}
			for(int j = 3; j < 17; j++)
			{
				if(m_nDiscardingCardsTable[j] >= 2 && (i!=j) && (j != m_nBaoValue))
				{
					CCard card;
					card.m_nValue = j;
					vecCard.push_back(card);
					vecCard.push_back(card);
					if (m_nDiscardingCardsTable[i] != 0)
					{
						m_vecTipsCards.push_back(vecCard);
						break;
					}
				}
			}
		}
	}

	return 0;
}

//六顺子
int CPlayCards::TipsSearch6(CCardsType& cCardsType)
{
	int nCount = 0;
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 10; i++)
	{
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];		
		if(m_nDiscardingCardsTable[i] >= 1)
		{
			nCount = 0;
			CCard card;
			VECCARD vecCard;
			for(int j = 0; j < 6; j++)
			{
				int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
				if((i + j) == m_nBaoValue)
				{
					nBaoCount = 0;
				}
				if(m_nDiscardingCardsTable[i + j] + nBaoCount >= 1)
				{
					card.m_nValue = i + j;
					for(int k = 0; k < min(1,m_nDiscardingCardsTable[i + j]); k++)
					{
						vecCard.push_back(card);
					}
					if(m_nDiscardingCardsTable[i + j] < 1)
					{
						int nUsedCatCount = min(nBaoCount, 1 - m_nDiscardingCardsTable[i + j]);
						for(int k = 0; k < nUsedCatCount; k++)
						{
							card.m_nValue = m_nBaoValue;
							vecCard.push_back(card);
						}
						m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;
					}
					nCount++;
				}
			}
			if(nCount == 6)
			{
				m_vecTipsCards.push_back(vecCard);
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;
	}
	TipsSoftBomb(cCardsType);

	return 0;
}

int CPlayCards::TipsSearch222(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 13; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 3; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 2)
			{
				CCard card;						
				card.m_nValue = i + j;
				for(int k = 0; k < min(2,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i + j] < 2)
				{
					int nUsedCatCount = min(nBaoCount, 2 - m_nDiscardingCardsTable[i + j]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;					
				}				
			}
			else
			{				
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;
		
		if(bContinue)
		{
			m_vecTipsCards.push_back(vecCard);
		}		
	}
	TipsSoftBomb(cCardsType);
	
	return 0;
}

// 两坎
int CPlayCards::TipsSearch33(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 14; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 2; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 3)
			{
				CCard card;
				card.m_nValue = i + j;
				for(int k = 0; k < min(3,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i + j] < 3)
				{
					int nUsedCatCount = min(nBaoCount, 3 - m_nDiscardingCardsTable[i + j]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;
				}
			}
			else
			{
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			m_vecTipsCards.push_back(vecCard);
		}
	}
	TipsSoftBomb(cCardsType);
	return 0;
}

// 四带两单
int CPlayCards::TipsSearch411(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 17; i++)
	{
		int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
		if(i == m_nBaoValue)
		{
			nBaoCount = 0;
		}		
		if(m_nDiscardingCardsTable[i] + nBaoCount >= 4)
		{			
			VECCARD vecCard;
			CCard card;		
			card.m_nValue = i;
			for(int j = 0; j < min(4,m_nDiscardingCardsTable[i]); j++)
			{
				vecCard.push_back(card);
			}
			int nUsedCatCount = min(nBaoCount, 4 - m_nDiscardingCardsTable[i]);
			for(int k = 0; k < nUsedCatCount; k++)
			{
				card.m_nValue = m_nBaoValue;
				vecCard.push_back(card);
			}
			int nCount = 0;
			for(int j = 3; j < 17; j++)
			{
				if(m_nDiscardingCardsTable[j] >= 1 && (i!=j) && (j != m_nBaoValue))
				{
					CCard card;					
					card.m_nValue = j;
					vecCard.push_back(card);
					if(++nCount >= 2)
					{
						if(m_nDiscardingCardsTable[i] != 0)
						{
							m_vecTipsCards.push_back(vecCard);
						}	
						break;
					}
				}
			}
		}
	}
	return 0;
}

// 7顺
int CPlayCards::TipsSearch7(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	int nCount = 0;
	for(int i = nBegin; i < 9; i++)
	{
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];		
		if(m_nDiscardingCardsTable[i] >= 1)
		{
			nCount = 0;
			CCard card;
			VECCARD vecCard;
			for(int j = 0; j < 7; j++)
			{
				int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
				if((i + j) == m_nBaoValue)
				{
					nBaoCount = 0;
				}
				if(m_nDiscardingCardsTable[i + j] + nBaoCount >= 1)
				{
					card.m_nValue = i + j;
					for(int k = 0; k < min(1,m_nDiscardingCardsTable[i + j]); k++)
					{
						vecCard.push_back(card);
					}
					if(m_nDiscardingCardsTable[i + j] < 1)
					{
						int nUsedCatCount = min(nBaoCount, 1 - m_nDiscardingCardsTable[i + j]);
						for(int k = 0; k < nUsedCatCount; k++)
						{
							card.m_nValue = m_nBaoValue;
							vecCard.push_back(card);
						}
						m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;
					}
					nCount++;
				}
			}
			if(nCount == 7)
			{
				m_vecTipsCards.push_back(vecCard);
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;		
	}
	TipsSoftBomb(cCardsType);
	return 0;
}

// 8顺
int CPlayCards::TipsSearch8(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	int nCount = 0;
	for(int i = nBegin; i < 8; i++)
	{
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		if(m_nDiscardingCardsTable[i] >= 1)
		{
			nCount = 0;
			CCard card;
			VECCARD vecCard;
			for(int j = 0; j < 8; j++)
			{
				int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
				if((i + j) == m_nBaoValue)
				{
					nBaoCount = 0;
				}
				if(m_nDiscardingCardsTable[i + j] + nBaoCount >= 1)
				{
					card.m_nValue = i + j;
					for(int k = 0; k < min(1,m_nDiscardingCardsTable[i + j]); k++)
					{
						vecCard.push_back(card);
					}
					if(m_nDiscardingCardsTable[i + j] < 1)
					{
						int nUsedCatCount = min(nBaoCount, 1 - m_nDiscardingCardsTable[i + j]);
						for(int k = 0; k < nUsedCatCount; k++)
						{
							card.m_nValue = m_nBaoValue;
							vecCard.push_back(card);
						}
						m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;
					}
					nCount++;
				}
			}
			if(nCount == 8)
			{
				m_vecTipsCards.push_back(vecCard);
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;
	}
	TipsSoftBomb(cCardsType);
	return 0;
}

//四对
int CPlayCards::TipsSearch2222(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 12; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 4; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 2)
			{
				CCard card;						
				card.m_nValue = i + j;
				for(int k = 0; k < min(2,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i + j] < 2)
				{
					int nUsedCatCount = min(nBaoCount, 2 - m_nDiscardingCardsTable[i + j]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;					
				}				
			}
			else
			{				
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			m_vecTipsCards.push_back(vecCard);
		}		
	}
	TipsSoftBomb(cCardsType);
	return 0;
}

int CPlayCards::TipsSearch3311(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 14; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 2; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 3)
			{
				CCard card;						
				card.m_nValue = i + j;
				for(int k = 0; k < min(3,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i + j] < 3)
				{
					int nUsedCatCount = min(nBaoCount, 3 - m_nDiscardingCardsTable[i + j]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;					
				}				
			}
			else
			{				
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			int nCount = 0;
			for(int k = 3; k < 17; k++)
			{
				if(m_nDiscardingCardsTable[k] >= 1 && (i!=k) && (k != i+1) && (k != m_nBaoValue))
				{
					CCard card;					
					card.m_nValue = k;
					vecCard.push_back(card);
					if(++nCount >= 2)
					{
						if(m_nDiscardingCardsTable[i] != 0)
						{
							m_vecTipsCards.push_back(vecCard);
						}
						break;
					}
				}
			}
		}
	}
	return 0;
}

//四张带两对
int CPlayCards::TipsSearch422(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 17; i++)
	{
		int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
		if(i == m_nBaoValue)
		{
			nBaoCount = 0;
		}		
		if(m_nDiscardingCardsTable[i] + nBaoCount >= 4)
		{			
			CCard card;		
			VECCARD vecCard;
			card.m_nValue = i;
			for(int j = 0; j < min(4,m_nDiscardingCardsTable[i]); j++)
			{
				vecCard.push_back(card);
			}
			int nUsedCatCount = min(nBaoCount, 4 - m_nDiscardingCardsTable[i]);
			for(int k = 0; k < nUsedCatCount; k++)
			{
				card.m_nValue = m_nBaoValue;
				vecCard.push_back(card);
			}	
			int nCount = 0;
			for(int m = 3; m < 17; m++)
			{
				if(m_nDiscardingCardsTable[m] >= 2 && (i!=m) && (m != m_nBaoValue))
				{
					CCard card;					
					card.m_nValue = m;
					vecCard.push_back(card);
					vecCard.push_back(card);
					if(++nCount >= 2)
					{
						if(m_nDiscardingCardsTable[i] != 0)
						{
							m_vecTipsCards.push_back(vecCard);
						}
						break;
					}
				}
			}
		}
	}
	return 0;
}

// 9顺
int CPlayCards::TipsSearch9(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	int nCount = 0;

	for(int i = nBegin; i < 7; i++)
	{
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];		
		if(m_nDiscardingCardsTable[i] >= 1)
		{
			nCount = 0;
			CCard card;
			VECCARD vecCard;
			for(int j = 0; j < 9; j++)
			{
				int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
				if ((i + j) == m_nBaoValue)
				{
					nBaoCount = 0;
				}
				if(m_nDiscardingCardsTable[i + j] + nBaoCount >= 1)
				{										
					card.m_nValue = i + j;
					for(int k = 0; k < min(1,m_nDiscardingCardsTable[i + j]); k++)
					{
						vecCard.push_back(card);
					}
					if(m_nDiscardingCardsTable[i + j] < 1)
					{
						int nUsedCatCount = min(nBaoCount, 1 - m_nDiscardingCardsTable[i + j]);
						for (int k = 0; k < nUsedCatCount; k++)
						{
							card.m_nValue = m_nBaoValue;
							vecCard.push_back(card);
						}
						m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;										
					}				
					nCount++;
				}
			}
			if(nCount == 9)
			{
				m_vecTipsCards.push_back(vecCard);
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;		
	}
	TipsSoftBomb(cCardsType);
	return 0;
}

//三坎
int CPlayCards::TipsSearch333(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 13; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 3; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 3)
			{
				CCard card;						
				card.m_nValue = i + j;
				for(int k = 0; k < min(3,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i + j] < 3)
				{
					int nUsedCatCount = min(nBaoCount, 3 - m_nDiscardingCardsTable[i + j]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;
				}				
			}
			else
			{				
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			m_vecTipsCards.push_back(vecCard);
		}
	}
	return 0;
}

// 10顺
int CPlayCards::TipsSearch10(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	int nCount = 0;

	for(int i = nBegin; i < 6; i++)
	{
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];		
		if(m_nDiscardingCardsTable[i] >= 1)
		{
			nCount = 0;
			CCard card;
			VECCARD vecCard;
			for(int j = 0; j < 10; j++)
			{
				int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
				if((i + j) == m_nBaoValue)
				{
					nBaoCount = 0;
				}
				if(m_nDiscardingCardsTable[i + j] + nBaoCount >= 1)
				{										
					card.m_nValue = i + j;
					for(int k = 0; k < min(1,m_nDiscardingCardsTable[i + j]); k++)
					{
						vecCard.push_back(card);
					}
					if(m_nDiscardingCardsTable[i + j] < 1)
					{
						int nUsedCatCount = min(nBaoCount, 1 - m_nDiscardingCardsTable[i + j]);
						for(int k = 0; k < nUsedCatCount; k++)
						{
							card.m_nValue = m_nBaoValue;
							vecCard.push_back(card);
						}
						m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;										
					}
					nCount++;
				}
			}
			if(nCount == 10)
			{
				m_vecTipsCards.push_back(vecCard);
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;		
	}

	TipsSoftBomb(cCardsType);
	return 0;
}

//五对
int CPlayCards::TipsSearch22222(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 11; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 5; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 2)
			{
				CCard card;
				card.m_nValue = i + j;
				for(int k = 0; k < min(2,m_nDiscardingCardsTable[i+j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i+j] < 2)
				{
					int nUsedCatCount = min(nBaoCount, 2 - m_nDiscardingCardsTable[i+j]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;					
				}
			}
			else
			{
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			m_vecTipsCards.push_back(vecCard);
		}
	}
	TipsSoftBomb(cCardsType);
	return 0;
}

//两坎两对
int CPlayCards::TipsSearch3322(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 14; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 2; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 3)
			{
				CCard card;
				card.m_nValue = i + j;
				for(int k = 0; k < min(3,m_nDiscardingCardsTable[i+j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i+j] < 3)
				{
					int nUsedCatCount = min(nBaoCount, 3 - m_nDiscardingCardsTable[i + j]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;					
				}				
			}
			else
			{
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			int nCount = 0;
			for(int k = 3; k < 17; k++)
			{
				if(m_nDiscardingCardsTable[k] >= 2 && (i!=k) && (k != i+1) && (k != m_nBaoValue))
				{
					CCard card;
					card.m_nValue = k;
					vecCard.push_back(card);
					vecCard.push_back(card);
					if(++nCount >= 2)
					{
						if (m_nDiscardingCardsTable[i] != 0)
						{
							m_vecTipsCards.push_back(vecCard);
						}	
						break;
					}
				}
			}
		}
	}
	return 0;
}

int CPlayCards::TipsSearch11(CCardsType& cCardsType)
{
	int nCount = 0;
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 5; i++)
	{
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];		
		if(m_nDiscardingCardsTable[i] >= 1)
		{
			nCount = 0;
			CCard card;
			VECCARD vecCard;
			for(int j = 0; j < 11; j++)
			{
				int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
				if((i + j) == m_nBaoValue)
				{
					nBaoCount = 0;
				}
				if(m_nDiscardingCardsTable[i + j] + nBaoCount >= 1)
				{
					card.m_nValue = i + j;
					for(int k = 0; k < min(1,m_nDiscardingCardsTable[i + j]); k++)
					{
						vecCard.push_back(card);
					}
					if(m_nDiscardingCardsTable[i + j] < 1)
					{
						int nUsedCatCount = min(nBaoCount, 1 - m_nDiscardingCardsTable[i + j]);
						for(int k = 0; k < nUsedCatCount; k++)
						{
							card.m_nValue = m_nBaoValue;
							vecCard.push_back(card);
						}
						m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;										
					}
					nCount++;
				}
			}
			if(nCount == 11)
			{
				m_vecTipsCards.push_back(vecCard);
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;		
	}

	TipsSoftBomb(cCardsType);
	return 0;
}

int CPlayCards::TipsSearch12(CCardsType& cCardsType)
{
	int nCount = 0;
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 4; i++)
	{
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];		
		if(m_nDiscardingCardsTable[i] >= 1)
		{
			nCount = 0;
			CCard card;
			VECCARD vecCard;
			for(int j = 0; j < 12; j++)
			{
				int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
				if((i + j) == m_nBaoValue)
				{
					nBaoCount = 0;
				}
				if(m_nDiscardingCardsTable[i + j] + nBaoCount >= 1)
				{
					card.m_nValue = i + j;
					for(int k = 0; k < min(1,m_nDiscardingCardsTable[i + j]); k++)
					{
						vecCard.push_back(card);
					}
					if(m_nDiscardingCardsTable[i + j] < 1)
					{
						int nUsedCatCount = min(nBaoCount, 1 - m_nDiscardingCardsTable[i + j]);
						for(int k = 0; k < nUsedCatCount; k++)
						{
							card.m_nValue = m_nBaoValue;
							vecCard.push_back(card);
						}
						m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;										
					}				
					nCount++;
				}
			}
			if(nCount == 12)
			{
				m_vecTipsCards.push_back(vecCard);
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;		
	}

	TipsSoftBomb(cCardsType);
	return 0;
}

//六对
int CPlayCards::TipsSearch222222(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 10; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 6; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 2)
			{
				CCard card;						
				card.m_nValue = i + j;
				for(int k = 0; k < min(2,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i + j] < 2)
				{
					int nUsedCatCount = min(nBaoCount, 2 - m_nDiscardingCardsTable[i + j]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;					
				}				
			}
			else
			{
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			m_vecTipsCards.push_back(vecCard);
		}
	}
	TipsSoftBomb(cCardsType);
	return 0;
}

//四坎
int CPlayCards::TipsSearch3333(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 12; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 4; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 3)
			{
				CCard card;						
				card.m_nValue = i + j;
				for(int k = 0; k < min(3,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i + j] < 3)
				{
					int nUsedCatCount = min(nBaoCount, 3 - m_nDiscardingCardsTable[i + j]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;					
				}				
			}
			else
			{
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			m_vecTipsCards.push_back(vecCard);
		}
	}
	return 0;
}

//三坎带三单张
int CPlayCards::TipsSearch333111(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 13; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 3; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 3)
			{
				CCard card;						
				card.m_nValue = i + j;
				for(int k = 0; k < min(3,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i + j] < 3)
				{
					int nUsedCatCount = min(nBaoCount, 3 - m_nDiscardingCardsTable[i + j]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;					
				}				
			}
			else
			{				
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			int nCount = 0;
			for(int k = 3; k < 17; k++)
			{
				if(m_nDiscardingCardsTable[k] >= 1 && (i!=k) && (k != i+1) && (k != i+2)&& (k != m_nBaoValue))
				{
					CCard card;					
					card.m_nValue = k;
					vecCard.push_back(card);
					if(++nCount >= 3)
					{
						if (m_nDiscardingCardsTable[i] != 0)
						{
							m_vecTipsCards.push_back(vecCard);
						}	
						break;
					}
				}
			}
		}
	}
	return 0;
}

//七对
int CPlayCards::TipsSearch2222222(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 9; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 7; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 2)
			{
				CCard card;						
				card.m_nValue = i + j;
				for (int k = 0; k < min(2,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i + j] < 2)
				{
					int nUsedCatCount = min(nBaoCount, 2 - m_nDiscardingCardsTable[i + j]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;					
				}				
			}
			else
			{				
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			m_vecTipsCards.push_back(vecCard);
		}		
	}
	TipsSoftBomb(cCardsType);
	return 0;
}

int CPlayCards::TipsSearch33333(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 11; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 5; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 3)
			{
				CCard card;
				card.m_nValue = i + j;
				for(int k = 0; k < min(3,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i + j] < 3)
				{
					int nUsedCatCount = min(nBaoCount, 3 - m_nDiscardingCardsTable[i + j]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;
				}
			}
			else
			{
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			m_vecTipsCards.push_back(vecCard);
		}
	}
	return 0;
}


int CPlayCards::TipsSearch22222222(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 8; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 8; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 2)
			{
				CCard card;
				card.m_nValue = i + j;
				for (int k = 0; k < min(2,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i + j] < 2)
				{
					int nUsedCatCount = min(nBaoCount, 2 - m_nDiscardingCardsTable[i + j]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;
				}
			}
			else
			{
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			m_vecTipsCards.push_back(vecCard);
		}		
	}
	TipsSoftBomb(cCardsType);
	return 0;
}

//三坎三对
int CPlayCards::TipsSearch333222(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 13; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 3; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 3)
			{
				CCard card;
				card.m_nValue = i + j;
				for(int k = 0; k < min(3,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i + j] < 3)
				{
					int nUsedCatCount = min(nBaoCount, 3 - m_nDiscardingCardsTable[i + j]);
					for(int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;					
				}
			}
			else
			{				
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			int nCount = 0;
			for(int k = 3; k < 17; k++)
			{
				if(m_nDiscardingCardsTable[k] >= 2 && (i!=k) && (k != i+1) && (k != i+2) && (k != m_nBaoValue))
				{
					CCard card;					
					card.m_nValue = k;
					vecCard.push_back(card);
					vecCard.push_back(card);
					if(++nCount >= 3)
					{
						if(m_nDiscardingCardsTable[i] != 0)
						{
							m_vecTipsCards.push_back(vecCard);
						}
						break;
					}
				}
			}
		}
	}
	return 0;
}

int CPlayCards::TipsSearch222222222(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 7; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 9; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 2)
			{
				CCard card;
				card.m_nValue = i + j;
				for(int k = 0; k < min(2,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if (m_nDiscardingCardsTable[i + j] < 2)
				{
					int nUsedCatCount = min(nBaoCount, 2 - m_nDiscardingCardsTable[i + j]);
					for (int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;					
				}
			}
			else
			{
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			m_vecTipsCards.push_back(vecCard);
		}
	}
	TipsSoftBomb(cCardsType);
	return 0;
}

int CPlayCards::TipsSearch33331111(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 12; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 4; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 3)
			{
				CCard card;
				card.m_nValue = i + j;
				for(int k = 0; k < min(3,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i + j] < 3)
				{
					int nUsedCatCount = min(nBaoCount, 3 - m_nDiscardingCardsTable[i + j]);
					for (int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;					
				}
			}
			else
			{
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			int nCount = 0;
			for(int k = 3; k < 17; k++)
			{
				if(m_nDiscardingCardsTable[k] >= 1 && (i!=k) && (k != i+1) &&
					(k != i+2) && (k != i+3) && (k != m_nBaoValue))
				{
					CCard card;					
					card.m_nValue = k;
					vecCard.push_back(card);
					if(++nCount >= 4)
					{
						if(m_nDiscardingCardsTable[i] != 0)
						{
							m_vecTipsCards.push_back(vecCard);
						}
						break;
					}
				}
			}
		}
	}
	return 0;
}

int CPlayCards::TipsSearch333333(CCardsType& cCardsType)
{
	int nBegin = cCardsType.m_nTypeValue + 1;
	for(int i = nBegin; i < 10; i++)
	{
		bool bContinue = true;
		VECCARD vecCard;
		int nTmpCatTable = m_nDiscardingCardsTable[m_nBaoValue];
		for(int j = 0; j < 6; j++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if((i + j) == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i+j] + nBaoCount >= 3)
			{
				CCard card;
				card.m_nValue = i + j;
				for (int k = 0; k < min(3,m_nDiscardingCardsTable[i + j]); k++)
				{
					vecCard.push_back(card);
				}
				if(m_nDiscardingCardsTable[i + j] < 3)
				{
					int nUsedCatCount = min(nBaoCount, 3 - m_nDiscardingCardsTable[i + j]);
					for (int k = 0; k < nUsedCatCount; k++)
					{
						card.m_nValue = m_nBaoValue;
						vecCard.push_back(card);
					}
					m_nDiscardingCardsTable[m_nBaoValue] -= nUsedCatCount;					
				}
			}
			else
			{				
				bContinue = false;
				vecCard.clear();
				break;
			}
		}
		m_nDiscardingCardsTable[m_nBaoValue] = nTmpCatTable;

		if(bContinue)
		{
			m_vecTipsCards.push_back(vecCard);
		}		
	}
	return 0;
}

void CPlayCards::SetBaoValue(const CCard& card)
{
	/*
	int nValue = card.m_nValue;
	if(nValue == 15)
		m_nBaoValue = 3;
	else if(nValue == 16 || nValue == 17)
		m_nBaoValue = 3;
	else
		m_nBaoValue = nValue + 1;
	*/
	m_nBaoValue = card.m_nValue;
}

bool CPlayCards::CheckTigerValue(int nTigerCount)
{	
	if (m_nDiscardingCardsTable[BAO] >= nTigerCount)
	{
		m_nDiscardingCardsTable[BAO] -= nTigerCount;
		return true;
	}
	return false;
}

void CPlayCards::RevertTigerValue()
{
	m_nDiscardingCardsTable[BAO] = 0;	
	m_nDiscardingCardsTable[m_nBaoValue] = 0;

	//扫描进表中;
	for(size_t i = 0; i < m_cChoosingCards.size(); i++)
	{
		if((int)m_cChoosingCards[i].m_nValue == m_nBaoValue)
		{
			m_nDiscardingCardsTable[BAO]++;
			m_cChoosingCards[i].m_nCard_Baovalue = -1;//Add by JL for robot
		}
	}
}

bool CPlayCards::SelectCard( VECCARD& vecCards )
{
	bool is_get_selected = true ;
	TipScanCardTable();
	m_cChoosingCards.clear();
	m_tempCard.clear();
	m_tempCard = m_cCards;
	for(size_t i=0;i<vecCards.size();i++)
	{
		if(m_nDiscardingCardsTable[(int)(vecCards[i].m_nValue)]>0)
		{
			for(size_t j=0;j<m_tempCard.size();j++)
			{
				if(m_tempCard[j].m_nValue == vecCards[i].m_nValue )
				{
					m_cChoosingCards.push_back(m_cCards[j]);
					m_tempCard[j].m_nValue = 0;
					m_tempCard[j].m_nColor = 0;
					m_nDiscardingCardsTable[(int)(vecCards[i].m_nValue)]--;
					//g_pLogger->Log("tips_card:%d",vecCards[i].m_nValue);
					break;
				}
			}
		}
	}
	return is_get_selected;
}


bool CPlayCards::CheckCards(VECCARD& vecCards)
{
	int nCount = 0;
	for(size_t i = 0; i < m_cCards.size(); i++)
	{
		for(size_t j = 0; j < vecCards.size(); j++)
		{
			if(m_cCards[i] == vecCards[j])
			{
				nCount++;
				break;
			}
		}
	}
	return (nCount == (int)vecCards.size());
}

int CPlayCards::TipsSoftBomb( CCardsType& cCardsType )
{
	if(m_nBaoValue > 0)
	{
		int nBegin = cCardsType.m_nTypeValue + 1;
		for(int i = nBegin; i < 16; i++)
		{
			int nBaoCount = m_nDiscardingCardsTable[m_nBaoValue];
			if(i == m_nBaoValue)
			{
				nBaoCount = 0;
			}
			if(m_nDiscardingCardsTable[i] + nBaoCount >= 4)
			{
				CCard card;
				VECCARD vecCard;
				card.m_nValue = i;
				for(int j = 0; j < min(4,m_nDiscardingCardsTable[i]); j++)
				{
					vecCard.push_back(card);
				}
				int nUsedCatCount = min(nBaoCount, 4 - m_nDiscardingCardsTable[i]);
				for(int k = 0; k < nUsedCatCount; k++)
				{
					card.m_nValue = m_nBaoValue;
					vecCard.push_back(card);
				}				
				if(m_nDiscardingCardsTable[i] != 0)
				{
					m_vecTipsCards.push_back(vecCard);
				}
			}	
		}
		
	}
	return 0;
}
//Delete by JL for robot
/*
void CPlayCards::RevertCardValue( int value,int num)
{
	for(int a =0;a<num;a++)
	{
		for(int i=0;i<m_cChoosingCards.size();i++)
		{
			if(m_cChoosingCards[i].m_nValue == m_nBaoValue && m_cChoosingCards[i].m_nCard_Baovalue < 3)
			{
				m_cChoosingCards[i].m_nCard_Baovalue = value;
			}
		}
	}
}*/
//Add by JL for robot
void CPlayCards::RevertCardValue( int value,int num)
{
	for(size_t i=0;i<m_cChoosingCards.size();i++)
	{
		if(m_cChoosingCards[i].m_nValue == m_nBaoValue && m_cChoosingCards[i].m_nCard_Baovalue < 3)
		{
			m_cChoosingCards[i].m_nCard_Baovalue = value;
			if(--num <= 0)
			{
				return;
			}
		}
	}
}