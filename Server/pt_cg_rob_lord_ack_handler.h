#pragma once
class CPlayer;
class CGameTable;
//客户端回应抢地主回应
class pt_cg_rob_lord_ack_handler
{
public:
	pt_cg_rob_lord_ack_handler(void);
	~pt_cg_rob_lord_ack_handler(void);
	static void handler(const pt_cg_rob_lord_ack& ack,CPlayer* player);
};

