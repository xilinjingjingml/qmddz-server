#include "stdafx.h"
#include "pt_bc_ply_leave_not_handler.h"
#include "UserSession.h"

pt_bc_ply_leave_not_handler::pt_bc_ply_leave_not_handler(void)
{
}


pt_bc_ply_leave_not_handler::~pt_bc_ply_leave_not_handler(void)
{
}

void pt_bc_ply_leave_not_handler::handler( const pt_bc_ply_leave_not& noti,CUserSession* session )
{
	SERVER_LOG("pt_bc_ply_leave_not_handler:%lld",noti.ply_guid_);
	SERVER_LOG("--%lld session->players_ size:%d", session->ply_guid(), session->players_.size());
	//session->players_.push_back();
	/*
	vector<PlyBaseData> &arr = session->players_;
	for(vector<PlyBaseData>::iterator it=arr.begin(); it!=arr.end(); )
	{
		if(it->ply_guid_ == noti.ply_guid_)
		{
			it = arr.erase(it); //不能写成arr.erase(it);
		}
		else
		{
			++it;
		}
	}
	SERVER_LOG("--session->players_ size2:%d", session->players_.size());
	*/
}
