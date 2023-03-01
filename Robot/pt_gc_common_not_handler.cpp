#include "stdafx.h"
#include "pt_gc_common_not_handler.h"
#include "RobotManager.h"
#include "UserSession.h"
#include "ConfigManager.h"

pt_gc_common_not_handler::pt_gc_common_not_handler(void)
{
}


pt_gc_common_not_handler::~pt_gc_common_not_handler(void)
{
}

void pt_gc_common_not_handler::handler( pt_gc_common_not& noti,CUserSession* session )
{
	SERVER_LOG("pt_gc_common_not_handler:nOp = %d, cChairID = %d, uid=%lld",noti.nOp, noti.cChairID, session->ply_guid());

	if (noti.nOp == CO_NOLORD)
	{
		Robot* robot = session->getRobot();
		if (NULL == robot)
		{
			return;
		}
		if (NULL != robot->lord_robot_)
		{
			//destoryRobot(robot->lord_robot_);
			//robot->lord_robot_ = NULL;
		}
		robot->is_initvard_ = false;
	}
	else if(noti.nOp == CO_GIVEUP)
	{		
		Robot* robot = session->getRobot();
		if (NULL == robot)
		{
			return;
		}
		if (true == robot->is_auto_)
		{
			return;
		}
		if (noti.cChairID != session->chair_id())
		{			
			if(NULL == robot || !robot->is_initvard_)
			{
				return;
			}
			int chair = session->chair_id();

			robot->lord_robot_->outs[0] = -1;
			robot->lord_robot_->laizi[0] = -1;
			if (g_max_table_num==2)
			{
				SERVER_LOG("pt_gc_common_not_handler:UP_PLAYER, s_ChairID =%d", session->chair_id());
				getPlayerTakeOutCards(robot->lord_robot_, robot->lord_robot_->outs, robot->lord_robot_->laizi, UP_PLAYER);
			}
			else
			{
				if (((chair + 1) % 3) == noti.cChairID)
				{
					SERVER_LOG("pt_gc_common_not_handler:DOWN_PLAYER, s_ChairID =%d", session->chair_id());
					getPlayerTakeOutCards(robot->lord_robot_, robot->lord_robot_->outs, robot->lord_robot_->laizi, DOWN_PLAYER);
				}
				else{
					SERVER_LOG("pt_gc_common_not_handler:UP_PLAYER, s_ChairID =%d", session->chair_id());
					getPlayerTakeOutCards(robot->lord_robot_, robot->lord_robot_->outs, robot->lord_robot_->laizi, UP_PLAYER);
				}
			}
		}
		else{
			if (true == robot->is_auto_)
			{
				int ret = 0;
				session->OnAuto(0);

				robot->lord_robot_->outs[0] = -1;
				robot->lord_robot_->laizi[0] = -1;
				SERVER_LOG("pt_gc_common_not_handler: CUR_PLAYER, s_ChairID =%d",session->chair_id());
				if( ret = userTakeOutAndTest(robot->lord_robot_, robot->lord_robot_->outs, robot->lord_robot_->laizi) != 0)
				{
					SERVER_LOG("------------auto pt_gc_common_not_handler CO_GIVEUP %d", ret);
				}
			}	
		}
	}
	
}
