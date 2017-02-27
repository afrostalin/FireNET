// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#define TITLE "[FireNet-Server] "

class CUdpServer;
class CNetworkThread;
class CGameStateSynchronization;

struct SFireNetProfile;

struct SPluginEnv
{
	SPluginEnv()
	{
		pNetworkThread = nullptr;
		pUdpServer = nullptr;
		pGameSync = nullptr;

		net_ip = nullptr;
		net_port = 0;
		net_timeout = 0;
		net_max_players = 0;
	}

	//! Pointers
	CUdpServer*                pUdpServer;
	CGameStateSynchronization* pGameSync;
	CNetworkThread*            pNetworkThread;

	//! CVars
	ICVar*                     net_ip;
	int                        net_port;
	int                        net_timeout;
	int                        net_max_players;
};

extern SPluginEnv* mEnv;