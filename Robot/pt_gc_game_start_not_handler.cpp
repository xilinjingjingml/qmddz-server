#include "stdafx.h"
#include "pt_gc_game_start_not_handler.h"
#include "RobotManager.h"
#include "UserSession.h"


pt_gc_game_start_not_handler::pt_gc_game_start_not_handler(void)
{
}


pt_gc_game_start_not_handler::~pt_gc_game_start_not_handler(void)
{
}

void pt_gc_game_start_not_handler::handler( const pt_gc_game_start_not& noti,CUserSession* session )
{
	SERVER_LOG("pt_gc_game_start_not:nCardNum = %d, nlaizi = %d",noti.nCardNum, noti.cLordCard.m_nValue);
	SERVER_LOG("pt_gc_game_start_not:guid = %lld, chair_id = %d",session->ply_guid(),session->chair_id());

	session->nSerialID_ = noti.nSerialID;

	Robot* robot = session->getRobot();
	if (NULL == robot || NULL == robot->lord_robot_ || (!robot->is_initvard_))
	{
		return;
	}
	
	setLaizi(robot->lord_robot_, noti.cLordCard.m_nValue -3 );
	/*
	session->OnGetCard();

	Robot* robot = session->getRobot();
	if (NULL == robot)
	{
		return;
	}
	if (NULL != robot->lord_robot_)
	{
		destoryRobot(robot->lord_robot_);
		robot->lord_robot_ = NULL;
	}

	robot->lord_robot_ = createRobot(robot->m_robot_type_, session->chair_id());
	robot->is_initvard_ = false;
	robot->laizi_ = -1;
	*/
	session->set_status(CUserSession::US_START);
	if ((g_debug_level_&0x10) == 16)
	{
		print_game_record(robot->lord_robot_, NULL);
		//print_struct_content( robot->lord_robot_->game.rec.history, sizeof(HISTORY_HAND));
	}
}
