#include "stdafx.h"
#include "pt_gc_lord_card_not_handler.h"
#include "RobotManager.h"
#include "UserSession.h"
#include "ConfigManager.h"

pt_gc_lord_card_not_handler::pt_gc_lord_card_not_handler(void)
{
}

pt_gc_lord_card_not_handler::~pt_gc_lord_card_not_handler(void)
{
}

void pt_gc_lord_card_not_handler::handler( const pt_gc_lord_card_not& noti, CUserSession* session )
{
	int i, chair, size=0;
	SERVER_LOG("pt_gc_lord_card_not_handler:cLord = %d, chair_id=%d",noti.cLord, session->chair_id());

	Robot* robot = session->getRobot();

	if ((NULL == robot) || (NULL == robot->lord_robot_)|| (!robot->is_initvard_))
	{
		return;
	}

	robot->converCardToRobot(noti.vecCards, robot->lord_robot_->pot, NULL, -1);
	//assert(!robot->checkCardIsOk());
	chair = session->chair_id();
	if (noti.cLord == chair)
	{
		beLord(robot->lord_robot_, robot->lord_robot_->pot, CUR_PLAYER);
		robot->lord_ = chair;
	}else{
		if (noti.cLord == ((chair + 1)%3))
		{
			beLord(robot->lord_robot_, robot->lord_robot_->pot, DOWN_PLAYER);
		}else{
			beLord(robot->lord_robot_, robot->lord_robot_->pot, UP_PLAYER);
		}

		for(size_t i = 0; i < session->players_.size(); ++i)
		{
			SERVER_LOG("%lld,chair_id_:%d",session->players_[i].ply_guid_,session->players_[i].chair_id_);
			if((Robot::isRobot(session->players_[i].ply_guid_))&&(session->players_[i].chair_id_ == noti.cLord)){
				setRobotLevel(robot->lord_robot_, 0);
				break;
			}
		}
		size = noti.vecCards.size();
		for (i=0; i<size; i++ )
		{
			SERVER_LOG("m_nValue = %d, m_nColor = %d ", noti.vecCards[i].m_nValue, noti.vecCards[i].m_nColor);
		}
	}

	//session->players_ 里面保存了除了自己的玩家
	if (g_nBetterForPlayer == 1)
	{
		int robotNum = 0;
		for(size_t i = 0; i < session->players_.size(); ++i)
		{
			if (Robot::isRobot(session->players_[i].ply_guid_))
			{
				robotNum++;
			}
		}

		if (robotNum == 1)
		{
			if (noti.cLord == chair) {
				setRobotLevel(robot->lord_robot_, 0);
			}else {

				for(size_t i = 0; i < session->players_.size(); ++i)
				{
					if (Robot::isRobot(session->players_[i].ply_guid_))
					{
						if ((session->players_[i].chair_id_ == noti.cLord))
						{
							setRobotLevel(robot->lord_robot_, 3);
							break;
						}else{
							setRobotLevel(robot->lord_robot_, 0);
							break;
						}
					}
				}
			}
		}
	}

	/*
	i = check_all_card_is_ok(robot->lord_robot_);
	if (0 != i)
	{
		g_pLogger->Log("check_all_card_is_ok:%d",i);
	}
	*/
	
}
