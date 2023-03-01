#include "stdafx.h"
#include "pt_gc_auto_not_handler.h"
#include "RobotManager.h"
#include "UserSession.h"

pt_gc_auto_not_handler::pt_gc_auto_not_handler(void)
{
}

pt_gc_auto_not_handler::~pt_gc_auto_not_handler(void)
{
}

void pt_gc_auto_not_handler::handler( const pt_gc_auto_not& noti, CUserSession* session )
{
	SERVER_LOG("@pt_gc_auto_not_handler.");

	int i, chair, size=0;

	Robot* robot = session->getRobot();

	if ((NULL == robot) || (NULL == robot->lord_robot_))
	{
		return;
	}

	if (noti.cChairID == session->chair_id())
	{
		SERVER_LOG("@pt_gc_auto_not_handler:cAuto = %d, chair=%d, uid=%lld",noti.cAuto, noti.cChairID ,session->ply_guid());
		if (noti.cAuto == 1)
		{
			robot->is_auto_ = true;
			//session->OnAuto(0);
		} 
		else
		{
			robot->is_auto_ = false;
		}
	}
}
