// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNet>
#include "Network/TcpClient.h"

#define TITLE "[FireNet-Core] "

class CNetworkThread;

// Global FireNet environment
struct SPluginEnv
{
	// Pointers
	CTcpClient*                       pTcpClient = nullptr;
	CNetworkThread*                   pNetworkThread = nullptr;
	
	// Listeners container
	std::vector<IFireNetListener*>    m_Listeners;

	// CVars - Master server
	ICVar*                            net_master_ip = nullptr;
	ICVar*                            net_version = nullptr;
	int                               net_master_port = 0;
	int                               net_master_timeout = 0;
	int                               net_answer_timeout = 0;
	int                               net_auto_connect = 0;

	// CVars - Other
#ifndef NDEBUG
	int                               net_debug = 0;
#endif
	int                               net_LogLevel = 0;
};

extern SPluginEnv* mEnv;

#define FireNetLog(...) do { if (mEnv->net_LogLevel > 0) { CryLog (__VA_ARGS__); } } while(0)
#define FireNetLogDebug(...) do { if (mEnv->net_LogLevel > 1) { CryLog (__VA_ARGS__); } } while(0)
#define FireNetLogAlways(...) do { CryLogAlways (__VA_ARGS__); } while(0)

namespace FireNet 
{
	//! Send FireNet event with arguments
	static void SendEmptyEvent(EFireNetEvents event)
	{
		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnEmptyEvent(event);
		}
	}

	//! Safe send TCP packet to master server
	static void SendPacket(CTcpPacket &packet)
	{
		if (mEnv->pNetworkThread && mEnv->pTcpClient && mEnv->pTcpClient->IsConnected())
			mEnv->pTcpClient->SendQuery(packet);
	}
}