#pragma once
class CUserSession;
class pt_bc_login_ack_handler
{
public:
	pt_bc_login_ack_handler(void);
	~pt_bc_login_ack_handler(void);

	static void handler(const pt_bc_login_ack& ack,CUserSession* session);
};

