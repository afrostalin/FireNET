// Copyright (Ñ) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "scripts.h"
#include "global.h"

#include <QXmlStreamReader>
#include <QFile>

Scripts::Scripts(QObject *parent) : QObject(parent)
{

}

void Scripts::Clear()
{
	// TODO
}

void Scripts::LoadShopScript()
{
	qDebug() << "Loading shop...";

	QFile shop("scripts/shop.xml");

	if (!shop.open(QIODevice::ReadOnly))
	{
		qCritical() << "Can't get shop.xml!!!";
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
				item.minLnl = minLvl;
				item.canBuy = canBuy;

				m_shop.push_back(item);

				qDebug() << "Adding <" << name << "> item to shop";
			}
		}
	}

	qDebug() << "Loaded" << m_shop.size() << "shop items.";
}

void Scripts::LoadTrustedServerList()
{
	qDebug() << "Loading trusted server list...";

	QFile serverList("scripts/server_list.xml");

	if (!serverList.open(QIODevice::ReadOnly))
	{
		qCritical() << "Can't get server_list.xml!!!";
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

				qDebug() << "Adding <" << list_name << "> server to trusted list";
			}
		}
	}

	qDebug() << "Loaded" << m_trustedServers.size() << "trusted servers.";
}

QVector<SShopItem> Scripts::GetShop()
{
	return m_shop;
}

QVector<STrustedServer> Scripts::GetTrustedList()
{
	return m_trustedServers;
}
