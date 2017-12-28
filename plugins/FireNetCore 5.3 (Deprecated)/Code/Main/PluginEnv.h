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
	SPluginEnv() : pTcpClient(nullptr)
		, pNetworkThread(nullptr)
		, net_master_ip(nullptr)
		, net_version(nullptr)
		, net_master_port(0)
		, net_master_timeout(0)
		, net_answer_timeout(0)
		, net_auto_connect(0)
#ifndef NDEBUG
		, net_debug(0)
#endif
		, net_LogLevel(0)
	{
	}

	// Pointers
	CTcpClient*                       pTcpClient;
	CNetworkThread*                   pNetworkThread;
	
	// Listeners container
	std::vector<IFireNetListener*>    m_Listeners;

	// CVars - Master server
	ICVar*                            net_master_ip;
	ICVar*                            net_version;
	int                               net_master_port;
	int                               net_master_timeout;
	int                               net_answer_timeout;
	int                               net_auto_connect;

	// CVars - Other
#ifndef NDEBUG
	int                               net_debug;
#endif
	int                               net_LogLevel;
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