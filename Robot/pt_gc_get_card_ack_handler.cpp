#include "stdafx.h"
#include "pt_gc_get_card_ack_handler.h"
#include "RobotManager.h"
#include "UserSession.h"


pt_gc_get_card_ack_handler::pt_gc_get_card_ack_handler(void)
{
}


pt_gc_get_card_ack_handler::~pt_gc_get_card_ack_handler(void)
{
}

void pt_gc_get_card_ack_handler::handler( const pt_gc_get_card_ack& ack, CUserSession* session )
{
	int i,k, chair, size = 0;
	SERVER_LOG("pt_gc_get_card_ack_handler:num = %d, chair = %d",ack.num, session->chair_id());

	Robot* robot = session->getRobot();

	if ((NULL == robot) || (NULL == robot->lord_robot_))
	{
		return;
	}

	chair = session->chair_id();

	if ((NULL != robot->lord_robot_) && (robot->is_initvard_==false))
	{
		robot->converCardToRobot(ack.vecCards0, robot->lord_robot_->card[0],  robot->lord_robot_->laizi,  robot->laizi_);
		robot->converCardToRobot(ack.vecCards1, robot->lord_robot_->card[1],  robot->lord_robot_->laizi,  robot->laizi_);
		robot->converCardToRobot(ack.vecCards2, robot->lord_robot_->card[2],  robot->lord_robot_->laizi,  robot->laizi_);

		initCard(robot->lord_robot_, robot->lord_robot_->card[chair],robot->lord_robot_->card[(chair+2)%3],robot->lord_robot_->card[(chair+1)%3]);
		robot->is_initvard_ = true;
	}

	session->OnSendCardOk();

	size = ack.vecCards0.size();
	SERVER_LOG("size = %d",size);
	for (i=0; i<size; i++ )
	{
		SERVER_LOG("m_nValue = %d, m_nColor = %d , m_nCard_Baovalue = %d ", ack.vecCards0[i].m_nValue, ack.vecCards0[i].m_nColor, ack.vecCards0[i].m_nCard_Baovalue);
	}
	size = ack.vecCards0.size();
	SERVER_LOG("size = %d",size);
	for (i=0; i<size; i++ )
	{
		SERVER_LOG("m_nValue = %d, m_nColor = %d , m_nCard_Baovalue = %d ", ack.vecCards1[i].m_nValue, ack.vecCards1[i].m_nColor, ack.vecCards1[i].m_nCard_Baovalue);
	}
	size = ack.vecCards0.size();
	SERVER_LOG("size = %d",size);
	for (i=0; i<size; i++ )
	{
		SERVER_LOG("m_nValue = %d, m_nColor = %d , m_nCard_Baovalue = %d ", ack.vecCards2[i].m_nValue, ack.vecCards2[i].m_nColor, ack.vecCards2[i].m_nCard_Baovalue);
	}
}
