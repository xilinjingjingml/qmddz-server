#include "stdafx.h"
#include "pt_bc_leave_table_ack_handler.h"
#include "UserSession.h"
#include "RobotManager.h"

pt_bc_leave_table_ack_handler::pt_bc_leave_table_ack_handler(void)
{
}


pt_bc_leave_table_ack_handler::~pt_bc_leave_table_ack_handler(void)
{
}

void pt_bc_leave_table_ack_handler::handler(const pt_bc_leave_table_ack& ack, CUserSession* session)
{
    LOG_INFO("@@--pt_bc_leave_table_ack_handler.-------ret:%d, %lld", ack.ret_, session->ply_guid());
    //for test robot
    if (!Robot::isRobot(session->ply_guid()))
    {
        session->set_status(CUserSession::US_LOBBY);
    }
    session->set_ready_time(0);
    //if(!session->isready_)
    //	return;

    session->players_.clear();
    session->isready_ = 0;
    Robot *robot = session->getRobot();
    if (!robot)
        return;
    RobotManager::GetInstancePtr()->pushRobot(robot);

    robot->is_initvard_ = false;
    robot->laizi_ = -1;
    robot->is_auto_ = false;
    robot->lord_ = -1;
    robot->userchairID = -1;

    if (NULL != robot->lord_robot_)
    {
        destoryRobot(robot->lord_robot_);
        robot->lord_robot_ = NULL;
    }
    SERVER_LOG("@@--pt_bc_leave_table_ack_handler.robot number:%d", RobotManager::GetInstancePtr()->size());

#if 0
    // 如果是长时间不举手被踢出，则走重连逻辑
    if (-4 == ack.ret_ && Robot::isRobot(session->ply_guid()))
    {
        SERVER_LOG("@@--pt_bc_leave_table_ack_handler.-4 CloseConnection %lld", session->ply_guid());
        session->CloseConnection();
    }
#else
    if (0 != ack.ret_ && Robot::isRobot(session->ply_guid()))
    {
        LOG_INFO("Close Connection %lld ret:%d", session->ply_guid(), ack.ret_);
        session->CloseConnection();
    }
#endif

}
