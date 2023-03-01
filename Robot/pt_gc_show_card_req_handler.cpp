#include "stdafx.h"
#include "pt_gc_show_card_req_handler.h"
#include "UserSession.h"

pt_gc_show_card_req_handler::pt_gc_show_card_req_handler(void)
{
}


pt_gc_show_card_req_handler::~pt_gc_show_card_req_handler(void)
{
}

void pt_gc_show_card_req_handler::handler( const pt_gc_show_card_req& req,CUserSession* session )
{
	SERVER_LOG("pt_gc_show_card_req_handler:nSerialID = %d",req.nSerialID);
}
