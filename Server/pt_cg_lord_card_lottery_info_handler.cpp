#include "stdafx.h"
#include "pt_cg_lord_card_lottery_info_handler.h"
#include "GameTable.h"
#include "Player.h"
#include "ConfigManager.h"

pt_cg_lord_card_lottery_info_handler::pt_cg_lord_card_lottery_info_handler(void)
{
}


pt_cg_lord_card_lottery_info_handler::~pt_cg_lord_card_lottery_info_handler(void)
{
}

void pt_cg_lord_card_lottery_info_handler::handler( const pt_cg_lord_card_lottery_info& req,CPlayer* player )
{
// 	CGameTable *m_game_table = player->m_pGameTable;
// 	if(!m_game_table)
// 		return ;
	if(g_nBaseLordCardLottery <= 0)
		return;
	pt_gc_lord_card_lottery_info ack;
	ack.opcode = gc_lord_card_lottery_info;
	ack.fee = g_nBaseLordCardLottery;
	//ack.vecReward = 0;
	player->SendPacket(ack);
}
