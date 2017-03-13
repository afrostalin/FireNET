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
	}

	//! Pointers
	CUdpServer*                pUdpServer;
	CGameStateSynchronization* pGameSync;
	CNetworkThread*            pNetworkThread;
};

extern SPluginEnv* mEnv;