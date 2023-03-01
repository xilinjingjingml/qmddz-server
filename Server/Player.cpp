//////////////////////////////////////////////////////////////////////////
/// Copyright (c) 2005-2010 ShangHai Banding Co., All Rights Reserved.
///
/// @file Player.cpp
/// @brief 玩�?�控制类
///
/// 处理玩�?�出牌操作，响应玩�?�操作�?�求
///
/// @author leiliang
/// @date  2009/04/17
///
/// �?改�?�录�? 
/// �? �?             版本     �?改人        �?改内�?
/// 2009/04/17        1.0      leiliang       新建
/// 2009/06/08        1.1      leiliang       添加任务系统

/// 2012/08/10        v1.1.2    wulei           癞子场完
/// 2012/08/15        v1.1.3    wulei           比赛�?
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Player.h"
#include "GameTable.h"
#include "ConfigManager.h"
#include "pt_cg_auto_req_handler.h"
#include "pt_cg_call_score_ack_handler.h"
#include "pt_cg_complete_data_req_handler.h"
#include "pt_cg_play_card_ack_handler.h"
#include "pt_cg_rob_lord_ack_handler.h"
#include "pt_cg_send_card_ok_ack_handler.h"
#include "pt_cg_show_card_ack_handler.h"
#include "pt_cg_card_count_req_handler.h"
#include "pt_cg_get_card_req_handler.h"
#include "pt_cg_bet_lord_card_req_handler.h"
#include "pt_cg_get_lord_card_reward_handler.h"
#include "pt_cg_lord_card_lottery_info_handler.h"
#include "pt_cg_double_score_ack_handler.h"

#define GAME_MESSAGE_HANDLER(opcode) \
    case opcode: \
{ \
    pt_##opcode packet; \
    if (use_proto()) {  \
        ReadPacketFromProto(is, packet); \
    } \
    else { \
        is >> packet; \
    } \
    pt_##opcode##_handler::handler(packet, this); \
    break;  \
}

#define GAME_DISPATCH_MESSAGE(opcode) \
    case opcode: \
{ \
    pt_##opcode packet; \
    if (use_proto()) {  \
        ReadPacketFromProto(is, packet); \
    } \
    else { \
        is >> packet; \
    } \
    OnPacket(packet); \
    break;  \
}

CPlayer::CPlayer()
	: m_pCorePlayer(NULL), m_pGameTable(NULL), m_nStatus(PS_NONE), newRoundBegin(false), m_nStarSkyScore(0),
	m_nWinRound(0),
	m_nVIPlevel(0),
	m_nRedPackets_AwardNum(0),
	m_nRedPackets_AwardPool(0),
	m_nRedPackets_Status(0),
	m_nRedPackets_LimitRound(g_nHBTaskRound),
	m_nRedPackets_TaskID(0),
	m_nRedPackets_TotalAward(0),
	m_nRedPackets_TotalAwardCount(0),
	m_nRedPackets_CurRound(0)
{
	//m_bPlayer = this;
	SERVER_LOG("CPlayer::CPlayer()");
	for (int i = 3; i <= 15; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			CCard Card;
			Card.m_nColor = j;
			Card.m_nValue = i;
			m_allCards.push_back(Card);
		}
	}
	//小王,大王
	for (int i = 0; i <= 1; i++)
	{
		CCard Card;
		Card.m_nColor = i;
		Card.m_nValue = 16;
		m_allCards.push_back(Card);
	}
	m_nBetLordCardIndex = -1;
	m_nBetLordCardReward = -1;
	m_nBetLordCardNextIndex = -1;

	m_nStarSkySeason = -1;

	m_nRedPackets_88Yuan_AwardNum = 0;
	m_nRedPackets_88Yuan_AwardPool = 0;
	m_nRedPackets_88Yuan_Status = 0;
	m_nRedPackets_88Yuan_LimitRound = 0;
	m_nRedPackets_88Yuan_TaskID = 0;
	m_nRedPackets_88Yuan_TotalAward = 0;
	m_nRedPackets_88Yuan_TotalAwardCount = 0;
	m_nRedPackets_88Yuan_CurRound = 0;

	m_nShowCard = 0;
	m_nMyDouble = 1;
	m_nTempForShowCardBet = 1;
	m_bLookCard = false;
	m_bIsDouble = false;
	m_nCallLordRate = 100;
	m_nGameScore = 0;

	m_rls_game_data.last_lose_money = 0;
	m_rls_game_data.last_lose_time = 0;
	m_rls_game_data.last_regain_count = 0;
	m_rls_game_data.total_count = -1;
	m_nisRoundStart = false;
	m_bInvincible = false;
	m_vecNewbieCardsId.clear();
	m_vecRedPackets_Iteminfo.clear();
	m_nBaiYuanHBCurRound = 0;
	m_vecBaiYuanHBRoundAward.clear();
	m_vecBaiYuanWinDouble.clear();
	m_vecBaiYuanRegainLose.clear();
	m_vecBaiYuanLuckWelfare.clear();
}

CPlayer::~CPlayer()
{
	SERVER_LOG("CPlayer::~CPlayer()");
	//m_bPlayer->Release()
}
int64 CPlayer::GetGameMoney()
{
	SERVER_LOG("CPlayer::GetGameMoney()");

	//return m_pCorePlayer->GetGameMoney();
	if (g_nHBMode > 0 && g_nHBMode < HONGBAO_H5_CHUJI)
	{
		return GetCorePlayer()->GetPlyAttr(ITEM_ATTR, ITEM_GOLD_LEAF);
	}
	else {
		return GetCorePlayer()->GetPlyInt64Attr(0, PLY_ATTR_MONEY);
	}
}
void CPlayer::NewRound()
{
	SERVER_LOG("CPlayer::NewRound()");
	m_PlayCard.New();
	m_nCallScore = 0;
	m_bGiveUp = false;
	m_bAuto = false;
	m_nStandardDoubleScore = 1;
	m_nStandardBigerDoubleScore = 1;
	m_nStandardDoubleStep = 0;
	m_nPutCount = 0;
	m_nTimeOut = 0;
	m_vecPutCard.clear();
	m_nBetLordCardIndex = -1;
	m_nBetLordCardIndex = GetCorePlayer()->GetPlyAttr(PLY_BASE_ATTR, PLY_BET_TYPE);
	m_nBetLordCardReward = -1;
	m_nBetLordCardNextIndex = -1;
	m_nWinRound = 0;
	m_nMyDouble = 1;
	m_bIsDouble = false;
	m_nisRoundStart = true;
	m_nGameScore = 0;
	if (g_nIsBaiYuan == 1 && g_nBaiYuanHBRound > 0 && m_nBaiYuanHBCurRound >= g_nBaiYuanHBRound)
	{
		m_nBaiYuanHBCurRound = 0;
		SendBaiYuanHBRound();
	}
	m_vecBaiYuanHBRoundAward.clear();
	m_vecBaiYuanWinDouble.clear();
	m_vecBaiYuanRegainLose.clear();
	m_vecBaiYuanLuckWelfare.clear();

#if 0
	if (m_nBetLordCardIndex >= 0 && GetGameMoney() >= g_nBaseLordCardLottery + g_nLimitMoney)
	{
		//      IPlayer::PlyAttrItem item_(UP_RN_CAIDIPAI_GAME,PLY_ATTR_MONEY,-g_nBaseLordCardLottery);
		//      if(GetCorePlayer()){
		//          GetCorePlayer()->UpdatePlyAttrs(item_);
		//      }
		pt_gc_lord_card_lottery_base noti;
		noti.opcode = gc_lord_card_lottery_base;
		noti.ret = 0;
		noti.index_ = m_nBetLordCardIndex;
		SendPacket(noti);
	}
	else if (m_nBetLordCardIndex >= 0 && GetGameMoney() < g_nBaseLordCardLottery + g_nLimitMoney)
	{
		pt_gc_lord_card_lottery_base noti;
		noti.opcode = gc_lord_card_lottery_base;
		noti.ret = -1;
		noti.index_ = m_nBetLordCardIndex;
		SendPacket(noti);
		m_nBetLordCardIndex = -1;
	}
#endif

	newRoundBegin = true;
	m_bUseCounts = false;

	if (g_nPrivateRoom == 1 || g_nNewMatch == 1 || g_nIsStarSky == 1) {}
	else if (GetCorePlayer()->GetPlyAttr(1, ITEM_CARD_RECORD) > 0)
	{
		UseCounts(); //使用记牌�?
	}

	if (m_pCorePlayer)
	{
		const char* guid = m_pCorePlayer->GetPlyStrAttr(0, PLY_ATTR_GUID);
		if (!CConfigManager::GetInstancePtr()->isRobotGuid(guid))
		{
			readRegainLoseMoney();
		}
	}

	resetRedPacketForNewBie();
}

void CPlayer::GetCard(CCard& cCard)
{
	//SERVER_LOG("CPlayer::GetCard()");
	m_PlayCard.m_cCards.push_back(cCard);
}

void CPlayer::GetPlayerCards(pt_gc_refresh_card_not& noti, bool bShow)
{
	SERVER_LOG("CPlayer::GetPlayerCards()");
	noti.opcode = gc_refresh_card_not;
	noti.cChairID = m_pCorePlayer->GetChairID();
	noti.vecCards = m_PlayCard.m_cCards;

	//显示牌面

	if (bShow || isShowCard()) {

	}
	else {
		for (size_t i = 0; i < noti.vecCards.size(); i++) {
			CCard& cCard = noti.vecCards[i];
			cCard.m_nColor = 0;
			cCard.m_nValue = 0;
		}
	}
}


bool CPlayer::DoPlayCard(CPlayer* pPutPlayer)
{
	SERVER_LOG("CPlayer::DoPlayCard()");
	//检查出的牌�?否存�?，防止�?�挂出牌
	if (!m_PlayCard.CheckCards(m_PlayCard.m_cChoosingCards))
	{
		g_pLogger->Log("-------------------------------------------");
		g_pLogger->Log("cheating !!! strGuid:%s time:%u", GetCorePlayer()->GetPlyStrAttr(0, PLY_ATTR_GUID), leaf::GetCurTime());
		g_pLogger->Log("player put card:");
		for (vector<CCard>::iterator itr = m_PlayCard.m_cChoosingCards.begin(); itr != m_PlayCard.m_cChoosingCards.end(); itr++)
		{
			g_pLogger->Log("m_nColor:%d, m_nValue:%d, m_nCard_Baovalue:%d", int(itr->m_nColor), int(itr->m_nValue), int(itr->m_nCard_Baovalue));
		}
		g_pLogger->Log("player has card:");
		for (vector<CCard>::iterator itr = m_PlayCard.m_cCards.begin(); itr != m_PlayCard.m_cCards.end(); itr++)
		{
			g_pLogger->Log("m_nColor:%d, m_nValue:%d, m_nCard_Baovalue:%d", int(itr->m_nColor), int(itr->m_nValue), int(itr->m_nCard_Baovalue));
		}
		g_pLogger->Log("-------------------------------------------");

		if (m_pGameTable)
		{
			m_pGameTable->RefreshCards(this, NULL, false, false, false);
		}

		SERVER_LOG("DoPlayCard 牌不存在");
		return false;
	}
	/*
		int nCompare = 0;
		if( m_PlayCard.CheckChoosing() == 1 )
		{
			SERVER_LOG("DoPlayCard ok");
			if( pPutPlayer == this )    //首�?�出�?
			{
				nCompare = 1;
			}
			else if( pPutPlayer->m_PlayCard.m_cDiscardingType.m_nTypeNum != 0 )
			{
				if(g_nBaoPai)
					m_PlayCard.CheckChoosing(pPutPlayer->m_PlayCard.m_cDiscardingType.m_nTypeNum);
				nCompare = m_PlayCard.CompareCards(pPutPlayer->m_PlayCard.m_cDiscardingType);
			}
			if( nCompare != 1 )
			{
				return false;
			}
		}
		else
		{
			SERVER_LOG("DoPlayCard 牌不合�?�则");
			return false;
		}
		*/
		//change by JL for e
	int nCompare = 0;

	if (pPutPlayer == this)    //首�?�出�?
	{
		nCompare = m_PlayCard.CheckChoosing();
	}
	else if (pPutPlayer->m_PlayCard.m_cDiscardingType.m_nTypeNum != 0)
	{
		int nTypeNum = 0;
		if (g_nBaoPai)
		{
			nTypeNum = pPutPlayer->m_PlayCard.m_cDiscardingType.m_nTypeNum;
		}

		if (m_PlayCard.CheckChoosing(nTypeNum) == 1)
		{
			nCompare = m_PlayCard.CompareCards(pPutPlayer->m_PlayCard.m_cDiscardingType);
		}
	}
	if (nCompare != 1)
	{
		SERVER_LOG("DoPlayCard 牌不合�?�则");
		return false;
	}


	//癞子斗地主    �?箭X4倍、硬炸弹X4倍、软炸弹X2倍、纯癞子炸弹X64�?

	//如果出的�?炸弹
	if (m_PlayCard.m_cDiscardingType.m_nTypeNum == 4)
	{
		if (g_nBaoPai == 1)
		{
			if (m_PlayCard.m_cDiscardingType.m_nTypeBomb == 1)   //�?炸弹
			{
				m_pGameTable->m_Poke.m_nBombCounter++;
				m_pGameTable->m_nDouble *= 2;
				m_pGameTable->setMutiDoubleDetail(BEI_SHU_INFO_BOMB, 2);
				m_pGameTable->SendDoubleInfo();
			}
			else if ((m_PlayCard.m_cDiscardingType.m_nTypeBomb == 2) && (m_PlayCard.m_cDiscardingType.m_nTypeValue == BAO))
			{
				m_pGameTable->m_Poke.m_nBombCounter += 6;
				m_pGameTable->m_nDouble *= 64;
				m_pGameTable->setMutiDoubleDetail(BEI_SHU_INFO_BOMB, 64);
				m_pGameTable->SendDoubleInfo();
			}
			//else if(m_PlayCard.m_cDiscardingType.m_nTypeBomb == 2 && m_PlayCard.m_cDiscardingType.m_nTypeValue == 16) //�?�?
			else if (m_PlayCard.m_cDiscardingType.m_nTypeBomb == 2)  //�?炸弹 (包括�?�?)
			{
				m_pGameTable->m_Poke.m_nBombCounter += 2;
				m_pGameTable->m_nDouble *= 4;
				m_pGameTable->setMutiDoubleDetail(BEI_SHU_INFO_BOMB, 4);
				m_pGameTable->SendDoubleInfo();
			}

		}
		else
		{
			m_pGameTable->m_Poke.m_nBombCounter++;
			m_pGameTable->m_nDouble *= 2;
			m_pGameTable->setMutiDoubleDetail(BEI_SHU_INFO_BOMB, 2);
			m_pGameTable->SendDoubleInfo();
		}

		if (g_nChipFall == 1)
		{
			chipFall();
		}

		// 百元�?
		if (g_nIsBaiYuan == 1)
		{
			m_pGameTable->BombBaiYuan(this);
		}
	}

	//减去玩�?�出的牌
	m_PlayCard.EraseCards(m_PlayCard.m_cChoosingCards);

	return true;
}

void CPlayer::afterBindCorePlayer()
{

}


void CPlayer::chipFall()
{
	const char* guid = m_pCorePlayer->GetPlyStrAttr(0, PLY_ATTR_GUID);
	if (CConfigManager::GetInstancePtr()->isRobotGuid(guid))
	{
		return;
	}

	structRewardInfo info;
	int round = GetCorePlayer()->GetPlyAttr(PLY_BASE_ATTR, PLY_ATTR_ROUND);
	if (g_nChipIndexNewbie1 > 0 && round < g_nChipNewbieRound1)
	{
		CConfigManager::GetInstancePtr()->getRewardNum(g_nChipIndexNewbie1, info);
	}
	else if (g_nChipIndexNewbie2 > 0 && round < g_nChipNewbieRound2)
	{
		CConfigManager::GetInstancePtr()->getRewardNum(g_nChipIndexNewbie2, info);
	}
	else if (g_nChipIndexNewbie3 > 0 && round < g_nChipNewbieRound3)
	{
		CConfigManager::GetInstancePtr()->getRewardNum(g_nChipIndexNewbie3, info);
	}
	else if (g_nChipIndex > 0)
	{
		CConfigManager::GetInstancePtr()->getRewardNum(g_nChipIndex, info);
	}

	if (info.nItemNum <= 0)
	{
		return;
	}

	if (g_nChipLegendMax > 0)
	{
		if (info.nItemId == ITEM_CHIP_LEGEND && GetCorePlayer()->GetPlyAttr(ITEM_ATTR, ITEM_CHIP_LEGEND) >= g_nChipLegendMax)
		{
			return;
		}
	}

	IPlayer::PlyAttrItem item_(UP_RN_SPECIFY_ITEM, info.nItemId, info.nItemNum, GET_CHIP, "GET_CHIP");
	GetCorePlayer()->UpdatePlyAttrs(item_);

	// 通知道具变化
	notiItemAdd(info.nItemId, info.nItemNum);
}

void CPlayer::BindCorePlayer2Player(IPlayer* pCorePlayer)
{
	SERVER_LOG("CPlayer::BindCorePlayer2Player()");
	if (pCorePlayer != NULL)
	{
		m_pCorePlayer = pCorePlayer;
	}

	return;
}

void CPlayer::Release()
{
	SERVER_LOG("CPlayer::Release()");

	m_pGameTable = NULL;
	m_pCorePlayer = NULL;

	delete this;
}

void CPlayer::ProcessPacket(const char* pData, int nLen)
{

	CInputStream is(pData, nLen);
	short opcode;
	is >> opcode;
	is.Reset();
	SERVER_LOG("CPlayer::ProcessPacket().opcode:%d", opcode);
	switch (opcode)
	{

		GAME_MESSAGE_HANDLER(cg_send_card_ok_ack);      //发牌动画完成
		GAME_MESSAGE_HANDLER(cg_call_score_ack);        //�?�?
		GAME_MESSAGE_HANDLER(cg_rob_lord_ack);          //抢地�?
		GAME_MESSAGE_HANDLER(cg_show_card_ack);         //�?�?
		GAME_MESSAGE_HANDLER(cg_play_card_ack);         //出牌
		GAME_MESSAGE_HANDLER(cg_auto_req);              //托�??
		GAME_MESSAGE_HANDLER(cg_complete_data_req);     //请求完整数据�?
		GAME_MESSAGE_HANDLER(cg_card_count_req);
		GAME_MESSAGE_HANDLER(cg_bet_lord_card_req);
		GAME_MESSAGE_HANDLER(cg_get_lord_card_reward);
		GAME_MESSAGE_HANDLER(cg_lord_card_lottery_info);
		GAME_MESSAGE_HANDLER(cg_double_score_ack);
		GAME_DISPATCH_MESSAGE(cg_private_room_result_req);
		GAME_DISPATCH_MESSAGE(magic_emoji_req);
		GAME_DISPATCH_MESSAGE(cg_starsky_season_noti);
		GAME_DISPATCH_MESSAGE(ss_rp_update_grading_info_ack);
		GAME_DISPATCH_MESSAGE(gc_card_recode_req);
		//GAME_MESSAGE_HANDLER(cg_get_card_req);
		GAME_DISPATCH_MESSAGE(cg_get_redpackets_award_req); // 客户�?请求�?否可红包
		GAME_DISPATCH_MESSAGE(cg_get_redpackets_88yuan_award_req);  // 客户�?请求�?否可红包
		GAME_DISPATCH_MESSAGE(cg_look_lord_card_req);   //请求看底�?
		GAME_DISPATCH_MESSAGE(cg_beishu_info_req);  // 请求倍数
		GAME_DISPATCH_MESSAGE(cg_regain_lose_score_req);    // 获取之前输掉的金�?
		GAME_DISPATCH_MESSAGE(cg_enable_invincible_req);    // 开局免输
		GAME_DISPATCH_MESSAGE(cg_get_redpackets_newbie_award_ack);
		DISPATCH_MESSAGE(ss_update_regain_lose_score_ack);    // 开局免输
		DISPATCH_MESSAGE(ss_get_newbie_reward_ack);    // 开局免输
		GAME_DISPATCH_MESSAGE(cg_look_lord_card_item_req);   //请求看底�?
		GAME_DISPATCH_MESSAGE(cg_win_doubel_req);
		GAME_DISPATCH_MESSAGE(cg_baiyuan_hb_round_award_req);
		GAME_DISPATCH_MESSAGE(cg_baiyuan_win_double_req);
		GAME_DISPATCH_MESSAGE(cg_baiyuan_regain_lose_req);
		GAME_DISPATCH_MESSAGE(cg_baiyuan_luck_welfare_req);
		GAME_DISPATCH_MESSAGE(cg_baiyuan_can_bankruptcy_defend_req);
		GAME_DISPATCH_MESSAGE(cg_baiyuan_bankruptcy_defend_req);

	default:
		//SERVER_LOG("Unknow Cmd %d", opcode);      
		break;
	}
}

void CPlayer::OnNetStatusChange(bool status)
{
	SERVER_LOG("CPlayer::OnNetStatusChange(status:%d)", status);
	if (!m_pGameTable)
		return;
	if (status)                                     //�?线重�?
	{
		if (m_pGameTable->m_bRacing)
		{
			m_pGameTable->GetCompleteData(this);

			//          m_bAuto = false ;

			//          pt_cg_auto_req req;

			//          req.opcode = cg_auto_req;

			//          req.cAuto = m_bAuto;

			//          pt_cg_auto_req_handler::handler(req,this);

			// 
			
			if (isShowCard())
			{
				m_pGameTable->SendCommonCmd(CO_SHOWCARD, GetChairID(), this);
			}

			if (g_nBaoPai)
			{
				if (m_pGameTable->m_currentTime_ == CGameTable::eSHOWCARD_EVENT || m_pGameTable->m_currentTime_ == CGameTable::ePLAYCARD_EVENT)
				{
					pt_gc_laizi_not noti;
					noti.opcode = gc_laizi_not;
					noti.card_value = m_PlayCard.m_nBaoValue;
					SendPacket(noti);
				}
			}

			if (g_nCounts == 1)
			{
				int n_counts = m_pCorePlayer->GetPlyAttr(1, ITEM_CARD_RECORD);
				pt_gc_counts_not noti;
				noti.opcode = gc_counts_not;
				noti.counts_num_ = n_counts;
				SendPacket(noti);

				pt_gc_counts_not1 noti1;
				noti1.opcode = gc_counts_not1;
				noti1.counts_num_ = n_counts;
				SendPacket(noti1);
			}

			CPlayer* pPutPlayer = m_pGameTable->GetPlayer(m_pGameTable->m_nPutCardsPlayerID);
			if (pPutPlayer)
			{
				SERVER_LOG("CPlayer::OnNetStatusChange().pt_gc_play_card_not");
				pt_gc_play_card_not noti;
				noti.opcode = gc_play_card_not;
				if (g_nPrivateRoom == 1)
				{
					noti.opcode = gc_play_card_private_not;
				}
				noti.cChairID = pPutPlayer->GetChairID();
				noti.vecCards = pPutPlayer->m_PlayCard.m_cChoosingCards;
				noti.cType = pPutPlayer->m_PlayCard.m_cDiscardingType;
				SendPacket(noti);
			}
		}

		if (m_pGameTable->m_bRacing && m_pGameTable->m_nWaitOpChairID == GetChairID())
		{
			if (m_pGameTable->m_currentTime_ == CGameTable::eCALLSCORE_EVENT)
			{
				pt_gc_call_score_req req;
				req.opcode = gc_call_score_req;
				req.nScore = m_pGameTable->m_nCallScore;
				req.nSerialID = m_pGameTable->m_nSerialID;
				req.nCallMode = g_nCallScore;
				SendPacket(req);
			}
			else if (m_pGameTable->m_currentTime_ == CGameTable::eROBLORD_EVENT)
			{
				pt_gc_rob_lord_req req;
				req.opcode = gc_rob_lord_req;
				req.cDefaultLord = m_pGameTable->m_Poke.m_nCurrentLord;
				req.nSerialID = m_pGameTable->m_nSerialID;
				SendPacket(req);
			}
			else if (m_pGameTable->m_currentTime_ == CGameTable::eSHOWCARD_EVENT)
			{
				pt_gc_show_card_req req;
				req.opcode = gc_show_card_req;
				req.nSerialID = m_pGameTable->m_nSerialID;
				SendPacket(req);
			}
			else if (m_pGameTable->m_currentTime_ == CGameTable::eDOUBLE_EVENT)
			{
				pt_gc_double_score_req req;
				req.opcode = gc_double_score_req;
				req.nSerialID = m_pGameTable->m_nSerialID;
				SendPacket(req);
			}
			else
			{
				SERVER_LOG("CPlayer::OnNetStatusChange().reconnet");
				pt_gc_play_card_req req;
				req.opcode = gc_play_card_req;
				req.nSerialID = m_pGameTable->m_nSerialID;
				req.cAuto = m_bAuto;
				SendPacket(req);
				m_pGameTable->SendShowCardReq(SHOW_CARD_IN_PUTCARD, this);
			}
		}

	}
	else                                            //�?�?
	{
		if (m_pGameTable->m_bRacing && g_nPauseTime == 0)
		{
			SERVER_LOG("CPlayer::OnNetStatusChange().unconnet");
			m_bAuto = true;
			pt_cg_auto_req req;
			req.opcode = cg_auto_req;
			req.cAuto = m_bAuto;
			pt_cg_auto_req_handler::handler(req, this);

		}
	}
}

bool CPlayer::UpdateLordCardLotteryReward(bool needNotify /*= true*/)
{
	pt_gc_get_lord_card_reward ack;
	ack.opcode = gc_get_lord_card_reward;
	ack.index = m_nBetLordCardIndex;
	if (m_nBetLordCardIndex >= 0 && m_nBetLordCardReward > 0)
	{
		IPlayer::PlyAttrItem item2_(UP_RN_CAIDIPAI_GAME, PLY_ATTR_MONEY, m_nBetLordCardReward);
		if (GetCorePlayer()) {
			GetCorePlayer()->UpdatePlyAttrs(item2_);
			ack.money = m_nBetLordCardReward;
			if (needNotify)SendPacket(ack);
			m_nBetLordCardIndex = -1;
			m_nBetLordCardReward = -1;
			return true;
		}
	}
	ack.money = -1;
	if (needNotify)SendPacket(ack);
	return false;
}

void CPlayer::UpdateStarScore(SR_STARREASON eSR, int nScore)
{
	if (m_nStarSkySeason < 0) {
		return;
	}

	if (eSR == SR_UPDATE && m_nStarSkyScore + nScore < 0)
	{
		nScore = -m_nStarSkyScore;
	}

	pt_ss_rp_update_grading_info_req req;
	req.opcode = ss_rp_update_grading_info_req;
	req.season_ = m_nStarSkySeason;
	req.value_ = nScore;
	req.reason_ = (int)eSR;

	if (m_pCorePlayer)
	{
		COutputStream os;
		os << req;
		m_pCorePlayer->UpdateToPlayer(os.GetData(), os.GetLength());
	}
}

void CPlayer::UpdateStarItem()
{
	if (!m_pCorePlayer)
		return;

	pt_gc_starsky_update_item_noti noti;
	noti.opcode = gc_starsky_update_item;
	noti.stamina_ = m_pCorePlayer->GetPlyAttr(ITEM_ATTR, ITEM_STAMINA);
	noti.match_ticket_ = m_pCorePlayer->GetPlyAttr(ITEM_ATTR, ITEM_JOIN_MATCH_TICKET);
	noti.score_ = m_nStarSkyScore;
	noti.savestar_card_ = m_pCorePlayer->GetPlyAttr(ITEM_ATTR, ITEM_SAVESTAR_CARD);
	SendPacket(noti);
}

void CPlayer::OnPacket(const pt_cg_private_room_result_req& req)
{
	if (!m_pGameTable)
	{
		SERVER_LOG("CPlayer pt_cg_private_room_result_req m_pGameTable = %x", m_pGameTable);
		return;
	}

	m_pGameTable->OnGetPrivateRoomResult(this);
}

void CPlayer::OnPacket(const pt_magic_emoji_req& req)
{
	if (!m_pGameTable)
		return;

	if (!m_pCorePlayer)
		return;

	int errorIndx = 0;
	char cToChairID = req.cToChairID;
	int itemIndex = req.cEmojiIndex;
	int localItemIndex = itemIndex;

	const vector<MagicEmojiItem>& tempMagicEmojiConfigs = CConfigManager::GetInstancePtr()->m_vecMagicEmojiConfigs;

	if (g_nIsNeedMagicEmoji != 1) //不支�?
	{
		errorIndx = NO_SUPPORT;
	}
	else if (localItemIndex < 0 || tempMagicEmojiConfigs.size() == 0 || (localItemIndex > tempMagicEmojiConfigs.size() - 1))
	{
		errorIndx = NO_EXIST;
	}
	else if (cToChairID == GetChairID()) //不能发给�?�?
	{
		errorIndx = NO_SEND_SELF;
	}

	//通知
	pt_magic_emoji_noti magicEmojiInfo;
	magicEmojiInfo.opcode = magic_emoji_noti;
	magicEmojiInfo.cIsError = errorIndx;
	magicEmojiInfo.cEmojiIndex = itemIndex;
	magicEmojiInfo.cFromChairID = GetChairID();
	magicEmojiInfo.cToChairID = cToChairID;
	magicEmojiInfo.cEmojiNum = 1;

	if (errorIndx > 0) // 判断消耗类�?
	{
		SendPacket(magicEmojiInfo);
		return;
	}

	MagicEmojiItem emojiItem = tempMagicEmojiConfigs[localItemIndex];

	int nCostType = emojiItem.nItemIndex;
	int nItemConsumeCoins = emojiItem.nItemConsumeCoins;
	if (req.cCostType == 2)
	{
		nCostType = emojiItem.nTenItemIndex;
		nItemConsumeCoins = emojiItem.nTenItemNum;
		magicEmojiInfo.cEmojiNum = emojiItem.nTenEmojiNum;
	}

	// 如果魔法表情消耗的�?金币
	if (nCostType == ITEM_MONEY)
	{
		int64 curPlayerMoney = GetGameMoney();
		if ((curPlayerMoney - nItemConsumeCoins <= g_nLimitMoney) || (curPlayerMoney - nItemConsumeCoins) < 0)
		{
			errorIndx = NO_ENOUGH_MONEY;
		}
	}
	else if (nCostType == ITEM_QTT_ICON)
	{

	}
	else
	{
		if (GetCorePlayer()->GetPlyAttr(ITEM_ATTR, nCostType) < nItemConsumeCoins)
		{
			errorIndx = NO_ENOUGH_MONEY;    //�?知的货币
		}
	}


	if (errorIndx > 0) // 判断消耗类�?
	{
		SendPacket(magicEmojiInfo);
		return;
	}

	//成功
	IPlayer::PlyAttrItem item_(nCostType == ITEM_MONEY ? UP_RN_CONSUMABLE_PROPS_DATA : UP_RN_USE_ITEM, nCostType, -nItemConsumeCoins, 0, m_pGameTable->GetUniqLabel());
	GetCorePlayer()->UpdatePlyAttrs(item_);

	//广播
	m_pGameTable->NotifyRoom(magicEmojiInfo);
}

void CPlayer::OnPacket(const pt_cg_starsky_season_noti& noti)
{
	SERVER_LOG("pt_cg_starsky_season_noti", noti.season_);
	if (m_pGameTable && m_pGameTable->m_bRacing)
	{
		return;
	}
	m_nStarSkySeason = noti.season_;

	UpdateStarScore(SR_QUERY);
}

void CPlayer::OnPacket(const pt_ss_update_regain_lose_score_ack& ack)
{
	SERVER_LOG("pt_ss_update_regain_lose_score_ack");
	m_rls_game_data.last_lose_money = ack.last_lose_money_;
	m_rls_game_data.last_lose_time = ack.last_lose_time_;
	m_rls_game_data.last_regain_count = ack.last_regain_count_;
	m_rls_game_data.total_count = ack.remain_total_count;
	getRegainLoseMoney(0);
}

void CPlayer::OnPacket(const pt_ss_rp_update_grading_info_ack& ack)
{
	SERVER_LOG("pt_ss_rp_update_grading_info_ack", ack.ret_);
	if (ack.ret_ == 0)
	{
		m_nStarSkyScore = ack.value_;

		UpdateStarItem();
	}
	else
	{
		SERVER_LOG("pt_ss_rp_update_grading_info_ack", ack.ret_);
	}
}

void CPlayer::OnPacket(const pt_gc_card_recode_req& req)
{
	int n_counts = m_pCorePlayer->GetPlyAttr(ITEM_ATTR, ITEM_CARD_RECORD);
	pt_gc_counts_not noti;
	noti.opcode = gc_counts_not;
	noti.counts_num_ = n_counts;
	SendPacket(noti);

	pt_gc_counts_not1 noti1;
	noti1.opcode = gc_counts_not1;
	noti1.counts_num_ = n_counts;
	SendPacket(noti1);
}

void CPlayer::OnPacket(const pt_ss_get_newbie_reward_ack& ack)
{
	m_redpackets_newbie_data.nIndex = ack.index_;
	m_redpackets_newbie_data.nRound = ack.round_;

	if (ack.index_ == 0 && ack.round_ == 0)
	{
		if (GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_ROUND) == 0)
		{
			m_redpackets_newbie_data.nRound++;
		}
		else
		{
			// 没有参加新人局数活动的老用�? 剔除
			m_redpackets_newbie_data.nIndex = -1;
		}

	}
	else if (ack.index_ >= CConfigManager().GetInstancePtr()->m_vecRedPacketNewBieInfo.size())
	{
		// 超出 剔除
		m_redpackets_newbie_data.nIndex = -1;
	}
}

void CPlayer::OnPacket(const pt_cg_get_redpackets_newbie_award_ack& ack)
{
	if (m_redpackets_newbie_data.nIndex < 0)
	{
		return;
	}

	if (m_redpackets_newbie_data.nIndex >= CConfigManager().GetInstancePtr()->m_vecRedPacketNewBieInfo.size())
	{
		g_pLogger->Log("pt_cg_get_redpackets_newbie_award_ack guid:%s nIndex:%d", GetCorePlayer()->GetPlyStrAttr(0, PLY_ATTR_NICKNAME), m_redpackets_newbie_data.nIndex);
		m_redpackets_newbie_data.nIndex = -2;
		return;
	}

	structRedPacketInfo& sRedPacketNewBieInfo = CConfigManager().GetInstancePtr()->m_vecRedPacketNewBieInfo[m_redpackets_newbie_data.nIndex];
	if (m_redpackets_newbie_data.nRound < sRedPacketNewBieInfo.nRoundInterval)
	{
		return;
	}

	// 通知	
	pt_gc_get_redpackets_newbie_award_not noti;
	noti.opcode = gc_get_redpackets_newbie_award_not;
	noti.nRet = 0;
	noti.nAmount = sRedPacketNewBieInfo.nAmount;
	// 新手翻�?
	if (ack.cDouble > 0 && ack.cDouble <= sRedPacketNewBieInfo.nDoubel)
	{
		noti.nAmount *= ack.cDouble;
	}
	SendPacket(noti);

	// 加钱
	IPlayer::PlyAttrItem item_(UP_RN_SPECIFY_ITEM, ITEM_GOLD_TICKET, noti.nAmount, GET_REDPACKET_NEWBIE, "GET_REDPACKET_NEWBIE");
	GetCorePlayer()->UpdatePlyAttrs(item_);

	//重置数据
	m_redpackets_newbie_data.nIndex++;
	m_redpackets_newbie_data.nRound = 0;
	if (m_redpackets_newbie_data.nIndex >= CConfigManager().GetInstancePtr()->m_vecRedPacketNewBieInfo.size())
	{
		m_redpackets_newbie_data.nIndex = -2;
	}

	writeRedPacketsNewbieData();
}


void CPlayer::notiItemAdd(int nItemIndex, int nItemCount)
{
	pt_gc_item_add_not noti;
	noti.opcode = gc_item_add_not;
	noti.nItemIndex = nItemIndex;
	noti.nItemCount = nItemCount;
	SendPacket(noti);
}


void CPlayer::notiItemInfo(ITEM_INDEX nItemIndex)
{
	pt_gc_item_info_not noti;
	noti.opcode = gc_item_info_not;
	noti.nItemIndex = nItemIndex;
	noti.nItemCount = GetCorePlayer()->GetPlyAttr(ITEM_ATTR, nItemIndex);
	SendPacket(noti);
}

void CPlayer::UseCounts()
{
	if (m_bUseCounts)
	{
		return;
	}

	m_bUseCounts = true;

	pt_gc_use_card_recode_noti noti;
	noti.opcode = gc_use_card_recode_noti;
	noti.cChairID = GetChairID();
	noti.cReconnection = 0;
	m_pGameTable->NotifyRoom(noti);
}

void CPlayer::readRedPacketsData()
{
	if (g_nHBMode < HONGBAO_CHUJI)
	{
		return;
	}
	//�?前只有高级场读取数据�?
	if (g_nHBMode == HONGBAO_GAOJI)
	{
		/*
		RPGAME_DATA_GIFT_POOL = 0,   // 奖池
		RPGAME_DATA_CURRENT_ROUND,   // 当前回合
		RPGAME_DATA_REWARD_STATUS,   // 奖励状�?
		RPGAME_DATA_AWARD,           // 领取金�??
		RPGAME_DATA_TOTAL_REWARD,    // 领取红包总�?
		RPGAME_DATA_REWARD_TIMES,    // 红包领取次数
		*/
		m_nRedPackets_AwardPool = GetCorePlayer()->GetPlyInt64Attr(PLY_RP_GAME_DATA, RPGAME_DATA_GIFT_POOL);
		m_nRedPackets_CurRound = GetCorePlayer()->GetPlyInt64Attr(PLY_RP_GAME_DATA, RPGAME_DATA_CURRENT_ROUND);
		m_nRedPackets_Status = GetCorePlayer()->GetPlyInt64Attr(PLY_RP_GAME_DATA, RPGAME_DATA_REWARD_STATUS);
		m_nRedPackets_AwardNum = GetCorePlayer()->GetPlyInt64Attr(PLY_RP_GAME_DATA, RPGAME_DATA_AWARD);
	}

	m_nRedPackets_TotalAward = GetCorePlayer()->GetPlyInt64Attr(PLY_RP_GAME_DATA, RPGAME_DATA_TOTAL_REWARD);
	m_nRedPackets_TotalAwardCount = GetCorePlayer()->GetPlyInt64Attr(PLY_RP_GAME_DATA, RPGAME_DATA_REWARD_TIMES);

	/*
	int nNumRound = GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_ROUND);
	if (nNumRound < 3 && g_nHBMode == HONGBAO_H5_CHUJI)
	{
		m_nRedPackets_LimitRound = 1;
	}
	*/

	// 清理状�?3
	if (m_nRedPackets_Status == REDPACKET_STATUS_EXTRA)
	{
		m_nRedPackets_Status = REDPACKET_STATUS_NONE;
		m_nRedPackets_AwardNum = 0;
	}

	SendRedPacketsResult();
}

void CPlayer::SendRedPacketsResult()
{
	if (g_nHBMode < HONGBAO_CHUJI)
	{
		return;
	}

	pt_gc_get_redpackets_award_ack ack;
	ack.cur_rounds_ = m_nRedPackets_CurRound;
	ack.limit_rounds_ = m_nRedPackets_LimitRound;
	ack.task_id_ = m_nRedPackets_TaskID;
	ack.ret_ = m_nRedPackets_Status;
	ack.nAmount = m_nRedPackets_AwardNum;
	ack.cItemtype = ITEM_GOLD_TICKET;
	if (m_nRedPackets_Status == REDPACKET_STATUS_READY || m_nRedPackets_Status == REDPACKET_STATUS_SEND)
	{
		if (m_vecRedPackets_Iteminfo.size() > 0)
		{
			ack.fakeItem = m_vecRedPackets_Iteminfo;
		}
		else
		{
			vector<structRedPacket> vecRedPacketConfig1;
			vector<structRedPacket> vecRedPacketConfig2;
			const vector<structRedPacket>& tmp_vecRedPacketConfig = CConfigManager::GetInstancePtr()->m_vecRedPacketConfig;
			for (int i = 0; i < tmp_vecRedPacketConfig.size(); i++)
			{
				if (m_nRedPackets_AwardNum >= tmp_vecRedPacketConfig[i].nAmountMin)
				{
					vecRedPacketConfig1.push_back(tmp_vecRedPacketConfig[i]);
				}
				else
				{
					vecRedPacketConfig2.push_back(tmp_vecRedPacketConfig[i]);
				}
			}
			vector<structRedPacket> vecRedPacketConfig;
			if (vecRedPacketConfig1.size() > 1)
			{
				vecRedPacketConfig.insert(vecRedPacketConfig.end(), vecRedPacketConfig1.begin() + max((int)vecRedPacketConfig1.size() - 3, 0), vecRedPacketConfig1.end());
			}
			if (vecRedPacketConfig2.size() > 0)
			{
				vecRedPacketConfig.insert(vecRedPacketConfig.end(), vecRedPacketConfig2.begin(), vecRedPacketConfig2.begin() + min((int)vecRedPacketConfig2.size(), 2));
			}
			for (int i = 0; i < 2; i++)
			{
				int index = rand() % vecRedPacketConfig.size();
				structRedPacket& redPacketConfig = vecRedPacketConfig[index];
				int rangeNum = redPacketConfig.nAmountMax - redPacketConfig.nAmountMin;

				player_itemInfo fakeitem;
				fakeitem.nItemIndex = redPacketConfig.nItemIndex;
				fakeitem.nItemNum = redPacketConfig.nAmountMin + (rangeNum == 0 ? 0 : rand() % rangeNum);
				ack.fakeItem.push_back(fakeitem);

				vecRedPacketConfig.erase(vecRedPacketConfig.begin() + index);
			}

			m_vecRedPackets_Iteminfo = ack.fakeItem;
		}
	}

	SendPacket(ack);
}


void CPlayer::writeRedPacketsStatusData()
{
	if (g_nHBMode < HONGBAO_CHUJI)
	{
		return;
	}
	//�?前只有高级场写入数据�?
	if (g_nHBMode == HONGBAO_GAOJI)
	{
		/*
		int64 gift_pool;    // 奖池
		int current_round;  // 当前回合
		int reward_status;  // 奖励状�?
		int64 award;        // 领取金�??
		int64 total_reward; // 领取红包总�?
		int reward_times;   // 红包领取次数
		*/
		pt_ss_rp_save_game_data_req req;
		req.opcode = ss_rp_save_game_data_req;
		req.gift_pool = m_nRedPackets_AwardPool;
		req.current_round = m_nRedPackets_CurRound;
		req.reward_status = m_nRedPackets_Status;
		req.award = m_nRedPackets_AwardNum;
		req.total_reward = m_nRedPackets_TotalAward;
		req.reward_times = m_nRedPackets_TotalAwardCount;
		req.type_ = 0;
		UpdateToPlayer(req);
	}
}

void CPlayer::writeRedPacketsTotalData()
{
	if (g_nHBMode < HONGBAO_CHUJI)
	{
		return;
	}

	if (g_nHBMode == HONGBAO_GAOJI)
	{
		writeRedPacketsStatusData();
	}
	else {
		pt_ss_rp_save_game_data_req req;
		req.opcode = ss_rp_save_game_data_req;
		req.gift_pool = 0;
		req.current_round = 0;
		req.reward_status = 0;
		req.award = 0;
		req.total_reward = m_nRedPackets_TotalAward;
		req.reward_times = m_nRedPackets_TotalAwardCount;
		req.type_ = 0;
		UpdateToPlayer(req);
	}
}

void CPlayer::pt_cg_get_redpackets_award_req_handler(const pt_cg_get_redpackets_award_req& req)
{
	if (g_nHBMode < HONGBAO_CHUJI)
	{
		return;
	}

	if (m_nRedPackets_Status == REDPACKET_STATUS_READY || 
		m_nRedPackets_Status == REDPACKET_STATUS_EXTRA) {

	}else {
		SendRedPacketsResult();
		return;
	}

	//写数�?库操作TODO    
	if (m_nRedPackets_AwardNum > 0)
	{
		m_nRedPackets_CurRound = 0;

		m_nRedPackets_TotalAward = m_nRedPackets_TotalAward + m_nRedPackets_AwardNum;
		m_nRedPackets_TotalAwardCount++;
		m_nRedPackets_AwardPool -= m_nRedPackets_AwardNum;
		if (m_nRedPackets_AwardPool < 0)
		{
			m_nRedPackets_AwardPool = 0;
		}

		int reason = GET_REDPACKET_NORMAL;
		if (m_nRedPackets_Status == REDPACKET_STATUS_EXTRA)
		{
			reason = GET_REDPACKET_EXTRA;
		}
		IPlayer::PlyAttrItem item_(UP_RN_SPECIFY_ITEM, ITEM_GOLD_TICKET, m_nRedPackets_AwardNum, reason, "GET_REDPACKET_NORMAL");
		this->GetCorePlayer()->UpdatePlyAttrs(item_);

		bool needGenExtraRedPacket = (m_nRedPackets_Status == REDPACKET_STATUS_READY);
		//通知客户�?
		m_nRedPackets_Status = REDPACKET_STATUS_SEND;
		SendRedPacketsResult();
		getRedPacketBroadcast(m_nRedPackets_AwardNum, ITEM_GOLD_TICKET);

		if (needGenExtraRedPacket && genExtraRedPacketNum()) {
			SendRedPacketsResult();
		}else {
			m_nRedPackets_Status = REDPACKET_STATUS_NONE;
			m_nRedPackets_AwardNum = 0;
		}

		writeRedPacketsTotalData();
	}

}

bool CPlayer::genExtraRedPacketNum()
{
	const char* guid = m_pCorePlayer->GetPlyStrAttr(0, PLY_ATTR_GUID);
	if (CConfigManager::GetInstancePtr()->isRobotGuid(guid))
	{
		return false;
	}

	// 新人�?3局不显示抽�?
	int nNumRound = GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_ROUND);
	if (nNumRound < g_nBetterSeatForNewBieRound)
	{
		return false;
	}

	if (g_nRedPacketExtraLimit < 1)
	{
		return false;
	}
	if (g_nRedPacketExtraProbabily < 1)
	{
		return false;
	}

	int extra_value = GetCorePlayer()->GetPlyAttr(PLY_BASE_ATTR, PLY_RP_EXTRA_COUNT);
	if (extra_value >= g_nRedPacketExtraLimit)
	{
		return false;
	}

	if (rand() % 100 >= g_nRedPacketExtraProbabily) {
		return false;
	}

	m_nRedPackets_AwardNum = genRedPacketNumForH5();


	if (m_nRedPackets_AwardNum < 1)
	{
		m_nRedPackets_AwardNum = 0;
		return false;
	}

	m_nRedPackets_Status = REDPACKET_STATUS_EXTRA;
	return true;

	
}

void CPlayer::readRedPacketsNewbieData()
{
	if (m_redpackets_newbie_data.nIndex != -1)
	{
		return;
	}

	if (g_nRedPacketsNewbie == 0)
	{
		return;
	}

	if (CConfigManager().GetInstancePtr()->m_vecRedPacketNewBieInfo.size() == 0)
	{
		return;
	}

	const char* guid = m_pCorePlayer->GetPlyStrAttr(0, PLY_ATTR_GUID);
	if (CConfigManager::GetInstancePtr()->isRobotGuid(guid))
	{
		return;
	}

	pt_ss_get_newbie_reward_req req;
	req.opcode = ss_get_newbie_reward_req;
	UpdateToPlayer(req);
}

void CPlayer::writeRedPacketsNewbieData()
{
	if (m_redpackets_newbie_data.nIndex == -1)
	{
		return;
	}

	pt_ss_update_newbie_reward_req req;
	req.opcode = ss_update_newbie_reward_req;
	req.index_ = m_redpackets_newbie_data.nIndex;
	req.round_ = m_redpackets_newbie_data.nRound;
	UpdateToPlayer(req);
}

void CPlayer::genRedPacketForNewBie()
{
	if (m_redpackets_newbie_data.nIndex < 0)
	{
		return;
	}

	if (m_redpackets_newbie_data.nIndex >= CConfigManager().GetInstancePtr()->m_vecRedPacketNewBieInfo.size())
	{
		m_redpackets_newbie_data.nIndex = -2;
		return;
	}

	m_redpackets_newbie_data.nRound++;
	structRedPacketInfo& sRedPacketNewBieInfo = CConfigManager().GetInstancePtr()->m_vecRedPacketNewBieInfo[m_redpackets_newbie_data.nIndex];
	if (m_redpackets_newbie_data.nRound < sRedPacketNewBieInfo.nRoundInterval)
	{
		return;
	}

	pt_gc_get_redpackets_newbie_award_req req;
	req.opcode = gc_get_redpackets_newbie_award_req;
	req.nAmount = sRedPacketNewBieInfo.nAmount;
	req.cDouble = sRedPacketNewBieInfo.nDoubel;
	SendPacket(req);
}

void CPlayer::resetRedPacketForNewBie()
{
	if (m_redpackets_newbie_data.nIndex < 0)
	{
		return;
	}

	int nRoundInterval = CConfigManager().GetInstancePtr()->m_vecRedPacketNewBieInfo[m_redpackets_newbie_data.nIndex].nRoundInterval;
	if (m_redpackets_newbie_data.nRound >= nRoundInterval && GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_ROUND) != 0)
	{
		m_redpackets_newbie_data.nRound = nRoundInterval - g_nRedPacketsNewbieAddRound;
	}
}

void CPlayer::OnPacket(const pt_cg_get_redpackets_award_req& req)
{
	pt_cg_get_redpackets_award_req_handler(req);
}

void CPlayer::pt_cg_get_redpackets_88yuan_award_req_handler(const pt_cg_get_redpackets_88yuan_award_req& req)
{
	if (g_nHBMode < HONGBAO_H5_CHUJI)
	{
		return;
	}

	if (m_nRedPackets_88Yuan_Status != 1)
	{
		sendRedPacket88YuanData();
		return;
	}

	if (m_nRedPackets_88Yuan_AwardNum > 0)
	{
		m_nRedPackets_88Yuan_TotalAward += m_nRedPackets_88Yuan_AwardNum;
		m_nRedPackets_88Yuan_TotalAwardCount++;
		m_nRedPackets_88Yuan_CurRound = 0;
		m_nRedPackets_88Yuan_AwardPool = 0;
		IPlayer::PlyAttrItem item_(UP_RN_SPECIFY_ITEM, ITEM_GOLD_TICKET, m_nRedPackets_88Yuan_AwardNum, 0, "88Yuan");
		this->GetCorePlayer()->UpdatePlyAttrs(item_);

		m_nRedPackets_88Yuan_Status = 2;
		sendRedPacket88YuanData();
		getRedPacketBroadcast(m_nRedPackets_88Yuan_AwardNum, ITEM_GOLD_TICKET);

		const vector<structRedPacket88Yuan>& tmp_vecRedPacket8YuanConfig = CConfigManager::GetInstancePtr()->m_vecRedPacket88YuanConfig;

		m_nRedPackets_88Yuan_TaskID++;
		m_nRedPackets_88Yuan_TaskID = m_nRedPackets_88Yuan_TaskID % 3;

		int nextIndex = m_nRedPackets_88Yuan_TaskID;
		if (m_nRedPackets_88Yuan_TaskID + 1 > tmp_vecRedPacket8YuanConfig.size())
		{
			nextIndex = 0;
		}

		m_nRedPackets_88Yuan_TaskID = tmp_vecRedPacket8YuanConfig[nextIndex].cLid;
		m_nRedPackets_88Yuan_AwardNum = tmp_vecRedPacket8YuanConfig[nextIndex].nAmount;
		m_nRedPackets_88Yuan_LimitRound = tmp_vecRedPacket8YuanConfig[nextIndex].nRoundLimit;

		m_nRedPackets_88Yuan_Status = 0;

		saveRedPacket88YuanData();
		sendRedPacket88YuanData();
	}
}

void CPlayer::OnPacket(const pt_cg_get_redpackets_88yuan_award_req& req)
{
	if (g_nHB88YuanEnable != 1)
	{
		return;
	}
	pt_cg_get_redpackets_88yuan_award_req_handler(req);
}

bool CPlayer::checkRedPacketsTask()
{
	if (g_nHBMode < HONGBAO_CHUJI)
	{
		return false;
	}

	if (m_nRedPackets_AwardNum > 0)
	{
		return true;
	}
	m_vecRedPackets_Iteminfo.clear();

	if (m_nRedPackets_CurRound >= m_nRedPackets_LimitRound)
	{
		m_nRedPackets_CurRound = m_nRedPackets_LimitRound;

		int nNumRound = GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_ROUND);

		if (nNumRound > 2)
		{
			m_nRedPackets_LimitRound = g_nHBTaskRound;
		}

		if (g_nHBMode < HONGBAO_H5_CHUJI)
		{
			m_nRedPackets_AwardNum = genRedPacketNum();
		}
		else {
			m_nRedPackets_AwardNum = genRedPacketNumForH5();
		}

		if (m_nRedPackets_AwardNum > 0)
		{
			m_nRedPackets_Status = REDPACKET_STATUS_READY;
		}
		return true;
	}
	m_nRedPackets_AwardNum = 0;
	m_nRedPackets_Status = REDPACKET_STATUS_NONE;
	return false;

}

void CPlayer::RoundEnd(int64 nScore)
{
	m_nRedPackets_CurRound++;

	m_nWinRound = nScore > 0 ? 2 : 1;
	m_nGameScore = nScore;
	//检查红包状态是否可以�??, �?以�?�直接生成红�?
	checkRedPacketsTask();
	//通知客户�?
	SendRedPacketsResult();

	checkRedPacket88YuanData();
	sendRedPacket88YuanData();

	// 新人前n局奖励红包
	genRedPacketForNewBie();

	// 胜利翻�?
	genWinDouble();

	setShowCard(0);
	m_nTempForShowCardBet = 1;
	m_bLookCard = false;
	m_nCallLordRate = 100;
	m_nisRoundStart = false;
	m_bInvincible = false;
	
	if (g_nIsBaiYuan == 1)
	{
		if (g_nBaiYuanHBRound > 0)
		{
			m_nBaiYuanHBCurRound++;

			//百元赛局数红�?
			SendBaiYuanHBRound();

			//百元赛局数红�?
			SendBaiYuanHBRoundAward();
		}

		//百元赛赢分加�?
		SendBaiYuanWinDouble();

		//百元赛追回损�?
		SendBaiYuanRegainLose();

		//百元赛幸运�?�利
		SendBaiYuanLuckWelfare();
	}
}


int CPlayer::genRedPacketNumForNewBie() {
	const vector<int>& tmp_vecRedPacketNewBieConfig = CConfigManager::GetInstancePtr()->m_vecRedPacketNewBieConfig;
	if (tmp_vecRedPacketNewBieConfig.size() < 1)
	{
		return 0;
	}

	int nNumRound = GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_ROUND) - 1;
	//int limitRound = tmp_vecRedPacketNewBieConfig.size() * 4 + 10;//放�?�局数显示，有的人可能中途破产了�?
	if (nNumRound >= 0 && nNumRound < tmp_vecRedPacketNewBieConfig.size())
	{
		if (m_nRedPackets_TotalAwardCount >= 0 && m_nRedPackets_TotalAwardCount < tmp_vecRedPacketNewBieConfig.size())
		{
			int nHBValue = tmp_vecRedPacketNewBieConfig[nNumRound];
			if (g_nRedpakcetControllerRatio == 1)
			{
				int ctrlOffset = GetCorePlayer()->GetPlyAttr(PLY_BASE_ATTR, PLY_REDPACKET_CONTROLLER_RATIO);
				nHBValue = nHBValue * ctrlOffset / 100;
			}
			return nHBValue;
		}
	}
	return 0;
}

int CPlayer::genRedPacketNumForH5(int extBet)
{
	int nHBValue = genRedPacketNumForNewBie();

	if (nHBValue > 0) {
		m_nRedPackets_AwardPool = 0;
		return nHBValue;
	}

	nHBValue = 0;
	extBet = 0;
	const vector<structRedPacket>& tmp_vecRedPacketConfig = CConfigManager::GetInstancePtr()->m_vecRedPacketConfig;
	vector<structRedPacket> realVecRedPacketConfig;

	int TotalProbabily = 0;

	int playerVip = GetCorePlayer()->GetPlyAttr(PLY_BASE_ATTR, PLY_VIP_LEVEL);
	playerVip = playerVip < 1 ? 0 : playerVip;

	int reduceProbabily = 0;
	int reduceRate = 40;


	for (size_t i = 0; i < tmp_vecRedPacketConfig.size(); i++) {
		if (m_nRedPackets_AwardPool < tmp_vecRedPacketConfig[i].nAmountMin ||
			playerVip < tmp_vecRedPacketConfig[i].nVipLimit)
		{
			break;
		}
		realVecRedPacketConfig.push_back(tmp_vecRedPacketConfig[i]);
	}

	int awardIndexCount = realVecRedPacketConfig.size();

	//限制抽取红包过�?�的�?
	if (g_nRedPacketLimit == 1)
	{
		vector<structRedPackeLimit>::reverse_iterator vecRedPacketLimitIt = CConfigManager::GetInstancePtr()->m_vecRedPacketLimitConfig.rbegin();
		for (; vecRedPacketLimitIt != CConfigManager::GetInstancePtr()->m_vecRedPacketLimitConfig.rend(); ++vecRedPacketLimitIt) {
			if (vecRedPacketLimitIt->nPoolDeltaRate > 0 && getPayValueRate() >= vecRedPacketLimitIt->nPayRate)
			{
				reduceProbabily = vecRedPacketLimitIt->nReduceProbabily;
				break;
			}
		}

		if (rand() % 100 < reduceProbabily) {
			if (awardIndexCount > 0)
			{
				int reduceNum = awardIndexCount / 2;
				if (awardIndexCount > reduceNum)
				{
					for (int i = 0; i < reduceNum; i++) {
						realVecRedPacketConfig[realVecRedPacketConfig.size() - 1 - i].nProbabily[0] *= ((100 - reduceRate) / 100);
					}
				}
			}
		}
	}


	int probabilyIdx = getRedPacketProbabilityIdx();

	for (size_t i = 0; i < realVecRedPacketConfig.size(); i++) {
		TotalProbabily = TotalProbabily + tmp_vecRedPacketConfig[i].nProbabily[probabilyIdx];
	}

	if (TotalProbabily < 1)
	{
		return 0;
	}

	int nRandNum = rand() % TotalProbabily;
	bool enableOffset = g_nRedpakcetControllerRatio == 1;
	for (size_t i = 0; i < tmp_vecRedPacketConfig.size(); i++)
	{
		nRandNum = nRandNum - tmp_vecRedPacketConfig[i].nProbabily[probabilyIdx];
		if (nRandNum < 0)
		{
			int rangeNum = tmp_vecRedPacketConfig[i].nAmountMax - tmp_vecRedPacketConfig[i].nAmountMin + 1;
			if (rangeNum > 1) {
				nHBValue = rand() % rangeNum + tmp_vecRedPacketConfig[i].nAmountMin;
			}
			else {
				nHBValue = tmp_vecRedPacketConfig[i].nAmountMin;
				enableOffset = false;
			}
			break;
		}
	}

	int ctrlAccumulate = GetCorePlayer()->GetPlyAttr(PLY_BASE_ATTR, PLY_REDPACKET_CONTROLLER_ACCUMULATE);
	int ctrlOffset = GetCorePlayer()->GetPlyAttr(PLY_BASE_ATTR, PLY_REDPACKET_CONTROLLER_RATIO);

	if (enableOffset)
	{
		nHBValue = nHBValue * ctrlOffset / 100;
	}

	if (ctrlAccumulate == 0 && g_nRedPacketExtraLimit < 1)
	{
		m_nRedPackets_AwardPool = 0;
	}

	if (false) {
		pt_gc_common_not noti;
		noti.opcode = gc_common_not;
		noti.nOp = 10000 + ctrlOffset * 10 + ctrlAccumulate;
		noti.cChairID = -2;
		SendPacket(noti);
	}

	return nHBValue;
}

int CPlayer::getRedPacketProbabilityIdx()
{
	int total_value = GetCorePlayer()->GetPlyAttr(PLY_BASE_ATTR, PLY_RP_TOTAL_VALUE);
	int today_count = GetCorePlayer()->GetPlyAttr(PLY_BASE_ATTR, PLY_RP_TODAY_COUNT); 
	int idx = 0;
	
	// 
	for (int i = 0; i < CConfigManager::GetInstancePtr()->m_vecRedpacketConfigTotal.size(); ++i) 
	{
		if (total_value < CConfigManager::GetInstancePtr()->m_vecRedpacketConfigTotal[i])
		{
			idx = i + CConfigManager::GetInstancePtr()->m_vecRedpacketConfigCount.size();
			return idx;
		}
	}

	for (int i = CConfigManager::GetInstancePtr()->m_vecRedpacketConfigCount.size()-1; i >= 0; --i)
	{
		if (today_count >= CConfigManager::GetInstancePtr()->m_vecRedpacketConfigCount[i])
		{
			idx = i;
			return idx;
		}
	}

	return idx;
}

int CPlayer::genRedPacketNum(int extBet)
{
	int nHBValue = 0;
	vector<structRedPacket> tmp_vecRedPacketConfig = CConfigManager::GetInstancePtr()->m_vecRedPacketConfig;

	int TotalProbabily = 0;

	if (m_nRedPackets_TotalAwardCount == 0)
	{
		extBet = 13;
	}
	else if (m_nWinRound == 2) { //赢的人倍数�?
		extBet = 8;
	}
	else if (m_nWinRound == 1) {
		extBet = 11;
	}

	int playerVip = GetCorePlayer()->GetPlyAttr(PLY_BASE_ATTR, PLY_VIP_LEVEL);
	playerVip = playerVip < 1 ? 0 : playerVip;

	for (size_t i = 0; i < tmp_vecRedPacketConfig.size(); i++) {
		if (m_nRedPackets_AwardPool < tmp_vecRedPacketConfig[i].nAmountMin ||
			playerVip < tmp_vecRedPacketConfig[i].nVipLimit)
		{
			if (TotalProbabily > 0 && extBet > 0)
			{
				tmp_vecRedPacketConfig[i].nProbabily[0] = tmp_vecRedPacketConfig[i].nProbabily[0] * extBet / 10;
				TotalProbabily = TotalProbabily + tmp_vecRedPacketConfig[i].nProbabily[0] * extBet / 10 - tmp_vecRedPacketConfig[i].nProbabily[0];
			}
			break;
		}
		TotalProbabily = TotalProbabily + tmp_vecRedPacketConfig[i].nProbabily[0];
	}

	if (TotalProbabily < 1)
	{
		return 0;
	}

	int nRandNum = rand() % TotalProbabily;
	for (size_t i = 0; i < tmp_vecRedPacketConfig.size(); i++)
	{
		nRandNum = nRandNum - tmp_vecRedPacketConfig[i].nProbabily[0];
		if (nRandNum < 0)
		{
			nHBValue = tmp_vecRedPacketConfig[i].nAmountMin;
			break;
		}
	}

	return nHBValue;
}

void CPlayer::getRedPacketBroadcast(int nums, int type)
{
	SERVER_LOG("getRedPacketBroadcast nums=%d nums=%d", nums, type);
	if (nums < g_nRedPacketBCSTTHR)
	{
		return;
	}
	if (type != ITEM_GOLD_TICKET)
	{
		return;
	}

	char buffer[256];
	pt_ss_broadcast_req req;
	req.opcode = ss_broadcast_req;
	sprintf(buffer, "%s|%d|%s|%d|%d", GetCorePlayer()->GetPlyStrAttr(0, PLY_ATTR_NICKNAME), m_nVIPlevel, g_sServerName.c_str(), nums, type);
	req.message_ = buffer;
	req.ply_guid_ = g_nGame_id; // GetCorePlayer()->GetPlyInt64Attr(0, PLY_ATTR_GUID);
	UpdateToPlayer(req);

	SERVER_LOG("getRedPacketBroadcast %s", buffer);
}

int CPlayer::getPayValue()
{
	return 0;
}

int CPlayer::getPayValueRate()
{

	//倍数*10 避免小数
	int payValue = getPayValue() + 10;
	payValue = payValue < 1 ? 1 : payValue;
	int getValue = m_nRedPackets_TotalAward < 1 ? 1 : m_nRedPackets_TotalAward;

	int result = getValue * 10 / payValue;

	return result;
}

void CPlayer::checkRedPacket88YuanData()
{
	if (m_nRedPackets_88Yuan_Status == 1 || m_nRedPackets_88Yuan_Status == 3)
	{
		return;
	}
	if (g_nHB88YuanEnable != 1)
	{
		return;
	}

	m_nRedPackets_88Yuan_CurRound++;

	if (m_nRedPackets_88Yuan_LimitRound > 0 && m_nRedPackets_88Yuan_CurRound >= m_nRedPackets_88Yuan_LimitRound)
	{
		m_nRedPackets_88Yuan_Status = 1;
	}

	sendRedPacket88YuanData();
}

void CPlayer::readRedPacket88YuanData()
{
	if (g_nHBMode < HONGBAO_H5_CHUJI)
	{
		return;
	}
	if (g_nHB88YuanEnable != 1)
	{
		return;
	}

	//�?前只有高级场读取数据�?
	/*
	RPGAME_DATA_GIFT_POOL = 0,   // 奖池
	RPGAME_DATA_CURRENT_ROUND,   // 当前回合
	RPGAME_DATA_REWARD_STATUS,   // 奖励状�?
	RPGAME_DATA_AWARD,           // 领取金�??
	RPGAME_DATA_TOTAL_REWARD,    // 领取红包总�?
	RPGAME_DATA_REWARD_TIMES,    // 红包领取次数
	*/
	m_nRedPackets_88Yuan_AwardPool = GetCorePlayer()->GetPlyInt64Attr(PLY_RP_88FIXED_DATA, RPGAME_DATA_GIFT_POOL);
	m_nRedPackets_88Yuan_CurRound = GetCorePlayer()->GetPlyInt64Attr(PLY_RP_88FIXED_DATA, RPGAME_DATA_CURRENT_ROUND);
	m_nRedPackets_88Yuan_Status = GetCorePlayer()->GetPlyInt64Attr(PLY_RP_88FIXED_DATA, RPGAME_DATA_REWARD_STATUS);
	m_nRedPackets_88Yuan_AwardNum = GetCorePlayer()->GetPlyInt64Attr(PLY_RP_88FIXED_DATA, RPGAME_DATA_AWARD);
	m_nRedPackets_88Yuan_TotalAward = GetCorePlayer()->GetPlyInt64Attr(PLY_RP_88FIXED_DATA, RPGAME_DATA_TOTAL_REWARD);
	m_nRedPackets_88Yuan_TotalAwardCount = GetCorePlayer()->GetPlyInt64Attr(PLY_RP_88FIXED_DATA, RPGAME_DATA_REWARD_TIMES);



	const vector<structRedPacket88Yuan>& tmp_vecRedPacket8YuanConfig = CConfigManager::GetInstancePtr()->m_vecRedPacket88YuanConfig;

	if (m_nRedPackets_88Yuan_AwardNum == 0)
	{
		m_nRedPackets_88Yuan_AwardNum = tmp_vecRedPacket8YuanConfig[0].nAmount;
	}

	for (size_t i = 0; i < tmp_vecRedPacket8YuanConfig.size(); i++) {
		if (m_nRedPackets_88Yuan_AwardNum == tmp_vecRedPacket8YuanConfig[i].nAmount)
		{
			m_nRedPackets_88Yuan_TaskID = tmp_vecRedPacket8YuanConfig[i].cLid;
			m_nRedPackets_88Yuan_LimitRound = tmp_vecRedPacket8YuanConfig[i].nRoundLimit;
		}
	}
	if (m_nRedPackets_88Yuan_LimitRound == 0)
	{

		glog.error("m_nRedPackets_88Yuan_AwardNum[%d], m_nRedPackets_88Yuan_LimitRound not found", m_nRedPackets_88Yuan_AwardNum);
	}

	sendRedPacket88YuanData();
}

void CPlayer::saveRedPacket88YuanData()
{

	//�?前只有高级场写入数据�?
	if (g_nHBMode < HONGBAO_H5_CHUJI)
	{
		return;
	}
	if (g_nHB88YuanEnable != 1)
	{
		return;
	}

	/*
	int64 gift_pool;    // 奖池
	int current_round;  // 当前回合
	int reward_status;  // 奖励状�?
	int64 award;        // 领取金�??
	int64 total_reward; // 领取红包总�?
	int reward_times;   // 红包领取次数
	*/
	pt_ss_rp_save_game_data_req req;
	req.opcode = ss_rp_save_game_data_req;
	req.gift_pool = m_nRedPackets_88Yuan_AwardPool;
	req.current_round = m_nRedPackets_88Yuan_CurRound;
	req.reward_status = m_nRedPackets_88Yuan_Status;
	req.award = m_nRedPackets_88Yuan_AwardNum;
	req.total_reward = m_nRedPackets_88Yuan_TotalAward;
	req.reward_times = m_nRedPackets_88Yuan_TotalAwardCount;
	req.type_ = 1;
	UpdateToPlayer(req);

}

void CPlayer::sendRedPacket88YuanData()
{
	if (g_nHBMode < HONGBAO_H5_CHUJI)
	{
		return;
	}
	if (g_nHB88YuanEnable != 1)
	{
		return;
	}

	pt_gc_get_redpackets_88yuan_award_ack ack;
	ack.cur_rounds_ = m_nRedPackets_88Yuan_CurRound;
	ack.limit_rounds_ = m_nRedPackets_88Yuan_LimitRound;
	ack.task_id_ = m_nRedPackets_88Yuan_TaskID;
	ack.ret_ = m_nRedPackets_88Yuan_Status;
	ack.nAmount = m_nRedPackets_88Yuan_AwardNum;
	ack.cItemtype = ITEM_GOLD_TICKET;

	SendPacket(ack);
}

void CPlayer::sendMagicEmojiConfig()
{
	const vector<MagicEmojiItem>& tempMagicEmojiConfigs = CConfigManager::GetInstancePtr()->m_vecMagicEmojiConfigs;
	if (tempMagicEmojiConfigs.size() == 0)
	{
		return;
	}
	pt_gc_magic_emoji_config_not ack;
	for (int i = 0; i < tempMagicEmojiConfigs.size(); ++i)
	{
		emojiConfig curItem;
		curItem.cEmojiIndex = i;
		curItem.cCostType = tempMagicEmojiConfigs[i].nItemIndex;
		curItem.cCostValue = tempMagicEmojiConfigs[i].nItemConsumeCoins;
		curItem.nTenItemIndex = tempMagicEmojiConfigs[i].nTenItemIndex;
		curItem.nTenItemNum = tempMagicEmojiConfigs[i].nTenItemNum;
		curItem.nTenEmojiNum = tempMagicEmojiConfigs[i].nTenEmojiNum;
		ack.emojiConfigs.push_back(curItem);
	}
	SendPacket(ack);

}

void CPlayer::OnPacket(const pt_cg_look_lord_card_req& req)
{
	m_bLookCard = true;
}

void CPlayer::OnPacket(const pt_cg_look_lord_card_item_req& req)
{
	pt_gc_look_lord_card_item_ack ack;
	ack.opcode = gc_look_lord_card_item_ack;
	if (g_nRPGMode != 1)
	{
		ack.nRet = -1;
	}
	else if (m_bLookCard)
	{
		ack.nRet = -2;
	}
	else if (m_pGameTable && m_pGameTable->m_bRacing && m_pGameTable->m_bLordDeal)
	{
		ack.nRet = -3;
	}
	else if(GetCorePlayer()->GetPlyAttr(ITEM_ATTR, ITEM_DDZ_LOOK_CARD) <= 0)
	{
		ack.nRet = -4;
	}
	else 
	{
		m_bLookCard = true;
		if (m_pGameTable && m_pGameTable->m_bRacing && m_pGameTable->m_Poke.m_bSendCardOK[GetChairID()])
		{
			ack.nRet = 1;
			SendPacket(ack);
			m_pGameTable->SendLookLordCard(this);
			return;
		}

		ack.nRet = 0;
	}

	SendPacket(ack);
}

void CPlayer::OnPacket(const pt_cg_regain_lose_score_req& req) {
	if (req.nOp == 1 && req.nItemNum > 0)
	{
		pt_cg_regain_lose_score_req reqCopy = req;
		getRegainLoseMoney(req.nOp, &reqCopy);
		return;
	}
	getRegainLoseMoney(req.nOp);
}

void CPlayer::OnPacket(const pt_cg_enable_invincible_req& req) {
	int ret = 0;
	do
	{
		if (g_nInvincibleMode != 1)
		{
			return;
		}
		if (isRoundStart())
		{
			ret = -1;
			break;
		}
		if (req.nOp == 1) {
			m_bInvincible = true;
			ret = 1;
			break;
		}
	} while (false);

	pt_gc_enable_invincible_ack ack;
	ack.nRet = ret;
	SendPacket(ack);
}

void CPlayer::OnPacket(const pt_cg_beishu_info_req& req)
{
	if (m_pGameTable)
	{
		m_pGameTable->SendDoubleDetail(this);
	}
}

void CPlayer::setMutiMyDouble(int m)
{
	if (m > 0)
	{
		m_nMyDouble *= m;
	}
	if (m_pGameTable)
	{
		m_pGameTable->setPlayerDouble(GetChairID(), m_nMyDouble);
	}
}

bool CPlayer::superDoubleCheck()
{
	if (GetCorePlayer()->GetPlyAttr(ITEM_ATTR, ITEM_DDZ_SUPER_BET) > 0) {

		IPlayer::PlyAttrItem item_(UP_RN_USE_ITEM, ITEM_DDZ_SUPER_BET, -1);

		notiItemInfo(ITEM_DDZ_SUPER_BET);

		if (GetCorePlayer()) {
			GetCorePlayer()->UpdatePlyAttrs(item_);
			return true;
		}
	}	

	return false;
}

int CPlayer::getMyDouble()
{
	return m_nMyDouble;
}

bool CPlayer::isShowCard()
{
	return m_nShowCard > 0;
}

void CPlayer::setShowCard(int s)
{
	m_nShowCard = s;
}

int CPlayer::getShowCard()
{
	return m_nShowCard;
}

void CPlayer::sendShowCardReq(int type, int bet)
{
	pt_gc_show_card_req req;
	req.opcode = gc_show_card_req;
	// req.nSerialID = m_pGameTable->m_nSerialID;
	req.nSerialID = 0;
	req.nShowCardType = type;
	req.nShowCardBet = bet;
	SendPacket(req);
}

void CPlayer::setCallLordRate(int t)
{
	m_nCallLordRate = t;
}

int CPlayer::getCallLordRate()
{
	return m_nCallLordRate;
}

void CPlayer::genRegainLoseMoney(int score)
{
	if (g_nRegainLoseCountLimit < 1)
	{
		return;
	}

	const char* guid = m_pCorePlayer->GetPlyStrAttr(0, PLY_ATTR_GUID);
	if (CConfigManager::GetInstancePtr()->isRobotGuid(guid))
	{
		return;
	}

	int nRegainLoseFreeCount = 0;
	if (g_nRegainLoseMAXCount > 0 && m_rls_game_data.last_lose_money < 20000000) {
		int nRegisteTime = GetCorePlayer()->GetPlyAttr(PLY_BASE_ATTR, PLY_REGISTER_TIME);
		nRegainLoseFreeCount = leaf::GetDay() == leaf::GetDay(nRegisteTime) ? g_nRegainLoseFreeCountNew : g_nRegainLoseFreeCount;
	}
	if (g_nRegainLoseMAXCount <= 0 || m_rls_game_data.total_count >= (g_nRegainLoseMAXCount + nRegainLoseFreeCount))
	{
		return;
	}

	if (score < 0) {

		m_rls_game_data.last_lose_money = score * -1;
		m_rls_game_data.last_lose_time = (int)time(0);
		
		if (m_rls_game_data.last_lose_money >= 20000000) {
			m_rls_game_data.last_regain_count = g_nRegainLoseCountLimit;
		} else {
			m_rls_game_data.last_regain_count = 1;
		}

		saveRegainLoseMoney(2);
		// getRegainLoseMoney(0);
	}
	else if (m_rls_game_data.last_regain_count > 0 && m_rls_game_data.last_lose_money > 0) {
		getRegainLoseMoney(0);
	}
}

void CPlayer::getRegainLoseMoney(int op, pt_cg_regain_lose_score_req* req)
{
	int ackRet = 0;
	vector<int> ackValue;
	int ackTime = 0;
	do {
		if (g_nRegainLoseCountLimit < 1) {
			ackRet = -1;  // 没开�?功能
			return;
		}

		ackTime = g_nRegainLoseTimeLimit - (int)time(0) + m_rls_game_data.last_lose_time ;		

		int nRegainLoseFreeCount = 0;
		if (g_nRegainLoseMAXCount > 0 && m_rls_game_data.last_lose_money < 20000000) {
			int nRegisteTime = GetCorePlayer()->GetPlyAttr(PLY_BASE_ATTR, PLY_REGISTER_TIME);
			nRegainLoseFreeCount = leaf::GetDay() == leaf::GetDay(nRegisteTime) ? g_nRegainLoseFreeCountNew : g_nRegainLoseFreeCount;
		}
		if (m_rls_game_data.total_count < 0 || g_nRegainLoseMAXCount <= 0 || m_rls_game_data.total_count >= (g_nRegainLoseMAXCount + nRegainLoseFreeCount))
		{
			ackRet = -7;	//每日次数用完�?
			break;
		}
			
		if(m_rls_game_data.last_lose_money >= 20000000) {
			ackValue = g_nRegainLoseMoneyLimit;
		} else if (m_rls_game_data.last_lose_money > 0) {
			int value = m_rls_game_data.last_lose_money * 0.8;
			value = min(value, 4000000);
			value = max(value, 20000);
			ackValue.push_back(value);
		}

		if (op != 1) {
			if (ackTime < 0) {
				ackRet = -6;
				break;
			}
			if (m_rls_game_data.last_lose_money <= 0)
			{
				ackRet = -8;
				break;
			}
			if (m_rls_game_data.total_count < nRegainLoseFreeCount)
			{
				ackRet = 2;
				break;
			}
			break;
		}		

		if (isRoundStart()) {
			ackRet = -2;  // 游戏开始了
			break;
		}

		if (m_rls_game_data.last_regain_count < 1) {
			ackRet = -3;  // 没有次数
			break;
		}

		if (ackTime < 0) {
			ackRet = -4; // 超出时间
			break;
		}

		if (m_rls_game_data.last_lose_money < 1) {
			ackRet = -5;  // 领完�?
			break;
		}

		// < 2kw 领取80% 一次�?�取�?
		// >= 2kw 领取0.8kw + 1.2kw 分两次�?�取
		ackRet = 1;

	} while (false);

	int curValueIdx = ackValue.size() - m_rls_game_data.last_regain_count;

	pt_gc_regain_lose_score_ack ack;
	ack.nItemIndex = 0;
	ack.nItemNum = 0;
	if (ackRet == 1) {
		
		m_rls_game_data.last_regain_count--;

		if (m_rls_game_data.last_regain_count == 0) {
			m_rls_game_data.last_lose_money = 0;
		}

		char buffer[512];
		sprintf(buffer, "%lld|%d|%d%d", m_rls_game_data.last_lose_money, m_rls_game_data.last_regain_count, (int)time(0), rand() % 10000);

		IPlayer::PlyAttrItem item_(UP_RN_REGAIN_LOSE_MONEY, PLY_ATTR_MONEY, ackValue[curValueIdx], 0, buffer);
		if (GetCorePlayer()) {
			GetCorePlayer()->UpdatePlyAttrs(item_);
		}

		// Warn 根据客户�?发送道�? 不做验证
		if (req != NULL && req->nItemNum > 0)
		{
			buffer[512];
			sprintf(buffer, "RegainLose|%ld|%d|%d%d", req->nItemIndex, req->nItemNum, (int)time(0), rand() % 10000);
			IPlayer::PlyAttrItem item_(UP_RN_SPECIFY_ITEM, req->nItemIndex, req->nItemNum, 0, buffer);
			if (GetCorePlayer()) {
				GetCorePlayer()->UpdatePlyAttrs(item_);
			}

			ack.nItemIndex = req->nItemIndex;
			ack.nItemNum = req->nItemNum;
		}
		saveRegainLoseMoney(1);
	}
	else {
		if (ackRet == -3 || ackRet == -4 || ackRet == -5)
		{
			m_rls_game_data.last_regain_count = 0;
			m_rls_game_data.last_lose_money = 0;
		}
	}

	ack.nRet = ackRet;
	ack.nValue = ackValue;
	ack.nTime = ackTime;
	ack.nCurCount = curValueIdx;
	SendPacket(ack);
}

void CPlayer::readRegainLoseMoney()
{
	if (m_rls_game_data.total_count < 0) {
		saveRegainLoseMoney(0);
	} else {
		getRegainLoseMoney(0);
	}
}

void CPlayer::saveRegainLoseMoney(int op_)
{
	pt_ss_update_regain_lose_score_req req;
	req.opcode = ss_update_regain_lose_score_req;
	req.op_ = op_;
	req.last_lose_money_ = m_rls_game_data.last_lose_money;
	req.last_lose_time_ = m_rls_game_data.last_lose_time;
	req.last_regain_count_ = m_rls_game_data.last_regain_count;

	UpdateToPlayer(req);
}

bool CPlayer::isRoundStart()
{
	return m_nisRoundStart;
}

bool CPlayer::getInvincibleStatus()
{
	return m_bInvincible;
}

void CPlayer::genWinDouble()
{
	if (g_nWinDoubleCan == 0)
	{
		return;
	}

	if (m_nRedPackets_Status == REDPACKET_STATUS_READY)
	{
		return;
	}

	if (m_nGameScore <= 0)
	{
		return;
	}

	if (g_nWinDoubleMaxAmount > 0 && m_nGameScore >= g_nWinDoubleMaxAmount)
	{
		return;
	}

	pt_gc_win_doubel_req req;
	req.opcode = gc_win_doubel_req;
	req.nAddAmount = g_nWinDoubleAddAmount;
	req.nAddProbabily = g_nWinDoubleAddProbabily;
	SendPacket(req);
}

void CPlayer::OnPacket(const pt_cg_win_doubel_req& req)
{
	pt_gc_win_doubel_ack ack;
	ack.opcode = gc_win_doubel_ack;
	if (g_nWinDoubleCan == 0)
	{
		ack.cRet = -1;
	}
	else if (m_nGameScore <= 0)
	{
		ack.cRet = -2;
	}
	else
	{
		ack.cRet = 0;

		ItemInfo info;
		info.nItemIndex = ITEM_MONEY;
		info.nItemNum = m_nGameScore;
		
		if (g_nWinDoubleMaxAmount > 0)
		{
			if (g_nWinDoubleMaxDouble > 0)
			{
				int curDouble = min(g_nWinDoubleMaxDouble, int(g_nWinDoubleMaxAmount / m_nGameScore));
				if (curDouble > 2)
				{
					info.nItemNum *= rand() % (curDouble - 2) + 2;
				}
				else if (curDouble == 2)
				{
					info.nItemNum *= curDouble;
				}
			}
			if (info.nItemNum > g_nWinDoubleMaxAmount)
			{
				info.nItemNum = g_nWinDoubleMaxAmount;
			}
		}
		else if (g_nWinDoubleAddAmount > 0 && g_nWinDoubleAddProbabily > 0 && rand() % 100 < g_nWinDoubleAddProbabily)
		{
			info.nItemNum += g_nWinDoubleAddAmount;
		}
		ack.vecItemInfo.push_back(info);

		// 加钱
		char buffer[512];
		sprintf(buffer, "%lld|%d|%d%d", m_nGameScore, g_nWinDoubleAddAmount, (int)time(0), rand() % 10000);
		IPlayer::PlyAttrItem item_(UP_RN_GAME_WIN_DDOBULE, PLY_ATTR_MONEY, (int)info.nItemNum, 0, buffer);
		GetCorePlayer()->UpdatePlyAttrs(item_);

		m_nGameScore = 0;
	}
	// 通知
	SendPacket(ack);
}

void CPlayer::SendBaiYuanHBRound()
{
	pt_gc_baiyuan_hb_round_not noti;
	noti.opcode = gc_baiyuan_hb_round_not;
	noti.nCurRound = m_nBaiYuanHBCurRound;
	noti.nLimitRound = g_nBaiYuanHBRound;
	SendPacket(noti);
}

void CPlayer::SendBaiYuanHBRoundAward()
{
	if (m_nBaiYuanHBCurRound < g_nBaiYuanHBRound)
	{
		return;
	}

	vector<structRewardInfo> vecReward;
	vector<int> vecIds;
	for (int i = g_nBaiYuanHBAwardStartId; i <= g_nBaiYuanHBAwardEndId; i++)
	{
		vecIds.push_back(i);
	}
	CConfigManager::GetInstancePtr()->getRewardCondition(vecReward, vecIds, getItemNum(ITEM_BY_CASH));
	if (vecReward.size() == 0)
	{
		return;
	}

	for (size_t i = 0; i < vecReward.size(); i++)
	{
		item_info info;
		info.nItemId = vecReward[i].nItemId;
		info.nItemNum = vecReward[i].nItemNum;
		m_vecBaiYuanHBRoundAward.push_back(info);
	}

	pt_gc_baiyuan_hb_round_award_not noti;
	noti.opcode = gc_baiyuan_hb_round_award_not;
	noti.vecItemInfo = m_vecBaiYuanHBRoundAward;
	SendPacket(noti);
}

void CPlayer::SendBaiYuanWinDouble()
{
	if (m_nGameScore <= 0)
	{
		return;
	}

	if (!CConfigManager::GetInstancePtr()->isToggleCondition(TC_WinDouble, getItemNum(ITEM_BY_CASH)))
	{
		return;
	}

/*
	int nDouble = g_nBaiYuanWinDoubleMin;
	if (nDouble != g_nBaiYuanWinDoubleMax)
	{
		nDouble += rand() % (g_nBaiYuanWinDoubleMax - g_nBaiYuanWinDoubleMin);
	}*/
	item_info info;
	info.nItemId = ITEM_BY_CASH;
	//info.nItemNum = m_nGameScore * nDouble / 100;
	info.nItemNum = m_nGameScore * 2; // ��Ԫ����ÿ�η���
	m_vecBaiYuanWinDouble.push_back(info);
	
	info.nItemNum += m_nGameScore;

	pt_gc_baiyuan_win_double_not noti;
	noti.opcode = gc_baiyuan_win_double_not;
	noti.vecItemInfo.push_back(info);
	SendPacket(noti);
}

void CPlayer::SendBaiYuanRegainLose()
{
	if (m_nGameScore >= 0)
	{
		return;
	}

/*
	if (!CConfigManager::GetInstancePtr()->isToggleCondition(TC_RegainLose, getItemNum(ITEM_BY_CASH)))
	{
		return;
	}
*/

	item_info info;
	info.nItemId = ITEM_BY_CASH;
	info.nItemNum = (int)-m_nGameScore;
	m_vecBaiYuanRegainLose.push_back(info);
	
	pt_gc_baiyuan_regain_lose_not noti;
	noti.opcode = gc_baiyuan_regain_lose_not;
	noti.vecItemInfo = m_vecBaiYuanRegainLose;
	SendPacket(noti);
}

void CPlayer::SendBaiYuanLuckWelfare()
{
	if (m_pGameTable == NULL)
	{
		return;
	}

	if (!CConfigManager::GetInstancePtr()->isToggleCondition(TC_LuckWelfare, getItemNum(ITEM_BY_CASH)))
	{
		return;
	}

	item_info info;
	info.nItemId = ITEM_BY_CASH;
	info.nItemNum = m_pGameTable->m_nBaseScore * m_pGameTable->m_nCallScore;
	m_vecBaiYuanLuckWelfare.push_back(info);

	pt_gc_baiyuan_luck_welfare_not noti;
	noti.opcode = gc_baiyuan_luck_welfare_not;
	noti.vecItemInfo = m_vecBaiYuanLuckWelfare;
	SendPacket(noti);
}

int CPlayer::getItemNum(ITEM_INDEX index)
{
	if (m_pCorePlayer)
	{
		return m_pCorePlayer->GetPlyAttr(ITEM_ATTR, index);
	}

	return 0;
}

void CPlayer::updatePlayerItems(vector<item_info>& vecItems, const char* buffer)
{
	for (size_t i = 0; i < vecItems.size(); i++)
	{
		IPlayer::PlyAttrItem item_(UP_RN_SPECIFY_ITEM, vecItems[i].nItemId, vecItems[i].nItemNum, 0, buffer);
		GetCorePlayer()->UpdatePlyAttrs(item_);
	}
}

void CPlayer::OnPacket(const pt_cg_baiyuan_hb_round_award_req& req)
{
	pt_gc_baiyuan_hb_round_award_ack ack;
	ack.opcode = gc_baiyuan_hb_round_award_ack;
	if (m_vecBaiYuanHBRoundAward.size() == 0)
	{
		ack.cRet = -1;
	}
	else
	{
		ack.cRet = 0;
		ack.vecItemInfo = m_vecBaiYuanHBRoundAward;

		m_vecBaiYuanHBRoundAward.clear();
		m_nBaiYuanHBCurRound = 0;

		// 加钱
		char buffer[512];
		sprintf(buffer, "baiyuan_hb_round_award item:%d", getItemNum(ITEM_BY_CASH));
		updatePlayerItems(ack.vecItemInfo, buffer);
	}
	SendPacket(ack);
	SendBaiYuanHBRound();
}

void CPlayer::OnPacket(const pt_cg_baiyuan_win_double_req& req)
{
	pt_gc_baiyuan_win_double_ack ack;
	ack.opcode = gc_baiyuan_win_double_ack;
	if (m_vecBaiYuanWinDouble.size() == 0)
	{
		ack.cRet = -1;
	}
	else
	{
		ack.cRet = 0;
		ack.vecItemInfo = m_vecBaiYuanWinDouble;

		m_vecBaiYuanWinDouble.clear();

		// 加钱
		char buffer[512];
		sprintf(buffer, "baiyuan_win_double item:%d score:%lld", getItemNum(ITEM_BY_CASH), m_nGameScore);
		updatePlayerItems(ack.vecItemInfo, buffer);
/*

		if (ack.vecItemInfo[0].nItemId == ITEM_BY_CASH && m_nGameScore > 0)
		{
			ack.vecItemInfo[0].nItemNum += m_nGameScore;
		}*/
	}
	SendPacket(ack);
}

void CPlayer::OnPacket(const pt_cg_baiyuan_regain_lose_req& req)
{
	pt_gc_baiyuan_regain_lose_ack ack;
	ack.opcode = gc_baiyuan_regain_lose_ack;
	if (m_vecBaiYuanRegainLose.size() == 0)
	{
		ack.cRet = -1;
	}
	else
	{
		ack.cRet = 0;
		ack.vecItemInfo = m_vecBaiYuanRegainLose;

		m_vecBaiYuanRegainLose.clear();

		// 加钱
		char buffer[512];
		sprintf(buffer, "baiyuan_regain_lose item:%d score:%lld", getItemNum(ITEM_BY_CASH), m_nGameScore);
		updatePlayerItems(ack.vecItemInfo, buffer);
	}
	SendPacket(ack);
}

void CPlayer::OnPacket(const pt_cg_baiyuan_luck_welfare_req& req)
{
	pt_gc_baiyuan_luck_welfare_ack ack;
	ack.opcode = gc_baiyuan_luck_welfare_ack;
	if (m_vecBaiYuanLuckWelfare.size() == 0)
	{
		ack.cRet = -1;
	}
	else
	{
		ack.cRet = 0;
		ack.vecItemInfo = m_vecBaiYuanLuckWelfare;

		m_vecBaiYuanLuckWelfare.clear();

		// 加钱
		char buffer[512];
		sprintf(buffer, "baiyuan_luck_welfare item:%d", getItemNum(ITEM_BY_CASH));
		updatePlayerItems(ack.vecItemInfo, buffer);
	}
	SendPacket(ack);
}

void CPlayer::OnPacket(const pt_cg_baiyuan_can_bankruptcy_defend_req& req)
{
	pt_gc_baiyuan_can_bankruptcy_defend_ack ack;
	ack.opcode = gc_baiyuan_can_bankruptcy_defend_ack;

	int num = getItemNum(ITEM_BY_CASH);
	if (g_nBaiYuanBankruptcyMin >= 0 && num < g_nBaiYuanBankruptcyMin)
	{
		int nScore = g_nBaiYuanBankruptcyMax - num;

		item_info info;
		info.nItemId = ITEM_BY_CASH;
		info.nItemNum = nScore;

		ack.cRet = 0;
		ack.vecItemInfo.push_back(info);
	}
	else
	{
		ack.cRet = -1;
	}
	SendPacket(ack);
}

void CPlayer::OnPacket(const pt_cg_baiyuan_bankruptcy_defend_req& req)
{
	pt_gc_baiyuan_bankruptcy_defend_ack ack;
	ack.opcode = gc_baiyuan_bankruptcy_defend_ack;

	int num = getItemNum(ITEM_BY_CASH);
	if (g_nBaiYuanBankruptcyMin >= 0 && num < g_nBaiYuanBankruptcyMin)
	{
		int nScore = g_nBaiYuanBankruptcyMax - num;

		item_info info;
		info.nItemId = ITEM_BY_CASH;
		info.nItemNum = nScore;

		ack.cRet = 0;
		ack.vecItemInfo.push_back(info);

		// 加钱
		char buffer[512];
		sprintf(buffer, "baiyuan_bankruptcy_subsidy item:%d min:%d max:%d", num, g_nBaiYuanBankruptcyMin, g_nBaiYuanBankruptcyMax);
		updatePlayerItems(ack.vecItemInfo, buffer);
	}
	else
	{
		ack.cRet = -1;
	}
	SendPacket(ack);
}
