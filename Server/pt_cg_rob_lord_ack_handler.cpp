#include "stdafx.h"
#include "pt_cg_rob_lord_ack_handler.h"
#include "GameTable.h"
#include "Player.h"

pt_cg_rob_lord_ack_handler::pt_cg_rob_lord_ack_handler(void)
{
}


pt_cg_rob_lord_ack_handler::~pt_cg_rob_lord_ack_handler(void)
{
}

void pt_cg_rob_lord_ack_handler::handler( const pt_cg_rob_lord_ack& ack,CPlayer* player )
{
	CGameTable *m_game_table = player->m_pGameTable;
	if(!m_game_table)
		return ;
	if(!m_game_table->m_bRacing)
		return ;
	if( ack.nSerialID == m_game_table->m_nSerialID && player->GetChairID() == m_game_table->m_nWaitOpChairID )
	{
		m_game_table->OnPutCardEndPause();
		if( player->GetChairID() == m_game_table->m_Poke.m_nCurrentLord )
			m_game_table->SendCommonCmd(CO_NOTCALLROB+ack.cRob, player->GetChairID());	//叫地主
		else
			m_game_table->SendCommonCmd(CO_NOTROB+ack.cRob, player->GetChairID());		//抢地主
		if( ack.cRob == 1 )
		{
			//叫地主后当前玩家变成默认地主
			if( player->GetChairID() == m_game_table->m_Poke.m_nCurrentLord )
			{
				m_game_table->m_Poke.m_nDefaultLord = m_game_table->m_Poke.m_nCurrentLord;
				m_game_table->m_Poke.m_nDecideLordRound = 1;
			}
			m_game_table->m_Poke.m_nCurrentLord = player->GetChairID();
			m_game_table->m_nDouble *= 2;
			m_game_table->m_nRobDouble *= 2;
			m_game_table->setMutiDoubleDetail(BEI_SHU_INFO_ROBLORD, 2);
			m_game_table->SendDoubleInfo();
			m_game_table->AddLetCardNum();
			m_game_table->AddRecordTimes(player->GetChairID(), CGameTable::eRECORD_CALL);
			player->m_nCallScore = 1;

		}
		else
		{
			if( m_game_table->m_nCallScore == 0 )	//没有叫分流程的情况
			{
				if( player->GetChairID() == m_game_table->m_Poke.m_nCurrentLord )	//谁第一个叫地主
				{
					m_game_table->m_Poke.m_nCurrentLord = (++m_game_table->m_Poke.m_nCurrentLord) % m_game_table->GetPlayNum();
					m_game_table->AddLetCardNum();
					m_game_table->AddRecordTimes(player->GetChairID(), CGameTable::eRECORD_CALL);
				}
			}
			player->m_nCallScore = -1;	//标识下不能再抢了
		}
		m_game_table->m_nWaitOpChairID = -1;

		m_game_table->RobLord();
	}
}
