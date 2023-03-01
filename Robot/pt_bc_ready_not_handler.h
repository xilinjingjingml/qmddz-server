#pragma once
class CUserSession;
class pt_bc_ready_not_handler
{
public:
	pt_bc_ready_not_handler(void);
	~pt_bc_ready_not_handler(void);
	
	static void handler(const pt_bc_ready_not& ack,CUserSession* session);
};

