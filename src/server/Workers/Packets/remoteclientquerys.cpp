// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "remoteclientquerys.h"
#include "clientquerys.h"

#include "Core/tcpserver.h"
#include "Core/remoteserver.h"
#include "Core/tcppacket.h"

#include "Workers/Databases/dbworker.h"

#include "Tools/settings.h"
#include "Tools/scripts.h"

#include <QCoreApplication>

RemoteClientQuerys::RemoteClientQuerys(QObject *parent) : QObject(parent),
	m_socket(nullptr),
	m_client(nullptr),
	m_connection(nullptr)
{
}

RemoteClientQuerys::~RemoteClientQuerys()
{
	qDebug() << "~RemoteClientQuerys";
	SAFE_DELETE(m_client->server);
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

// Error types : 0 - Login not found, 1 - Incorrect password, 2 - Admin alredy log in
void RemoteClientQuerys::onAdminLogining(CTcpPacket &packet)
{
	qWarning() << "Client (" << m_socket << ") trying login in administrator mode!";

	if (gEnv->pRemoteServer->IsHaveAdmin())
	{
		qCritical() << "Error authorization in administator mode! Reason = Administrator alredy has entered";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::AdminLoginFail);
		m_packet.WriteInt(2);
		m_connection->SendMessage(m_packet);

		return;
	}

	QString login = packet.ReadString();
	QString password = packet.ReadString();

	if (login.isEmpty() || password.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
		qDebug() << "Login = " << login << "Password = " << password;
		return;
	}

	if (login == gEnv->pSettings->GetVariable("remote_root_user").toString())
	{
		if (password == gEnv->pSettings->GetVariable("remote_root_password").toString())
		{
			qWarning() << "Client (" << m_socket << ") success login in administator mode!";
			gEnv->pRemoteServer->SetAdmin(true);

			m_client->isAdmin = true;
			gEnv->pRemoteServer->UpdateClient(m_client);

			CTcpPacket m_packet(EFireNetTcpPacketType::Result);
			m_packet.WriteResult(EFireNetTcpResult::AdminLoginComplete);
			m_connection->SendMessage(m_packet);

			return;
		}
		else
		{
			qCritical() << "Error authorization in administator mode! Reason = Wrong password. Password = " << password;

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::AdminLoginFail);
			m_packet.WriteInt(1);
			m_connection->SendMessage(m_packet);

			return;
		}

	}
	else
	{
		qCritical() << "Error authorization in administator mode! Reason = Wrong login. Login = " << login;

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::AdminLoginFail);
		m_packet.WriteInt(0);
		m_connection->SendMessage(m_packet);

		return;
	}
}

// Error types : 0 - Command not found
// Complete types : 0 - status, 1 - message, 2 - command, 3 - players, 4 - servers
void RemoteClientQuerys::onConsoleCommandRecived(CTcpPacket &packet)
{
	if (!m_client->isAdmin)
	{
		qCritical() << "Only administrator can use console commands!";
		return;
	}

	QString command = packet.ReadString();
	QString value = packet.ReadString();

	if (command == "status")
	{
		CTcpPacket m_packet(EFireNetTcpPacketType::Result);
		m_packet.WriteResult(EFireNetTcpResult::AdminCommandComplete);
		m_packet.WriteInt(0);
		m_packet.WriteString(qApp->applicationVersion().toStdString()); // Server version
		m_packet.WriteString(gEnv->pSettings->GetVariable("sv_ip").toString().toStdString()); // Server ip
		m_packet.WriteInt(gEnv->pSettings->GetVariable("sv_port").toInt()); // Server port
		m_packet.WriteInt(gEnv->pSettings->GetVariable("sv_tickrate").toInt()); // Server tick rate
		m_packet.WriteString(gEnv->pSettings->GetVariable("db_mode").toString().toStdString()); // Database mode
		m_packet.WriteInt(gEnv->pServer->GetClientCount()); // Players ammount
		m_packet.WriteInt(gEnv->pSettings->GetVariable("sv_max_players").toInt()); // Max players count
		m_packet.WriteInt(gEnv->pRemoteServer->GetClientCount()); // Game servers count
		m_packet.WriteInt(gEnv->pSettings->GetVariable("sv_max_servers").toInt()); // Max game servers count
		m_connection->SendMessage(m_packet);

		return;
	}
	else if (command == "send_message")
	{
		CTcpPacket msg(EFireNetTcpPacketType::ServerMessage);
		msg.WriteServerMessage(EFireNetTcpSMessage::ServerMessage);
		msg.WriteString(value.toStdString());
		gEnv->pServer->sendGlobalMessage(msg);

		CTcpPacket m_packet(EFireNetTcpPacketType::Result);
		m_packet.WriteResult(EFireNetTcpResult::AdminCommandComplete);
		m_packet.WriteInt(1);
		m_connection->SendMessage(m_packet);

		return;
	}
	else if (command == "send_command")
	{
		CTcpPacket msg(EFireNetTcpPacketType::ServerMessage);
		msg.WriteServerMessage(EFireNetTcpSMessage::ServerCommand);
		msg.WriteString(value.toStdString());
		gEnv->pServer->sendGlobalMessage(msg);

		CTcpPacket m_packet(EFireNetTcpPacketType::Result);
		m_packet.WriteResult(EFireNetTcpResult::AdminCommandComplete);
		m_packet.WriteInt(2);
		m_connection->SendMessage(m_packet);

		return;
	}
	else if (command == "players")
	{
		QStringList players = gEnv->pServer->GetPlayersList();

		if (!players.isEmpty())
		{
			CTcpPacket m_packet(EFireNetTcpPacketType::Result);
			m_packet.WriteResult(EFireNetTcpResult::AdminCommandComplete);
			m_packet.WriteInt(3);
			m_packet.WriteString(players.join(",").toStdString());
			m_connection->SendMessage(m_packet);
		}
		else
		{
			CTcpPacket m_packet(EFireNetTcpPacketType::Result);
			m_packet.WriteResult(EFireNetTcpResult::AdminCommandComplete);
			m_packet.WriteInt(3);
			m_packet.WriteString("No any players");
			m_connection->SendMessage(m_packet);
		}

		return;
	}
	else if (command == "servers")
	{
		QStringList servers = gEnv->pRemoteServer->GetServerList();

		if (!servers.isEmpty())
		{
			CTcpPacket m_packet(EFireNetTcpPacketType::Result);
			m_packet.WriteResult(EFireNetTcpResult::AdminCommandComplete);
			m_packet.WriteInt(4);
			m_packet.WriteString(servers.join(",").toStdString());
			m_connection->SendMessage(m_packet);
		}
		else
		{
			CTcpPacket m_packet(EFireNetTcpPacketType::Result);
			m_packet.WriteResult(EFireNetTcpResult::AdminCommandComplete);
			m_packet.WriteInt(4);
			m_packet.WriteString("No any game servers");
			m_connection->SendMessage(m_packet);
		}

		return;
	}

	CTcpPacket m_packet(EFireNetTcpPacketType::Error);
	m_packet.WriteError(EFireNetTcpError::AdminCommandFail);
	m_packet.WriteInt(0);
	m_connection->SendMessage(m_packet);
}

// Error types : 0 - Game server not found in trusted list, 1 - Server alredy registered
void RemoteClientQuerys::onGameServerRegister(CTcpPacket &packet)
{
	QString serverName = packet.ReadString();
	QString serverIp = packet.ReadString();
	int serverPort = packet.ReadInt();
	QString mapName = packet.ReadString();
	QString gamerules = packet.ReadString();
	int online = packet.ReadInt();
	int maxPlayers = packet.ReadInt();

	if (serverName.isEmpty() || serverIp.isEmpty() || mapName.isEmpty() || gamerules.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
		qDebug() << "ServerName = " << serverName << "ServerIp = " << serverIp << "MapName = " << mapName << "Gamerules = " << gamerules;
		return;
	}

	if (!CheckInTrustedList(serverName, serverIp, serverPort))
	{
		qWarning() << "Game server" << serverName << "not found in trusted server list. Registration not posible!";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::RegisterServerFail);
		m_packet.WriteInt(0);
		m_connection->SendMessage(m_packet);
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
		gEnv->pRemoteServer->IsHaveAdmin() ? gameServersCount = gEnv->pRemoteServer->GetClientCount() - 1 : gameServersCount = gEnv->pRemoteServer->GetClientCount();

		CTcpPacket m_packet(EFireNetTcpPacketType::Result);
		m_packet.WriteResult(EFireNetTcpResult::RegisterServerComplete);
		m_connection->SendMessage(m_packet);

		qDebug() << "Game server [" << serverName << "] registered!";
		qDebug() << "Connected game servers count = " << gameServersCount;
	}
	else
	{
		qDebug() << "----------Server with this address or name alredy registered---------";
		qDebug() << "---------------------REGISTER GAME SERVER FAILED---------------------";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::RegisterServerFail);
		m_packet.WriteInt(1);
		m_connection->SendMessage(m_packet);
	}
}

// Error tyoes : 0 - Game server not found
void RemoteClientQuerys::onGameServerUpdateInfo(CTcpPacket &packet)
{
	if (!m_client->isGameServer)
	{
		qWarning() << "Only registered game servers can update info";
		return;
	}

	QString serverName = packet.ReadString();
	QString serverIp = packet.ReadString();
	int serverPort = packet.ReadInt();
	QString mapName = packet.ReadString();
	QString gamerules = packet.ReadString();
	int online = packet.ReadInt();
	int maxPlayers = packet.ReadInt();

	if (serverName.isEmpty() || serverIp.isEmpty() || mapName.isEmpty() || gamerules.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
		qDebug() << "ServerName = " << serverName << "ServerIp = " << serverIp << "MapName = " << mapName << "Gamerules = " << gamerules;
		return;
	}

	if (gEnv->pRemoteServer->CheckGameServerExists(serverName, serverIp, serverPort))
	{
		m_client->server->name = serverName;
		m_client->server->ip = serverIp;
		m_client->server->port = serverPort;
		m_client->server->map = mapName;
		m_client->server->gamerules = gamerules;
		m_client->server->online = online;
		m_client->server->maxPlayers = maxPlayers;

		gEnv->pRemoteServer->UpdateClient(m_client);

		CTcpPacket m_packet(EFireNetTcpPacketType::Result);
		m_packet.WriteResult(EFireNetTcpResult::UpdateServerComplete);
		m_connection->SendMessage(m_packet);

		qDebug() << "Game server [" << serverName << "] updated info";
	}
	else
	{
		qDebug() << "-------------------------Server not found--------------------------";
		qDebug() << "---------------------UPDATE GAME SERVER FAILED---------------------";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::UpdateServerFail);
		m_packet.WriteInt(0);
		m_connection->SendMessage(m_packet);
	}
}

// Error types :  0 - Profile not found
void RemoteClientQuerys::onGameServerGetOnlineProfile(CTcpPacket &packet)
{
	if (!m_client->isGameServer)
	{
		qWarning() << "Only registered game servers can get online profiles";
		return;
	}

	int uid = packet.ReadInt();

	SProfile* pProfile = gEnv->pServer->GetProfileByUid(uid);

	if (pProfile != nullptr)
	{
		CTcpPacket profile(EFireNetTcpPacketType::Result);
		profile.WriteResult(EFireNetTcpResult::GetProfileComplete);
		profile.WriteInt(pProfile->uid);
		profile.WriteString(pProfile->nickname.toStdString());
		profile.WriteString(pProfile->fileModel.toStdString());
		profile.WriteInt(pProfile->lvl);
		profile.WriteInt(pProfile->xp);
		profile.WriteInt(pProfile->money);
		profile.WriteString(pProfile->items.toStdString());
		profile.WriteString(pProfile->friends.toStdString());

		m_connection->SendMessage(profile);
		return;
	}
	else
	{
		qDebug() << "Failed get online profile";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::GetProfileFail);
		m_packet.WriteInt(0);
		m_connection->SendMessage(m_packet);
	}
}

// Error types : 0 - Profile not found, 1 - Can't update profile
void RemoteClientQuerys::onGameServerUpdateOnlineProfile(CTcpPacket &packet)
{
	if (!m_client->isGameServer)
	{
		qWarning() << "Only registered game servers can update profiles";
		return;
	}

	int uid = packet.ReadInt();
	QString nickname = packet.ReadString();
	QString fileModel = packet.ReadString();
	int lvl = packet.ReadInt();
	int xp = packet.ReadInt();
	int money = packet.ReadInt();
	QString items = packet.ReadString();
	QString friends = packet.ReadString();

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
			CTcpPacket m_packet(EFireNetTcpPacketType::Result);
			m_packet.WriteResult(EFireNetTcpResult::UpdateProfileComplete);
			m_connection->SendMessage(m_packet);
		}
		else
		{
			qDebug() << "Failed update online profile";

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::UpdateProfileFail);
			m_packet.WriteInt(1);
			m_connection->SendMessage(m_packet);
		}
	}
	else
	{
		qDebug() << "Failed get online profile";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::UpdateProfileFail);
		m_packet.WriteInt(0);
		m_connection->SendMessage(m_packet);
	}
}

bool RemoteClientQuerys::CheckInTrustedList(const QString &name, const QString &ip, int port)
{
	QVector<STrustedServer> m_server = gEnv->pScripts->GetTrustedList();

	if (m_server.size() > 0)
	{
		for (auto it = m_server.begin(); it != m_server.end(); ++it)
		{
			if (it->name == name && it->ip == ip && it->port == port)
			{
				qDebug() << "Server found in trusted server list";
				return true;
			}
		}
	}

	qWarning() << "Server not found in trusted server list!";
	return false;
}