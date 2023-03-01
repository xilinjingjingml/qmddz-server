#pragma once

struct structRedPacket
{
	int cLId;
	int nVipLimit;
	int nItemIndex;
	int nAmountMin;
	int nAmountMax;
	vector<int> nProbabily;
};

struct structRedPackeLimit
{
	int cLid;
	int nPayRate;
	int nReduceProbabily;
	int nPoolDeltaRate;
};

struct structRedPacket88Yuan
{
	int cLid;
	int nAmount;
	int nRoundLimit;
};

struct structRedPacketInfo
{
	int nRoundInterval;
	int nAmount;
	int nDoubel;
};

struct structRewardInfo
{
	int nItemId;
	int nItemNum;
	structRewardInfo() {
		nItemId = 0;
		nItemNum = 0;
	}
};

struct structRewardConfig
{
	int nItemId;
	int nItemNumMin;
	int nItemNumMax;
	int nWeight;
	int nConditionMin;
	int nConditionMax;
};

struct structRewardConfigList
{
	int nWeightSum;
	vector<structRewardConfig> vecRewardConfig;
};

struct structRewardItem
{
	int nItemId;
	int nItemNumMin;
	int nItemNumMax;
};

struct structRewardCondition
{
	int nRewardId;
	int nWeight;
	int nConditionMin;
	int nConditionMax;
};

struct structToggleCondition
{
	int nPercent;
	int nConditionMin;
	int nConditionMax;
};

struct structMustLoseCardConfig
{
	int nCardValueMax;
	int nCardValueMin;
	int nCardNum;
	int nCardLength;
};

typedef struct _MagicEmojiItem
{
	int nItemIndex;
	int nItemConsumeCoins;
	string nItemName;
	int nTenItemIndex;
	int nTenItemNum;
	int nTenEmojiNum;
	_MagicEmojiItem() :nItemIndex(0), nItemConsumeCoins(0), nItemName(""), nTenItemIndex(0), nTenItemNum(0), nTenEmojiNum(0) {}
}MagicEmojiItem;


class CConfigManager
{
public:
	enum
	{
		CARD_DB_NORMAL,
		CARD_DB_SPECIFIC,
		CARD_DB_LAIZI_NORMAL,
		CARD_DB_LAIZI_SPECIFIC
	};
public:
	SINGLETON(CConfigManager)
	CConfigManager(void);
	~CConfigManager(void);
	bool Init(const char* m_szConfigFile);

	int CalcCharCountInUTF8( const char * pszText, int maxCharNumInWidth );
	int CalcCharCountInWidth( const char * pszText );
	
	bool isRobotGuid(const char *guid);
	const string& GetString(const string& key) { return strings_[key]; }
	vector<TaskItem>	vecTask_;
	vector<int>			vecBroadCastTask_;	//广播任务的任务ID
	vector<string>		vecBroadCastTask_desc_;//广播任务的描述
	const MagicEmojiItem& GetMagicEmojiItem(int nItemIndex) { return mapMagicEmojiItems_[nItemIndex]; }
public:
	map<string, string>		strings_;
	vector<MagicEmojiItem> m_vecMagicEmojiConfigs;

	vector<int> m_vecRedPacketNewBieConfig;
	vector<int>	m_vecRedpacketConfigCount;			// 领取红包次数分配不同的概率 优先级低
	vector<int>	m_vecRedpacketConfigTotal;			// 领取红包总额分配不同的概率 优先级高
	vector<structRedPacket> m_vecRedPacketConfig;
	vector<structRedPackeLimit> m_vecRedPacketLimitConfig;
	vector<structRedPacket88Yuan> m_vecRedPacket88YuanConfig;
	vector<structRedPacketInfo> m_vecRedPacketNewBieInfo;

	int GetPlayNum(){ return m_nPlay_Num; };//获取牌桌人数
	int GetStarSkyScoreByDouble(int d);// 星空赛根据倍数得到输赢大小
	int GetStarSkyTaxByDouble(int d);// 星空赛根据输赢大小得到税收

	void getRedPacketInfoConfig();
	void getRedPacketInfoConfigForH5();
	void getRedPacketLimitConfig();
	void getRedPacketNewBieInfoConfig();
	void getRedPacket88YuanConfig();
	void getRedPacketInfoConfigCount();
	void getRedPacketNewBieInfo();
	void readRewardConfig();
	void getRewardNum(int nId, structRewardInfo& info);

	void readRewardItem();
	void readRewardCondition();
	void getRewardCondition(vector<structRewardInfo>& vecReward, vector<int>& vecIds, int condition = 0);

	void readToggleCondition();
	bool isToggleCondition(int nId, int condition, bool def = false);

	void readMustLoseCardConfig();
	void readMustLoseCardMaxConfig();
	vector< vector<structMustLoseCardConfig> > m_vecMustLoseCardConfigs;
	vector< vector<structMustLoseCardConfig> > m_vecMustLoseCardMaxConfigs;

private:
	int m_nPlay_Num;
	map<int, MagicEmojiItem> mapMagicEmojiItems_;
	map<int, int> m_mapStarSkyDouble;
	map<int, int> m_mapStarSkyTax;
	map<int, structRewardConfigList> m_mapRewardConfig;

	map<int, vector<structRewardItem> > m_mapRewardItem;
	map<int, vector<structRewardCondition> > m_mapRewardCondition;

	map<int, vector<structToggleCondition> > m_mapToggleCondition;
};

extern int g_nCallScore;	//是否叫分  0(叫地主,不叫）   1(1分，2分，3分）2(1分，2分，4分）
extern int g_nRobLord;		//是否抢地主
extern int g_nShowCard;		//是否明牌
extern int g_nBaseScore;	//底注,0:动态调整
extern int g_nHighMoney;	//判断是否为高分玩家的金币
extern int g_nHighScore;	//高分玩家的底注,0:动态调整
extern int g_nHighTaxMoney;	//高分玩家高税
extern int g_nTask;			//是否有任务 0 无任务 1 有任务
extern int g_nEscapeMode;	//逃跑扣分模式,0:扣给其他玩家,1:系统回收
extern int g_nAutoSave;		//自动存盘功能,0:不开启,1:开启
extern int g_nEscapeMoney;	//逃跑扣分,0:系统自动计算扣分
extern int g_nChangeTable;	//是否自动换桌,0:不换,1:换
extern int g_nAutoAddFan;	//本局没人叫地主,下局底注是否自动加翻,0:不开启,1:开启
extern int g_nLordWinScore;	//积分场，地主赢，获得额外游戏币奖励
extern int g_nFarmerWinScore;	//积分场，农民赢，获得额外游戏币奖励
extern int g_nDefaultLord;	//是否有默认地主，0:无，1:有
extern int g_nTID;///<增加任务的配置属性 -1 随机任务 > 0 就是要出现的任务ID
extern int g_nMission;		//奖励的种类 1 游戏币 2 元宝
extern int g_nTax;			//是否抽税
extern int g_nBroadcastWin;	//是否广播赢钱
extern int g_nBroadcastWinNum;	//广播赢钱的数字
extern int g_nBroadcastTask;	//是否广播任务
extern int g_nBaoPai;			//癞子场
extern int g_nCounts;			//记牌器
extern int g_nRace;				//比赛场
extern int g_nGameType;			//游戏场级别
extern int g_nZhaDanDef;		//炸弹产生概率 1~10
extern int g_nZhaDanNum;		//炸弹产生个数 2~5 
extern int g_nSaveNum;			//保存牌型	0不保存  1保存
extern int g_nSendCardNum;		//发牌数量	每次发牌数量   0默认发牌   1~17
extern int g_nDealCardNum;		//发牌数量	总共发牌数量   0默认发牌   1~17
extern int g_nSaveCardDouble;	//特殊牌型保存倍数	192倍
extern int g_nDynamic;			//动态底注
extern int g_nMaxMoney;			//输赢最大限制
extern int g_nLimitMoney;		//入场限制
extern int g_nLimitMoneyNext; //下个级别入场限制
extern int g_nSendCardCommon;	//普通发牌 0 非普通发牌 使用牌池   1 使用普通发牌  默认为0 
extern int g_debug_level; //0 不打印log， 1 打印log，2 输入log文本
extern int g_nNewShuffleCards; //是否使用新发牌规则
extern int g_nBombMultiple; // 炸弹导致的平均倍数(比10大的话 就会/10)=>CPoke::NewShuffleCards() GetRandomParams()
extern int g_nBetterSeat; // 优势座位号：0: 不调整, 1 : 低 2:中 3 :高 {0,10,40,70,100};
extern int g_nCardTidiness; // 手牌的整齐度 0: 不调整, 1 : 低 2:中 3 :高 {0,30,50,80,100};
extern int g_nBaseGood;   // 底牌关键张，针对先叫地主的座位手牌 0: 不调整, 1 : 低 2:中 3 :高 {0,30,50,80,100};
extern int g_nMatch; //比赛场 0：不是，1是
extern int g_nNewMatch; //新的比赛场 0：不是，1是

extern int g_nMinDouble; //倍数限制 0:不限制，>0限制
extern int g_nMaxDouble; //倍数限制 0:不限制，>0限制
extern int g_nLessThanZero;//can lose than zero
extern int g_nBaseLordCardLottery;
extern int g_nRecordPlayerCard;
extern int g_nBaoDiRate;//玩家三连抽金币是本金的倍数
extern int g_nBaoDiMoney;//玩家三连抽本金底数
extern int g_nTwoPai;//是否为二人斗地主场
extern int g_nTwoBaseRate;//二人斗地主场初始倍数
extern int g_nDoubleScore;//是否有加倍流程 1.依次加倍，2同时加倍
extern int g_nPrivateRoom;//是否是私人场
extern int g_nPauseTime;//暂停游戏时间
extern int g_nTimeOut;//超时次数
extern int g_nPlayCardTime;//出牌时间
extern int g_nTwoPaiMinLet;//最少让牌数目
extern int g_nTwoPaiLordLet;//地主让牌数目
extern int g_nTwoPaiLordDouble;//地主地主牌倍数开关
extern int g_nIdleTime;
extern int g_nFirstPlayCardTime;
extern int g_nIsNeedMagicEmoji;
extern int g_nIsStandard;			// 是否为标准版	0: 不是 ， 1: 是
extern int g_nIsStarSky;			// 是否为星空赛	0: 不是 ， 1: 是

extern int g_nHBPoolRate; //红包奖池从玩家桌费的抽成
extern int g_nHB88YuanPoolRate; //88元固定红包额抽成
extern int g_nHB88YuanEnable;	//开启88元模式
extern int g_nHBMode; //1普通模式（退出游戏会清空奖池），2高级模式（持续化用户奖池）
extern int g_nHBTaskRound; //红包任务局数
extern int g_nMinFan;		//最小番
extern int g_nMaxFan;		//最大番

extern int g_nRedPacketBCSTTHR;
extern string g_sServerName;
extern int g_nGame_id;

extern int g_nBetterSeatForNewBie;
extern int g_nBetterSeatForNewBieRound;
extern int g_nRedPacketLimit;

extern int g_nShowCardDoubleInReady;	//准备时明牌加倍
extern int g_nShowCardDoubleInDeal;		//发牌时明牌加倍
extern int g_nShowCardDoubleInPutCard;	//第一次出牌时明牌加倍
extern int g_nRPGMode;
extern int g_nLetCard;					// 让牌

extern vector<int> g_nRegainLoseMoneyLimit;
extern int g_nRegainLoseMAXCount;	//免输最大次数限制
extern int g_nRegainLoseCountLimit;	//免输单次次数限制
extern int g_nRegainLoseTimeLimit;	//免输时间限制
extern int g_nRegainLoseFreeCount;//老玩家免输次数
extern int g_nRegainLoseFreeCountNew;//新用户免输次数

extern int g_nInvincibleMode;	// 开局免输功能开关

extern int g_nRedPacketExtraLimit;		//额外红包次数
extern int g_nRedPacketExtraProbabily;	//额外红包触发概率

extern int g_nBaseInitDouble;	//初始倍数
extern int g_nBuXiPai;
extern int g_nCleverRobot;     // 聪明的机器人
extern int g_nLoggerHandCard;
extern int g_nDebugShowCard;
extern int g_nRedPacketsNewbie;  //新人前几局红包
extern int g_nRedPacketsNewbieAddRound;  //新人前几局红包不领取添加局数
extern int g_nChipFall;			//出炸弹碎片掉落 开关
extern int g_nChipLegendMax;	//出炸弹碎片掉落 传奇总的最大值
extern int g_nChipIndex;			//出炸弹碎片掉落 奖励index
extern int g_nChipIndexNewbie1;		//出炸弹碎片掉落 新手奖励index
extern int g_nChipIndexNewbie2;		//出炸弹碎片掉落 新手奖励index
extern int g_nChipIndexNewbie3;		//出炸弹碎片掉落 新手奖励index
extern int g_nChipNewbieRound1;		//出炸弹碎片掉落 新手前n1局
extern int g_nChipNewbieRound2;		//出炸弹碎片掉落 新手前n2局
extern int g_nChipNewbieRound3;		//出炸弹碎片掉落 新手前n3局
extern int g_nRedpakcetControllerRatio; //抽红包红包调整开关
extern int g_nWinDoubleCan; //是否支持胜利翻倍
extern int g_nWinDoubleAddAmount; //胜利翻倍额外赠送金额
extern int g_nWinDoubleAddProbabily; //胜利翻倍额外赠送几率
extern int g_nWinDoubleMaxAmount; //胜利翻倍额外赠送最大值
extern int g_nWinDoubleMaxDouble; //胜利翻倍额外赠送最大倍数
extern int g_nShuffleCardsForNewBieFileABTest; //新手洗牌读取文件abtest
extern int g_nShuffleCardsForNewBieFileRound;  //新手洗牌读取文件局数限制
extern int g_nShuffleCardsForNewBieFileOrder;		//新手洗牌读取顺序文件开关
extern int g_nShuffleCardsForNewBieFileOrderRound;	//新手洗牌读取顺序文件局数限制
extern int g_nShuffleCardsForNewBieFileOrderMoney;	//新手洗牌读取顺序文件提现道具数量限制
extern int g_nIsBaiYuan;				//百元赛开关
extern double g_dBaiYuanBombDouble;		//百元赛炸弹倍数
extern int g_nBaiYuanHBRound;			//百元赛红包局数
extern int g_nBaiYuanHBAwardStartId;	//百元赛局数红包奖励起始id
extern int g_nBaiYuanHBAwardEndId;		//百元赛局数红包奖励结束id
extern int g_nBaiYuanWinDoubleMin;		//百元赛赢分翻倍最小值 单位*100
extern int g_nBaiYuanWinDoubleMax;		//百元赛赢分翻倍最大值 单位*100
extern int g_nBaiYuanBankruptcyMin;		//百元赛破产补助最小值
extern int g_nBaiYuanBankruptcyMax;		//百元赛破产补助最大值
extern int g_nBaiYuanMustLostRound;		//百元赛必杀局
extern int g_nBaiYuanMustLostMaxMoney;	//百元赛必杀局道具数量
