// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#include <stdarg.h>
#include <signal.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
using namespace std;

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
//LobbyServer\Protocol\WorldServer\global_define.h
#include "BD_Logger/BD_Logger.h"
#include "BD_NetModule/BD_Net.h"

#include "WorldServer/opcode.h"
#include "WorldServer/packet_define.h"
#include "WorldServer/global_define.h"

#include "LoginServer/opcode.h"
#include "LoginServer/packet_define.h"

#include "BaseGameServer/opcode.h"
#include "BaseGameServer/packet_define.h"

#include "BaseGameServer/IServerModule.h"
#include "BaseGameServer/game_define.h"

#include "../Common/opcode.h"
#include "../Common/packetdefine.h"



extern IBDLogger*		g_pLogger;
//extern IBDLogger*		g_pRobotLogger;
extern IBDNetModule*	g_pNetModule;
extern IBDListener*		g_pGSListener;
extern uint g_uiCurTime;
extern int g_nTableID;
extern int g_nLastID;
extern int g_debug_level_;

#if 1
#define SERVER_LOG(format, ...) \
	if (g_debug_level_ == 0) \
	{ \
	} \
	else if ((g_debug_level_&0x1) == 1) \
	{ \
		glog.log(format, ##__VA_ARGS__); \
	} \
	else if ((g_debug_level_&0x2) == 2) \
	{ \
		if(g_pLogger) \
		{ \
			g_pLogger->Log(format, ##__VA_ARGS__); \
		} \
	}
	
#endif


#define LOG_INFO(format, ...) \
    glog.log(format, ##__VA_ARGS__); \
    g_pLogger->Log(format, ##__VA_ARGS__);
