#include "stdafx.h"
#include "pt_br_need_send_robot_not_handler.h"
#include "UserSession.h"
#include "RobotManager.h"
pt_br_need_send_robot_not_handler::pt_br_need_send_robot_not_handler(void)
{
}


pt_br_need_send_robot_not_handler::~pt_br_need_send_robot_not_handler(void)
{
}

void pt_br_need_send_robot_not_handler::handler( const pt_br_need_send_robot_not& noti,CUserSession* session )
{
    LOG_INFO("-------------------------------------");
    LOG_INFO("--pt_br_need_send_robot_not_handler--:%d,tableid:%d",RobotManager::GetInstancePtr()->size(),noti.table_id_);
    LOG_INFO("-------------------------------------");
	//session->set_status(CUserSession::US_ROBOT);

	Robot *ready_join = RobotManager::GetInstancePtr()->popRobot();
	if(!ready_join)
		return;

	ready_join->table_id_ = noti.table_id_;
	SERVER_LOG("need_send_robot_not table id:%d",noti.table_id_);
	ready_join->getBindSession()->OnSendRobot();
}
