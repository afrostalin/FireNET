// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "scripts.h"
#include "global.h"
#include "console.h"

#include <QXmlStreamReader>
#include <QFile>

Scripts::Scripts(QObject *parent) 
	: QObject(parent)
{
}

Scripts::~Scripts()
{
	Clear();
}

void Scripts::Clear()
{
	m_shop.clear();
	m_trustedServers.clear();
	m_GameMaps.clear();
}

void Scripts::ReloadAll()
{
	Clear();

	LogInfo("Reloading all scripts ...");

	if (gEnv->pConsole->GetBool("bUseScriptsSystem"))
	{
		LoadShopScript();
		LoadTrustedServerList();
	}
	
	LoadGameMaps();
}

void Scripts::LoadShopScript()
{
	LogDebug("Loading shop...");

	QFile shop("scripts/shop.xml");

	if (!shop.open(QIODevice::ReadOnly))
	{
		LogError("Can't get shop.xml!!!");
		return;
	}

	QByteArray data = shop.readAll();
	shop.close();
	shop.deleteLater();
	QXmlStreamReader xml(data);

	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "item")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			QString name = attributes.value("name").toString();
			int cost = attributes.value("cost").toInt();
			int minLvl = attributes.value("minLvl").toInt();
			bool canBuy = attributes.value("canBuy").toInt() == 1 ? true : false;

			if (!name.isEmpty())
			{
				SShopItem item;
				item.name = name;
				item.cost = cost;
				item.minLvl = minLvl;
				item.canBuy = canBuy;

				m_shop.push_back(item);

				LogDebug("Adding <%s> item to shop", item.name.toStdString().c_str());
			}
		}
	}

	LogDebug("Loaded <%d> shop items.", m_shop.size());
}

void Scripts::LoadTrustedServerList()
{
	LogDebug("Loading trusted server list...");

	QFile serverList("scripts/server_list.xml");

	if (!serverList.open(QIODevice::ReadOnly))
	{
		LogError("Can't get server_list.xml!!!");
		return;
	}

	QByteArray data = serverList.readAll();
	serverList.close();
	serverList.deleteLater();
	QXmlStreamReader xml(data);

	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "Server")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			QString list_name = attributes.value("name").toString();
			QString list_ip = attributes.value("ip").toString();
			int list_port = attributes.value("port").toInt();

			if (!list_name.isEmpty() && !list_ip.isEmpty() && list_port > 0)
			{
				STrustedServer m_server;
				m_server.name = list_name;
				m_server.ip = list_ip;
				m_server.port = list_port;

				m_trustedServers.push_back(m_server);

				LogDebug("Adding <%s> server to trusted list", m_server.name.toStdString().c_str());
			}
		}
	}

	qDebug() << "Loaded" << m_trustedServers.size() << "trusted servers.";
}

void Scripts::LoadGameMaps()
{
	LogDebug("Loading game maps...");

	QFile maps("scripts/game_maps.xml");

	if (!maps.open(QIODevice::ReadOnly))
	{
		LogError("Can't get game_maps.xml!!!");
		return;
	}

	QByteArray data = maps.readAll();
	maps.close();
	maps.deleteLater();
	QXmlStreamReader xml(data);

	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "map")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			QString name = attributes.value("name").toString();
			QString gamerules = attributes.value("gamerules").toString();
			int id = attributes.value("id").toInt();

			if (!name.isEmpty())
			{
				SGameMap map;
				map.mapName = name;
				map.mapID = id;
				map.gamerules = gamerules;

				m_GameMaps.push_back(map);

				LogDebug("Adding <%s> game map to list", name.toStdString().c_str());
			}
		}
	}

	LogDebug("Loaded <%d> game maps.", m_GameMaps.size());
}

std::vector<SShopItem> Scripts::GetShop() const
{
	return m_shop;
}

std::vector<STrustedServer> Scripts::GetTrustedList() const
{
	return m_trustedServers;
}

std::vector<SGameMap> Scripts::GetGameMaps() const
{
	return m_GameMaps;
}
