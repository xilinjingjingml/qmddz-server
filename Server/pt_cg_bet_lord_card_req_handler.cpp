#include "stdafx.h"
#include "pt_cg_bet_lord_card_req_handler.h"
#include "ConfigManager.h"
#include "GameTable.h"
#include "Player.h"

pt_cg_bet_lord_card_req_handler::pt_cg_bet_lord_card_req_handler(void)
{
}


pt_cg_bet_lord_card_req_handler::~pt_cg_bet_lord_card_req_handler(void)
{
}

void pt_cg_bet_lord_card_req_handler::handler( const pt_cg_bet_lord_card_req& req,CPlayer* player )
{
// 	CGameTable *m_game_table = player->m_pGameTable;
// 	if(!m_game_table)
// 		return ;
// 	if(!m_game_table->m_bRacing)
// 		return ;
	if(!player->newRoundBegin)return;
	pt_gc_bet_lord_card_ack ack;
	ack.opcode = gc_bet_lord_card_ack;
	CGameTable *m_game_table = player->m_pGameTable;
	if (req.index == 100) //断线重连后，查询
	{
		if (m_game_table && m_game_table->m_bRacing && player->m_nBetLordCardNextIndex > 0)
		{
			ack.index = player->m_nBetLordCardNextIndex;
			ack.ret = -5;
			player->SendPacket(ack);
		}
		return;
	}

	if (m_game_table)
	{
		if (!m_game_table->m_bLordDeal)
		{
			ack.ret = -3;
			ack.index = player->m_nBetLordCardIndex;
			player->SendPacket(ack);
			return;
		}
	}
	if(req.index < 0 || req.index > 5)
	{
		ack.ret = -2;
		ack.index = req.index;
		player->SendPacket(ack);
		return ;
	}
	
	if(player->m_nBetLordCardNextIndex >= 0)
	{
		ack.ret = -1; // 已经押注过了，不能重复押注
		ack.index = player->m_nBetLordCardNextIndex;
	}
	else
	{
		player->m_nBetLordCardNextIndex = req.index;
		ack.ret = player->m_nBetLordCardNextIndex;
		IPlayer::PlyAttrItem item_(UP_RN_CAIDIPAI_GAME,PLY_BET_TYPE,req.index);
		if(player->GetCorePlayer()){
			player->GetCorePlayer()->UpdatePlyAttrs(item_);
		}
	}
	player->SendPacket(ack);
}
