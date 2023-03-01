#include "stdafx.h"
#include "pt_cg_card_count_req_handler.h"
#include "GameTable.h"
#include "Player.h"

pt_cg_card_count_req_handler::pt_cg_card_count_req_handler(void)
{
}


pt_cg_card_count_req_handler::~pt_cg_card_count_req_handler(void)
{
}

void pt_cg_card_count_req_handler::handler( const pt_cg_card_count_req& req,CPlayer* player )
{
	CGameTable *m_game_table = player->m_pGameTable;
	if(!m_game_table)
		return ;
	if(!m_game_table->m_bRacing)
		return ;
	if(player->GetChairID()>-1)			//当前桌子上面玩家
	{
		int n_counts =  player->GetCorePlayer()->GetPlyAttr(1,ITEM_CARD_RECORD);
		if(n_counts>0)
		{
			player->UseCounts(); //使用记牌器

			pt_gc_card_count_ack1 ack1;
			ack1.opcode = gc_card_count_ack1;
			ack1.counts_num_ = n_counts;

			pt_gc_card_count_ack ack;
			ack.opcode = gc_card_count_ack;
			ack.counts_num_ = n_counts ;
			vector<CCard> all_card = player->m_allCards;
			vector<CCard> vec_card ;
			bool is_putvec_ = true;
			for (int i = 0; i<m_game_table->GetPlayNum(); i++)
			{
				CPlayer* pPlayer = m_game_table->GetPlayer(i);
				if (pPlayer && !pPlayer->m_vecPutCard.empty())
				{
					is_putvec_ = false;
				}
				//if(m_game_table->GetPlayer(i)->m_vecPutCard.size()>0)
				//	is_putvec_ = false;
			}

			if(is_putvec_)				//如果玩家都没有出牌 记牌器为空
			{
				ack.m_vecPutCard = vec_card;
				player->SendPacket(ack);

				ack1.m_vecPutCard = vec_card;
				player->SendPacket(ack1);

				return ;
			}

			for (int i = 0; i<m_game_table->GetPlayNum(); i++)
			{
				CPlayer* pPlayer = m_game_table->GetPlayer(i);
				if (!pPlayer)
				{
					continue;
				}
				for (size_t j = 0; j<pPlayer->m_PlayCard.m_cCards.size(); j++)
				{
					for(size_t k=0;k<all_card.size();k++)
					{
						if (pPlayer->m_PlayCard.m_cCards[j].m_nValue == all_card[k].m_nValue && pPlayer->m_PlayCard.m_cCards[j].m_nColor == all_card[k].m_nColor)
						{
							all_card[k].m_nValue = 0;
							all_card[k].m_nColor = 0;
							break;
						}
					}
				}
			}

			
			for(size_t i=0;i<all_card.size();i++)
			{
				if(all_card[i].m_nValue > 0)
				{
					if(all_card[i].m_nValue == 16)
					{
						if(all_card[i].m_nColor == 0)
							all_card[i].m_nColor = 1;
						else if(all_card[i].m_nColor == 1)
							all_card[i].m_nColor = 0;
					}
					vec_card.push_back(all_card[i]);
				}
			}
			ack.m_vecPutCard = vec_card;
			player->SendPacket(ack);

			ack1.m_vecPutCard = vec_card;
			player->SendPacket(ack1);
		}
		
	}
}
