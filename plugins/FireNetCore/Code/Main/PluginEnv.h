// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNet-Core>
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

	// Send FireNet event with arguments
	inline void SendFireNetEvent(EFireNetEvents event, SFireNetEventArgs& args = SFireNetEventArgs())
	{
		for (auto it = m_Listeners.begin(); it != m_Listeners.end(); ++it)
		{
			(*it)->OnFireNetEvent(event, args);
		}
	}
	
	// Safe send TCP packet to master server
	inline void SendPacket(CTcpPacket &packet)
	{
		if (pNetworkThread && pTcpClient && pTcpClient->IsConnected())
		{
			pTcpClient->SendQuery(packet);
		}
	}
};

extern SPluginEnv* mEnv;