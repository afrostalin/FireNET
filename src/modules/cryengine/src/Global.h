// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef _ModuleGlobal_
#define _ModuleGlobal_


#include "CVars.h"
#include "UIEvents.h"
#include "XmlWorker.h"
#include "Network.h"

#define TITLE "[FireNET] "

struct SFriend
{
	QString nickname;
	int uid;
	int status;
};

struct SItem
{
	QString name;
	QString icon;
	QString description;
};

struct SShopItem
{
	QString name;
	QString icon;
	QString description;
	int cost;
	int minLvl;
};

struct SProfile
{
	int uid;
	QString nickname;
	QString model;
	int lvl;
	int xp;
	int money;
	QString items;
	QString friends;
	QString achievements;
	QString stats;
};

struct SGlobalCryModule
{
	CModuleCVars* pCVars;
	CModuleUIEvents* pUIEvents;
	CXmlWorker* pXmlWorker;
	CNetwork* pNetwork;
	SProfile* m_profile;

	QVector<SItem> m_items;
	QVector<SFriend> m_friends;
	QVector<SShopItem> m_shopItems;

	string online_ms_address;
	int online_ms_port;
	float online_ms_timeout;

	bool bConnected;

	inline void Init()
	{
		gEnv->pLog->Log(TITLE "Global environment Init()");
		// Strings
		online_ms_address = "127.0.0.1";
		// Ints
		online_ms_port = 3322;
		// Floats
		online_ms_timeout = 3.0f;
		// Booleans
		bConnected = false;
		// Pointers
		pCVars = new CModuleCVars;
		pUIEvents = new CModuleUIEvents;
		pXmlWorker = new CXmlWorker;
		m_profile = nullptr;
		pNetwork = nullptr;
	}

};

extern SGlobalCryModule* gCryModule;

#endif