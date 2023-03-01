#ifndef _ROBOTMANAGER_H_
#define _ROBOTMANAGER_H_

#include "UserSession.h"
#include "ddz_interface.h"
class Robot
{
public:
	enum RobotType
	{
		kRobotTypeNormal=0,
		kRobotTypeFast=1,
		kRobotTypeQuanmen=3,
	};
public:
	Robot(CUserSession* session)
	{
		connect_session_ = session;
		m_robot_type_ = kRobotTypeQuanmen;
		m_rule_type_ = 0;
		laizi_ = -1;
		lord_robot_ = NULL;
		is_auto_ = false;
		lord_ = -1;
		win_ = 0;
		lost_ = 0;
		win1_ = 0;
		lost1_ = 0;
		userchairID = -1;
		table_id_ = -1;
	}
	~Robot(){
		if (NULL != lord_robot_)
		{
			destoryRobot(lord_robot_);
		}
	}

	CUserSession* getBindSession(){return connect_session_;}

	static bool isSpecialRobot(guid pid);

	static bool isRobot(guid ply_guid_){
		char buffer[64];
		sprintf(buffer, "%lld", ply_guid_);

		if(buffer[0] == '9' && buffer[1] == '9'){
			return true;
		}else{
			return false;
		}
	}

	/*
	**	设置机器人类型;
	快速场机器人or全闷场机器人or普通场机器人;
	*/
	void set_robot_type(int rt)
	{
		if(rt == kRobotTypeNormal || rt == kRobotTypeFast || rt == kRobotTypeQuanmen )
			m_robot_type_ = rt;
	}
	void set_rule_type(int rt)
	{
		m_rule_type_ = rt;
	}
	/*
	**	解析服务器规则配置;
	*/
	void parse_addtional_rule(const string& rule);

	/*
	**	获取从本地tablerule map中获取指定规则的值;
	*/
	const char*		get_rule_value(const string& key);
	void SortCards(vector<CCard>& cards, int laizi);
	void converCardToRobot(vector<CCard> in, int* out, int* lz, int laizi);
	vector<CCard> converCardToServer(int* card);
	vector<CCard> converCardToServer(int* card, int len);
	vector<CCard> converCardToServer(int* card, int* lz, int laizi);
	int checkCardIsOk();
public:
	guid guid_;
	string nickname_;
	int table_id_;
	int m_robot_type_;
	int m_rule_type_;

	LordRobot* lord_robot_;
	bool is_auto_;
	int   laizi_;
	int   is_initvard_;
	int lord_;
	int win_;
	int lost_;
	int win1_;
	int lost1_;
	int   nWaitOpChairID;
	int userchairID;



	string m_additional_rule_string_;
	map<string,string> m_additional_rule_map_;
protected:
	CUserSession* connect_session_;
};

class RobotManager
{
public:
	SINGLETON(RobotManager);
	RobotManager(void){};
	~RobotManager(void){};

public:
	void pushRobot(Robot *robot);
	Robot *popRobot();
	int size();

private:
	leaf::CBDEventQueue<Robot *> robot_quene_;
};
#endif
