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
		m_game_table->OnPutCardEndPause();
		player->m_nCallScore = (ack.nScore == 0 ? -1 : ack.nScore);

		if(g_nCallScore == 0)
		{
			int nScore = ack.nScore;
			if (ack.nScore > 0)
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
		else if(g_nCallScore == 1)
		{
			m_game_table->SendCommonCmd(CO_CALL0+ack.nScore, player->GetChairID());
			//判断是否比当前地主叫分高
			CPlayer* pLordPlayer = m_game_table->GetPlayer(m_game_table->m_Poke.m_nCurrentLord);
			if( pLordPlayer )
			{
				if( player->m_nCallScore > pLordPlayer->m_nCallScore || player == pLordPlayer )
				{
					if( ack.nScore > 0 )
					{
						m_game_table->m_nCallScore = ack.nScore;
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
			//如果有人叫了3分，则叫分结束
			if( player->m_nCallScore == 3 )	
			{
				m_game_table->m_Poke.m_nDecideLordRound = 3;
			}
		}
		m_game_table->m_nWaitOpChairID = -1;

		m_game_table->CallScore();
	}
}
