#pragma once
class CUserSession;
class pt_gc_update_player_tokenmoney_not_handler
{
public:
	pt_gc_update_player_tokenmoney_not_handler();
	~pt_gc_update_player_tokenmoney_not_handler();
	static void handler(const pt_gc_update_player_tokenmoney_not& noti, CUserSession* session);
};

