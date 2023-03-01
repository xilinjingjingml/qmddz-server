#pragma once
class CUserSession;
class pt_br_need_send_robot_not_handler
{
public:
	pt_br_need_send_robot_not_handler(void);
	~pt_br_need_send_robot_not_handler(void);

	static void handler(const pt_br_need_send_robot_not& noti,CUserSession* session);
};

