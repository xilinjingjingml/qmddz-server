#include "stdafx.h"
#include "Task.h"
#include "GameTable.h"
#include "ConfigManager.h"
#include "Player.h"

CTask::CTask()
{
//	Init("task.xml");
	InitTaskCSV("task.csv");
}

CTask::~CTask()
{
}

bool CTask::InitTaskCSV( const char* m_szConfigFile )
{
// 	ExTable task_data;
// 	if(!task_data.InitFromFile(m_szConfigFile)){
// 		glog.error("load task.csv faield");
// 		return false;
// 	}
// 	for(int i = 0; i < task_data.GetRecordNum(); ++i){
// 		TaskItem task_item ;
// 		task_item.task_id_ = task_data.GetIntField(i,0);
// 		task_item.task_desc_ = task_data.GetStrField(i,1);
// 		task_item.task_mission_ = task_data.GetStrField(i,2);
// 		task_item.task_money_type_ = task_data.GetIntField(i,3);
// 		task_item.task_money_ = task_data.GetIntField(i,4);
// 		task_item.task_rate_ = task_data.GetIntField(i,5);
// 		vecTask_.push_back(task_item);
// 	}
	if(CConfigManager::GetInstancePtr()->vecTask_.size()>0)
		vecTask_ = CConfigManager::GetInstancePtr()->vecTask_;
	current_task_id_ = -1;
	return true ;
}

bool CTask::Init(const char* m_szConfigFile)
{
	TiXmlDocument doc(m_szConfigFile);
	if(!doc.LoadFile())
	{
		return false;
	}
	TiXmlElement* pElement = NULL;
	TiXmlElement* pSub = NULL;
	pElement = doc.FirstChildElement("GameTask");
	if(!pElement)
		return false;
	pSub = pElement->FirstChildElement("TaskItem");
	if(!pSub) 
		return false ;
	for(;pSub;pSub = pSub->NextSiblingElement())
	{
		TaskItem task_item ;
		pSub->Attribute("task_id",&task_item.task_id_);
		char buffer[1024];
		strncpy(buffer,pSub->Attribute("task_desc"),sizeof(buffer));
		task_item.task_desc_ = buffer;
		strncpy(buffer,pSub->Attribute("task_mission"),sizeof(buffer));
		task_item.task_mission_ = buffer;
		pSub->Attribute("task_money_type",&task_item.task_money_type_);
		pSub->Attribute("task_money_num",&task_item.task_money_);
		pSub->Attribute("rate",&task_item.task_rate_);
		vecTask_.push_back(task_item);
	}
	current_task_id_ = -1;
	return true ;
}

void CTask::GetTask(TaskItem& task_item)
{
	current_task_id_ = -1;
	int nIndex = -1 ;
	if(g_nTID == -1)
	{
		//nIndex = rand() % (vecTask_.size());
		int n_rand = rand()%100;
		int n_temp = 0 ;
		for(size_t i=0;i<vecTask_.size();i++)
		{
			n_temp += vecTask_[i].task_rate_;
			if(n_temp>n_rand)
			{
				nIndex = i;
				break;
			}
		}
	}
	else
	{
		if((g_nTID-vecTask_.size())>=0)
			nIndex = vecTask_.size()-1;
		else
			nIndex = g_nTID;
//		(g_nTID>=vecTask_.size()) ? (nIndex=(vecTask_.size()-1)) : (nIndex = g_nTID);
	}

	if(vecTask_.size()>0)
	{
		current_task_id_ = nIndex;
		task_item = vecTask_[nIndex];
	}

}

TaskItem CTask::GetTask( int task_id_ )
{
	TaskItem task_item;
	//if(task_id_<vecTask_.size())
	if((vecTask_.size()-task_id_)>0)	
		task_item = vecTask_[task_id_];
	current_task_id_ = task_id_;
	return task_item;
}

double CTask::GetRandNum( int nBase , double nCount )
{
	return (rand() % nBase) / nCount;
}

bool CTask::CheckTaskComplete( CPlayer* pPlayer,int nScore )
{
	switch( current_task_id_ ) 
	{
	case 0:								//打出王并获胜 奖励游戏币40个
		return CheckHandCard(pPlayer,16,1,nScore);
	case 1:								//打出火箭 奖励游戏币50个
		return CheckRocket(pPlayer);
	case 2:								//打出火箭并获胜 奖励游戏币60个
		return CheckRocketWin(pPlayer,nScore);
	case 3:								//打出炸弹 奖励游戏币80个
		return CheckBomb(pPlayer);
	case 4:								//打出炸弹并获胜 奖励游戏币90个
		return CheckBombWin(pPlayer,nScore);
	case 5:								//打出4连对及以上并获胜 奖励游戏币120个
		return CheckDoubleFour(pPlayer,nScore);			
	case 6:								//打出J以上炸弹 奖励游戏币100个
		return CheckBombJ(pPlayer);
	case 7:								//打出Q炸弹并获胜 奖励游戏币110个
		return CheckBombQ(pPlayer,nScore);
	case 8:								//底牌抓2 奖励游戏币20个
		return CheckLordCard(pPlayer);
	case 9:							//底牌抓大王或小王 奖励游戏币50个
		return CheckLordCardW(pPlayer);
	case 10:							//底牌抓大王并获胜 奖励游戏币60个
		return CheckLordCardWinW(pPlayer,nScore);
	case 11:							//底牌抓两张相同牌 奖励游戏币20个
		return CheckLordCardTwo(pPlayer);
	case 12:							//底牌抓到火箭并获胜 奖励游戏币120个
		return CheckLordCardRocket(pPlayer,nScore);
	case 13:							//底牌抓两张2 奖励游戏币90个
		return CheckLordCardTT(pPlayer);
	case 14:							//底牌抓顺子 奖励游戏币100个
		return CheckLordCardShunZi(pPlayer);
	case 15:							//一次打出12张单顺
		return CheckDingTianShun(pPlayer);
	case 16:							//倍数在96倍以上获胜
		return Check96Times(pPlayer,nScore);
	case 17:							//打出顺子并获胜
		return CheckShunZi(pPlayer,nScore);
	case 18:							//打出连对并获胜
		return CheckLianDui(pPlayer,nScore);
	case 19:							//打出春天牌局
		return CheckSpring(pPlayer,nScore);
	case 20:							//手牌抓4张癞子
		return Check4LaiZi(pPlayer,nScore);
	case 21:							//手牌抓2张癞子并获胜
		return Check2LaiZiWin(pPlayer,nScore);
	case 22:							//手牌抓2张癞子牌2并获胜
		return Check2LaiZi2Win(pPlayer,nScore);
	case 23:							//打出纯癞子炸弹并获胜
		return Check4LaiZiWin(pPlayer,nScore);
		break;
	case 24:							//打出5软炸弹并获胜
		return Check5SoftBombWin(pPlayer,nScore);
	case 25:							//底牌抓癞子牌并获胜
		return CheckLordLaiZiWin(pPlayer,nScore);
	case 26:							//测试任务
		if(nScore>0)
			return true ;
		break;
	default:
		return false;
	}
	return false ;
}

//打出王并获胜 奖励游戏币40个
bool CTask::CheckHandCard( CPlayer* pPlayer, int nCard, int nMaxCount, int nScore)
{
	if(nScore < 0)
		return false ;
	int nCount = 0 ;
	//int n_size = (int)pPlayer->m_vecHandCard.size();
	for(size_t t = 0 ; t < pPlayer->m_vecPutCard.size() ; t++)
	{
		if(pPlayer->m_vecPutCard[t].m_nTypeNum == 1 && pPlayer->m_vecPutCard[t].m_nTypeValue == 16)	//小王
			nCount++ ;
		if(pPlayer->m_vecPutCard[t].m_nTypeNum == 1 && pPlayer->m_vecPutCard[t].m_nTypeValue == 17)	//大王
			nCount++ ;
	}
	return (nCount++ >= nMaxCount);
}

//打出火箭 奖励游戏币50个
bool CTask::CheckRocket( CPlayer* pPlayer )
{
	for(size_t i = 0; i < pPlayer->m_vecPutCard.size(); i++)
	{
		if( pPlayer->m_vecPutCard[i].m_nTypeBomb == 2 && pPlayer->m_vecPutCard[i].m_nTypeValue == 16 )
			return true;
	}
	return false;
}

//打出火箭并获胜 奖励游戏币60个
bool CTask::CheckRocketWin( CPlayer* pPlayer,int nSCore )
{
	if(nSCore <= 0)
		return false;
	return CheckRocket(pPlayer);
}

//打出炸弹 奖励游戏币80个
bool CTask::CheckBomb( CPlayer* pPlayer )
{
	if(g_nBaoPai == 1)					//打出硬炸弹
	{
		for(size_t i = 0; i < pPlayer->m_vecPutCard.size(); i++)
		{
			if(pPlayer->m_vecPutCard[i].m_nTypeBomb == 2)
				return true;
		}
	}
	else
	{
		for(size_t i = 0; i < pPlayer->m_vecPutCard.size(); i++)
		{
			if( pPlayer->m_vecPutCard[i].m_nTypeBomb != 0 )
				return true;
		}
	}
	return false;
}

//打出炸弹并获胜 奖励游戏币90个
bool CTask::CheckBombWin( CPlayer* pPlayer,int nScore )
{
	if(nScore<=0)
		return false ;
	return CheckBomb(pPlayer);
}

//打出J以上炸弹 奖励游戏币100个
bool CTask::CheckBombJ( CPlayer* pPlayer )
{
	if(g_nBaoPai == 1)
	{
		for(size_t i=0;i<pPlayer->m_vecPutCard.size();i++)
		{
			if( pPlayer->m_vecPutCard[i].m_nTypeBomb == 2 && pPlayer->m_vecPutCard[i].m_nTypeValue >= 11)
				return true;
		}
	}
	else
	{
		for(size_t i = 0; i < pPlayer->m_vecPutCard.size(); i++)
		{
			if( pPlayer->m_vecPutCard[i].m_nTypeBomb != 0 && pPlayer->m_vecPutCard[i].m_nTypeValue >= 11)
				return true;
		}
	}
	return false;
}

//打出Q炸弹并获胜 奖励游戏币110个
bool CTask::CheckBombQ( CPlayer* pPlayer, int nScore)
{
	if(nScore<=0)
		return false ;
	if(g_nBaoPai == 1)
	{
		for(size_t i=0;i<pPlayer->m_vecPutCard.size();i++)
		{
			if( pPlayer->m_vecPutCard[i].m_nTypeBomb == 2 && pPlayer->m_vecPutCard[i].m_nTypeValue == 12)
				return true;
		}
	}
	else
	{
		for(size_t i = 0; i < pPlayer->m_vecPutCard.size(); i++)
		{
			if( pPlayer->m_vecPutCard[i].m_nTypeBomb != 0 && pPlayer->m_vecPutCard[i].m_nTypeValue == 12)
				return true;
		}
	}
	return false;
}

//底牌抓2 奖励游戏币20个
bool CTask::CheckLordCard(CPlayer* pPlayer)
{
	if( CheckLord(pPlayer) )
	{
		for(size_t i = 0; i < pPlayer->m_pGameTable->m_Poke.m_cLordCard.size(); i++)
		{
			if( pPlayer->m_pGameTable->m_Poke.m_cLordCard[i].m_nValue == 15)
				return true;
		}
	}
	return false ;
}

//底牌抓大王或小王 奖励游戏币50个
bool CTask::CheckLordCardW( CPlayer* pPlayer)
{
	if( CheckLord(pPlayer) )
	{
		for(size_t i = 0; i < pPlayer->m_pGameTable->m_Poke.m_cLordCard.size(); i++)
		{
			if( pPlayer->m_pGameTable->m_Poke.m_cLordCard[i].m_nValue == 16)
				return true;
		}
	}
	return false ;
}

//底牌抓大王并获胜 奖励游戏币60个
bool CTask::CheckLordCardWinW( CPlayer* pPlayer,int nScore)
{
	if(nScore<=0)
		return false;
	if( CheckLord(pPlayer) )
	{
		for(size_t i = 0; i < pPlayer->m_pGameTable->m_Poke.m_cLordCard.size(); i++)
		{
			if( pPlayer->m_pGameTable->m_Poke.m_cLordCard[i].m_nValue == 16 && pPlayer->m_pGameTable->m_Poke.m_cLordCard[i].m_nColor == 1 )
				return true;
		}
	}
	return false;
}

//底牌抓两张相同牌 奖励游戏币20个
bool CTask::CheckLordCardTwo( CPlayer* pPlayer )
{
	if( CheckLord(pPlayer) )
	{
		if((pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue == pPlayer->m_pGameTable->m_Poke.m_cLordCard[1].m_nValue) &&
			(pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue < 16))
			return true ;
		else if((pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue == pPlayer->m_pGameTable->m_Poke.m_cLordCard[2].m_nValue) && 
			(pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue < 16))
			return true ;
		else if((pPlayer->m_pGameTable->m_Poke.m_cLordCard[1].m_nValue == pPlayer->m_pGameTable->m_Poke.m_cLordCard[2].m_nValue) && 
			(pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue < 16))
			return true ;
	}
	return false;
}

//底牌抓到火箭并获胜 奖励游戏币120个
bool CTask::CheckLordCardRocket(CPlayer* pPlayer,int nScore)
{
	if(nScore<=0)
		return false;
	if( CheckLord(pPlayer) )
	{
		if((pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue == pPlayer->m_pGameTable->m_Poke.m_cLordCard[1].m_nValue) &&
			(pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue == 16))
			return true ;
		else if((pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue == pPlayer->m_pGameTable->m_Poke.m_cLordCard[2].m_nValue) && 
			(pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue == 16))
			return true ;
		else if((pPlayer->m_pGameTable->m_Poke.m_cLordCard[1].m_nValue == pPlayer->m_pGameTable->m_Poke.m_cLordCard[2].m_nValue) && 
			(pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue == 16))
			return true ; 
	}
	return false;
}

//底牌抓两张2 奖励游戏币90个
bool CTask::CheckLordCardTT( CPlayer* pPlayer )
{
	if( CheckLord(pPlayer) )
	{
		if((pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue == pPlayer->m_pGameTable->m_Poke.m_cLordCard[1].m_nValue) &&
			(pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue == 15))
			return true ;
		else if((pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue == pPlayer->m_pGameTable->m_Poke.m_cLordCard[2].m_nValue) && 
			(pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue == 15))
			return true ;
		else if((pPlayer->m_pGameTable->m_Poke.m_cLordCard[1].m_nValue == pPlayer->m_pGameTable->m_Poke.m_cLordCard[2].m_nValue) && 
			(pPlayer->m_pGameTable->m_Poke.m_cLordCard[1].m_nValue == 15))		//问题修复
			return true ; 
	}
	return false ;
}

//底牌抓顺子 奖励游戏币100个
bool CTask::CheckLordCardShunZi( CPlayer* pPlayer )
{
	if(CheckLord(pPlayer))
	{
		int m1 = pPlayer->m_pGameTable->m_Poke.m_cLordCard[0].m_nValue;
		int m2 = pPlayer->m_pGameTable->m_Poke.m_cLordCard[1].m_nValue;
		int m3 = pPlayer->m_pGameTable->m_Poke.m_cLordCard[2].m_nValue;
		int t ;
		if(m1<m2)
		{
			t=m2;
			m2=m1;
			m1=t;
		}
		if(m1<m3)
		{
			t=m3;
			m3=m1;
			m1=t;
		}
		if(m2<m3)
		{
			t=m3;
			m3=m2;
			m2=t;
		}
		if( (m1-m2) == (m2-m3) && (m1-m2)==1 && m1<15)
			return true ;
	}
	return false ;
}

//打出4连对及以上并获胜 奖励游戏币120个
bool CTask::CheckDoubleFour( CPlayer* pPlayer,int nScore )
{
	if(nScore<=0)
		return false ;
	for(size_t i = 0; i < pPlayer->m_vecPutCard.size(); i++)
	{
		if( pPlayer->m_vecPutCard[i].m_nTypeNum == 2222 || pPlayer->m_vecPutCard[i].m_nTypeNum == 22222 || pPlayer->m_vecPutCard[i].m_nTypeNum == 222222
			|| pPlayer->m_vecPutCard[i].m_nTypeNum == 2222222 ||pPlayer->m_vecPutCard[i].m_nTypeNum == 22222222 || pPlayer->m_vecPutCard[i].m_nTypeNum == 222222222)
			return true;
	}
	return false ;
}

//检查当前用户是否为地主
bool CTask::CheckLord( CPlayer* pPlayer )
{
	return (pPlayer->GetChairID() == pPlayer->m_pGameTable->m_Poke.m_nCurrentLord);
}

bool CTask::CheckDingTianShun( CPlayer* pPlayer )
{
	for(size_t i = 0; i < pPlayer->m_vecPutCard.size(); i++)
	{
		if( pPlayer->m_vecPutCard[i].m_nTypeNum == 12)
			return true;
	}
	return false;
}

bool CTask::Check96Times( CPlayer* pPlayer,int nSCore)
{
	if(nSCore<=0)
		return false;
	if(pPlayer->m_pGameTable->m_nDouble >= 96)
		return true;
	return false;
}

bool CTask::CheckShunZi( CPlayer* pPlayer,int nSCore )
{
	if(nSCore<=0)
		return false;
	for(size_t i = 0; i < pPlayer->m_vecPutCard.size(); i++)
	{
		int num = pPlayer->m_vecPutCard[i].m_nTypeNum;
		if( num == 5 || num == 6 || num == 7 || num == 8 
			|| num == 9 || num == 10 || num == 11 || num == 12 || num==13)
			return true;
	}
	return false;
}

bool CTask::CheckLianDui( CPlayer* pPlayer,int nScore )
{
	if(nScore<=0)
		return false;
	int num = 0;
	for(size_t i = 0; i < pPlayer->m_vecPutCard.size(); i++)
	{
		num = pPlayer->m_vecPutCard[i].m_nTypeNum;
		if(num == 222 ||  num == 2222 || num == 22222 || num == 222222
			|| num == 2222222 || num == 22222222 || num == 222222222)
			return true; 
	}
	return false;
}

bool CTask::CheckSpring( CPlayer* pPlayer,int nScore )
{
	if(nScore<=0)
		return false;
	if(CheckLord(pPlayer))
	{	
		bool isSpring = true;
		for (int i = 0; i < pPlayer->m_pGameTable->GetPlayNum(); i++)
		{
			if( i != pPlayer->GetChairID() )
			{
				CPlayer* pLose = pPlayer->m_pGameTable->GetPlayer(i);
				if( pLose )
				{
					if( pLose->m_PlayCard.m_cCards.size() < 17 )	//农民手上的牌不得少于17张
					{
						isSpring = false;
						break;
					}
				}
			}
		}
		//春天次数
		if(isSpring)
		{
			return true;
		}
	}
	return false;
}

//手牌抓4张癞子
bool CTask::Check4LaiZi( CPlayer* pPlayer,int nSCore )
{
	if(g_nBaoPai == 1)
	{
		vector<CCard> cards_= pPlayer->m_vecHandCard;
		int num = 0 ;
		for(size_t i=0;i<cards_.size();i++)
		{
			if(cards_[i].m_nValue == pPlayer->m_PlayCard.m_nBaoValue)
			{
				num++;
			}
		}
		if(num == 4)
			return true;
		return false;
	}
	return false;
}

//手牌抓2张以上癞子并获胜
bool CTask::Check2LaiZiWin( CPlayer* pPlayer,int nSCore )
{
	if(g_nBaoPai == 1)
	{
		if(nSCore<0)
			return false;
		vector<CCard> cards_= pPlayer->m_vecHandCard;
		int num = 0 ;
		for(size_t i=0;i<cards_.size();i++)
		{
			if(cards_[i].m_nValue == pPlayer->m_PlayCard.m_nBaoValue)
				num++;
		}
		if(num >= 2)
			return true;
		return false;
	}
	return false;
}

//手牌抓两张癞子牌2并获胜
bool CTask::Check2LaiZi2Win( CPlayer* pPlayer,int nSCore )
{
	if(g_nBaoPai == 1)
	{
		if(nSCore<0)								//输牌
			return false;
		if(pPlayer->m_PlayCard.m_nBaoValue != 15)
			return false;
		vector<CCard> cards_= pPlayer->m_vecHandCard;
		int num = 0 ;
		for(size_t i=0;i<cards_.size();i++)
		{
			if(cards_[i].m_nValue == pPlayer->m_PlayCard.m_nBaoValue)
				num++;
		}
		if(num >= 2)
			return true;
		return false;
	}
	return false;
}

//打出纯癞子炸弹并获胜
bool CTask::Check4LaiZiWin( CPlayer* pPlayer,int nScore )
{
	if(g_nBaoPai == 0)
		return false;
	if(nScore<0)
		return false;
	for(size_t i = 0; i < pPlayer->m_vecPutCard.size(); i++)
	{
		if( pPlayer->m_vecPutCard[i].m_nTypeBomb != 0 && pPlayer->m_vecPutCard[i].m_nTypeBomb == 2 && pPlayer->m_vecPutCard[i].m_nTypeValue == BAO)
			return true;
	}
	return false ;
	
}

//打出5软炸弹并获胜
bool CTask::Check5SoftBombWin( CPlayer* pPlayer,int nScore )
{
	if(g_nBaoPai == 0)
		return false;
	if(nScore<0)
		return false;
	for(size_t i = 0; i < pPlayer->m_vecPutCard.size(); i++)
	{
		if( pPlayer->m_vecPutCard[i].m_nTypeBomb != 0 && pPlayer->m_vecPutCard[i].m_nTypeBomb == 1 && pPlayer->m_vecPutCard[i].m_nTypeValue == 5)
			return true;
	}
	return false;
}

//底牌抓癞子牌并获胜
bool CTask::CheckLordLaiZiWin( CPlayer* pPlayer,int nSCore )
{
	if(g_nBaoPai == 0)
		return false;
	if(nSCore<0)
		return false;
	if(CheckLord(pPlayer))
	{
		int num = 0 ;
		for(size_t i=0;i<pPlayer->m_pGameTable->m_Poke.m_cLordCard.size();i++)
		{
			if(pPlayer->m_pGameTable->m_Poke.m_cLordCard[i].m_nValue == pPlayer->m_PlayCard.m_nBaoValue)
				num ++ ;
		}
		if(num > 0)
			return true;
		return false;
	}
	return false ;
}
