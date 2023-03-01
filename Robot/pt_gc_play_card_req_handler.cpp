#include "stdafx.h"
#include "ConfigManager.h"
#include "pt_gc_play_card_req_handler.h"
#include "RobotManager.h"
#include "UserSession.h"

pt_gc_play_card_req_handler::pt_gc_play_card_req_handler(void)
{
}


pt_gc_play_card_req_handler::~pt_gc_play_card_req_handler(void)
{
}

void pt_gc_play_card_req_handler::handler( const pt_gc_play_card_req& req, CUserSession* session )
{

	SERVER_LOG("pt_gc_play_card_req_handler:cAuto = %d, guid = %lld",req.cAuto, session->ply_guid());

	Robot* robot = session->getRobot();
	session->nSerialID_ = req.nSerialID;
	if ((NULL == robot) || (NULL == robot->lord_robot_)|| (!robot->is_initvard_))
	{
		return;
	}
	robot->nWaitOpChairID = session->chair_id();
	if (1 == req.cAuto || (true == robot->is_auto_))
	{
		//session->OnAuto(0);
		return;
	}

	uint curTime = leaf::GetCurTime();

	robot->lord_robot_->outs[0] = -1;
	robot->lord_robot_->laizi[0] = -1;

	bool mustBomb = g_nMustBomb == 1 && (session->chair_id() == robot->lord_ || (session->chair_id() + robot->lord_robot_->game.pre_playernum + 1) % 3 == robot->lord_);
	if(takeOut(robot->lord_robot_, robot->lord_robot_->outs, robot->lord_robot_->laizi, mustBomb) == 0)
	{
		robot->lord_robot_->outs[0] = -1;
		SERVER_LOG("pt_gc_play_card_req_handler:pass");
	}

	curTime = leaf::GetCurTime() - curTime;
	SERVER_LOG("pt_gc_play_card_req_handler: time=%u", curTime);

	session->vecCards_ = robot->converCardToServer(robot->lord_robot_->outs, robot->lord_robot_->laizi, robot->laizi_);

	robot->nWaitOpChairID++;


	int i, size = session->vecCards_.size();
	SERVER_LOG("%lld takeout Auto:%d, size:%d",session->ply_guid(), req.cAuto, size);
	for (i=0; i<size; i++ )
	{
		SERVER_LOG("m_nValue = %d, m_Baovalue = %d ", session->vecCards_[i].m_nValue, session->vecCards_[i].m_nCard_Baovalue);
	}
	
	session->set_status(CUserSession::US_PLAY);
}
