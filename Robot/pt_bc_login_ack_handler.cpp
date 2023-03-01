#include "stdafx.h"
#include "pt_bc_login_ack_handler.h"
#include "UserSession.h"
#include "RobotManager.h"
#include "ConfigManager.h"

pt_bc_login_ack_handler::pt_bc_login_ack_handler(void)
{
}


pt_bc_login_ack_handler::~pt_bc_login_ack_handler(void)
{
}

void pt_bc_login_ack_handler::handler( const pt_bc_login_ack& ack,CUserSession* session )
{
    if (ack.ret_ != 0)
    {
        LOG_INFO("pt_bc_login_ack.ret_:%d ,%lld ", ack.ret_, session->ply_guid());
    }
    else
    {
        glog.log("pt_bc_login_ack.ret_:%d ,%lld ", ack.ret_, session->ply_guid());
    }
	//int random = rand() % 10;
	if(ack.ret_==0)
	{

		//int ran = rand() % 10;
		//if(ran < 1){
		//	session->CloseConnection();
		//}
		//else{
			//session->set_status(CUserSession::US_LOBBY);
		//}

		//if(!Robot::isSpecialRobot(session->ply_guid()))
		{
			SERVER_LOG("--pt_bc_login_ack_handler. %lld", ack.ply_base_data_.ply_guid_);
			if (NULL == session->getRobot())
			{
				SERVER_LOG("--pt_bc_login_ack_handler. new Robot");
				Robot *robot = new Robot(session);
				robot->set_robot_type(g_robot_type_);
				robot->set_rule_type(g_rule_type_);
				robot->lord_robot_ = NULL;
				//robot->lord_robot_ = createRobot(robot->m_robot_type_, 0);
				RobotManager::GetInstancePtr()->pushRobot(robot);

				session->setRobot(robot);
				//for robottest
				if(!Robot::isRobot(session->ply_guid()))
				{
					session->set_status(CUserSession::US_LOBBY);
				}
				
			}
		}
	}
	else{
		SERVER_LOG("--pt_bc_login_ack_handler US_CONNECTED");

        session->CloseConnection();
		//session->set_status(CUserSession::US_CONNECTED);
	}
}
