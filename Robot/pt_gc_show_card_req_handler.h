#pragma once
class CUserSession;
class pt_gc_show_card_req_handler
{
public:
	pt_gc_show_card_req_handler(void);
	~pt_gc_show_card_req_handler(void);

	static void handler(const pt_gc_show_card_req& req,CUserSession* session);
};

