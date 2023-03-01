#include "stdafx.h"
#include "pt_cg_get_lord_card_reward_handler.h"
#include "GameTable.h"
#include "Player.h"

pt_cg_get_lord_card_reward_handler::pt_cg_get_lord_card_reward_handler(void)
{
}


pt_cg_get_lord_card_reward_handler::~pt_cg_get_lord_card_reward_handler(void)
{
}

void pt_cg_get_lord_card_reward_handler::handler( const pt_cg_get_lord_card_reward& req,CPlayer* player )
{
// 	CGameTable *m_game_table = player->m_pGameTable;
// 	if(!m_game_table)
// 		return ;
// 	if(!m_game_table->m_bRacing)
// 		return ;
// 	pt_gc_get_lord_card_reward ack;
// 	ack.opcode = gc_get_lord_card_reward;
// 	ack.index = player->m_nBetLordCardIndex;
// 	if(player->m_nBetLordCardIndex >= 0 && player->m_nBetLordCardReward > 0)
// 	{
// 		if(player->UpdateLordCardLotteryReward())
// 			ack.money = player->m_nBetLordCardReward;
// 		else
// 			ack.money = -1;
// 
// 	}
// 	else
// 	{
// 		ack.money = -1;
// 	}
// 	player->SendPacket(ack);
	player->UpdateLordCardLotteryReward();
}