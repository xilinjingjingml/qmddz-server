#pragma once
class CPlayer;

class pt_cg_get_lord_card_reward_handler
{
public:
	pt_cg_get_lord_card_reward_handler(void);
	~pt_cg_get_lord_card_reward_handler(void);
	
	static void handler(const pt_cg_get_lord_card_reward& req,CPlayer* player);
};

