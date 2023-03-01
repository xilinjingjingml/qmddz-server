#include "stdafx.h"
#include "pt_gc_update_player_tokenmoney_not_handler.h"
#include "RobotManager.h"
#include "UserSession.h"
#include "ConfigManager.h"

pt_gc_update_player_tokenmoney_not_handler::pt_gc_update_player_tokenmoney_not_handler()
{
}


pt_gc_update_player_tokenmoney_not_handler::~pt_gc_update_player_tokenmoney_not_handler()
{
}

void pt_gc_update_player_tokenmoney_not_handler::handler(const pt_gc_update_player_tokenmoney_not& noti, CUserSession* session)
{
	Robot* robot = session->getRobot();

	if (NULL == robot || NULL == robot->lord_robot_)
	{
		return;
	}

	if (session->bBetterForPlayer && g_nBetterForPlayerCloseMoney > 0)
	{
		for (size_t i = 0; i < session->players_.size(); i++)
		{
			if (noti.ply_chairid_ == session->players_[i].chair_id_)
			{
				if (!Robot::isRobot(session->players_[i].ply_guid_))
				{
					for (size_t j = 0; j < noti.itemInfo.size(); j++)
					{
						if (noti.itemInfo[j].nItemIndex == ITEM_BY_CASH && noti.itemInfo[j].nItemNum >= g_nBetterForPlayerCloseMoney)
						{
							session->bBetterForPlayer = false;
							break;
						}
					}
				}
				break;
			}
		}
	}
}
