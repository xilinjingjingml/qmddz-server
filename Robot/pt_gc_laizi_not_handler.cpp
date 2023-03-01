#include "stdafx.h"
#include "pt_gc_laizi_not_handler.h"
#include "RobotManager.h"
#include "UserSession.h"


pt_gc_laizi_not_handler::pt_gc_laizi_not_handler(void)
{
}


pt_gc_laizi_not_handler::~pt_gc_laizi_not_handler(void)
{
}

void pt_gc_laizi_not_handler::handler( const pt_gc_laizi_not& noti, CUserSession* session )
{
	SERVER_LOG("pt_gc_laizi_not_handler:laizi = %d",noti.card_value);

	Robot* robot = session->getRobot();
	if (NULL == robot || NULL == robot->lord_robot_  || (!robot->is_initvard_))
	{
		return;
	}
	if (noti.card_value >= 3)
	{
		robot->laizi_ = noti.card_value - 3;
		setLaizi(robot->lord_robot_, robot->laizi_);
	}
}
