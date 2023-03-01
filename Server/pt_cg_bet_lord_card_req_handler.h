#pragma once
class CPlayer;

class pt_cg_bet_lord_card_req_handler
{
public:
	pt_cg_bet_lord_card_req_handler(void);
	~pt_cg_bet_lord_card_req_handler(void);
	
	static void handler(const pt_cg_bet_lord_card_req& req,CPlayer* player);
};

