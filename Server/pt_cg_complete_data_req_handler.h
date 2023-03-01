#pragma once
class CPlayer;
//获取游戏数据，旁观、断线重连用
class pt_cg_complete_data_req_handler
{
public:
	pt_cg_complete_data_req_handler(void);
	~pt_cg_complete_data_req_handler(void);
	static void handler(const pt_cg_complete_data_req& req,CPlayer* player);
};

