#include "stdafx.h"
#include "pt_gc_game_result_not_handler.h"
#include "RobotManager.h"
#include "UserSession.h"

pt_gc_game_result_not_handler::pt_gc_game_result_not_handler(void)
{
}

pt_gc_game_result_not_handler::~pt_gc_game_result_not_handler(void)
{
}

void pt_gc_game_result_not_handler::handler( const pt_gc_game_result_not& noti,CUserSession* session )
{
	SERVER_LOG("pt_gc_game_result_not_handler:bType = %d, cDouble=%d",noti.bType, noti.cDouble);

	Robot* robot = session->getRobot();
	if (noti.bType == 1) //
	{
		if (robot->lord_ == session->chair_id())
		{
			robot->win_++; 
			if ((g_debug_level_&0x4) == 4)
			{
				g_pLogger->Log("%lld is Lord winer win:%d-lost:%d,CallScore:%d,RobLord:%d,Bomb:%d,Double:%d,Spring:%d-:%d",session->ply_guid() , robot->win_, robot->lost_,noti.cCallScore,noti.bRobLord,noti.nBombCount,noti.cDouble,noti.bSpring,noti.bReverseSpring);
			}
		}else{
			robot->lost1_++; 
			if ((g_debug_level_&0x4) == 4)
			{
				g_pLogger->Log("%lld is Farmer loser win1:%d-lost1:%d,CallScore:%d,RobLord:%d,Bomb:%d,Double:%d,Spring:%d-:%d",session->ply_guid() , robot->win1_, robot->lost1_,noti.cCallScore,noti.bRobLord,noti.nBombCount,noti.cDouble,noti.bSpring,noti.bReverseSpring);
			}
		} 

	}else if (noti.bType == 2)
	{
		if (robot->lord_ == session->chair_id())
		{
			robot->lost_++;
			if ((g_debug_level_&0x4) == 4)
			{
				g_pLogger->Log("%lld is Lord loser win:%d-lost:%d,CallScore:%d,RobLord:%d,Bomb:%d,Double:%d,Spring:%d-:%d",session->ply_guid() , robot->win_, robot->lost_,noti.cCallScore,noti.bRobLord,noti.nBombCount,noti.cDouble,noti.bSpring,noti.bReverseSpring);
			}
		}else{
			robot->win1_++;
			if ((g_debug_level_&0x4) == 4)
			{
				g_pLogger->Log("%lld is Farmer winer win1:%d-lost1:%d,CallScore:%d,RobLord:%d,Bomb:%d,Double:%d,Spring:%d-:%d",session->ply_guid() , robot->win1_, robot->lost1_,noti.cCallScore,noti.bRobLord,noti.nBombCount,noti.cDouble,noti.bSpring,noti.bReverseSpring);
			}
		}
	}
	session->players_.clear();
	session->isready_ = 0;
	if ((NULL != robot) && (NULL != robot->lord_robot_))
	{
		if ((g_debug_level_&0x8) == 8 && noti.cDouble >=96)
		{
			print_history_content(robot->lord_robot_, NULL);
			//print_struct_content( robot->lord_robot_->game.rec.history, sizeof(HISTORY_HAND));
		}
		if ((g_debug_level_&0x10) == 16)
		{
			print_history_content2(robot->lord_robot_, NULL);
			//print_struct_content( robot->lord_robot_->game.rec.history, sizeof(HISTORY_HAND));
		}
		destoryRobot(robot->lord_robot_);
		robot->lord_robot_ = NULL;
		
		
	}

	robot->is_initvard_ = false;
	robot->laizi_ = -1;
	robot->is_auto_ = false;
	robot->lord_ = -1;
	robot->userchairID = -1;
	//is this need?
	//usleep(1000000);
	//pt_cb_leave_table_req req;
	//req.opcode = cb_leave_table_req;
	//pt_cb_change_table_req req;
	//req.opcode = cb_change_table_req;
	//session->SendPacket(req);
	session->set_status(CUserSession::US_RESULT);
	
}
