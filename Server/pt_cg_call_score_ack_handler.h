#pragma once
class CPlayer;
//客户端回应叫分结果
class pt_cg_call_score_ack_handler
{
public:
	pt_cg_call_score_ack_handler(void);
	~pt_cg_call_score_ack_handler(void);

	static void handler(const pt_cg_call_score_ack& ack,CPlayer* player);
};

