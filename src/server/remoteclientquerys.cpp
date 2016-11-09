// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "global.h"
#include "remoteclientquerys.h"
#include "clientquerys.h"
#include "tcpserver.h"
#include "remoteserver.h"
#include "dbworker.h"
#include "settings.h"

#include <QDebug>
#include <QCoreApplication>

RemoteClientQuerys::RemoteClientQuerys(QObject *parent) : QObject(parent)
{
	m_socket = nullptr;
}

RemoteClientQuerys::~RemoteClientQuerys()
{
}

void RemoteClientQuerys::SetClient(SRemoteClient * client)
{
	m_client = client;
	m_client->server = new SGameServer;

	m_client->server->name = "";
	m_client->server->ip = "";
	m_client->server->port = 0;
	m_client->server->map = "";
	m_client->server->gamerules = "";
	m_client->server->online = 0;
	m_client->server->maxPlayers = 0;
}

void RemoteClientQuerys::onAdminLogining(QByteArray & bytes)
{
	qWarning() << "Client (" << m_socket << ") trying login in administrator mode!";

	if (gEnv->pRemoteServer->bHaveAdmin)
	{
		qCritical() << "Error authorization in administator mode! Reason = Administrator alredy has entered";

		QByteArray msg;
		msg.append("Error authorization in administator mode! Reason = Administrator alredy has entered.");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);
		return;
	}

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString login = attributes.value("login").toString();
	QString password = attributes.value("password").toString();

	if (login.isEmpty() || password.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
		qDebug() << "Login = " << login << "Password = " << password;

		QByteArray msg;
		msg.append("Wrong packet data! Some values empty!");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);

		return;
	}

	if (login == gEnv->pSettings->GetVariable("sv_root_user").toString())
	{
		if (password == gEnv->pSettings->GetVariable("sv_root_password").toString())
		{
			qWarning() << "Client (" << m_socket << ") success login in administator mode!";
			gEnv->pRemoteServer->bHaveAdmin = true;

			m_client->isAdmin = true;
			gEnv->pRemoteServer->UpdateClient(m_client);

			QByteArray msg;
			msg.append("You success login in administator mode!");
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);

			return;
		}
		else
		{
			qCritical() << "Error authorization in administator mode! Reason = Wrong password. Password = " << password;

			QByteArray msg;
			msg.append("Error authorization in administator mode! Reason = Wrong password.");
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);

			return;
		}

	}
	else
	{
		qCritical() << "Error authorization in administator mode! Reason = Wrong login. Login = " << login;

		QByteArray msg;
		msg.append("Error authorization in administator mode! Reason = Wrong login.");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);

		return;
	}
}

void RemoteClientQuerys::onConsoleCommandRecived(QByteArray & bytes)
{
	if (!m_client->isAdmin)
	{
		qCritical() << "Only administrator can use console commands!";

		QByteArray msg;
		msg.append("Only administrator can use console commands!");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);

		return;
	}

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString command = attributes.value("command").toString();
	QString value = attributes.value("value").toString();

	if (command == "status")
	{
		int gameServersCount = 0;
		gEnv->pRemoteServer->bHaveAdmin ? gameServersCount = gEnv->pRemoteServer->GetClientCount() - 1 : gameServersCount = gEnv->pRemoteServer->GetClientCount();

		QByteArray msg;
		msg.append("***Server status***\n");
		msg.append("Server version = " + qApp->applicationVersion() + "\n");
		msg.append("Server ip = " + gEnv->pSettings->GetVariable("sv_ip").toString() + "\n");
		msg.append("Server port = " + gEnv->pSettings->GetVariable("sv_port").toString() + "\n");
		msg.append("Server tickrate = " + gEnv->pSettings->GetVariable("sv_tickrate").toString() + " per/sec.\n");
		msg.append("Database mode = " + gEnv->pSettings->GetVariable("db_mode").toString() + "\n");
		msg.append("Authrorization type = " + gEnv->pSettings->GetVariable("auth_mode").toString() + "\n");
		msg.append("Players ammount = " + QString::number(gEnv->pServer->GetClientCount()) + ". Maximum players count = " + gEnv->pSettings->GetVariable("sv_max_players").toString() + "\n");
		msg.append("Game servers ammount = " + QString::number(gameServersCount) + ". Maximum game servers count = " + gEnv->pSettings->GetVariable("sv_max_servers").toString() + "\n");
		msg.append("*******************");

		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);

		return;
	}

	if (command == "send_global_message")
	{
		QByteArray msg;
		msg.append("<server><data type='message' value='" + value + "'/></server>");
		gEnv->pServer->sendGlobalMessage(msg);

		QByteArray answer("Message sended.");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, answer);

		return;
	}

	if (command == "send_console_command")
	{
		QByteArray msg;
		msg.append("<server><data type='command' value='" + value + "'/></server>");
		gEnv->pServer->sendGlobalMessage(msg);

		QByteArray answer("Console command sended.");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, answer);

		return;
	}

	if (command == "players")
	{
		QStringList players = gEnv->pServer->GetPlayersList();
		QByteArray answer;
		answer.clear();

		for (int i = 0; i < players.size(); i++)
		{
			answer.append(players[i] + "\n");
		}

		if (!players.isEmpty() && !answer.isEmpty())
		{
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, answer);
		}
		else
		{
			answer.append("Player list empty.");
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, answer);
		}

		return;
	}

	if (command == "servers")
	{
		QStringList servers = gEnv->pRemoteServer->GetServerList();
		QByteArray answer;
		answer.clear();

		for (int i = 0; i < servers.size(); i++)
		{
			answer.append(servers[i] + "\n");
		}

		if (!servers.isEmpty() && !answer.isEmpty())
		{
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, answer);
		}
		else
		{
			answer.append("Server list empty.");
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, answer);
		}

		return;
	}

	QByteArray msg;
	msg.append("Command <" + command + "> not found!");
	gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);
}

void RemoteClientQuerys::onGameServerRegister(QByteArray & bytes)
{
	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString serverName = attributes.value("name").toString();
	QString serverIp = attributes.value("ip").toString();
	int serverPort = attributes.value("port").toInt();
	QString mapName = attributes.value("map").toString();
	QString gamerules = attributes.value("gamerules").toString();
	int online = attributes.value("online").toInt();
	int maxPlayers = attributes.value("maxPlayers").toInt();

	if (serverName.isEmpty() || serverIp.isEmpty() || mapName.isEmpty() || gamerules.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
		qDebug() << "ServerName = " << serverName << "ServerIp = " << serverIp << "MapName = " << mapName << "Gamerules = " << gamerules;
		return;
	}

	if (!gEnv->pRemoteServer->CheckGameServerExists(serverName, serverIp, serverPort))
	{
		m_client->isGameServer = true;
		m_client->server->name = serverName;
		m_client->server->ip = serverIp;
		m_client->server->port = serverPort;
		m_client->server->map = mapName;
		m_client->server->gamerules = gamerules;
		m_client->server->online = online;
		m_client->server->maxPlayers = maxPlayers;

		gEnv->pRemoteServer->UpdateClient(m_client);

		int gameServersCount = 0;
		gEnv->pRemoteServer->bHaveAdmin ? gameServersCount = gEnv->pRemoteServer->GetClientCount() - 1 : gameServersCount = gEnv->pRemoteServer->GetClientCount();

		QByteArray result;
		result.append("<result type='register_game_server_complete'/>");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, result);

		qDebug() << "Game server [" << serverName << "] registered!";
		qDebug() << "Connected game servers count = " << gameServersCount;
	}
	else
	{
		qDebug() << "----------Server with this address or name alredy registered---------";
		qDebug() << "---------------------REGISTER GAME SERVER FAILED---------------------";

		QByteArray result("<error type='register_game_server_failed' reason = '0'/>");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, result);
	}
}

void RemoteClientQuerys::onGameServerUpdateInfo(QByteArray & bytes)
{
	if (!m_client->isGameServer)
	{
		qWarning() << "Only registered game servers can update info";
		return;
	}

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString serverName = attributes.value("name").toString();
	QString serverIp = attributes.value("ip").toString();
	int serverPort = attributes.value("port").toInt();
	QString mapName = attributes.value("map").toString();
	QString gamerules = attributes.value("gamerules").toString();
	int online = attributes.value("online").toInt();
	int maxPlayers = attributes.value("maxPlayers").toInt();

	if (serverName.isEmpty() || serverIp.isEmpty() || mapName.isEmpty() || gamerules.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
		qDebug() << "ServerName = " << serverName << "ServerIp = " << serverIp << "MapName = " << mapName << "Gamerules = " << gamerules;
		return;
	}

	if (gEnv->pRemoteServer->CheckGameServerExists(serverName, serverIp, serverPort))
	{
		m_client->isGameServer = true;
		m_client->server->name = serverName;
		m_client->server->ip = serverIp;
		m_client->server->port = serverPort;
		m_client->server->map = mapName;
		m_client->server->gamerules = gamerules;
		m_client->server->online = online;
		m_client->server->maxPlayers = maxPlayers;

		gEnv->pRemoteServer->UpdateClient(m_client);

		QByteArray result("<result type='game_server_info_updated'/>");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, result);

		qDebug() << "Game server [" << serverName << "] updated info";
	}
	else
	{
		qDebug() << "-------------------------Server not found--------------------------";
		qDebug() << "---------------------UPDATE GAME SERVER FAILED---------------------";

		QByteArray result("<error type='update_game_server_failed' reason = '0'/>");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, result);
	}
}

void RemoteClientQuerys::onGameServerGetOnlineProfile(QByteArray & bytes)
{
	if (!m_client->isGameServer)
	{
		qWarning() << "Only registered game servers can get online profiles";
		return;
	}

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	int uid = attributes.value("uid").toInt();

	SProfile* pProfile = gEnv->pServer->GetProfileByUid(uid);

	if (pProfile != nullptr)
	{
		QByteArray result;
		result.append("<result type='profile_data'>" + ProfileToString(pProfile) + "</result>");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, result);
		return;
	}
	else
	{
		qDebug() << "Failed get online profile";

		QByteArray result("<error type='get_online_profile_failed' reason = '0'/>");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, result);
	}
}

void RemoteClientQuerys::onGameServerUpdateOnlineProfile(QByteArray & bytes)
{
	if (!m_client->isGameServer)
	{
		qWarning() << "Only registered game servers can update profiles";
		return;
	}

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	int uid = attributes.value("uid").toInt();
	QString nickname = attributes.value("nickname").toString();
	QString fileModel = attributes.value("fileModel").toString();
	int lvl = attributes.value("lvl").toInt();
	int xp = attributes.value("xp").toInt();
	int money = attributes.value("money").toInt();
	QString items = attributes.value("items").toString();
	QString friends = attributes.value("friends").toString();

	SProfile* pOldProfile = gEnv->pServer->GetProfileByUid(uid);	

	if (pOldProfile != nullptr)
	{
		// Game server can update only lvl, xp and money!
		SProfile* pNewProfile = new SProfile;
		pNewProfile->uid = pOldProfile->uid;
		pNewProfile->nickname = pOldProfile->nickname;
		pNewProfile->fileModel = pOldProfile->fileModel;
		pNewProfile->lvl = lvl;
		pNewProfile->xp = xp;
		pNewProfile->money = money;
		pNewProfile->items = pOldProfile->items;
		pNewProfile->friends = pOldProfile->friends;

		if (gEnv->pServer->UpdateProfile(pNewProfile))
		{
			QByteArray result("<result type='profile_updated'/>");
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, result);
		}
		else
		{
			qDebug() << "Failed update online profile";

			QByteArray result("<error type='update_online_profile_failed' reason = '1'/>");
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, result);
		}	
	}
	else
	{
		qDebug() << "Failed get online profile";

		QByteArray result("<error type='update_online_profile_failed' reason = '0'/>");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, result);
	}
}

QXmlStreamAttributes RemoteClientQuerys::GetAttributesFromArray(QByteArray & bytes)
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

QString RemoteClientQuerys::ProfileToString(SProfile * profile)
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
