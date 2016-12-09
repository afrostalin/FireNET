// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

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
			int id = attributes.value("id").toInt();
			QString name = attributes.value("name").toString();
			QString icon = attributes.value("icon").toString();
			QString description = attributes.value("description").toString();
			int cost = attributes.value("cost").toInt();
			int minLvl = attributes.value("minLvl").toInt();

			if (id > 0 && !name.isEmpty() && !icon.isEmpty() && !description.isEmpty())
			{
				SShopItem item;
				item.id = id;
				item.name = name;
				item.icon = icon;
				item.description = description;
				item.cost = cost;
				item.minLnl = minLvl;

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
