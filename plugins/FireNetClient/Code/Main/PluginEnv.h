// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#define TITLE "[FireNet-Client] "

class CGameStateSynchronization;
class CUdpClient;
class CNetworkThread;

struct SFireNetProfile;

struct SPluginEnv
{
	SPluginEnv()
	{
		pNetworkThread = nullptr;
		pUdpClient = nullptr;
		pGameSync = nullptr;

		m_FireNetProfile = nullptr;
	}

	//! Pointers
	CUdpClient*                pUdpClient;
	CGameStateSynchronization* pGameSync;
	CNetworkThread*            pNetworkThread;

	//! Local FireNet profile
	SFireNetProfile*           m_FireNetProfile;
};

extern SPluginEnv* mEnv;