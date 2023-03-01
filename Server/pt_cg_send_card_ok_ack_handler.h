#pragma once
class CPlayer;
//客户端回应发牌完毕
class pt_cg_send_card_ok_ack_handler 
{
public:
	
	pt_cg_send_card_ok_ack_handler(void);
	~pt_cg_send_card_ok_ack_handler(void);
	static void handler(const pt_cg_send_card_ok_ack& ack,CPlayer* player);
};

