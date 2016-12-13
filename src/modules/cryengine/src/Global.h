// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef _ModuleGlobal_
#define _ModuleGlobal_

#include "CVars.h"
#include "UIEvents.h"
#include "NetWorker.h"
#include "NetworkThread.h"
#include "../includes/FireNET_Base.h"

#define TITLE "[FireNET] "

struct SFriend
{
	string nickname;
	int uid;
	int status;
};

struct SItem
{
	string name;
	string icon;
	string description;
};

struct SShopItem
{
	string name;
	string icon;
	string description;
	int cost;
	int minLvl;
};

struct SModuleEnv
{
	SModuleEnv()
	{
		m_firenet_ip = "127.0.0.1";
		m_firenet_port = 3322;
		m_magic_key = 2016207; // You need update this key on server and client!
		m_timeout = 3.0f;

		bConnected = false;

		pCVars = new CModuleCVars;
		pNetWorker = new CNetWorker;
		pNetwork = nullptr;

#ifndef DEDICATED_SERVER
		pUIEvents = new CModuleUIEvents;
		m_profile = nullptr;
#endif
	}

	CModuleCVars* pCVars;
	CNetWorker* pNetWorker;
	CNetwork* pNetwork;

#ifdef DEDICATED_SERVER
	std::vector<FireNET::SProfile> m_Profiles;
#else
	CModuleUIEvents* pUIEvents;
	FireNET::SProfile* m_profile;

	std::vector<SItem> m_items;
	std::vector<SFriend> m_friends;
	std::vector<SShopItem> m_shopItems;
#endif

	string m_firenet_ip;
	int m_firenet_port;
	int m_magic_key;
	float m_timeout;

	bool bConnected;
	bool bHaveNewResult;
};

extern SModuleEnv* gModuleEnv;

#endif