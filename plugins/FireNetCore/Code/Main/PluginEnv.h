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

		net_master_ip = nullptr;
		net_master_port = 0;
		net_master_remote_port = 0;
		net_master_timeout = 0;

		net_game_server_ip = nullptr;
		net_game_server_map = nullptr;
		net_game_server_gamerules = nullptr;
		net_game_server_port = 0;
	    net_game_server_timeout = 0;
		net_game_server_max_players = 0;

		net_debug = 0;
	}

	// Pointers
	CTcpClient*                       pTcpClient;
	CNetworkThread*                   pNetworkThread;
	
	// Listeners container
	std::vector<IFireNetListener*>    m_Listeners;

	// CVars - Master server
	ICVar*                            net_master_ip;
	int                               net_master_port;
	int                               net_master_remote_port;
	int                               net_master_timeout;
	// CVars - Game server
	ICVar*                            net_game_server_ip;
	ICVar*                            net_game_server_map;
	ICVar*                            net_game_server_gamerules;
	int                               net_game_server_port;
	int                               net_game_server_timeout;
	int                               net_game_server_max_players;
	// CVars - Other
	int                               net_debug;
};

extern SPluginEnv* mEnv;

namespace FireNet 
{
	//! Send FireNet event with arguments
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