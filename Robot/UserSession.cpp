#include "stdafx.h"
#include "UserSession.h"
#include "RobotManager.h"
#include "ConfigManager.h"
#include "pt_bc_login_ack_handler.h"
#include "pt_bc_join_table_ack_handler.h"
#include "pt_bc_leave_table_ack_handler.h"
#include "pt_bc_ready_not_handler.h"
#include "pt_bc_ply_join_not_handler.h"
#include "pt_bc_ply_leave_not_handler.h"
#include "pt_br_need_send_robot_not_handler.h"
#include "pt_gc_common_not_handler.h"
#include "pt_gc_refresh_card_not_handler.h"
#include "pt_gc_game_start_not_handler.h"
#include "pt_gc_call_score_req_handler.h"
#include "pt_gc_rob_lord_req_handler.h"
#include "pt_gc_show_card_req_handler.h"
#include "pt_gc_play_card_req_handler.h"
#include "pt_gc_play_card_not_handler.h"
#include "pt_gc_game_result_not_handler.h"
#include "pt_gc_complete_data_not_handler.h"
//#include "pt_gc_get_card_ack_handler.h"
#include "pt_gc_lord_card_not_handler.h"
#include "pt_gc_laizi_not_handler.h"
#include "pt_gc_auto_not_handler.h"
#include "pt_gc_double_score_req_handler.h"
#include "pt_gc_two_lord_card_not_handler.h"
CUserSession::CUserSession(void)
{
	status_ = US_UNCONNECTED;
	connection_ = NULL;
	connector_ = g_pNetModule->CreateConnector();
	connector_->SetSession(this);
	dis_time_ = leaf::GetCurTime();

	first_join_ = 0;
	robot_ = NULL;
	isready_ = 0;
	lastElapse_ = 0;
	chat_time_ = 0;
	ndelat_ = 0;
	table_time_ = 0;
	login_timer_ = 0;
	ready_time_ = 0;
	callscore_time_ = 0;
	roblord_time_ = 0;
	playcard_time_ = 0;
	leavetable_time_=0;
	players_.clear();
    reconnect_time_ = 0;
}


CUserSession::~CUserSession(void)
{
	BD_SAFE_RELEASE(connector_);
	if (robot_)
	{
		glog.log("CUserSession::~CUserSession(void): %p",robot_);
		//delete robot_;
	}
}

void CUserSession::OnConnection( IBDConnection* pConnection )
{
	connection_ = pConnection;
	status_ = US_CONNECTED;
}

void CUserSession::OnDisConnect()
{
	status_ = US_UNCONNECTED;
	dis_time_ = leaf::GetCurTime();
}

void CUserSession::OnDisConnection()
{
	status_ = US_UNCONNECTED;
	dis_time_ = leaf::GetCurTime();
}

void CUserSession::OnRecv( const char* pData, int nLen )
{
	CInputStream is(pData, nLen);
	short opcode;
	is >> opcode;
	//glog.log("opcode [%d]", opcode);
	is.Reset();
	
	switch(opcode)
	{
		GAME_MESSAGE_HANDLER(bc_login_ack);
		GAME_MESSAGE_HANDLER(bc_join_table_ack);
		GAME_MESSAGE_HANDLER(bc_leave_table_ack);
		GAME_MESSAGE_HANDLER(bc_ready_not);
		GAME_MESSAGE_HANDLER(bc_ply_leave_not);
		GAME_MESSAGE_HANDLER(bc_ply_join_not);
		GAME_MESSAGE_HANDLER(br_need_send_robot_not);

	default:
		dispatchOpcode(opcode,is);
	}
	
}

void CUserSession::dispatchOpcode(short opcode,CInputStream& is)
{

	switch(opcode)
	{

		GAME_MESSAGE_HANDLER(gc_game_start_not);
		GAME_MESSAGE_HANDLER(gc_game_result_not);
		GAME_MESSAGE_HANDLER(gc_call_score_req);
		GAME_MESSAGE_HANDLER(gc_show_card_req);
		GAME_MESSAGE_HANDLER(gc_play_card_req);
		GAME_MESSAGE_HANDLER(gc_refresh_card_not);
		GAME_MESSAGE_HANDLER(gc_rob_lord_req);
		GAME_MESSAGE_HANDLER(gc_play_card_not);
		GAME_MESSAGE_HANDLER(gc_common_not);
		GAME_MESSAGE_HANDLER(gc_complete_data_not);
		//GAME_MESSAGE_HANDLER(gc_get_card_ack);
		GAME_MESSAGE_HANDLER(gc_lord_card_not);
		GAME_MESSAGE_HANDLER(gc_laizi_not);
		GAME_MESSAGE_HANDLER(gc_auto_not);
		GAME_MESSAGE_HANDLER(gc_double_score_req);
		GAME_MESSAGE_HANDLER(gc_two_lord_card_not);
		
	}
}

void CUserSession::OnRecvFrom( struct sockaddr* remoteaddr, const char* pData, int nLen )
{

}

void CUserSession::Release()
{

}

void CUserSession::CloseConnection()
{
	connection_->CloseConnection();
}

void CUserSession::Send( const char* pData, int nLen )
{
	assert(connection_);
	connection_->Send(pData,nLen);
}

void CUserSession::Connect()
{
	if(leaf::GetCurTime() - dis_time_ > 1000){
		status_ = US_PROCESS;
		connector_->Connect(server_addr_.c_str(),server_port_);
	}
}

void CUserSession::SetConnectInfo( const string& strAddr, int nPort, guid nPID )
{
	server_addr_ = strAddr;
	server_port_ = nPort ;
	user_pid_ = nPID;
}

void CUserSession::OnLoop( int ndelat )
{
	//glog.log("CUserSession::OnLoop delat: %d", ndelat);
	ndelat_ = ndelat;

	switch(status_)
	{
	case US_PROCESS:		break;
	case US_UNCONNECTED:	OnUnConnect();	break;
	case US_CONNECTED:		OnConnect();	break;
	case US_LOBBY:
		{
			//glog.log("--OnLogin--");
			OnLogin();
			break;
		}
	case US_TABLE:
		{
			//glog.log("--OnTable--");
			OnTable();
			break;
		}
	case US_READY:
		{
			//glog.log("--OnReady--");
			//OnReady();
			break;
		}
	case US_CALL:
		{
			//glog.log("--OnCallScore--");
			OnCallScore();
			break;
		}
	case US_ROB:
		{
			//glog.log("--OnRobLord--");
			OnRobLord();
			break;
		}
	case US_PLAY:
		{
			//glog.log("--OnPlayCard--");
			OnPlayCard();
			break;
		}
	case US_RESULT:
		{
			//glog.log("--OnResult--");
			OnResult();
			break;
		}
	case US_Double_Socore:
		{
			 DoubleSoce();
			 break;
		}
	case US_RACING:			OnRacing();		break;
	case US_INVITE:			OnInvite();		break;
	case US_ROBOT:			OnSendRobot();  break;
	default:
		break;
	}

	lastElapse_ += ndelat;
	if(status_ >= US_CONNECTED && lastElapse_ >= g_ping_time_){
		lastElapse_ = 0;
		SendPingPacket();
	}
    if(g_chat_time_ != 0)
    {
        chat_time_ += ndelat;
        if(isready_ == 1 && chat_time_ >= g_chat_time_)
        {
            chat_time_ = 0;
            if (rand()%2)
            {
                OnChat();
            }
        }
    }
}

void CUserSession::OnUnConnect()
{
    if (reconnect_time_ > time(0))
    {
        return;
    }
    reconnect_time_ = time(0) + 5;
	Connect();
}

void CUserSession::OnConnect()
{
	pt_cb_login_req req ;
	req.opcode = cb_login_req;
	req.ply_guid_ = user_pid_;
	
	// use new protocol
	req.version_ = SVN_VERSION;
	
	//sprintf(szbuffer,64,"%u",req.nPID);
	SendPacket(req);

	glog.log("--OnConnect send cb_login_req.");

	status_ = US_PROCESS;
}

void CUserSession::set_playcardtime(int value /*= -1*/)
{
	if (value == -1) {
		playcard_time_ = g_playcard_time_min_ + rand() % (g_playcard_time_max_ - g_playcard_time_min_);
	}
	else {
		playcard_time_ = value;
	}
}

void CUserSession::SendPingPacket()
{
	pt_ping req;
	req.opcode = ping;
	req.now_ = leaf::GetCurTime();
	SendPacket(req);
}

void CUserSession::OnLogin()
{
	if (g_user_robot_ == 1 && g_need_userrobot_ == 0)
	{
		return;
	}

	login_timer_ += ndelat_;
	if (login_timer_ >= g_login_time_)
	{
		g_need_userrobot_ =0;
		login_timer_ = 0;
		pt_cb_join_table_req req; 
		req.opcode = cb_join_table_req;

		req.table_id_ = -1;
		SendPacket(req);
		glog.log("-----------------------------------------------");
		glog.log("--OnLogin send cb_join_table_req. %lld", user_pid_);
		glog.log("-----------------------------------------------");
		status_ = US_PROCESS;

		first_join_ ++;
	}
}

void CUserSession::OnTable()
{
	ready_time_ += ndelat_;
	if (ready_time_ >= g_ready_time_ )
	{
		ready_time_ = 0;
		pt_cb_ready_req req;
		req.opcode = cb_ready_req;
		SendPacket(req);
		//isready_ = 1;
		status_ = US_PROCESS;
		glog.log("--OnTable send cb_ready_req. %lld",  user_pid_);
	}
	
}

void CUserSession::OnRacing()
{
	//int ran = rand() % 10000;
	//if(ran < 1){
	//	CloseConnection();
	//	return;
	//}
}

void CUserSession::OnInvite()
{

}

void CUserSession::OnSendRobot()
{
	// notify to robot server, then send join table request.
	pt_cb_join_table_req req; 
	req.opcode = cb_join_table_req;

	req.table_id_ = robot_->table_id_;
	SendPacket(req);

	glog.log("--OnSendRobot send cb_join_table_req. table id:%d , %lld", robot_->table_id_, user_pid_);

	status_ = US_PROCESS;
}

void CUserSession::OnReady( )
{
	ready_time_ += ndelat_;
	if(ready_time_ >= g_ready_timeout_)
	{
		ready_time_ = 0;
		//pt_cb_change_table_req req;
		//req.opcode = cb_change_table_req;
		pt_cb_leave_table_req req;
		req.opcode = cb_leave_table_req;
		SendPacket(req);
		status_ = US_PROCESS;
		glog.log("--OnReady send cb_leave_table_req. %lld",user_pid_);
	}
}

void CUserSession::OnCallScore()
{
	callscore_time_ += ndelat_;
	if(callscore_time_ >= g_callscore_time_)
	{
		callscore_time_ = 0;

		pt_cg_call_score_ack ack;
		ack.opcode = cg_call_score_ack;
		ack.nSerialID = nSerialID_;
		ack.nScore = nScore_;

		SendPacket(ack);
		status_ = US_PROCESS;

		glog.log("--OnCallScore send cg_call_score_ack. %lld",user_pid_);
	}
}
void CUserSession::DoubleSoce()
{
	callscore_time_ += ndelat_;
	if (callscore_time_ >= g_callscore_time_)
	{
		callscore_time_ = 0;

		pt_cg_call_score_ack ack;
		ack.opcode = cg_double_score_ack;
		ack.nSerialID = nSerialID_;
		ack.nScore = nScore_;

		SendPacket(ack);
		status_ = US_PROCESS;

		glog.log("--OnCallScore send cg_call_score_ack. %lld", user_pid_);
	}
}
void CUserSession::OnRobLord()
{
	roblord_time_ += ndelat_;
	if(roblord_time_ >= g_roblord_time_)
	{
		
		roblord_time_ = 0;

		pt_cg_rob_lord_ack ack;
		ack.opcode = cg_rob_lord_ack;
		ack.cRob = cRob_;
		ack.nSerialID = nSerialID_;
		SendPacket(ack);

		status_ = US_PROCESS;

		glog.log("--OnRobLord send cg_rob_lord_ack. %lld",user_pid_);
	}
}

void CUserSession::OnResult()
{
	//glog.log("-- OnResult %lld", user_pid_);
	if (Robot::isRobot(user_pid_))
	{
		return;
	}
	leavetable_time_ += ndelat_;
	if(leavetable_time_ >= g_leavetable_time_)
	{
		leavetable_time_ = 0;
		if (rand()%2 == 0)
		{
			pt_cb_leave_table_req req;
			req.opcode = cb_leave_table_req;
			SendPacket(req);
		} 
		else
		{
			pt_cb_change_table_req req;
			req.opcode = cb_change_table_req;
			SendPacket(req);
		}
	}
}

void CUserSession::OnPlayCard( )
{
	// playcard_time_ += (ndelat_ * (1 + 1.0f * (rand() % 100) / 100));
	playcard_time_ -= ndelat_;
	if(playcard_time_ < 0)
	{
		set_playcardtime(0);

		pt_cg_play_card_ack ack;
		ack.opcode = cg_play_card_ack;
		ack.nSerialID = nSerialID_;
		ack.cTimeOut = 0; //1:超时出牌,0:正常出牌
		ack.vecCards = vecCards_;

		SendPacket(ack);

		status_ = US_PROCESS;

		//glog.log("--OnPlayCard send cg_play_card_ack. %lld",user_pid_);
	}
}

void CUserSession::OnChat( )
{
	// 不要说话，NO TALKING.
	return;

	string strName;
	char buff[64]={'\0'};
	if (rand()%2)
	{
		//sprintf(buff,"<A┃┃btn_chat_%d",rand()%18);
		sprintf(buff,"<A┃┃%d",((rand()%18) +1) );
		strName = buff;
	}else{
		//sprintf(buff,"<B┃┃chat_%d",rand()%5);
		sprintf(buff,"<B┃┃%d",((rand()%5)+1));
		strName = buff;
	}
	//glog.log("OnChat:%s",strName.c_str());
	pt_cb_chat_req req;
	req.opcode = cb_chat_req;
	req.type_ = 0;
	req.message_  = strName.c_str();

	SendPacket(req);

}

void CUserSession::set_status(int status){
	if (status == CUserSession::US_PLAY) {
		set_playcardtime();
	}
	status_ = status;
	//glog.log("pid[%lld], status[%d]", user_pid_, status);
}
void CUserSession::OnProcess(){

}

CUserSession* CUserSession::createSession()
{
	CUserSession *session = new CUserSession();
	return session;
}

void CUserSession::OnAuto(char cauto)
{
	pt_cg_auto_req req;
	req.opcode = cg_auto_req;
	req.cAuto = cauto;

	SendPacket(req);
	glog.log("--OnAuto send pt_cg_auto_req. cauto=%d",cauto);
}


void CUserSession::OnGetCard()
{/*
	// notify to robot server, then send card request.
	pt_cg_get_card_req req; 
	req.opcode = cg_get_card_req;
	req.nSerialID = nSerialID_;
	SendPacket(req);

	glog.log("--OnGetCard send pt_cb_get_card_req. %lld,table id:%d", user_pid_,robot_->table_id_);

	status_ = US_PROCESS;
*/
}

void CUserSession::OnSendCardOk()
{
	g_need_userrobot_ = 1;
	// notify to robot server, then send card request.
	pt_cg_send_card_ok_ack ack; 
	ack.opcode = cg_send_card_ok_ack;
	ack.nSerialID = nSerialID_;

	SendPacket(ack);

	//glog.log("--OnSendCardOk send pt_cg_send_card_ok_ack. %lld,table id:%d", user_pid_,robot_->table_id_);

	status_ = US_PROCESS;
}

uint CUserSession::get_play_time(uint size)
{
	//glog.log("--get_play_time time:%d", size);
	if (size < 400 || size > 0)
	{
		return 400000 - 1000*size ;
	} 
	return 100000;
	//return 1;
	if (size < 3)
	{
		return 1;
	}
	if (size < 5)
	{
		return 1;
	}
	if (size < 9)
	{
		return 2;
	} 
	if (size < 12)
	{
		return 3;
	}
	if (size < 15)
	{
		return 4;
	}
	else{
		return 5;
	}
}



