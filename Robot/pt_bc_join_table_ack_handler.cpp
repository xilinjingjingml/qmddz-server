#include "stdafx.h"
#include "pt_bc_join_table_ack_handler.h"
#include "UserSession.h"
#include "RobotManager.h"

pt_bc_join_table_ack_handler::pt_bc_join_table_ack_handler(void)
{
}


pt_bc_join_table_ack_handler::~pt_bc_join_table_ack_handler(void)
{
}

void pt_bc_join_table_ack_handler::handler( const pt_bc_join_table_ack& ack,CUserSession* session )
{
    LOG_INFO("pt_bc_join_table_ack_handler. ret:%d, s_uid:%lld", ack.ret_, session->ply_guid());
	if(ack.ret_==0)
	{
		Robot* robot = session->getRobot();
		//assert(robot);
		if (NULL == robot)
		{
			glog.log("gameServerce has quick!!!");
			return;
		}
		//assert(ack.table_attrs_.table_id_ == robot->table_id_);
		
		//if(session->isready_)
		//	return;

		bool is_need_ready = false;
		SERVER_LOG("--player size: %d", ack.table_attrs_.players_.size());
		for(size_t i = 0; i < ack.table_attrs_.players_.size(); ++i){
			
			if(ack.table_attrs_.players_[i].ply_guid_ == session->ply_guid())
			{
				SERVER_LOG("--guid: %lld, chair: %d", session->ply_guid(), ack.table_attrs_.players_[i].chair_id_);
				session->set_chair_id(ack.table_attrs_.players_[i].chair_id_);
			}
			else
			{
				session->players_.push_back(ack.table_attrs_.players_[i]);
				if(!Robot::isRobot(ack.table_attrs_.players_[i].ply_guid_) && ack.table_attrs_.players_[i].ready_ == 1){
					is_need_ready = true;
				}
			}
		}

		if(!Robot::isRobot(session->ply_guid()))
		{
			session->set_status(CUserSession::US_TABLE);
		}else{
			if(is_need_ready)
			{
				session->set_status(CUserSession::US_TABLE);
				//session->OnTable();
			}
		}
		
		if (NULL != robot->lord_robot_)
		{
			destoryRobot(robot->lord_robot_);
			robot->lord_robot_ = NULL;
		}

		robot->lord_robot_ = createRobot(robot->m_robot_type_, session->chair_id());
		
		robot->is_initvard_ = false;
		robot->laizi_ = -1;
		robot->is_auto_ = false;
		robot->lord_ = -1;
		robot->userchairID = -1;

		//SERVER_LOG("pt_bc_join_table_ack_handler end.");
	}
	else{
		//SERVER_LOG("--pt_bc_join_table_ack_handler. ret:%d.", ack.ret_);
		//session->set_status(CUserSession::US_LOBBY);

        session->CloseConnection();

#if 0
		Robot *robot = session->getRobot();
		if(!robot)
			return;

		RobotManager::GetInstancePtr()->pushRobot(robot);
#endif
		SERVER_LOG("@@--pt_bc_join_table_ack_handler.size:%d",RobotManager::GetInstancePtr()->size());
	}
}
