/*
CopyRight(c)2006 by Banding,Shanghai, All Right Reserved.

@Date:	        2009/4/15
@Created:	    15:03
@Filename: 	GameTable.cpp
@File path:	BDLeyoyoV2\DDZV2\Server
@Author:		leiliang

@Description:	房间类，主�?�负责�?�理绑定，响应事件，收发消息
*/

#include "stdafx.h"
#include "Player.h"
#include "GameTable.h"
#include "pt_cg_auto_req_handler.h"
#include "pt_cg_call_score_ack_handler.h"
#include "pt_cg_complete_data_req_handler.h"
#include "pt_cg_play_card_ack_handler.h"
#include "pt_cg_rob_lord_ack_handler.h"
#include "pt_cg_send_card_ok_ack_handler.h"
#include "pt_cg_show_card_ack_handler.h"
#include "pt_cg_double_score_ack_handler.h"
#include "sql_define.h"
#include "EasyJson.h"



int g_nTaskID = -1;
CGameTable::CGameTable() 
	:m_nTax(0), 
	m_nSerialID(0), 
	m_bRacing(false), 
	m_nDouble(1), 
	m_nAddFan(1), 
	m_nAddFan2(1),  
	m_bLordDeal(false), 
	m_bStart(false), 
	m_nRobLord(g_nRobLord), 
	m_nNoCallNum(0), 
	m_startPauseTime(0), 
	m_nSettleType(-1), 
	m_bHadStart(false), 
	m_nMaJiangJu(0), 
	m_bRoomChecked(false),
	m_vecDoubleDetail(vector<int>(BEI_SHU_INFO_END, 1)),
	m_vecPlayerDouble(vector<int>(CConfigManager::GetInstancePtr()->GetPlayNum(), 1)) {
	SERVER_LOG("CGameTable::CGameTable()");
	m_nPlay_Num = CConfigManager::GetInstancePtr()->GetPlayNum();
	m_sGameStatic.resize(m_nPlay_Num);
	for (int i = 0; i < m_nPlay_Num; i++)
	{
		m_sGameStatic[i].cChairID = i;
	}

	m_nMaxDouble = g_nMaxDouble;
	m_nMustCallSeat = -1;
	m_nShuffleLogType = -1;
	m_nShuffleLogBetterSeat = -1;
	m_nShuffleLogRobotSeat = -1;

	setDailyCountVal();//设置优势位权�?
	
}

CGameTable::~CGameTable()
{
	SERVER_LOG("CGameTable::~CGameTable()");
}

void CGameTable::StartTimer(char cChair, int nEvent)
{
	SERVER_LOG(" CGameTable::StartTimer(cChair:%d,nEvent:%d)", cChair, nEvent);
	m_pCoreTable->RemoveTimer();
	m_currentTime_ = nEvent;
	if (nEvent == eDEALCARD_ANIMATE_EVENT)
	{
		m_pCoreTable->SetTimer((int)eANI_PERIOD, (void*)eDEALCARD_ANIMATE_EVENT);
	}
	else if (nEvent == eSHOWANI_EVENT)
	{
		m_pCoreTable->SetTimer((int)eSHOWANI_PERIOD, (void*)eSHOWANI_EVENT);
	}
	else if (nEvent == eCALLSCORE_EVENT)
	{
		int nCallScoreTime = this->is_standard_game() ? eSTANDARD_CALL_PERIOD : eCALL_PERIOD;
		ClientTimerNot(cChair, nCallScoreTime);
		m_pCoreTable->SetTimer((int)nCallScoreTime, (void*)eCALLSCORE_EVENT);
	}
	else if (nEvent == eROBLORD_EVENT)
	{
		ClientTimerNot(cChair, eROB_PERIOD);
		m_pCoreTable->SetTimer((int)eROB_PERIOD, (void*)eROBLORD_EVENT);
	}
	else if (nEvent == eDOUBLE_EVENT)
	{
		ClientTimerNot(cChair, eDOUBLE_PERIOD);
		m_pCoreTable->SetTimer((int)eDOUBLE_PERIOD, (void*)eDOUBLE_EVENT);
	}
	// 标准模式下的防守者加倍过�?
	else if (nEvent == eSTANDARD_DEFENDER_DOUBLE_EVENT)
	{
		for (int nIndex = 0; nIndex < GetPlayNum(); nIndex++)
		{
			if (nIndex != m_Poke.m_nCurrentLord)
			{
				CPlayer* pPlayer = GetPlayer(nIndex);
				if (pPlayer)
				{
					ClientTimerNot(nIndex, eSTANDARD_DEFENDER_DOUBLE_PERIOPD);
				}
			}
		}

		m_pCoreTable->SetTimer((int)eSTANDARD_DEFENDER_DOUBLE_PERIOPD, (void*)eSTANDARD_DEFENDER_DOUBLE_EVENT);
	}
	// 标准模式下的庄�?�加倍过�?
	else if (nEvent == eSTANDARD_BANKER_DOUBLE_EVENT)
	{
		ClientTimerNot(cChair, eSTANDARD_DEFENDER_DOUBLE_PERIOPD);
		m_pCoreTable->SetTimer((int)eSTANDARD_DEFENDER_DOUBLE_PERIOPD, (void*)eSTANDARD_BANKER_DOUBLE_EVENT);
	}
	else if (nEvent == ePLAYCARD_EVENT)
	{
		CPlayer* pPlayer = GetPlayer(cChair);
		if (pPlayer)
		{
			int nPeriod = ePLAY_PERIOD;
			if (cChair == m_Poke.m_nCurrentLord && pPlayer->m_nPutCount == 0)
			{
				if (g_nFirstPlayCardTime > 0)
				{
					nPeriod = g_nFirstPlayCardTime * 1000;
				}
				else
				{
					nPeriod = eFIRST_PLAY_PERIOD;
				}
			}
			else if (g_nPlayCardTime > 0)
			{
				nPeriod = g_nPlayCardTime * 1000;
			}
			ClientTimerNot(cChair, nPeriod);
			m_pCoreTable->SetTimer((int)(pPlayer->m_bAuto ? ePLAYCARD_AUTO_PERIOD : nPeriod), (void*)ePLAYCARD_EVENT);
		}
	}
	else if (nEvent == eSHOWCARD_EVENT)
	{
		ClientTimerNot(cChair, eSHOW_PERIOD);
		m_pCoreTable->SetTimer((int)eSHOW_PERIOD, (void*)eSHOWCARD_EVENT);
	}
	else if (nEvent == ePLAYCARD_AUTO_EVENT)
	{
		m_pCoreTable->SetTimer((int)ePLAYCARD_AUTO_PERIOD, (void*)ePLAYCARD_AUTO_EVENT);
	}
	else if (nEvent == eREADY_EVENT)
	{
		m_pCoreTable->SetTimer((int)eREADY_PERIOD, (void*)eREADY_EVENT);
	}
	else if (nEvent == eREADY_PAUSE_EVENT)
	{
		m_pCoreTable->SetTimer((int)ePAUSE_GAME_PERIOD, (void*)eREADY_PAUSE_EVENT);
	}
}

void CGameTable::OnTimer(void* pParam)
{
	int nEvent = (int)pParam;
	SERVER_LOG("CGameTable::OnTimer(nEvent:%d)", nEvent);
	if (ePLAYCARD_AUTO_EVENT != nEvent && g_nPauseTime == 0)
	{
		SendCommonCmd(CO_TIMEOUT);
	}
	if (eDEALCARD_ANIMATE_EVENT == nEvent)			//发牌超时
	{
		pt_cg_send_card_ok_ack ack;
		ack.opcode = cg_send_card_ok_ack;
		ack.nSerialID = m_nSerialID;
		for (int i = 0; i < GetPlayNum(); i++)
		{
			if (!m_Poke.m_bSendCardOK[i])
			{
				CPlayer* pPlayer = GetPlayer(i);
				if (pPlayer)
				{
					pt_cg_send_card_ok_ack_handler::handler(ack, pPlayer);
				}
			}
		}
	}
	else if (ePAUSE_GAME_EVENT == nEvent)
	{
		OnPauseGame();
	}
	else if (eREADY_EVENT == nEvent)
	{
		OnReadyTimeOut();
	}
	else if (eREADY_PAUSE_EVENT == nEvent)
	{
		SendReadyTimeOutNot();
	}
	else if (eSHOWANI_EVENT == nEvent)	//明牌动画结束
	{
		//地主出牌
		SendCommonCmd(CO_NEW);
		m_nWaitOpChairID = m_Poke.m_nCurrentLord;
		m_nPutCardsPlayerID = m_nWaitOpChairID;
		SvrPlayCardReq(m_nWaitOpChairID);
	}
	else if (eCALLSCORE_EVENT == nEvent)	//�?分超�?
	{
		if (g_nPauseTime > 0)
		{
			m_startPauseTime = (int)time(0);
			m_pCoreTable->RemoveTimer();
			OnPauseGame();
			return;
		}

		CPlayer* pPlayer = GetPlayer(m_nWaitOpChairID);
		if (pPlayer)
		{
			pt_cg_call_score_ack ack;
			ack.opcode = cg_call_score_ack;
			ack.nScore = 0;
			ack.nSerialID = m_nSerialID;
			pt_cg_call_score_ack_handler::handler(ack, pPlayer);
		}
	}
	else if (eROBLORD_EVENT == nEvent)		//抢地主超�?
	{
		if (g_nPauseTime > 0)
		{
			m_startPauseTime = (int)time(0);
			m_pCoreTable->RemoveTimer();
			OnPauseGame();
			return;
		}

		CPlayer* pPlayer = GetPlayer(m_nWaitOpChairID);
		if (pPlayer)
		{
			pt_cg_rob_lord_ack ack;
			ack.opcode = cg_rob_lord_ack;
			ack.cRob = 0;
			ack.nSerialID = m_nSerialID;
			pt_cg_rob_lord_ack_handler::handler(ack, pPlayer);
		}
	}
	else if (eDOUBLE_EVENT == nEvent)//加倍超�?
	{
		if (g_nPauseTime > 0)
		{
			m_startPauseTime = (int)time(0);
			m_pCoreTable->RemoveTimer();
			OnPauseGame();
			return;
		}

		if (g_nDoubleScore == 2) {
			for (int nIndex = 0; nIndex < GetPlayNum(); nIndex++) {
				CPlayer* pPlayer = GetPlayer(nIndex);
				if (pPlayer && !pPlayer->m_bIsDouble) {
					pt_cg_double_score_ack noti;
					noti.opcode = cg_double_score_ack;
					noti.nScore = 0;
					noti.nSerialID = m_nSerialID;
					pt_cg_double_score_ack_handler::handler(noti, pPlayer);
				}
			}
		}else {
			CPlayer* pPlayer = GetPlayer(m_nWaitOpChairID);
			if (pPlayer)
			{
				pt_cg_double_score_ack noti;
				noti.opcode = cg_double_score_ack;
				noti.nScore = 0;
				noti.nSerialID = m_nSerialID;
				pt_cg_double_score_ack_handler::handler(noti, pPlayer);
			}
		}
	}
	// 标准模式下防守者加倍超�?
	else if (eSTANDARD_DEFENDER_DOUBLE_EVENT == nEvent)
	{
		for (int nIndex = 0; nIndex < GetPlayNum(); nIndex++)
		{
			if (nIndex != m_Poke.m_nCurrentLord)
			{
				CPlayer* pPlayer = GetPlayer(nIndex);
				if (pPlayer)
				{
					pt_cg_double_score_ack noti;
					noti.opcode = cg_double_score_ack;
					noti.nScore = 0;
					noti.nSerialID = m_nSerialID;
					pt_cg_double_score_ack_handler::handler(noti, pPlayer);
				}
			}
		}
	}
	// 标准模式庄�?�加倍超�?
	else if (eSTANDARD_BANKER_DOUBLE_EVENT == nEvent)
	{
		CPlayer* pPlayer = GetPlayer(m_Poke.m_nCurrentLord);
		if (pPlayer)
		{
			pt_cg_double_score_ack noti;
			noti.opcode = cg_double_score_ack;
			noti.nScore = 0;
			noti.nSerialID = m_nSerialID;
			pt_cg_double_score_ack_handler::handler(noti, pPlayer);
		}
	}

	else if (ePLAYCARD_EVENT == nEvent || ePLAYCARD_EVENT1 == nEvent)	//出牌超时
	{
		if (g_nPauseTime > 0)
		{
			m_startPauseTime = (int)time(0);
			m_pCoreTable->RemoveTimer();
			OnPauseGame();
			return;
		}

		CPlayer* pPlayer = GetPlayer(m_nWaitOpChairID);
		if (pPlayer)
		{
			//超时两�?�自动进入拖管状�?
			if (++pPlayer->m_nTimeOut > (g_nTimeOut - 1) && !pPlayer->m_bAuto)
			{
				SERVER_LOG("CGameTable::OnTimer m_nTimeOut:%d", pPlayer->m_nTimeOut);
				pt_cg_auto_req req;
				req.opcode = cg_auto_req;
				req.cAuto = 1;
				pt_cg_auto_req_handler::handler(req, pPlayer);
			}

			pt_cg_play_card_ack ack;
			ack.opcode = cg_play_card_ack;
			ack.nSerialID = m_nSerialID;
			ack.cTimeOut = 1;
			if (m_nWaitOpChairID == m_nPutCardsPlayerID)	//如果�?首�?�出�?
			{
				pPlayer->m_PlayCard.Tips();
				if (pPlayer->m_PlayCard.m_vecTipsCards.size() > 0)
				{
					pPlayer->m_PlayCard.SelectCard(pPlayer->m_PlayCard.m_vecTipsCards[0]);
					for (size_t t = 0; t < pPlayer->m_PlayCard.m_cChoosingCards.size(); t++)
					{
						ack.vecCards.push_back(pPlayer->m_PlayCard.m_cChoosingCards[t]);
					}
				}
			}
			else
			{
				pPlayer->m_PlayCard.Tips(GetPlayer(m_nPutCardsPlayerID)->m_PlayCard.m_cDiscardingType);
				if (pPlayer->m_PlayCard.m_vecTipsCards.size()>0)
				{
					pPlayer->m_PlayCard.SelectCard(pPlayer->m_PlayCard.m_vecTipsCards[0]);
					for (size_t t = 0; t < pPlayer->m_PlayCard.m_cChoosingCards.size(); t++)
					{
						ack.vecCards.push_back(pPlayer->m_PlayCard.m_cChoosingCards[t]);
					}
				}
			}
			pt_cg_play_card_ack_handler::handler(ack, pPlayer);
		}
	}
	else if (eSHOWCARD_EVENT == nEvent)	//�?牌超�?
	{
		CPlayer* pPlayer = GetPlayer(m_nWaitOpChairID);
		if (pPlayer)
		{
			pt_cg_show_card_ack ack;
			ack.opcode = cg_show_card_ack;
			ack.nSerialID = m_nSerialID;
			ack.cShowCard = 0;
			pt_cg_show_card_ack_handler::handler(ack, pPlayer);
		}
	}
	else if (ePLAYCARD_AUTO_EVENT == nEvent)
	{
		CPlayer* pPlayer = GetPlayer(m_nWaitOpChairID);
		if (pPlayer)
		{
			pt_cg_play_card_ack_handler::handler(ack_, pPlayer);
		}
	}
}

void CGameTable::OnUserJoin(IPlayerDelegate* pPlayer)
{
	SERVER_LOG("CGameTable::OnUserJoin(pPlayer:%p)", pPlayer);
	CPlayer* cPlayer = (CPlayer*)pPlayer;
	if (cPlayer)
	{
		cPlayer->SetStatus(CPlayer::PS_PLAYER);
		cPlayer->SetGameTable(this);

		if (g_nCounts == 1)
		{
			int n_counts = cPlayer->GetCorePlayer()->GetPlyAttr(ITEM_ATTR, ITEM_CARD_RECORD);
			pt_gc_counts_not noti;
			noti.opcode = gc_counts_not;
			noti.counts_num_ = n_counts;
			cPlayer->SendPacket(noti);

			pt_gc_counts_not1 noti1;
			noti1.opcode = gc_counts_not1;
			noti1.counts_num_ = n_counts;
			cPlayer->SendPacket(noti1);
		}

		int expression_ = cPlayer->GetCorePlayer()->GetPlyAttr(ITEM_ATTR, ITEM_EXPRESSION_PARCEL);
		pt_gc_expression_not noti;
		noti.opcode = gc_expression_not;
		noti.expression_num_ = 0;
		if (expression_ > 0)
			noti.expression_num_ = 1;
		noti.expression_type_ = EXPRESSION_CAT;
		cPlayer->SendPacket(noti);

		cPlayer->sendMagicEmojiConfig();
		cPlayer->readRedPacketsData();
		cPlayer->readRedPacket88YuanData();
		cPlayer->readRedPacketsNewbieData();
	}
	m_nAddFan = 1;
}

void CGameTable::OnUserJoined(IPlayerDelegate* pPlayer)
{
	SERVER_LOG("CGameTable::OnUserJoined()");
	SetRoomConfig();
}

void CGameTable::SetRoomConfig()
{
	if (g_nPrivateRoom != 1 || m_bRoomChecked)
	{
		return;
	}
	m_bRoomChecked = true;

	m_nRobLord = g_nRobLord;

	string strJson = m_pCoreTable->GetGameJson();
	RemoveString(strJson, " ");
	SERVER_LOG("strJson:%s", strJson.c_str());

	m_nMaxDouble = GetJsonValue(strJson, "max_beishu");
	if (GetJsonValue(strJson, "game_rule") == 1)
	{
		m_nRobLord = 0;
	}
}

void CGameTable::BindCoreTable2Table(ITable* pTable)
{
	SERVER_LOG("CGameTable::BindCoreTable2Table(pTable:%p)", pTable);
	m_pCoreTable = pTable;
}

void CGameTable::Release()
{
	SERVER_LOG("CGameTable::Release()");
	delete this;
}

void CGameTable::SetBaseMoney()
{
	m_nBaseScore = m_pCoreTable->GetBaseScore();
	m_nSettleType = (m_nBaseScore > 0) ? 0 : 1;
	m_nTax = (m_nBaseScore > 0) ? g_nTax : 0;
	glog.log("SetBaseMoney m_nBaseScore:%d m_nSettleType:%d m_nTax:%d g_nTax:%d", m_nBaseScore, m_nSettleType, m_nTax, g_nTax);
	if (g_nPrivateRoom == 0 && g_nDynamic == 1)//二维动态地主改 所有玩家高于g_nHighMoney则使用高分底�?
	{
		int nHighPlayerNum = 0;
		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* player = GetPlayer(i);
			if (player)
			{
				if (player->GetGameMoney() > g_nHighMoney)
				{
					nHighPlayerNum++;
				}
			}
		}
		if (nHighPlayerNum == GetPlayNum())
		{
			m_nBaseScore = g_nHighScore;
			m_nTax = g_nHighTaxMoney;//设置�?
		}
		else
		{
			m_nBaseScore = g_nBaseScore;
			m_nTax = g_nTax;//设置�?
		}
	}
}

void CGameTable::SendBaseMoney()
{
	pt_gc_send_dizhu_not noti;
	noti.opcode = gc_send_dizhu_not;
	noti.nGameMoney = m_nBaseScore;
	NotifyRoom(noti);
}

int CGameTable::GetNextPlayer()
{
	SERVER_LOG("CGameTable::GetNextPlayer()");
	for (int i = 0; i < GetPlayNum(); i++)
	{
		m_nWaitOpChairID++;
		m_nWaitOpChairID %= GetPlayNum();
		CPlayer* pPlayer = GetPlayer(m_nWaitOpChairID);
		if (pPlayer && !pPlayer->m_bGiveUp)
		{
			break;
		}
	}
	//新的一�?开�?,清掉桌面出牌信息
	if (m_nWaitOpChairID == m_nPutCardsPlayerID)
	{
		SendCommonCmd(CO_NEW);
		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* pPlayer = GetPlayer(i);
			if (pPlayer)
			{
				pPlayer->m_bGiveUp = false;
			}
		}
	}
	return m_nWaitOpChairID;
}

void CGameTable::SvrStartGameNot()
{
	SERVER_LOG("CGameTable::SvrStartGameNot().m_nSerialID:%d", m_nSerialID);
	pt_gc_game_start_not noti;
	noti.opcode = gc_game_start_not;
	noti.nGameMoney = m_nBaseScore;
	noti.nCardNum = 51;
	noti.nLordPos = m_Poke.m_nRanCardPos;
	noti.cLordCard = m_Poke.m_cLaiziCard;
	noti.nSerialID = ++m_nSerialID;
	NotifyRoom(noti);

	SendJuCount();
}

void CGameTable::SendJuCount(CPlayer* pPlayer)
{
	if (g_nPrivateRoom != 1)
	{
		return;
	}

	pt_gc_ju_count_not noti_ju;
	noti_ju.opcode = gc_ju_count_not;
	noti_ju.nJuCount = m_nMaJiangJu;

	if (pPlayer)
	{
		pPlayer->SendPacket(noti_ju);
	}
	else
	{
		NotifyRoom(noti_ju);
	}
}

void CGameTable::AddRecordTimes(int nChairID, RecordTpye eType, int64 num /*= 1*/)//记录抢地�?
{
	SERVER_LOG("CGameTable::AddRecordTimes nChairID:%d, RecordTpye:%d num:%d", nChairID, eType, num);
	if (g_nPrivateRoom != 1)//不是私人�?
	{
		return;
	}

	if (eType == eRECORD_CALL)//�?地主
	{
		m_sGameStatic[nChairID].nCallTimes += num;
	}
	else if (eType == eRECORD_LORD)//当地�?
	{
		m_sGameStatic[nChairID].nLordTimes += num;
	}
	else if (eType == eRECORD_WIN)//胜利
	{
		m_sGameStatic[nChairID].nWinTimes += num;
	}
	else if (eType == eRECORD_ZHNAJI)//战绩
	{
		m_sGameStatic[nChairID].nZhanJi += num;
	}
	else
	{
		SERVER_LOG("CGameTable::AddRecordTimes ERROR");
	}
}

void CGameTable::OnReadyTimeOut()
{
	m_startPauseTime = time(0);
	SendReadyTimeOutNot();
	StartTimer(-1, eREADY_PAUSE_EVENT);
}

void CGameTable::NotiStartGame()
{
	if (m_startPauseTime > 0)
	{
		m_startPauseTime = 0;
		pt_gc_pause_game_not noti;
		noti.opcode = gc_pause_game_not;
		noti.nFlag = 3;
		NotifyRoom(noti, eALLUSER, NULL, false);
	}
}

void CGameTable::SendReadyTimeOutNot()
{
	pt_gc_pause_game_not noti;
	noti.opcode = gc_pause_game_not;
	noti.nFlag = 2;
	noti.nMinTime = g_nIdleTime;
	noti.nSecTime = m_startPauseTime - time(0) + g_nIdleTime * 60;
	if (noti.nSecTime <= 0)
	{
		PauseEndGame();
	}
	else
	{
		NotifyRoom(noti, eALLUSER, NULL, false);
		StartTimer(-1, eREADY_PAUSE_EVENT);
	}
}

void CGameTable::OnPauseGame(bool bIsReturn /*= false*/)
{
	char cChairId = m_nWaitOpChairID;
	CPlayer* pPlayer = GetPlayer(cChairId);
	if (NULL != pPlayer)
	{
		pt_gc_pause_game_not noti;
		noti.opcode = gc_pause_game_not;
		noti.cChairId = cChairId;
		noti.sNickName = pPlayer->GetNickName();
		if (bIsReturn)
		{
			noti.nFlag = 1;
			NotifyRoom(noti, eALLUSER, NULL, false);
		}
		else
		{
			noti.nFlag = 0;
			noti.nMinTime = g_nPauseTime;
			noti.nSecTime = m_startPauseTime - time(0) + g_nPauseTime * 60;
			if (noti.nSecTime <= 0)
			{
				PauseEndGame();
			}
			else
			{
				NotifyRoom(noti, eALLUSER, NULL, false);
				m_pCoreTable->RemoveTimer();
				m_pCoreTable->SetTimer(ePAUSE_GAME_PERIOD, (void*)ePAUSE_GAME_EVENT);
			}
		}
	}
	else
	{
		PauseEndGame();
	}
}

void CGameTable::PauseEndGame()
{
	SERVER_LOG("CGameTable::PauseEndGame()");
	//pt_ss_force_end_private_game packet;
	//packet.opcode = ss_force_end_private_game;
	//UpdateToTable(packet);
	
	pt_ss_update_round_mj packet;
	packet.opcode = ss_update_round_mj;
	packet.num_ = 100;
	UpdateToTable(packet);

	//AbnormalEndGame();//扣除税收
	if (m_pCoreTable)
	{
		m_pCoreTable->RemoveTimer();
		m_pCoreTable->EndGame();
	}
}

void CGameTable::AbnormalEndGame()//流局
{
	SERVER_LOG("CGameTable::AbnormalEndGame()");
	int nScore[3];
	memset(nScore, 0, sizeof(nScore));
	int tax = m_nTax;
	for (int i = 0; i < m_nPlay_Num; i++)
	{
		CPlayer* pPlayer = GetPlayer(i);
		if (pPlayer == NULL)
		{
			continue;
		}
		IPlayer* pIPlayer = pPlayer->GetCorePlayer();
		if (pIPlayer == NULL)
		{
			continue;
		}
		if (m_nSettleType == 1)//�?�?
		{
			PLY_ATTR_UPDATE_REASON eReason = UP_RN_PRIVATE_ROOM_SCORE;
			if (g_nNewMatch == 1)
			{
				eReason = UP_RN_MATCH_ROOM_SCORE;
			}

			IPlayer::PlyAttrItem item_(eReason, ITEM_SCORE, nScore[i], 0, m_strGameLabel);
			if (GetPlayer(i)->GetCorePlayer())
			{
				GetPlayer(i)->GetCorePlayer()->UpdatePlyAttrs(item_);
			}
		}
		else if (g_nHBMode > 0 && g_nHBMode < HONGBAO_H5_CHUJI)
		{
			if (g_nPrivateRoom == 0 && m_nSettleType == 0)
			{
				IPlayer::PlyAttrItem item_(UP_RN_GAME_RUSULT_STATUS, PLY_ATTR_MONEY, nScore[i] > 0 ? 1 : 0);
				if (pIPlayer)
				{
					pIPlayer->UpdatePlyAttrs(item_);
				}
			}

			int nChangeNum = g_nTax > 1 ? nScore[i] - tax : nScore[i];
			//IPlayer::PlyAttrItem item2_(UP_RN_PRIVATE_ROOM_SCORE, PLY_ATTR_MONEY, nChangeNum, 0, m_strGameLabel);
			//if (pIPlayer)
			//{
			//	pIPlayer->UpdatePlyAttrs(item2_);
			//}

			IPlayer::PlyAttrItem item3_(UP_RN_SPECIFY_ITEM, ITEM_GOLD_LEAF, nChangeNum, 0, m_strGameLabel);
			if (pIPlayer)
			{
				pIPlayer->UpdatePlyAttrs(item3_);
			}
		}
		else//游戏�?
		{
			if (g_nPrivateRoom == 0 && m_nSettleType == 0)
			{
				IPlayer::PlyAttrItem item_(UP_RN_GAME_RUSULT_STATUS, PLY_ATTR_MONEY, nScore[i] > 0 ? 1 : 0);
				if (pIPlayer)
				{
					pIPlayer->UpdatePlyAttrs(item_);
				}
			}
			
			int nChangeNum = g_nTax > 1 ? nScore[i] - tax : nScore[i];
			IPlayer::PlyAttrItem item2_(UP_RN_PRIVATE_ROOM_SCORE, PLY_ATTR_MONEY, nChangeNum, 0, m_strGameLabel);
			if (pIPlayer)
			{
				pIPlayer->UpdatePlyAttrs(item2_);
			}

			IPlayer::PlyAttrItem item3_(UP_RN_GAME_RESULT, PLY_ATTR_MONEY, nChangeNum, 0, m_strGameLabel);
			if (pIPlayer)
			{
				pIPlayer->UpdatePlyAttrs(item3_);
			}
		}
		//记录金币变化
		AddRecordTimes(i, eRECORD_ZHNAJI, nScore[i]);

		if (g_nCounts == 1 && pPlayer->m_bUseCounts)//�?否有记牌�?
		{
			int n_num = pIPlayer->GetPlyAttr(ITEM_ATTR, ITEM_CARD_RECORD);
			if (n_num > 0)
			{
				IPlayer::PlyAttrItem item2_(UP_RN_USE_ITEM, ITEM_CARD_RECORD, -1);
				pIPlayer->UpdatePlyAttrs(item2_);

				pt_gc_counts_not noti;
				noti.opcode = gc_counts_not;
				noti.counts_num_ = n_num - 1;
				pPlayer->SendPacket(noti);

				pt_gc_counts_not1 noti1;
				noti1.opcode = gc_counts_not1;
				noti1.counts_num_ = n_num - 1;
				pPlayer->SendPacket(noti1);
			}
		}

		if (g_nBroadcastWin == 1) //赢牌广播
		{
			if (nScore[i] >= g_nBroadcastWinNum)
			{
				char buffer[256];
				pt_lw_trumpet_req req;
				req.opcode = lw_trumpet_req;
				sprintf(buffer, CConfigManager::GetInstancePtr()->GetString("broadcast_win").c_str(), pIPlayer->GetPlyStrAttr(0, PLY_ATTR_NICKNAME), nScore[i] / 10000);
				req.message_ = buffer;
				req.ply_guid_ = 0;

				COutputStream os;
				os << req;
				m_pCoreTable->Send2World(os.GetData(), os.GetLength());
			}
		}
	}
}

void CGameTable::OnPutCardEndPause()
{
	if (m_startPauseTime > 0)
	{
		m_startPauseTime = 0;
		OnPauseGame(true);
	}
}

void CGameTable::NewRound()
{
	SERVER_LOG("CGameTable::NewRound()");
	m_bLordDeal = false;
	m_bStart = false;
	m_bRacing = true;
	m_bShowCard = false;
	m_nBaseScore = 0;
	m_nWaitOpChairID = 0;
	m_nPutCardsPlayerID = 0;
	m_nCallScore = 0;
	m_nRobDouble = 1;
	m_nLetCardNum = 0;
	m_nDoubleNum = 0;
	m_currentTime_ = 0;
	m_nNoCallNum++;
	m_startPauseTime = 0;
	m_nMaJiangJu++;
	m_nInitDouble = 1;
	m_nMustCallSeat = -1;
	m_nShuffleLogType = -1;
	m_nShuffleLogBetterSeat = -1;
	m_nShuffleLogRobotSeat = -1;
	std::fill(m_vecDoubleDetail.begin(), m_vecDoubleDetail.end(), 1);
	std::fill(m_vecPlayerDouble.begin(), m_vecPlayerDouble.end(), 1);
	
	if (g_nBaseInitDouble > 0)
	{
		m_nInitDouble = g_nBaseInitDouble;
	}
	

	if (g_nTwoPai == 1 && g_nTwoBaseRate > 0)//使用二人斗地主初始倍数(失�?? 应�?�是部分情况�?)
	{
		m_nInitDouble = g_nTwoBaseRate;
	}
	if (g_nMatch == 1)
	{
		CPlayer* pShowPlayer = GetPlayer(0);
		if (pShowPlayer)
		{
			m_nInitDouble = pShowPlayer->GetCorePlayer()->GetPlyAttr(0, PLY_CURRENT_ROUND);
			m_nInitDouble = pow(2, m_nInitDouble);
		}
	}

	m_nDouble = m_nInitDouble;

	for (int i = 0; i < GetPlayNum(); i++)
	{
		CPlayer* pPlayer = GetPlayer(i);
		if (pPlayer)
		{
			pPlayer->NewRound();
		}
	}
}

bool CGameTable::CheckBetterSeat(int& nBetterSeat, int& nRobotSeat)
{
	if (g_nNewShuffleCards == 1)
	{
		int player_val[3];
		memset(player_val, 0, sizeof(player_val));
		int robotNumber = -1;

		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* player = GetPlayer(i);
			if (player)
			{
				if (g_nGameType >= 0)
				{
					const char* guid = player->GetCorePlayer()->GetPlyStrAttr(0, PLY_ATTR_GUID);
					if (CConfigManager::GetInstancePtr()->isRobotGuid(guid))
					{
						if (g_nGameType > 1)
						{
							player_val[i] = 4 + rand() % 4;
						}
						else{
							player_val[i] = 4 + rand() % 4;
						}
						player_val[0] = 0; // 降低机器人的优势�?
						robotNumber = i;
						continue;
					}
				}

				if (g_nNewMatch == 1)
				{
					continue;
				}

				//SERVER_LOG("---------first login: %d.", player->GetCorePlayer()->IsFirstLogin());
				//if(player->GetCorePlayer()->IsFirstLogin()==1)   //1�?一次进入房�?  0不是�?一次登�?
				//{
				//	player_val[i] += 3; //每天游戏的�??1局
				//}	

				//每天游戏局�?
				int daily_count = player->GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_DAILY_COUNT);
				if (daily_count <= 60)
				{
					map<int, int>::iterator itr = m_mapDailyCountVal.find(daily_count);
					if (itr != m_mapDailyCountVal.end() && itr->second > 0)
					{
						player_val[i] += itr->second;
					}

					if (daily_count <= 5)
					{
						int nNumRound = player->GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_ROUND);
						if (nNumRound > 0 && nNumRound <= 3)
						{
							player_val[i] += 4;
						}
						else if (nNumRound > 3 && nNumRound <= 5)
						{
							player_val[i] += 1;
						}
					}
				}

				//充值用�?
				if (player->GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_CHARGE) > 0 && (rand() % 100 < 70))
				{
					player_val[i] += 1;
				}

				//高分玩�?�进入发�?�?
				if (g_nLimitMoneyNext > 0)
				{
					if (player->GetGameMoney() > g_nLimitMoneyNext)
					{
						if (rand() % 100 < 80)
						{
							player_val[i] += 1;
						}
						else
						{
							player_val[i] -= 1;
						}
					}
				}

				//连输玩�??
				int lost_count = player->GetCorePlayer()->GetPlyAttr(0, PLY_CONTINUE_LOST);
				if (lost_count > 1)
				{
					player_val[i] += lost_count;
				}
			}
		}
		SERVER_LOG("CheckBetterSeat0:%d,1:%d,2:%d", player_val[0], player_val[1], player_val[2]);
		//取最大�?
		int maxNumber = 0;
		if (player_val[1] > player_val[0])  {
			maxNumber = 1;
		}
		if (player_val[2] > player_val[maxNumber])
		{
			maxNumber = 2;
		}
		if ((player_val[1] == player_val[0]) && (player_val[1] == player_val[2]))
		{
			maxNumber = robotNumber;
		}
		nBetterSeat = maxNumber;
		nRobotSeat = robotNumber;
		return true;
	}
	else
	{
		bool is_new_player = false;		//�?否有新玩�?
		bool is_specific = false;		//�?否特殊牌�?
		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* player = GetPlayer(i);
			if (player)
			{
				SERVER_LOG("---------first login: %d.", player->GetCorePlayer()->IsFirstLogin());
				if (player->GetCorePlayer()->IsFirstLogin() == 1)   //1�?一次进入房�?  0不是�?一次登�?
				{
					is_specific = true;
					is_new_player = true;
					nRobotSeat = 192;
				}
			}
		}

		int64 n_player_limit_money = 0;
		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* player = GetPlayer(i);
			if (player)
			{
				if (n_player_limit_money == 0)
				{
					n_player_limit_money = player->GetGameMoney();
				}
				if (n_player_limit_money > player->GetGameMoney())
				{
					n_player_limit_money = player->GetGameMoney();
				}
			}
		}

		if (n_player_limit_money > g_nLimitMoney * 10)	//一位玩家游戏币�?入场限制�?10�?
		{
			nRobotSeat = 96;
			if (!is_new_player)
			{
				if (n_player_limit_money > g_nLimitMoney * 15)
				{
					nRobotSeat = 192;
				}
				if (n_player_limit_money > g_nLimitMoney * 20)
				{
					nRobotSeat = 384;
				}
				if (n_player_limit_money > g_nLimitMoney * 25)
				{
					nRobotSeat = 768;
				}
				if (n_player_limit_money > g_nLimitMoney * 30)
				{
					nRobotSeat = 1536;
				}
			}
			is_specific = true;
		}

		if (is_specific)
		{
			if (g_nBaoPai == 1)
			{
				nBetterSeat = SPECIFIC_LAIZI_TYPE;
			}
			else
			{
				nBetterSeat = SPECIFIC_TYPE;
			}
		}
		else
		{
			if (g_nBaoPai == 1)
			{
				nBetterSeat = COMMON_LAIZI_TYPE;
			}
			else
			{
				nBetterSeat = COMMON_TYPE;
			}
		}

		return is_specific;
	}
}
void CGameTable::OnStartGame()
{
	char buffer[512];
	sprintf(buffer, "%d%d", (int)time(0), rand() % 10000);
	m_strGameLabel = buffer;

	SERVER_LOG("CGameTable::OnStartGame()");
#if 0
	if (!checkPlayersItem())
	{
		return;
	}
#endif

	if (g_nHBMode == 1 || g_nIsBaiYuan == 1)
	{
		refreshPlayerTokenMoney();
	}

	m_bHadStart = true;
	NotifyGameModle();
	NotiStartGame();
	NewRound();//开始新的一�?，洗�?

	ShuffleCards(); // 洗牌
	Dealing(); // 发牌

	SetBaseMoney();//设置游戏底注和税�?
	DealingTask();//任务设置
	SvrStartGameNot();//通知开�?
	SendDoubleInfo();//通知倍数
	// SendDoubleDetail();
	setMutiDoubleDetail(BEI_SHU_INFO_INIT, m_nInitDouble);
	
	CheckShowCardInReady();
	SendShowCardReq(SHOW_CARD_IN_DEAL);
	if (g_nRPGMode == 1)
	{
		calcLordChairID();
	}
}
void CGameTable::setDailyCountVal()
{
	m_mapDailyCountVal.clear();
	m_mapDailyCountVal[1] = 3;
	m_mapDailyCountVal[3] = 3;
	m_mapDailyCountVal[5] = 3;
	m_mapDailyCountVal[8] = 3;
	m_mapDailyCountVal[9] = 3;
	m_mapDailyCountVal[16] = 3;
	m_mapDailyCountVal[17] = 3;
	m_mapDailyCountVal[30] = 3;
	m_mapDailyCountVal[31] = 3;
	m_mapDailyCountVal[40] = 3;
	m_mapDailyCountVal[50] = 3;
	m_mapDailyCountVal[60] = 3;
	m_mapDailyCountVal[2] = 1;
	m_mapDailyCountVal[4] = 1;
	m_mapDailyCountVal[11] = 1;
	m_mapDailyCountVal[20] = 1;
	m_mapDailyCountVal[21] = 1;
	m_mapDailyCountVal[35] = 1;
	m_mapDailyCountVal[45] = 1;
	m_mapDailyCountVal[55] = 1;
}

void CGameTable::OnEndGame()
{
	SERVER_LOG("CGameTable::OnEndGame()");
	if (m_pCoreTable == NULL)
	{
		return;
	}

	for (int i = 0; i < GetPlayNum(); i++)
	{
		CPlayer* pPlayer = GetPlayer(i);
		if (pPlayer == NULL)
		{
			continue;
		}
		IPlayer* pIPlayer = pPlayer->GetCorePlayer();
		if (pIPlayer == NULL)
		{
			continue;
		}

		pIPlayer->EndGame();
	}

	m_pCoreTable->RemoveTimer();
	m_pCoreTable->EndGame();
}

//void CGameTable::OnUserReconnection(IPlayer* pPlayer)
//{
//	CPlayer *pCurPlayer = (CPlayer*) pPlayer;
//	pCurPlayer->SetStatus(CPlayer::PS_PLAYER);	
//	pCurPlayer->SetGameTable(this);
//	GetCompleteData(pCurPlayer);
//}

void CGameTable::OnUserJoinVisitor(IPlayerDelegate* pPlayer)
{
	SERVER_LOG("CGameTable::OnUserJoinVisitor()");
	CPlayer *pCurPlayer = (CPlayer*)pPlayer;
	pCurPlayer->SetStatus(CPlayer::PS_VISTOR);
	pCurPlayer->SetGameTable(this);
	GetCompleteData(pCurPlayer);
}

void CGameTable::OnUserLeave(IPlayerDelegate* pPlayer)
{
	SERVER_LOG("CGameTable::OnUserLeave(pPlayer:%p)", pPlayer);
	//有人退出，则累计送的游戏币从新开始�?�算
	CPlayer *pCurPlayer = (CPlayer*)pPlayer;
	if (pCurPlayer)
	{
		if (g_nPrivateRoom == 1)//私人房结束后通知离开
		{
			pt_gc_private_room_result_not noti;
			noti.opcode = gc_private_room_result_not;
			noti.ret_ = (m_bRacing || m_startPauseTime > 0 || m_bHadStart == false) ? -1 : 0;
			noti.vecGameStatiscs = m_sGameStatic;
			pCurPlayer->SendPacket(noti);
		}


		//退出时�?动�?�取红包
		/*
		pt_cg_get_redpackets_award_req req;
		req.type_ = 1;
		if (pCurPlayer->m_nRedPackets_Status == REDPACKET_STATUS_READY)
		{
			pCurPlayer->pt_cg_get_redpackets_award_req_handler(req);
		}
		pCurPlayer->writeRedPacketsStatusData();
		*/
		pCurPlayer->saveRedPacket88YuanData();
		pCurPlayer->writeRedPacketsNewbieData();

		pCurPlayer->SetGameTable(NULL);
	}
}

CPlayer* CGameTable::GetPlayer(int nChairID)
{
	SERVER_LOG("CGameTable::GetPlayer(nChairID:%d)", nChairID);
	if (nChairID >= 0 && nChairID < GetPlayNum())
	{
		IPlayer* pCorePlayer = m_pCoreTable->GetPlayer(nChairID);
		if (pCorePlayer)
		{
			return (CPlayer*)pCorePlayer->GetPlayerDelegate();
		}
	}

	return NULL;
}

void CGameTable::RefreshCards(CPlayer* pPlayer, CPlayer* pExceptPlayer, bool bPlyShow, bool bShowAll, bool bSaveCard)
{
	SERVER_LOG("CGameTable::RefreshCards()");
	pt_gc_refresh_card_not minnoti;
	pPlayer->GetPlayerCards(minnoti, true);		//明牌 

	pt_gc_refresh_card_not annoti;
	pPlayer->GetPlayerCards(annoti, false);		//暗牌

	COutputStream anos;
	anos << annoti;
	COutputStream minos;
	minos << minnoti;

	for (int i = 0; i < GetPlayNum(); i++)
	{
		CPlayer* pCur = GetPlayer(i);
		if (pCur == NULL || pCur == pExceptPlayer)
		{
			continue;
		}
		if (pCur == pPlayer || bPlyShow || g_nDebugShowCard == 1)	//如果�?�?己或同意显示给其它人�?
		{
			pCur->SendPacket(minnoti);
		}
		else
		{
			const char* guid = pCur->GetCorePlayer()->GetPlyStrAttr(0, PLY_ATTR_GUID);			
			if(CConfigManager::GetInstancePtr()->isRobotGuid(guid))
			{
				pCur->SendPacket(minnoti);
			}
			else
			{
				pCur->SendPacket(annoti);
			}
		}
	}

	/*
	if (bShowAll)
	{
		m_pCoreTable->NotifyVisitor(-1, minos.GetData(), minos.GetLength());
	}
	else
	{
		m_pCoreTable->NotifyVisitor(pPlayer->GetChairID(), minos.GetData(), minos.GetLength(), false);
		m_pCoreTable->NotifyVisitor(pPlayer->GetChairID(), anos.GetData(), anos.GetLength(), true);
	}
	*/
	if (bSaveCard && g_nPrivateRoom == 1)
	{
		m_pCoreTable->PushReplayPacket(minos.GetData(), minos.GetLength());
	}
}

void CGameTable::SendPutCards(CPlayer* pPlayer, bool bAll)
{
	SERVER_LOG("CGameTable::SendPutCards(bAll:%d).chairid:%d", bAll, pPlayer->GetChairID());
	pt_gc_play_card_not noti;
	noti.opcode = gc_play_card_not;
	noti.cChairID = pPlayer->GetChairID();
	noti.vecCards = pPlayer->m_PlayCard.m_cChoosingCards;
	noti.cType = pPlayer->m_PlayCard.m_cDiscardingType;
	if (bAll)
	{
		NotifyRoom(noti);
	}
	else
	{
		NotifyRoom(noti, 0, pPlayer);
	}
}

void CGameTable::sendRobotRemainCard(vector<CCard> vecCCard)
{
	pt_gc_refresh_card_not noti;
	noti.opcode = gc_refresh_card_not;
	noti.cChairID = 2;
	noti.vecCards = vecCCard;

	for (int i = 0; i < GetPlayNum(); i++)
	{
		CPlayer* pCur = GetPlayer(i);
		if (pCur == NULL)
		{
			continue;
		}
		const char* guid = pCur->GetCorePlayer()->GetPlyStrAttr(0, PLY_ATTR_GUID);		
		if(CConfigManager::GetInstancePtr()->isRobotGuid(guid))
		{
			pCur->SendPacket(noti);
		}
	}
}

void CGameTable::Dealing()
{

	for (int i = 0; i < GetPlayNum(); i++)
	{
		int nChairID = i;
		if (g_nCleverRobot == 1)
		{
			nChairID = GetPlayNum() - 1 - i;
		}
		CPlayer* pPlayer = GetPlayer(nChairID);
		if (pPlayer)
		{
			for (int j = 0; j < g_nDealCardNum; j++)
			{
				pPlayer->GetCard(m_Poke.GetCard());
			}
		}
	}

	//二人斗地�? 分完牌后告诉所有玩家一张地主牌
	if (g_nTwoPai == 1)
	{
		CPlayer* pPlayer = GetPlayer(m_Poke.m_nDefaultLord);
		if (pPlayer && pPlayer->m_PlayCard.m_cCards.empty() == false)
		{
			int num = pPlayer->m_PlayCard.m_cCards.size() > 8 ? 2 : 0;
			int nRand = rand() % (pPlayer->m_PlayCard.m_cCards.size() - 2 * num);

			pt_gc_two_show_card_not noti;
			noti.opcode = gc_two_show_card_not;
			noti.cChairID = m_Poke.m_nDefaultLord;
			noti.nLordPos = nRand + num;
			noti.cLordCard = pPlayer->m_PlayCard.m_cCards[noti.nLordPos];
			NotifyRoom(noti);
		}

		vector<CCard> m_cCards;
		for (int i = 0; i < g_nDealCardNum; i++)
		{
			CCard& card = m_Poke.GetCard();
			m_cCards.push_back(card);
		}
		sendRobotRemainCard(m_cCards);
	}

	//刷新牌面
	for (int i = 0; i < GetPlayNum(); i++)
	{
		CPlayer* pPlayer = GetPlayer(i);
		if (pPlayer)
		{
			RefreshCards(pPlayer, NULL, false, false, false);
		}
	}
	SaveAllCards();
	if (g_nRecordPlayerCard == 1 || m_nShuffleLogType == 5 || m_nShuffleLogType == 10)
	{
		string card_record = "RecordPlayerCard:" + m_strGameLabel + "|";
		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* pPlayer = GetPlayer(i);
			if (pPlayer)
			{
				card_record += pPlayer->GetCorePlayer()->GetPlyStrAttr(0, PLY_ATTR_GUID);
				card_record += ":";
				for (size_t j = 0; j < pPlayer->m_PlayCard.m_cCards.size(); j++)
				{
					char card_[32];
					CCard& card = pPlayer->m_PlayCard.m_cCards[j];
					sprintf(card_, "%d,", card.m_nValue * 10 + card.m_nColor);
					card_record += card_;
				}
				card_record += "|";
			}
		}

		card_record += "lord_card:";
		for (size_t i = 0; i < m_Poke.m_cLordCard.size(); i++)
		{
			char card_[32];
			CCard& card = m_Poke.m_cLordCard[i];
			sprintf(card_, "%d,", card.m_nValue * 10 + card.m_nColor);
			card_record += card_;
		}
		g_pLogger->Log("%s", card_record.c_str());
	}
	//�?动发牌动画定时器
	StartTimer(-1, eDEALCARD_ANIMATE_EVENT);
}

void CGameTable::CheckShowCardInReady()
{
	if (g_nShowCard == 0) {
		return;
	}

	for (int nIndex = 0; nIndex < GetPlayNum(); nIndex++)
	{
		CPlayer* pPlayer = GetPlayer(nIndex);
		if (pPlayer)
		{
			if (pPlayer->getShowCard() == SHOW_CARD_IN_READY) {
				SendCommonCmd(CO_SHOWCARD, pPlayer->GetChairID());
				pPlayer->setCallLordRate(pPlayer->getCallLordRate() * 2);
				if (pPlayer->m_nTempForShowCardBet > 1 && pPlayer->m_nTempForShowCardBet <= g_nShowCardDoubleInReady)
				{
					setMutiDoubleDetail(BEI_SHU_INFO_SHOWCARD, pPlayer->m_nTempForShowCardBet);
				}
				SendShowCardNot(pPlayer);
			}
		}
	}
}

void CGameTable::SendShowCardReq(int type, CPlayer* toPlayer /*= NULL*/)
{
	if (g_nShowCard == 0) {
		return;
	}

	if (type == SHOW_CARD_IN_PUTCARD)
	{
		if (!toPlayer) {
			return;
		}
		if (toPlayer->GetChairID() != m_Poke.m_nCurrentLord) {
			return;
		}
		if (toPlayer->m_nPutCount != 0) {
			return;
		}
		if (toPlayer->isShowCard()) {
			return;
		}

		toPlayer->sendShowCardReq(type, g_nShowCardDoubleInPutCard);
		
	}else if (type == SHOW_CARD_IN_DEAL) {
		for (int nIndex = 0; nIndex < GetPlayNum(); nIndex++)
		{
			CPlayer* pPlayer = GetPlayer(nIndex);
			if (pPlayer)
			{
				if (!pPlayer->isShowCard()) {
					pPlayer->sendShowCardReq(type, g_nShowCardDoubleInDeal);
				}
			}
		}
	}
}

void CGameTable::CallScore()
{
	SERVER_LOG("CGameTable::CallScore()");
	if (m_Poke.m_nDecideLordRound > GetPlayNum() - 1)
	{
		CPlayer* pLordPlayer = GetPlayer(m_Poke.m_nCurrentLord);
		if (pLordPlayer && pLordPlayer->m_nCallScore == -1)	//没人�?地主
		{
			// 标准模式�?,没人�?地主,则直接洗牌进行下一局
			if (this->is_standard_game())
			{
				SendCommonCmd(CO_NOLORD);
				OnStartGame();
				return;
			}

			if (g_nDoubleScore == 1 && m_nNoCallNum < 3)//加倍流�? && 没叫地主小于3回合
			{
				SendCommonCmd(CO_NOLORD);
				OnStartGame();
				return;
			}
			if (g_nDefaultLord)
			{
				m_nCallScore = (g_nTwoPai == 1) ? 2 : 1;//二人斗地主默认叫2�?
				m_Poke.m_nCurrentLord = m_Poke.m_nDefaultLord;
				if (g_nTwoPai == 1)
				{
					AddLetCardNum();
				}
			}
			else
			{
				if (g_nAutoAddFan == 1) m_nAddFan *= 2;
				SendCommonCmd(CO_NOLORD);
				OnStartGame();
				return;
			}

			if (g_nRPGMode == 1)
			{
				if (g_nCallScore == 1 || g_nCallScore == 2) {
					m_nCallScore = 1;
				}else {
					m_nCallScore = 1;
				}
				
				CallScoreAndSendDoubleInfo();
			}
		}
		// 标准模式�?,地主�?认后,没有抢地主环�?
		//�?地主流程结束,看是否有抢地主流�?
		if (m_nRobLord == 1 && !this->is_standard_game())
		{
			m_Poke.m_nDecideLordRound = 1;
			m_Poke.m_nDefaultLord = m_Poke.m_nCurrentLord;
			RobLord();
		}
		else
		{
			DealingLord();
		}
	}
	else
	{
		//从默认地主开始依次�?�问�?否叫�?
		m_nWaitOpChairID = (m_Poke.m_nDefaultLord + m_Poke.m_nDecideLordRound) % GetPlayNum();
		CPlayer* pPlayer = GetPlayer(m_nWaitOpChairID);
		if (pPlayer)
		{
			pt_gc_call_score_req req;
			req.opcode = gc_call_score_req;
			if (g_nCallScore == 0)			//�?地主  不叫
			{
				req.nScore = -1;
			}
			else if (g_nCallScore == 1 || g_nCallScore == 2 || g_nCallScore == 3)		//1�?  2�?  3�?
			{
				req.nScore = m_nCallScore;
			}
			req.nSerialID = ++m_nSerialID;
			req.nCallMode = g_nCallScore;
			pPlayer->SendPacket(req);
			StartTimer(m_nWaitOpChairID, eCALLSCORE_EVENT);
		}
		m_Poke.m_nDecideLordRound++;
	}
}

void CGameTable::RobLord()
{
	SERVER_LOG("CGameTable::RobLord()");
	bool bIsRobEnd = m_Poke.m_nDecideLordRound > GetPlayNum();
	if (g_nTwoPai == 1)//二人斗地�?
	{
		bIsRobEnd = m_Poke.m_nDecideLordRound > 4;
	}
	if (bIsRobEnd)
	{
		if (g_nCallScore == 0)
		{
			CPlayer* pLordPlayer = GetPlayer(m_Poke.m_nCurrentLord);
			if (pLordPlayer && pLordPlayer->m_nCallScore == -1)	//没人�?地主
			{
				if (g_nDefaultLord)
				{
					m_Poke.m_nCurrentLord = m_Poke.m_nDefaultLord;
				}
				else
				{
					if (g_nAutoAddFan == 1)
					{
						m_nAddFan *= 2;
					}
					SendCommonCmd(CO_NOLORD);
					OnStartGame();
					return;
				}
			}
		}

		DealingLord();
	}
	else
	{
		//从默认地主开始依次�?�问�?否叫�?
		if (g_nCallScore == 1 || g_nCallScore == 2)
		{
			bool bFind = false;
			for (int i = 0; i < GetPlayNum(); i++)
			{
				int nChairID = (m_Poke.m_nCurrentLord + i + 1) % GetPlayNum();
				CPlayer* pPlayer = GetPlayer(nChairID);
				if (pPlayer && pPlayer->m_nCallScore != -1 && nChairID != m_Poke.m_nCurrentLord)
				{
					bFind = true;
					m_nWaitOpChairID = nChairID;
					break;
				}
			}
			if (!bFind)
			{
				m_Poke.m_nDecideLordRound = 9999;
				RobLord();
				return;
			}
		}
		else
		{
			bool bFind = false;
			for (int i = 0; i < GetPlayNum(); i++)
			{
				int nChairID = (m_Poke.m_nDefaultLord + m_Poke.m_nDecideLordRound) % GetPlayNum();
				CPlayer* pPlayer = GetPlayer(nChairID);
				if (pPlayer && pPlayer->m_nCallScore != -1)
				{
					bFind = true;
					m_nWaitOpChairID = nChairID;
					break;
				}
				m_Poke.m_nDecideLordRound++;
			}
			if (!bFind || (m_Poke.m_nDecideLordRound > 2 && m_Poke.m_nDefaultLord == m_Poke.m_nCurrentLord))
			{
				m_Poke.m_nDecideLordRound = 4;
				RobLord();
				return;
			}
		}
		CPlayer* pPlayer = GetPlayer(m_nWaitOpChairID);
		if (pPlayer)
		{
			pt_gc_rob_lord_req req;
			req.opcode = gc_rob_lord_req;
			req.cDefaultLord = m_Poke.m_nCurrentLord;
			req.nSerialID = ++m_nSerialID;
			pPlayer->SendPacket(req);
			StartTimer(m_nWaitOpChairID, eROBLORD_EVENT);
		}
		m_Poke.m_nDecideLordRound++;
	}
}

void CGameTable::sendLordDoubleScore(int nChairID /*= -1*/)
{
	if (!m_bLordDeal || g_nTwoPaiLordDouble == 0)
	{
		return;
	}
	int nLordCardDouble = 1;
	if (LordCardIsSameColor()/* || LordCardIsSmall()*/)
	{
		nLordCardDouble = 2;
	}
	if (LordCardIsSeries())
	{
		nLordCardDouble = 3;
	}
	if (LordCardIs3())
	{
		nLordCardDouble = 4;
	}
	if (nLordCardDouble < 2)
	{
		return;
	}

	if (nChairID < 0)
	{
		m_nDouble *= nLordCardDouble;
		if (m_nMaxDouble > 0 && m_nDouble > m_nMaxDouble)
		{
			m_nDouble = m_nMaxDouble;
		}
	}

	pt_gc_extra_double_score_not noti;
	noti.opcode = gc_extra_double_score_not;
	noti.nDouble = m_nDouble;
	noti.nLordDouble = nLordCardDouble;
	noti.nSerialID = m_nSerialID;

	if (nChairID < 0)
	{
		NotifyRoom(noti);
	}
	else
	{
		CPlayer* pPlayer = GetPlayer(nChairID);
		if (pPlayer)
		{
			pPlayer->SendPacket(noti);
		}
	}
}

void CGameTable::sendDoubleScore(int nDouble)
{
	if (nDouble < 1)
	{
		nDouble = 1;
	}
	m_nDouble *= nDouble;
	if (m_nMaxDouble > 0 && m_nDouble > m_nMaxDouble)
	{
		m_nDouble = m_nMaxDouble;
	}

	pt_gc_double_score_not noti;
	noti.opcode = gc_double_score_not;
	noti.nDouble = m_nDouble;
	noti.nSerialID = m_nSerialID;
	NotifyRoom(noti);

	DoubleScore();//加倍流�?
}


//---------------------------- [[ 标准模式下的玩�?�加�? ]] ---------------------------- //
void CGameTable::StandardSendDoubleScore(int nDouble, CPlayer* pPlayer)
{
	// 如果�?防守者发送的加倍�?�求
	if (pPlayer->GetChairID() != m_Poke.m_nCurrentLord)
	{
		CPlayer* pCurrentLord = GetPlayer(m_Poke.m_nCurrentLord);
		if (pCurrentLord)
		{
			int nNewScore	= m_nDouble * pPlayer->m_nStandardDoubleScore * pCurrentLord->m_nStandardDoubleScore;

			pt_gc_double_score_not noti;
			noti.opcode		= gc_double_score_not;
			noti.nDouble	= nNewScore;
			noti.nSerialID	= m_nSerialID;
			pPlayer->SendPacket(noti);

			if (nNewScore > pCurrentLord->m_nStandardBigerDoubleScore)
			{
				pCurrentLord->SendPacket(noti);
				pCurrentLord->m_nStandardBigerDoubleScore = nNewScore;
			}
		}
	}
	// 如果�?庄�?�发送的加倍�?�求,则向加倍过的防守者同步新的加倍�?�求
	else
	{
		int nBigerScore			= 1;
		for (int nIndex = 0; nIndex < GetPlayNum(); nIndex++)
		{
			CPlayer* pDefender	= GetPlayer(nIndex);
			if (pDefender && nIndex != m_Poke.m_nCurrentLord && pDefender->m_nStandardDoubleScore > 1)
			{
				int nNewScore	= m_nDouble * pPlayer->m_nStandardDoubleScore * pDefender->m_nStandardDoubleScore;
				nBigerScore		= nBigerScore > nNewScore ? nBigerScore : nNewScore;

				pt_gc_double_score_not noti;
				noti.opcode		= gc_double_score_not;
				noti.nDouble	= nNewScore;
				noti.nSerialID	= m_nSerialID;
				pDefender->SendPacket(noti);
			}
		}

		if (nBigerScore > 1)
		{
			pt_gc_double_score_not noti;
			noti.opcode		= gc_double_score_not;
			noti.nDouble	= nBigerScore;
			noti.nSerialID	= m_nSerialID;
			pPlayer->SendPacket(noti);

			pPlayer->m_nStandardBigerDoubleScore = nBigerScore;
		}
	}


	// 加倍流�?
	this->DoubleScore();
	return;
}

void CGameTable::DoubleScore()
{
	SERVER_LOG("CGameTable::DoubleScore()");

	if (g_nDoubleScore == 2) {
		pt_gc_double_score_req req;
		req.opcode = gc_double_score_req;
		req.nSerialID = ++m_nSerialID;
		NotifyRoom(req);
		StartTimer(m_nWaitOpChairID, eDOUBLE_EVENT);
		return;
	}else if (this->is_standard_game())	{
		// 如果�?标准模式
		this->StandardDoubleScore();
		return;
	}

	if (m_nDoubleNum >= GetPlayNum())
	{
		SendCommonCmd(CO_DOUBLE);
		startPutCard();
	}
	else
	{
		//从默认地主开始依次�?�问加�?
		m_nWaitOpChairID = (m_Poke.m_nCurrentLord + m_nDoubleNum) % GetPlayNum();
		CPlayer* pPlayer = GetPlayer(m_nWaitOpChairID);
		if (pPlayer)
		{
			pt_gc_double_score_req req;
			req.opcode = gc_double_score_req;
			req.nSerialID = ++m_nSerialID;
			pPlayer->SendPacket(req);
			StartTimer(m_nWaitOpChairID, eDOUBLE_EVENT);
		}
	}
	m_nDoubleNum++;
}


//---------------------------- [[ 标准模式下的加倍流�? ]] ---------------------------- //
void CGameTable::StandardDoubleScore()
{
	bool bDefenderNodify		= false;		// 防守人是否需要通知
	bool bDefenderCallScore		= false;		// 防守人是否已经加倍过
	bool bAllDefenderRespond	= true;

	int nBankerStep				= 0;			// 庄�?�加倍�?��??
	for (int nIndex = 0; nIndex < GetPlayNum(); nIndex++)
	{
		CPlayer* pPlayer = GetPlayer(nIndex);
		if (pPlayer)
		{
			if (nIndex != m_Poke.m_nCurrentLord)
			{
				bDefenderNodify		= bDefenderNodify || pPlayer->m_nStandardDoubleStep == 0;
				bDefenderCallScore	= bDefenderCallScore || pPlayer->m_nStandardDoubleScore > 1;
				bAllDefenderRespond = bAllDefenderRespond && pPlayer->m_nStandardDoubleStep == 2;
			}
			else
			{
				nBankerStep			= pPlayer->m_nStandardDoubleStep;
			}
		}
	}

	// 1. 通知农民�?否加�?
	if (bDefenderNodify)
	{
		for (int nIndex = 0; nIndex < GetPlayNum(); nIndex++)
		{
			if (nIndex != m_Poke.m_nCurrentLord)
			{
				CPlayer* pPlayer = GetPlayer(nIndex);
				if (pPlayer)
				{
					// 1_1. 记录状�?
					pPlayer->m_nStandardDoubleStep = 1;

					// 1_2. 通知客户�?
					pt_gc_double_score_req req;
					req.opcode = gc_double_score_req;
					req.nSerialID = m_nSerialID;
					pPlayer->SendPacket(req);
				}
			}
		}

		StartTimer(-1, eSTANDARD_DEFENDER_DOUBLE_EVENT);
		return;
	}

	// 2. 如果防守人都已经通知过，且防守人都返回消�?到�?�户�?
	if (!bDefenderNodify && bAllDefenderRespond && (nBankerStep == 0 || bDefenderCallScore))
	{

	}

	// 3. 如果防守人都已经通知过且有一�?防守者选择了加�?,则通知庄�?�是否选择加�?
	if (!bDefenderNodify && bAllDefenderRespond && nBankerStep == 0 && bDefenderCallScore)
	{
		CPlayer* pPlayer = GetPlayer(m_Poke.m_nCurrentLord);
		if (pPlayer)
		{
			// 3_1. 记录状�?
			pPlayer->m_nStandardDoubleStep = 1;

			// 3_2. 通知客户�?
			pt_gc_double_score_req req;
			req.opcode = gc_double_score_req;
			req.nSerialID = ++m_nSerialID;
			pPlayer->SendPacket(req);
		}

		StartTimer(m_Poke.m_nCurrentLord, eSTANDARD_BANKER_DOUBLE_EVENT);
		return;
	}


	// 4. 如果防守人都已经通知过且没人选择加�?,或者庄家也处理完是否加�?
	if (!bDefenderNodify && bAllDefenderRespond && (!bDefenderCallScore || nBankerStep == 2))
	{
		startPutCard();
	}
}

void CGameTable::startPutCard()//开始打�?
{
	m_bStart = true;

	CPlayer* pLordPlayer = GetPlayer(m_Poke.m_nCurrentLord);
	if (pLordPlayer) {
		SendShowCardReq(SHOW_CARD_IN_PUTCARD, pLordPlayer);
	}

	//地主出牌
	SendCommonCmd(CO_NEW);
	m_nWaitOpChairID = m_Poke.m_nCurrentLord;
	m_nPutCardsPlayerID = m_nWaitOpChairID;
	SvrPlayCardReq(m_nWaitOpChairID);	
}

void CGameTable::DealingLord()
{
	SERVER_LOG("CGameTable::DealingLord()");
	m_nAddFan = 1;
	m_bLordDeal = true;
	//地主拿底�?
	CPlayer* pLordPlayer = GetPlayer(m_Poke.m_nCurrentLord);
	if (pLordPlayer)
	{
		for (size_t i = 0; i < m_Poke.m_cLordCard.size(); i++)
		{
			pLordPlayer->GetCard(m_Poke.m_cLordCard[i]);
		}
	}
	RefreshCards(pLordPlayer);

	//记录玩�?�手�?
	for (int i = 0; i < GetPlayNum(); i++)
	{
		CPlayer* pPlayer = GetPlayer(i);
		if (pPlayer)
		{
			pPlayer->m_vecHandCard = pPlayer->m_PlayCard.m_cCards;
			if (pPlayer->m_nBetLordCardIndex >= 0)
			{
				if (pPlayer->m_nBetLordCardIndex == m_Poke.GetLordCardType())
				{
					switch (pPlayer->m_nBetLordCardIndex)
					{
					case 0:
						pPlayer->m_nBetLordCardReward = g_nBaseLordCardLottery * 12 / 10;
						break;
					case 1:
						pPlayer->m_nBetLordCardReward = g_nBaseLordCardLottery * 5;
						break;
					case 2:
						pPlayer->m_nBetLordCardReward = g_nBaseLordCardLottery * 18;
						break;
					case 3:
						pPlayer->m_nBetLordCardReward = g_nBaseLordCardLottery * 25;
						break;
					case 4:
						pPlayer->m_nBetLordCardReward = g_nBaseLordCardLottery * 300;
						break;
					case 5:
						pPlayer->m_nBetLordCardReward = g_nBaseLordCardLottery * 400;
						break;
					default:
						pPlayer->m_nBetLordCardReward = 0;
						break;
					}
				}
				else
				{
					pPlayer->m_nBetLordCardReward = 0;
				}
				pt_gc_bet_lord_card_result_ack ack;
				ack.opcode = gc_bet_lord_card_result_ack;
				ack.ret = pPlayer->m_nBetLordCardIndex;
				ack.money = pPlayer->m_nBetLordCardReward;
				pPlayer->SendPacket(ack);
			}
		}
	}

	//设置玩�?�的宝牌
	if (g_nBaoPai == 1)
	{
		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* pPlayer = GetPlayer(i);
			if (pPlayer)
			{
				pPlayer->m_PlayCard.SetBaoValue(m_Poke.m_cLaiziCard);
			}
			if (i == (GetPlayNum() - 1))
			{
				pt_gc_laizi_not noti;
				noti.opcode = gc_laizi_not;
				noti.card_value = pPlayer->m_PlayCard.m_nBaoValue;
				NotifyRoom(noti);
			}
		}
	}

	//显示底牌
	if (g_nLetCard == 1)
	{
		int nMinLet = 0;
		if (g_nTwoPaiMinLet == 1)
		{
			nMinLet = g_nTwoPaiMinLet;
		}
		else if (g_nTwoPai == 1)
		{
			nMinLet = 3;
		}
		int nMinRate = 1 << nMinLet;
		if (m_nDouble < nMinRate)
		{
			m_nDouble = nMinRate;
			SendDoubleInfo();
		}
		if (m_nLetCardNum < nMinLet)
		{
			m_nLetCardNum = nMinLet;
		}
		pt_gc_two_lord_card_not noti;
		noti.opcode = gc_two_lord_card_not;
		noti.cLord = m_Poke.m_nCurrentLord;
		noti.vecCards = m_Poke.m_cLordCard;
		noti.nLetNum = m_nLetCardNum;
		noti.nLordLetNum = g_nTwoPaiLordLet;
		NotifyRoom(noti);

		sendLordDoubleScore();
	}
	else
	{
		pt_gc_lord_card_not noti;
		noti.opcode = gc_lord_card_not;
		noti.cLord = m_Poke.m_nCurrentLord;
		noti.vecCards = m_Poke.m_cLordCard;
		NotifyRoom(noti);
	}

	// 配置要求加倍或者标准比赛都有加倍过�?
	if (g_nDoubleScore == 1 || g_nDoubleScore == 2 || this->is_standard_game())
	{
		DoubleScore();
	}
	else
	{
		startPutCard();//开始打�?
	}

	//记录地主
	AddRecordTimes(m_Poke.m_nCurrentLord, eRECORD_LORD);
}

void CGameTable::SvrPlayCardReq(int nChairID)
{
	SERVER_LOG("CGameTable::SvrPlayCardReq(nChairID:%d)", nChairID);

	CPlayer* pPlayer = GetPlayer(nChairID);
	if (pPlayer)
	{
		//清除出的�?        
		pPlayer->m_PlayCard.m_cDiscardingType.SetValue(0, 0, 0);
		pPlayer->m_PlayCard.m_cChoosingCards.clear();
		SendPutCards(pPlayer);
		//发送出牌�?�求
		pPlayer->m_curTime = leaf::GetCurTime();
		pt_gc_play_card_req req;
		req.opcode = gc_play_card_req;
		req.nSerialID = ++m_nSerialID;
		req.cAuto = pPlayer->m_bAuto;
		pPlayer->SendPacket(req);

		StartTimer(nChairID, ePLAYCARD_EVENT);
	}
}

//判断�?否春�?
void CGameTable::IsSpring(CPlayer* pPlayer)
{
	bool bSpring = true;
	bool bReverseSpring = true;
	if (pPlayer->GetChairID() == m_Poke.m_nCurrentLord)	//地主胜利
	{
		for (int i = 0; i < GetPlayNum(); i++)
		{
			if (i != pPlayer->GetChairID())
			{
				CPlayer* pLose = GetPlayer(i);
				if (pLose)
				{
					if (pLose->m_PlayCard.m_cCards.size() < 17)	//农民手上的牌不得少于17�?
					{
						bSpring = false;
						break;
					}
				}
			}
		}
		bReverseSpring = false;
	}
	else
	{
		CPlayer* pLose = GetPlayer(m_Poke.m_nCurrentLord);
		if (pLose)
		{
			if (pLose->m_nPutCount > 1)	//地主出牌次数少于1
			{
				bReverseSpring = false;
			}
		}
		bSpring = false;
	}

	//春天
	if (bSpring)
	{
		m_nDouble *= 2;
		setMutiDoubleDetail(BEI_SHU_INFO_SPRING, 2);
		m_Poke.m_bSpring = true;
		SendDoubleInfo();
	}
	if (bReverseSpring)
	{
		m_nDouble *= 2;
		setMutiDoubleDetail(BEI_SHU_INFO_SPRING, 2);
		m_Poke.m_bReverseSpring = true;
		SendDoubleInfo();
	}
	////反春天广�?
	//if (bReverseSpring)
	//{
	//	pt_lw_trumpet_req req;
	//	req.opcode = lw_trumpet_req;
	//	sprintf(buffer, CConfigManager::GetInstancePtr()->GetString("trumpet_reverse_spring_notify").c_str(), pWin->GetCorePlayer()->GetPlyStrAttr(PLY_ATTR_NICKNAME));
	//	req.message_ = buffer;
	//	req.ply_guid_ = 0;
	//	NotifyRoom(req, 3);
	//}
	////春天广播
	//if (bSpring)
	//{
	//	pt_lw_trumpet_req req;
	//	req.opcode = lw_trumpet_req;
	//	sprintf(buffer, CConfigManager::GetInstancePtr()->GetString("trumpet_spring_notify").c_str(), pPlayer->GetCorePlayer()->GetPlyStrAttr(PLY_ATTR_NICKNAME));
	//	req.message_ = buffer;
	//	req.ply_guid_ = 0;
	//	NotifyRoom(req, 3);
	//}
}

void CGameTable::UpdateTaskH5(CPlayer* pPlayer, int nScore[3])
{
	for (int i = 0; i < GetPlayNum(); i++)
	{
		CPlayer* t_player = GetPlayer(i);
		if (t_player)
		{
			IPlayer::PlyAttrItem itemTask(UP_RN_AT_ACHIEVE_COMMON_TOTAL, ATH5_ROUND_CHUJI + g_nHBMode%20 - 1, 1);
			t_player->GetCorePlayer()->UpdatePlyAttrs(itemTask);
			
			if (nScore[i] > 0 && (t_player->GetChairID() == m_Poke.m_nCurrentLord)) {
				IPlayer::PlyAttrItem itemTask(UP_RN_AT_ACHIEVE_TOTAL, ATH5_DDZ_LORD_WIN, 1);
				t_player->GetCorePlayer()->UpdatePlyAttrs(itemTask);				
			}

			if (nScore[i] > 0) {
				IPlayer::PlyAttrItem item2(UP_RN_SUCCESSIVE_VICTORY_ROUND, AT_LIAN_WIN, 1);
				t_player->GetCorePlayer()->UpdatePlyAttrs(item2);
			}
			else {
				IPlayer::PlyAttrItem item2(UP_RN_SUCCESSIVE_VICTORY_ROUND, AT_LIAN_WIN, 0);
				t_player->GetCorePlayer()->UpdatePlyAttrs(item2);
			}
		}
	}
}

void CGameTable::UpdateTask(CPlayer* pPlayer, int nScore[3])
{
	if (g_nIsBaiYuan == 1)
	{
		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* pPlayer = GetPlayer(i);
			if (pPlayer)
			{
				IPlayer::PlyAttrItem itemTask(UP_RN_AT_ACHIEVE_COMMON_TOTAL, ATH5_ROUND_CHUJI, 1);
				pPlayer->GetCorePlayer()->UpdatePlyAttrs(itemTask);
			}
		}
		return;
	}
	if (g_nHBMode >= HONGBAO_H5_CHUJI && g_nHBMode < HONGBAO_H5_END)	{
		UpdateTaskH5(pPlayer, nScore);
		return;
	}

	if (g_nNewMatch == 1)
	{
		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* t_player = GetPlayer(i);
			if (t_player)
			{
				IPlayer::PlyAttrItem itemDuiZhan(UP_RN_AT_ACHIEVE_TOTAL_QTT, AT_MATCH_DUIZHAN, 1);
				t_player->GetCorePlayer()->UpdatePlyAttrs(itemDuiZhan);

				if (nScore[i] > 0 && m_nDouble >= 48)
				{
					IPlayer::PlyAttrItem itemBeiShu(UP_RN_AT_ACHIEVE_TOTAL_QTT, AT_MATCH_48BEISHU, 1);
					t_player->GetCorePlayer()->UpdatePlyAttrs(itemBeiShu);
				}
			}
		}
		return;
	}
	char buffer[256];
	bool is_task_done = false;
	for (int i = 0; i < GetPlayNum(); i++)
	{
		CPlayer* t_player = GetPlayer(i);
		if (t_player)
		{
			bool complete_task = CompleteTask(t_player, nScore[i]);
			if (complete_task)						//检测是否有完成的任务需要广�?
			{
				if (g_nBroadcastTask == 1)
				{
					for (int j = CConfigManager::GetInstancePtr()->vecBroadCastTask_.size() - 1; j >= 0; j--)
					{
						if (CConfigManager::GetInstancePtr()->vecBroadCastTask_[j] == task_.current_task_id())
						{
							pt_lw_trumpet_req req;
							req.opcode = lw_trumpet_req;
							string nickname = t_player->GetCorePlayer()->GetPlyStrAttr(0, PLY_ATTR_NICKNAME);
							if (CConfigManager::GetInstancePtr()->CalcCharCountInWidth(nickname.c_str()) > 7)
							{
								nickname = nickname.substr(0, CConfigManager::GetInstancePtr()->CalcCharCountInUTF8(nickname.c_str(), 7)).append("...");
							}
							sprintf(buffer, CConfigManager::GetInstancePtr()->GetString("broadcast_complete_task").c_str(), nickname.c_str(), CConfigManager::GetInstancePtr()->vecBroadCastTask_desc_[j].c_str());
							req.message_ = buffer;
							req.ply_guid_ = 0;
							//NotifyRoom(req, 3, NULL, false);

							COutputStream os;
							os << req;
							m_pCoreTable->Send2World(os.GetData(), os.GetLength());
							break;
						}
					}
				}
			}
			if (complete_task)
			{
				is_task_done = true;
			}
			ComplteAchieve(t_player, nScore[i], complete_task, m_nDouble);
		}
	}

	if (g_nTask > 0)
	{
		if (is_task_done)
		{
			//默�?��?�战次数完成
			IPlayer::PlyAttrItem item(UP_RN_UPDATE_MISSION_INFO, task_.current_task_id(), 1);
			pPlayer->GetCorePlayer()->UpdatePlyAttrs(item);
		}
		else
		{
			IPlayer::PlyAttrItem item(UP_RN_UPDATE_MISSION_INFO, task_.current_task_id(), 0);
			pPlayer->GetCorePlayer()->UpdatePlyAttrs(item);
		}
	}
}
void CGameTable::RoundEnd(CPlayer* pWinPlayer)
{
	SERVER_LOG("CGameTable::RoundEnd()");
	m_nNoCallNum = 0;
	int tax = m_nTax;
	if (m_nBaseScore < 0)
	{
		m_nBaseScore = -m_nBaseScore;
	}
	SERVER_LOG("CGameTable::RoundEnd() tax:%d, m_nBaseScore:%d", tax, m_nBaseScore);

	int64 nScore[3];
	memset(nScore, 0, sizeof(nScore));

	int nRaceScore[3];
	memset(nRaceScore, 0, sizeof(nRaceScore));

	int64 nMoney[3];
	memset(nMoney, 0, sizeof(nMoney));

	IsSpring(pWinPlayer);

	LeftCardDouble(pWinPlayer);

	// 最低限�?
	if (g_nMinDouble > 0)
	{
		if (m_nDouble < g_nMinDouble)
		{
			m_nDouble = g_nMinDouble;
		}
	}

	// 最高限�?
	if (m_nMaxDouble > 0)
	{ 
		if (m_nDouble > m_nMaxDouble)
		{
			m_nDouble = m_nMaxDouble;
		}
	}
	//星空赛控制倍数
	if (g_nIsStarSky == 1)
	{
		m_nDouble = CConfigManager::GetInstancePtr()->GetStarSkyScoreByDouble(m_nDouble);
	}

	//�?�?
	for (int i = 0; i < GetPlayNum(); i++)
	{
		CPlayer* pShowPlayer = GetPlayer(i);
		if (pShowPlayer)
		{
			RefreshCards(pShowPlayer, NULL, true, true, false);
			if (g_nMatch == 1){//比赛�?
				nMoney[i] = pShowPlayer->GetCorePlayer()->GetPlyAttr(PLY_BASE_ATTR, PLY_MATCH_SCORE);
				glog.log("RoundEnd 0 chairId:%d plyUid:%s itemId:%d money:%lld g_nTax:%d tax:%d", i, pShowPlayer->m_pCorePlayer->GetPlyStrAttr(PLY_BASE_ATTR, PLY_ATTR_GUID), ITEM_BY_CASH, nMoney[i], g_nTax, tax);
			}
			else if (m_nSettleType == 1) {//私人�?分场
				nMoney[i] = 0;
				glog.log("RoundEnd 1 chairId:%d plyUid:%s itemId:%d money:%lld g_nTax:%d tax:%d", i, pShowPlayer->m_pCorePlayer->GetPlyStrAttr(PLY_BASE_ATTR, PLY_ATTR_GUID), ITEM_BY_CASH, nMoney[i], g_nTax, tax);
			}
			else if (g_nIsStarSky == 1){ //星空�?
				nMoney[i] = pShowPlayer->GetCorePlayer()->GetPlyAttr(ITEM_ATTR, ITEM_JOIN_MATCH_TICKET);
				glog.log("RoundEnd 2 chairId:%d plyUid:%s itemId:%d money:%lld g_nTax:%d tax:%d", i, pShowPlayer->m_pCorePlayer->GetPlyStrAttr(PLY_BASE_ATTR, PLY_ATTR_GUID), ITEM_BY_CASH, nMoney[i], g_nTax, tax);
			}
			else if (g_nIsBaiYuan == 1) { //百元�?
				nMoney[i] = pShowPlayer->getItemNum(ITEM_BY_CASH);
				glog.log("RoundEnd 3 chairId:%d plyUid:%s itemId:%d money:%lld g_nTax:%d tax:%d", i, pShowPlayer->m_pCorePlayer->GetPlyStrAttr(PLY_BASE_ATTR, PLY_ATTR_GUID), ITEM_BY_CASH, nMoney[i], g_nTax, tax);
			}
			else{
				nMoney[i] = pShowPlayer->GetGameMoney();
				glog.log("RoundEnd 4 chairId:%d plyUid:%s itemId:%d money:%lld g_nTax:%d tax:%d", i, pShowPlayer->m_pCorePlayer->GetPlyStrAttr(PLY_BASE_ATTR, PLY_ATTR_GUID), ITEM_BY_CASH, nMoney[i], g_nTax, tax);
			}
		}

		if (g_nTax > 1)
		{
			nMoney[i] -= tax;
			glog.log("RoundEnd 000 chairId:%d itemId:%d money:%lld g_nTax:%d tax:%d", i, ITEM_BY_CASH, nMoney[i], g_nTax, tax);
		}
		glog.log("RoundEnd --- chairId:%d itemId:%d money:%lld g_nTax:%d tax:%d", i, ITEM_BY_CASH, nMoney[i], g_nTax, tax);

		if (m_nSettleType == 0)//金币�?
		{
			if (nMoney[i] < 0)
			{
				glog.log("RoundEnd 111 chairId:%d itemId:%d money:%lld g_nTax:%d tax:%d", i, ITEM_BY_CASH, nMoney[i], g_nTax, tax);
				nMoney[i] = 0;
			}
		}
		glog.log("RoundEnd === chairId:%d itemId:%d money:%lld g_nTax:%d tax:%d", i, ITEM_BY_CASH, nMoney[i], g_nTax, tax);

		if (pShowPlayer && pShowPlayer->m_nBetLordCardIndex >= 0 && pShowPlayer->m_nBetLordCardReward > 0)
		{
			pShowPlayer->UpdateLordCardLotteryReward();
		}
	}

	SaveAllCards();

	// 计算分�?
	CalculateScore(pWinPlayer, nScore, nRaceScore, nMoney);

	//记录赢�?��?�数
	if (g_nPrivateRoom == 1)//不是私人�?
	{
		if (pWinPlayer->GetChairID() == m_Poke.m_nCurrentLord)
		{
			AddRecordTimes(pWinPlayer->GetChairID(), eRECORD_WIN);
		}
		else
		{
			for (int i = 0; i < GetPlayNum(); i++)
			{
				if (i == m_Poke.m_nCurrentLord)
				{
					continue;
				}
				CPlayer* pWin = GetPlayer(i);
				if (pWin)
				{
					AddRecordTimes(pWin->GetChairID(), eRECORD_WIN);
				}
			}
		}
	}

	int nScoreBackup[3] = { nScore[0], nScore[1], nScore[2] };
	UpdateTask(pWinPlayer, nScoreBackup);

	//新加结果消息 增加比赛场积�?
	pt_gc_game_result_not1 noti1;
	noti1.opcode = gc_game_result_not1;

	// 发送结果通知
	pt_gc_game_result_not noti;
	noti.opcode = gc_game_result_not;
	noti.bType = (pWinPlayer->GetChairID() == m_Poke.m_nCurrentLord ? 1 : 2);
	noti.cDouble = m_nDouble;
	noti.cCallScore = m_nCallScore;
	noti.bShowCard = m_bShowCard;
	noti.nBombCount = (m_Poke.m_nBombCounter > 0 ? (1 << m_Poke.m_nBombCounter) : 0);
	noti.bSpring = m_Poke.m_bSpring;
	noti.bReverseSpring = m_Poke.m_bReverseSpring;
	noti.bRobLord = m_nRobDouble > 1 ? m_nRobDouble : 1;


	for (int i = 0; i < GetPlayNum(); i++)
	{
		SERVER_LOG("===========g_nMaxMoney:%d  player:%d  money:%d", g_nMaxMoney, i, nScore[i]);
		CPlayer* pPlayer = GetPlayer(i);
		if (pPlayer)
		{

			stUserResult rs;
			rs.nChairID = i;
			rs.nScore = nScore[i];
			noti.vecUserResult.push_back(rs);

			stUserResult1 rs1;
			rs1.nChairID = i;

			if (g_nMatch == 1){
				rs1.nScore = nRaceScore[i];
			}
			else{
				rs1.nScore = nScore[i];
			}
			if (g_nRace == 1){
				rs1.nJifen = nRaceScore[i];
			}
			else{
				rs1.nJifen = nMoney[i];
			}

			noti1.vecUserResult1.push_back(rs1);

			IPlayer* corePlayer = pPlayer->GetCorePlayer();
			if (g_nMatch == 1)
			{
				IPlayer::PlyAttrItem item(UP_RN_MATCH_SCORE, PLY_MATCH_SCORE, nRaceScore[i]);
				if (corePlayer)
				{
					corePlayer->UpdatePlyAttrs(item);
				}
			}
			else
			{
				if (g_nIsStarSky == 1)
				{
					IPlayer::PlyAttrItem item_(UP_RN_SPECIFY_ITEM, ITEM_JOIN_MATCH_TICKET, nScore[i] - CConfigManager::GetInstancePtr()->GetStarSkyTaxByDouble(nScore[i]), 0, m_strGameLabel);
					if (corePlayer)
					{
						corePlayer->UpdatePlyAttrs(item_);
					}

					pPlayer->UpdateStarScore(CPlayer::SR_UPDATE, nScore[i] > 0 ? 4 : 0);
				}
				else if (g_nIsBaiYuan == 1)
				{
					char buffer[512];
					sprintf(buffer, "baiyuan_result item:%d lord:%d win:%d shuffle:%d better:%d robot:%d", pPlayer->getItemNum(ITEM_BY_CASH), m_Poke.m_nCurrentLord, pWinPlayer->GetChairID(), m_nShuffleLogType, m_nShuffleLogBetterSeat, m_nShuffleLogRobotSeat);

					IPlayer::PlyAttrItem item_(UP_RN_SPECIFY_ITEM, ITEM_BY_CASH, nScore[i], 0, buffer);
					if (corePlayer)
					{
						corePlayer->UpdatePlyAttrs(item_);
					}

					int index = PLY_ATTR_MONEY;
					int score = 0;
					if (g_nGame_id == GAME_BY_DDZ) {
						index = -1;
						score = nScore[i];
					}

					IPlayer::PlyAttrItem item2_(UP_RN_GAME_RESULT, index, nScore[i], 0, m_strGameLabel);
					if (corePlayer)
					{
						corePlayer->UpdatePlyAttrs(item2_);
					}
				}
				else if (m_nSettleType == 1)//�?�?
				{
					PLY_ATTR_UPDATE_REASON eReason = UP_RN_PRIVATE_ROOM_SCORE;
					if (g_nNewMatch == 1)
					{
						eReason = UP_RN_MATCH_ROOM_SCORE;

						// 记录当局最大倍数
						if (nScore[i] > 0){
							g_pLogger->Log("CGameTable::RoundEnd BeiMatch GUID[%s], Double[%d], Score[%d]", corePlayer->GetPlyStrAttr(PLY_BASE_ATTR, PLY_ATTR_GUID), m_nDouble, nScore[i]);
							IPlayer::PlyAttrItem item_(UP_RN_UPDATE_MATCH_ACHIEV, 0, m_nDouble, 0, m_strGameLabel);
							if (corePlayer)
							{
								corePlayer->UpdatePlyAttrs(item_);
							}
						}
					}

					IPlayer::PlyAttrItem item_(eReason, ITEM_SCORE, nScore[i], 0, m_strGameLabel);
					if (corePlayer)
					{
						corePlayer->UpdatePlyAttrs(item_);
					}
					if (g_nPrivateRoom == 1)
					{
						AddRecordTimes(i, eRECORD_ZHNAJI, nScore[i]);
					}
				}
				else if (g_nHBMode > 0 && g_nHBMode < HONGBAO_H5_CHUJI)
				{
					int nChangeNum = g_nTax > 1 ? nScore[i] - tax : nScore[i];
					IPlayer::PlyAttrItem item_(UP_RN_SPECIFY_ITEM, ITEM_GOLD_LEAF, nChangeNum, 0, m_strGameLabel);
					if (corePlayer)
					{
						corePlayer->UpdatePlyAttrs(item_);
					}
// 					IPlayer::PlyAttrItem item2_(UP_RN_PRIVATE_ROOM_SCORE, PLY_ATTR_MONEY, nChangeNum, 0, m_strGameLabel);
// 					if (corePlayer)
// 					{
// 						corePlayer->UpdatePlyAttrs(item2_);
// 					}

					//if (g_nRace == 1)
					//{
					//	IPlayer::PlyAttrItem item3_(UP_RN_GAME_RESULT, PLY_ATTR_SCORE, nRaceScore[i]);
					//	if (corePlayer)
					//	{
					//		corePlayer->UpdatePlyAttrs(item3_);
					//	}
					//}
					//else if (g_nPrivateRoom == 1)
					//{
					//	AddRecordTimes(i, eRECORD_ZHNAJI, nScore[i]);
					//}
					//else
					//{
					//	IPlayer::PlyAttrItem item3_(UP_RN_GAME_RUSULT_STATUS, PLY_ATTR_MONEY, nScore[i] > 0 ? 1 : 0);
					//	if (corePlayer)
					//	{
					//		corePlayer->UpdatePlyAttrs(item3_);
					//	}
					//}
				}
				else//游戏�?
				{
					int nChangeNum = g_nTax > 1 ? nScore[i] - tax : nScore[i];

					if (nChangeNum < 0 && pPlayer->getInvincibleStatus() && g_nInvincibleMode == 1)
					{
						nChangeNum = 0;
					}

					IPlayer::PlyAttrItem item_(UP_RN_GAME_RESULT, PLY_ATTR_MONEY, nChangeNum, 0, m_strGameLabel);
					if (corePlayer)
					{
						corePlayer->UpdatePlyAttrs(item_);
					}
					IPlayer::PlyAttrItem item2_(UP_RN_PRIVATE_ROOM_SCORE, PLY_ATTR_MONEY, nChangeNum, 0, m_strGameLabel);
					if (corePlayer)
					{
						corePlayer->UpdatePlyAttrs(item2_);
					}

					if (g_nRace == 1)
					{
						IPlayer::PlyAttrItem item3_(UP_RN_GAME_RESULT, PLY_ATTR_SCORE, nRaceScore[i]);
						if (corePlayer)
						{
							corePlayer->UpdatePlyAttrs(item3_);
						}
					}
					else if (g_nPrivateRoom == 1)
					{
						AddRecordTimes(i, eRECORD_ZHNAJI, nScore[i]);
					}
					else
					{
						IPlayer::PlyAttrItem item3_(UP_RN_GAME_RUSULT_STATUS, PLY_ATTR_MONEY, nScore[i] > 0 ? 1 : 0);
						if (corePlayer)
						{
							corePlayer->UpdatePlyAttrs(item3_);
						}
					}
				}
			}

			if (corePlayer && g_nCounts == 1 && pPlayer->m_bUseCounts)//�?否使用过了�?�牌�?
			{
				int n_num = corePlayer->GetPlyAttr(ITEM_ATTR, ITEM_CARD_RECORD);
				if (n_num > 0)
				{
					IPlayer::PlyAttrItem item2_(UP_RN_USE_ITEM, ITEM_CARD_RECORD, -1);
					corePlayer->UpdatePlyAttrs(item2_);

					pt_gc_counts_not noti;
					noti.opcode = gc_counts_not;
					noti.counts_num_ = n_num - 1;
					pPlayer->SendPacket(noti);

					pt_gc_counts_not1 noti1;
					noti1.opcode = gc_counts_not1;
					noti1.counts_num_ = n_num - 1;
					pPlayer->SendPacket(noti1);
				}
			}
		}

		if (g_nBroadcastWin == 1) //赢牌广播
		{
			if (nScore[i] >= g_nBroadcastWinNum)
			{
				char buffer[256];
				pt_lw_trumpet_req req;
				req.opcode = lw_trumpet_req;
				sprintf(buffer, CConfigManager::GetInstancePtr()->GetString("broadcast_win").c_str(), GetPlayer(i)->GetCorePlayer()->GetPlyStrAttr(0, PLY_ATTR_NICKNAME), nScore[i] / 10000);
				req.message_ = buffer;
				req.ply_guid_ = 0;
				//NotifyRoom(req, 3, NULL, false);

				COutputStream os;
				os << req;
				m_pCoreTable->Send2World(os.GetData(), os.GetLength());
			}
		}
	}

	noti1.bType = noti.bType;
	noti1.cDouble = noti.cDouble;
	noti1.cCallScore = noti.cCallScore;
	noti1.bShowCard = noti.bShowCard;
	noti1.nBombCount = noti.nBombCount;
	noti1.bSpring = noti.bSpring;
	noti1.bReverseSpring = noti.bReverseSpring;
	noti1.bRobLord = noti.bRobLord;
	NotifyRoom(noti1);

	NotifyRoom(noti, 0, NULL, false);

	if (g_nHBMode == 1 || g_nIsBaiYuan == 1)
	{
		refreshPlayerTokenMoney();
	}
	//////////////////////////////////////////////////////////////////////////

	for (int i = 0; i < GetPlayNum(); i++)
	{
		CPlayer* p = GetPlayer(i);
		if (p != NULL)
		{
			p->genRegainLoseMoney(nScore[i]);
		}
	}

	for (int i = 0; i < GetPlayNum(); i++)
	{
		CPlayer* p = GetPlayer(i);
		if (p != NULL)
		{
			int poolDeltaRate = 0;
			vector<structRedPackeLimit>::reverse_iterator vecRedPacketLimitIt = CConfigManager::GetInstancePtr()->m_vecRedPacketLimitConfig.rbegin();

			for (; vecRedPacketLimitIt != CConfigManager::GetInstancePtr()->m_vecRedPacketLimitConfig.rend(); ++vecRedPacketLimitIt) {
				if (vecRedPacketLimitIt->nPoolDeltaRate > 0 && p->getPayValueRate() >= vecRedPacketLimitIt->nPayRate)
				{
					poolDeltaRate = vecRedPacketLimitIt->nPoolDeltaRate;
					break;
				}
			}
			
			int realPoolRate = g_nHBPoolRate-poolDeltaRate;
			if (g_nHB88YuanEnable > 0)
			{
				realPoolRate = realPoolRate - g_nHB88YuanPoolRate;
			}
			realPoolRate = realPoolRate < 0 ? 0 : realPoolRate;
			
			p->m_nRedPackets_AwardPool += g_nTax * realPoolRate / 100;
			p->m_nRedPackets_88Yuan_AwardPool += g_nTax * g_nHB88YuanPoolRate / 100;
			p->m_nGameScore = nScore[i];
			p->RoundEnd(nScore[i]);
		}
	}


	//////////////////////////////////////////////////////////////////////////

	m_currentTime_ = 0;
	m_pCoreTable->RemoveTimer();
	m_pCoreTable->EndGame();
	m_nAddFan2 = 0;
	m_bRacing = false;

	if (g_nPrivateRoom == 1)
	{
		pt_ss_update_round_mj packet;
		packet.opcode = ss_update_round_mj;
		packet.num_ = 1;
		UpdateToTable(packet);

		StartTimer(-1, eREADY_EVENT);
	}


	


}

void CGameTable::SendCommonCmd(int nOp, int nChairID, CPlayer* toPlayer /*= NULL*/)
{
	SERVER_LOG("CGameTable::SendCommonCmd(nOp:%d,nChairID:%d)", nOp, nChairID);
	pt_gc_common_not noti;
	noti.opcode = gc_common_not;
	noti.nOp = nOp;
	noti.cChairID = nChairID;
	

	if (toPlayer)
	{
		toPlayer->SendPacket(noti);
		return;
	}
	NotifyRoom(noti);


	//	if( nOp == CO_NEW )
	//	{
	//
	//	}
	//	else if( nOp >= CO_CALL0 && nOp <= CO_ROB )
	//	{
	//
	//	}
	//	else if( nOp == CO_GIVEUP )
	//	{
	//
	//	}
}

void CGameTable::SendDoubleInfo()
{
	SERVER_LOG("CGameTable::SendDoubleInfo()");
	if (g_nRPGMode)
	{
		return;
	}
	if (m_nMaxDouble > 0)
	{
		if (m_nDouble > m_nMaxDouble)
		{
			m_nDouble = m_nMaxDouble;
		}
	}
	pt_gc_bomb_not noti;
	noti.opcode = gc_bomb_not;
	noti.nDouble = m_nDouble;
	NotifyRoom(noti);
}

void CGameTable::CallScoreAndSendDoubleInfo()
{
	if (g_nTwoPai == 1)//二人斗地�?
	{
		m_nDouble = m_nInitDouble * 2;
	}
	else
	{
		m_nDouble = m_nInitDouble * m_nCallScore;

	}


	if (g_nRPGMode == 1) {		
		setMutiDoubleDetail(BEI_SHU_INFO_CALLSCORE, m_nCallScore);
	}else {
		setMutiDoubleDetail(BEI_SHU_INFO_INIT, m_nDouble);
	}

	

	SERVER_LOG("------CGameTable::CallScoreAndSendDoubleInfo() %d", m_nDouble);
	SendDoubleInfo();
}

void CGameTable::GetCompleteData(CPlayer *pPlayer)
{
	SERVER_LOG("CGameTable::GetCompleteData()");
	if (!m_bRacing)
	{
		return;
	}
	else if (m_bRacing)
	{
	
		if (g_nHBMode == 1 || g_nIsBaiYuan == 1)
		{
			refreshPlayerTokenMoney();
			pPlayer->sendMagicEmojiConfig();
		}

		if (g_nHBMode == 1)
		{
			pPlayer->SendRedPacketsResult();
			pPlayer->sendRedPacket88YuanData();
		}

		if (pPlayer->m_nBetLordCardNextIndex >= 0)
		{
			pt_gc_bet_lord_card_ack ack;
			ack.opcode = gc_bet_lord_card_ack;
			ack.index = pPlayer->m_nBetLordCardNextIndex;
			ack.ret = -4;
			pPlayer->SendPacket(ack);
		}
		if (g_nTwoPai == 1)//二人斗地�?
		{
			pt_gc_two_complete_data_not noti;
			noti.opcode = gc_two_complete_data_not;
			noti.nGameMoney = m_nBaseScore;
			noti.nDouble = m_nDouble;//(1 << m_Poke.m_nBombCounter);
			noti.nStart = m_bStart ? 1 : 0;
			noti.cLord = m_Poke.m_nCurrentLord;
			noti.vecLordCards = m_Poke.m_cLordCard;
			if (!m_bLordDeal)//m_Poke.m_nDecideLordRound < 3
			{
				for (size_t i = 0; i < noti.vecLordCards.size(); i++)
				{
					CCard& cCard = noti.vecLordCards[i];
					cCard.m_nColor = 0;
					cCard.m_nValue = 0;
				}
			}

			for (int i = 0; i < GetPlayNum(); i++)
			{
				CPlayer* pCurPlayer = GetPlayer(i);
				if (pCurPlayer)
				{
					stUserData data;
					data.cChairID = i;
					data.vecPutCards = pCurPlayer->m_PlayCard.m_cChoosingCards;
					data.vecHandCards = pCurPlayer->m_PlayCard.m_cCards;

					if ((pPlayer->GetStatus() == CPlayer::PS_PLAYER && pPlayer->GetChairID() == i) ||
						(pPlayer->GetStatus() == CPlayer::PS_VISTOR && pPlayer->GetChairID() == i && pCurPlayer->CanWatch()))
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
			noti.nLetNum = GetLetCardNum();
			pPlayer->SendPacket(noti);

			sendLordDoubleScore(pPlayer->GetChairID());
		}
		else
		{
			SendDoubleDetail();

			pt_gc_complete_data_not noti;
			noti.opcode = gc_complete_data_not;
			noti.nGameMoney = m_nBaseScore;
			noti.nDouble = m_nDouble;//(1 << m_Poke.m_nBombCounter);
			noti.cLord = m_Poke.m_nCurrentLord;
			noti.vecLordCards = m_Poke.m_cLordCard;
			
			if (!m_bLordDeal && !pPlayer->m_bLookCard)//m_Poke.m_nDecideLordRound < 3
			{
				for (size_t i = 0; i < noti.vecLordCards.size(); i++)
				{
					CCard& cCard = noti.vecLordCards[i];
					cCard.m_nColor = 0;
					cCard.m_nValue = 0;
				}
			}

			for (int i = 0; i < GetPlayNum(); i++)
			{
				CPlayer* pCurPlayer = GetPlayer(i);
				if (pCurPlayer)
				{
					stUserData data;
					data.cChairID = i;
					data.vecPutCards = pCurPlayer->m_PlayCard.m_cChoosingCards;
					data.vecHandCards = pCurPlayer->m_PlayCard.m_cCards;

					if ((pPlayer->GetStatus() == CPlayer::PS_PLAYER && pPlayer->GetChairID() == i) ||
						(pPlayer->GetStatus() == CPlayer::PS_VISTOR && pPlayer->GetChairID() == i && pCurPlayer->CanWatch()) ||
						pCurPlayer->isShowCard())
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
			pPlayer->SendPacket(noti);
		}

#if 0 //TODO
		if (m_bShowCard)
		{
			pt_gc_show_card_not show_noti;
			show_noti.opcode = gc_show_card_not;
			show_noti.nChairID = m_Poke.m_nCurrentLord;
			//show_noti.vecCards = pPlayer->m_PlayCard.m_cCards;
			show_noti.vecCards = GetPlayer(m_Poke.m_nCurrentLord)->m_PlayCard.m_cCards;
			pPlayer->SendPacket(show_noti);
		}
#endif
		if (g_nLetCard == 1)
		{
			pt_gc_two_let_card_not noti;
			noti.opcode = gc_two_let_card_not;
			noti.nLetNum = m_nLetCardNum;
			pPlayer->SendPacket(noti);
		}

		if (g_nIsBaiYuan == 1 && g_nBaiYuanHBRound > 0)
		{
			pPlayer->SendBaiYuanHBRound();
		}

		if (task_.current_task_id() >= 0)			//
		{
			pt_gc_task_not task_noti;
			task_noti.opcode = gc_task_not;
			task_noti.task_item_ = task_.GetTask(task_.current_task_id());


			pPlayer->SendPacket(task_noti);
		}

		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* pCurPlayer = GetPlayer(i);
			if (pCurPlayer)
			{
				if (pCurPlayer->m_bAuto)
				{
					pt_gc_auto_not noti;
					noti.opcode = gc_auto_not;
					noti.cChairID = pCurPlayer->GetChairID();
					noti.cAuto = pCurPlayer->m_bAuto;
					pPlayer->SendPacket(noti);
				}

				if (pCurPlayer->m_bUseCounts) // �?否使用�?�牌�?
				{
					pt_gc_use_card_recode_noti noti;
					noti.opcode = gc_use_card_recode_noti;
					noti.cChairID = pCurPlayer->GetChairID();
					noti.cReconnection = 1;
					pPlayer->SendPacket(noti);
				}
			}
		}

		SendJuCount(pPlayer);
	}
}

void CGameTable::ClientTimerNot(char cChairID, int nPeriod)
{
	SERVER_LOG("CGameTable::ClientTimerNot(cChairID:%d,nPeriod:%d)", cChairID, nPeriod);
	pt_gc_clienttimer_not noti;
	noti.opcode = gc_clienttimer_not;
	noti.chairId = cChairID;
	noti.sPeriod = nPeriod;
	NotifyRoom(noti);
}

void CGameTable::DealingTask()
{
	SERVER_LOG("CGameTable::DealingTask()");
	if (g_nTask == 1)
	{
		if (g_nTID >= 0)   //指定任务
		{
			pt_gc_task_not noti;
			noti.opcode = gc_task_not;
			noti.task_item_ = task_.GetTask(g_nTID);
			NotifyRoom(noti, 0, NULL, false);
		}
		else				//随机任务
		{
			pt_gc_task_not noti;
			noti.opcode = gc_task_not;
			task_.GetTask(noti.task_item_);
			NotifyRoom(noti, 0, NULL, false);
		}
	}
}

bool CGameTable::CompleteTask(CPlayer* pPlayer, int nScore)
{
	SERVER_LOG("CGameTable::CompleteTask()");
	if (g_nTask == 0)
		return false;
	if (task_.CheckTaskComplete(pPlayer, nScore))  //完成任务
	{
		pt_gc_task_complete_not noti;
		noti.opcode = gc_task_complete_not;
		noti.chair_id_ = pPlayer->GetChairID();
		noti.task_status_ = 1;			//0：未完成  1：完�?
		pPlayer->SendPacket(noti);//NotifyRoom(noti);

		//		vector<IPlayer::PlyAttrItem> values;
		// 		IPlayer::PlyAttrItem item_ ;
		// 		item_.reason_ = UP_RN_FNINISH_MISSION;
		if (task_.current_task().task_money_type_ == 0)		//0游戏�? 1元宝
		{
			// 			item_.index_ = PLY_ATTR_MONEY;
			// 			item_.value1_ = task_.current_task().task_money_;
			// 			char buff[64];
			// 			sprintf(buff,"%d",task_.current_task().task_id_);
			// 			item_.value2_ = buff;
			// 			values.push_back(item_);
			// 			pPlayer->GetCorePlayer()->UpdatePlyAttrs(values);
			IPlayer::PlyAttrItem item_(UP_RN_FNINISH_MISSION, PLY_ATTR_MONEY, task_.current_task().task_money_);
			pPlayer->GetCorePlayer()->UpdatePlyAttrs(item_);
		}
		else
		{
			// 			item_.index_ = PLY_ATTR_GIFT;
			// 			item_.value1_ = task_.current_task().task_money_;
			// 			char buff[64];
			// 			sprintf(buff,"%d",task_.current_task().task_id_);
			// 			item_.value2_ = buff;
			// 			values.push_back(item_);
			// 			pPlayer->GetCorePlayer()->UpdatePlyAttrs(values);
			IPlayer::PlyAttrItem item_(UP_RN_FNINISH_MISSION, PLY_ATTR_GIFT, task_.current_task().task_money_);
			pPlayer->GetCorePlayer()->UpdatePlyAttrs(item_);
		}


		return true;
	}
	return false;

}

void CGameTable::ComplteAchieve(CPlayer* pPlayer, int nScore, bool isCompleteTask, int n_times)
{
	SERVER_LOG("CGameTable::ComplteAchieve()");
	vector<IPlayer::PlyAttrItem> items;
	//默�?��?�战次数完成
	IPlayer::PlyAttrItem item(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_DUIZHAN, 0);
	items.push_back(item);

	//检查是否胜�?
	if (nScore > 0)
	{
		if (g_nGameType == eBISAICHANG)
		{
			if (n_times >= 24)
			{
				IPlayer::PlyAttrItem item(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_BISAIRENWU, 0);
				items.push_back(item);
			}
		}
		else if (g_nGameType == eZHIZUNCHANGE)
		{
			IPlayer::PlyAttrItem item0(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_DAILY_ZHIZUN, 0);
			items.push_back(item0);

			IPlayer::PlyAttrItem item(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_DAILY_ZHIZUN_WIN, 0);
			items.push_back(item);
		}
		else if (g_nGameType == eGAOJICHANG)
		{
			if (n_times >= 48)
			{
				IPlayer::PlyAttrItem item(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_GAOJI48BEISHU, 0);
				items.push_back(item);
			}

			//完成任务次数
			if (isCompleteTask)
			{
				IPlayer::PlyAttrItem item(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_GAOJIRENWU, 0);
				items.push_back(item);
			}

			IPlayer::PlyAttrItem item0(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_DAILY_GAOJI, 0);
			items.push_back(item0);

			IPlayer::PlyAttrItem item1(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_DAILY_GAOJI_WIN, 0);
			items.push_back(item1);

			IPlayer::PlyAttrItem item2(UP_RN_SUCCESSIVE_VICTORY_ROUND, AT_LIAN_WIN, 1);
			items.push_back(item2);
		}
		else if (g_nGameType == eZHONGJICHANG)
		{
			if (n_times >= 12)
			{
				IPlayer::PlyAttrItem item(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_ZHONGJI12BEISHU, 0);
				items.push_back(item);
			}

			//出炸弹�?�数
			bool is_zhadan_ = false;
			for (size_t i = 0; i < pPlayer->m_vecPutCard.size(); i++)
			{
				if (pPlayer->m_vecPutCard[i].m_nTypeBomb != 0)
					is_zhadan_ = true;
			}

			for (size_t i = 0; i < pPlayer->m_vecPutCard.size(); i++)
			{
				if (pPlayer->m_vecPutCard[i].m_nTypeBomb == 2 && pPlayer->m_vecPutCard[i].m_nTypeValue == 16)
					is_zhadan_ = true;
			}
			if (is_zhadan_)
			{
				IPlayer::PlyAttrItem item(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_ZHONGJIZHADAN, 0);
				items.push_back(item);
			}

			IPlayer::PlyAttrItem item0(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_DAILY_ZHONGJI, 0);
			items.push_back(item0);

			IPlayer::PlyAttrItem item1(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_DAILY_ZHONGJI_WIN, 0);
			items.push_back(item1);

			IPlayer::PlyAttrItem item2(UP_RN_SUCCESSIVE_VICTORY_ROUND, AT_LIAN_WIN, 1);
			items.push_back(item2);
		}
		else if (g_nGameType == eCHUJICHANG)
		{
			IPlayer::PlyAttrItem item0(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_CHUJISHENGLI, 0);
			items.push_back(item0);

			IPlayer::PlyAttrItem item(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_DAILY_CHUJI, 0);
			items.push_back(item);

			IPlayer::PlyAttrItem item1(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_DAILY_CHUJI_WIN, 0);
			items.push_back(item1);

			IPlayer::PlyAttrItem item2(UP_RN_SUCCESSIVE_VICTORY_ROUND, AT_LIAN_WIN, 1);
			items.push_back(item2);
		}

		IPlayer::PlyAttrItem itemWinTask(UP_RN_AT_ACHIEVE_TOTAL, T_WIN_ROUND + 1000, 1);
		pPlayer->GetCorePlayer()->UpdatePlyAttrs(itemWinTask);
	}
	else
	{
		if (g_nGameType == eCHUJICHANG)
		{
			IPlayer::PlyAttrItem item(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_DAILY_CHUJI, 0);
			items.push_back(item);

			IPlayer::PlyAttrItem item2(UP_RN_SUCCESSIVE_VICTORY_ROUND, AT_LIAN_WIN, 0);
			items.push_back(item2);
		}
		else if (g_nGameType == eZHONGJICHANG)
		{
			IPlayer::PlyAttrItem item(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_DAILY_ZHONGJI, 0);
			items.push_back(item);

			IPlayer::PlyAttrItem item2(UP_RN_SUCCESSIVE_VICTORY_ROUND, AT_LIAN_WIN, 0);
			items.push_back(item2);
		}
		else if (g_nGameType == eGAOJICHANG)
		{
			IPlayer::PlyAttrItem item(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_DAILY_GAOJI, 0);
			items.push_back(item);

			IPlayer::PlyAttrItem item2(UP_RN_SUCCESSIVE_VICTORY_ROUND, AT_LIAN_WIN, 0);
			items.push_back(item2);
		}
		else if (g_nGameType == eZHIZUNCHANGE)
		{
			IPlayer::PlyAttrItem item(UP_RN_UPDATE_ACHIEVE_CONDITION, AT_DAILY_ZHIZUN, 0);
			items.push_back(item);
		}
	}
	for (size_t i = 0; i < items.size(); i++)
	{
		pPlayer->GetCorePlayer()->UpdatePlyAttrs(items[i]);
	}
}

void CGameTable::AddLetCardNum()//添加让牌数目并发送给所有玩�?
{
	SERVER_LOG("CGameTable::AddLetCardNum");
	if (g_nLetCard == 1)
	{
		m_nLetCardNum++;

		SERVER_LOG("CGameTable::AddLetCardNum %d", m_nLetCardNum);

		pt_gc_two_let_card_not noti;
		noti.opcode = gc_two_let_card_not;
		noti.nLetNum = m_nLetCardNum;
		NotifyRoom(noti);
	}
}

bool CGameTable::IsCanEndRound(CPlayer* player)
{
	if (player == NULL)
	{
		return false;
	}
	
	// 19.05.21 新需求：二人斗地主地主也让牌获胜
	if (player->GetChairID() == m_Poke.m_nCurrentLord)//地主
	{
		return g_nTwoPaiLordLet >= (int)player->m_PlayCard.m_cCards.size();
		//return player->m_PlayCard.m_cCards.empty();
	}
	else//二人斗地主的话m_nLetCardNum = 0
	{
		return m_nLetCardNum >= (int)player->m_PlayCard.m_cCards.size();
	}
}

bool CGameTable::LordCardIsSeries()
{
	if (m_Poke.m_cLordCard.size() != 3)
	{
		return false;
	}

	for (size_t i = 0; i < m_Poke.m_cLordCard.size(); i++)
	{
		if (m_Poke.m_cLordCard[i].m_nValue >= 16)
		{
			return false;//有大小王
		}
	}

	//for (int i = 1; i < m_Poke.m_cLordCard.size(); i++)//todo
	//{
	//	int nValue = m_Poke.m_cLordCard[i - 1].m_nValue;
	//	if ((nValue + 1) != m_Poke.m_cLordCard[i].m_nValue)
	//	{
	//		return false;//大小不一�?
	//	}
	//}

	int nValue[3];
	memset(nValue, 0, sizeof(nValue));
	nValue[0] = m_Poke.m_cLordCard[0].m_nValue - m_Poke.m_cLordCard[1].m_nValue;
	nValue[1] = m_Poke.m_cLordCard[1].m_nValue - m_Poke.m_cLordCard[2].m_nValue;
	nValue[2] = m_Poke.m_cLordCard[2].m_nValue - m_Poke.m_cLordCard[0].m_nValue;
	for (int i = 0; i < 3; i++)
	{
		if (nValue[i] < 0)
		{
			nValue[i] = -nValue[i];
		}
	}
	if (nValue[0] == 1)
	{
		if (nValue[1] == 1 && nValue[2] == 2)
		{
			return true;
		}
		else if (nValue[1] == 2 && nValue[2] == 1)
		{
			return true;
		}
	}
	else if (nValue[0] == 2)
	{
		if (nValue[1] == 1 && nValue[2] == 1)
		{
			return true;
		}
	}

	return false;
}

bool CGameTable::LordCardIsSameColor()
{
	if (m_Poke.m_cLordCard.size() != 3)
	{
		return false;
	}

	for (size_t i = 0; i < m_Poke.m_cLordCard.size(); i++)
	{
		if (m_Poke.m_cLordCard[i].m_nValue >= 16)
		{
			return false;//有大小王
		}
	}

	int nColor = m_Poke.m_cLordCard[0].m_nColor;
	for (size_t i = 1; i < m_Poke.m_cLordCard.size(); i++)
	{
		if (nColor != m_Poke.m_cLordCard[i].m_nColor)
		{
			return false;//颜色不一�?
		}
	}

	return true;
}

bool CGameTable::LordCardIs3()
{
	if (m_Poke.m_cLordCard.size() != 3)
	{
		return false;
	}

	for (size_t i = 0; i < m_Poke.m_cLordCard.size(); i++)
	{
		if (m_Poke.m_cLordCard[i].m_nValue >= 16)
		{
			return false;//有大小王
		}
	}

	int nValue = m_Poke.m_cLordCard[0].m_nValue;
	for (size_t i = 1; i < m_Poke.m_cLordCard.size(); i++)
	{
		if (nValue != m_Poke.m_cLordCard[i].m_nValue)
		{
			return false;//大小不一�?
		}
	}

	return true;
}

bool CGameTable::LordCardIsSmall()
{
	if (m_Poke.m_cLordCard.size() != 3)
	{
		return false;
	}

	for (size_t i = 0; i < m_Poke.m_cLordCard.size(); i++)
	{
		if (m_Poke.m_cLordCard[i].m_nValue >= 10)
		{
			return false;
		}
	}

	return true;
}

void CGameTable::ProcessPacket(const char* pData, int nLength)
{
	CInputStream is(pData, nLength);
	short opcode;
	is >> opcode;

	is.Reset();
	switch (opcode)
	{
		// DISPATCH_MESSAGE(bg_change_basescore_not);
	}
}

void CGameTable::OnPacket(const pt_bg_change_basescore_not& noti)
{
	SetBaseMoney();
	SendBaseMoney();
}

void CGameTable::SaveAllCards()
{
	if (g_nPrivateRoom == 0)
	{
		return;
	}
	pt_gc_replay_data_not noti;
	noti.opcode = gc_replay_data_not;
	for (int i = 0; i < GetPlayNum(); ++i)
	{
		CPlayer* pPlayer = GetPlayer(i);
		if (pPlayer)
		{
			pt_gc_refresh_card_not minnoti;
			pPlayer->GetPlayerCards(minnoti, true);		//明牌 
			noti.vecChangeCards.push_back(minnoti);
		}
	}

	COutputStream os;
	os << noti;
	m_pCoreTable->PushReplayPacket(os.GetData(), os.GetLength());
}

void CGameTable::OnGetPrivateRoomResult(CPlayer* pPlayer)
{
	pt_gc_private_room_result_ack ack;
	ack.opcode = gc_private_room_result_ack;
	ack.vecGameStatiscs = m_sGameStatic;
	pPlayer->SendPacket(ack);
}

//---------------------------- [[ 通知�?标玩家游戏模�? ]] ---------------------------- //
void CGameTable::NotifyGameModle(CPlayer* pPlayer /*= nullptr*/)
{
	// 1. 消息定义
	pt_gc_game_model noti;
	noti.opcode			= gc_game_modle;
	noti.cModelType		= g_nIsStandard != 1 ? 0 : 1;

	// 2. 发送给玩�??
	if (pPlayer)
	{
		pPlayer->SendPacket(noti);
		return;
	}
	NotifyRoom(noti, eALLUSER, NULL, false);
}

bool CGameTable::checkPlayersItem()
{
	bool check = true;

	if (g_nIsStarSky)//星空�?
	{
		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* pPlayer = GetPlayer(i);
			if (!pPlayer)
			{
				continue;
			}

			IPlayer* iPlayer = pPlayer->GetCorePlayer();
			if (!iPlayer)
			{
				continue;
			}

			const char* guid = iPlayer->GetPlyStrAttr(PLY_BASE_ATTR, PLY_ATTR_GUID);
			if (guid[0] != '9' && iPlayer->GetPlyAttr(ITEM_ATTR, ITEM_STAMINA) <= 0)//体力�? > 0
			{
				check = false;
				break;
			}

			//参赛�? > 4
			if (pPlayer->GetCorePlayer()->GetPlyAttr(ITEM_ATTR, ITEM_JOIN_MATCH_TICKET) < 4)
			{
				check = false;
				break;
			}
		}

		if (check)
		{
			for (int i = 0; i < GetPlayNum(); i++)
			{
				CPlayer* pPlayer = GetPlayer(i);
				if (!pPlayer)
				{
					continue;
				}

				IPlayer* iPlayer = pPlayer->GetCorePlayer();
				if (!iPlayer)
				{
					continue;
				}

				const char* guid = iPlayer->GetPlyStrAttr(PLY_BASE_ATTR, PLY_ATTR_GUID);
				if (guid[0] != '9')
				{
					IPlayer::PlyAttrItem item_(UP_RN_USE_ITEM, ITEM_STAMINA, -1, 0, m_strGameLabel);
					iPlayer->UpdatePlyAttrs(item_);

					pPlayer->UpdateStarItem();
				}
			}
		}
	}

	if (!check)
	{
		OnEndGame();
	}

	return check;
}

string CGameTable::GetUniqLabel()
{
	return m_strGameLabel;
}

// void CGameTable::SaveSpecificCardToLocal()
// {
// 	SERVER_LOG("CGameTable::SaveSpecificCardToLocal()");
// 	CppSQLite3DB* db = CConfigManager::GetInstancePtr()->db_sqlite();
// 	if(db)
// 	{
// 		if(!db->tableExists("tb_normal_card"))
// 		{	
// 			db->execDML(CREATE_NORMAL_CARD_TABLE);
// 		}

// 		// 		int n = db->execDML("insert into card_lordcard values('xiaohei',21,'2001-5-4');");

// 		CppSQLite3Table query = db->getTable("select * from tb_normal_card;");
// 		for(size_t j=0;j<query.numRows();j++)
// 		{
// 			for(size_t i=0;i<query.numFields();i++)
// 			{
// 				query.setRow(j);
// 			}
// 		}

// 		char str[] = "1,2,3,4,5"; 
// 		char delims[] = ",";
// 		char* result = NULL;
// 		result = strtok(str,delims);
// 		while(result != NULL)
// 		{
// 		 	result = strtok(NULL,delims);
// 		}
// 	}
// 	else
// 		g_pLogger->Log("cgametable savespecificcardtolocal---------------------->db is not open when saving data to loca time :%d",leaf::GetDay());
// }

// void CGameTable::SaveCardToLocal(int n_double)
// {
// 	SERVER_LOG("CGameTable::SaveCardToLocal()");
// 	if(g_nSaveNum == 0)
// 		return;
// 	char c[256];
// 	CppSQLite3DB* db = CConfigManager::GetInstancePtr()->db_sqlite();
// 	g_pLogger->Log("------------------------------------------------------------savecardtoLocal start");
// 	if(db)
// 	{
// 		string player_[3];
// 		string lord_card;
// 		for(size_t i=0;i<ePLY_NUM;i++)
// 		{
// 			CPlayer* player = GetPlayer(i);
// 			if(player)
// 			{
// 				if(player->m_vecHandCard.size() == 20)
// 				{
// 					for(size_t i=0;i<3;i++)
// 						player->m_vecHandCard.pop_back();
// 				}
// 				for(size_t j=0;j<player->m_vecHandCard.size();j++)
// 				{
// 					if(j<player->m_vecHandCard.size()-1)
// 					{
// 						sprintf(c,"%d,",player->m_vecHandCard[j].m_nValue+player->m_vecHandCard[j].m_nColor*16);
// 						player_[i] += c;
// 					}
// 					else
// 					{
// 						sprintf(c,"%d,",player->m_vecHandCard[j].m_nValue+player->m_vecHandCard[j].m_nColor*16);
// 						player_[i] += c;
// 					}
// 				}
// 			}
// 		}
// 		for(size_t j=0;j<m_Poke.m_cLordCard.size();j++)
// 		{

// 				if(j<m_Poke.m_cLordCard.size()-1)
// 				{
// 					sprintf(c,"%d,",m_Poke.m_cLordCard[j].m_nValue+m_Poke.m_cLordCard[j].m_nColor*16);
// 					lord_card += c;
// 				}
// 				else
// 				{
// 					sprintf(c,"%d,",m_Poke.m_cLordCard[j].m_nValue+m_Poke.m_cLordCard[j].m_nColor*16);
// 					lord_card += c;
// 				}
// 		}
// 		boost::asio::detail::mutex::scoped_lock lock(CConfigManager::GetInstancePtr()->db_mutex());
// 		string str_table =  CConfigManager::GetInstancePtr()->get_current_table_insert(n_double);
// 		string insert_sql = (string)CConfigManager::GetInstancePtr()->make_message(str_table.c_str(),player_[0].c_str(),player_[1].c_str(),player_[2].c_str(),lord_card.c_str(),n_double);
// 		int n_result = db->execDML(insert_sql.c_str());
//  		g_pLogger->Log("------------------------------------------------------------savecardtoLocal end");
// 	}
// 	else{
// 		g_pLogger->Log("cgametable savecardtolocal---------------------->db is not open when saving data to local %d ",leaf::GetDay());
// 	}
// }

void CGameTable::refreshPlayerTokenMoney()
{
	for (int i = 0; i < GetPlayNum(); ++i)
	{
		CPlayer* pPlayer = GetPlayer(i);
		if (pPlayer)
		{
			int user_gold_leaf = pPlayer->GetCorePlayer()->GetPlyAttr(ITEM_ATTR, ITEM_GOLD_LEAF);
			int user_gold_ticket = pPlayer->GetCorePlayer()->GetPlyAttr(ITEM_ATTR, ITEM_GOLD_TICKET);
			int user_chair = pPlayer->GetChairID();

			player_itemInfo item1;
			if (g_nHBMode < HONGBAO_H5_CHUJI)
			{
				item1.nItemIndex = ITEM_GOLD_LEAF;
				item1.nItemNum = user_gold_leaf;
				item1.nItemNum64 = user_gold_leaf;
			}else{
				item1.nItemIndex = 0;
				item1.nItemNum = pPlayer->GetGameMoney();
				item1.nItemNum64 = pPlayer->GetGameMoney();
			}

			player_itemInfo item2;
			item2.nItemIndex = ITEM_GOLD_TICKET;
			item2.nItemNum = user_gold_ticket;
			item2.nItemNum64 = user_gold_ticket;

			pt_gc_update_player_tokenmoney_not noti;
			noti.ply_chairid_ = user_chair;
			noti.itemInfo.push_back(item1);
			noti.itemInfo.push_back(item2);

			if (g_nIsBaiYuan == 1)
			{
				player_itemInfo item3;
				item3.nItemIndex = ITEM_BY_CASH;
				item3.nItemNum = pPlayer->getItemNum(ITEM_BY_CASH);
				item3.nItemNum64 = item3.nItemNum;
				noti.itemInfo.push_back(item3);
			}

			glog.log("--- refreshPlayerTokenMoney user_chair:%d ply_guid:%lld user_gold_ticket:%d user_gold_leaf:%d", user_chair, pPlayer->GetCorePlayer()->GetPlyInt64Attr(0, PLY_ATTR_GUID), user_gold_ticket, user_gold_leaf);
			for (int j = 0; j < GetPlayNum(); ++j)
			{
				//if (i == j)
				//	continue;
				CPlayer* pPlayer = GetPlayer(j);
				if (pPlayer)
				{
					pPlayer->SendPacket(noti);
				}

			}
		}
		else
		{
			glog.log("==== refreshPlayerTokenMoney chair:%d offline", i);
		}
	}
}

bool CGameTable::isTableStatusIn(int s)
{
	return m_currentTime_ == s;
}

void CGameTable::SendShowCardNot(CPlayer* fromPlayer, CPlayer* toPlayer /*= NULL*/)
{
	if (!fromPlayer)
	{
		return;
	}

	pt_gc_show_card_not noti;
	noti.opcode = gc_show_card_not;
	noti.nChairID = fromPlayer->GetChairID();
	noti.vecCards = fromPlayer->m_PlayCard.m_cCards;
	if (toPlayer) {
		toPlayer->SendPacket(noti);
	}
	else {
		NotifyRoom(noti);
	}

}

void CGameTable::SendDoubleDetail(CPlayer* toPlayer /*= NULL*/)
{
	pt_gc_beishu_info_ack ack;
	ack.opcode = gc_beishu_info_ack;
	ack.vecBeiShuInfo = m_vecDoubleDetail;
	ack.vecPlayerBeiShu = m_vecPlayerDouble;

	if (toPlayer) {
		toPlayer->SendPacket(ack);
	}
	else {
		NotifyRoom(ack);
	}
}

int CGameTable::setMutiDoubleDetail(int type, int bet)
{
	if (type < 0 || 
		type >= m_vecDoubleDetail.size() ||
		bet < 1 ||
		bet > 128)
	{
		SERVER_LOG("CGameTable::setDoubleDetail(int type, int bet)%d, %d Error", type, bet);
		return 1;
	}
	
	if (type == BEI_SHU_INFO_CALLSCORE)	{
		m_vecDoubleDetail[type] = bet;
	} else {
		m_vecDoubleDetail[type] *= bet;
	}
	
	if (m_vecDoubleDetail[type] > 1)
	{
		SendDoubleDetail();
	}
	return m_vecDoubleDetail[type];
}

int CGameTable::getDoubleDetail(int type /*= -1*/)
{
	if (type == -1)	{
		int res = 1;
		for (int i = 0; i < BEI_SHU_INFO_END; ++i) {
			if (m_vecDoubleDetail[i] > 0)
			{
				res *= m_vecDoubleDetail[i];
			}
		}
		return res;
	}
	else if (type >= 0 && type < m_vecDoubleDetail.size()) {
		return m_vecDoubleDetail[type];
	}

	SERVER_LOG(" CGameTable::getDoubleDetail(int type /*= -1*/)%d Error", type);
	return 1;
}

int CGameTable::setPlayerDouble(int chairid, int bet)
{
	if (chairid < 0 || chairid >= m_vecPlayerDouble.size())
	{
		SERVER_LOG("CGameTable::setPlayerDouble(int chairid, int bet)%d, %d Error", chairid, bet);
		return 1;
	}
	m_vecPlayerDouble[chairid] = bet;
	if (m_vecPlayerDouble[chairid] > 1)
	{
		SendDoubleDetail();
	}
	return m_vecPlayerDouble[chairid];
}

void CGameTable::SendLookLordCard(CPlayer* toPlayer)
{
	pt_gc_lord_card_not noti;
	noti.opcode = gc_lord_card_not;
	noti.cLord = -1; //m_Poke.m_nCurrentLord;
	noti.vecCards = m_Poke.m_cLordCard;

	if (toPlayer && toPlayer->m_bLookCard) {
		if (toPlayer->GetCorePlayer()->GetPlyAttr(ITEM_ATTR, ITEM_DDZ_LOOK_CARD) > 0)
		{
			IPlayer::PlyAttrItem item_(UP_RN_USE_ITEM, ITEM_DDZ_LOOK_CARD, -1);
			toPlayer->GetCorePlayer()->UpdatePlyAttrs(item_);

			toPlayer->notiItemInfo(ITEM_DDZ_LOOK_CARD);

			toPlayer->SendPacket(noti);
		}
	}else {
		for (int i = 0; i < GetPlayNum(); i++) {
			CPlayer* pPlayer = GetPlayer(i);
			if (pPlayer && pPlayer->m_bLookCard) {
				if (pPlayer->GetCorePlayer()->GetPlyAttr(ITEM_ATTR, ITEM_DDZ_LOOK_CARD) > 0)
				{
					IPlayer::PlyAttrItem item_(UP_RN_USE_ITEM, ITEM_DDZ_LOOK_CARD, -1);
					pPlayer->GetCorePlayer()->UpdatePlyAttrs(item_);

					pPlayer->notiItemInfo(ITEM_DDZ_LOOK_CARD);

					pPlayer->SendPacket(noti);
				}
			}
		}
	}

}

void CGameTable::AfterDealCard()
{
	if (g_nRPGMode == 1)
	{
		SendLookLordCard();
	}
}

void CGameTable::calcLordChairID()
{
	int totalRate = 0;
	for (int i = 0; i < GetPlayNum(); i++) {
		CPlayer* pPlayer = GetPlayer(i);
		if (pPlayer)
		{
			totalRate += pPlayer->getCallLordRate();
		}
	}
	
	if (totalRate < 1)
	{
		return;
	}

	int value = rand() % totalRate;
	int chairid = -1;

	for (int i = 0; i < GetPlayNum(); i++) {
		CPlayer* pPlayer = GetPlayer(i);
		if (pPlayer)
		{
			value -= pPlayer->getCallLordRate();
			if (value < 0)
			{
				chairid = i;
				break;
			}
		}
	}

	m_Poke.SetDefaultLord(chairid);
}

//剩牌加�?
void CGameTable::LeftCardDouble(CPlayer* pPlayer)
{
	if (g_nLetCard == 0 || g_nTwoPai == 1)
	{
		return;
	}
	int nMinLeftCardNum = 0;
	if (pPlayer->GetChairID() == m_Poke.m_nCurrentLord)	//地主胜利
	{
		for (int i = 0; i < GetPlayNum(); i++)
		{
			if (i != m_Poke.m_nCurrentLord)
			{
				if (nMinLeftCardNum == 0)
				{
					nMinLeftCardNum = (int)GetPlayer(i)->m_PlayCard.m_cCards.size();
				}
				else
				{
					nMinLeftCardNum = min(nMinLeftCardNum, (int)GetPlayer(i)->m_PlayCard.m_cCards.size());
				}
				
			}
		}
	}
	else
	{
		nMinLeftCardNum = GetPlayer(m_Poke.m_nCurrentLord)->m_PlayCard.m_cCards.size();
	}

	if (nMinLeftCardNum > 3)
	{
		int nDouble = 2;
		if (nMinLeftCardNum >= 7)
		{
			nDouble = 3;
		}
		m_nDouble *= nDouble;
		setMutiDoubleDetail(BEI_SHU_INFO_LEFTCARD, nDouble);
	}
}

bool CGameTable::CheckBetterSeatForCleverRobot(int& nBetterSeat, int& nRobotSeat)
{
	if (g_nNewShuffleCards == 1)
	{
		int player_val[3];
		memset(player_val, 0, sizeof(player_val));
		int robotNumber = -1;

		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* player = GetPlayer(i);
			if (player)
			{
				if (g_nGameType >= 0)
				{
					const char* guid = player->GetCorePlayer()->GetPlyStrAttr(0, PLY_ATTR_GUID);
					if (CConfigManager::GetInstancePtr()->isRobotGuid(guid)) {
						if (g_nGameType > 1)
						{
							player_val[i] = 3 + rand() % 5;
						}
						else{
							player_val[i] = 3 + rand() % 5;
						}
						robotNumber = i;
						continue;
					}
					else {
						player_val[i] = 1 + rand() % 3;
					}
				}

				if (g_nNewMatch == 1)
				{
					continue;
				}

				//SERVER_LOG("---------first login: %d.", player->GetCorePlayer()->IsFirstLogin());
				//if(player->GetCorePlayer()->IsFirstLogin()==1)   //1�?一次进入房�?  0不是�?一次登�?
				//{
				//	player_val[i] += 3; //每天游戏的�??1局
				//}	

				//每天游戏局�?
				int daily_count = player->GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_DAILY_COUNT);
				if (daily_count <= 60)
				{
					map<int, int>::iterator itr = m_mapDailyCountVal.find(daily_count);
					if (itr != m_mapDailyCountVal.end() && itr->second > 0)
					{
						player_val[i] += itr->second;
					}

					if (daily_count <= 5)
					{
						int nNumRound = player->GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_ROUND);
						if (nNumRound > 0 && nNumRound <= 3)
						{
							player_val[i] += 4;
						}
						else if (nNumRound > 3 && nNumRound <= 5)
						{
							player_val[i] += 1;
						}
					}
				}

				int reliefRound = player->GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_RELIEF_COUNT);
				if (reliefRound < 2)
				{
					player_val[i] += 2;
				}

				//VIP
				int playerVip = max(0, player->GetCorePlayer()->GetPlyAttr(PLY_BASE_ATTR, PLY_VIP_LEVEL));
				playerVip = min(playerVip, 10);

				int vipPro = 30;
				if (playerVip > 0)
				{
					vipPro += playerVip * 5;
				}
				else {
					vipPro = 0;
				}

				if (vipPro > 0 && (rand() % 100 < vipPro))
				{
					player_val[i] += 1;
				}

				//充值用�?
				if (player->GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_CHARGE) > 0 && (rand() % 100 < 70))
				{
					// player_val[i] += 1;
				}

				//高分玩�?�进入发�?�?
				if (g_nLimitMoneyNext > 0)
				{
					if (player->GetGameMoney() > g_nLimitMoneyNext)
					{
						if (rand() % 100 < 80)
						{
							player_val[i] += 1;
						}
						else
						{
							player_val[i] -= 1;
						}
					}
				}

				//连输玩�??
				int lost_count = player->GetCorePlayer()->GetPlyAttr(0, PLY_CONTINUE_LOST);
				if (lost_count >= 1)
				{
					player_val[i] += min(5, lost_count);
				}
			}
		}
		SERVER_LOG("CheckBetterSeat0:%d,1:%d,2:%d", player_val[0], player_val[1], player_val[2]);
		//取最大�?
		int maxNumber = 0;
		if (player_val[1] > player_val[0])  {
			maxNumber = 1;
		}
		if (player_val[2] > player_val[maxNumber])
		{
			maxNumber = 2;
		}
		if ((player_val[1] == player_val[0]) && (player_val[1] == player_val[2]))
		{
			maxNumber = robotNumber;
		}
		nBetterSeat = maxNumber;
		nRobotSeat = robotNumber;
		
		
		if (g_nLoggerHandCard == 1)
		{
			const int buffer_size = 512;
			char buffer[buffer_size];
			int length = 0;
			for (int i = 0; i < GetPlayNum(); i++) {

				CPlayer* pPlayer = GetPlayer(i);
				if (pPlayer) {
					const char* guid = pPlayer->GetCorePlayer()->GetPlyStrAttr(0, PLY_ATTR_GUID);
					length += snprintf(buffer + length, buffer_size - length, "[uid: %16s, r:%2d:], ", guid, player_val[i]);
				}
			}
			g_pLogger->Log("CheckBetterSeat: better:%d, %s", nBetterSeat, buffer);
					
		}

		return true;
	}
	else
	{
		bool is_new_player = false;		//�?否有新玩�?
		bool is_specific = false;		//�?否特殊牌�?
		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* player = GetPlayer(i);
			if (player)
			{
				SERVER_LOG("---------first login: %d.", player->GetCorePlayer()->IsFirstLogin());
				if (player->GetCorePlayer()->IsFirstLogin() == 1)   //1�?一次进入房�?  0不是�?一次登�?
				{
					is_specific = true;
					is_new_player = true;
					nRobotSeat = 192;
				}
			}
		}

		int64 n_player_limit_money = 0;
		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* player = GetPlayer(i);
			if (player)
			{
				if (n_player_limit_money == 0)
				{
					n_player_limit_money = player->GetGameMoney();
				}
				if (n_player_limit_money > player->GetGameMoney())
				{
					n_player_limit_money = player->GetGameMoney();
				}
			}
		}

		if (n_player_limit_money > g_nLimitMoney * 10)	//一位玩家游戏币�?入场限制�?10�?
		{
			nRobotSeat = 96;
			if (!is_new_player)
			{
				if (n_player_limit_money > g_nLimitMoney * 15)
				{
					nRobotSeat = 192;
				}
				if (n_player_limit_money > g_nLimitMoney * 20)
				{
					nRobotSeat = 384;
				}
				if (n_player_limit_money > g_nLimitMoney * 25)
				{
					nRobotSeat = 768;
				}
				if (n_player_limit_money > g_nLimitMoney * 30)
				{
					nRobotSeat = 1536;
				}
			}
			is_specific = true;
		}

		if (is_specific)
		{
			if (g_nBaoPai == 1)
			{
				nBetterSeat = SPECIFIC_LAIZI_TYPE;
			}
			else
			{
				nBetterSeat = SPECIFIC_TYPE;
			}
		}
		else
		{
			if (g_nBaoPai == 1)
			{
				nBetterSeat = COMMON_LAIZI_TYPE;
			}
			else
			{
				nBetterSeat = COMMON_TYPE;
			}
		}

		return is_specific;
	}
}

void CGameTable::CalculateScore(CPlayer* pPlayer, int64 nScore[], int nRaceScore[], int64 nMoney[])
{
	if (g_nIsBaiYuan == 1)
	{
		CalculateScore_baiyuan(pPlayer, nScore, nRaceScore, nMoney);
		return;
	}

	CalculateScore_justice(pPlayer, nScore, nRaceScore, nMoney);
}

void CGameTable::CalculateScore_justice(CPlayer* pPlayer, int64 nScore[], int nRaceScore[], int64 nMoney[])
{
	int nFarmerNum = GetPlayNum() - 1;

	int farmerDouble = 0;

	for (int i = 0; i < GetPlayNum(); i++)
	{
		CPlayer* pFarmer = GetPlayer(i);
		if (pFarmer && i != m_Poke.m_nCurrentLord)
		{
			farmerDouble += pFarmer->getMyDouble();
		}
	}

	if (pPlayer->GetChairID() == m_Poke.m_nCurrentLord)	//地主胜利
	{
		int64 nAllLoseMoney = 0;

		for (int i = 0; i < GetPlayNum(); i++) {
			CPlayer* pLose = GetPlayer(i);
			if (pLose && i != pPlayer->GetChairID()) {
				int nActualDouble = m_nDouble;

				if (g_nRPGMode == 1) {
					nActualDouble = getDoubleDetail() * pPlayer->getMyDouble() * pLose->getMyDouble();
				}

				// 最低限�?
				if (g_nMinDouble > 0) {
					nActualDouble = max(nActualDouble, g_nMinDouble);
				}

				// 最高限�?
				if (m_nMaxDouble > 0) {
					nActualDouble = min(nActualDouble, m_nMaxDouble);
				}

				nAllLoseMoney += m_nBaseScore * nActualDouble;
			}
		}

		if (nAllLoseMoney < 0 || m_nDouble > 100000000) //
		{
			nAllLoseMoney = 100000000; //1E
		}

		if (g_nMaxMoney > 0 && m_nSettleType != 1 && nAllLoseMoney > g_nMaxMoney)
		{
			nAllLoseMoney = g_nMaxMoney;
		}

		nAllLoseMoney = min(nAllLoseMoney, nMoney[m_Poke.m_nCurrentLord]);


		int64 nWinScore = 0;
		int64 nDLoseMoney = nAllLoseMoney / nFarmerNum;
		int nStandardRankerWin = 0;


		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* pLose = GetPlayer(i);
			if (pLose && i != pPlayer->GetChairID())
			{
				int nActualDouble = m_nDouble;

				if (g_nRPGMode == 1) {
					if (farmerDouble > 0 && pLose->getMyDouble() * nFarmerNum != farmerDouble)
					{
						nDLoseMoney = nAllLoseMoney * pLose->getMyDouble() / farmerDouble;
					}
					nActualDouble = getDoubleDetail() * pPlayer->getMyDouble() * pLose->getMyDouble();
				}
				else if (this->is_standard_game() && pLose->m_nStandardDoubleScore > 1) {
					nActualDouble = m_nDouble * pLose->m_nStandardDoubleScore * pPlayer->m_nStandardDoubleScore;
				}

				// 最低限�?
				if (g_nMinDouble > 0)
				{
					nActualDouble = max(nActualDouble, g_nMinDouble);
				}

				// 最高限�?
				if (m_nMaxDouble > 0)
				{
					nActualDouble = min(nActualDouble, m_nMaxDouble);
				}

				int64 nLoseMoney = m_nBaseScore * nActualDouble;
				if (nLoseMoney < 0 || m_nDouble > 100000000) //
				{
					nLoseMoney = 100000000; //1E
				}
				if (g_nMaxMoney > 0 && m_nSettleType != 1 && nLoseMoney > g_nMaxMoney)
				{
					nLoseMoney = g_nMaxMoney;
				}
				if (g_nMatch == 0 && m_nSettleType != 1)
				{
					if (nMoney[i] < nLoseMoney)
					{
						nLoseMoney = nMoney[i];
					}
				}
				else if (g_nMatch == 1)
				{
					if (g_nLessThanZero == 0)
					{
						if (nMoney[i] < nLoseMoney)
						{
							nLoseMoney = nMoney[i];
						}
					}
				}

				if (nLoseMoney > nDLoseMoney && m_nSettleType != 1)
				{
					nLoseMoney = nDLoseMoney;
				}
				nScore[i] = -nLoseMoney;
				nWinScore += nLoseMoney;

				if (g_nRace == 1)
				{
					nRaceScore[i] = -nActualDouble;
					nStandardRankerWin = this->is_standard_game() ? nStandardRankerWin + nActualDouble : 1;
				}
				if (g_nMatch == 1) {
					nRaceScore[i] = -nActualDouble * m_nBaseScore;
					nStandardRankerWin = this->is_standard_game() ? nStandardRankerWin + nActualDouble : 1;
				}
			}
		}

		if (g_nRace == 1) {
			nRaceScore[m_Poke.m_nCurrentLord] = this->is_standard_game() ? nStandardRankerWin : m_nDouble * nFarmerNum;
		}
		if (g_nMatch == 1) {
			nRaceScore[m_Poke.m_nCurrentLord] = this->is_standard_game() ? nStandardRankerWin * m_nBaseScore : m_nDouble * nFarmerNum * m_nBaseScore;
		}
		if (g_nTax == 1)
		{
			nScore[m_Poke.m_nCurrentLord] = nWinScore * 0.9;
		}
		else {
			nScore[m_Poke.m_nCurrentLord] = nWinScore;
		}

	}
	else	//农民胜利
	{
		int64 nWinScore = 0;
		int64 nDLoseMoney = 0;
		int nActualDouble = m_nDouble;
		int64 nActualLose = 0;
		int64 nLoseMoney = 0;

		CPlayer* pLord = GetPlayer(m_Poke.m_nCurrentLord);
		if (pLord)
		{
			if (this->is_standard_game()) {
				for (int i = 0; i < GetPlayNum(); i++) {
					CPlayer* pWin = GetPlayer(i);
					if (pWin && i != m_Poke.m_nCurrentLord)
					{
						if (pWin->m_nStandardDoubleScore > 1)
						{
							nActualDouble = nActualDouble + m_nDouble * pWin->m_nStandardDoubleScore * pLord->m_nStandardDoubleScore;
						}
						else
						{
							nActualDouble = nActualDouble + m_nDouble;
						}
					}
				}
			}
			else if (g_nRPGMode == 1) {
				nActualDouble = getDoubleDetail() * farmerDouble * pLord->getMyDouble();
			}

			if (g_nMinDouble > 0) {
				nActualDouble = max(nActualDouble, g_nMinDouble);
			}

			if (m_nMaxDouble > 0) {
				nActualDouble = min(nActualDouble, m_nMaxDouble);
			}



			if (g_nRPGMode == 1) {
				nLoseMoney = m_nBaseScore * nActualDouble;
			}
			else if (this->is_standard_game()) {
				nLoseMoney = m_nBaseScore * nActualDouble * nFarmerNum;
			}
			else {
				nLoseMoney = m_nBaseScore * m_nDouble * nFarmerNum;
			}

			if (nLoseMoney < 0 || m_nDouble > 100000000) //
			{
				nLoseMoney = 100000000;
			}
			if (g_nMaxMoney > 0 && m_nSettleType != 1 && nLoseMoney > g_nMaxMoney)
			{
				nLoseMoney = g_nMaxMoney;
			}

			if (g_nMatch == 0 && m_nSettleType != 1)
			{
				if (nMoney[m_Poke.m_nCurrentLord] < nLoseMoney)
				{
					nLoseMoney = nMoney[m_Poke.m_nCurrentLord];
				}

			}
			else if (g_nMatch == 1)
			{
				if (g_nLessThanZero == 0)
				{
					if (nMoney[m_Poke.m_nCurrentLord] < nLoseMoney)
					{
						nLoseMoney = nMoney[m_Poke.m_nCurrentLord];
					}
				}
			}

			nScore[m_Poke.m_nCurrentLord] = -nLoseMoney;
			//nWinScore = nLoseMoney >> 1;
			nWinScore = nLoseMoney / nFarmerNum;

			nActualLose = abs(nLoseMoney);

			if (g_nRace == 1) {
				nRaceScore[m_Poke.m_nCurrentLord] = this->is_standard_game() ? -m_nDouble * pLord->m_nStandardBigerDoubleScore : -m_nDouble;
			}
			if (g_nMatch == 1) {
				nRaceScore[m_Poke.m_nCurrentLord] = -nLoseMoney;
			}
		}


		for (int i = 0; i < GetPlayNum(); i++)
		{
			CPlayer* pWin = GetPlayer(i);
			if (pWin && i != m_Poke.m_nCurrentLord)
			{
				int nWinnerDouble = m_nDouble;
				int64 nActualWinScore = nWinScore;

				if (g_nRPGMode == 1) {
					nActualWinScore = nLoseMoney * pWin->getMyDouble() / farmerDouble;
				}
				else if (this->is_standard_game())
				{
					if (pWin->m_nStandardDoubleScore > 1)
					{
						nWinnerDouble = m_nDouble * pWin->m_nStandardDoubleScore * pLord->m_nStandardDoubleScore;
					}
					nActualWinScore = floor((nActualLose * nWinnerDouble) / nActualDouble);
				}

				nScore[i] = nActualWinScore;
				if (nMoney[i] < nActualWinScore && m_nSettleType != 1)
				{
					nScore[i] = nMoney[i];
				}

				nDLoseMoney += nScore[i];

				if (g_nTax == 1)
				{
					nScore[i] = nScore[i] * 0.9;
				}/*
				else if(g_nTax == 0){
				nScore[i] = nWinScore ;
				}else {
				nScore[i] = nWinScore;
				}*/

				if (g_nRace == 1)
				{
					nRaceScore[i] = m_nDouble;
					if (i == pPlayer->GetChairID())
						nRaceScore[i] += m_nDouble / 10;
				}
				if (g_nMatch == 1)
				{
					nRaceScore[i] = m_nDouble * m_nBaseScore;
				}
			}
		}
		nScore[m_Poke.m_nCurrentLord] = -nDLoseMoney;
	}
}

void CGameTable::CalculateScore_baiyuan(CPlayer* pPlayer, int64 nScore[], int nRaceScore[], int64 nMoney[])
{
	bool nFreeLose[3];
	memset(nFreeLose, false, sizeof(nFreeLose));
	for (int i = 0; i < GetPlayNum(); i++)
	{
		CPlayer* player = GetPlayer(i);
		if (player && player->GetCorePlayer())
		{
			// 免输
			if (player->GetCorePlayer()->GetPlyAttr(ITEM_ATTR, ITEM_FREE_LOSE) > 0)
			{
				IPlayer::PlyAttrItem item_(UP_RN_USE_ITEM, ITEM_FREE_LOSE, -1);
				player->GetCorePlayer()->UpdatePlyAttrs(item_);

				nFreeLose[i] = true;
			}
		}
	}

	int nWinMoney = m_nBaseScore * m_nCallScore;
	//地主胜利
	if (pPlayer->GetChairID() == m_Poke.m_nCurrentLord)
	{
		for (int i = 0; i < GetPlayNum(); i++)
		{
			if (i == m_Poke.m_nCurrentLord)
			{
				continue;
			}
			
			int money = min(nWinMoney, (int)nMoney[i]);

			nMoney[m_Poke.m_nCurrentLord] += money;
			nScore[m_Poke.m_nCurrentLord] += money;

			// 免输
			if (nFreeLose[i])
			{
				continue;
			}

			nMoney[i] -= money;
			nScore[i] -= money;
		}
	}
	//农民胜利
	else if (nMoney[m_Poke.m_nCurrentLord] > 0)
	{
		int money = nWinMoney;
		int nFarmerNum = GetPlayNum() - 1;
		if ((nWinMoney * nFarmerNum) > nMoney[m_Poke.m_nCurrentLord])
		{
			money = nMoney[m_Poke.m_nCurrentLord] / nFarmerNum;
		}

		for (int i = 0; i < GetPlayNum(); i++)
		{
			if (i == m_Poke.m_nCurrentLord)
			{
				continue;
			}

			nMoney[i] += money;
			nScore[i] += money;

			// 免输
			if (nFreeLose[m_Poke.m_nCurrentLord])
			{
				continue;
			}

			nMoney[m_Poke.m_nCurrentLord] -= money;
			nScore[m_Poke.m_nCurrentLord] -= money;
		}
	}
}

void CGameTable::BombBaiYuan(CPlayer* pPlayer)
{
	pt_gc_baiyuan_tocash_item_not noti;
	noti.opcode = gc_baiyuan_tocash_item_not;
	noti.cType = 1;

	TocashItemInfo infoSelf;
	infoSelf.cChairID = pPlayer->GetChairID();
	infoSelf.nItemChange = 0;

	//int nWinMoney = m_nBaseScore * m_nCallScore * g_dBaiYuanBombDouble;
	int nWinMoney = 100;
	// 地主炸弹
	if (pPlayer->GetChairID() == m_Poke.m_nCurrentLord)
	{
		for (int i = 0; i < GetPlayNum(); i++)
		{
			if (i == m_Poke.m_nCurrentLord)
			{
				continue;
			}

			CPlayer* player = GetPlayer(i);
			if (player && player->GetCorePlayer())
			{
				int money = min(nWinMoney, player->getItemNum(ITEM_BY_CASH));

				TocashItemInfo info;
				info.cChairID = player->GetChairID();
				info.nItemChange = -money;
				noti.vecItemInfo.push_back(info);

				infoSelf.nItemChange += money;
			}
		}
	}
	// 农民炸弹
	else
	{
		CPlayer* player = GetPlayer(m_Poke.m_nCurrentLord);
		if (player && player->GetCorePlayer())
		{
			int money = min(nWinMoney, player->getItemNum(ITEM_BY_CASH));

			TocashItemInfo info;
			info.cChairID = player->GetChairID();
			info.nItemChange = -money;
			noti.vecItemInfo.push_back(info);

			infoSelf.nItemChange += money;
		}
	}
	noti.vecItemInfo.push_back(infoSelf);

	for (size_t i = 0; i < noti.vecItemInfo.size(); i++)
	{
		TocashItemInfo& itemInfo = noti.vecItemInfo[i];
		if (itemInfo.nItemChange == 0)
		{
			continue;
		}

		CPlayer* player = GetPlayer(itemInfo.cChairID);
		if (player && player->GetCorePlayer())
		{
			// 加钱
			char buffer[512];
			sprintf(buffer, "baiyuan_bomb item:%d base:%d call:%d double:%.2f", player->getItemNum(ITEM_BY_CASH), m_nBaseScore, m_nCallScore, g_dBaiYuanBombDouble);
			IPlayer::PlyAttrItem item_(UP_RN_SPECIFY_ITEM, ITEM_BY_CASH, itemInfo.nItemChange, 0, buffer);
			player->GetCorePlayer()->UpdatePlyAttrs(item_);
		}
	}

	NotifyRoom(noti, eALLUSER, NULL, false);
}


void CGameTable::ShuffleCards()
{
	m_nShuffleLogType = 0;
	m_nShuffleLogBetterSeat = -1;
	m_nShuffleLogRobotSeat = -1;
	// 初�?�化参数
	m_Poke.NewRound();

	// 使用配牌�?
	if (m_Poke.ShuffleCardsFile())
	{
		m_nShuffleLogType = 1;
		return;
	}

	// �?随机洗牌
	if (g_nNewShuffleCards != 1 && g_nSendCardCommon == 1)
	{
		m_Poke.ShuffleDefaultCards();
		m_nShuffleLogType = 2;
		return;
	}

	int nBetterSeat = 0; // 优势�?
	int nRobotSeat = 0;	 // 机器人座�?
	ShuffleCardsInfo info;
	for (int i = 0; i < GetPlayNum(); i++)
	{
		CPlayer* player = GetPlayer(i);
		if (player)
		{
			const char* guid = player->GetCorePlayer()->GetPlyStrAttr(0, PLY_ATTR_GUID);
			if (CConfigManager::GetInstancePtr()->isRobotGuid(guid))
			{
				info.vecRobotSeat.push_back(i);
			}
			else
			{
				info.vecPlayerSeat.push_back(i);
			}
		}
	}

	// �?1�?玩�?��?�都�?机器�?
	// 特殊场�?? �?和机器人�?
	if (info.vecPlayerSeat.size() == 1 && info.vecRobotSeat.size() == (GetPlayNum() - 1))
	{
		// 新手顺序牌组
		if (g_nShuffleCardsForNewBieFileOrder == 1)
		{
			int nOrderIndex = 0;
			if (CheckShuffleCardsForNewBieFileOrder(info, nBetterSeat, nOrderIndex))
			{
				if (m_Poke.ShuffleCardsForNewBieFileOrder(nBetterSeat, nOrderIndex))
				{
					m_nShuffleLogType = 3;
					m_nShuffleLogBetterSeat = nBetterSeat;
					m_nShuffleLogRobotSeat = nOrderIndex;
					return;
				}
			}
		}

		// 百元�? 根据玩�?�道具数量调整优势位
		if (g_nIsBaiYuan == 1)
		{
			if (CheckShuffleCardsForBaiYuanMustLose(info, nBetterSeat, nRobotSeat))
			{
				bool bUseMaxConfig = false;
				if (g_nBaiYuanMustLostMaxMoney > 0)
				{
					CPlayer* pPlayer = GetPlayer(info.vecPlayerSeat.front());
					if (pPlayer && pPlayer->getItemNum(ITEM_BY_CASH) >= g_nBaiYuanMustLostMaxMoney)
					{
						bUseMaxConfig = true;
					}
				}
				if (m_Poke.ShuffleCardsForBaiYuanMustLose(nBetterSeat, nRobotSeat, bUseMaxConfig))
				{
					m_nShuffleLogType = bUseMaxConfig ? 10: 5;
					m_nShuffleLogBetterSeat = nBetterSeat;
					m_nShuffleLogRobotSeat = nRobotSeat;
					return;
				}
			}

			if (CheckShuffleCardsForBaiYuanMatchCard(info, nBetterSeat, nRobotSeat))
			{
				m_Poke.ShuffleCardsForBaiYuanMatchCard(nBetterSeat, nRobotSeat);
				m_nShuffleLogType = 4;
				m_nShuffleLogBetterSeat = nBetterSeat;
				m_nShuffleLogRobotSeat = nRobotSeat;
				return;
			}
		}

		// 新手随机牌组
		if (g_nShuffleCardsForNewBieFileABTest > 0)
		{
			vector<int> vecCardsId;
			if (CheckShuffleCardsForNewBieFile(info, nBetterSeat, vecCardsId))
			{
				if (m_Poke.ShuffleCardsForNewBieFile(nBetterSeat, vecCardsId))
				{
					m_nShuffleLogType = 6;
					m_nShuffleLogBetterSeat = nBetterSeat;
					m_nShuffleLogRobotSeat = nRobotSeat;
					return;
				}
			}
		}

		// 新手随机洗牌
		if (g_nBetterSeatForNewBie == 1)
		{
			if (CheckShuffleCardsForNewBie(info, nBetterSeat))
			{
				m_Poke.ShuffleCardsForNewBie(nBetterSeat);
				m_nShuffleLogType = 7;
				m_nShuffleLogBetterSeat = nBetterSeat;
				m_nShuffleLogRobotSeat = nRobotSeat;
				return;
			}
		}
	}

	// �?明机器人
	if (g_nCleverRobot == 1)
	{
		CheckBetterSeatForCleverRobot(nBetterSeat, nRobotSeat);
		m_Poke.ShuffleCards(nBetterSeat, nRobotSeat);
		m_nShuffleLogType = 8;
		m_nShuffleLogBetterSeat = nBetterSeat;
		m_nShuffleLogRobotSeat = nRobotSeat;
		return;
	}

	CheckBetterSeat(nBetterSeat, nRobotSeat); // 检查优势座�?
	m_Poke.ShuffleCards(nBetterSeat, nRobotSeat);
	m_nShuffleLogType = 9;
	m_nShuffleLogBetterSeat = nBetterSeat;
	m_nShuffleLogRobotSeat = nRobotSeat;
}

bool CGameTable::CheckShuffleCardsForNewBie(const ShuffleCardsInfo& info, int& nBetterSeat)
{
	if (g_nBetterSeatForNewBieRound <= 0)
	{
		return false;
	}

	// �?1�?玩�?��?�都�?机器�?
	if (info.vecPlayerSeat.size() == 0 || info.vecRobotSeat.size() != (GetPlayNum() - 1))
	{
		return false;
	}

	int nPlayerSeat = info.vecPlayerSeat.front();
	CPlayer* pPlayer = GetPlayer(nPlayerSeat);
	if (pPlayer == NULL)
	{
		return false;
	}

	// 局数超�?
	int nRoundSum = pPlayer->GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_ROUND);
	if (nRoundSum >= g_nBetterSeatForNewBieRound)
	{
		return false;
	}

	nBetterSeat = nPlayerSeat;
	return true;
}

bool CGameTable::CheckShuffleCardsForNewBieFile(const ShuffleCardsInfo& info, int& nBetterSeat, vector<int>& vecCardsId)
{
	if (g_nShuffleCardsForNewBieFileRound <= 0)
	{
		return false;
	}

	int nPlayerSeat = info.vecPlayerSeat.front();
	CPlayer* pPlayer = GetPlayer(nPlayerSeat);
	if (pPlayer == NULL)
	{
		return false;
	}

	// 局数超�?
	int nRoundSum = pPlayer->GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_ROUND);
	if (nRoundSum >= g_nShuffleCardsForNewBieFileRound)
	{
		return false;
	}

	int64 guid = pPlayer->GetCorePlayer()->GetPlyInt64Attr(0, PLY_ATTR_GUID);
	if (guid % g_nShuffleCardsForNewBieFileABTest != 0)
	{
		return false;
	}

	nBetterSeat = nPlayerSeat;
	vecCardsId = pPlayer->m_vecNewbieCardsId;
	return true;
}

bool CGameTable::CheckShuffleCardsForNewBieFileOrder(const ShuffleCardsInfo& info, int& nBetterSeat, int& nOrderIndex)
{
	if (g_nShuffleCardsForNewBieFileOrderRound <= 0)
	{
		return false;
	}

	int nPlayerSeat = info.vecPlayerSeat.front();
	CPlayer* pPlayer = GetPlayer(nPlayerSeat);
	if (pPlayer == NULL)
	{
		return false;
	}

	// 红包�?�?
	if (g_nShuffleCardsForNewBieFileOrderMoney > 0 && pPlayer->getItemNum(ITEM_BY_CASH) >= g_nShuffleCardsForNewBieFileOrderMoney)
	{
		return false;
	}

	// 局数超�?
	int nRoundSum = pPlayer->GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_ROUND);
	if (nRoundSum > g_nShuffleCardsForNewBieFileOrderRound)
	{
		return false;
	}

	nBetterSeat = nPlayerSeat;
	nOrderIndex = nRoundSum;
	return true;
}


// 玩家输赢控制处理
bool CGameTable::CheckShuffleCardsForBaiYuanMatchCard(const ShuffleCardsInfo& info, int& nBetterSeat, int& nRobotSeat)
{
	int nPlayerSeat = info.vecPlayerSeat.front();
	CPlayer* pPlayer = GetPlayer(nPlayerSeat);
	if (pPlayer == NULL)
	{
		return false;
	}

	if (!CConfigManager::GetInstancePtr()->isToggleCondition(CPlayer::TC_MatchCard, pPlayer->getItemNum(ITEM_BY_CASH)))
	{
		return false;
	}

	nBetterSeat = nPlayerSeat;
	nRobotSeat = info.vecRobotSeat[rand() % info.vecRobotSeat.size()];;
	return true;
}

bool CGameTable::CheckShuffleCardsForBaiYuanMustLose(const ShuffleCardsInfo& info, int& nBetterSeat, int& nRobotSeat)
{
	int nPlayerSeat = info.vecPlayerSeat.front();
	CPlayer* pPlayer = GetPlayer(nPlayerSeat);
	if (pPlayer == NULL)
	{
		return false;
	}

	do
	{
		// 特定局数必杀
		if (g_nBaiYuanMustLostRound > 0 && (pPlayer->GetCorePlayer()->GetPlyAttr(0, PLY_ATTR_ROUND) + 1) == g_nBaiYuanMustLostRound)
		{
			break;
		}

		if (CConfigManager::GetInstancePtr()->isToggleCondition(CPlayer::TC_MustLose, pPlayer->getItemNum(ITEM_BY_CASH)))
		{
			break;
		}

		return false;
	} while (false);

	nRobotSeat = info.vecRobotSeat[rand() % info.vecRobotSeat.size()];
	nBetterSeat = nRobotSeat;
	m_nMustCallSeat = nRobotSeat;
	return true;
}
