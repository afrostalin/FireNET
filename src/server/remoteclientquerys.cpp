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
		QByteArray msg;
		msg.append("***Server status***\n");
		msg.append("Server version = " + qApp->applicationVersion() + "\n");
		msg.append("Server ip = " + gEnv->pSettings->GetVariable("sv_ip").toString() + "\n");
		msg.append("Server port = " + gEnv->pSettings->GetVariable("sv_port").toString() + "\n");
		msg.append("Server tickrate = " + gEnv->pSettings->GetVariable("sv_tickrate").toString() + " per/sec.\n");
		msg.append("Database mode = " + gEnv->pSettings->GetVariable("db_mode").toString() + "\n");
		msg.append("Authrorization type = " + gEnv->pSettings->GetVariable("auth_mode").toString() + "\n");
		msg.append("Players ammount = " + QString::number(gEnv->pServer->GetClientCount()) + ". Maximum players count = " + gEnv->pSettings->GetVariable("sv_max_players").toString() + "\n");
		//msg.append("Game servers ammount = . Maximum game servers count = ");
		//msg.append("Errors count = ");
		//msg.append("Warnings count = ");
		msg.append("*******************");

		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);

		return;
	}

	if (command == "list")
	{
		QByteArray msg;
		msg.append("status - Get full server status\n");
		msg.append("send_global_message <message> - Send message to all connected players\n");
		msg.append("send_console_command <r_displayinfo 1> - Send console command to all connected players\n");
		msg.append("players - Get player list\n");
		msg.append("servers - Get game server list\n");
		msg.append("quit - Shutdown server\n");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);
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

	/*QVector<SGameServer>::iterator it;
	for (it = vServers.begin(); it != vServers.end(); ++it)
	{
	if (it->name == serverName)
	{
	qDebug() << "---------------Server with this name alredy registered---------------";
	qDebug() << "---------------------REGISTER GAME SERVER FAILED---------------------";
	return;
	}
	if (it->ip == serverIp && it->port == serverPort)
	{
	qDebug() << "-------------Server with this address alredy registered--------------";
	qDebug() << "---------------------REGISTER GAME SERVER FAILED---------------------";
	return;
	}
	}*/

	// Register new game server here
	SGameServer gameServer;
	gameServer.name = serverName;
	gameServer.ip = serverIp;
	gameServer.port = serverPort;
	gameServer.map = mapName;
	gameServer.gamerules = gamerules;
	gameServer.online = online;
	gameServer.maxPlayers = maxPlayers;

	//vServers.push_back(gameServer);

	// Update client info
	/*QVector<SClient>::iterator clientIt;
	for (clientIt = vClients.begin(); clientIt != vClients.end(); ++it)
	{
	if (clientIt->socket == m_socket)
	{
	clientIt->isGameServer = true;
	break;
	}
	}*/

	qDebug() << "Game server [" << serverName << "] registered!";
	//qDebug() << "Connected game servers count = " << vServers.size();
}

void RemoteClientQuerys::onGameServerUpdateInfo(QByteArray & bytes)
{
}

void RemoteClientQuerys::onGameServerGetOnlineProfile(QByteArray & bytes)
{
}

void RemoteClientQuerys::onGameServerUpdateOnlineProfile(QByteArray & bytes)
{
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
