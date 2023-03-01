#include "stdafx.h"
#include "pt_cg_call_score_ack_handler.h"
#include "Player.h"
#include "GameTable.h"
#include "ConfigManager.h"

pt_cg_call_score_ack_handler::pt_cg_call_score_ack_handler(void)
{
}


pt_cg_call_score_ack_handler::~pt_cg_call_score_ack_handler(void)
{
}

void pt_cg_call_score_ack_handler::handler( const pt_cg_call_score_ack& ack,CPlayer* player )
{
	CGameTable* m_game_table = player->m_pGameTable;
	if(!m_game_table)
		return ;
	if(!m_game_table->m_bRacing)
		return ;
	if( ack.nSerialID == m_game_table->m_nSerialID && player->GetChairID() == m_game_table->m_nWaitOpChairID )
	{
		int nScore = ack.nScore;
		if (m_game_table->m_nMustCallSeat >= 0)
		{
			if (m_game_table->m_nMustCallSeat == player->GetChairID())
			{
				nScore = 3;
			}
			else if (CConfigManager::GetInstancePtr()->isRobotGuid(player->GetCorePlayer()->GetPlyStrAttr(0, PLY_ATTR_GUID)))
			{
				nScore = 0;
			}
		}
		m_game_table->OnPutCardEndPause();
		player->m_nCallScore = (nScore == 0 ? -1 : nScore);

		if(g_nCallScore == 0)
		{
			if (nScore > 0)
			{
				nScore = 1;
				player->m_nCallScore = nScore;
			}

			m_game_table->SendCommonCmd(CO_NOTCALLROB + nScore, player->GetChairID());
			if(player->m_nCallScore == 1)
			{
				//判断是否比当前地主叫分高
			
				m_game_table->m_nCallScore = nScore;
				m_game_table->m_Poke.m_nCurrentLord = player->GetChairID();
				m_game_table->CallScoreAndSendDoubleInfo();
				if (g_nTwoPai == 1)
				{
					m_game_table->AddLetCardNum();
				}
				m_game_table->m_Poke.m_nDecideLordRound = 3;
				m_game_table->AddRecordTimes(player->GetChairID(), CGameTable::eRECORD_CALL);
			}
		}
		else if(g_nCallScore == 1 || g_nCallScore == 2 || g_nCallScore == 3)
		{
			if (g_nCallScore == 1 && nScore > 3)
			{
				nScore = 3;
			}
			int nOp = CO_CALL0 + nScore;
			if (g_nCallScore == 2 && nScore >= 3)
			{
				nScore = 4;
				nOp = CO_CALL4;
			}
			// 百元赛直接叫 4元 8元
			if (g_nCallScore == 3)
			{
				if (nScore >= 8) {
					nScore = 8;
					nOp = CO_CALL8;
				}
				else if (nScore > 0){
					nScore = 4;
					nOp = CO_CALL4;
				}
			}
			m_game_table->SendCommonCmd(nOp, player->GetChairID());
			//判断是否比当前地主叫分高
			CPlayer* pLordPlayer = m_game_table->GetPlayer(m_game_table->m_Poke.m_nCurrentLord);
			if( pLordPlayer )
			{
				if( player->m_nCallScore > pLordPlayer->m_nCallScore || player == pLordPlayer )
				{
					if(nScore > 0 )
					{
						m_game_table->m_nCallScore = nScore;
						m_game_table->m_Poke.m_nCurrentLord = player->GetChairID();
						m_game_table->CallScoreAndSendDoubleInfo();
						if (g_nTwoPai == 1)
						{
							m_game_table->AddLetCardNum();
						}
						m_game_table->AddRecordTimes(player->GetChairID(), CGameTable::eRECORD_CALL);
					}
				}
			}
			//如果有人叫了3分，则叫分结束 // 百元赛直接叫 4元 8元
			if((g_nCallScore == 1 && player->m_nCallScore >= 3) || (g_nCallScore == 3 && player->m_nCallScore >= 8) )	
			{
				m_game_table->m_Poke.m_nDecideLordRound = 3;
			}
			
		}
		m_game_table->m_nWaitOpChairID = -1;

		m_game_table->CallScore();
	}
}
