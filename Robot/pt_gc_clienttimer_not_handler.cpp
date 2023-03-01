#include "stdafx.h"
#include "ConfigManager.h"
#include "pt_gc_clienttimer_not_handler.h"
#include "RobotManager.h"
#include "UserSession.h"

pt_gc_clienttimer_not_handler::pt_gc_clienttimer_not_handler(void)
{
}


pt_gc_clienttimer_not_handler::~pt_gc_clienttimer_not_handler(void)
{
}

void pt_gc_clienttimer_not_handler::handler( const pt_gc_clienttimer_not& noti, CUserSession* session )
{
	SERVER_LOG("pt_gc_clienttimer_not_handler:guid = %lld", session->ply_guid());
	session->set_chat_time(0);
	if (g_chat_time_ == 0)
	{
		return;
	}

	Robot* robot = session->getRobot();
	if ((NULL == robot) || (NULL == robot->lord_robot_)|| (!robot->is_initvard_))
	{
		return;
	}

	if (noti.chairId == session->chair_id())
	{
		return;
	}

	if (session->chair_id() == robot->lord_)
	{
		session->set_chat_time(g_chat_time_);
	}
	else if (noti.chairId == robot->lord_)
	{
		if ((session->chair_id() + 1) % 3 == robot->lord_)
		{
			POKERS* h = robot->lord_robot_->game.players[DOWN_PLAYER]->h;
			if (h->total % 2 == 0)
			{
				session->set_chat_time(g_chat_time_);
			}
		}
		else
		{
			POKERS* h = robot->lord_robot_->game.players[UP_PLAYER]->h;
			if (h->total % 2 == 1)
			{
				session->set_chat_time(g_chat_time_);
			}
		}
	}
}
