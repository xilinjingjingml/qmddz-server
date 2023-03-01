#pragma once
class CPlayer;
//客户端回应出牌
class pt_cg_get_card_req_handler
{
public:
	pt_cg_get_card_req_handler(void);
	~pt_cg_get_card_req_handler(void);

	static void handler(const pt_cg_get_card_req& req, CPlayer* player);
};

