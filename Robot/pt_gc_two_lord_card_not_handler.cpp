#include "stdafx.h"
#include "pt_gc_two_lord_card_not_handler.h"
#include "RobotManager.h"
#include "UserSession.h"
#include "Pokers.h"

pt_gc_two_lord_card_not_handler::pt_gc_two_lord_card_not_handler()
{
}


pt_gc_two_lord_card_not_handler::~pt_gc_two_lord_card_not_handler()
{
}
void pt_gc_two_lord_card_not_handler::handler(const pt_gc_two_lord_card_not& noti, CUserSession* session)
{
	int i, chair, size = 0;
	SERVER_LOG("pt_gc_lord_card_not_handler:cLord = %d, chair_id=%d", noti.cLord, session->chair_id());

	Robot* robot = session->getRobot();

	if ((NULL == robot) || (NULL == robot->lord_robot_) || (!robot->is_initvard_))
	{
		return;
	}
	SERVER_LOG("--------------4444--------game.pot-------------%d", robot->lord_robot_->game.pot->total);


	robot->converCardToRobot(noti.vecCards, robot->lord_robot_->pot, NULL, -1);
	//assert(!robot->checkCardIsOk());
	chair = session->chair_id();
	if (noti.cLord == chair)
	{

		for (int i = 0; i < 15; i++)
		{
			SERVER_LOG("----------------------game.pot ..i -------------%d", robot->lord_robot_->game.pot->hands[i]);

		}

		SERVER_LOG("---------4------CUR_PLAYER-------------%d", robot->lord_robot_->game.players[CUR_PLAYER]->h->total);
		SERVER_LOG("=========4=========DOWN_PLAYER================%d", robot->lord_robot_->game.players[DOWN_PLAYER]->h->total);
		SERVER_LOG("---------4-------------UP_PLAYER-------------%d", robot->lord_robot_->game.players[UP_PLAYER]->h->total);
		SERVER_LOG("----------------------game.pot-------------%d", robot->lord_robot_->game.pot->total);
		beLord(robot->lord_robot_, robot->lord_robot_->pot, CUR_PLAYER);
		SERVER_LOG("--------5-------CUR_PLAYER-------------%d", robot->lord_robot_->game.players[CUR_PLAYER]->h->total);
		SERVER_LOG("========5==========DOWN_PLAYER================%d", robot->lord_robot_->game.players[DOWN_PLAYER]->h->total);
		SERVER_LOG("--------5--------------UP_PLAYER-------------%d", robot->lord_robot_->game.players[UP_PLAYER]->h->total);
		robot->lord_ = chair;
	}
	else{


		for (int i = 0; i < 15;i++)
		{
			SERVER_LOG("----------------------game.pot ..i -------------%d", robot->lord_robot_->game.pot->hands[i]);

		}
		SERVER_LOG("---------------CUR_PLAYER-------------%d", robot->lord_robot_->game.players[CUR_PLAYER]->h->total);
		SERVER_LOG("==================DOWN_PLAYER================%d", robot->lord_robot_->game.players[DOWN_PLAYER]->h->total);
		SERVER_LOG("----------------------UP_PLAYER-------------%d", robot->lord_robot_->game.players[UP_PLAYER]->h->total);
		SERVER_LOG("----------------------game.pot-------------%d", robot->lord_robot_->game.pot->total);

		beLord(robot->lord_robot_, robot->lord_robot_->pot, UP_PLAYER);
		SERVER_LOG("--------2-------CUR_PLAYER-------------%d", robot->lord_robot_->game.players[CUR_PLAYER]->h->total);
		SERVER_LOG("========2==========DOWN_PLAYER================%d", robot->lord_robot_->game.players[DOWN_PLAYER]->h->total);
		SERVER_LOG("--------2--------------UP_PLAYER-------------%d", robot->lord_robot_->game.players[UP_PLAYER]->h->total);
		//robot->lord_robot_->game.players[CUR_PLAYER]->h->total = robot->lord_robot_->game.players[CUR_PLAYER]->h->total -  noti.nLetNum;
		for (size_t i = 0; i < session->players_.size(); ++i)
		{
			SERVER_LOG("%lld,chair_id_:%d", session->players_[i].ply_guid_, session->players_[i].chair_id_);
			if ((Robot::isRobot(session->players_[i].ply_guid_)) && (session->players_[i].chair_id_ == noti.cLord)){
				setRobotLevel(robot->lord_robot_, 0);
				break;
			}
		}
		size = noti.vecCards.size();
		//for (i = 0; i<size; i++)
		//{
		//	SERVER_LOG("m_nValue = %d, m_nColor = %d ", noti.vecCards[i].m_nValue, noti.vecCards[i].m_nColor);
		//}

	}

	/*
	i = check_all_card_is_ok(robot->lord_robot_);
	if (0 != i)
	{
	g_pLogger->Log("check_all_card_is_ok:%d",i);
	}
	*/

}