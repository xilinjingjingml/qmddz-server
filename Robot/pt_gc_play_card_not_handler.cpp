#include "stdafx.h"
#include "pt_gc_play_card_not_handler.h"
#include "RobotManager.h"
#include "UserSession.h"
#include "ConfigManager.h"
pt_gc_play_card_not_handler::pt_gc_play_card_not_handler(void)
{
}


pt_gc_play_card_not_handler::~pt_gc_play_card_not_handler(void)
{
}

void pt_gc_play_card_not_handler::handler( const pt_gc_play_card_not& noti,CUserSession* session )
{
	int chair, i, size = noti.vecCards.size();
	SERVER_LOG("pt_gc_play_card_not_handler:cChairID = %d, guid=%lld, session->chair_id=%d, size =%d", noti.cChairID, session->ply_guid(), session->chair_id(), size);

	if (size <= 0)
	{
		return;
	}

	Robot* robot = session->getRobot();

	if ((NULL == robot) || (NULL == robot->lord_robot_) || (!robot->is_initvard_))
	{
		return;
	}
	
	if (true == robot->is_auto_)
	{
		return;
	}
	if (noti.cChairID != session->chair_id())
	{
		chair = session->chair_id();
		robot->lord_robot_->outs[0] = -1;
		robot->lord_robot_->laizi[0] = -1;
		robot->converCardToRobot(noti.vecCards, robot->lord_robot_->outs, robot->lord_robot_->laizi, robot->laizi_);
		SERVER_LOG("cur player chair:%d", chair);
		if (g_max_table_num==2)
		{
			SERVER_LOG("pt_gc_play_card_not_handler:DOWN_PLAYER =%d", noti.cChairID);
			getPlayerTakeOutCards(robot->lord_robot_, robot->lord_robot_->outs, robot->lord_robot_->laizi, UP_PLAYER);
		}
		else if (((chair+1)%3) == noti.cChairID)
		{
			SERVER_LOG("pt_gc_play_card_not_handler:DOWN_PLAYER =%d",noti.cChairID);
			getPlayerTakeOutCards(robot->lord_robot_, robot->lord_robot_->outs, robot->lord_robot_->laizi, DOWN_PLAYER);
		}else{
			SERVER_LOG("pt_gc_play_card_not_handler:UP_PLAYER =%d",noti.cChairID);
			getPlayerTakeOutCards(robot->lord_robot_, robot->lord_robot_->outs, robot->lord_robot_->laizi, UP_PLAYER);
		}
	}
	else if (true == robot->is_auto_)
	{
		uint curTime = leaf::GetCurTime();
		int ret = 0;
		session->OnAuto(0);

		if(robot->nWaitOpChairID != noti.cChairID)
		{
			return;
		}

		robot->lord_robot_->outs[0] = -1;
		robot->lord_robot_->laizi[0] = -1;
		robot->converCardToRobot(noti.vecCards, robot->lord_robot_->outs, robot->lord_robot_->laizi, robot->laizi_);
		for (int i=0; i<5;i++)
		{
			SERVER_LOG("------------auto cards laizi : %d",robot->lord_robot_->laizi[i]);
		}
		for (int i=0; i<size;i++)
		{
			SERVER_LOG("------------auto cards out : %d",robot->lord_robot_->outs[i]);
		}

		if( ret = userTakeOutAndTest(robot->lord_robot_, robot->lord_robot_->outs, robot->lord_robot_->laizi) != 0)
		{
			SERVER_LOG("------------auto play a error card!!!!!!!! %d", ret);
		}
		curTime = leaf::GetCurTime() - curTime;
		SERVER_LOG("pt_gc_play_card_not_handler: time=%d", curTime);
	
	}
/*
	for(size_t i = 0; i < session->players_.size(); ++i)
	{
		if((!Robot::isRobot(session->players_[i].ply_guid_))&&(session->players_[i].chair_id_ == noti.cChairID)){
			SERVER_LOG("%lld takeout",session->players_[i].ply_guid_);
			for (i=0; i<size; i++ )
			{
				SERVER_LOG("m_nValue = %d , m_nCard_Baovalue = %d ", noti.vecCards[i].m_nValue, noti.vecCards[i].m_nCard_Baovalue);
			}
		}
	}	*/
}
