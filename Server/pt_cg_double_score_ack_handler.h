#pragma once
class CPlayer;
class pt_cg_double_score_ack_handler
{
public:
	pt_cg_double_score_ack_handler(void);
	~pt_cg_double_score_ack_handler(void);

	static void handler(const pt_cg_double_score_ack& req, CPlayer* player);
};
