#pragma once
class CUserSession;
class pt_gc_two_lord_card_not_handler
{
public:
	pt_gc_two_lord_card_not_handler();
	~pt_gc_two_lord_card_not_handler();
	static void handler(const pt_gc_two_lord_card_not & req, CUserSession* session);
};

