// Copyright (Ñ) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "clientquerys.h"
#include "dbworker.h"
#include "tcpserver.h"
#include "settings.h"
#include "scripts.h"

ClientQuerys::ClientQuerys(QObject *parent) : QObject(parent)
{
    startMoney = 10000;
	m_socket = nullptr;
	bAuthorizated = false;
	bRegistered = false;
	bProfileCreated = false;
}

void ClientQuerys::SetClient(SClient * client)
{
	m_Client = client;
	m_Client->profile = new SProfile;
	m_Client->profile->uid = 0;
	m_Client->profile->nickname = "";
	m_Client->profile->fileModel = "";
	m_Client->profile->lvl = 0;
	m_Client->profile->xp = 0;
	m_Client->profile->money = 0;
	m_Client->profile->items = "";
	m_Client->profile->friends = "";
}

bool ClientQuerys::UpdateProfile(SProfile* profile)
{
	if (!gEnv->pDBWorker->pRedis && !gEnv->pDBWorker->pMySql)
	{
        qCritical() << "Can't update profile, because no connected database";
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
        else
            qCritical() << "Profile can't be updated! Database return error!!!";
    }
    else
        qCritical() << "Profile can't be updated, because profile not found!!!";

    return false;
}

SShopItem ClientQuerys::GetShopItemByName(QString name)
{
    SShopItem item;

	QVector<SShopItem> m_shop = gEnv->pScripts->GetShop();

	if (m_shop.size() > 0)
	{
		for (auto it = m_shop.begin(); it != m_shop.end(); ++it)
		{
			if (it->name == name)
			{
				item.name = it->name;
				item.cost = it->cost;
				item.minLnl = it->minLnl;
				item.canBuy = it->canBuy;

				return item;
			}
		}
	}
	return item;
}