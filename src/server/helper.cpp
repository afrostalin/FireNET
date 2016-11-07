// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "global.h"
#include "clientquerys.h"
#include "dbworker.h"
#include "tcpserver.h"
#include "settings.h"

#include <qdatastream.h>


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

QXmlStreamAttributes ClientQuerys::GetAttributesFromArray(QByteArray &bytes)
{
	QXmlStreamReader xml(bytes);
	QXmlStreamAttributes attributes;

	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "data")
		{
			return xml.attributes();
		}
	}

	return attributes;
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
    QFile file("scripts/shop.xml");
    QByteArray shop;

    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "Can't get shop.xml!!!";
        return item;
    }

    shop = file.readAll();

    QXmlStreamReader xml(shop);
    xml.readNext();

    while (!xml.atEnd() && !xml.hasError())
    {
        xml.readNext();

        if (xml.name() == "item")
        {
            QXmlStreamAttributes attributes = xml.attributes();
            if (!attributes.isEmpty())
            {
                QString shopItem = attributes.value("name").toString();

                if (shopItem == name)
                {
                    item.name = attributes.value("name").toString();
                    item.icon = attributes.value("icon").toString();
                    item.description = attributes.value("description").toString();
                    item.cost = attributes.value("cost").toInt();
                    item.minLnl = attributes.value("minLvl").toInt();
                    return item;
                }
            }
        }
    }

    if (item.name.isEmpty())
		qDebug() << "Shop item not finded!!!";

    return item;
}

bool ClientQuerys::CheckAttributeInRow(QString source, QString tag, QString attribute, QString checkAttribute)
{
    QXmlStreamReader xml(source);
    xml.readNext();
    while (!xml.atEnd() && !xml.hasError())
    {
        xml.readNext();

        if (xml.name() == tag)
        {
            QXmlStreamAttributes attributes = xml.attributes();
            if (!attributes.isEmpty())
            {
                if (checkAttribute == attributes.value(attribute).toString())
                    return true;
            }
        }
    }

    return false;
}

QString ClientQuerys::RemoveElementFromRow(QString source, QString element)
{
	QString complete = source.remove(element);
	return complete;
}

QString ClientQuerys::ProfileToString(SProfile * profile)
{
	if (profile == nullptr)
		return QString();

	QString Profile = "<data>"
		"<profile uid='" + QString::number(profile->uid) +
		"' nickname='" + profile->nickname +
		"' fileModel='" + profile->fileModel +
		"' lvl='" + QString::number(profile->lvl) +
		"' xp='" + QString::number(profile->xp) +
		"' money='" + QString::number(profile->money) + "' >"	
		"<items>" + profile->items + "</items>"
		"<friends>" + profile->friends + "</friends>"
		"</profile>"
		"</data>";

	return Profile;
}