#include "stdafx.h"
#include "pt_cg_send_card_ok_ack_handler.h"
#include "GameTable.h"
#include "Player.h"
#include "ConfigManager.h"


pt_cg_send_card_ok_ack_handler::pt_cg_send_card_ok_ack_handler(void)
{
}


pt_cg_send_card_ok_ack_handler::~pt_cg_send_card_ok_ack_handler(void)
{
}

void pt_cg_send_card_ok_ack_handler::handler( const pt_cg_send_card_ok_ack& ack,CPlayer* player )
{
	SERVER_LOG("pt_cg_send_card_ok_ack_handler");
	if(!player->m_pGameTable)
		return ;
	if(!player->m_pGameTable->m_bRacing)
		return ;
	if( ack.nSerialID == player->m_pGameTable->m_nSerialID )
	{
		int nChairID = -1;
		if(player->GetCorePlayer())
			 nChairID = player->GetCorePlayer()->GetChairID();
		if( nChairID >= 0 && nChairID < 3 )
		{
			if( player->m_pGameTable->m_Poke.m_bSendCardOK[nChairID] )	//避免收到重复消息
				return;
			player->m_pGameTable->m_Poke.m_bSendCardOK[nChairID] = true;
		}
		else
		{
			SERVER_LOG("pt_cg_send_card_ok_ack(%d) Error", nChairID);
			return;
		}
		//是否3家都发牌完毕
		int nSendCardOK = 0;
		for (int i = 0; i < player->m_pGameTable->GetPlayNum(); i++)
		{
			if (player->m_pGameTable->m_Poke.m_bSendCardOK[i])
			{
				nSendCardOK++;
			}
		}
		if (nSendCardOK == player->m_pGameTable->GetPlayNum())
		{			
			SERVER_LOG("pt_cg_send_card_ok_ack,all ready");
			player->m_pGameTable->AfterDealCard();
			if( g_nCallScore == 1 || g_nCallScore == 0 || g_nCallScore == 2 || g_nCallScore == 3)	//判断是否有叫分流程
				player->m_pGameTable->CallScore();
			else if (player->m_pGameTable->m_nRobLord == 1)	//判断是否有抢地主流程
				player->m_pGameTable->RobLord();
			else	//摸到地主牌的玩家即为地主
				player->m_pGameTable->DealingLord();
		}
	}
}
