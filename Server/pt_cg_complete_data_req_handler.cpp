#include "stdafx.h"
#include "pt_cg_complete_data_req_handler.h"
#include "Player.h"
#include "GameTable.h"
#include "ConfigManager.h"

pt_cg_complete_data_req_handler::pt_cg_complete_data_req_handler(void)
{
}


pt_cg_complete_data_req_handler::~pt_cg_complete_data_req_handler(void)
{
}

void pt_cg_complete_data_req_handler::handler( const pt_cg_complete_data_req& req,CPlayer* player )
{
	CGameTable *m_game_table = player->m_pGameTable;
	if( !m_game_table )
		return ;
	if( m_game_table->m_bRacing )
	{
		if (g_nTwoPai == 1)//二人斗地主
		{
			pt_gc_two_complete_data_not noti;
			noti.opcode = gc_two_complete_data_not;
			noti.nGameMoney = m_game_table->m_nBaseScore;
			noti.nDouble = (m_game_table->m_nCallScore == 0 ? 1 : m_game_table->m_nCallScore) * (1 << m_game_table->m_Poke.m_nBombCounter);
			noti.cLord = m_game_table->m_Poke.m_nCurrentLord;
			noti.nStart = m_game_table->m_bStart ? 1 : 0;
			noti.vecLordCards = m_game_table->m_Poke.m_cLordCard;
			if (!m_game_table->m_bLordDeal)//m_bLordDeal m_game_table->m_Poke.m_nDecideLordRound < 3
			{
				for (size_t i = 0; i < noti.vecLordCards.size(); i++)
				{
					CCard& cCard = noti.vecLordCards[i];
					cCard.m_nColor = 0;
					cCard.m_nValue = 0;
				}
			}

			for (int i = 0; i < m_game_table->GetPlayNum(); i++)
			{
				CPlayer* pCurPlayer = m_game_table->GetPlayer(i);
				if (pCurPlayer)
				{
					stUserData data;
					data.cChairID = i;
					data.vecPutCards = pCurPlayer->m_PlayCard.m_cChoosingCards;
					data.vecHandCards = pCurPlayer->m_PlayCard.m_cCards;

					if ((player->GetStatus() == CPlayer::PS_PLAYER && player->GetChairID() == i) ||
						(player->GetStatus() == CPlayer::PS_VISTOR && player->GetChairID() == i && pCurPlayer->isShowCard()))
					{

					}
					else
					{
						for (size_t i = 0; i < data.vecHandCards.size(); i++)
						{
							CCard& cCard = data.vecHandCards[i];
							cCard.m_nColor = 0;
							cCard.m_nValue = 0;
						}
					}
					noti.vecData.push_back(data);
				}
			}
			noti.nLetNum = m_game_table->GetLetCardNum();
			player->SendPacket(noti);

			m_game_table->sendLordDoubleScore(player->GetChairID());
			return;
		}

		pt_gc_complete_data_not noti;
		noti.opcode = gc_complete_data_not;
		noti.nGameMoney = m_game_table->m_nBaseScore;
		noti.nDouble = (m_game_table->m_nCallScore == 0 ? 1 : m_game_table->m_nCallScore) * (1 << m_game_table->m_Poke.m_nBombCounter);
		noti.cLord = m_game_table->m_Poke.m_nCurrentLord;
		noti.vecLordCards = m_game_table->m_Poke.m_cLordCard;
		if(m_game_table->m_Poke.m_nDecideLordRound < 3 )
		{
			for(size_t i = 0; i < noti.vecLordCards.size(); i++)
			{
				CCard& cCard = noti.vecLordCards[i];
				cCard.m_nColor = 0;
				cCard.m_nValue = 0;
			}
		}

		for (int i = 0; i < m_game_table->GetPlayNum(); i++)
		{
			CPlayer* pCurPlayer = m_game_table->GetPlayer(i);
			if( pCurPlayer )
			{
				stUserData data;
				data.cChairID = i;
				data.vecPutCards = pCurPlayer->m_PlayCard.m_cChoosingCards;
				data.vecHandCards = pCurPlayer->m_PlayCard.m_cCards;

				if( (player->GetStatus() == CPlayer::PS_PLAYER && player->GetChairID() == i) || 
					(player->GetStatus() == CPlayer::PS_VISTOR && player->GetChairID() == i && pCurPlayer->isShowCard()) )
				{

				}
				else
				{
					for(size_t i = 0; i < data.vecHandCards.size(); i++)
					{
						CCard& cCard = data.vecHandCards[i];
						cCard.m_nColor = 0;
						cCard.m_nValue = 0;
					}
				}
				noti.vecData.push_back(data);
			}
		}
		player->SendPacket(noti);
	}
}
