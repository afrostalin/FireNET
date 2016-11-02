// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "clientquerys.h"
#include <qdatastream.h>
#include "global.h"
#include "tcpserver.h"
#include "dbworker.h"

ClientQuerys::ClientQuerys(QObject *parent) : QObject(parent)
{
    startMoney = 10000;
	clientStatus = 0;
	m_socket = nullptr;

	clientProfile = new SProfile;
	clientProfile->uid = 0;
	clientProfile->achievements = "";
	clientProfile->friends = "";
	clientProfile->items = "";
	clientProfile->lvl = 0;
	clientProfile->model = "";
	clientProfile->money = 0;
	clientProfile->nickname = "";
	clientProfile->stats = "";
	clientProfile->xp = 0;
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

int ClientQuerys::GetUidBySocket(QSslSocket* socket)
{
	int uid = 0;
	QVector<SClient>::iterator it;
	for (it = vClients.begin(); it != vClients.end(); ++it)
	{
		if (it->socket == socket)
		{
			if (it->profile)
			{
				uid = it->profile->uid;
				break;
			}
		}
	}

    return uid;
}

bool ClientQuerys::UpdateProfile(QSslSocket* socket, SProfile* profile)
{
	if (!gEnv->pDataBases->pRedis && !gEnv->pDataBases->pMySql)
	{
        qCritical() << "Can't update profile, because no connected database";
        return false;
    }

	DBWorker* pDataBase = gEnv->pDataBases;

    if (pDataBase->ProfileExists(profile->uid))
    {
		if (pDataBase->UpdateProfile(profile))
        {
			AcceptProfileToGlobalList(socket, profile, clientStatus);
			return true;
        }
        else
            qCritical() << "Profile can't be updated! Database return error!!!";
    }
    else
        qCritical() << "Profile can't be updated, because profile not found!!!";

    return false;
}

// Deprecated funtion
SProfile* ClientQuerys::GetProfileFromString(QString &stringProfile)
{
    SProfile* profile = new SProfile;

    if (!stringProfile.isEmpty())
    {
        QXmlStreamReader xml(stringProfile);
        xml.readNext();
        while (!xml.atEnd() && !xml.hasError())
        {
            xml.readNext();

            if (xml.name() == "profile")
            {
                QXmlStreamAttributes attributes = xml.attributes();
                if (!attributes.isEmpty())
                {
                    profile->uid = attributes.value("id").toInt();
                    profile->nickname = attributes.value("nickname").toString();
                    profile->model = attributes.value("model").toString();
                    profile->money = attributes.value("money").toInt();
                    profile->xp = attributes.value("xp").toInt();
                    profile->lvl = attributes.value("lvl").toInt();
                }
            }

            if (xml.name() == "item")
            {
                QXmlStreamAttributes attributes = xml.attributes();
                if (!attributes.isEmpty())
                {
                    QString name = attributes.value("name").toString();
                    QString icon = attributes.value("icon").toString();
                    QString description = attributes.value("description").toString();

                    profile->items = profile->items + "<item name='" + name +
                        "' icon='" + icon +
                        "' description='" + description + "'/>";
                }
            }

            if (xml.name() == "friend")
            {

                QXmlStreamAttributes attributes = xml.attributes();
                if (!attributes.isEmpty())
                {
                    QString name = attributes.value("name").toString();
					QString uid = attributes.value("uid").toString();
					QString status = attributes.value("status").toString();

                    profile->friends = profile->friends + "<friend name='" + name + "' uid='" + uid + "' status='" + status + "'/>";
                }
            }

            if (xml.name() == "achievement")
            {

                QXmlStreamAttributes attributes = xml.attributes();
                if (!attributes.isEmpty())
                {
                    QString name = attributes.value("name").toString();
                    QString icon = attributes.value("icon").toString();
                    QString description = attributes.value("description").toString();

                    profile->achievements = profile->achievements + "<achievement name='" + name +
                        "' icon='" + icon +
                        "' description='" + description + "'/>";
                }
            }

            if (xml.name() == "stats")
            {
                QXmlStreamAttributes attributes = xml.attributes();
                if (!attributes.isEmpty())
                {
                    QString kills = attributes.value("kills").toString();
                    QString deaths = attributes.value("deaths").toString();
                    QString kd = attributes.value("kd").toString();

                    profile->stats = "<stats kills='" + kills +
                        "' deaths='" + deaths +
                        "' kd='" + kd + "'/>";
                }
            }
        }

		return profile;
    }
	else
	{
		qWarning() << "Can't get profile from string, because string profile empty!";
	}
    return nullptr;
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
        qWarning() << "Shop item not finded!!!";

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
		"<profile id='" + QString::number(profile->uid) +
		"' nickname='" + profile->nickname +
		"' model='" + profile->model +
		"' money='" + QString::number(profile->money) +
		"' xp='" + QString::number(profile->xp) +
		"' lvl='" + QString::number(profile->lvl) + "'/>"
		"<items>" + profile->items + "</items>"
		"<friends>" + profile->friends + "</friends>"
		"<achievements>" + profile->achievements + "</achievements>"
		"<stats>" + profile->stats + "</stats>"
		"</data>";

	return Profile;
}

void ClientQuerys::AcceptProfileToGlobalList(QSslSocket* socket, SProfile * profile, int status)
{
	if (socket == nullptr)
		return;

	QVector<SClient>::iterator it;
	for (it = vClients.begin(); it != vClients.end(); ++it)
	{
		if (it->socket == socket)
		{
			it->profile = profile;
			it->status = status;
			break;
		}
	}
}

QSslSocket * ClientQuerys::GetSocketByUid(int uid)
{
	QSslSocket* socket = nullptr;
	QVector<SClient>::iterator it;	
	for (it = vClients.begin(); it != vClients.end(); ++it)
	{
		if (it->profile != nullptr)
		{
			if (it->profile->uid == uid)
			{
				socket = it->socket;
				break;
			}
		}
	}

	return socket;
}
