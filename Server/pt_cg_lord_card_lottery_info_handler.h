#pragma once
class CPlayer;

class pt_cg_lord_card_lottery_info_handler
{
public:
	pt_cg_lord_card_lottery_info_handler(void);
	~pt_cg_lord_card_lottery_info_handler(void);
	
	static void handler(const pt_cg_lord_card_lottery_info& req,CPlayer* player);
};

