#pragma once
class CUserSession;
class pt_bc_ply_join_not_handler
{
public:
	pt_bc_ply_join_not_handler(void);
	~pt_bc_ply_join_not_handler(void);

	static void handler(const pt_bc_ply_join_not& noti,CUserSession* session);
};

