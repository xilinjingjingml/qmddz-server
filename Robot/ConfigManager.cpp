#include "stdafx.h"
#include "ConfigManager.h"
#include <stdlib.h>

int g_debug_level_ = 0;
int g_chat_time_ = 0;
int g_ping_time_ = 0;
int g_ready_time_ = 0;
int g_ready_timeout_ = 0;
int g_callscore_time_ = 0;
int g_roblord_time_ = 0;
int g_playcard_time_ = 0;
int g_leavetable_time_ = 0;
int g_login_time_ = 0;
int g_robot_type_ = 0; // 机器人等级类型;
int g_rule_type_ = 0; // 游戏规则，赖子，非赖子;
int g_test_robot_ = 0; //机器人进入自动测试状态
int g_user_robot_ =0; //机器人扮演玩家角色
int g_need_userrobot_=0; //需要机器人玩家进入，只有当g_user_robot_=1时有效
int g_max_table_num = 0; 
int g_nBetterForPlayer = 0;
int g_nNotCallScore = 0;
int g_playcard_time_min_ = 0;
int g_playcard_time_max_ = 0;
#define CHAT_TIME 15000
#define PING_TIME  5000
#define	READY_TIME  1000
#define	READY_TIMEOUT  15000
#define	CALLSCORE_TIME  2000
#define	ROBLORD_TIME  2000
#define	PLARCARD_TIME  3000
#define	LEAVETABLE_TIME  2000
#define	LOGIN_TIME  20000
#define	PLARCARD_TIME_MIN  1000
#define	PLARCARD_TIME_MAX  4000

CConfigManager::CConfigManager(void)
{
	m_nTableID = 0;
	g_robot_type_ = 0;
	g_rule_type_ = 0;
	g_debug_level_ = 0;
	g_chat_time_ = CHAT_TIME;
	g_ping_time_ = PING_TIME;
	g_ready_time_ = READY_TIME;
	g_ready_timeout_ = READY_TIMEOUT;
	g_callscore_time_ = CALLSCORE_TIME;
	g_roblord_time_ = ROBLORD_TIME;
	g_playcard_time_ = PLARCARD_TIME;
	g_leavetable_time_ = LEAVETABLE_TIME;
	g_login_time_ = LOGIN_TIME;
	g_test_robot_ = 0;
	g_user_robot_ = 0;
	g_need_userrobot_ =0;
	g_max_table_num = 0;
	g_playcard_time_min_ = PLARCARD_TIME_MIN;
	g_playcard_time_max_ = PLARCARD_TIME_MAX;
}

CConfigManager::~CConfigManager(void)
{
}

bool CConfigManager::Init(const char* m_szConfigFile)
{
	char buffer[256];

	TiXmlDocument doc(m_szConfigFile);
	if(!doc.LoadFile())
	{
		glog.log(doc.ErrorDesc());
		return false;
	}
	TiXmlElement* pElement = NULL;
	TiXmlElement* pSub = NULL;
	pElement = doc.FirstChildElement("config");
	if(!pElement)
		return false;
	strncpy(m_szGSListenAddr, pElement->Attribute("listenip"), sizeof(m_szGSListenAddr));
	pElement->Attribute("listenport", &m_nGSListenPort);

	//pElement->Attribute("firstroomid", &m_nFirstRoomID);
	//pElement->Attribute("roomnum", &m_nRoomID);
	//pElement->Attribute("firsttableid", &m_nTableID);
	//pElement->Attribute("lasttableid", &m_nLastID);
	//robot_additional_rules_ = pElement->Attribute("additional_rule_value");
	pElement->Attribute("plynum", &m_nPlyNum);
	pElement->Attribute("robot_type", &g_robot_type_);
	pElement->Attribute("RuleType",&g_rule_type_);
	pElement->Attribute("DebugLevel",&g_debug_level_);

	pElement->Attribute("ChatTime", &g_chat_time_);
	pElement->Attribute("PingTime", &g_ping_time_);
	pElement->Attribute("ReadyTime", &g_ready_time_);
	pElement->Attribute("ReadyTimeOut",&g_ready_timeout_);
	pElement->Attribute("CallScoreTime",&g_callscore_time_);
	pElement->Attribute("RobLordTime",&g_roblord_time_);
	if (!pElement->Attribute("PlayCardTimeMin", &g_playcard_time_min_)) {
		g_playcard_time_min_ = PLARCARD_TIME_MIN;
	}
	if (!pElement->Attribute("PlayCardTimeMax", &g_playcard_time_max_)) {
		g_playcard_time_max_ = PLARCARD_TIME_MAX;
	}
	pElement->Attribute("LeaveTableTime",&g_leavetable_time_);
	pElement->Attribute("TestRobot",&g_test_robot_);
	pElement->Attribute("UserRobot",&g_user_robot_);
	pElement->Attribute("BetterForPlayer",&g_nBetterForPlayer);
	pElement->Attribute("NotCallScore",&g_nNotCallScore);
	
	

	if (!pElement->Attribute("Max_Table_Num", &g_max_table_num))
		g_max_table_num = 3;

	strncpy(buffer, pElement->Attribute("robotagentid"), sizeof(buffer));
	if (g_test_robot_ == 1 || g_user_robot_ == 1 )
	{
		buffer[1] = '8';
	}
	m_robot_agent_ = strtoll(buffer, NULL, 10);

	strncpy(buffer, pElement->Attribute("robotbeginid"), sizeof(buffer));
	if (g_test_robot_ == 1 || g_user_robot_ == 1 )
	{
		buffer[1] = '8';
	}
	m_robot_begin_id_ = strtoll(buffer, NULL, 10);

	if (g_user_robot_ == 1 )
	{
		g_need_userrobot_ = 1;
	}

	return true;
}

bool CConfigManager::parseAddtionalRule()
{
	if(robot_additional_rules_.empty())
	{
		robot_additional_rule_map_.clear();
		return false;
	}

	// 服务器费
	string map_key,map_value;
	char c_rule[1024]={0};
	strcpy(c_rule,robot_additional_rules_.c_str());
	const char *child_content=strtok(c_rule,"|");
	while(child_content)
	{
		string s = string(child_content);
		size_t pos = s.find_first_of(":");
		if(pos==string::npos)
			continue;

		map_key = s.substr(0,pos);
		map_value = s.substr(pos+1);

		robot_additional_rule_map_.insert(pair< string,string >(map_key,map_value));

		child_content = strtok(NULL,"|");
	}

	return true;
}

const char* CConfigManager::getRuleValueByKey( const char* key )
{
	for(map<string,string>::iterator it=robot_additional_rule_map_.begin();it!=robot_additional_rule_map_.end();it++)
	{
		if(it->first == key)
		{
			return it->second.c_str();
		}
	}
	return 0;
}

