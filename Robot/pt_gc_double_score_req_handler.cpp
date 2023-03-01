#include "stdafx.h"
#include "pt_gc_double_score_req_handler.h"
#include "RobotManager.h"
#include "UserSession.h"

pt_gc_double_score_req_handler::pt_gc_double_score_req_handler()
{
}


pt_gc_double_score_req_handler::~pt_gc_double_score_req_handler()
{
}
void pt_gc_double_score_req_handler::handler(const pt_gc_double_score_req & req, CUserSession* session)
{
	SERVER_LOG("pt_gc_double_score_req_handler:lord = %d, guid:%lld", req.nSerialID, session->ply_guid());
	Robot* robot = session->getRobot();
	session->nSerialID_ = req.nSerialID;
	int lord = 0;

	if (NULL == robot || NULL == robot->lord_robot_)
	{
		return;
	}
	if (robot->is_auto_)
	{
		SERVER_LOG("cg_call_score_handler: timeout");
		//session->OnAuto(0);
		session->set_status(CUserSession::US_PROCESS);
		return;
	}

	if (!robot->is_initvard_)
	{
		return;
	}
	uint curTime = leaf::GetCurTime();
	//session->cRob_ = forceLord(robot->lord_robot_);
	session->nScore_ = 0;
	curTime = leaf::GetCurTime() - curTime;
	SERVER_LOG("pt_gc_rob_lord_req_handler: cRob=%d, time=%d", session->cRob_, curTime);
	session->set_status(CUserSession::US_Double_Socore);
	
}

