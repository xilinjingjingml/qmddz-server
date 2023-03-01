#pragma once
class CUserSession;
//class FastRobotSession;
class pt_bc_join_table_ack_handler
{
public:
	pt_bc_join_table_ack_handler(void);
	~pt_bc_join_table_ack_handler(void);

	static void handler(const pt_bc_join_table_ack& ack,CUserSession* session);

	//static void fast_handler(const pt_bc_join_table_ack& ack,FastRobotSession* session);
};

