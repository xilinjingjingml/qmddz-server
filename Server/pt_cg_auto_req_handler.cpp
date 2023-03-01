#include "stdafx.h"
#include "pt_cg_auto_req_handler.h"
#include "Player.h"
#include "GameTable.h"

pt_cg_auto_req_handler::pt_cg_auto_req_handler(void)
{
}


pt_cg_auto_req_handler::~pt_cg_auto_req_handler(void)
{
}

void pt_cg_auto_req_handler::handler( const pt_cg_auto_req& req,CPlayer* player )
{
	SERVER_LOG("pt_cg_auto_req_handler cAuto:%d,chairid:%d",req.cAuto,player->GetChairID());
	CGameTable* m_game_table = player->m_pGameTable;
 	if( m_game_table && m_game_table->m_bRacing )
 	{
		if (req.cAuto == 0)
		{
			player->m_nTimeOut = 0;
		}
		player->m_bAuto = req.cAuto;
 		pt_gc_auto_not noti;
 		noti.opcode = gc_auto_not;
 		noti.cChairID = player->GetChairID();
 		noti.cAuto = req.cAuto;
 		m_game_table->NotifyRoom(noti);

		if((req.cAuto == 1) && (m_game_table->m_nWaitOpChairID == player->GetChairID()))
			m_game_table->OnTimer((void*)m_game_table->m_currentTime_);
 	}
}
