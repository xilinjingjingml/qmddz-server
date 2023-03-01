#include "stdafx.h"
#include "pt_bc_ready_not_handler.h"
#include "UserSession.h"
#include "RobotManager.h"

pt_bc_ready_not_handler::pt_bc_ready_not_handler(void)
{
}


pt_bc_ready_not_handler::~pt_bc_ready_not_handler(void)
{
}

void pt_bc_ready_not_handler::handler( const pt_bc_ready_not& noti,CUserSession* session )
{
    LOG_INFO("bc_ready_not:%lld, session guid:%lld",noti.ply_guid_, session->ply_guid());

	if(noti.ply_guid_ == session->ply_guid())
	{
		session->set_status(CUserSession::US_READY);
		session->isready_ = 1;
	}
	else
	{
		bool is_need_ready = false;
		for(size_t i=0; i<session->players_.size(); i++)
		{
			if(noti.ply_guid_ == session->players_[i].ply_guid_)
			{
				session->players_[i].ready_ = 1;
			}
			if(!Robot::isRobot(session->players_[i].ply_guid_) && session->players_[i].ready_ == 1)
			{
				is_need_ready = true;
			}
		}

		if(is_need_ready && !session->isready_){
			session->set_status(CUserSession::US_TABLE);
			//session->OnReady(0);
		}
	}	
}
