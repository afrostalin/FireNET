// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "clientquerys.h"

#include "Core/tcpserver.h"
#include "Workers/Databases/dbworker.h"
#include "Tools/scripts.h"

ClientQuerys::ClientQuerys(QObject *parent) 
	: QObject(parent)
	, m_socket(nullptr)
	, m_Client(nullptr)
	, m_Connection(nullptr)
	, bAuthorizated(false)
	, bRegistered(false)
	, bProfileCreated(false)
{
}

ClientQuerys::~ClientQuerys()
{
	SAFE_DELETE(m_Client->profile);
}

void ClientQuerys::SetClient(SClient * client)
{
	m_Client = client;
	if (m_Client != nullptr)
	{
		m_Client->profile = new SProfile();
	}
}

bool ClientQuerys::UpdateProfile(SProfile* profile) const
{
	if (!gEnv->pDBWorker->pRedis && !gEnv->pDBWorker->pMySql)
	{
        LogError("Can't update profile, because no connected database");
        return false;
    }

	DBWorker* pDataBase = gEnv->pDBWorker;

    if (pDataBase->ProfileExists(profile->uid))
    {
		if (pDataBase->UpdateProfile(profile))
        {
			gEnv->pServer->UpdateClient(m_Client);
			return true;
        }
	    LogError("Profile can't be updated! Database return error!!!");
    }
	else
	{
		LogError("Profile can't be updated, because profile not found!!!");
	}

    return false;
}

SShopItem ClientQuerys::GetShopItemByName(const QString &name)
{
    SShopItem item;

	std::vector<SShopItem> m_shop = gEnv->pScripts->GetShop();

	if (m_shop.size() > 0)
	{
		for (auto it = m_shop.begin(); it != m_shop.end(); ++it)
		{
			if (it->name == name)
			{
				item.name = it->name;
				item.cost = it->cost;
				item.minLvl = it->minLvl;
				item.canBuy = it->canBuy;

				return item;
			}
		}
	}
	return item;
}