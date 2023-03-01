/*
 CopyRight(c)2006 by Banding,Shanghai, All Right Reserved.

 @Date:	        2009/4/15
 @Created:	    14:59
 @Filename: 	main.cpp
 @File path:	BDLeyoyoV2\DDZV2\Server 
 @Author:		leiliang
 @Description:	程序的入口点，含打印服务器日志功能
*/

/*
积分结算规则：
1、地主获胜： 地主胜：当局倍数*2=所获积分 （192倍*2=384分）
农民输：当局倍数=所扣积分 （192倍=192分）
2、农民获胜： 
地主输：当局倍数*2=所扣积分 （192倍*2=384积分）
第1农民胜：当局倍数+（当局倍数÷10%）=所获积分 （192倍+（192÷10%）=211分）
第2农民胜：当局倍数=所获积分  （192倍=192分）
@Date:	        2012/9/3
*/

#include "stdafx.h"
#include "PlayerCreator.h"
#include "TableCreator.h"
#include "ConfigManager.h"
#include "Poke.h"
#include "sql_define.h"
#include <iostream>  
#include <iomanip>  
#include <fstream>
IBDLogger* g_pLogger = 0;
bool g_bRunning = true;
void exit_sigal(int)
{
	g_bRunning = false;
	return;
}

void print_vector_bool(vector<bool>& vec)
{
	const int buffer_size = 512;
	char buffer[buffer_size];
	int length = 0;
	length += snprintf(buffer + length, buffer_size - length, "{ ");
	for (size_t i = 0; i < vec.size(); i++)
	{
		length += snprintf(buffer + length, buffer_size - length, "%d, ", vec[i]);
	}
	length += snprintf(buffer + length, buffer_size - length, "}");
	glog.log(buffer);
}

void print_vector_int(vector<int>& vec)
{
	const int buffer_size = 512;
	char buffer[buffer_size];
	int length = 0;
	length += snprintf(buffer + length, buffer_size - length, "{ ");
	for (size_t i = 0; i < vec.size(); i++)
	{
		length += snprintf(buffer + length, buffer_size - length, "%d, ", vec[i]);
	}
	length += snprintf(buffer + length, buffer_size - length, "}");
	glog.log(buffer);
}

void print_vector_card(vector<CCard>& vec)
{
	const int buffer_size = 512;
	char buffer[buffer_size];
	int length = 0;
	length += snprintf(buffer + length, buffer_size - length, "{ ");
	for (size_t i = 0; i < vec.size(); i++)
	{
		length += snprintf(buffer + length, buffer_size - length, "%d%d, ", vec[i].m_nValue, vec[i].m_nColor);
	}
	length += snprintf(buffer + length, buffer_size - length, "}");
	glog.log(buffer);
}

//////////////////////////////////////////////////////////////////////////
//								测试代码								//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	//转为服务进程
	if(argc > 1 && strcmp("-D", argv[1]) == 0)
	{
		daemon(1, 0);

	}
	srand(time(0)+getpid());
	signal(SIGTERM, exit_sigal);
	IServerModule* pServerModule = NULL;
	
	modulefactory logfac("libbd_logger.so");
	modulefactory servermodulefac("libbase_game_server.so");
	//modulefactory logfac("atest.so");
	
	g_pLogger = logfac.createmodule<IBDLogger>("CreateLogger");
 	if(g_pLogger == 0 || !g_pLogger->Init(0, "DDZ_log", 0))
 	{
 		glog.log("init logger failed!");
 		goto FAILED;
 	}

	glog.log("load gameconfig file start");
	//装载配置文件
	if(!CConfigManager::GetInstancePtr()->Init("gameconfig.xml"))
	{
		glog.log("load gameconfig file failed!");
		goto FAILED;
	}

	pServerModule = servermodulefac.createmodule<IServerModule>("CreateServerModule");
	if(pServerModule == 0)
	{
		glog.log("load servermodule failed!");
		goto FAILED;
	}
	pServerModule->SetPlayerCreator(CPlayerCreator::GetInstancePtr());
	pServerModule->SetTableCreator(CTableCreator::GetInstancePtr());
	if(!pServerModule->InitServerModule())
	{
		glog.log("init servermodule failed!");
		goto FAILED;
	}

	g_pLogger->Log("DDZServer Version: 2013.4.8.2101");
	glog.log("DDZV2 start successfull !");

	while(g_bRunning)
	{
		if (!pServerModule->Run())
		{
			usleep(12000);
		}
	}
FAILED:
	glog.log("DDZV2 terminated !");
	BD_SAFE_RELEASE(g_pLogger);
	BD_SAFE_RELEASE(pServerModule);
}
