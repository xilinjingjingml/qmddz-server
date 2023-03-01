#pragma once
class CUserSession;
class pt_gc_double_score_req_handler
{
public:
	pt_gc_double_score_req_handler();
	~pt_gc_double_score_req_handler();
	static void handler(const pt_gc_double_score_req & noti, CUserSession* session);
};

