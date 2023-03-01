#pragma once


enum GAME_OPCODE
{	
	gc_game_start_not = 5100,
	cg_send_card_ok_ack,				//告诉客户端游戏开始
	gc_refresh_card_not,				//客户端牌画完后发送给服务器,服务器收到三家消息后继续下一步动作
	gc_lord_card_not,					//刷新玩家手上的牌
	gc_play_card_req,					//请求出牌
	cg_play_card_ack,					//服务器请求客户端出牌通知
	gc_play_card_not,					//客户端出牌回应
	gc_call_score_req,					//服务器广播客户端出牌
	cg_call_score_ack,					//询问玩家是否叫分
	gc_rob_lord_req,					//客户端回应叫分结果
	cg_rob_lord_ack,					//询问玩家是否抢地主
	gc_common_not,						//客户端回应抢地主结果
	gc_game_result_not,					//通用消息
	gc_bomb_not,						//游戏结果
	cg_auto_req,						//炸弹倍数
	gc_auto_not,						//客户端请求托管
	cg_complete_data_req,				//客户端广播托管信息
	gc_complete_data_not,				//取得完整数据请求
	gc_show_card_req,					//发送完整数据通知
	cg_show_card_ack,					//服务器询问客户端是否明牌
	gc_show_card_not,					//客户端回应服务器是否明牌
	gc_clienttimer_not,				
	gc_task_not,						//服务器通知玩家做任务
	gc_task_complete_not,				//服务器通知玩家完成任务
	cg_card_count_req,					//客户端请求获取打出牌
	gc_card_count_ack,					//服务器广播已经打出的牌
	gc_laizi_not,						//癞子牌广播
	gc_counts_not,						//通知小喇叭计数

	gc_counts_not1,						//通知小喇叭计数调整
	gc_card_count_ack1,					//服务器广播已经打出的牌调整
	gc_game_result_not1,				//通用消息
	gc_expression_not,					    //发送表情 5131
	cg_bet_lord_card_req,
	gc_bet_lord_card_ack,
	gc_bet_lord_card_result_ack,
	cg_get_lord_card_reward,
	gc_get_lord_card_reward,
	cg_lord_card_lottery_info,
	gc_lord_card_lottery_info,
	gc_lord_card_lottery_base,
	//cg_get_card_req,
	//gc_get_card_ack

	cg_three_draw_req_card,					//三连抽抽出的牌
	gc_three_draw_ack_card,					//三连抽抽出的牌
	cg_three_draw_req_data,					//三连抽数据
	gc_three_draw_ack_data,					//三连抽数据
	gc_three_draw_notify,					//通知全部在玩三连抽的人有人拿到高分了

	gc_two_show_card_not,       			//二人斗地主告诉默认地主一张牌
	gc_two_let_card_not,					//二人斗地主让牌数目通知
	gc_two_lord_card_not,					//二人斗地主发地主牌
	gc_two_complete_data_not,				//二人斗地主断线重连
	
	gc_extra_double_score_not,				//通知客户端因为地主牌而加倍
	gc_double_score_not,					//通知客户端底注加倍
	gc_double_score_req,					//通知客户端开始加倍
	cg_double_score_ack,					//客户端加倍
	cg_private_room_result_req,  			//客户端请求私人房战绩
	gc_private_room_result_ack,      		//服务端回应私人房战绩
	gc_private_room_result_not,				//通知解散私人房
	gc_pause_game_not,						//通知客户端暂停游戏
	gc_had_start_not,                       //通知客户端这个房间开始过游戏
	gc_ju_count_not,                        //通知客户端这个房间开始过游戏
	gc_play_card_private_not,			    //客户端出牌回应
	gc_replay_data_not,					    //录像回放
	gc_send_dizhu_not,

	magic_emoji_req,						// 魔法表情请求
	magic_emoji_noti,						// 魔法表情通知

	gc_item_info_not,						// 物品信息更新
	gc_game_modle,							// 通知客户端游戏模式
	cg_starsky_season_noti,					// 通知服务器赛季
	gc_starsky_update_item,					// 通知客户端积分
	gc_user_savestar_card_noti,
	gc_use_card_recode_noti,				// 通知客户端有玩家使用记牌器
	gc_card_recode_req,						// 客户端向服务器请求记牌器数量
	//sicbo start
	// 环节1(摇骰子)
	sic_new_round_not = 10000,			// 服务器通知桌子全体玩家，开始新的回合，并播放摇色子动画

	// 玩家状态[Ready]
	sic_history_req,					// 玩家中途进入游戏或每回合结束，服务器通知客户端更新历史记录
	sic_history_ack,

	// 玩家状态[Play]
	// 环节2(开始下注)
	sic_bet_begin_not,					// 服务器通知桌子全体玩家，开始下注
	sic_bet_req,						// 客户端玩家押注请求
	sic_bet_ack,						// 服务器端响应押注玩家消息
	sic_bet_clear_req,					// 客户端玩家清除押注请求
	sic_bet_clear_ack,					// 服务器响应玩家清除押注结果
	sic_bet_update_req,					// 客户端timer请求服务器返回桌面当前全部格子押注情况
	sic_bet_update_ack,					// 服务器响应玩家，更新押注(每隔一段时间更新)
	// 环节3
	sic_result_data_not,				// 服务器通知桌子全体玩家，下注结束，游戏进入摇色子阶段,对每个玩家分别发送 回合结果数据包
	// 环节4(不一定要)
	sic_show_light_cell_not,			// 服务器通知桌子全体玩家，显示中奖区域
	// 环节5
	sic_show_result_not,				// 显示结算画面


	mj_completedata_req,				// 需要
	//任务
	//gc_task_not,						//服务器通知玩家做任务
	//gc_task_complete_not,				//服务器通知玩家完成任务

	cli_my_req,							// 自定义客户端请求服务器
	svr_my_ack,							// 自定义服务器响应客户端
	svr_test_not,						// 自定义
	//sicbo end
	cg_get_redpackets_award_req,		// 红包查询，红包领取
	gc_get_redpackets_award_ack,		// 红包结果返回
	gc_update_player_tokenmoney_not,	// 刷新桌面玩家道具

	cg_get_redpackets_88yuan_award_req,		// 红包查询，红包领取
	gc_get_redpackets_88yuan_award_ack,		// 红包结果返回

	gc_magic_emoji_config_not,

	cg_look_lord_card_req,
	cg_beishu_info_req,
	gc_beishu_info_ack,
	cg_regain_lose_score_req,	//	结局免输请求
	gc_regain_lose_score_ack,	//	结局免输回复

	cg_enable_invincible_req,	//	开局免输
	gc_enable_invincible_ack,	//

	gc_get_redpackets_newbie_award_req,	// 新人前n局红包奖励
	cg_get_redpackets_newbie_award_ack,	// 新人前n局红包奖励
	gc_get_redpackets_newbie_award_not,	// 新人前n局红包奖励

	cg_look_lord_card_item_req,         // 消耗道具优先看底牌
	gc_look_lord_card_item_ack,         // 消耗道具优先看底牌

	gc_item_add_not,					// 物品信息变化

	gc_win_doubel_req,         // 胜利翻倍
	cg_win_doubel_req,         // 胜利翻倍
	gc_win_doubel_ack,         // 胜利翻倍
};

/**表情包**/
enum EXPRESSION_TYPE
{
	EXPRESSION_CAT,
};

enum COMMON_OPCODE
{
	CO_NEW = 0,								//开始新的一圈牌
	CO_CALL0,								//不叫
	CO_CALL1,								//叫1分
	CO_CALL2,								//叫2分
	CO_CALL3,								//叫3分
	CO_NOTCALLROB,							//不叫地主
	CO_CALLROB,								//叫地主
	CO_NOTROB,								//不抢地主
	CO_ROB,									//抢地主
	CO_GIVEUP,								//过牌
	CO_SHOWCARD,							//亮牌
	CO_TIMEOUT,								//超时	
	CO_READY,								//准备
	CO_NOLORD,								//本局没有地主，请求清理桌面，重新发牌
	CO_START,								//开始游戏
	CO_PUT,									//出牌
	CO_LORDCARD,							//地主底牌
	CO_END,									//游戏结束
	CO_VER,									//游戏版本信息
	CO_DATA,								//保存的玩家信息
	CO_DOUBLE,								//加倍
	CO_F_DOUBLE,							//不加倍
	CO_T_DOUBLE,							//加倍
	CO_CAN_LEAVE_TABLE = 23,				// //离桌
	CO_FORCE_LEAVE_TABLE = 24,				// //强制离桌
	CO_NO_STAMINA = 25,						// //没有体力
	CO_DELAY_KEEPSTAR_TIME = 26,
	CO_SUPER_T_DOUBLE,						//超级加倍
};

//V1.0

enum ACHIEVE_TYPE
{
	AT_DUIZHAN=1,							//对战次数
	AT_CHUJISHENGLI,						//初级场胜利
	AT_ZHONGJISHENGLI,						//中级场胜利
	AT_GAOJISHENGLI,						//高级场胜利
	AT_GUIBINSHENGLI,						//贵宾场胜利
	AT_ZHIZUNSHENGLI,						//至尊场胜利
	AT_ZHADAN,								//炸弹胜利
	AT_FEIJI,								//出飞机次数
	AT_SHUNZI,								//出顺子次数
	AT_LIANDUI,								//出连队次数
	AT_CHUNTIAN,							//春天次数
	AT_FANCHUNTIAN,							//反春天次数
	AT_TASK,								//完成任务次数
	AT_ZHONGJI12BEISHU,						//中级场胜利12倍以上
	AT_GAOJI48BEISHU,						//高级场胜利48倍以上
	AT_ZHONGJIZHADAN,						//中级场打出炸弹次数
	AT_GAOJIRENWU,							//高机场完成任务
	AT_BISAIRENWU,							//比赛场胜利24倍以上
	AT_DAILY_CHUJI,							//初级场玩5局数
	AT_DAILY_CHUJI_WIN,						//初级场赢5局
	AT_DAILY_ZHONGJI,						//中级场玩5局
	AT_DAILY_ZHONGJI_WIN,					//中级场赢5局
	AT_DAILY_GAOJI,							//高级场玩5局
	AT_DAILY_GAOJI_WIN,						//高级场赢5局
	AT_DAILY_ZHIZUN,						//至尊场玩5局
	AT_DAILY_ZHIZUN_WIN,					//至尊长赢5局
	AT_LIAN_WIN,							//连胜
	AT_MATCH_DUIZHAN = 1010,				//比赛场对战次数
	AT_MATCH_48BEISHU = 1014,				//比赛场胜利48倍以上
	AT_MAX
};

enum ACHIEVE_TYPE_H5
{
	ATH5_ROUND_CHUJI    = 111,  //斗地主或麻将初级场局数	//广告环境 新手
	ATH5_ROUND_ZHONGJI  = 112,  //斗地主或麻将中级场局数	//广告环境 初级
	ATH5_ROUND_GAOJI = 113,  //斗地主或麻将高级场局数		//广告环境 精英
	ATH5_ROUND_DASHI = 115,  //斗地主或麻将高级场局数		//广告环境 大师
	ATH5_ROUND_ZHIZUN = 116,  //斗地主或麻将高级场局数	//广告环境 至尊

	ATH5_DDZ_LORD_WIN = 38901,  //以地主身份赢得比赛
	ATH5_ERMJ_WIN = 39101,  //在二人麻将获胜
	ATH5_XXL_ROUND = 39301,  //消消乐局数
	ATH5_XXL_TOTALBET = 39302,  //消消乐押注
	ATH5_ZHUOQIU_ROUND = 40001,  //台球局数
	ATH5_ZHUOQIU_TOTALBET = 40002,  //台球押注
};

enum Task_Type
{
	T_WIN_ROUND=1,
};

enum SEND_CARD_TYPE
{
	COMMON_TYPE,
	COMMON_LAIZI_TYPE,
	SPECIFIC_TYPE,
	SPECIFIC_LAIZI_TYPE,
};

//V1.1
//enum ACHIEVE_TYPE
//{
//	AT_DUIZHAN = 1,
//	AT_CHUJISHENGLI,
//	AT_ZHONGJISHENGLI,
//	AT_LAIZISHENGLI,
//	AT_BISAISHENGLI,
//	AT_ZHONGJIBOMB,
//	AT_LAIZITASK,
//};

enum MAGIC_EMOJI_ERROR
{
	NO_SUPPORT = 1,			//不支持魔法表情
	NO_ENOUGH_MONEY = 2,	//没有足够的钱
	NO_EXIST = 3,			//该魔法表情不存在
	NO_SEND_SELF = 4,			//不能发给自己
	NO_ENOUGH_CARD =5,		// 魔法表情道具卡不足
};

enum BEI_SHU_INFO
{
	BEI_SHU_INFO_INIT,			// 初始倍数
	BEI_SHU_INFO_SHOWCARD,		// 明牌倍数
	BEI_SHU_INFO_ROBLORD,		// 抢地主倍数
	BEI_SHU_INFO_BOMB,			// 炸弹倍数
	BEI_SHU_INFO_SPRING,		// 春天倍数
	BEI_SHU_INFO_LEFTCARD,		// 剩牌倍数
	BEI_SHU_INFO_CALLSCORE,		// 叫分倍数 和抢地主公用
	BEI_SHU_INFO_END,			// 倍数种类
};

enum SHOW_CARD_TYPE
{
	SHOW_CARD_IN_READY = 1,		// 准备时明牌
	SHOW_CARD_IN_DEAL = 2,		// 发牌时明牌 
	SHOW_CARD_IN_PUTCARD = 3,	// 出牌时明牌 ，仅限地主第一次出牌
};

enum GET_REDPACKET_TYPE
{
	GET_REDPACKET_NORMAL = 0,	// 普通红包
	GET_REDPACKET_EXTRA = 1,	// 额外红包
	GET_REDPACKET_NEWBIE = 2,	// 新手红包
	GET_CHIP = 3,			// 获得碎片
	GET_WIN_DOUBEL = 4,			// 胜利翻倍
};

enum CHECK_REDPACKET_STATUS
{
	REDPACKET_STATUS_NONE = 0,	// 未生成
	REDPACKET_STATUS_READY = 1,	// 可领取
	REDPACKET_STATUS_SEND = 2,	// 发放成功
	REDPACKET_STATUS_EXTRA = 3,	// 可领取额外红包
};