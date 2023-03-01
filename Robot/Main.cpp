// ServerTemplate.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ConfigManager.h"
#include "UserSession.h"
#include "RobotManager.h"
#include "process_check.h"

IBDLogger*		g_pLogger = 0;
//IBDLogger*      g_pRobotLogger =0;
IBDNetModule*	g_pNetModule = 0;
IBDListener*	g_pGSListener = 0;
uint g_uiCurTime = 0;
int  g_nTableID = 0;
int  g_nLastID = 0;
bool g_bRunning = true;
bool g_dump_info = false;
void dump_robot_info(int)
{
    g_dump_info = true;
}


void exit_sigal(int)
{
	g_bRunning = false;
	return;
}

int main(int argc, char* argv[])
{
   	guid ply_guid_init_ = 0;
	guid robot_delegate_guid = 0;
	CUserSession* session = NULL;

	//转为服务进程
	if(argc > 1 && strcmp("-D", argv[1]) == 0)
	{
		daemon(1, 0);
	}

    process_check();    
	srand(time(0)+getpid());
	signal(SIGTERM, exit_sigal);
    if ( signal(SIGUSR1, dump_robot_info)== SIG_ERR)
    {
        assert(false);
    }

	modulefactory logfac("libbd_logger.so");
	modulefactory netfac("libbd_netmodule.so");

	uint uiLastTime = leaf::GetCurTime();
	uint uiElapse = 0;

	vector<CUserSession*>	vecUsers;

	if(!CConfigManager::GetInstancePtr()->Init("robotconfig.xml"))
	{
		goto FAILED;
	}

	//日志模块
	g_pLogger = logfac.createmodule<IBDLogger>("CreateLogger");
	if(g_pLogger == 0 || !g_pLogger->Init(0, "robot_log", 0))
	{
		SERVER_LOG("init logger failed!");
		goto FAILED;
	}

	////机器人数据日志模块;
	//g_pRobotLogger = logfac.createmodule<IBDLogger>("CreateLogger");
	//if(g_pRobotLogger == 0 || !g_pRobotLogger->Init(0, "robot_data_log", 0))
	//{
	//	SERVER_LOG("init robot logger failed!");
	//	goto FAILED;
	//}

	g_pNetModule = netfac.createmodule<IBDNetModule>("CreateNetModule");
	if(g_pNetModule == 0)
	{
		SERVER_LOG("init IBDNetModule failed!");
		goto FAILED;
	}

	robot_delegate_guid = CConfigManager::GetInstancePtr()->GetRobotAgent();
	session = new CUserSession();
	session->set_status(CUserSession::US_UNCONNECTED);
	session->SetConnectInfo(CConfigManager::GetInstancePtr()->GetGSListenIP(), CConfigManager::GetInstancePtr()->GetGSListenPort(), robot_delegate_guid);
	vecUsers.push_back(session);

	ply_guid_init_ = CConfigManager::GetInstancePtr()->GetRobotBeginID();
	for(guid i = ply_guid_init_; i < ply_guid_init_ + CConfigManager::GetInstancePtr()->GetPlyNum(); ++i)
	{
		CUserSession* pSession = CUserSession::createSession();
		if(!pSession)
		{
			SERVER_LOG("error robot_type value in robotconfig.xml at robot_type field!");
			goto FAILED;
		}
		pSession->set_status(CUserSession::US_UNCONNECTED);
		pSession->SetConnectInfo(CConfigManager::GetInstancePtr()->GetGSListenIP(), CConfigManager::GetInstancePtr()->GetGSListenPort(), i);
		vecUsers.push_back(pSession);

		//pSession->OnConnect();
	}

    LOG_INFO("Robot start success!");

	while(g_bRunning)
	{
		bool bIdle = true;
		if(g_pNetModule->Run()){			
			bIdle = false ;
			//SERVER_LOG("g_pNetModule->Run().uiElapse:%d", leaf::GetCurTime() - g_uiCurTime);
		}
		g_uiCurTime = leaf::GetCurTime();
		uiElapse = g_uiCurTime - uiLastTime;
		//SERVER_LOG("g_bRunning.uiElapse:%d", uiElapse);
		if(uiElapse > 500)
		{
			for(size_t i=0; i<vecUsers.size(); i++)
			{
				vecUsers[i]->OnLoop(uiElapse);
			}
			uiLastTime = g_uiCurTime;
			g_uiCurTime = leaf::GetCurTime();
			//SERVER_LOG("OnLoop().uiElapse:%d", g_uiCurTime - uiLastTime);
			
		}


        if (g_dump_info)
        {
            g_dump_info = false;
            for (size_t i = 0; i < vecUsers.size(); i++)
            {
                LOG_INFO("ply_guid:%lld status:%d ready:%d", vecUsers[i]->ply_guid(), vecUsers[i]->status(), vecUsers[i]->isready_);
            }
        }
		
		if(bIdle)
		{
			usleep(12000);
		}
	}

FAILED:

	BD_SAFE_RELEASE(g_pLogger);
	//BD_SAFE_RELEASE(g_pRobotLogger);
	BD_SAFE_RELEASE(g_pNetModule);
	return 0;
}

