/*
 CopyRight(c)2006 by Banding,Shanghai, All Right Reserved.

 @Date:	        2009/4/16
 @Created:	    10:36
 @Filename: 	packetdefine.h
 @File path:	BDLeyoyoV2\DouDiZhu\Common 
 @Author:		leiliang
	
 @Description:	网络协议
*/

#pragma once

NET_PACKET(CCard)
{
public:
	char m_nColor;  //花色,-1表示没有任意花色
	char m_nValue;  //数值,-1表示还没有牌
	char m_nCard_Baovalue; //如果是宝牌 实际使用值
	CCard() : m_nColor(-1), m_nValue(-1), m_nCard_Baovalue(-1){}
	CCard(int nColor, int nValue) : m_nColor(nColor), m_nValue(nValue){}
	bool operator == (const CCard &srcCard)
	{
		return (m_nColor == srcCard.m_nColor && m_nValue == srcCard.m_nValue /*&& m_nCard_Baovalue == srcCard.m_nCard_Baovalue*/);//chanege by JL 
	}
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(CCardsType)
{
	int m_nTypeBomb;	// 0:不是炸弹 1:软炸弹 2:硬炸弹 
	int m_nTypeNum;		//牌的数量
	int m_nTypeValue;	//牌的值
	void SetValue(int nBomb, int nNum, int nValue)
	{
		m_nTypeBomb = nBomb;
		m_nTypeNum = nNum;
		m_nTypeValue = nValue;
	}
};
NET_PACKET(TaskItem)
{
	int		task_id_;
	string	task_desc_;
	string  task_mission_;
	int		task_money_type_;	//奖励类型
	int		task_money_;		//奖励数量
	int		task_rate_;
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(pt_gc_game_start_not)
{
	int nGameMoney;		//底注
	int nCardNum;		//牌的数量
	int nLordPos;		//地主牌的位置
	CCard cLordCard;	//地主牌 
	int nSerialID;		//消息序列
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(pt_cg_send_card_ok_ack)
{
	int nSerialID;						//消息序列
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(pt_gc_refresh_card_not)
{
	char cChairID;						//玩家坐位号
	vector<CCard> vecCards;				//手中的牌
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(pt_cg_get_card_req)
{
	int nSerialID;		//消息序列
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(pt_gc_get_card_ack)
{
	int num;						//玩家坐位号
	vector<CCard> vecCards0;				//手中的牌
	vector<CCard> vecCards1;	
	vector<CCard> vecCards2;	
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(pt_gc_call_score_req)
{
	int nScore;			//叫分基础
	int nSerialID;		//消息序列
};
NET_PACKET(pt_cg_call_score_ack)
{
	int nScore;			//0:不叫,1:1分,2:2分,3:3分
	int nSerialID;		//消息序列
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(pt_gc_rob_lord_req)
{
	char cDefaultLord;	//默认地主座位号
	int nSerialID;		//消息序列
};
NET_PACKET(pt_cg_rob_lord_ack)
{
	char cRob;			//0:不抢,1:抢地主
	int nSerialID;		//消息序列
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(pt_gc_extra_double_score_not)
{
	int nDouble;		//加倍倍数
	int nLordDouble;	//地主牌加倍倍数
	int nSerialID;		//消息序列
};
NET_PACKET(pt_gc_double_score_not)
{
	int nDouble;		//加倍倍数
	int nSerialID;		//消息序列
	char cChairID;		//玩家坐位号
};
NET_PACKET(pt_gc_double_score_req)
{
	int nSerialID;		//消息序列
};
NET_PACKET(pt_cg_double_score_ack)
{
	int nScore;			//加倍数目
	int nSerialID;		//消息序列
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(pt_gc_lord_card_not)
{
	char cLord;							//地主
	vector<CCard> vecCards;				//地主的底牌
};

NET_PACKET(pt_gc_bet_lord_card_result_ack)
{
	int ret;
	int money;
};

NET_PACKET(pt_gc_bet_lord_card_ack)
{
	int ret;
	int index;
};

NET_PACKET(pt_cg_bet_lord_card_req)
{
	int index;
};

NET_PACKET(pt_cg_get_lord_card_reward)
{
	int index;
};

NET_PACKET(pt_gc_get_lord_card_reward)
{
	int index;
	int money;
};

NET_PACKET(pt_gc_lord_card_lottery_info)
{
	int fee;
	vector<int> vecReward;
};

NET_PACKET(pt_cg_lord_card_lottery_info)
{

};

NET_PACKET(pt_gc_lord_card_lottery_base)
{
	int ret;
	int index_;
};

//////////////////////////////////////////////////////////////////////////
NET_PACKET(pt_gc_play_card_req)
{
	char cAuto;							//1:自动出牌,0:手动出牌
	int nSerialID;						//消息序列
};
NET_PACKET(pt_cg_play_card_ack)
{
	int nSerialID;						//消息序列
	char cTimeOut;						//1:超时出牌,0:正常出牌
	vector<CCard> vecCards;				//打出的牌
};
NET_PACKET(pt_gc_play_card_not)
{
	char cChairID;						//玩家坐位号
	vector<CCard> vecCards;				//出的牌
	CCardsType cType;					//出的牌型
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(pt_gc_common_not)
{
	int nOp;							//消息类型
	char cChairID;						//玩家坐位号
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(stUserResult)
{
	char nChairID;
	int	nScore;
};
NET_PACKET(pt_gc_game_result_not)
{
	char bType;					//0:异常结束,1:地主赢,2:农民赢
	int cDouble;				//总倍数
	char cCallScore;			//叫分倍数
	char bShowCard;				//地主是否亮牌打的,1:亮牌
	char nBombCount;			//炸弹个数
	char bSpring;				//是否春天,1:春天
	char bReverseSpring;		//是否反春天,1:反春天
	char bRobLord;				//抢地主倍数
	vector<stUserResult> vecUserResult;
};
NET_PACKET(stUserResult1)
{
	char nChairID;
	int	nScore;		//游戏币
	int nJifen;		//积分
};
NET_PACKET(pt_gc_game_result_not1)
{
	char bType;					//0:异常结束,1:地主赢,2:农民赢
	int cDouble;				//总倍数
	char cCallScore;			//叫分倍数
	char bShowCard;				//地主是否亮牌打的,1:亮牌
	char nBombCount;			//炸弹个数
	char bSpring;				//是否春天,1:春天
	char bReverseSpring;		//是否反春天,1:反春天
	char bRobLord;				//抢地主倍数
	vector<stUserResult1> vecUserResult1;
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(pt_gc_bomb_not)
{
	int nDouble;			//底注倍数
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(pt_cg_auto_req)
{
	char cAuto;				//1:托管，0:取消托管
};
NET_PACKET(pt_gc_auto_not)
{
	char cChairID;			//托管玩家座位号
	char cAuto;				//1:托管，0:取消托管
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(stUserData)
{
	char cChairID;
	vector<CCard> vecHandCards;
	vector<CCard> vecPutCards;
};
NET_PACKET(pt_cg_complete_data_req)
{
};
NET_PACKET(pt_gc_complete_data_not)
{
	int nGameMoney;								//本局游戏底分
	int nDouble;								//倍数
	char cLord;									//地主
	vector<CCard> vecLordCards;					//地主的3张牌
	vector<stUserData> vecData;					//玩家数据
};
//////////////////////////////////////////////////////////////////////////
NET_PACKET(pt_gc_show_card_req)
{
	int nSerialID;						// 消息序列
	int nShowCardType;					// 亮牌类型 1组局亮牌， 2发牌时亮牌
	int nShowCardBet;					// 亮牌倍数
};
NET_PACKET(pt_cg_show_card_ack)
{
	char cShowCard;						// 0:不亮,1:亮牌
	int nSerialID;						// 消息序列
	int nShowCardBet;					// 亮牌倍数
	int nShowCardType;					// 亮牌类型 1组局亮牌， 2发牌时亮牌
};
NET_PACKET(pt_gc_show_card_not)
{
	int nChairID;						//明牌的座位号
	vector<CCard> vecCards;				//手中的牌
};
NET_PACKET(pt_cg_look_lord_card_req)
{

};
NET_PACKET(pt_cg_look_lord_card_item_req)
{

};
NET_PACKET(pt_gc_look_lord_card_item_ack)
{
	int nRet;
};
NET_PACKET(pt_gc_item_add_not)
{
	int nItemIndex;
	int nItemCount;
};
NET_PACKET(pt_cg_beishu_info_req)
{

};
NET_PACKET(pt_cg_regain_lose_score_req) {
	char nOp;	//0 查询，1 领取
	int nItemIndex;	//道具id
	int nItemNum;	//道具数量
};
NET_PACKET(pt_gc_regain_lose_score_ack) {	
	int nRet; // 0 查询时间，>0 获取成功, -1,没开启功能; -2,没有次数; -3,超出时间; -4,领完了	
	int nTime;	// 剩余时间
	vector<int> nValue;
	int nCurCount;
	int nItemIndex;
	int nItemNum;
	pt_gc_regain_lose_score_ack() {
		opcode = gc_regain_lose_score_ack;
	}
};
NET_PACKET(pt_cg_enable_invincible_req)
{
	char nOp;
};
NET_PACKET(pt_gc_enable_invincible_ack)
{
	int nRet;
	pt_gc_enable_invincible_ack() {
		opcode = gc_enable_invincible_ack;
	}
};


NET_PACKET(pt_gc_get_redpackets_newbie_award_req)
{
	int nAmount;
	char cDouble;
};
NET_PACKET(pt_cg_get_redpackets_newbie_award_ack)
{
	char cDouble;
};
NET_PACKET(pt_gc_get_redpackets_newbie_award_not)
{
	int nRet;
	int nAmount;
};

NET_PACKET(pt_gc_beishu_info_ack)
{	
	vector<int> vecBeiShuInfo;		//0初始， 1明牌， 2抢地主， 3，炸弹 4，春天
	vector<int> vecPlayerBeiShu;
	pt_gc_beishu_info_ack() {
		vecBeiShuInfo.clear();
		vecPlayerBeiShu.clear();
	}
};

NET_PACKET(pt_gc_clienttimer_not)
{
	int			chairId;
	int			sPeriod;
};
NET_PACKET(pt_gc_task_not)
{
	TaskItem    task_item_;				//本局任务
};
NET_PACKET(pt_gc_task_complete_not)
{
	int			chair_id_;				//任务完成通知	
	char		task_status_;			//0：未完成  1：完成
};
NET_PACKET(pt_cg_card_count_req)
{

};

NET_PACKET(pt_gc_card_count_ack1)
{
	int counts_num_;
	vector<CCard> m_vecPutCard;
};

NET_PACKET(pt_gc_card_count_ack)
{
	char counts_num_;
	vector<CCard> m_vecPutCard;
};

NET_PACKET(pt_gc_laizi_not)
{
	char card_value;
};

NET_PACKET(pt_gc_counts_not)
{
	char counts_num_;
};

NET_PACKET(pt_gc_counts_not1)
{
	int counts_num_;
};

// 因为游戏内消耗道具，新的数量是没有通知到客户端，所以新加消失提示客户端新的物品数量
NET_PACKET(pt_gc_item_info_not)
{
	int nItemIndex;
	int nItemCount;
};

NET_PACKET(pt_gc_expression_not)
{
	int expression_type_;     //FACE_CAT
	int expression_num_;		//0 表情不可用  1 表情可用
};

//sicbo start
NET_PACKET(pt_sic_new_round_not)
{
    
};
NET_PACKET(pt_sic_history_req)
{
    
};
NET_PACKET(pt_sic_history_ack)
{
	int bIsFounder;	// 是否是桌子的建立者
	int nPlayerMoneyBefore;	// 玩家比赛之前的钱
	vector<string> vecHistory;	// 如 "145" 前20回合的历史骰子记录
	int nChipType;	// 筹码类型 1普通筹码 2动态筹码
	vector<int> vecChipValue;	// 筹码面值 5 个
	int nAmountMax;	// 每回合最大注金限制
	float fNextRoundTimeWait;	// 到下一回合开始还需要等待多少时间[毫秒]
	vector<int> vecPeriod; // 每回合中各阶段的时间长度 [毫秒]
};
NET_PACKET(pt_sic_bet_begin_not)
{
    
};
NET_PACKET(pt_sic_bet_req)
{
	char cCellID;	// 格子
	int nAmount;	// 下注
};
NET_PACKET(pt_sic_bet_ack)
{
	int bAllow;			// 是否允许下注
	char cCellID;
	int nAmountCell;	// 下注成功后的增量
	int nAmountCellTotal;	// 这个格子的总注数更新
};

NET_PACKET(pt_sic_bet_update_req)
{
	int nBetUpdateAckTag;	// 递增版本号
};

// 定义结构体 格子id 所押注数
NET_PACKET(st_sic_cell)
{
	char cCellID;		// 所有玩家下注的钱
	int nAmount;    // 这个格子的总注数更新
};

NET_PACKET(pt_sic_bet_update_ack)
{
	int nBetUpdateAckTag;	// 服务器端全体玩家下注格子筹码版本号
	vector<st_sic_cell> vecAmountCell; // 每个桌子内装有所有玩家总下注数
};

NET_PACKET(pt_sic_bet_clear_req)
{
    
};
NET_PACKET(pt_sic_bet_clear_ack)
{
	int nBetUpdateAckTag; // 服务器端全体玩家下注格子筹码版本号
	int nAmountBack;	// 返回玩家的钱
	int nAmountTotal;	// 所有玩家下注的钱
};
NET_PACKET(pt_sic_result_data_not)
{
	vector<char> vecDice;	// 骰子
	int	nWinMoney;  // 总共赢的钱
	int nPlayerMoneyAfter;	// 玩家之后的钱
	vector<char> vecWinCells; //赢的区域
	vector<char> vecPlayerWinCells; //自己中的区域
	vector<string> vecHitAward;	// 本局中奖数据 [奖项名,倍率,筹码,奖金]
	vector<string> vecRank;		// 本局收益排行榜 [排名,昵称,赢得金币]
};
NET_PACKET(pt_sic_show_light_cell_not)
{
    
};
NET_PACKET(pt_sic_show_result_not)
{
    
};

NET_PACKET(pt_cli_my_req)
{
	int a;
};

NET_PACKET(pt_svr_my_ack)
{
	int b;
};

NET_PACKET(pt_svr_test_not)
{
	char			cTest;		//test1
	int				nTest;		//test2
	vector<char>	vecTest;	//test3
};

NET_PACKET(pt_mj_completedata_req)
{
    
};
//////////////////////////////////////////////////////////////////////////
//三连抽
NET_PACKET(Money_DiZhu)//本金-底注
{
	int64 nMoney;//本金
	int64 nDiZhu;//底注
};
NET_PACKET(Kind_Rate)//牌型-倍率
{
	string strKind;//牌型
	string strRate;//倍率
};
NET_PACKET(pt_cg_three_draw_req_data)
{
};
NET_PACKET(pt_gc_three_draw_ack_data)
{
	int					ret_;			//结果
	vector<Money_DiZhu>	vecMoneyDiZhus;	//不同本金不同底注
	vector<Kind_Rate>	vecKindRates;	//不同牌型不同倍数
	int					nBaoDiMoney;	//抽奖保底金币
	int					nBaoDiRate;		//抽奖保底倍数
	float				fMinRate;		//抽奖最低倍数
};
NET_PACKET(pt_cg_three_draw_req_card)
{
};
NET_PACKET(pt_gc_three_draw_ack_card)
{
	int				ret_;		//结果
	vector<CCard>	vecCards;	//显示的9张牌的信息
	vector<float>	vecRates;	//每一行获得的倍数
	int64			nMoney;		//抽奖前玩家的金币
	int64			nDiZhu;		//抽奖底注
	float			fRate;		//抽奖保底倍数
};
NET_PACKET(pt_gc_three_draw_notify)
{
	string	message_;	//发送的信息
};

//魔法表情 请求
NET_PACKET(pt_magic_emoji_req)
{
	char cEmojiIndex;
	char cToChairID;
	char cCostType;
};
//魔法表情 广播
NET_PACKET(pt_magic_emoji_noti)
{
	char cIsError; //0,为成功
	char cEmojiIndex;
	char cFromChairID;
	char cToChairID;
	char cEmojiNum;
};
//魔法表情配置 广播
NET_PACKET(emojiConfig)
{
	int cEmojiIndex;
	int cCostType;
	int cCostValue;
	int nTenItemIndex;
	int nTenItemNum;
	int nTenEmojiNum;
};

NET_GET(emojiConfig)
{
	return is >> pt.cEmojiIndex >> pt.cCostType >> pt.cCostValue >> pt.nTenItemIndex >> pt.nTenItemNum >> pt.nTenEmojiNum;
}
NET_PUT(emojiConfig)
{
	return os << pt.cEmojiIndex << pt.cCostType << pt.cCostValue << pt.nTenItemIndex << pt.nTenItemNum << pt.nTenEmojiNum;
}

NET_PACKET(pt_gc_magic_emoji_config_not)
{
	vector<emojiConfig> emojiConfigs;
	pt_gc_magic_emoji_config_not() {
		opcode = gc_magic_emoji_config_not;
	}
};

NET_GET(pt_gc_magic_emoji_config_not)
{
	return is >> pt.opcode >> pt.emojiConfigs;
}
NET_PUT(pt_gc_magic_emoji_config_not)
{
	return os << pt.opcode << pt.emojiConfigs;
}

// 通知客户端游戏模式
NET_PACKET(pt_gc_game_model)
{
	char cModelType;		// 0为普通模式，1位标准模式
};

NET_PACKET(pt_cg_starsky_season_noti)
{
	int season_;
};

NET_PACKET(pt_gc_starsky_update_item_noti)
{
	int stamina_;	//体力值
	int match_ticket_;//参赛券
	int score_;		// 分数
	int savestar_card_;// 保星卡
};

NET_PACKET(pt_gc_user_savestar_card_noti)
{
};

NET_PACKET(pt_gc_use_card_recode_noti)
{
	char cChairID;						//通知使用记牌器玩家
	char cReconnection;					//通知是否断线重连
};
NET_PACKET(pt_gc_card_recode_req)
{
};

//////////////////////////////////////////////////////////////////////////
//二人斗地主
NET_PACKET(pt_gc_two_show_card_not)
{
	char cChairID;						//默认地主的座位号
	int nLordPos;						//显示的牌的位置
	CCard cLordCard;					//显示牌的坐标
};
NET_PACKET(pt_gc_two_let_card_not)
{
	int nLetNum;						//让牌数目
};
NET_PACKET(pt_gc_two_lord_card_not)
{
	char cLord;							//地主
	vector<CCard> vecCards;				//地主的底牌
	int nLetNum;                        //让牌数目
	int nLordLetNum;					//地主让牌数目
};
NET_PACKET(pt_gc_two_complete_data_not)
{
	int nGameMoney;						//本局游戏底分
	int nDouble;						//倍数
	char cLord;							//地主
	vector<CCard> vecLordCards;			//地主的3张牌
	vector<stUserData> vecData;			//玩家数据
	int nLetNum;						//让牌数目
	int nStart;							//是否开始游戏0 未开始 1 开始
};
//////////////////////////////////////////////////////////////////////////
//私人房
NET_PACKET(GameStatisc)
{
	char cChairID;
	int nCallTimes;//叫地主次数
	int nLordTimes;//当地主次数
	int nWinTimes;//胜利次数
	int64 nZhanJi;//输赢数目
	GameStatisc()
	{
		cChairID = -1;
		nCallTimes = 0;
		nLordTimes = 0;
		nWinTimes = 0;
		nZhanJi = 0;
	}
};
NET_PACKET(pt_cg_private_room_result_req)
{
};
NET_PACKET(pt_gc_private_room_result_ack)
{
	vector<GameStatisc> vecGameStatiscs;//玩家战绩通知
};
NET_PACKET(pt_gc_private_room_result_not)
{
	int ret_;//0 正常结束 -1 异常结束
	vector<GameStatisc> vecGameStatiscs;//玩家战绩通知
};
NET_PACKET(pt_gc_pause_game_not)
{
	int nFlag; //0：暂停 1：恢复
	int nMinTime; //分
	int nSecTime; //秒
	char cChairId;
	string sNickName;
};
NET_PACKET(pt_gc_had_start_not)
{
};
NET_PACKET(pt_gc_ju_count_not)
{
	int nJuCount;
};
//////////////////////////////////////////////////////////////////////////

NET_GET(pt_sic_new_round_not)
{
	return is >> pt.opcode;
}
NET_PUT(pt_sic_new_round_not)
{
	return os << pt.opcode;
}

NET_GET(pt_sic_history_req)
{
	return is >> pt.opcode;
}
NET_PUT(pt_sic_history_req)
{
	return os << pt.opcode;
}

NET_GET(pt_sic_history_ack)
{
	return is >> pt.opcode >> pt.bIsFounder >> pt.nPlayerMoneyBefore >> pt.vecHistory >> pt.nChipType >> pt.vecChipValue >> pt.nAmountMax >> pt.fNextRoundTimeWait >> pt.vecPeriod;
}
NET_PUT(pt_sic_history_ack)
{
	return os << pt.opcode << pt.bIsFounder << pt.nPlayerMoneyBefore <<pt.vecHistory << pt.nChipType << pt.vecChipValue << pt.nAmountMax << pt.fNextRoundTimeWait << pt.vecPeriod;
}


NET_GET(pt_sic_bet_begin_not)
{
	return is >> pt.opcode;
}
NET_PUT(pt_sic_bet_begin_not)
{
	return os << pt.opcode;
}

NET_GET(pt_sic_bet_req)
{
	return is >> pt.opcode >> pt.cCellID >> pt.nAmount;
}
NET_PUT(pt_sic_bet_req)
{
	return os << pt.opcode << pt.cCellID << pt.nAmount;
}

NET_GET(pt_sic_bet_ack)
{
	return is >> pt.opcode >> pt.bAllow >> pt.cCellID >> pt.nAmountCell >> pt.nAmountCellTotal;
}
NET_PUT(pt_sic_bet_ack)
{
	return os << pt.opcode << pt.bAllow << pt.cCellID << pt.nAmountCell << pt.nAmountCellTotal;
}


NET_GET(pt_sic_bet_update_req)
{
	return is >> pt.opcode >> pt.nBetUpdateAckTag;
}
NET_PUT(pt_sic_bet_update_req)
{
	return os << pt.opcode << pt.nBetUpdateAckTag;
}

NET_GET(st_sic_cell)
{
	return is >> pt.opcode >> pt.cCellID >> pt.nAmount;
}
NET_PUT(st_sic_cell)
{
	return os << pt.opcode << pt.cCellID << pt.nAmount;
}


NET_GET(pt_sic_bet_update_ack)
{
	return is >> pt.opcode >> pt.vecAmountCell >> pt.nBetUpdateAckTag;
}
NET_PUT(pt_sic_bet_update_ack)
{
	return os << pt.opcode << pt.vecAmountCell << pt.nBetUpdateAckTag;
}

NET_GET(pt_sic_bet_clear_req)
{
	return is >> pt.opcode;
}
NET_PUT(pt_sic_bet_clear_req)
{
	return os << pt.opcode;
}

NET_GET(pt_sic_bet_clear_ack)
{
	return is >> pt.opcode >> pt.nBetUpdateAckTag >> pt.nAmountBack >> pt.nAmountTotal;
}
NET_PUT(pt_sic_bet_clear_ack)
{
	return os << pt.opcode << pt.nBetUpdateAckTag << pt.nAmountBack << pt.nAmountTotal;
}

NET_GET(pt_sic_result_data_not)
{
	return is >> pt.opcode >> pt.vecDice >> pt.nWinMoney >> pt.nPlayerMoneyAfter >> pt.vecWinCells >> pt.vecPlayerWinCells >> pt.vecHitAward >> pt.vecRank;
}
NET_PUT(pt_sic_result_data_not)
{
	return os << pt.opcode << pt.vecDice << pt.nWinMoney << pt.nPlayerMoneyAfter << pt.vecWinCells << pt.vecPlayerWinCells << pt.vecHitAward << pt.vecRank;
}

NET_GET(pt_sic_show_light_cell_not)
{
	return is >> pt.opcode;
}
NET_PUT(pt_sic_show_light_cell_not)
{
	return os << pt.opcode;
}

NET_GET(pt_sic_show_result_not)
{
	return is >> pt.opcode;
}
NET_PUT(pt_sic_show_result_not)
{
	return os << pt.opcode;
}


// -----------------------------------

NET_GET(pt_cli_my_req)
{
	return is >> pt.opcode >> pt.a;
}
NET_PUT(pt_cli_my_req)
{
	return os << pt.opcode << pt.a;
}

NET_GET(pt_svr_my_ack)
{
	return is >> pt.opcode >> pt.b;
}
NET_PUT(pt_svr_my_ack)
{
	return os << pt.opcode << pt.b;
}

NET_GET(pt_svr_test_not)
{
	return is >> pt.opcode >> pt.cTest >> pt.nTest >> pt.vecTest;
}
NET_PUT(pt_svr_test_not)
{
	return os << pt.opcode << pt.cTest << pt.nTest << pt.vecTest;
}

NET_GET(pt_mj_completedata_req)
{
	return is >> pt.opcode;
}
NET_PUT(pt_mj_completedata_req)
{
	return os << pt.opcode;
}
//////////////////////////////////////////////////////////////////////////
//三连抽
NET_GET(Money_DiZhu)
{
	return is >> pt.opcode >> pt.nMoney >> pt.nDiZhu;
}
NET_PUT(Money_DiZhu)
{
	return os << pt.opcode << pt.nMoney << pt.nDiZhu;
}
NET_GET(Kind_Rate)
{
	return is >> pt.opcode >> pt.strKind >> pt.strRate;
}
NET_PUT(Kind_Rate)
{
	return os << pt.opcode << pt.strKind << pt.strRate;
}
NET_GET(pt_cg_three_draw_req_data)
{
	return is >> pt.opcode;
}
NET_PUT(pt_cg_three_draw_req_data)
{
	return os << pt.opcode;
}
NET_GET(pt_gc_three_draw_ack_data)
{
	return is >> pt.opcode >> pt.ret_ >> pt.vecMoneyDiZhus >> pt.vecKindRates >> pt.nBaoDiMoney >> pt.nBaoDiRate >> pt.fMinRate;
}
NET_PUT(pt_gc_three_draw_ack_data)
{
	return os << pt.opcode << pt.ret_ << pt.vecMoneyDiZhus << pt.vecKindRates << pt.nBaoDiMoney << pt.nBaoDiRate << pt.fMinRate;
}
NET_GET(pt_cg_three_draw_req_card)
{
	return is >> pt.opcode;
}
NET_PUT(pt_cg_three_draw_req_card)
{
	return os << pt.opcode;
}
NET_GET(pt_gc_three_draw_ack_card)
{
	return is >> pt.opcode >> pt.ret_ >> pt.vecCards >> pt.vecRates >> pt.nMoney >> pt.nDiZhu >> pt.fRate;
}
NET_PUT(pt_gc_three_draw_ack_card)
{
	return os << pt.opcode << pt.ret_ << pt.vecCards << pt.vecRates << pt.nMoney << pt.nDiZhu << pt.fRate;
}
NET_GET(pt_gc_three_draw_notify)
{
	return is >> pt.opcode >> pt.message_;
}
NET_PUT(pt_gc_three_draw_notify)
{
	return os << pt.opcode << pt.message_;
}
//////////////////////////////////////////////////////////////////////////
//二人斗地主
NET_GET(pt_gc_two_show_card_not)
{
	return is >> pt.opcode >> pt.cChairID >> pt.nLordPos >> pt.cLordCard;
}
NET_PUT(pt_gc_two_show_card_not)
{
	return os << pt.opcode << pt.cChairID << pt.nLordPos << pt.cLordCard;
}
NET_GET(pt_gc_two_let_card_not)
{
	return is >> pt.opcode >> pt.nLetNum;
}
NET_PUT(pt_gc_two_let_card_not)
{
	return os << pt.opcode << pt.nLetNum;
}
NET_GET(pt_gc_two_lord_card_not)
{
	return is >> pt.opcode >> pt.cLord >> pt.vecCards >> pt.nLetNum >> pt.nLordLetNum;
}
NET_PUT(pt_gc_two_lord_card_not)
{
	return os << pt.opcode << pt.cLord << pt.vecCards << pt.nLetNum << pt.nLordLetNum;
}
NET_GET(pt_gc_two_complete_data_not)
{
	return is >> pt.opcode >> pt.nGameMoney >> pt.nDouble >> pt.cLord >> pt.vecLordCards >> pt.vecData >> pt.nLetNum >> pt.nStart;
}
NET_PUT(pt_gc_two_complete_data_not)
{
	return os << pt.opcode << pt.nGameMoney << pt.nDouble << pt.cLord << pt.vecLordCards << pt.vecData << pt.nLetNum << pt.nStart;
}
//////////////////////////////////////////////////////////////////////////
//私人房
NET_GET(GameStatisc)
{
	return is >> pt.opcode >> pt.cChairID >> pt.nCallTimes >> pt.nLordTimes >> pt.nWinTimes >> pt.nZhanJi;
}
NET_PUT(GameStatisc)
{
	return os << pt.opcode << pt.cChairID << pt.nCallTimes << pt.nLordTimes << pt.nWinTimes << pt.nZhanJi;
}
NET_GET(pt_cg_private_room_result_req)
{
	return is >> pt.opcode;
}
NET_PUT(pt_cg_private_room_result_req)
{
	return os << pt.opcode;
}
NET_GET(pt_gc_private_room_result_ack)
{
	return is >> pt.opcode >> pt.vecGameStatiscs;
}
NET_PUT(pt_gc_private_room_result_ack)
{
	return os << pt.opcode << pt.vecGameStatiscs;
}
NET_GET(pt_gc_private_room_result_not)
{
	return is >> pt.opcode >> pt.ret_ >> pt.vecGameStatiscs;
}
NET_PUT(pt_gc_private_room_result_not)
{
	return os << pt.opcode << pt.ret_ << pt.vecGameStatiscs;
}
NET_GET(pt_gc_pause_game_not)
{
	return is >> pt.opcode >> pt.nFlag >> pt.nMinTime >> pt.nSecTime >> pt.cChairId >> pt.sNickName;
}
NET_PUT(pt_gc_pause_game_not)
{
	return os << pt.opcode << pt.nFlag << pt.nMinTime << pt.nSecTime << pt.cChairId << pt.sNickName;
}
NET_GET(pt_gc_had_start_not)
{
	return is >> pt.opcode;
}
NET_PUT(pt_gc_had_start_not)
{
	return os << pt.opcode;
}
NET_GET(pt_gc_ju_count_not)
{
	return is >> pt.opcode >> pt.nJuCount;
}
NET_PUT(pt_gc_ju_count_not)
{
	return os << pt.opcode << pt.nJuCount;
}
NET_PACKET(pt_gc_replay_data_not)
{
	vector<pt_gc_refresh_card_not> vecChangeCards;
};
NET_PACKET(pt_gc_send_dizhu_not)
{
	int	nGameMoney;
};

//sicbo end
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
NET_GET(pt_gc_expression_not)
{
	return is >> pt.opcode >> pt.expression_type_ >> pt.expression_num_;
}

NET_PUT(pt_gc_expression_not)
{
	return os << pt.opcode << pt.expression_type_ << pt.expression_num_;
}

NET_GET(pt_gc_counts_not1)
{
	return is >> pt.opcode >> pt.counts_num_;
}

NET_PUT(pt_gc_counts_not1)
{
	return os << pt.opcode << pt.counts_num_;
}


NET_GET(pt_gc_counts_not)
{
	return is >> pt.opcode >> pt.counts_num_;
}

NET_PUT(pt_gc_counts_not)
{
	return os << pt.opcode << pt.counts_num_;
}

NET_GET(pt_gc_item_info_not)
{
	return is >> pt.opcode >> pt.nItemIndex >> pt.nItemCount;
}

NET_PUT(pt_gc_item_info_not)
{
	return os << pt.opcode << pt.nItemIndex << pt.nItemCount;
}

NET_GET(pt_gc_laizi_not)
{
	return is >> pt.opcode >> pt.card_value;
}

NET_PUT(pt_gc_laizi_not)
{
	return os << pt.opcode << pt.card_value;
}

NET_GET(pt_cg_card_count_req)
{
	return is >> pt.opcode ;
}

NET_PUT(pt_cg_card_count_req)
{
	return os << pt.opcode ;
}

NET_GET(pt_gc_card_count_ack1)
{
	return is >> pt.opcode >> pt.counts_num_ >> pt.m_vecPutCard ;
}

NET_PUT(pt_gc_card_count_ack1)
{
	return os << pt.opcode << pt.counts_num_ << pt.m_vecPutCard ;
}

NET_GET(pt_gc_card_count_ack)
{
	return is >> pt.opcode >> pt.counts_num_ >> pt.m_vecPutCard ;
}

NET_PUT(pt_gc_card_count_ack)
{
	return os << pt.opcode << pt.counts_num_ << pt.m_vecPutCard ;
}

NET_GET(pt_gc_task_not)
{
	return is >> pt.opcode >> pt.task_item_ ;
}
NET_PUT(pt_gc_task_not)
{
	return os << pt.opcode << pt.task_item_ ;
}
NET_GET(pt_gc_task_complete_not)
{
	return is >> pt.opcode >> pt.chair_id_ >> pt.task_status_ ;
}
NET_PUT(pt_gc_task_complete_not)
{
	return os << pt.opcode << pt.chair_id_ << pt.task_status_ ;
}
NET_GET(TaskItem)
{
	return is >> pt.opcode >> pt.task_id_ >> pt.task_desc_ >> pt.task_mission_ >> pt.task_money_type_ >> pt.task_money_;
}
NET_PUT(TaskItem)
{
	return os << pt.opcode << pt.task_id_ << pt.task_desc_ << pt.task_mission_ << pt.task_money_type_ << pt.task_money_;
}
NET_GET(CCard)
{
	return is >> pt.opcode >> pt.m_nColor >> pt.m_nValue >> pt.m_nCard_Baovalue;
}
NET_PUT(CCard)
{
	return os << pt.opcode << pt.m_nColor << pt.m_nValue << pt.m_nCard_Baovalue;
}
NET_GET(CCardsType)
{
	return is >> pt.opcode >> pt.m_nTypeBomb >> pt.m_nTypeNum >> pt.m_nTypeValue;
}
NET_PUT(CCardsType)
{
	return os << pt.opcode << pt.m_nTypeBomb << pt.m_nTypeNum << pt.m_nTypeValue;
}
NET_GET(pt_gc_game_start_not)
{
	return is >> pt.opcode >> pt.nGameMoney >> pt.nCardNum >> pt.nLordPos >> pt.cLordCard >> pt.nSerialID;
}
NET_PUT(pt_gc_game_start_not)
{
	return os << pt.opcode << pt.nGameMoney << pt.nCardNum << pt.nLordPos << pt.cLordCard << pt.nSerialID;
}
NET_GET(pt_cg_send_card_ok_ack)
{
	return is >> pt.opcode >> pt.nSerialID;
}
NET_PUT(pt_cg_send_card_ok_ack)
{
	return os << pt.opcode << pt.nSerialID;
}
NET_GET(pt_gc_refresh_card_not)
{
	return is >> pt.opcode >> pt.cChairID >> pt.vecCards;
}
NET_PUT(pt_gc_refresh_card_not)
{
	return os << pt.opcode << pt.cChairID << pt.vecCards;
}

NET_GET(pt_cg_get_card_req)
{
	return is >> pt.opcode >> pt.nSerialID;
}

NET_PUT(pt_cg_get_card_req)
{
	return os << pt.opcode << pt.nSerialID;
}

NET_GET(pt_gc_get_card_ack)
{
	return is >> pt.opcode >> pt.num >> pt.vecCards0 >> pt.vecCards1 >> pt.vecCards2;
}

NET_PUT(pt_gc_get_card_ack)
{
	return os << pt.opcode << pt.num << pt.vecCards0 << pt.vecCards1 << pt.vecCards2;
}

NET_GET(pt_gc_call_score_req)
{
	return is >> pt.opcode >> pt.nScore >> pt.nSerialID;
}
NET_PUT(pt_gc_call_score_req)
{
	return os << pt.opcode << pt.nScore << pt.nSerialID;
}
NET_GET(pt_cg_call_score_ack)
{
	return is >> pt.opcode >> pt.nScore >> pt.nSerialID;
}
NET_PUT(pt_cg_call_score_ack)
{
	return os << pt.opcode << pt.nScore << pt.nSerialID;
}
NET_GET(pt_gc_rob_lord_req)
{
	return is >> pt.opcode >> pt.cDefaultLord >> pt.nSerialID;
}
NET_PUT(pt_gc_rob_lord_req)
{
	return os << pt.opcode << pt.cDefaultLord << pt.nSerialID;
}
NET_GET(pt_cg_rob_lord_ack)
{
	return is >> pt.opcode >> pt.cRob >> pt.nSerialID;
}
NET_PUT(pt_cg_rob_lord_ack)
{
	return os << pt.opcode << pt.cRob << pt.nSerialID;
}
NET_GET(pt_gc_extra_double_score_not)
{
	return is >> pt.opcode >> pt.nDouble >> pt.nLordDouble >> pt.nSerialID;
}
NET_PUT(pt_gc_extra_double_score_not)
{
	return os << pt.opcode << pt.nDouble << pt.nLordDouble << pt.nSerialID;
};
NET_GET(pt_gc_double_score_not)
{
	return is >> pt.opcode >> pt.nDouble >> pt.nSerialID >> pt.cChairID;
}
NET_PUT(pt_gc_double_score_not)
{
	return os << pt.opcode << pt.nDouble << pt.nSerialID << pt.cChairID;
};
NET_GET(pt_gc_double_score_req)
{
	return is >> pt.opcode >> pt.nSerialID;
}
NET_PUT(pt_gc_double_score_req)
{
	return os << pt.opcode << pt.nSerialID;
};
NET_GET(pt_cg_double_score_ack)
{
	return is >> pt.opcode >> pt.nScore >> pt.nSerialID;
}
NET_PUT(pt_cg_double_score_ack)
{
	return os << pt.opcode << pt.nScore << pt.nSerialID;
};
NET_GET(pt_gc_lord_card_not)
{
	return is >> pt.opcode >> pt.cLord >> pt.vecCards;
}
NET_PUT(pt_gc_lord_card_not)
{
	return os << pt.opcode << pt.cLord << pt.vecCards;
}
NET_GET(pt_gc_play_card_req)
{
	return is >> pt.opcode >> pt.cAuto >> pt.nSerialID;
}
NET_PUT(pt_gc_play_card_req)
{
	return os << pt.opcode << pt.cAuto << pt.nSerialID;
}
NET_GET(pt_cg_play_card_ack)
{
	return is >> pt.opcode >> pt.nSerialID >> pt.cTimeOut >> pt.vecCards;
}
NET_PUT(pt_cg_play_card_ack)
{
	return os << pt.opcode << pt.nSerialID << pt.cTimeOut << pt.vecCards;
}
NET_GET(pt_gc_play_card_not)
{
	return is >> pt.opcode >> pt.cChairID >> pt.vecCards >> pt.cType;
}
NET_PUT(pt_gc_play_card_not)
{
	return os << pt.opcode << pt.cChairID << pt.vecCards << pt.cType;
}
NET_GET(pt_gc_common_not)
{
	return is >> pt.opcode >> pt.nOp >> pt.cChairID;
}
NET_PUT(pt_gc_common_not)
{
	return os << pt.opcode << pt.nOp << pt.cChairID;
}
NET_GET(stUserResult)
{
	return is >> pt.opcode >> pt.nChairID >> pt.nScore;
}
NET_PUT(stUserResult)
{
	return os << pt.opcode << pt.nChairID << pt.nScore;
}
NET_GET(stUserResult1)
{
	return is >> pt.opcode >> pt.nChairID >> pt.nScore >> pt.nJifen;
}
NET_PUT(stUserResult1)
{
	return os << pt.opcode << pt.nChairID << pt.nScore << pt.nJifen;
}
NET_GET(pt_gc_game_result_not)
{
	return is >> pt.opcode >> pt.bType >> pt.cDouble >> pt.cCallScore >> pt.bShowCard >> pt.nBombCount >> pt.bSpring >> pt.bReverseSpring >> pt.bRobLord >> pt.vecUserResult;
}
NET_PUT(pt_gc_game_result_not)
{
	return os << pt.opcode << pt.bType << pt.cDouble << pt.cCallScore << pt.bShowCard << pt.nBombCount << pt.bSpring << pt.bReverseSpring << pt.bRobLord << pt.vecUserResult;
}
NET_GET(pt_gc_game_result_not1)
{
	return is >> pt.opcode >> pt.bType >> pt.cDouble >> pt.cCallScore >> pt.bShowCard >> pt.nBombCount >> pt.bSpring >> pt.bReverseSpring >> pt.bRobLord >> pt.vecUserResult1;
}
NET_PUT(pt_gc_game_result_not1)
{
	return os << pt.opcode << pt.bType << pt.cDouble << pt.cCallScore << pt.bShowCard << pt.nBombCount << pt.bSpring << pt.bReverseSpring << pt.bRobLord << pt.vecUserResult1;
}
NET_GET(pt_gc_bomb_not)
{
	return is >> pt.opcode >> pt.nDouble;
}
NET_PUT(pt_gc_bomb_not)
{
	return os << pt.opcode << pt.nDouble;
}
NET_GET(pt_cg_auto_req)
{
	return is >> pt.opcode >> pt.cAuto;
}
NET_PUT(pt_cg_auto_req)
{
	return os << pt.opcode << pt.cAuto;
}
NET_GET(pt_gc_auto_not)
{
	return is >> pt.opcode >> pt.cChairID >> pt.cAuto;
}
NET_PUT(pt_gc_auto_not)
{
	return os << pt.opcode << pt.cChairID << pt.cAuto;
}
NET_GET(stUserData)
{
	return is >> pt.opcode >> pt.cChairID >> pt.vecHandCards >> pt.vecPutCards;
}
NET_PUT(stUserData)
{
	return os << pt.opcode << pt.cChairID << pt.vecHandCards << pt.vecPutCards;
}
NET_GET(pt_cg_complete_data_req)
{
	return is >> pt.opcode;
}
NET_PUT(pt_cg_complete_data_req)
{
	return os << pt.opcode;
}
NET_GET(pt_gc_complete_data_not)
{
	return is >> pt.opcode >> pt.nGameMoney >> pt.nDouble >> pt.cLord >> pt.vecLordCards >> pt.vecData;
}
NET_PUT(pt_gc_complete_data_not)
{
	return os << pt.opcode << pt.nGameMoney << pt.nDouble << pt.cLord << pt.vecLordCards << pt.vecData;
}
NET_GET(pt_gc_show_card_req)
{
	return is >> pt.opcode >> pt.nSerialID >> pt.nShowCardBet;
}
NET_PUT(pt_gc_show_card_req)
{
	return os << pt.opcode << pt.nSerialID << pt.nShowCardBet;
}
NET_GET(pt_cg_show_card_ack)
{
	return is >> pt.opcode >> pt.cShowCard >> pt.nSerialID >> pt.nShowCardBet >> pt.nShowCardType;
}
NET_PUT(pt_cg_show_card_ack)
{
	return os << pt.opcode << pt.cShowCard << pt.nSerialID << pt.nShowCardBet << pt.nShowCardType;
}
NET_GET(pt_gc_show_card_not)
{
	return is >> pt.opcode >> pt.nChairID >> pt.vecCards;
}
NET_PUT(pt_gc_show_card_not)
{
	return os << pt.opcode << pt.nChairID << pt.vecCards;
}
NET_GET(pt_cg_look_lord_card_req)
{
	return is >> pt.opcode;
}
NET_PUT(pt_cg_look_lord_card_req)
{
	return os << pt.opcode;
}
NET_GET(pt_cg_look_lord_card_item_req)
{
	return is >> pt.opcode;
}
NET_PUT(pt_cg_look_lord_card_item_req)
{
	return os << pt.opcode;
}
NET_GET(pt_gc_look_lord_card_item_ack)
{
	return is >> pt.opcode >> pt.nRet;
}
NET_PUT(pt_gc_look_lord_card_item_ack)
{
	return os << pt.opcode << pt.nRet;
}
NET_GET(pt_gc_item_add_not)
{
	return is >> pt.opcode >> pt.nItemIndex >> pt.nItemCount;
}
NET_PUT(pt_gc_item_add_not)
{
	return os << pt.opcode << pt.nItemIndex << pt.nItemCount;
}
NET_GET(pt_cg_beishu_info_req)
{
	return is >> pt.opcode;
}
NET_PUT(pt_cg_beishu_info_req)
{
	return os << pt.opcode;
}
NET_GET(pt_gc_beishu_info_ack)
{
	return is >> pt.opcode >> pt.vecBeiShuInfo >> pt.vecPlayerBeiShu;
}
NET_PUT(pt_gc_beishu_info_ack)
{
	return os << pt.opcode << pt.vecBeiShuInfo << pt.vecPlayerBeiShu;
}
NET_GET(pt_gc_clienttimer_not)
{
	return is >> pt.opcode >> pt.chairId >> pt.sPeriod ; 
}
NET_PUT(pt_gc_clienttimer_not)
{
	return os << pt.opcode << pt.chairId << pt.sPeriod ;
}
NET_GET(pt_cg_bet_lord_card_req)
{
	return is >> pt.opcode >> pt.index;
}
NET_PUT(pt_cg_bet_lord_card_req)
{
	return os << pt.opcode << pt.index;
}

NET_GET(pt_gc_bet_lord_card_result_ack)
{
	return is >> pt.opcode >> pt.ret >> pt.money;
}
NET_PUT(pt_gc_bet_lord_card_result_ack)
{
	return os << pt.opcode << pt.ret << pt.money;
}

NET_GET(pt_gc_bet_lord_card_ack)
{
	return is >> pt.opcode >> pt.ret >> pt.index;
}
NET_PUT(pt_gc_bet_lord_card_ack)
{
	return os << pt.opcode << pt.ret << pt.index;
}

NET_GET(pt_cg_get_lord_card_reward)
{
	return is >> pt.opcode >> pt.index;
}
NET_PUT(pt_cg_get_lord_card_reward)
{
	return os << pt.opcode << pt.index;
}

NET_GET(pt_gc_get_lord_card_reward)
{
	return is >> pt.opcode >> pt.index >> pt.money;
}
NET_PUT(pt_gc_get_lord_card_reward)
{
	return os << pt.opcode << pt.index << pt.money;
}

NET_GET(pt_gc_lord_card_lottery_info)
{
	return is >> pt.opcode >> pt.fee >> pt.vecReward;
}
NET_PUT(pt_gc_lord_card_lottery_info)
{
	return os << pt.opcode << pt.fee << pt.vecReward;
}

NET_GET(pt_cg_lord_card_lottery_info)
{
	return is >> pt.opcode;
}
NET_PUT(pt_cg_lord_card_lottery_info)
{
	return os << pt.opcode;
}

NET_GET(pt_gc_lord_card_lottery_base)
{
	return is >> pt.opcode >> pt.ret >> pt.index_;
}
NET_PUT(pt_gc_lord_card_lottery_base)
{
	return os << pt.opcode << pt.ret << pt.index_;
}

NET_GET(pt_gc_replay_data_not)
{
	return is >> pt.opcode >> pt.vecChangeCards;
}
NET_PUT(pt_gc_replay_data_not)
{
	return os << pt.opcode << pt.vecChangeCards;
}

NET_GET(pt_gc_send_dizhu_not)
{
	return is >> pt.opcode >> pt.nGameMoney;
}
NET_PUT(pt_gc_send_dizhu_not)
{
	return os << pt.opcode << pt.nGameMoney;
}

NET_GET(pt_magic_emoji_req)
{
	return is >> pt.opcode >> pt.cEmojiIndex >> pt.cToChairID >> pt.cCostType;
}

NET_PUT(pt_magic_emoji_req)
{
	return os << pt.opcode << pt.cEmojiIndex << pt.cToChairID << pt.cCostType;
}

NET_GET(pt_magic_emoji_noti)
{
	return is >> pt.opcode >> pt.cIsError >> pt.cEmojiIndex >> pt.cFromChairID >> pt.cToChairID >> pt.cEmojiNum;
}

NET_PUT(pt_magic_emoji_noti)
{
	return os << pt.opcode << pt.cIsError << pt.cEmojiIndex << pt.cFromChairID << pt.cToChairID << pt.cEmojiNum;
}

NET_GET(pt_gc_game_model)
{
	return is >> pt.opcode >> pt.cModelType;
}

NET_PUT(pt_gc_game_model)
{
	return os << pt.opcode << pt.cModelType;
}

NET_GET(pt_cg_starsky_season_noti)
{
	return is >> pt.opcode >> pt.season_;
}

NET_PUT(pt_cg_starsky_season_noti)
{
	return os << pt.opcode << pt.season_;
}
NET_GET(pt_gc_starsky_update_item_noti)
{
	return is >> pt.opcode >> pt.stamina_>>pt.match_ticket_ >> pt.score_ >> pt.savestar_card_;
}

NET_PUT(pt_gc_starsky_update_item_noti)
{
	return os << pt.opcode << pt.stamina_  << pt.match_ticket_ << pt.score_ << pt.savestar_card_;
}

NET_GET(pt_gc_user_savestar_card_noti)
{
	return is >> pt.opcode;
}

NET_PUT(pt_gc_user_savestar_card_noti)
{
	return os << pt.opcode;
}

NET_GET(pt_gc_use_card_recode_noti)
{
	return is >> pt.opcode >> pt.cChairID >> pt.cReconnection;
}

NET_PUT(pt_gc_use_card_recode_noti)
{
	return os << pt.opcode << pt.cChairID << pt.cReconnection;
}

NET_GET(pt_gc_card_recode_req)
{
	return is >> pt.opcode;
}

NET_PUT(pt_gc_card_recode_req)
{
	return os << pt.opcode;
}

//红包
NET_PACKET(pt_cg_get_redpackets_award_req)
{
	char	type_;		//0查询, 1领取
};
NET_GET(pt_cg_get_redpackets_award_req)
{
	return is >> pt.opcode >> pt.type_;
}
NET_PUT(pt_cg_get_redpackets_award_req)
{
	return os << pt.opcode << pt.type_;
}


//红包
NET_PACKET(pt_cg_get_redpackets_88yuan_award_req)
{
	char	type_;		//0查询, 1领取
};
NET_GET(pt_cg_get_redpackets_88yuan_award_req)
{
	return is >> pt.opcode >> pt.type_;
}
NET_PUT(pt_cg_get_redpackets_88yuan_award_req)
{
	return os << pt.opcode << pt.type_;
}

NET_PACKET(player_itemInfo)
{
	int	nItemIndex;
	int	nItemNum;
	int64 nItemNum64;
};

NET_GET(player_itemInfo)
{
	return is >> pt.nItemIndex >> pt.nItemNum >> pt.nItemNum64;
}
NET_PUT(player_itemInfo)
{
	return os << pt.nItemIndex << pt.nItemNum << pt.nItemNum64;
}

NET_PACKET(pt_gc_update_player_tokenmoney_not)
{
	char	ply_chairid_;
	vector<player_itemInfo> itemInfo;
	pt_gc_update_player_tokenmoney_not() {
		opcode = gc_update_player_tokenmoney_not;
		itemInfo.clear();
	}
};

NET_GET(pt_gc_update_player_tokenmoney_not)
{
	return is >> pt.opcode >> pt.ply_chairid_ >> pt.itemInfo;
}
NET_PUT(pt_gc_update_player_tokenmoney_not)
{
	return os << pt.opcode << pt.ply_chairid_ << pt.itemInfo;
}

NET_PACKET(pt_gc_get_redpackets_award_ack)
{
	char	ret_;			//0通知, 1成功领取
	int 	cur_rounds_;	//当前局数
	int	limit_rounds_;	//目标局数
	int	nAmount;		//金额
	int	cItemtype;		//
	int	task_id_;		//数据库自增ID唯一标识，用于发送给web查询
	vector<player_itemInfo> fakeItem;
	pt_gc_get_redpackets_award_ack() {
		opcode = gc_get_redpackets_award_ack;
		nAmount = 0;
		cItemtype = -1;
		fakeItem.clear();
	}
};
NET_GET(pt_gc_get_redpackets_award_ack)
{
	return is >> pt.opcode >> pt.ret_ >> pt.cur_rounds_ >> pt.limit_rounds_ >> pt.nAmount >> pt.cItemtype >> pt.task_id_ >> pt.fakeItem;
}
NET_PUT(pt_gc_get_redpackets_award_ack)
{
	return os << pt.opcode << pt.ret_ << pt.cur_rounds_ << pt.limit_rounds_ << pt.nAmount << pt.cItemtype << pt.task_id_ << pt.fakeItem;
}

NET_PACKET(pt_gc_get_redpackets_88yuan_award_ack)
{
	char	ret_;			//0通知, 1成功领取
	int 	cur_rounds_;	//当前局数
	int	limit_rounds_;	//目标局数
	int	nAmount;		//金额
	int	cItemtype;		//
	int	task_id_;		//
	pt_gc_get_redpackets_88yuan_award_ack() {
		opcode = gc_get_redpackets_88yuan_award_ack;
		nAmount = 0;
		cItemtype = -1;
	}
};
NET_GET(pt_gc_get_redpackets_88yuan_award_ack)
{
	return is >> pt.opcode >> pt.ret_ >> pt.cur_rounds_ >> pt.limit_rounds_ >> pt.nAmount >> pt.cItemtype >> pt.task_id_;
}
NET_PUT(pt_gc_get_redpackets_88yuan_award_ack)
{
	return os << pt.opcode << pt.ret_ << pt.cur_rounds_ << pt.limit_rounds_ << pt.nAmount << pt.cItemtype << pt.task_id_;
}
NET_GET(pt_cg_regain_lose_score_req)
{
	return is >> pt.opcode >> pt.nOp >> pt.nItemIndex >> pt.nItemNum;
}
NET_PUT(pt_cg_regain_lose_score_req)
{
	return os << pt.opcode << pt.nOp << pt.nItemIndex << pt.nItemNum;
}
NET_GET(pt_gc_regain_lose_score_ack)
{
	return is >> pt.opcode >> pt.nRet >> pt.nTime >> pt.nValue >> pt.nCurCount >> pt.nItemIndex >> pt.nItemNum;
}
NET_PUT(pt_gc_regain_lose_score_ack)
{
	return os << pt.opcode << pt.nRet << pt.nTime << pt.nValue << pt.nCurCount << pt.nItemIndex << pt.nItemNum;
}
NET_GET(pt_cg_enable_invincible_req)
{
	return is >> pt.opcode >> pt.nOp;
}
NET_PUT(pt_cg_enable_invincible_req)
{
	return os << pt.opcode << pt.nOp;
}
NET_GET(pt_gc_enable_invincible_ack)
{
	return is >> pt.opcode >> pt.nRet;
}
NET_PUT(pt_gc_enable_invincible_ack)
{
	return os << pt.opcode << pt.nRet;
}
NET_GET(pt_gc_get_redpackets_newbie_award_req)
{
	return is >> pt.opcode >> pt.nAmount >> pt.cDouble;
}
NET_PUT(pt_gc_get_redpackets_newbie_award_req)
{
	return os << pt.opcode << pt.nAmount << pt.cDouble;
}
NET_GET(pt_cg_get_redpackets_newbie_award_ack)
{
	return is >> pt.opcode >> pt.cDouble;
}
NET_PUT(pt_cg_get_redpackets_newbie_award_ack)
{
	return os << pt.opcode << pt.cDouble;
}
NET_GET(pt_gc_get_redpackets_newbie_award_not)
{
	return is >> pt.opcode >> pt.nRet >> pt.nAmount;
}
NET_PUT(pt_gc_get_redpackets_newbie_award_not)
{
	return os << pt.opcode << pt.nRet << pt.nAmount;
}
NET_PACKET(ItemInfo)
{
	int	nItemIndex;
	int64 nItemNum;
};
NET_GET(ItemInfo)
{
	return is >> pt.nItemIndex >> pt.nItemNum;
}
NET_PUT(ItemInfo)
{
	return os << pt.nItemIndex << pt.nItemNum;
}
NET_PACKET(pt_gc_win_doubel_req)
{
	int nAddAmount;
	int nAddProbabily;
};
NET_GET(pt_gc_win_doubel_req)
{
	return is >> pt.opcode >> pt.nAddAmount >> pt.nAddProbabily;
}
NET_PUT(pt_gc_win_doubel_req)
{
	return os << pt.opcode << pt.nAddAmount << pt.nAddProbabily;
}
NET_PACKET(pt_cg_win_doubel_req)
{

};
NET_GET(pt_cg_win_doubel_req)
{
	return is >> pt.opcode;
}
NET_PUT(pt_cg_win_doubel_req)
{
	return os << pt.opcode;
}
NET_PACKET(pt_gc_win_doubel_ack)
{
	char cRet;
	vector<ItemInfo> vecItemInfo;
};
NET_GET(pt_gc_win_doubel_ack)
{
	return is >> pt.opcode >> pt.cRet >> pt.vecItemInfo;
}
NET_PUT(pt_gc_win_doubel_ack)
{
	return os << pt.opcode << pt.cRet << pt.vecItemInfo;
}
