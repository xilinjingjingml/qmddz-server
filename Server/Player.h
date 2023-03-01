//////////////////////////////////////////////////////////////////////////
/// Copyright (c) 2005-2010 ShangHai Banding Co., All Rights Reserved.
///
/// @file Player.h
/// @brief 玩家控制类
///
/// 处理玩家出牌操作，响应玩家操作请求
///
/// @author leiliang
/// @date  2009/04/17
//////////////////////////////////////////////////////////////////////////

#pragma once

class CGameTable;



struct RegainLoseScoreGameData
{
	int64 last_lose_money;		// 输掉的钱
	int last_lose_time;	// 输钱的时间戳
	int last_regain_count;	// 剩余领取次数
	int total_count;		// 每日总次数
};

struct RedPacketsNewbieData
{
	int nIndex;	// 当前档次
	int nRound;	// 当前档次需要局数
	RedPacketsNewbieData() {
		nIndex = -1;
		nRound = -1;
	}
};

/// 本类的功能：玩家控制类
///
/// 处理玩家出牌操作，响应玩家操作请求
class CPlayer : public IPlayerDelegate
{
public:
	/// 玩家游戏状态定义
	///
	/// 该枚举定义了系统中需要用到的玩家游戏状态
	enum PS_GAMESTATE
	{
		PS_NONE,	///< 枚举，初始化状态
		PS_PLAYER,	///< 枚举，游戏玩家状态
		PS_VISTOR,	///< 枚举，旁观玩家状态
	};

	enum SR_STARREASON
	{
		SR_QUERY,	///< 枚举，查询
		SR_UPDATE,	///< 枚举，更新
	};

	enum TC_ToggleCondition
	{
		TC_MatchCard = 1,	// 配牌
		TC_MustLose,		// 强杀
		TC_RegainLose,		// 追回损失
		TC_LuckWelfare,		// 幸运福利
		TC_WinDouble,		// 赢分加倍
	};
public:
	/// 构造函数
	CPlayer();

	/// 析构函数
	~CPlayer();

	/// 开始新的一轮
	///
	/// 初始化各种变量
	void NewRound();

	// 一局结束
	void RoundEnd(int64 nScore);
	/// 给玩家发牌
	///
	/// 洗牌后给各玩家派牌，每派一张牌调用一次该函数
	/// @param[in] cCard 从牌河中发的一张牌
	void GetCard(CCard& cCard);

	/// 获得玩家手上的牌
	///
	/// 获得玩家手上的牌，当bShow为true时获得明牌
	/// @param[out] noti 刷新牌面的数据结构
	/// @param[in] bShow 是否显示明牌
	void GetPlayerCards(pt_gc_refresh_card_not& noti, bool bShow);

	/// 玩家出牌
	///
	/// 判断是否符合出牌规则
	/// @param[in] pPutPlayer 出牌的玩家
	/// @return 出牌结果
	/// @retval true 成功
	/// @retval false 失败
	bool DoPlayCard(CPlayer* pPutPlayer);


	/// 获取玩家身上所带的游戏币
	///
	/// @return 游戏币
	int64 GetGameMoney();

	/// 获取玩家的昵称
	///
	/// @return 昵称
	string GetNickName() { return m_pCorePlayer ? m_pCorePlayer->GetPlyStrAttr(0, PLY_ATTR_NICKNAME) : ""; }

	/// 获取玩家状态
	///
	/// @return 玩家状态值
	int	GetStatus(){ return m_nStatus; }

	/// 设置玩家状态
	///
	/// @param[in] nStatus 状态值
	void SetStatus(int nStatus) { m_nStatus = nStatus; }

	/// 获取玩家座位号
	///
	/// @return 玩家座位号
	int	GetChairID() { return m_pCorePlayer->GetChairID(); }
	//是否允许旁观
	bool CanWatch() { return false; }

	bool UpdateLordCardLotteryReward(bool needNotify = true);

	void UpdateStarScore(SR_STARREASON eSR, int nScore = 0);
	void UpdateStarItem();
	void UseCounts();				// 使用记牌器

	void readRedPacketsData();	//读数据库
	void SendRedPacketsResult(); //发送结果给客户端
	bool checkRedPacketsTask(); //检查是否可以领取红包
	void writeRedPacketsStatusData(); //写数据库
	void writeRedPacketsTotalData();

	void checkRedPacket88YuanData(); //88元红包检查
	void readRedPacket88YuanData(); //88元红包写入
	void saveRedPacket88YuanData();	//88元红包读取
	void sendRedPacket88YuanData();	//88元红包发送到客户端

	void pt_cg_get_redpackets_award_req_handler(const pt_cg_get_redpackets_award_req& req);
	void pt_cg_get_redpackets_88yuan_award_req_handler(const pt_cg_get_redpackets_88yuan_award_req& req);
	//生成红包金额
	int getRedPacketProbabilityIdx();
	int genRedPacketNum(int extBet = 0);
	int genRedPacketNumForH5(int extBet = 0);
	int genRedPacketNumForNewBie();
	bool genExtraRedPacketNum();	// 再抽一次
	void readRedPacketsNewbieData();
	void writeRedPacketsNewbieData();
	void genRedPacketForNewBie();
	void resetRedPacketForNewBie();
	void getRedPacketBroadcast(int nums, int type);
	int getPayValue();
	int getPayValueRate();

	void sendMagicEmojiConfig();

	void	setMutiMyDouble(int m);	// 个人倍数
	bool	superDoubleCheck();
	int		getMyDouble();		// 个人倍数
	bool	isShowCard();		// 是否明牌
	void	setShowCard(int s);
	int		getShowCard();
	void	sendShowCardReq(int type, int bet);
	void	setCallLordRate(int t);
	int		getCallLordRate();

	void	genRegainLoseMoney(int score);
	void	getRegainLoseMoney(int op, pt_cg_regain_lose_score_req* req = NULL);	//获取输掉的金币
	void	readRegainLoseMoney();
	void	saveRegainLoseMoney(int op = 0); //0读取，1领取，2掉线写入

	bool	isRoundStart();
	bool	getInvincibleStatus();

	void	afterBindCorePlayer();
	void	chipFall();
	void	notiItemAdd(int nItemIndex, int nItemCount);
	void	notiItemInfo(ITEM_INDEX nItemIndex);
	void	genWinDouble(); // 胜利翻倍

public:
	//以下是系统接口函数
	IPlayer* GetCorePlayer() { return m_pCorePlayer; }
	void SetGameTable(CGameTable* pTable) { m_pGameTable = pTable; }
	virtual void Release();
	virtual void BindCorePlayer2Player(IPlayer* pCorePlayer);
	virtual void ProcessPacket(const char* pData, int nLength);

	virtual void OnNetStatusChange(bool status);
	template<class T>
	void SendPacket(const T& packet)
	{
		if(m_pCorePlayer)
		{
			COutputStream os;
			if (use_proto())
			{
				SendPacketToProto(os, packet);
			}
			else {
				os << packet;
			}
			m_pCorePlayer->Send(os.GetData(), os.GetLength());
		}
	}
	bool use_proto() { return m_pCorePlayer->GetPlyAttr(PLY_PROTOCOL_TYPE, 0) == 1; }

	template<class T>
	void UpdateToPlayer(const T& packet)
	{
		if (m_pCorePlayer)
		{
			COutputStream os;
			os << packet;
			m_pCorePlayer->UpdateToPlayer(os.GetData(), os.GetLength());
		}
	}
	void UpdateToPlayer(const char* pData, int nLength)
	{
		if (m_pCorePlayer)
		{
			m_pCorePlayer->UpdateToPlayer(pData, nLength);
		}
	}

public:
	//以下是网络协议处理函数
// 	void OnPacket(const pt_Cli_PlayCard_Ack& ack);
// 	void OnPacket(const pt_Cli_RobLord_Ack& ack);
// 	void OnPacket(const pt_Cli_Auto_Req& req);
// 	void OnPacket(const pt_Cli_CompleteData_Req& req);
// 	void OnPacket(const pt_Cli_ShowCard_Ack& ack);
	void OnPacket(const pt_cg_private_room_result_req& req);

	void OnPacket(const pt_magic_emoji_req& req);
	void OnPacket(const pt_cg_starsky_season_noti& noti);
	void OnPacket(const pt_ss_rp_update_grading_info_ack& ack);
	void OnPacket(const pt_ss_update_regain_lose_score_ack& ack);	
	void OnPacket(const pt_gc_card_recode_req& req);
	void OnPacket(const pt_cg_get_redpackets_award_req& req);
	void OnPacket(const pt_cg_get_redpackets_88yuan_award_req& req);
	void OnPacket(const pt_cg_look_lord_card_req& req);
	void OnPacket(const pt_cg_beishu_info_req& req);
	void OnPacket(const pt_cg_regain_lose_score_req& req);
	void OnPacket(const pt_cg_enable_invincible_req& req);
	void OnPacket(const pt_cg_get_redpackets_newbie_award_ack& ack);
	void OnPacket(const pt_ss_get_newbie_reward_ack& ack);
	void OnPacket(const pt_cg_look_lord_card_item_req& req);
	void OnPacket(const pt_cg_win_doubel_req& req);
	void OnPacket(const pt_cg_baiyuan_hb_round_award_req& req);
	void OnPacket(const pt_cg_baiyuan_win_double_req& req);
	void OnPacket(const pt_cg_baiyuan_regain_lose_req& req);
	void OnPacket(const pt_cg_baiyuan_luck_welfare_req& req);
	void OnPacket(const pt_cg_baiyuan_can_bankruptcy_defend_req& req);
	void OnPacket(const pt_cg_baiyuan_bankruptcy_defend_req& req);
	
public:
	IPlayer* m_pCorePlayer;			///< 系统接口
	CGameTable* m_pGameTable;		///< 系统接口
	CPlayCards m_PlayCard;			///< 牌类规则
	int m_nCallScore;				///< 叫分情况,0:没叫,不能抢地主
	bool m_bGiveUp;					///< 是否过牌
	bool m_bAuto;					///< 是否托管
	int m_nPutCount;				///< 出牌次数
	int m_nTimeOut;					///< 超时出牌次数
	vector<CCardsType> m_vecPutCard;///< 打出的牌型
	vector<CCard> m_vecHandCard;	///< 初始化的手牌
	vector<CCard> m_allCards;		///<全副牌
	int m_nStatus;                  ///身份
	unsigned int m_curTime;         ///用于计算各个阶段用时
	int m_nBetLordCardIndex;
	int m_nBetLordCardReward;
	int m_nBetLordCardNextIndex;
	bool newRoundBegin;
	int m_nStandardDoubleScore;				// 标准模式下加倍分数
	int m_nStandardDoubleStep;				// 标准模式下加倍步骤(0为还没处理，1为服务器已经通知客户端，2为客户端已经处理并反馈给服务器)
	int m_nStandardBigerDoubleScore;		// 标准模式下最大的加倍分数

	int m_nStarSkySeason;					//当前赛季
	int m_nStarSkyScore;					//当前分数
	bool m_bUseCounts;						//是否使用了记牌器

	int m_nWinRound;	//0平局 1输 2赢
	int m_nVIPlevel;	//vip等级，只在生成红包的时候才赋值

	int m_nRedPackets_AwardNum;			// 奖励的红包金额
	int m_nRedPackets_AwardPool;		// 奖池
	int m_nRedPackets_Status;			// 0-不可领取，1-可以领取
	int m_nRedPackets_LimitRound;		// 限制局数
	int m_nRedPackets_TaskID;				// 任务ID;
	int m_nRedPackets_TotalAward;		// 获取总的奖励
	int m_nRedPackets_TotalAwardCount;	// 获取总的奖励次数
	int m_nRedPackets_CurRound;			// 当前局数

	int m_nRedPackets_88Yuan_AwardNum;         // 奖励的红包金额
	int m_nRedPackets_88Yuan_AwardPool;        // 奖池
	int m_nRedPackets_88Yuan_Status;           // 0-不可领取，1-可以领取 2已经领取
	int m_nRedPackets_88Yuan_LimitRound;       // 限制局数
	int m_nRedPackets_88Yuan_TaskID;           // 任务ID;
	int m_nRedPackets_88Yuan_TotalAward;       // 获取总的奖励
	int m_nRedPackets_88Yuan_TotalAwardCount;  // 获取总的奖励次数
	int m_nRedPackets_88Yuan_CurRound;         // 当前局数

	int m_nShowCard;	///< 是否亮牌打
	int m_nMyDouble;	// 个人倍数
	bool m_bLookCard;	// 开局看底牌
	int m_nTempForShowCardBet;

	bool m_bIsDouble;	// 是否加倍

	int m_nCallLordRate;
	bool m_nisRoundStart;
	int64 m_nGameScore; // 玩家输赢多少 为胜利翻倍开发 
	vector<int> m_vecNewbieCardsId;
	vector<player_itemInfo> m_vecRedPackets_Iteminfo;
private:	
	RegainLoseScoreGameData m_rls_game_data;
	RedPacketsNewbieData m_redpackets_newbie_data;

	bool m_bInvincible;			// true 开局免输

// 百元赛
public:
	int m_nBaiYuanHBCurRound;		
	vector<item_info> m_vecBaiYuanHBRoundAward;	// 百元赛红包局数
	vector<item_info> m_vecBaiYuanWinDouble;	// 百元赛赢分加倍
	vector<item_info> m_vecBaiYuanRegainLose;	// 百元赛追回损失
	vector<item_info> m_vecBaiYuanLuckWelfare;	// 百元赛幸运福利
	void SendBaiYuanHBRound();		// 百元赛红包局数
	void SendBaiYuanHBRoundAward();	// 百元赛赢分加倍
	void SendBaiYuanWinDouble();	// 百元赛赢分加倍
	void SendBaiYuanRegainLose();	// 百元赛追回损失
	void SendBaiYuanLuckWelfare();	// 百元赛幸运福利

	// 根据道具id获得玩家道具数量
	int getItemNum(ITEM_INDEX index);
	void updatePlayerItems(vector<item_info>& vecItems, const char* buffer);
};
