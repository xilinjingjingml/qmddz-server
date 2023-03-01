#include "stdafx.h"
#include "pt_cg_get_card_req_handler.h"
#include "GameTable.h"
#include "Player.h"


pt_cg_get_card_req_handler::pt_cg_get_card_req_handler(void)
{
}


pt_cg_get_card_req_handler::~pt_cg_get_card_req_handler(void)
{
}

void pt_cg_get_card_req_handler::handler( const pt_cg_get_card_req& req, CPlayer* player )
{/*
	SERVER_LOG("pt_cg_get_card_req_handler");	
	CGameTable* m_game_table = player->m_pGameTable;
	if(!m_game_table)
		return ;
	if(!m_game_table->m_bRacing)
		return ;

	pt_gc_get_card_ack ack;
	ack.opcode = gc_get_card_ack;
	ack.num = 0;
	

	for(int i = 0; i < m_game_table->GetPlayNum(); i++)
	{
		CPlayer* pUser = m_game_table->GetPlayer(i);
		if( pUser )
		{
			//pt_gc_refresh_card_not noti;
			//pUser->GetPlayerCards(noti, true);
			SERVER_LOG("pt_cg_get_card_req_handler, i= %d, chair=%d",i, pUser->m_pCorePlayer->GetChairID());	
			if(pUser->m_pCorePlayer->GetChairID() == 0)
			{
				ack.vecCards0 =pUser->m_PlayCard.m_cCards;
			}else if(pUser->m_pCorePlayer->GetChairID() == 1)
			{
				ack.vecCards1 =pUser->m_PlayCard.m_cCards;
			}else if(pUser->m_pCorePlayer->GetChairID() == 2)
			{
				ack.vecCards2 =pUser->m_PlayCard.m_cCards;
			}else{
				SERVER_LOG("unknown chair %d", pUser->m_pCorePlayer->GetChairID());		
			}
			ack.num++;
		}
	}
	player->SendPacket(ack);
	*/
}
