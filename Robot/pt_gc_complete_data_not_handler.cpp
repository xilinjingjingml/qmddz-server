#include "stdafx.h"
#include "pt_gc_complete_data_not_handler.h"
#include "UserSession.h"

pt_gc_complete_data_not_handler::pt_gc_complete_data_not_handler(void)
{
}


pt_gc_complete_data_not_handler::~pt_gc_complete_data_not_handler(void)
{
}

void pt_gc_complete_data_not_handler::handler( const pt_gc_complete_data_not& noti,CUserSession* session )
{
	SERVER_LOG("pt_gc_complete_data_not_handler: ply_guid=%lld", session->ply_guid());
	//session->set_status(CUserSession::US_RACING);
	//session->sendAutoReq();
}
