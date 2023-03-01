#include "stdafx.h"
#include "RobotManager.h"
#include "ConfigManager.h"

void RobotManager::pushRobot( Robot *robot )
{
	robot_quene_.PushEvent(robot);
}

Robot * RobotManager::popRobot()
{
	return robot_quene_.PopEvent();
}

int RobotManager::size()
{
	return robot_quene_.Size();
}

bool Robot::isSpecialRobot( guid pid )
{
	if(pid == CConfigManager::GetInstancePtr()->GetRobotAgent())
	{
		return true;
	}
	return false;
}

void Robot::parse_addtional_rule( const string& rule )
{
	if(rule.empty())
	{
		m_additional_rule_map_.clear();
		return;
	}

	// 服务器费
	string map_key,map_value;
	char c_rule[1024]={0};
	strcpy(c_rule,rule.c_str());
	const char *child_content=strtok(c_rule,"|");
	while(child_content)
	{
		string s = string(child_content);
		size_t pos = s.find_first_of(":");
		if(pos==string::npos)
			continue;

		map_key = s.substr(0,pos);
		map_value = s.substr(pos+1);

		m_additional_rule_map_.insert(pair< string,string >(map_key,map_value));

		child_content = strtok(NULL,"|");
	}
}

const char* Robot::get_rule_value( const string& key )
{
	for(map<string,string>::iterator it=m_additional_rule_map_.begin();it!=m_additional_rule_map_.end();it++)
	{
		if(it->first == key)
		{
			return it->second.c_str();
		}
	}
	return 0;
}

int Robot::checkCardIsOk()
{
	if (NULL != lord_robot_)
	{
		int ret =0;
		int card_used[54]= {0};
		int* card = NULL; 
		{
			int i,j,n;
			for (j=0; j<3; j++)
			{
				card = lord_robot_->card[j];
				for (i=0; i<17; i++)
				{
					if (-1 == card[i])
					{				
						continue;
					}		
					if (card_used[ card[i] ] == 1)
					{
						ret = 1;
						if(card[i]<52){
							n = card[i] % 13;
						}else if(card[i]<54){
							n = card[i] - 39;
						}
						g_pLogger->Log("checkCardIsOk:excess-%d(%d)",n+3,card[i]); 	
						continue;
					}
					
					card_used[ card[i] ] = 1;
				}
			}
			//pot card
			card = lord_robot_->pot;
			for (i=0; i<3; i++)
			{
				if (-1 == card[i])
				{				
					continue;
				}		
				if (card_used[ card[i] ] == 1)
				{
					ret = 1;
					if(card[i]<52){
						n = card[i] % 13;
					}else if(card[i]<54){
						n = card[i] - 39;
					}
					g_pLogger->Log("checkCardIsOk:excess-%d(%d)",n+3,card[i]); 	
					continue;
				}
				card_used[ card[i] ] = 1;
			}
		}

		{
			int k=0,n;
			for(int k=0; k<54; k++)
			{
				if(card_used[k]==0)
				{
					ret = 1;
					if(k<52){
						n = k % 13;
					}else if(k<54){
						n = k - 39;
					}
					g_pLogger->Log("checkCardIsOk:miss-%d(%d)",n+3,k); 	
				}
			}
		}		
		return ret;
	}
	return -2;
}

void Robot::converCardToRobot(vector<CCard> in, int* out, int* lz, int laizi)
{
	if (laizi ==-1)
	{
		int size = in.size();
		for (int i = 0; i < size;i++)
		{
			if (in[i].m_nValue == 16)
			{
				if(in[i].m_nColor == 0){
					out[i] = 52;
				}else{
					out[i] = 53;
				}
			}else{
				out[i] = (in[i].m_nValue - 3)+ 13*in[i].m_nColor;
			}
			//glog.log("%d",out[i]);
		}
		out[size] = -1;
		for (int i = size + 1; i <= 17; i++)
		{
			out[i] = -1;
		}
	}else{
		int size = in.size();
		int num = 0;

		laizi +=3;
		for (int i = 0; i < size;i++)
		{
			if (in[i].m_nValue == 16)
			{
				if(in[i].m_nColor == 0){
					out[i] = 52;
				}else{
					out[i] = 53;
				}
			}else {				
				out[i] = (in[i].m_nValue - 3)+ 13*in[i].m_nColor;
				if (in[i].m_nValue == laizi && in[i].m_nCard_Baovalue >= 3 )
				{
					lz[num++] = in[i].m_nCard_Baovalue - 3;
				}
			}
		}
		out[size] = -1;
		lz[num] = -1;
	}
}

vector<CCard> Robot::converCardToServer(int* card)
{
	CCard vecCard;
	vector<CCard> vecCards;

	for (int i = 0; card[i] != -1;i++)
	{
		if(card[i] > 51){
			vecCard.m_nColor = card[i]%2;
			vecCard.m_nValue = 16;
		}else{
			vecCard.m_nColor = card[i]/13;
			vecCard.m_nValue = card[i]%13+3;				
		}

		vecCard.m_nCard_Baovalue = -1;				
		vecCards.push_back(vecCard);
	}
	return vecCards;
}

vector<CCard> Robot::converCardToServer(int* card, int len)
{
	CCard vecCard;
	vector<CCard> vecCards;
	for (int i = 0; i < len;i++)
	{
		if (card[i] != -1)
		{					
			if(card[i] > 51){
				vecCard.m_nColor = card[i]%2;
				vecCard.m_nValue = 16;
			}else{
				vecCard.m_nColor = card[i]/13;
				vecCard.m_nValue = card[i]%13+3;
			}
			vecCard.m_nCard_Baovalue = -1;
			vecCards.push_back(vecCard);
		}
	}
	return vecCards;
}

vector<CCard> Robot::converCardToServer(int* card, int* lz, int laizi)
{
	if (laizi != -1)
	{
		CCard vecCard;
		vector<CCard> vecCards;
		int num = 0;

		laizi += 3;

		for (int i = 0; card[i] != -1;i++)
		{
			if(card[i] > 51){
				vecCard.m_nColor = card[i]%2;
				vecCard.m_nValue = 16;
			}else{
				vecCard.m_nColor = card[i]/13;
				vecCard.m_nValue = card[i]%13+3;				
			}
			if(vecCard.m_nValue == laizi)
			{
				vecCard.m_nCard_Baovalue = lz[num++]%13+3;
			}else{
				vecCard.m_nCard_Baovalue = -1;
			}
			vecCards.push_back(vecCard);
		}
		return vecCards;
	}else{
		return converCardToServer(card);
	}

}

/*
void Robot::SortCards(vector<CCard>& cards, int laizi)
{
	if (laizi != -1)
	{
		laizi += 3;
		for(int i = 0; i < cards.size(); i++)
		{
			int nIValue = cards[i].m_nValue;
			if (nIValue == laizi)
			{
				nIValue =  cards[i].m_nCard_Baovalue;
				for(int j = 0; j < cards.size(); j++)
				{
					int nJValue = cards[j].m_nValue;
					if (nJValue == laizi)
					{
						nJValue = cards[j].m_nCard_Baovalue;
					}
					if( (nIValue * 4 + cards[i].m_nColor) < (nJValue * 4 + cards[j].m_nColor))
					{
						std::swap(cards[i], cards[j]);
					}
				}
			}


		}
	}
}*/
/*
void Robot::SortCards(vector<CCard>& cards, int laizi)
{
	if (laizi != -1)
	{
		laizi += 3;
		for(int i = 0; i < cards.size(); i++)
		{
			for(int j = i+1; j < cards.size(); j++)
			{
				int nIValue = cards[i].m_nValue;
				int nJValue = cards[j].m_nValue;
				if (nIValue == laizi)
				{
					nIValue =  cards[i].m_nCard_Baovalue;
				}
				if (nJValue == laizi)
				{
					nJValue = cards[j].m_nCard_Baovalue;
				}
				if( (nIValue * 4 + cards[i].m_nColor) < (nJValue * 4 + cards[j].m_nColor))
				{
					std::swap(cards[i], cards[j]);
				}
			}
		}
	}
}*/