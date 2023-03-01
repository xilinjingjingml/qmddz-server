#include "stdafx.h"
#include "pt_cg_show_card_ack_handler.h"
#include "GameTable.h"
#include "Player.h"

pt_cg_show_card_ack_handler::pt_cg_show_card_ack_handler(void)
{
}


pt_cg_show_card_ack_handler::~pt_cg_show_card_ack_handler(void)
{
}

void pt_cg_show_card_ack_handler::handler( const pt_cg_show_card_ack& ack, CPlayer* player )
{

	if (g_nShowCard == 0) {
		return;
	}

	if (player->isShowCard()) {
		// 已经明牌了
		return;
	}

	if (ack.nShowCardType == SHOW_CARD_IN_READY) {
		CGameTable* m_game_table = player->m_pGameTable;
		if (m_game_table && !m_game_table->isTableStatusIn(CGameTable::eTIMER_EVENT_NULL)) {
			return;
		}

		if (ack.nShowCardBet > 1 && ack.nShowCardBet <= g_nShowCardDoubleInReady) {
			player->m_nTempForShowCardBet = ack.nShowCardBet;
		}
		player->setShowCard(SHOW_CARD_IN_READY);
	}else if (ack.nShowCardType == SHOW_CARD_IN_DEAL) {

		CGameTable* m_game_table = player->m_pGameTable;
		if (!m_game_table) {
			return;
		}

		if (m_game_table->isTableStatusIn(CGameTable::eDEALCARD_ANIMATE_EVENT) || m_game_table->isTableStatusIn(CGameTable::eCALLSCORE_EVENT)) {

			m_game_table->SendCommonCmd(CO_SHOWCARD, player->GetChairID());

			player->setShowCard(SHOW_CARD_IN_DEAL);

			if (ack.nShowCardBet > 1 && ack.nShowCardBet <= g_nShowCardDoubleInDeal) {
				m_game_table->setMutiDoubleDetail(BEI_SHU_INFO_SHOWCARD, ack.nShowCardBet);
			}

			m_game_table->SendShowCardNot(player);
		} else {
			return;
		}

	}
	else if (ack.nShowCardType == SHOW_CARD_IN_PUTCARD) {

		CGameTable* m_game_table = player->m_pGameTable;
		if (!m_game_table) {
			return;
		}

		if (m_game_table->isTableStatusIn(CGameTable::ePLAYCARD_EVENT) && player->m_nPutCount == 0 && player->GetChairID() == m_game_table->m_Poke.m_nCurrentLord) {
			m_game_table->SendCommonCmd(CO_SHOWCARD, player->GetChairID());
			player->setShowCard(SHOW_CARD_IN_PUTCARD);			
			m_game_table->setMutiDoubleDetail(BEI_SHU_INFO_SHOWCARD, 3);
			m_game_table->SendShowCardNot(player);
		}
		else {
			return;
		}

	}
	else {
		return;
	}


}
