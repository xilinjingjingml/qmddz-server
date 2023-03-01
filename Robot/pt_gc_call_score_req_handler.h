#pragma once
class CUserSession;
class pt_gc_call_score_req_handler
{
public:
	pt_gc_call_score_req_handler(void);
	~pt_gc_call_score_req_handler(void);
	
	static void handler(const pt_gc_call_score_req& req,CUserSession* session);
};

