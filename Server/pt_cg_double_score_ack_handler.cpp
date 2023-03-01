#include "stdafx.h"
#include "pt_cg_double_score_ack_handler.h"
#include "Player.h"
#include "GameTable.h"
pt_cg_double_score_ack_handler::pt_cg_double_score_ack_handler(void)
{
}


pt_cg_double_score_ack_handler::~pt_cg_double_score_ack_handler(void)
{
}

void pt_cg_double_score_ack_handler::handler(const pt_cg_double_score_ack& req, CPlayer* pPlayer)
{
	if (pPlayer == NULL)
	{
		return;
	}
	CGameTable* pGameTable = pPlayer->m_pGameTable;
	if (pGameTable == NULL || req.nSerialID != pGameTable->m_nSerialID)
	{
		return;
	}
	glog.log("pt_cg_double_score_ack_handler charID:%d", pPlayer->GetChairID());

	if (g_nDoubleScore == 2)
	{
		if (pPlayer->m_bIsDouble)
		{
			return;
		}
		
		pPlayer->m_bIsDouble = true;

		if (req.nScore == 2)//底注加倍 
		{
			pPlayer->setMutiMyDouble(2);
			pGameTable->SendCommonCmd(CO_T_DOUBLE, pPlayer->GetChairID());
		}
		else if (req.nScore == 4) {
			if (pPlayer->superDoubleCheck())
			{
				pPlayer->setMutiMyDouble(4);
				pGameTable->SendCommonCmd(CO_SUPER_T_DOUBLE, pPlayer->GetChairID());
			}
		}
		else {
			pGameTable->SendCommonCmd(CO_F_DOUBLE, pPlayer->GetChairID());
		}

		// try startputcard
		int nDoubleOK = 0;
		for (int i = 0; i < pGameTable->GetPlayNum(); i++)
		{
			CPlayer* player = pGameTable->GetPlayer(i);
			if (player && player->m_bIsDouble)
			{
				nDoubleOK++;
			}
		}

		if (nDoubleOK == pGameTable->GetPlayNum())
		{
			pGameTable->startPutCard();
		}
		
		return;
	}

	int nDouble = 1;
	if (req.nScore > 0)//底注加倍
	{
		nDouble = (pPlayer->GetChairID() == pGameTable->m_Poke.m_nCurrentLord) ? 4 : 2;
	}
	// 标准模式下记录玩家加倍处理记录
	if (pGameTable->is_standard_game())
	{
		pPlayer->m_nStandardDoubleStep		= 2;
		pPlayer->m_nStandardDoubleScore		= req.nScore > 0 ? 2 : 1;
	}

	pGameTable->OnPutCardEndPause();
	glog.log("pt_cg_double_score_ack_handler nDouble:%d", nDouble);
	
	// 标准模式下记录玩家加倍处理记录
	if (pGameTable->is_standard_game())
	{
		pGameTable->StandardSendDoubleScore(nDouble , pPlayer);
	}
	else
	{
		pGameTable->sendDoubleScore(nDouble);
	}
}
