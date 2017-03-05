// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNet>
#include "Network/TcpClient.h"

#define TITLE "[FireNet-Core] "

class CNetworkThread;

// Global FireNet environment
struct SPluginEnv
{
	SPluginEnv()
	{
		pTcpClient = nullptr;
		pNetworkThread = nullptr;

		net_ip = nullptr;
		net_port = 0;
		net_timeout = 0;
		net_debug = 0;
	}

	// Pointers
	CTcpClient*                       pTcpClient;
	CNetworkThread*                   pNetworkThread;
	
	// Containers
	std::vector<IFireNetListener*>    m_Listeners;

	// CVars
	ICVar*                            net_ip;
	int                               net_port;
	int                               net_remote_port;
	int                               net_timeout;
	int                               net_debug;
};

extern SPluginEnv* mEnv;

namespace FireNet 
{
	// Send FireNet event with arguments
	static void SendFireNetEvent(EFireNetEvents event, SFireNetEventArgs& args = SFireNetEventArgs())
	{
		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnFireNetEvent(event, args);
		}
	}

	// Safe send TCP packet to master server
	static void SendPacket(CTcpPacket &packet)
	{
		if (mEnv->pNetworkThread && mEnv->pTcpClient && mEnv->pTcpClient->IsConnected())
			mEnv->pTcpClient->SendQuery(packet);
	}
}