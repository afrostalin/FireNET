// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "global.h"
#include "remoteclientquerys.h"
#include "clientquerys.h"
#include "tcpserver.h"
#include "remoteserver.h"
#include "dbworker.h"
#include "settings.h"
#include "netpacket.h"
#include "scripts.h"

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

// Error types : 0 - Login not found, 1 - Incorrect password, 2 - Admin alredy log in
void RemoteClientQuerys::onAdminLogining(NetPacket &packet)
{
	qWarning() << "Client (" << m_socket << ") trying login in administrator mode!";

	if (gEnv->pRemoteServer->bHaveAdmin)
	{
		qCritical() << "Error authorization in administator mode! Reason = Administrator alredy has entered";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_remote_admin_login_fail);
		m_packet.WriteInt(2);
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);

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

	if (login == gEnv->pSettings->GetVariable("sv_root_user").toString())
	{
		if (password == gEnv->pSettings->GetVariable("sv_root_password").toString())
		{
			qWarning() << "Client (" << m_socket << ") success login in administator mode!";
			gEnv->pRemoteServer->bHaveAdmin = true;

			m_client->isAdmin = true;
			gEnv->pRemoteServer->UpdateClient(m_client);

			NetPacket m_packet(net_Result);
			m_packet.WriteInt(net_result_remote_admin_login_complete);
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);

			return;
		}
		else
		{
			qCritical() << "Error authorization in administator mode! Reason = Wrong password. Password = " << password;

			NetPacket m_packet(net_Error);
			m_packet.WriteInt(net_result_remote_admin_login_fail);
			m_packet.WriteInt(1);
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);

			return;
		}

	}
	else
	{
		qCritical() << "Error authorization in administator mode! Reason = Wrong login. Login = " << login;

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_remote_admin_login_fail);
		m_packet.WriteInt(0);
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);

		return;
	}
}

// Error types : 0 - Command not found
// Complete types : 0 - status, 1 - message, 2 - command, 3 - players, 4 - servers
void RemoteClientQuerys::onConsoleCommandRecived(NetPacket &packet)
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
		int gameServersCount = 0;
		gEnv->pRemoteServer->bHaveAdmin ? gameServersCount = gEnv->pRemoteServer->GetClientCount() - 1 : gameServersCount = gEnv->pRemoteServer->GetClientCount();

		NetPacket m_packet(net_Result);
		m_packet.WriteInt(net_result_remote_command_complete);
		m_packet.WriteInt(0);
		m_packet.WriteString(qApp->applicationVersion().toStdString()); // Server version
		m_packet.WriteString(gEnv->pSettings->GetVariable("sv_ip").toString().toStdString()); // Server ip
		m_packet.WriteInt(gEnv->pSettings->GetVariable("sv_port").toInt()); // Server port
		m_packet.WriteInt(gEnv->pSettings->GetVariable("sv_tickrate").toInt()); // Server tick rate
		m_packet.WriteString(gEnv->pSettings->GetVariable("db_mode").toString().toStdString()); // Database mode
		m_packet.WriteString(gEnv->pSettings->GetVariable("auth_mode").toString().toStdString()); // Authorization mode
		m_packet.WriteInt(gEnv->pServer->GetClientCount()); // Players ammount
		m_packet.WriteInt(gEnv->pSettings->GetVariable("sv_max_players").toInt()); // Max players count
		m_packet.WriteInt(gameServersCount); // Game servers count
		m_packet.WriteInt(gEnv->pSettings->GetVariable("sv_max_servers").toInt()); // Max game servers count
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);

		return;
	}else if(command == "send_message")
	{
		NetPacket msg(net_Server);
		msg.WriteInt(net_server_message);
		msg.WriteString(value.toStdString());
		gEnv->pServer->sendGlobalMessage(msg);

		NetPacket m_packet(net_Result);
		m_packet.WriteInt(net_result_remote_command_complete);
		m_packet.WriteInt(1);
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);

		return;
	}else if (command == "send_command")
	{
		NetPacket msg(net_Server);
		msg.WriteInt(net_server_command);
		msg.WriteString(value.toStdString());
		gEnv->pServer->sendGlobalMessage(msg);

		NetPacket m_packet(net_Result);
		m_packet.WriteInt(net_result_remote_command_complete);
		m_packet.WriteInt(2);
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);

		return;
	}else if (command == "players")
	{
		QStringList players = gEnv->pServer->GetPlayersList();

		if (!players.isEmpty())
		{
			NetPacket m_packet(net_Result);
			m_packet.WriteInt(net_result_remote_command_complete);
			m_packet.WriteInt(3);
			m_packet.WriteString(players.join(",").toStdString());
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);
		}
		else
		{
			NetPacket m_packet(net_Result);
			m_packet.WriteInt(net_result_remote_command_complete);
			m_packet.WriteInt(3);
			m_packet.WriteString("No any players");
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);
		}

		return;
	}else if(command == "servers")
	{
		QStringList servers = gEnv->pRemoteServer->GetServerList();

		if (!servers.isEmpty())
		{
			NetPacket m_packet(net_Result);
			m_packet.WriteInt(net_result_remote_command_complete);
			m_packet.WriteInt(4);
			m_packet.WriteString(servers.join(",").toStdString());
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);
		}
		else
		{
			NetPacket m_packet(net_Result);
			m_packet.WriteInt(net_result_remote_command_complete);
			m_packet.WriteInt(4);
			m_packet.WriteString("No any game servers");
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);
		}

		return;
	}

	NetPacket m_packet(net_Error);
	m_packet.WriteInt(net_result_remote_command_fail);
	m_packet.WriteInt(0);
	gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);
}

// Error types : 0 - Game server not found in trusted list, 1 - Server alredy registered
void RemoteClientQuerys::onGameServerRegister(NetPacket &packet)
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

		NetPacket m_packet(net_Result);
		m_packet.WriteInt(net_result_remote_register_server_fail);
		m_packet.WriteInt(0);
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);
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

		NetPacket m_packet(net_Result);
		m_packet.WriteInt(net_result_remote_register_server_complete);
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);

		qDebug() << "Game server [" << serverName << "] registered!";
		qDebug() << "Connected game servers count = " << gameServersCount;
	}
	else
	{
		qDebug() << "----------Server with this address or name alredy registered---------";
		qDebug() << "---------------------REGISTER GAME SERVER FAILED---------------------";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_remote_register_server_fail);
		m_packet.WriteInt(1);
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);
	}
}

// Error tyoes : 0 - Game server not found
void RemoteClientQuerys::onGameServerUpdateInfo(NetPacket &packet)
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

		NetPacket m_packet(net_Result);
		m_packet.WriteInt(net_result_remote_update_server_complete);
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);

		qDebug() << "Game server [" << serverName << "] updated info";
	}
	else
	{
		qDebug() << "-------------------------Server not found--------------------------";
		qDebug() << "---------------------UPDATE GAME SERVER FAILED---------------------";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_remote_update_server_fail);
		m_packet.WriteInt(0);
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);
	}
}

// Error types :  0 - Profile not found
void RemoteClientQuerys::onGameServerGetOnlineProfile(NetPacket &packet)
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
		NetPacket profile(net_Result);
		profile.WriteInt(net_result_remote_get_profile_complete);
		profile.WriteInt(pProfile->uid);
		profile.WriteString(pProfile->nickname.toStdString());
		profile.WriteString(pProfile->fileModel.toStdString());
		profile.WriteInt(pProfile->lvl);
		profile.WriteInt(pProfile->xp);
		profile.WriteInt(pProfile->money);
		profile.WriteString(pProfile->items.toStdString());
		profile.WriteString(pProfile->friends.toStdString());

		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, profile);
		return;
	}
	else
	{
		qDebug() << "Failed get online profile";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_remote_get_profile_fail);
		m_packet.WriteInt(0);
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);
	}
}

// Error types : 0 - Profile not found, 1 - Can't update profile
void RemoteClientQuerys::onGameServerUpdateOnlineProfile(NetPacket &packet)
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
			NetPacket m_packet(net_Result);
			m_packet.WriteInt(net_result_remote_update_profile_complete);
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);
		}
		else
		{
			qDebug() << "Failed update online profile";

			NetPacket m_packet(net_Error);
			m_packet.WriteInt(net_result_remote_update_profile_fail);
			m_packet.WriteInt(1);
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);
		}	
	}
	else
	{
		qDebug() << "Failed get online profile";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_remote_update_profile_fail);
		m_packet.WriteInt(0);
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, m_packet);
	}
}

bool RemoteClientQuerys::CheckInTrustedList(QString name, QString ip, int port)
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