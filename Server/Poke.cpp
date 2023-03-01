/*
 @Date:	        2013/4/15
 @Created:	    9:10
 @Filename: 	Poke.cpp
 @Author:		wulei
 @Description:	
*/

#include "stdafx.h"
#include "Poke.h"
#include "ConfigManager.h"
#include "ddz_interface.h"

int ParseLine(string strLine, vector<string>& retVec, const char cTrim)
{
	std::string::size_type pos1, pos2;
	pos2 = 0;
	while (pos2 != std::string::npos)
	{
		pos1 = strLine.find_first_not_of(cTrim, pos2);
		if (pos1 == std::string::npos)
			break;
		pos2 = strLine.find_first_of(cTrim, pos1 + 1);
		if (pos2 == std::string::npos)
		{
			if (pos1 != strLine.size())
				retVec.push_back(strLine.substr(pos1)); 
			break;
		}
		retVec.push_back(strLine.substr(pos1, pos2 - pos1));
	}
	return retVec.size();
}

CPoke::CPoke()
{
}

CPoke::~CPoke()
{
}

int CPoke::SetLaiziValue( CCard& card)
{
	int nValue = card.m_nValue;
	if (nValue >= 15)
	{
		if (g_nTwoPai == 1)
		{
			return 5;
		}
		return 3;
	}
	else
	{
		return  nValue + 1;
	}
}

void CPoke::ShuffleDefaultCards()
{
	SERVER_LOG("---------------shufflecadrd fail------------");
	if (g_nLetCard == 1)
	{
		int nStartCard = 3;
		if (g_nLetCard == 1)
		{
			nStartCard = g_nTwoPai == 1 ? 5 : 9;
		}
		vector<CCard> vecTemp;
		//二人斗地主默认从5开始
		for (int i = nStartCard; i <= 15; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				CCard Card;
				Card.m_nColor = j;
				Card.m_nValue = i;
				vecTemp.push_back(Card);
			}
		}
		//小王,大王
		for(int i = 0; i <= 1; i++)
		{
			CCard Card;
			Card.m_nColor = i;
			Card.m_nValue = 16;
			vecTemp.push_back(Card);
		}
		//洗牌
		random_shuffle(vecTemp.begin(), vecTemp.end());
		
		//添加 3 4
		if (g_nTwoPai == 1)
		{
			for (int i = 3; i <= nStartCard; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					CCard Card;
					Card.m_nValue = i;
					Card.m_nColor = j;
					m_AllCards.push_back(Card);
				}
			}
		}
		//合并
		m_AllCards.insert(m_AllCards.end(), vecTemp.begin(), vecTemp.end());
	}
	else
	{
		for (int i = 3; i <= 15; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				CCard Card;
				Card.m_nColor = j;
				Card.m_nValue = i;
				m_AllCards.push_back(Card);
			}
		}
		//小王,大王
		for (int i = 0; i <= 1; i++)
		{
			CCard Card;
			Card.m_nColor = i;
			Card.m_nValue = 16;
			m_AllCards.push_back(Card);
		}

		//洗牌
		random_shuffle(m_AllCards.begin(), m_AllCards.end());
	}
	if(g_nSendCardNum>0)
	{

	}
	else
	{
		if(g_nZhaDanDef>0 && g_nZhaDanDef<11)  //炸弹几率
		{
			int n = rand()%10;
			if(n <= (g_nZhaDanDef-1))  //特殊流程
			{
				SetZhaDanCard();
			}
		}
	}
	//地主牌
	for(int i = 0; i < 3; i++)
	{
		m_cLordCard.push_back(GetCard());
	}

	//laizi
	m_cLaiziCard.m_nValue = -1;
	if (g_nBaoPai)
	{
		SortLordCards();
		m_cLaiziCard.m_nValue = SetLaiziValue(m_cLordCard[1]);
	}
}

// bool CPoke::ShuffleCardsDB(int send_card_type,int send_card_num,int n_double)
// {
// 	static int rand_key = 0; 
// 	CppSQLite3DB*	db_ = NULL;
// 	db_ = CConfigManager::GetInstancePtr()->db_sqlite();

// 	if(db_)	//加载数据库成功
// 	{
// 		boost::asio::detail::mutex::scoped_lock lock(CConfigManager::GetInstancePtr()->db_mutex());
// 		string str_table ;
// 		str_table =  CConfigManager::GetInstancePtr()->get_current_table_query(send_card_type,n_double);
// 		string player_[3];
// 		string lord_card;
// 		CppSQLite3Table query = db_->getTable(str_table.c_str());
// 		g_pLogger->Log("-------------output: %s",str_table.c_str());
// 		if(query.numRows()>0) //查询数据不为空
// 		{
// 			g_pLogger->Log("-------------output:%d",query.numRows());
// 			srand(time(0)+(rand_key++));
// 			int tempRow = rand()%query.numRows();
// 			query.setRow(tempRow);//在每一次需要获取数据的时候都要设置要访问的行值 
// 			g_pLogger->Log("------------tempRow = %d",tempRow);
// 			player_[0] = query.fieldValue(1);
// 			player_[1] = query.fieldValue(2);
// 			player_[2] = query.fieldValue(3);
// 			lord_card = query.fieldValue(4);

// 			vector<CCard> card_[3];
// 			char delims[] = ",";
// 			CCard card;
// 			char* result = NULL;

// 			for(size_t n=0;n<3;n++)
// 			{
// 				char* buf = new char[strlen(player_[n].c_str())+1];
// 				strcpy(buf,player_[n].c_str());
// 				result = strtok(buf,delims);
// 				while(result != NULL)
// 				{
// 					card.m_nValue = atoi(result)%16;
// 					card.m_nColor = atoi(result)/16;
// 					if(card.m_nValue == 0)
// 					{
// 						card.m_nValue = 16;
// 						card.m_nColor -=1;
// 					}
// 					//g_pLogger->Log("%d        %d",card.m_nValue,card.m_nColor);
// 					card_[n].push_back(card);
// 					result = strtok(NULL,delims);
// 				}
// 			}


// 			for(size_t n=0;n<3;n++)
// 			{
// 				for(size_t m=0;m<17;m++)
// 				{
// 					m_AllCards.push_back(card_[n][m]);
// 				}
// 			}

// 			char* buf = new char[strlen(lord_card.c_str())+1];
// 			strcpy(buf,lord_card.c_str());
// 			result = strtok(buf,delims);
// 			while(result != NULL)
// 			{
// 				card.m_nValue = atoi(result)%16;
// 				card.m_nColor = atoi(result)/16;
// 				if(card.m_nValue == 0)
// 				{
// 					card.m_nValue = 16;
// 					card.m_nColor -=1;
// 				}
// 				//g_pLogger->Log("%d  %d",card.m_nValue,card.m_nColor);
// 				m_cLordCard.push_back(card);
// 				result = strtok(NULL,delims);
// 			}

// 			//laizi
// 			m_cLaiziCard.m_nValue = -1;
// 			if (g_nBaoPai)
// 			{
// 				SortLordCards();
// 				m_cLaiziCard.m_nValue = SetLaiziValue(m_cLordCard[1]);
// 			}

// 			return true;
// 		}
// 	}
// 	return false;
// }
bool CPoke::ShuffleCardsFile(int send_card_type, int send_card_num, int n_double)
{
	char szBuffer[512];
	FILE* fp = fopen("cards.csv", "rt");
	if (NULL == fp)
	{
		return false;
	}

	vector<CCard> vecAllCards;
	ShuffleDefaultCards();

	for (size_t i = 0; i < m_cLordCard.size(); i++)
	{
		m_AllCards.push_back(m_cLordCard[i]);
	}
	m_cLordCard.clear();

	while (fgets(szBuffer, 512, fp))
	{
		if (strchr(szBuffer, '#') != NULL) //包涵#字符行为注释行
			continue;
		vector<string> vecRet;
		int nItem = ParseLine(szBuffer, vecRet, ',');
		for (int i = 0; i < nItem; ++i)
		{
			int nCard = atoi(vecRet[i].c_str());
			CCard Card;
			Card.m_nColor = nCard % 10;
			Card.m_nValue = nCard / 10;

			if (Card.m_nColor < 0 || Card.m_nValue < 0 || (Card.m_nColor > 3 && Card.m_nValue > 15) || (Card.m_nColor > 1 && Card.m_nValue == 16))
			{
				continue;
			}

			for (size_t i = 0; i < m_AllCards.size(); i++)
			{
				if (m_AllCards[i] == Card)
				{
					vecAllCards.push_back(Card);
					m_AllCards.erase(m_AllCards.begin() + i);
					break;
				}
			}
		}
	}
	fclose(fp);

	//设置3张底牌
	fp = fopen("lord_cards.csv", "rt");
	vector<CCard> vecLordCards;
	if (fp != NULL)
	{
		while (fgets(szBuffer, 512, fp))
		{
			if (strchr(szBuffer, '#') != NULL) //包涵#字符行为注释行
			{
				continue;
			}

			vector<string> vecRet;
			int nItem = ParseLine(szBuffer, vecRet, ',');
			for (int i = 0; i < nItem; ++i)
			{
				int nCard = atoi(vecRet[i].c_str());
				CCard Card;
				Card.m_nColor = nCard % 10;
				Card.m_nValue = nCard / 10;

				if (Card.m_nColor < 0 || Card.m_nValue < 0 || (Card.m_nColor > 3 && Card.m_nValue > 15) || (Card.m_nColor > 1 && Card.m_nValue == 16))
				{
					continue;
				}

				for (size_t i = 0; i < m_AllCards.size(); i++)
				{
					if (m_AllCards[i] == Card)
					{
						vecLordCards.push_back(Card);
						m_AllCards.erase(m_AllCards.begin() + i);
						break;
					}
				}
			}
		}
		fclose(fp);
	}

	for (size_t i = 0; i < min(vecLordCards.size(), 3); i++)
	{
		m_cLordCard.push_back(vecLordCards[i]);
	}

	//地主牌
	for (size_t i = 0; i < max(3 - vecLordCards.size(), 0); i++)
	{
		m_cLordCard.push_back(GetCard());
	}

	for (vector<CCard>::reverse_iterator iter = vecAllCards.rbegin(); iter != vecAllCards.rend(); iter++)
	{
		m_AllCards.push_back(*iter);
	}

	//laizi
	m_cLaiziCard.m_nValue = -1;
	if (g_nBaoPai)
	{
		SortLordCards();
		m_cLaiziCard.m_nValue = SetLaiziValue(m_cLordCard[1]);
	}
	return true;
}
void CPoke::ShuffleCards(int send_card_type, int send_card_num, int n_double)
{
	g_pLogger->Log("---------------specific ShuffleCards begin------------");

	if (ShuffleCardsFile(send_card_type,send_card_num,n_double) == false)
	{
		// if(ShuffleCardsDB(send_card_type,send_card_num,n_double) == false)
		{
			ShuffleDefaultCards();
		}
	}

	/*
	static int rand_key = 0; 
	char szBuffer[512];
	FILE* fp = fopen("cards.csv", "rt");
	if(fp == NULL)
	{
		CppSQLite3DB*	db_;
		db_ = CConfigManager::GetInstancePtr()->db_sqlite();

		if(db_)	//加载数据库失败
		{
			boost::asio::detail::mutex::scoped_lock lock(CConfigManager::GetInstancePtr()->db_mutex());
			string str_table ;
			str_table =  CConfigManager::GetInstancePtr()->get_current_table_query(send_card_type,n_double);
			string player_[3];
			string lord_card;
			CppSQLite3Table query = db_->getTable(str_table.c_str());
	        g_pLogger->Log("-------------output: %s",str_table.c_str());
			if(query.numRows()>0) //查询数据不为空
			{
	      	g_pLogger->Log("-------------output:%d",query.numRows());
				srand(time(0)+(rand_key++));
				int tempRow = rand()%query.numRows();
				query.setRow(tempRow);//在每一次需要获取数据的时候都要设置要访问的行值 
				g_pLogger->Log("------------tempRow = %d",tempRow);
			//	glog.log("tempRow = %d", tempRow);
	//	glog.log("numFields = %d", query.numFields());
				player_[0] = query.fieldValue(1);
	//	glog.log("player_[0] = %s", player_[0]);
				player_[1] = query.fieldValue(2);
//		glog.log("player_[1] = %s", player_[1]);
			    player_[2] = query.fieldValue(3);
//		glog.log("player_[2] = %s", player_[2]);
				
//		glog.log("query.fieldValue(4) = %s", query.fieldValue(4));
				lord_card = query.fieldValue(4);

//		g_pLogger->Log("-------output:%d",player_[0]);

					vector<CCard> card_[3];
					char delims[] = ",";

					CCard card;
					char* result = NULL;

					for(size_t n=0;n<3;n++)
					{
						char* buf = new char[strlen(player_[n].c_str())+1];
						strcpy(buf,player_[n].c_str());
						result = strtok(buf,delims);
						while(result != NULL)
						{
							card.m_nValue = atoi(result)%16;
							card.m_nColor = atoi(result)/16;
							if(card.m_nValue == 0)
							{
								card.m_nValue = 16;
								card.m_nColor -=1;
							}
							//g_pLogger->Log("%d        %d",card.m_nValue,card.m_nColor);
							card_[n].push_back(card);
							result = strtok(NULL,delims);
						}
					}
				
					
					for(size_t n=0;n<3;n++)
					{
						for(size_t m=0;m<17;m++)
						{
							m_AllCards.push_back(card_[n][m]);
						}
					}
					
					char* buf = new char[strlen(lord_card.c_str())+1];
					strcpy(buf,lord_card.c_str());
					result = strtok(buf,delims);
					while(result != NULL)
					{
						card.m_nValue = atoi(result)%16;
						card.m_nColor = atoi(result)/16;
						if(card.m_nValue == 0){
							card.m_nValue = 16;
							card.m_nColor -=1;
						}
						//g_pLogger->Log("%d  %d",card.m_nValue,card.m_nColor);
						m_cLordCard.push_back(card);
						result = strtok(NULL,delims);
					}
			}
			else
				ShuffleDefaultCards();
		}
		else
		{
			ShuffleDefaultCards();
		}
	}
	else
	{
		while(fgets(szBuffer, 512,  fp))
		{
			if(strchr(szBuffer, '#') != NULL) //包涵#字符行为注释行
				continue;
			vector<string> vecRet;
			int nItem = ParseLine(szBuffer, vecRet, ',');
			if(nItem >= 3)
			{
				for(int i = 0; i < nItem; ++i)
				{
					char cCard = atoi(vecRet[i].c_str());
					if(cCard == 0)
						continue;
					CCard Card;
					if( cCard == 64 )
					{
						Card.m_nColor = 0;
						Card.m_nValue = 16;
					}
					else if( cCard == 65 )
					{
						Card.m_nColor = 1;
						Card.m_nValue = 16;
					}
					else
					{
						Card.m_nColor = cCard / 16;
						Card.m_nValue = cCard % 16;
					}
					m_AllCards.push_back(Card);
				}
			}
		}

		fp = fopen("lord_cards.csv", "rt");
		while(fgets(szBuffer, 512,  fp))
		{
			if(strchr(szBuffer, '#') != NULL) //包涵#字符行为注释行
				continue;
			vector<string> vecRet;
			int nItem = ParseLine(szBuffer, vecRet, ',');
			for(int i = 0; i < nItem; ++i)
			{
				char cCard = atoi(vecRet[i].c_str());
				if(cCard == 0)
					continue;
				CCard Card;
				if( cCard == 64 )
				{
					Card.m_nColor = 0;
					Card.m_nValue = 16;
				}
				else if( cCard == 65 )
				{
					Card.m_nColor = 1;
					Card.m_nValue = 16;
				}
				else
				{
					Card.m_nColor = cCard / 16;
					Card.m_nValue = cCard % 16;
				}
				m_cLordCard.push_back(Card);
			}
		}
	}

	//laizi
	m_cLaiziCard.m_nValue = -1;
	if (g_nBaoPai)
	{
		SortLordCards();
		m_cLaiziCard.m_nValue = SetLaiziValue(m_cLordCard[1]);
	}
	*/
	g_pLogger->Log("---------------specific ShuffleCards ends------------");
}

void CPoke::ShuffleCards()
{
	g_pLogger->Log("---------------PT ShuffleCards begin------------");

	if (ShuffleCardsFile(0,0,0) == false)
	{
		ShuffleDefaultCards();
	}
	/*
	char szBuffer[512];
	FILE* fp = fopen("cards.csv", "rt");
	if(fp == NULL)
	{
		//3, 4, 5, .....K, A, 2, 
		for(int i = 3; i <= 15; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				CCard Card;
				Card.m_nColor = j;
				Card.m_nValue = i;
				m_AllCards.push_back(Card);
			}
		}
		//小王,大王
		for(int i = 0; i <= 1; i++)
		{
			CCard Card;
			Card.m_nColor = i;
			Card.m_nValue = 16;
			m_AllCards.push_back(Card);
		}

		//洗牌
		random_shuffle(m_AllCards.begin(), m_AllCards.end());

		if(g_nSendCardNum>0)
		{

		}
		else
		{
			if(g_nZhaDanDef>0 && g_nZhaDanDef<11)  //炸弹几率
			{
				int n = rand()%10;
				if(n <= (g_nZhaDanDef-1))  //特殊流程
				{
					SetZhaDanCard();
				}
			}
		}
	}
	else
	{
		while(fgets(szBuffer, 512,  fp))
		{
			if(strchr(szBuffer, '#') != NULL) //包涵#字符行为注释行
				continue;
			vector<string> vecRet;
			int nItem = ParseLine(szBuffer, vecRet, ',');
			if(nItem >= 3)
			{
				for(int i = 0; i < nItem; ++i)
				{
					char cCard = atoi(vecRet[i].c_str());
					if(cCard == 0)
						continue;
					CCard Card;
					if( cCard == 64 )
					{
						Card.m_nColor = 0;
						Card.m_nValue = 16;
					}
					else if( cCard == 65 )
					{
						Card.m_nColor = 1;
						Card.m_nValue = 16;
					}
					else
					{
						Card.m_nColor = cCard / 16;
						Card.m_nValue = cCard % 16;
					}
					m_AllCards.push_back(Card);
				}
			}
		}
	}
	//设置3张底牌
	fp = fopen("lord_cards.csv", "rt");
	if(fp == NULL)
	{
		for(int i = 0; i < 3; i++)
		{
			m_cLordCard.push_back(GetCard());
		}
	}
	else
	{
		while(fgets(szBuffer, 512,  fp))
		{
			if(strchr(szBuffer, '#') != NULL) //包涵#字符行为注释行
				continue;
			vector<string> vecRet;
			int nItem = ParseLine(szBuffer, vecRet, ',');
			for(int i = 0; i < nItem; ++i)
			{
				char cCard = atoi(vecRet[i].c_str());
				if(cCard == 0)
					continue;
				CCard Card;
				if( cCard == 64 )
				{
					Card.m_nColor = 0;
					Card.m_nValue = 16;
				}
				else if( cCard == 65 )
				{
					Card.m_nColor = 1;
					Card.m_nValue = 16;
				}
				else
				{
					Card.m_nColor = cCard / 16;
					Card.m_nValue = cCard % 16;
				}
				m_cLordCard.push_back(Card);
			}
		}
	}

	//laizi
	m_cLaiziCard.m_nValue = -1;
	if (g_nBaoPai)
	{
		SortLordCards();
		m_cLaiziCard.m_nValue = SetLaiziValue(m_cLordCard[1]);
	}
	*/
	g_pLogger->Log("---------------PT ShuffleCards begin------------");
}

CCard CPoke::ConverCard(int card)
{
	CCard cCard;

	if(card > 51){
		cCard.m_nColor = card%2;
		cCard.m_nValue = 16;
	}else{
		cCard.m_nColor = card/13;
		cCard.m_nValue = card%13+3;				
	}

	cCard.m_nCard_Baovalue = -1;				

	return cCard;
}

int CPoke::checkCardIsOk(int* card)
{
	int ret = 0;
	if (NULL != card)
	{
		int card_used[54]= {0};
		{
			int i=0,j=0;
			for (j=0; j<54; j++)
			{	
				if (-1 == card[j])
				{				
					continue;
				}		
				if (g_nLetCard == 1 && g_nTwoPai == 0)
				{
					if (j % 17 >= 9)
					{
						continue;
					}
				}
				if (card_used[ card[j] ] == 1)
				{
					ret = 1;
					if(card[j]<52){
						i = card[j] % 13;
					}else if(card[j]<54){
						i = card[j] - 39;
					}
					g_pLogger->Log("checkCardIsOk:excess-%d(%d)",i+3,card[j]); 	
					continue;
				}
				
				card_used[ card[j] ] = 1;
			}
		}
		
		{
			int /*k=0,*/i=0;
			for(int k=0; k<54; k++)
			{
				if (g_nLetCard == 1 && g_nTwoPai == 0)
				{
					if (k < 52 && k % 13 < 6)
					{
						continue;
					}
				}
				if(card_used[k]==0)
				{
					ret = 1;
					if(k<52){
						i = k % 13;
					}else if(k<54){
						i = k - 39;
					}
					g_pLogger->Log("checkCardIsOk:miss-%d(%d)",i+3,k); 	
				}
			}
		}		
		return ret;
	}
	return -2;
}

int CPoke::checkCardHas34(int* card)
{
	if (NULL == card)
	{
		SERVER_LOG("NULL == card");
		return -2;
	}
	for (int i = 0; i < 54; i++)
	{
		if (i >= 34 && i < 51)
		{
			continue;
		}
		CCard pCard = ConverCard(card[i]);
		if ((int)pCard.m_nValue < 5)
		{
			SERVER_LOG("i:%d pCard.m_nValue:%d", i, (int)pCard.m_nValue);
			return -1;
		}
	}
	return 0;
}

int CPoke::checkLowCardIsOk(int* card)
{
	if (NULL == card)
	{
		SERVER_LOG("NULL == card");
		return -2;
	}
	for (int i = 0; i < 54; i++)
	{
		if (i % 17 >= 9)
		{
			continue;
		}
		CCard pCard = ConverCard(card[i]);
		if ((int)pCard.m_nValue < 9)
		{
			SERVER_LOG("i:%d pCard.m_nValue:%d", i, (int)pCard.m_nValue);
			return -1;
		}
	}
	return 0;
}

void CPoke::NewShuffleCardsForNewBie()
{
	genPackOfCards(m_AllCards);

	// SetZhaDanCard(m_BetterSeat);
	vector<CCard> temp_hand_card[3];


	random_shuffle(m_AllCards.begin(), m_AllCards.end());

	//Joker 1 or 2
	int number_joker = 2;//rand()%2 + 1;

	if (number_joker > 0) {
		for(size_t i=0; i < m_AllCards.size(); i++) {
			if (m_AllCards[i].m_nValue == 16) {
				temp_hand_card[m_BetterSeat].push_back(m_AllCards[i]);
				m_AllCards[i].m_nValue = -1;
				number_joker--;
				if (number_joker == 0)
				{
					break;
				}
			}
		}
	}

	int number_2 = 1;
	if (number_2 > 0) {
		for(size_t i=0; i < m_AllCards.size(); i++) {
			if (m_AllCards[i].m_nValue == 15) {
				temp_hand_card[m_BetterSeat].push_back(m_AllCards[i]);
				m_AllCards[i].m_nValue = -1;
				number_2--;
				if (number_2 == 0)
				{
					break;
				}
			}
		}
	}


	seriesCardsTemp seriesTemp;
	seriesTemp.cards_size = 2;
	seriesTemp.cards_start = 11;
	seriesTemp.cards_end = 14;
	seriesTemp.cards_type = SERIES_BOMB;
	genSeriesCards(m_AllCards, temp_hand_card[m_BetterSeat], seriesTemp);


	int typeIn1 = rand()%3;
	
	if (typeIn1 == 0) {
		//3-10飞机3个
		seriesCardsTemp seriesTemp;
		seriesTemp.cards_size = 2;
		seriesTemp.cards_start = 3;
		seriesTemp.cards_end = 10;
		seriesTemp.cards_type = SERIES_FEIJI;
		genSeriesCards(m_AllCards, temp_hand_card[m_BetterSeat], seriesTemp);

	}else if(typeIn1 == 1) {
		//3-10连对4个
		seriesCardsTemp seriesTemp;
		seriesTemp.cards_size = 3;
		seriesTemp.cards_start = 3;
		seriesTemp.cards_end = 10;
		seriesTemp.cards_type = SERIES_LIANDUI;
		genSeriesCards(m_AllCards, temp_hand_card[m_BetterSeat], seriesTemp);

	}else if(typeIn1 == 2) {
		//3-10顺子8张
		seriesCardsTemp seriesTemp;
		seriesTemp.cards_size = 6;
		seriesTemp.cards_start = 3;
		seriesTemp.cards_end = 10;
		seriesTemp.cards_type = SERIES_SHUNZI;
		genSeriesCards(m_AllCards, temp_hand_card[m_BetterSeat], seriesTemp);
	}

	for(size_t i=0;i<m_AllCards.size();i++)
	{
		if(m_AllCards[i].m_nValue != -1)
		{
			m_cLordCard.push_back(m_AllCards[i]);
			m_AllCards[i].m_nValue = -1;
			if (m_cLordCard.size() >= 3)
			{
				break;
			}
		}
	}	

	for(size_t i=0;i<m_AllCards.size();i++)
	{
		if(m_AllCards[i].m_nValue != -1)
		{
			if(temp_hand_card[i%3].size()>=17)
			{
				if(temp_hand_card[(i+1)%3].size()>=17)
					temp_hand_card[(i+2)%3].push_back(m_AllCards[i]);
				else
					temp_hand_card[(i+1)%3].push_back(m_AllCards[i]);
			}
			else
				temp_hand_card[i%3].push_back(m_AllCards[i]);
		}
	}

	//push back
	m_AllCards.clear();
	for(int i=0;i<3;i++)
	{
		random_shuffle(temp_hand_card[i].begin(), temp_hand_card[i].end());
	}

	for(size_t j=0;j<3;j++)
	{
		for(size_t i=0;i<17;i++)
		{
			m_AllCards.push_back(temp_hand_card[j].back());
			temp_hand_card[j].pop_back();
		}
	}
	if (g_nCleverRobot == 1)
	{
		return;
	}
	std::reverse(m_AllCards.begin(), m_AllCards.end());	
}


bool CPoke::ShuffleCardsForNewBieFile()
{
	FILE* fp = fopen("newbie_cards.csv", "rt");
	if (NULL == fp)
	{
		return false;
	}

	// 默认洗牌
	ShuffleDefaultCards();

	// 将地主牌放到牌堆中
	vector<CCard> vecAllCards;
	for (size_t i = 0; i < m_cLordCard.size(); i++)
	{
		m_AllCards.push_back(m_cLordCard[i]);
	}
	m_cLordCard.clear();

	vector<string> vecBuffer;
	char buffer[512];
	while (fgets(buffer, 512, fp))
	{
		//包涵#字符行为注释行
		if (strchr(buffer, '#') != NULL) {
			continue;
		}
		vecBuffer.push_back(buffer);
	}
	fclose(fp);

	vector<string> vecCardPoint;
	int nId = 0;
	int nCount = 10;
	do 
	{
		nId = rand() % vecBuffer.size();
		if (std::find(m_vecNewbieCardsId.begin(), m_vecNewbieCardsId.end(), nId) == m_vecNewbieCardsId.end())
		{
			break;
		}
	} while (--nCount > 0);
	ParseLine(vecBuffer[nId], vecCardPoint, ',');
	m_vecNewbieCardsId.push_back(nId);
	for (size_t i = 0; i < vecCardPoint.size(); ++i)
	{
		int nCard = atoi(vecCardPoint[i].c_str());
		CCard Card;
		Card.m_nColor = nCard % 10;
		Card.m_nValue = nCard / 10;

		if (Card.m_nColor < 0 || Card.m_nValue < 0 || (Card.m_nColor > 3 && Card.m_nValue > 15) || (Card.m_nColor > 1 && Card.m_nValue == 16))
		{
			continue;
		}

		for (size_t i = 0; i < m_AllCards.size(); i++)
		{
			if (m_AllCards[i] == Card)
			{
				vecAllCards.push_back(Card);
				m_AllCards.erase(m_AllCards.begin() + i);
				break;
			}
		}
	}

	//设置3张底牌
	fp = fopen("lord_cards.csv", "rt");
	vector<CCard> vecLordCards;
	if (fp != NULL)
	{
		while (fgets(buffer, 512, fp))
		{
			//包涵#字符行为注释行
			if (strchr(buffer, '#') != NULL) {
				continue;
			}

			vecCardPoint.clear();
			ParseLine(buffer, vecCardPoint, ',');
			for (size_t i = 0; i < vecCardPoint.size(); ++i)
			{
				int nCard = atoi(vecCardPoint[i].c_str());
				CCard Card;
				Card.m_nColor = nCard % 10;
				Card.m_nValue = nCard / 10;

				if (Card.m_nColor < 0 || Card.m_nValue < 0 || (Card.m_nColor > 3 && Card.m_nValue > 15) || (Card.m_nColor > 1 && Card.m_nValue == 16))
				{
					continue;
				}

				for (size_t i = 0; i < m_AllCards.size(); i++)
				{
					if (m_AllCards[i] == Card)
					{
						vecLordCards.push_back(Card);
						m_AllCards.erase(m_AllCards.begin() + i);
						break;
					}
				}
			}
		}
		fclose(fp);
	}

	for (size_t i = 0; i < min(vecLordCards.size(), 3); i++)
	{
		m_cLordCard.push_back(vecLordCards[i]);
	}

	//地主牌
	for (size_t i = 0; i < max(3 - vecLordCards.size(), 0); i++)
	{
		m_cLordCard.push_back(GetCard());
	}

	for (vector<CCard>::reverse_iterator iter = vecAllCards.rbegin(); iter != vecAllCards.rend(); iter++)
	{
		m_AllCards.push_back(*iter);
	}

	if (m_BetterSeat != 0)
	{
		vector<CCard>::reverse_iterator iter = vecAllCards.rbegin();
		for (int i = 0; i < g_nDealCardNum; i++)
		{
			std::iter_swap(iter + i, iter + (m_BetterSeat * 17 + i));
		}
	}

	//laizi
	m_cLaiziCard.m_nValue = -1;
	if (g_nBaoPai)
	{
		SortLordCards();
		m_cLaiziCard.m_nValue = SetLaiziValue(m_cLordCard[1]);
	}
	return true;
}

void CPoke::genSeriesCards(vector<CCard>& srcCards, vector<CCard>& dstCards, seriesCardsTemp SeriesTemp)
{
	int series_size = SeriesTemp.cards_size;
	int series_start = SeriesTemp.cards_start;
	int series_end = SeriesTemp.cards_end;
	int series_type = SeriesTemp.cards_type;

	vector<int> series_value;

	for (int i = series_start; i <= series_end; ++i)
	{
		series_value.push_back(i);
	}
	while (series_value.size() > series_size)
	{
		if (rand()%2 == 1)
		{
			std::reverse(series_value.begin(), series_value.end()); 
		}
		series_value.pop_back();
	}       

	for(size_t j=0; j < series_value.size(); j++) {        
		int cardCount = 0;
		for(size_t i=0; i < srcCards.size(); i++) {
			if (srcCards[i].m_nValue == series_value[j])
			{
				dstCards.push_back(srcCards[i]);
				cardCount++;
				srcCards[i].m_nValue = -1;
				if (cardCount == series_type)
				{
					break;
				}
			}
		}
	}
}


void CPoke::genPackOfCards(vector<CCard>& cards)
{
	cards.clear();
	for (int cValue = 3; cValue < 16; ++cValue)
	{
		for (int cColor = 0; cColor < 4; ++cColor)
		{
			cards.push_back(CCard(cColor, cValue));
		}
	}

	cards.push_back(CCard(0, 16));
	cards.push_back(CCard(1, 16));

}

void CPoke::NewShuffleCards()
{
	Request res;
	int laizi=-1,k=0;
	int card_list[54];
	double d_bomb_multiple = g_nBombMultiple > 10 ? (g_nBombMultiple / 10.f) : g_nBombMultiple;
	bool ret = GetRandomParams(&res, d_bomb_multiple, g_nBetterSeat, g_nCardTidiness, g_nBaseGood);
	if( ret == true )
	{
		res.call_lord_first = m_nCurrentLord; //???
		if (m_BetterSeat >=0 && m_BetterSeat <= 2)
		{
			if (m_robotSeat == m_BetterSeat)
			{
				res.better_seat = m_BetterSeat;
				res.call_lord_first = m_robotSeat;
			}
			else if (res.better_seat < 3)
			{
				res.better_seat = m_BetterSeat;
			}
			//res.better_seat = m_BetterSeat;
			//if (m_robotSeat == m_BetterSeat)
			//{
			//	res.call_lord_first = m_robotSeat;
			//}
		}
		SERVER_LOG("better_seat:%d,bomb_multiple:%d,call_lord_first:%d,base_good:%d", res.better_seat, res.bomb_multiple, res.call_lord_first, res.base_good);
		int inRuleType = 0;
		if (g_nBaoPai != 1 || g_nTwoPai != 1)
		{
			if (g_nBaoPai == 1)
			{
				inRuleType = 1;
			}
			if (g_nTwoPai == 1)
			{
				inRuleType = 2;
			}
			else if (g_nLetCard == 1)
			{
				inRuleType = 3;
			}
		}

		if (g_nBuXiPai > 0)
		{
			res.bomb_multiple = -2;
		}
		ret = InitCardRandom(res, inRuleType, card_list, &laizi);
		SERVER_LOG("laizi:%d,robotSeat:%d", laizi, m_robotSeat);
	}
	
	if (ret == false)
	{
		SERVER_LOG("---------------NewShuffleCards ERROR------------");
		ShuffleDefaultCards();
		return;
	}
	if(checkCardIsOk(card_list))
	{
		SERVER_LOG("---------------checkCardIsOk ERROR------------");
		ShuffleDefaultCards();
		return;
	}

	if (g_nTwoPai == 1 && checkCardHas34(card_list))
	{
		SERVER_LOG("---------------checkCardHas34 ERROR------------");
		ShuffleDefaultCards();
		return;
	}
	else if (g_nLetCard == 1 && g_nTwoPai == 0 && checkLowCardIsOk(card_list))
	{
		SERVER_LOG("---------------checkLowCardIsOk ERROR------------");
		ShuffleDefaultCards();
		return;
	}
	
	SERVER_LOG("---------------CPoke::NewShuffleCards()---------------");
	if (g_nTwoPai == 1)
	{
		for (int i = 3; i > 0; i--)
		{
			for (int j = 0; j < 17; j++)
			{
				m_AllCards.push_back(ConverCard(card_list[i * 17 + j - 17]));
				SERVER_LOG("m_AllCards %d--%d", j, m_AllCards[i * 17 + j - 17].m_nValue);
			}
		}
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
		//SERVER_LOG("---------------CPoke::NewShuffleCards()---------------");
			for (int j = 0; j < 17; j++)
			{
				if (g_nLetCard == 1 && j % 17 >= 9)
				{
					continue;
				}
				m_AllCards.push_back(ConverCard(card_list[i * 17 + j]));
			//SERVER_LOG("%d--%d",m_AllCards[k-1].m_nValue,k-1);
			}
		}
	}

	//地主牌
	for (int i = 0; i < 3; i++)
	{
		m_cLordCard.push_back(ConverCard(card_list[i + 51]));
		SERVER_LOG("m_cLordCard:%d--%d", i, m_cLordCard[i].m_nValue);
	}
	//
	m_cLaiziCard.m_nValue = -1;
	//m_cLaiziCard.m_nColor = 0;
	//m_cLaiziCard.m_nCard_Baovalue = -1;
	if (g_nBaoPai)
	{
		m_cLaiziCard = ConverCard(laizi);
	}
	//SERVER_LOG("laizi:%d",m_cLaiziCard.m_nValue);
}

void CPoke::SetZhaDanCard(int chairid)
{
	if(g_nZhaDanNum>0)
	{
		for(int i=0;i<g_nZhaDanNum;i++)
		{
			int value = GetZhaDanValue();
			m_zhadan.push_back(value);
		}

		if(g_nZhaDanNum == (int)m_zhadan.size())
		{
			//vector<CCard> m_t_all_card;
			vector<CCard> m_card_[3];
			for(size_t j=0;j<m_zhadan.size();j++)
			{
				int value = m_zhadan[j];
				int zhadan_pos = rand()%3;
				if (chairid < 0) {
					zhadan_pos = rand()%3;
				}else{
					zhadan_pos = chairid;
				}
				for(size_t i=0;i<m_AllCards.size();i++)
				{
					if(m_AllCards[i].m_nValue == value)
					{
						m_card_[zhadan_pos].push_back(m_AllCards[i]);
						m_AllCards[i].m_nValue = -1;
					}

				}
			}


			random_shuffle(m_AllCards.begin(), m_AllCards.end());
			int lordcardsize = 0;
			for(size_t i=0;i<m_AllCards.size();i++)
			{
				if(m_AllCards[i].m_nValue != -1) {
					m_cLordCard.push_back(m_AllCards[i]);
					m_AllCards[i].m_nValue = -1;
					lordcardsize++;
				}
				if (lordcardsize == 3)
				{
					break;
				}
			}

			for(size_t i=0;i<m_AllCards.size();i++)
			{
				if(m_AllCards[i].m_nValue != -1)
				{
					if(m_card_[i%3].size()>=17)
					{
						if(m_card_[(i+1)%3].size()>=17)
							m_card_[(i+2)%3].push_back(m_AllCards[i]);
						else
							m_card_[(i+1)%3].push_back(m_AllCards[i]);
					}
					else
						m_card_[i%3].push_back(m_AllCards[i]);
				}
			}

			m_AllCards.clear();
			for(int i=0;i<3;i++)
			{
				random_shuffle(m_card_[i].begin(), m_card_[i].end());
			}

			for(size_t j=0;j<3;j++)
			{
				for(size_t i=0;i<17;i++)
				{
					m_AllCards.push_back(m_card_[j].back());
					m_card_[j].pop_back();
				}
			}

			std::reverse(m_AllCards.begin(), m_AllCards.end());			
		}
	}

}

int CPoke::GetZhaDanValue()
{
	int zhadan_value = 0;
	
	while(zhadan_value == 0)
	{
		zhadan_value = rand()%14 +3;
		for(size_t i=0;i<m_zhadan.size();i++)
		{
			if(m_zhadan[i] == zhadan_value)
				zhadan_value = 0;
		}
	}
	return zhadan_value;
}

void CPoke::NewRound()
{
	//成员初始化
	m_bSpring = false;
	m_bReverseSpring = false;
	m_robotSeat = -1;
	m_BetterSeat = 2;
	m_nNextPlayer = 0;
	m_nBombCounter = 0;
	m_nDecideLordRound = 0;
	memset(m_bSendCardOK, false, sizeof(m_bSendCardOK));
	m_AllCards.clear();
	m_cLordCard.clear();
	m_zhadan.clear();
	
	//设定地主
	SetDefaultLord();
	//洗牌
	if (ShuffleCardsFile(0,0,0) == false)
	{
		if (g_nNewShuffleCards == 1)
		{
			NewShuffleCards();
		}
		else
		{
			ShuffleDefaultCards();
		}
	}	
}

void CPoke::NewRound(int send_card_type,int send_card_num,int n_double)
{
	//成员初始化
	m_bSpring = false;
	m_bReverseSpring = false;
	m_robotSeat = 0;
	m_BetterSeat = 0;
	m_nNextPlayer = 0;
	m_nBombCounter = 0;
	m_nDecideLordRound = 0;
	memset(m_bSendCardOK, false, sizeof(m_bSendCardOK));
	m_AllCards.clear();
	m_cLordCard.clear();
	m_zhadan.clear();

	//设定地主
	SetDefaultLord();

	//洗牌&设置底牌
	if (ShuffleCardsFile(send_card_type, send_card_num, n_double) == false)
	{
		if (g_nNewShuffleCards == 1)
		{
			m_BetterSeat = send_card_type;
			m_robotSeat = n_double;
			if (g_nBetterSeatForNewBie == 1) {
				if (n_double == -10)
				{
					NewShuffleCardsForNewBie();
				}
				else if (n_double == -11) // ab测试 读取文件方式
				{
					if (!ShuffleCardsForNewBieFile())
					{
						NewShuffleCardsForNewBie();
					}
				}
				else
				{
					NewShuffleCards();
				}
			} else{
				NewShuffleCards();
			}
		}
		else
		{
			ShuffleDefaultCards();
		}
	}
	
}

void CPoke::SortLordCards()
{
	for(size_t i = 0; i < m_cLordCard.size(); i++)
	{
		for(size_t j = i+1; j < m_cLordCard.size(); j++)
		{
			int nIValue = m_cLordCard[i].m_nValue;			
			int nJValue = m_cLordCard[j].m_nValue;
			if( (nIValue * 4 + m_cLordCard[i].m_nColor) < (nJValue * 4 + m_cLordCard[j].m_nColor))
			{
				std::swap(m_cLordCard[i], m_cLordCard[j]);
			}
		}
	}
}

void CPoke::SetDefaultLord(int chairid /*= -1*/)
{
	//随机翻出的一张牌
	if (chairid > -1 && chairid < CConfigManager::GetInstancePtr()->GetPlayNum()) {
		m_nDefaultLord = chairid;
		m_nCurrentLord = chairid;
	}
	else {
		m_nRanCardPos = rand() % 54;//m_AllCards.size();
		m_nDefaultLord = m_nRanCardPos % CConfigManager::GetInstancePtr()->GetPlayNum();
		m_nCurrentLord = m_nDefaultLord;
	}

	//m_nRanCardPos = (int)m_AllCards.size() - m_nRanCardPos - 1;
	//m_cRanCard = m_AllCards[m_nRanCardPos];
}

CCard& CPoke::GetCard(int nPos)
{
	if( nPos >= 0 && nPos < (int)m_AllCards.size() )
	{
		return m_AllCards[nPos];
	}
	//g_pLogger->Log("CPoke::GetCard(%d) Error", nPos);
	static CCard cCard;
	return cCard;
}

CCard& CPoke::GetCard()
{
	if( !m_AllCards.empty() )
	{
		CCard& cCard = m_AllCards.back();
		m_AllCards.pop_back();
		return cCard;
	}
	//g_pLogger->Log("CPoke::GetCard() Is Empty");
	static CCard cCard;
	return cCard;
}

void CPoke::SetSendCardNum()
{
	//int send_card_num = g_nSendCardNum;
	//m_AllCards;
	vector<CCard> m_card_[3];
	for(size_t i=0;i<m_AllCards.size();i++)
	{
		for(size_t j=0;j<m_card_->size();j++)
		{
//			if(j<send_card_num)
		}
	}
}

int CPoke::GetLordCardType()
{
	//m_cLordCard;
	bool sameColor =false;
	bool sequence = false;
	if (m_cLordCard[1].m_nValue == m_cLordCard[2].m_nValue &&
		m_cLordCard[0].m_nValue == m_cLordCard[1].m_nValue)
		return 4; // 三条

	if (m_cLordCard[1].m_nValue == m_cLordCard[2].m_nValue ||
		m_cLordCard[0].m_nValue == m_cLordCard[1].m_nValue ||
		m_cLordCard[0].m_nValue == m_cLordCard[2].m_nValue)
		return 1; // 对子

	if (m_cLordCard[0].m_nColor == m_cLordCard[1].m_nColor &&
		m_cLordCard[1].m_nColor == m_cLordCard[2].m_nColor)
		sameColor = true;

	int card1 = m_cLordCard[0].m_nValue,card2 = m_cLordCard[1].m_nValue,card3 = m_cLordCard[2].m_nValue,tmp;
	if(card1 > card2)
	{
		tmp = card1;
		card1 = card2;
		card2 = tmp;
	}
	if (card1 > card3)
	{
		tmp = card1;
		card1 = card3;
		card3 = tmp;
	}
	if (card2 > card3)
	{
		tmp = card2;
		card2 = card3;
		card3 = tmp;
	}
	if(card1 == card2 - 1 &&
		card2 == card3 - 1)
		sequence = true;
	if (sameColor && sequence)
		return 5;
	if (sameColor)
		return 2;
	if (sequence)
		return 3;
	return 0;

}
