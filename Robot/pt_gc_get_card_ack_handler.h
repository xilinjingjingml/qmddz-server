#pragma once
class CUserSession;
class pt_gc_get_card_ack_handler
{
public:
	pt_gc_get_card_ack_handler(void);
	~pt_gc_get_card_ack_handler(void);

	static void handler(const pt_gc_get_card_ack& noti,CUserSession* session);
};

