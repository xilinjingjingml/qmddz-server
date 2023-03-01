#include "stdafx.h"
#include "pt_cg_play_card_ack_handler.h"
#include "GameTable.h"
#include "Player.h"


pt_cg_play_card_ack_handler::pt_cg_play_card_ack_handler(void)
{
}


pt_cg_play_card_ack_handler::~pt_cg_play_card_ack_handler(void)
{
}

void pt_cg_play_card_ack_handler::handler( const pt_cg_play_card_ack& ack,CPlayer* player )
{
	SERVER_LOG("pt_cg_play_card_ack_handler size:%d,cTimeOut:%d",ack.vecCards.size(), ack.cTimeOut);
	CGameTable* m_game_table = player->m_pGameTable;
	if(!m_game_table)
		return ;
	if(!m_game_table->m_bRacing)
		return ;
	if( ack.nSerialID == m_game_table->m_nSerialID && player->GetChairID() == m_game_table->m_nWaitOpChairID )
	{
		m_game_table->OnPutCardEndPause();
		CPlayer* pPutPlayer = m_game_table->GetPlayer(m_game_table->m_nPutCardsPlayerID);
		if( pPutPlayer )
		{
			unsigned int diffTime = leaf::GetCurTime() - pPutPlayer->m_curTime;
			glog.log("play card req to ack time:%u", diffTime); 
			player->m_PlayCard.m_cDiscardingType.SetValue(0,0,0);
			player->m_PlayCard.m_cChoosingCards = ack.vecCards;
			if( player->m_PlayCard.m_cChoosingCards.empty() )	//没有出牌
			{
 				if(player->GetChairID() != m_game_table->m_nPutCardsPlayerID)
 				{
					player->m_bGiveUp = true;
					m_game_table->RefreshCards(player);
					m_game_table->SendPutCards(player, true);
					m_game_table->SendCommonCmd(CO_GIVEUP, player->GetChairID());
	
				}
				else
				{
					vector<CCard> auto_card;
					auto_card.push_back(player->m_PlayCard.m_cCards.back());
					player->m_PlayCard.m_cChoosingCards = auto_card;
					if( player->DoPlayCard(pPutPlayer) )
					{
						player->m_vecPutCard.push_back(player->m_PlayCard.m_cDiscardingType);
						m_game_table->m_nPutCardsPlayerID = m_game_table->m_nWaitOpChairID;
						player->m_nPutCount++;
						//有人出牌后,其他2家则可以再出牌
						for (int i = 0; i < m_game_table->GetPlayNum(); i++)
						{
							CPlayer* pUser = m_game_table->GetPlayer(i);
							if( pUser )
							{
								pUser->m_bGiveUp = false;
							}
						}
					}
					m_game_table->RefreshCards(player);
					m_game_table->SendPutCards(player, true);
				}
			}
			else
			{
				if( player->DoPlayCard(pPutPlayer) )	//比较牌型，看是否符合出牌规则
				{
					player->m_vecPutCard.push_back(player->m_PlayCard.m_cDiscardingType);
					m_game_table->m_nPutCardsPlayerID = m_game_table->m_nWaitOpChairID;
					player->m_nPutCount++;
					//有人出牌后,其他2家则可以再出牌
					for (int i = 0; i < m_game_table->GetPlayNum(); i++)
					{
						CPlayer* pUser = m_game_table->GetPlayer(i);
						if( pUser )
						{
							pUser->m_bGiveUp = false;
						}
					}
					m_game_table->RefreshCards(player);
					if( ack.cTimeOut == 1 )	//超时出牌通知所有玩家
					{
						m_game_table->SendPutCards(player, true);
					}
					else
					{
						player->m_nTimeOut = 0;
						m_game_table->SendPutCards(player);
					}
				}
				else
				{
					//g_pLogger->Log("DoPlayCard(%d) Error", player->GetChairID());
					//出牌不符合规则 系统自动选择不出
					if(player->GetChairID() != m_game_table->m_nPutCardsPlayerID)
					{
						player->m_PlayCard.m_cChoosingCards.clear();
						player->m_PlayCard.m_cDiscardingType.SetValue(0,0,0);
						m_game_table->RefreshCards(player);
						m_game_table->SendPutCards(player, true);
						m_game_table->SendCommonCmd(CO_GIVEUP, player->GetChairID());
					}
					else
					{
						vector<CCard> auto_card;
						auto_card.push_back(player->m_PlayCard.m_cCards.back());
						player->m_PlayCard.m_cChoosingCards = auto_card;
						if (player->DoPlayCard(pPutPlayer))
						{
							player->m_vecPutCard.push_back(player->m_PlayCard.m_cDiscardingType);
							m_game_table->m_nPutCardsPlayerID = m_game_table->m_nWaitOpChairID;
							player->m_nPutCount++;
							//有人出牌后,其他2家则可以再出牌
							for (int i = 0; i < m_game_table->GetPlayNum(); i++)
							{
								CPlayer* pUser = m_game_table->GetPlayer(i);
								if (pUser)
								{
									pUser->m_bGiveUp = false;
								}
							}
						}
						m_game_table->RefreshCards(player);
						m_game_table->SendPutCards(player, true);
					}
				}
			}

			//判断游戏是否结束
			if (m_game_table->IsCanEndRound(player))
			{
				m_game_table->RoundEnd(player);
			}
			else
			{
				m_game_table->SvrPlayCardReq(m_game_table->GetNextPlayer());
			}
		}
	}
}
