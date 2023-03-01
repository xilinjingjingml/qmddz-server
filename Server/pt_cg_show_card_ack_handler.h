#pragma once
class CPlayer;
//客户端回应亮牌
class pt_cg_show_card_ack_handler
{
public:
	pt_cg_show_card_ack_handler(void);
	~pt_cg_show_card_ack_handler(void);
	
	static void handler(const pt_cg_show_card_ack& ack,CPlayer* player);
};

