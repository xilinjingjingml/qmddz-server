#include "stdafx.h"
#include "pt_gc_call_score_req_handler.h"
#include "RobotManager.h"
#include "UserSession.h"
#include "ConfigManager.h"

pt_gc_call_score_req_handler::pt_gc_call_score_req_handler(void)
{
}


pt_gc_call_score_req_handler::~pt_gc_call_score_req_handler(void)
{
}

void pt_gc_call_score_req_handler::handler(const pt_gc_call_score_req& req, CUserSession* session)
{
	SERVER_LOG("pt_gc_call_score_req_handler:nScore = %d, guid:%lld", req.nScore, session->ply_guid());

	Robot* robot = session->getRobot();
	int nScore = req.nScore;
	session->nSerialID_ = req.nSerialID;

	if (NULL == robot || NULL == robot->lord_robot_ || (!robot->is_initvard_))
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

	if (nScore == 0) //0:不叫,1:1分,2:2分,3:3分
	{
		nScore = -1;//不叫
	}

	// 百元赛叫分逻辑
	if (req.nCallMode == 3) {
		if (nScore == 8) {
			nScore = 3;
		}
		else if (nScore == 4) {
			nScore = 1;
		}
	}

	getPlayerCallScore(robot->lord_robot_, nScore, DOWN_PLAYER);
	getPlayerCallScore(robot->lord_robot_, nScore, UP_PLAYER);
	nScore = callScore(robot->lord_robot_);

	
	// 百元赛叫分逻辑
	if (req.nCallMode == 3) {
		if (nScore == 3) {
			nScore = 8;
		} else if (nScore == 1 || nScore == 2){
			nScore = 4;
		}
	}
	
	if(nScore == -1)
	{
		nScore = 0;
	}

	if (nScore > 0 && g_nNotCallScore > 0)
	{
		if ((rand()%100 ) < g_nNotCallScore)
		{
			nScore = 0;
		}		
	}


	curTime = leaf::GetCurTime() - curTime;
	SERVER_LOG("cg_call_score_handler:  score=%d,time=%d", nScore, curTime);
	session->nScore_ = nScore;
	session->set_status(CUserSession::US_CALL);
}
