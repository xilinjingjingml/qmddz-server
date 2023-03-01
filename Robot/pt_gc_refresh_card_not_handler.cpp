#include "stdafx.h"
#include "pt_gc_refresh_card_not_handler.h"
#include "RobotManager.h"
#include "UserSession.h"

pt_gc_refresh_card_not_handler::pt_gc_refresh_card_not_handler( void )
{

}

pt_gc_refresh_card_not_handler::~pt_gc_refresh_card_not_handler( void )
{

}

void pt_gc_refresh_card_not_handler::handler( const pt_gc_refresh_card_not& noti,CUserSession* session )
{
	int i, chair=-1, size = 0;
	SERVER_LOG("pt_gc_refresh_card_not_handler:cChairID = %d, session_chair=%d,%lld",noti.cChairID, session->chair_id(),session->ply_guid());

	Robot* robot = session->getRobot();

	if ((NULL == robot) || (NULL == robot->lord_robot_))
	{
		return;
	}

	if (false == robot->is_initvard_)
	{
		session->set_status(CUserSession::US_CARD);
		if ( -1 == robot->lord_robot_->card[noti.cChairID][0])
		{
			SERVER_LOG("pt_gc_refresh_card_not_handler converCardToRobot : %d",noti.cChairID);
			robot->converCardToRobot(noti.vecCards, robot->lord_robot_->card[noti.cChairID],  NULL, -1);
		}

		if ((-1 != robot->lord_robot_->card[0][0])&&(-1 != robot->lord_robot_->card[1][0])&&(-1 != robot->lord_robot_->card[2][0]))
		{
			//assert(!robot->checkCardIsOk());
			chair = session->chair_id();
			initCard(robot->lord_robot_, robot->lord_robot_->card[chair],robot->lord_robot_->card[(chair+2)%3],robot->lord_robot_->card[(chair+1)%3]);
			robot->is_initvard_ = true;
			session->OnSendCardOk();		
		}
		if (noti.cChairID == session->chair_id())
		{
			SERVER_LOG("%lld initCard : %d",session->ply_guid(), chair);
			size = noti.vecCards.size();
			for (i=0; i<size; i++ )
			{
				SERVER_LOG("m_nValue = %d, m_nColor = %d ", noti.vecCards[i].m_nValue, noti.vecCards[i].m_nColor);
			}
		}
		else if ((noti.cChairID == 0) && (-1 != chair))
		{
			for(size_t i = 0; i < session->players_.size(); ++i)
			{
				if((!Robot::isRobot(session->players_[i].ply_guid_))&&(session->players_[i].chair_id_ == noti.cChairID))
				{
					SERVER_LOG("%lld initCard : %d",session->players_[i].ply_guid_, noti.cChairID);
					size = noti.vecCards.size();
					for (i=0; i<size; i++ )
					{
						SERVER_LOG("m_nValue = %d , m_nCard_Baovalue = %d ", noti.vecCards[i].m_nValue, noti.vecCards[i].m_nCard_Baovalue);
					}
				}
			}	
		
		}
	}	
	/*
	size = noti.vecCards.size();
	SERVER_LOG("size = %d",size);
	for (i=0; i<size; i++ )
	{
		SERVER_LOG("m_nValue = %d, m_nColor = %d , m_nCard_Baovalue = %d ", noti.vecCards[i].m_nValue, noti.vecCards[i].m_nColor, noti.vecCards[i].m_nCard_Baovalue);
	}*/
}

