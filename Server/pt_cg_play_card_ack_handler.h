#pragma once
class CPlayer;
//客户端回应出牌
class pt_cg_play_card_ack_handler
{
public:
	pt_cg_play_card_ack_handler(void);
	~pt_cg_play_card_ack_handler(void);

	static void handler(const pt_cg_play_card_ack& ack,CPlayer* player);
};

