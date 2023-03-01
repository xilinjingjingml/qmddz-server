#include "stdafx.h"
#include "ConfigManager.h"
#include "sql_define.h"

#define READ_XML_ATTR(attr, member, value) \
	if (!pElement->Attribute(attr, &member)) \
		member = value; \

int g_nCallScore = 0;
int g_nRobLord = 0;
int g_nShowCard = 0;
int g_nBaseScore = 0;
int g_nHighMoney = 0;
int g_nHighScore = 0;
int g_nHighTaxMoney = 0;
int g_nTask = 0;
int g_nEscapeMode = 0;
int g_nAutoSave = 0;
int g_nEscapeMoney = 0;
int g_nChangeTable = 0;
int g_nAutoAddFan = 0;
int g_nLordWinScore = 0;
int g_nFarmerWinScore = 0;
int g_nDefaultLord = 0;
int g_nTID = 0;
int g_nMission = 0;
int g_nTax = 0;
int g_nBroadcastWin = 0;
int g_nBroadcastWinNum =0;
int g_nBroadcastTask=0;
int g_nBaoPai=0; 
int g_nCounts=0;
int g_nRace=0;
int g_nGameType=0;
int g_nZhaDanDef=0;
int g_nZhaDanNum=0;
int g_nSaveNum=0;
int g_nSendCardNum=0;
int g_nSaveCardDouble=0;
int g_nDynamic=0;
int g_nMaxMoney=0;
int g_nLimitMoney=0;
int g_nLimitMoneyNext=0;
int g_nSendCardCommon=0;
int g_debug_level = 0;
int g_nNewShuffleCards = 0; 
int g_nBombMultiple = 0; 
int g_nBetterSeat = 0; 
int g_nCardTidiness = 0; 
int g_nBaseGood = 0;   
int g_nMatch = 0;
int g_nNewMatch = 0;
int g_nMinDouble = 0;
int g_nMaxDouble = 0;
int g_nLessThanZero= 0;
int g_nBaseLordCardLottery = 0;
int g_nRecordPlayerCard = 0;
int g_nBaoDiRate = 0;
int g_nBaoDiMoney = 0;
int g_nTwoPai = 0;
int g_nTwoBaseRate = 0;
int g_nDoubleScore = 0;
int g_nPrivateRoom = 0;
int g_nPauseTime = 0;
int g_nTimeOut = 1;
int g_nPlayCardTime = 0;
int g_nIdleTime = 10;
int g_nTwoPaiMinLet = 0;
int g_nTwoPaiLordLet = 0;
int g_nTwoPaiLordDouble = 1;
int g_nFirstPlayCardTime = 0;
int g_nIsNeedMagicEmoji = 0;
int g_nIsStandard = 0;
int g_nIsStarSky = 0;

int g_nHBPoolRate;
int g_nHB88YuanPoolRate;
int g_nHB88YuanEnable = 0;
int g_nHBMode;
int g_nHBTaskRound;
int g_nMinFan;		//最小番
int g_nMaxFan;		//最大番
int g_nRedPacketBCSTTHR;
string g_sServerName;
int g_nGame_id = 0;

int g_nBetterSeatForNewBie = 0;
int g_nBetterSeatForNewBieRound = 0;
int g_nRedPacketLimit = 0;

int g_nShowCardDoubleInReady = 1;
int g_nShowCardDoubleInDeal = 1;
int g_nShowCardDoubleInPutCard = 1;
int g_nRPGMode = 0;
int g_nLetCard = 0;
int g_nDealCardNum = 17;

int g_nRegainLoseCountLimit = 0;
int g_nRegainLoseMAXCount = 0;
int g_nRegainLoseTimeLimit = 0;
int g_nRegainLoseFreeCount = 0;
int g_nRegainLoseFreeCountNew = 0;
int g_nInvincibleMode = 0;

int g_nRedPacketExtraLimit = 0;
int g_nRedPacketExtraProbabily = 0;
int g_nBaseInitDouble = 0;
int g_nBuXiPai = 0;
int g_nCleverRobot = 0;
int g_nDebugShowCard = 0;
int g_nLoggerHandCard = 0;
int g_nRedPacketsNewbie = 0;
int g_nRedPacketsNewbieAddRound = 0;
int g_nChipFall = 0;
int g_nChipLegendMax = 0;
int g_nChipIndex = 0;
int g_nChipIndexNewbie1 = 0;
int g_nChipIndexNewbie2 = 0;
int g_nChipIndexNewbie3 = 0;
int g_nChipNewbieRound1 = 0;
int g_nChipNewbieRound2 = 0;
int g_nChipNewbieRound3 = 0;
int g_nRedpakcetControllerRatio = 1;
int g_nWinDoubleCan = 0;
int g_nWinDoubleAddAmount = 0;
int g_nWinDoubleAddProbabily = 0;
int g_nShuffleCardsForNewBieFileABTest = 0;
int g_nShuffleCardsForNewBieFileRound = 0;
vector<int> g_nRegainLoseMoneyLimit;

CConfigManager::CConfigManager(void) :m_nPlay_Num(3)
{
}

CConfigManager::~CConfigManager(void)
{
}

bool InitSQLite()
{	
	return true;
}

bool CConfigManager::Init(const char* m_szConfigFile)
{

	TiXmlDocument doc(m_szConfigFile);
	if(!doc.LoadFile())
	{
		glog.log(doc.ErrorDesc());

		return false;
	}

	TiXmlElement* pElement = NULL;
	pElement = doc.FirstChildElement("config");
	if(!pElement)
		return false;
	READ_XML_ATTR("CallScore", g_nCallScore, 0);
	READ_XML_ATTR("RobLord", g_nRobLord, 0);
	READ_XML_ATTR("ShowCard", g_nShowCard, 0);
	READ_XML_ATTR("BaseScore", g_nBaseScore, 0);
	READ_XML_ATTR("HighMoney", g_nHighMoney, 0);
	READ_XML_ATTR("HighScore", g_nHighScore, 0);
	READ_XML_ATTR("HighTaxMoney", g_nHighTaxMoney, 0);
	READ_XML_ATTR("Task", g_nTask, 0);
	READ_XML_ATTR("Escape", g_nEscapeMode, 0);
	READ_XML_ATTR("AutoSave", g_nAutoSave, 0);
	READ_XML_ATTR("EscapeMoney", g_nEscapeMoney, 0);
	READ_XML_ATTR("ChangeTable", g_nChangeTable, 0);
	READ_XML_ATTR("AutoFan", g_nAutoAddFan, 0);
	READ_XML_ATTR("LordScore", g_nLordWinScore, 0);
	READ_XML_ATTR("FarmerScore", g_nFarmerWinScore, 0);
	READ_XML_ATTR("DefaultLord", g_nDefaultLord, 0);
	READ_XML_ATTR("TID", g_nTID, 1);
	READ_XML_ATTR("Mission", g_nMission, 0);
	READ_XML_ATTR("Tax", g_nTax, 0);
	READ_XML_ATTR("BroadCastTask", g_nBroadcastTask, 0);
	READ_XML_ATTR("BroadCastWin", g_nBroadcastWin, 0);
	READ_XML_ATTR("BroadCastWinNum", g_nBroadcastWinNum, 0);
	READ_XML_ATTR("LaiZi", g_nBaoPai, 0);
	READ_XML_ATTR("CardCounts", g_nCounts, 0);
	READ_XML_ATTR("RaceRule", g_nRace, 0);
	READ_XML_ATTR("GameType", g_nGameType, 0);
	READ_XML_ATTR("ZhaDanDef", g_nZhaDanDef, 0);
	READ_XML_ATTR("ZhaDanNum", g_nZhaDanNum, 0);
	READ_XML_ATTR("SaveNum", g_nSaveNum, 0);
	READ_XML_ATTR("SendCardNum", g_nSendCardNum, 0);
	READ_XML_ATTR("SaveCardDouble", g_nSaveCardDouble, 0);
	READ_XML_ATTR("Dynamic", g_nDynamic, 0);
	READ_XML_ATTR("LoseMaxMoney", g_nMaxMoney, 0);
	READ_XML_ATTR("LimitMoney", g_nLimitMoney, 0);
	READ_XML_ATTR("LimitMoneyNext", g_nLimitMoneyNext, 0);
	READ_XML_ATTR("SendCardCommon", g_nSendCardCommon, 0);
	READ_XML_ATTR("DebugLevel", g_debug_level, 0);
	READ_XML_ATTR("NewShuffleCards", g_nNewShuffleCards, 0);
	READ_XML_ATTR("BombMultiple", g_nBombMultiple, 0);
	READ_XML_ATTR("BetterSeat", g_nBetterSeat, 0);
	READ_XML_ATTR("CardTidiness", g_nCardTidiness, 0);
	READ_XML_ATTR("BaseGood", g_nBaseGood, 0);
	READ_XML_ATTR("Match", g_nMatch, 0);
	READ_XML_ATTR("MinDouble", g_nMinDouble, 0);
	READ_XML_ATTR("MaxDouble", g_nMaxDouble, 0);
	READ_XML_ATTR("LessThanZero", g_nLessThanZero, 0);
	READ_XML_ATTR("BaseLordCardLottery", g_nBaseLordCardLottery, 0);
	READ_XML_ATTR("RecordPlayerCard", g_nRecordPlayerCard, 0);
	READ_XML_ATTR("BaoDiRate", g_nBaoDiRate, 0);
	READ_XML_ATTR("BaoDiMoney", g_nBaoDiMoney, 0);
	READ_XML_ATTR("TwoPai", g_nTwoPai, 0);
	READ_XML_ATTR("TwoPaiBaseRate", g_nTwoBaseRate, 0);
	READ_XML_ATTR("TwoPaiMinLet", g_nTwoPaiMinLet, 0);
	READ_XML_ATTR("TwoPaiLordLet", g_nTwoPaiLordLet, 0);
	READ_XML_ATTR("TwoPaiLordDouble", g_nTwoPaiLordDouble, 1);
	READ_XML_ATTR("DoubleScore", g_nDoubleScore, 0);
	READ_XML_ATTR("Private", g_nPrivateRoom, 0);
	READ_XML_ATTR("PauseTime", g_nPauseTime, 0);
	READ_XML_ATTR("TimeOut", g_nTimeOut, 1);
	READ_XML_ATTR("PlayCardTime", g_nPlayCardTime, 0);
	READ_XML_ATTR("IdleTime", g_nIdleTime, 10);
	READ_XML_ATTR("NewMatch", g_nNewMatch, 0);
	READ_XML_ATTR("FirstPlayCardTime", g_nFirstPlayCardTime, 0);
	READ_XML_ATTR("Standard", g_nIsStandard, 0);
	READ_XML_ATTR("StarSky", g_nIsStarSky, 0);
	READ_XML_ATTR("HBPoolRate", g_nHBPoolRate, 0);
	READ_XML_ATTR("HB88YuanPoolRate", g_nHB88YuanPoolRate, 0);
	READ_XML_ATTR("HBMode", g_nHBMode, 0);
	READ_XML_ATTR("HBTaskRound", g_nHBTaskRound, 0);
	READ_XML_ATTR("MinFan", g_nMinFan, 0);
	READ_XML_ATTR("MaxFan", g_nMaxFan, 0);
	READ_XML_ATTR("RedPacketBCSTTHR", g_nRedPacketBCSTTHR, 0);
	READ_XML_ATTR("GameId", g_nGame_id, 0);
	READ_XML_ATTR("BetterSeatForNewBie", g_nBetterSeatForNewBie, 0);
	READ_XML_ATTR("BetterSeatForNewBieRound", g_nBetterSeatForNewBieRound, 0);
	READ_XML_ATTR("RedPacketLimit", g_nRedPacketLimit, 0);
	READ_XML_ATTR("IsNeedMagicEmoji", g_nIsNeedMagicEmoji, 0);
	READ_XML_ATTR("ShowCardDoubleInReady", g_nShowCardDoubleInReady, 1);
	READ_XML_ATTR("ShowCardDoubleInDeal", g_nShowCardDoubleInDeal, 1);
	READ_XML_ATTR("ShowCardDoubleInPutCard", g_nShowCardDoubleInPutCard, 1);
	READ_XML_ATTR("RPGMode", g_nRPGMode, 0);
	READ_XML_ATTR("LetCard", g_nLetCard, 0);
	READ_XML_ATTR("DealCardNum", g_nDealCardNum, 17);
	READ_XML_ATTR("RegainLoseCountLimit", g_nRegainLoseCountLimit, 0);
	READ_XML_ATTR("RegainLoseTimeLimit", g_nRegainLoseTimeLimit, 0);
	READ_XML_ATTR("RegainLoseFreeCount", g_nRegainLoseFreeCount, 0);
	READ_XML_ATTR("RegainLoseFreeCountNew", g_nRegainLoseFreeCountNew, 0);
	READ_XML_ATTR("InvincibleMode", g_nInvincibleMode, 0);
	READ_XML_ATTR("RegainLoseMAXCount", g_nRegainLoseMAXCount, 0);
	READ_XML_ATTR("RedPacketExtraProbabily", g_nRedPacketExtraProbabily, 0);
	READ_XML_ATTR("RedPacketExtraLimit", g_nRedPacketExtraLimit, 0);
	READ_XML_ATTR("BaseInitDouble", g_nBaseInitDouble, 0);
	READ_XML_ATTR("BuXiPai", g_nBuXiPai, 0);
	READ_XML_ATTR("CleverRobot", g_nCleverRobot, 0);
	READ_XML_ATTR("DebugShowCard", g_nDebugShowCard, 0);
	READ_XML_ATTR("LoggerHandCard", g_nLoggerHandCard, 0);
	READ_XML_ATTR("RedPacketsNewbie", g_nRedPacketsNewbie, 0);
	READ_XML_ATTR("RedPacketsNewbieAddRound", g_nRedPacketsNewbieAddRound, 0);
	READ_XML_ATTR("RedPacketsNewbieAddRound", g_nRedPacketsNewbieAddRound, 0);
	READ_XML_ATTR("ChipFall", g_nChipFall, 0);
	READ_XML_ATTR("ChipLegendMax", g_nChipLegendMax, 294);
	READ_XML_ATTR("ChipIndex", g_nChipIndex, 0);
	READ_XML_ATTR("ChipIndexNewbie1", g_nChipIndexNewbie1, 1);
	READ_XML_ATTR("ChipIndexNewbie2", g_nChipIndexNewbie2, 2);
	READ_XML_ATTR("ChipIndexNewbie3", g_nChipIndexNewbie3, 3);
	READ_XML_ATTR("ChipNewbieRound1", g_nChipNewbieRound1, 10);
	READ_XML_ATTR("ChipNewbieRound2", g_nChipNewbieRound2, 20);
	READ_XML_ATTR("ChipNewbieRound3", g_nChipNewbieRound3, 30);
	READ_XML_ATTR("RedpakcetControllerRatio", g_nRedpakcetControllerRatio, 1);
	READ_XML_ATTR("WinDoubleCan", g_nWinDoubleCan, 0);
	READ_XML_ATTR("WinDoubleAddAmount", g_nWinDoubleAddAmount, 0);
	READ_XML_ATTR("WinDoubleAddProbabily", g_nWinDoubleAddProbabily, 0);
	READ_XML_ATTR("ShuffleCardsForNewBieFileABTest", g_nShuffleCardsForNewBieFileABTest, 0);
	READ_XML_ATTR("ShuffleCardsForNewBieFileRound", g_nShuffleCardsForNewBieFileRound, 0);
	g_sServerName = pElement->Attribute("ServerName");
	
	

	if (g_nRegainLoseCountLimit > 0)
	{
		g_nRegainLoseMoneyLimit.clear();
		g_nRegainLoseMoneyLimit.push_back(4000000);
		g_nRegainLoseMoneyLimit.push_back(6000000);
	}
	
	
	

	
	if (g_nTimeOut < 1)
	{
		g_nTimeOut = 1;
	}
	if (g_nPrivateRoom == 1)
	{
		g_nCounts = 0;
	}
	//字符串
	ExTable string_table;
	if(!string_table.InitFromFile("string_data.csv")){
		glog.error("load string_table.csv faield");
		return false;
	}
	for(int i = 0; i < string_table.GetRecordNum(); ++i){
		string key		= string_table.GetStrField(i, 0);
		string value	= string_table.GetStrField(i, 1);
		strings_[key] = value;
	}

	if( g_nTask == 1 )
	{
		ExTable task_data;
		if(!task_data.InitFromFile("task.csv")){
			glog.error("load task.csv faield");
			return false;
		}
		for(int i = 0; i < task_data.GetRecordNum(); ++i){
			TaskItem task_item ;
			task_item.task_id_ = task_data.GetIntField(i,0);
			task_item.task_desc_ = task_data.GetStrField(i,1);
			task_item.task_mission_ = task_data.GetStrField(i,2);
			task_item.task_money_type_ = task_data.GetIntField(i,3);
			task_item.task_money_ = task_data.GetIntField(i,4);
			task_item.task_rate_ = task_data.GetIntField(i,5);
			vecTask_.push_back(task_item);

		}
	}

	if(g_nBroadcastTask == 1)
	{
		ExTable broadcast_data;
		if(!broadcast_data.InitFromFile("broadcast_task.csv"))
		{
			glog.error("load broadcast_task.csv failed");
			return false;
		}

		for(int i = 0; i < broadcast_data.GetRecordNum(); ++i){
			//int key		= broadcast_data.GetIntField(i, 0);
			string value	= broadcast_data.GetStrField(i, 1);
			vecBroadCastTask_.push_back(i);
			vecBroadCastTask_desc_.push_back(value);
		}
	}
	if (g_nPrivateRoom == 1 || g_nMatch == 1 || g_nNewMatch == 1)
	{
		g_nIsNeedMagicEmoji = 1;
	}
	if (g_nIsNeedMagicEmoji == 1)
	{
		ExTable magic_emoji_table;
		if (!magic_emoji_table.InitFromFile("magic_emoji_config.csv")) {
			glog.error("load magic_emoji_config.csv failed");
			return false;
		}

		m_vecMagicEmojiConfigs.clear();
		for (int i = 0; i < magic_emoji_table.GetRecordNum(); ++i) {

			MagicEmojiItem item;
			item.nItemIndex = magic_emoji_table.GetIntField(i, 0);
			item.nItemConsumeCoins = magic_emoji_table.GetIntField(i, 1);
			item.nItemName = magic_emoji_table.GetStrField(i, 2);
			item.nTenItemIndex = magic_emoji_table.GetIntField(i, 3);
			item.nTenItemNum = magic_emoji_table.GetIntField(i, 4);
			item.nTenEmojiNum = magic_emoji_table.GetIntField(i, 5);
			m_vecMagicEmojiConfigs.push_back(item);
		}
	}

	if (g_nIsStarSky == 1)
	{
		ExTable starsky_data;
		if (!starsky_data.InitFromFile("starsky_double_config.csv"))
		{
			glog.error("load starsky_double_config.csv failed");
			return false;
		}

		m_mapStarSkyDouble.clear();
		for (int i = 0; i < starsky_data.GetRecordNum(); ++i){
			m_mapStarSkyDouble[starsky_data.GetIntField(i, 0)] = starsky_data.GetIntField(i, 1);
		}

		if (!starsky_data.InitFromFile("starsky_tax_config.csv"))
		{
			glog.error("load starsky_tax_config.csv failed");
			return false;
		}

		m_mapStarSkyTax.clear();
		for (int i = 0; i < starsky_data.GetRecordNum(); ++i){
			m_mapStarSkyTax[starsky_data.GetIntField(i, 0)] = starsky_data.GetIntField(i, 1);
		}
	}

	//是否二人斗地主
	if (g_nTwoPai == 1)
	{
		m_nPlay_Num = 2;
		g_nLetCard = 1;
	}

	if (g_nHBMode > 0)
	{
		if (g_nHBMode <= HONGBAO_GAOJI) {
			getRedPacketInfoConfig();
		}else if (g_nHBMode >= HONGBAO_H5_CHUJI && g_nHBMode < HONGBAO_H5_END) {
			if (g_nHBMode == HONGBAO_H5_CHUJI) {
				getRedPacketNewBieInfoConfig();
			}
			getRedPacketNewBieInfo();
			getRedPacketInfoConfigForH5();
			getRedPacket88YuanConfig();
		}
	}

	readRewardConfig();

	SERVER_LOG("---------------m_nPlay_Num %d", m_nPlay_Num);
	return true;
}

int CConfigManager::CalcCharCountInUTF8( const char * pszText, int maxCharNumInWidth )
{
	int n = 0, m=0;
	char ch = 0;

	bool flag = false;
	while ((ch = *pszText))
	{
		if(!ch)
			break;

		if (0x80 != (0xC0 & ch))
		{
			if(n+m/2 >= maxCharNumInWidth)
			{
				if(n+m/2 == maxCharNumInWidth)
					return n+m;
				else if(n+m/2 > maxCharNumInWidth)
				{
					if(flag)
						return n+m-3;
					else
						return n+m-1;
				}
			}
			else
			{
				flag = false;
				++n;
			}
		}
		else
		{
			flag = true;
			++m;
		}
		++pszText;
	}
	return n+m;
}

int CConfigManager::CalcCharCountInWidth( const char * pszText )
{
	int n = 0, m=0;
	char ch = 0;

	while ((ch = *pszText))
	{
		if(!ch)
			break;

		if (0x80 != (0xC0 & ch))
			++n;
		else
			++m;

		++pszText;
	}
	return n+m/2;
}

int CConfigManager::GetStarSkyScoreByDouble(int d)
{
	for (map<int, int>::reverse_iterator itr = m_mapStarSkyDouble.rbegin(); itr != m_mapStarSkyDouble.rend(); itr++)
	{
		if (d > itr->first)
		{
			return itr->second;
		}
	}

	return 0;
}

int CConfigManager::GetStarSkyTaxByDouble(int d)
{
	for (map<int, int>::reverse_iterator itr = m_mapStarSkyTax.rbegin(); itr != m_mapStarSkyTax.rend(); itr++)
	{
		if (d >= itr->first)
		{
			return itr->second;
		}
	}

	return 0;
}

void CConfigManager::getRedPacketLimitConfig()
{
	glog.log("CConfigManager:getRedPacketLimitConfig");
	ExTable string_table2;
	if (!string_table2.InitFromFile("redpacket_limit_config.csv"))
	{
		glog.error("load redpacket_limit_config.csv.csv failed");
		return;
	}


	for (int i = 0; i < string_table2.GetRecordNum(); i++)
	{
		structRedPackeLimit redPacketLimit;
		redPacketLimit.cLid = string_table2.GetIntField(i, 0);
		redPacketLimit.nPayRate = string_table2.GetIntField(i, 1);
		redPacketLimit.nReduceProbabily = string_table2.GetIntField(i, 2);
		redPacketLimit.nPoolDeltaRate = string_table2.GetIntField(i, 3);

		m_vecRedPacketLimitConfig.push_back(redPacketLimit);

		glog.log("redPacketLimit cLid: %d,nPayRate: %d,nReduceRate: %d,nPoolDeltaRate: %d,", 
			redPacketLimit.cLid, redPacketLimit.nPayRate, redPacketLimit.nReduceProbabily, redPacketLimit.nPoolDeltaRate);
	}
	

}

void CConfigManager::getRedPacketInfoConfigCount()
{
	glog.log("CConfigManager:getRedPacketInfoConfigCount");
	ExTable string_table2;
	if (!string_table2.InitFromFile("redpacket_limit_h5_config.csv"))
	{
		glog.error("load redpacket_limit_h5_config.csv failed");
		m_vecRedpacketConfigCount.push_back(0);
		return;
	}
	for (int i = 0; i < string_table2.GetRecordNum(); i++)
	{
		if (string_table2.GetIntField(i, 0) == 0)
		{
			m_vecRedpacketConfigCount.push_back(string_table2.GetIntField(i, 1));
		}else if (string_table2.GetIntField(i, 0) == 1) {
			m_vecRedpacketConfigTotal.push_back(string_table2.GetIntField(i, 1));
		}
		
	}

}

void CConfigManager::getRedPacketNewBieInfo()
{
	if (g_nRedPacketsNewbie == 0)
	{
		return;
	}

	glog.log("CConfigManager:getRedPacketNewBieInfo");
	ExTable string_table2;
	if (!string_table2.InitFromFile("redpacket_newbie_info.csv"))
	{
		glog.error("load redpacket_newbie_info.csv failed");
		return;
	}
	for (int i = 0; i < string_table2.GetRecordNum(); i++)
	{
		structRedPacketInfo redPacketInfo;
		redPacketInfo.nRoundInterval = string_table2.GetIntField(i, 0);
		redPacketInfo.nAmount = string_table2.GetIntField(i, 1);
		redPacketInfo.nDoubel = string_table2.GetIntField(i, 2);
		m_vecRedPacketNewBieInfo.push_back(redPacketInfo);
	}
}

void CConfigManager::readRewardConfig()
{
	glog.log("CConfigManager:readRewardConfig");
	ExTable table;
	if (!table.InitFromFile("reaward_config.csv"))
	{
		glog.error("load reaward_config.csv failed");
		return;
	}

	for (int i = 0; i < table.GetRecordNum(); i++)
	{
		int nWeight = table.GetIntField(i, 4);
		if (nWeight <= 0)
		{
			continue;
		}

		structRewardConfig sRewardConfig;
		int nId = table.GetIntField(i, 0);
		sRewardConfig.nItemId = table.GetIntField(i, 1);
		sRewardConfig.nItemNumMin = table.GetIntField(i, 2);
		sRewardConfig.nItemNumMax = table.GetIntField(i, 3);
		sRewardConfig.nWeight = nWeight;
		
		map<int, structRewardConfigList>::iterator iterConfig = m_mapRewardConfig.find(nId);
		if (iterConfig == m_mapRewardConfig.end())
		{
			structRewardConfigList sRewardConfigList;
			sRewardConfigList.nWeightSum = 0;
			sRewardConfigList.vecRewardConfig.push_back(sRewardConfig);

			m_mapRewardConfig[nId] = sRewardConfigList;
		}
		else
		{
			iterConfig->second.vecRewardConfig.push_back(sRewardConfig);
		}
	}

	// 计算权重和
	for (map<int, structRewardConfigList>::iterator iterConfig = m_mapRewardConfig.begin(); iterConfig != m_mapRewardConfig.end(); iterConfig++)
	{
		for (vector<structRewardConfig>::iterator iter = iterConfig->second.vecRewardConfig.begin(); iter != iterConfig->second.vecRewardConfig.end(); iter++)
		{
			iterConfig->second.nWeightSum += iter->nWeight;
		}
	}
}

void CConfigManager::getRewardNum(int nId, structRewardInfo& info)
{
	map<int, structRewardConfigList>::iterator iterConfig = m_mapRewardConfig.find(nId);
	if (iterConfig != m_mapRewardConfig.end())
	{
		int nWeight = rand() % iterConfig->second.nWeightSum;
		for (vector<structRewardConfig>::iterator iter = iterConfig->second.vecRewardConfig.begin(); iter != iterConfig->second.vecRewardConfig.end(); iter++)
		{
			nWeight -= iter->nWeight;
			if (nWeight < 0)
			{
				info.nItemId = iter->nItemId;
				info.nItemNum = iter->nItemNumMin;
				if (iter->nItemNumMin != iter->nItemNumMax)
				{
					info.nItemNum += rand() % (iter->nItemNumMin - iter->nItemNumMin);
				}
				break;
			}
		}
	}
}

void CConfigManager::getRedPacketInfoConfigForH5()
{
	getRedPacketInfoConfigCount();

	glog.log("CConfigManager:getRedPacketInfoConfig");
	ExTable string_table2;
	if (!string_table2.InitFromFile("redpacket_h5_config.csv"))
	{
		glog.error("load redpacket_h5_config.csv.csv failed");
		return;
	}
	for (int i = 0; i < string_table2.GetRecordNum(); i++)
	{
		structRedPacket redPacketInfo;
		redPacketInfo.cLId = string_table2.GetIntField(i, 0);
		redPacketInfo.nVipLimit = string_table2.GetIntField(i, 1);
		redPacketInfo.nItemIndex = string_table2.GetIntField(i, 2);
		redPacketInfo.nAmountMin = string_table2.GetIntField(i, 3);
		redPacketInfo.nAmountMax = string_table2.GetIntField(i, 4);
		
		for (int j = 0; j < m_vecRedpacketConfigCount.size(); ++j) {
			redPacketInfo.nProbabily.push_back(10 * string_table2.GetIntField(i, j + 5));
		}
		for (int j = 0; j < m_vecRedpacketConfigTotal.size(); ++j) {
			redPacketInfo.nProbabily.push_back(10 * string_table2.GetIntField(i, j + 5 + m_vecRedpacketConfigCount.size()));
		}
		

		m_vecRedPacketConfig.push_back(redPacketInfo);
		glog.log("getRedPacketInfoConfig cLId_:[%d] nVipLimit:[%d] nAmount_[%d] nProbabily[%d]",
			redPacketInfo.cLId, redPacketInfo.nVipLimit, redPacketInfo.nAmountMax, redPacketInfo.nProbabily.size());
	}
}


void CConfigManager::getRedPacketInfoConfig()
{

	glog.log("CConfigManager:getRedPacketInfoConfig");
	ExTable string_table2;
	if (!string_table2.InitFromFile("redpacket_config.csv"))
	{
		glog.error("load redpacket_config.csv failed");
		return;
	}
	for (int i = 0; i < string_table2.GetRecordNum(); i++)
	{
		structRedPacket redPacketInfo;
		redPacketInfo.cLId = string_table2.GetIntField(i, 0);
		redPacketInfo.nVipLimit = string_table2.GetIntField(i, 1);
		redPacketInfo.nItemIndex = string_table2.GetIntField(i, 2);
		redPacketInfo.nAmountMin = string_table2.GetIntField(i, 3);
		redPacketInfo.nAmountMax = redPacketInfo.nAmountMin;
		redPacketInfo.nProbabily.push_back(string_table2.GetIntField(i, 4));

		m_vecRedPacketConfig.push_back(redPacketInfo);
		glog.log("getRedPacketInfoConfig cLId_:[%d] nVipLimit:[%d] nAmount_[%d] nProbabily[%d]",
			redPacketInfo.cLId, redPacketInfo.nVipLimit, redPacketInfo.nAmountMax, redPacketInfo.nProbabily.size());
	}
}


void CConfigManager::getRedPacketNewBieInfoConfig()
{

	glog.log("CConfigManager:getRedPacketNewBieInfoConfig");
	ExTable string_table2;
	if (!string_table2.InitFromFile("redpacket_newbie_config.csv"))
	{
		glog.error("load redpacket_newbie_config.csv failed");
		return;
	}
	for (int i = 0; i < string_table2.GetRecordNum(); i++)
	{
		int nValue = string_table2.GetIntField(i, 0);
		m_vecRedPacketNewBieConfig.push_back(nValue);
	}
}


void CConfigManager::getRedPacket88YuanConfig()
{

	glog.log("CConfigManager:getRedPacket88YuanConfig");
	ExTable string_table2;
	if (!string_table2.InitFromFile("redpacket_88yuan_config.csv"))
	{
		glog.error("load redpacket_88yuan_config.csv failed");
		return;
	}

	for (int i = 0; i < string_table2.GetRecordNum(); i++)
	{
		structRedPacket88Yuan redPacket88YuanInfo;
		redPacket88YuanInfo.cLid = string_table2.GetIntField(i, 0);
		redPacket88YuanInfo.nAmount = string_table2.GetIntField(i, 1);
		redPacket88YuanInfo.nRoundLimit = string_table2.GetIntField(i, 2);
		m_vecRedPacket88YuanConfig.push_back(redPacket88YuanInfo);
		g_nHB88YuanEnable = 1;
	}
}

bool CConfigManager::isRobotGuid( const char *guid )
{
	if (!guid)
	{
		return false;
	}
	
	if (strlen(guid) < 15)
	{
		return false;
	}

	if (guid[0] == '9' && guid[1] == '9')
	{
		return true;
	}

	return false;
}

