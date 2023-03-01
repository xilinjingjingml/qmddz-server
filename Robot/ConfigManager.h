#ifndef _CONFIGMANAGER_H_
#define _CONFIGMANAGER_H_

#define RuChangJin "ruchangjin"

extern int g_debug_level_;
extern int g_chat_time_;
extern int g_ping_time_;
extern int g_ready_time_;
extern int g_ready_timeout_;
extern int g_callscore_time_;
extern int g_roblord_time_;
extern int g_playcard_time_;
extern int g_playcard_time_min_;
extern int g_playcard_time_max_;
extern int g_leavetable_time_;
extern int g_login_time_;
extern int g_robot_type_; // 机器人等级类型;
extern int g_rule_type_; // 游戏规则，赖子，非赖子;
extern int g_test_robot_;
extern int g_user_robot_;
extern int g_need_userrobot_; //需要机器人玩家进入，只有当g_user_robot_=1时有效
extern int g_max_table_num;
extern int g_nBetterForPlayer; //对玩家友好，当自己是地主，变为等级0; 当自己是农民，玩家是地主时，变为等级0
extern int g_nNotCallScore;
class CConfigManager
{
public:
	SINGLETON(CConfigManager);
	CConfigManager(void);
	~CConfigManager(void);
	bool Init(const char* m_szConfigFile);
	const char* GetGSListenIP() { return m_szGSListenAddr; }
	int GetGSListenPort() { return m_nGSListenPort; }
	int GetFirstRoomID() {return m_nFirstRoomID;}
	int GetRoomNum() { return m_nRoomID; }
	int GetPlyNum() { return m_nPlyNum; }
	int GetTableID() {return m_nTableID; }
	int GetLastID() {return m_nLastID;}

	guid GetRobotAgent() { return m_robot_agent_; }
	guid GetRobotBeginID() { return m_robot_begin_id_; }

	int getRobotType(){return g_robot_type_;}
	int getRuleType(){return g_rule_type_;}
	int getDebugLevel(){return g_debug_level_;}
	bool parseAddtionalRule();

	const char* getRuleValueByKey(const char* key);
private:
	//网络设置
	char m_szGSListenAddr[64];
	int m_nGSListenPort;
	int m_nRoomID;
	int m_nFirstRoomID;
	int m_nPlyNum;
	int m_nTableID;
	int m_nLastID;
	
	guid m_robot_agent_;
	guid m_robot_begin_id_;

//////////////////////////////////////////////////////////////////////////
	/*******************************
	**
	**	@version	v0.1;	
	**	@author	liuhongbin;
	**	@date	2013-6-26;
	**	@content	快速场机器人;
	**
	********************************/

	// 机器人附加配置;比如：入场金默认值;
	string robot_additional_rules_;
	map< string,string > robot_additional_rule_map_;
};
#endif
