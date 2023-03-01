#pragma once
#define GAME_MESSAGE_HANDLER(opcode) \
	case opcode: \
{ \
	pt_##opcode packet; \
	is >> packet; \
	pt_##opcode##_handler::handler(packet, this); \
	break;	\
}

#define SVN_VERSION	16488
//#define SVN_VERSION 1400730000

class RobotPlayCard;
class Robot;
class CUserSession : public IBDSession
{
public:
	enum USER_STATUS
	{
		US_UNCONNECTED,		///< 0	未连接
		US_CONNECTED,		///< 1	连接成功
		US_LOBBY,			///< 2	登录成功
		US_TABLE,			///< 3	玩家进入房间
		US_VISITOR,			///< 4	旁观进入房间
		US_READY,			///< 5	举手
		US_RACING,			///< 6	比赛中
		US_BROKEN,			///< 7	断线中
		US_INVITE,          ///< 8  被邀请 
		US_ROBOT,		    ///< 9  机器人
		US_CARD,            ///<10  发牌
		US_CARD_OK,
		US_CALL,            ///<11  叫分
		US_ROB,             ///<12  抢地主
		US_START,
		US_PLAY,            ///出牌
		US_RESULT,          ///<13  结束
		US_Double_Socore,   ///<14  加倍
		US_PROCESS = 100,
	};
public:
	CUserSession(void);
	~CUserSession(void);

	virtual void OnConnection(IBDConnection* pConnection);
	virtual void OnDisConnect();
	virtual void OnDisConnection();
	virtual void OnRecv(const char* pData, int nLen);
	virtual void dispatchOpcode(short opcode,CInputStream& is);
	virtual void OnRecvFrom(struct sockaddr* remoteaddr, const char* pData, int nLen);
	virtual void Release();
	void CloseConnection();
	template<class T>
	void SendPacket(const T& packet)
	{
		COutputStream os;
		os << packet;
		if(connection_)
			connection_->Send(os.GetData(), os.GetLength());
	}
	void Send(const char* pData, int nLen);

	void Connect();
	void SetConnectInfo(const string& strAddr, int nPort, guid nPID);
	void OnLoop(int ndelat);
	void OnUnConnect();
	void OnConnect();
	void OnLogin();
	void OnTable();
	void OnReady();
	void OnCallScore();
	void DoubleSoce();
	void OnRobLord();
	void OnPlayCard();
	void OnRacing();
	void OnInvite();
	void OnGetCard();
	void OnSendCardOk();
	void OnResult();
	void OnAuto(char cauto);
	void OnChat();
	// new
	void OnSendRobot();

	void set_status(int status);
	int status(){return status_;}
	void OnProcess();
	void set_ready_time(int time) { ready_time_ = time; }
	void set_chair_id(int id){chair_id_ = id;}
	int chair_id() { return chair_id_; }
	void set_table_id(int id){table_id_ = id;}
	int table_id(){return table_id_;}	
	guid ply_guid() { return user_pid_; }
	uint get_play_time(uint size);
	void setRobot(Robot *robot){robot_ = robot;}
	Robot *getRobot(){return robot_;}
	void set_playcardtime(int value = -1);
	void SendPingPacket();
	void set_chat_time(int time) { chat_time_ = time; }

	/*
	**	获取牌型;
	*/
	int getCardType();

	static CUserSession* createSession();
private:
	IBDConnection* connection_;
	IBDConnector* connector_;
	int	status_;
	int pre_status_;

	string server_addr_;
	int	server_port_;
	guid user_pid_;
	int	room_id_;
	int chair_id_;
	int table_id_;
	int ndelat_;
	int dis_time_;
	int table_time_;
	int	login_timer_;
	int ready_time_;
	int callscore_time_;
	int roblord_time_;
	int playcard_time_;
	int leavetable_time_;
	int lastElapse_;			// 统计时间，用来发送心跳包
	int chat_time_;
	Robot *robot_;

public:

	/*
	**	游戏规则;
	*/

	int first_join_;			// 第一次进桌


	vector<PlyBaseData>	players_;
	vector<CCard> vecCards_;
	int nSerialID_;
	int isready_;
	int nScore_;
	char cRob_;
    int reconnect_time_;
	bool bBetterForPlayer;
};
