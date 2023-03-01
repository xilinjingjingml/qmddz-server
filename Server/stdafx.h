// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#include <signal.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <math.h>
#include <iostream>
#include <algorithm>
using namespace std;

// #define CHUJI_CHANG 20
// #define ZHONGJI_CHANG 50
// #define GAOJI_CHANG  100
// #define GUIBIN_CHANG 500
// #define ZHIZUN_CHANG 1000

// use gold leaf itemid 364
#define HONGBAO_CHUJI 1
#define HONGBAO_GAOJI 2

// use gamemoney itemid 0

enum HONGBAO_CHANGCI
{
	HONGBAO_H5_CHUJI = 21,	//广告 新手
	HONGBAO_H5_ZHONGJI,		//广告 初级
	HONGBAO_H5_GAOJI,		//广告 精英
	HONGBAO_H5_SKIP,		//跳过 //因为任务id 114被每日登入使用
	HONGBAO_H5_DASHI,		//广告 大师
	HONGBAO_H5_ZHIZUN,		//广告 至尊
	HONGBAO_H5_END
};


#include "BD_BaseHelper/include/modulefactory.h"
#include "BD_BaseHelper/include/globaldefine.h"
#include "BD_BaseHelper/include/netstream.h"
#include "BD_BaseHelper/include/defpacketdefine.h"
#include "BD_BaseHelper/include/log.h"
#include "BD_BaseHelper/include/ex_table.h"
#include "BD_BaseHelper/tinyxml/tinystr.h"
#include "BD_BaseHelper/tinyxml/tinyxml.h"
#include "BD_BaseHelper/include/objectpool.h"
#include "BD_BaseHelper/include/utilty.h"
#include "BD_BaseHelper/sqlite/sqlite3.h"
#include "BD_BaseHelper/sqlite/CppSQLite3.h"
#include "BD_BaseHelper/sqlite/sqlite3ext.h"
//LobbyServer\Protocol\WorldServer\global_define.h
#include "BD_Logger/BD_Logger.h"


#include "WorldServer/opcode.h"
#include "WorldServer/packet_define.h"
#include "WorldServer/global_define.h"

#include "BaseGameServer/opcode.h"
#include "BaseGameServer/packet_define.h"



#include "BaseGameServer/IServerModule.h"
#include "BaseGameServer/game_define.h"


#include "../Common/opcode.h"
#include "../Common/packetdefine.h"
#include "../Common/PlayCard.h"
#include "../Common/Rule.h"
#include "../Common/Task.h"
#include "../gen_src/packet_define.pb.h"
#include "../gen_src/common.inl"


#include "gen_src/basegameserver.pb.h"
#include "gen_src/basegameserver.inl"

#include "gen_src/worldserver.pb.h"
#include "gen_src/worldserver.inl"

extern IBDLogger*		g_pLogger;
extern int g_debug_level; //0 不打印log， 1 打印log，2 输入log文本


#if 1
#define SERVER_LOG(format, ...) \
if (g_debug_level == 0) \
{ \
} \
else if (g_debug_level == 1) \
{ \
	glog.log(format, ##__VA_ARGS__); \
} \
else if (g_debug_level == 2) \
{ \
	if(g_pLogger) \
	{ \
		g_pLogger->Log(format, ##__VA_ARGS__); \
	} \
}

#endif