// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "remoteclientquerys.h"
#include "serverThread.h"

#include "Core/tcpserver.h"
#include "Core/remoteserver.h"
#include "Core/tcppacket.h"

#include "Tools/console.h"
#include "Tools/scripts.h"

#include <QCoreApplication>

RemoteClientQuerys::RemoteClientQuerys(QObject *parent) 
	: QObject(parent)
	, m_socket(nullptr)
	, m_client(nullptr)
	, m_connection(nullptr)
	, bReloadRequestSended(false)
	, m_ServerAlive(false)
{
}

RemoteClientQuerys::~RemoteClientQuerys()
{
	m_Timer.stop();
	SAFE_DELETE(m_client->server);
	SAFE_DELETE(m_client->pArbitrator);
}

void RemoteClientQuerys::SetClient(SRemoteClient * client)
{
	m_client = client;
	m_client->server = new SGameServer;
	m_client->pArbitrator = new SDedicatedArbitrator;
	m_client->pQuerys = this;
}

void RemoteClientQuerys::SetConnection(RemoteConnection * connection)
{
	m_connection = connection;
	connect(m_connection, &RemoteConnection::finished, this, &RemoteClientQuerys::OnConnectionClosed);
}

bool RemoteClientQuerys::ReadPacket(CTcpPacket & packet)
{
	EFireNetTcpPacketType type = packet.getType();

	switch (type)
	{
	case EFireNetTcpPacketType::Empty:
	{
		return false;
	}
	case EFireNetTcpPacketType::Query:
	{
		return ReadQuery(packet);
	}
	case EFireNetTcpPacketType::Result:
	{
		return false;
	}
	case EFireNetTcpPacketType::Error:
	{
		return false;
	}
	case EFireNetTcpPacketType::ServerMessage:
	{
		return false;
	}
	default:
	{
		return false;
	}
	}
}

bool RemoteClientQuerys::ReadQuery(CTcpPacket & packet)
{
	const EFireNetTcpQuery type = packet.ReadQuery();

	switch (type)
	{
	case EFireNetTcpQuery::AdminLogin:
	{
		onAdminLogining(packet);
		break;
	}
	case EFireNetTcpQuery::AdminCommand:
	{
		onConsoleCommandRecived(packet);
		break;
	}
	case EFireNetTcpQuery::RegisterServer:
	{
		onGameServerRegister(packet);
		break;
	}
	case EFireNetTcpQuery::UpdateServer:
	{
		onGameServerUpdateInfo(packet);
		break;
	}
	case EFireNetTcpQuery::UpdateProfile:
	{
		onGameServerUpdateOnlineProfile(packet);
		break;
	}
	case EFireNetTcpQuery::PingPong:
	{
		onPingPong(packet);
		break;
	}
	case EFireNetTcpQuery::RegisterArbitrator:
	{
		onArbitratorRegister(packet);
		break;
	}
	case EFireNetTcpQuery::UpdateArbitrator:
	{
		onArbitratorUpdate(packet);
		break;
	}
	default:
	{
		return false;
	}
	}

	return true;
}

void RemoteClientQuerys::onAdminLogining(CTcpPacket &packet) const
{
	LogWarning("Client with socket <%p> trying login in administrator mode!", m_socket);

	if (gEnv->pRemoteServer->IsHaveAdmin())
	{
		LogError("Error authorization in administator mode! Reason = Administrator alredy has entered");

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::AdminLoginFail);
		m_packet.WriteErrorCode(EFireNetTcpErrorCode::AdminAlredyLogined);
		m_connection->SendMessage(m_packet);

		return;
	}

	QString login = packet.ReadString();
	QString password = packet.ReadString();

	if (login.isEmpty() || password.isEmpty())
	{
		LogWarning("Wrong packet data! Some values empty!");
		LogDebug("Login = <%s>, password = <%s>", login.toStdString().c_str(), password.toStdString().c_str());
		return;
	}

	if (login == gEnv->pConsole->GetString("remote_root_user").c_str())
	{
		if (password == gEnv->pConsole->GetString("remote_root_password").c_str())
		{
			LogWarning("Client with socket <%p> success login in administrator mode!", m_socket);
			gEnv->pRemoteServer->SetAdmin(true);

			m_client->isAdmin = true;
			gEnv->pRemoteServer->UpdateClient(m_client);

			CTcpPacket m_packet(EFireNetTcpPacketType::Result);
			m_packet.WriteResult(EFireNetTcpResult::AdminLoginComplete);
			m_connection->SendMessage(m_packet);

			return;
		}
		LogError("Error authorization in administator mode! Reason = Wrong password. Password = <%s>", password.toStdString().c_str());

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::AdminLoginFail);
		m_packet.WriteErrorCode(EFireNetTcpErrorCode::AdminIncorrectPassword);
		m_connection->SendMessage(m_packet);

		return;
	}
	LogError("Error authorization in administator mode! Reason = Wrong login. Login = <%s>", login.toStdString().c_str());

	CTcpPacket m_packet(EFireNetTcpPacketType::Error);
	m_packet.WriteError(EFireNetTcpError::AdminLoginFail);
	m_packet.WriteErrorCode(EFireNetTcpErrorCode::AdminLoginNotFound);
	m_connection->SendMessage(m_packet);

	return;
}

void RemoteClientQuerys::onConsoleCommandRecived(CTcpPacket &packet)
{
	if (!m_client->isAdmin)
	{
		LogError("Only administrator can use console commands!");
		return;
	}

	EFireNetAdminCommands commandType = packet.ReadAdminCommand();

	switch (commandType)
	{
	case EFireNetAdminCommands::CMD_Status:
	{
		CTcpPacket m_packet(EFireNetTcpPacketType::Result);
		m_packet.WriteResult(EFireNetTcpResult::AdminCommandComplete);
		m_packet.WriteAdminCommand(commandType);

		std::vector<std::string> status = gEnv->pMainThread->DumpStatistic();

		m_packet.WriteArray(status);

		m_connection->SendMessage(m_packet);
		break;
	}
	case EFireNetAdminCommands::CMD_SendGlobalMessage:
	{
		const char* message = packet.ReadString();

		CTcpPacket msg(EFireNetTcpPacketType::ServerMessage);
		msg.WriteServerMessage(EFireNetTcpSMessage::ServerMessage);
		msg.WriteString(message);
		gEnv->pServer->sendGlobalMessage(msg);


		CTcpPacket callback(EFireNetTcpPacketType::Result);
		callback.WriteResult(EFireNetTcpResult::AdminCommandComplete);
		callback.WriteAdminCommand(commandType);
		m_connection->SendMessage(callback);
		break;
	}
	case EFireNetAdminCommands::CMD_SendGlobalCommand:
	{
		const char* command = packet.ReadString();

		CTcpPacket msg(EFireNetTcpPacketType::ServerMessage);
		msg.WriteServerMessage(EFireNetTcpSMessage::ServerCommand);
		msg.WriteString(command);
		gEnv->pServer->sendGlobalMessage(msg);

		CTcpPacket callback(EFireNetTcpPacketType::Result);
		callback.WriteResult(EFireNetTcpResult::AdminCommandComplete);
		callback.WriteAdminCommand(commandType);
		m_connection->SendMessage(callback);
		break;
	}
	case EFireNetAdminCommands::CMD_SendRemoteMessage:
	{
		break;
	}
	case EFireNetAdminCommands::CMD_SendRemoteCommand:
	{
		const char* command = packet.ReadString();

		CTcpPacket msg(EFireNetTcpPacketType::ServerMessage);
		msg.WriteServerMessage(EFireNetTcpSMessage::ServerCommand);
		msg.WriteString(command);
		gEnv->pRemoteServer->SendMessageToAllRemoteClients(msg);

		CTcpPacket callback(EFireNetTcpPacketType::Result);
		callback.WriteResult(EFireNetTcpResult::AdminCommandComplete);
		callback.WriteAdminCommand(commandType);
		m_connection->SendMessage(callback);
		break;
	}
	case EFireNetAdminCommands::CMD_GetPlayersList:
	{
		std::vector<std::string> playerList = gEnv->pServer->DumpPlayerList();

		CTcpPacket callback(EFireNetTcpPacketType::Result);
		callback.WriteResult(EFireNetTcpResult::AdminCommandComplete);
		callback.WriteAdminCommand(commandType);
		callback.WriteArray(playerList);

		m_connection->SendMessage(callback);
		break;
	}
	case EFireNetAdminCommands::CMD_GetGameServersList:
	{
		std::vector<std::string> serverList = gEnv->pRemoteServer->DumpServerList();

		CTcpPacket callback(EFireNetTcpPacketType::Result);
		callback.WriteResult(EFireNetTcpResult::AdminCommandComplete);
		callback.WriteAdminCommand(commandType);
		callback.WriteArray(serverList);
		m_connection->SendMessage(callback);
		break;
	}
	case EFireNetAdminCommands::CMD_RawMasterServerCommand:
	{
		const char* command = packet.ReadString();

		gEnv->pConsole->ExecuteCommand(command);

		CTcpPacket callback(EFireNetTcpPacketType::Result);
		callback.WriteResult(EFireNetTcpResult::AdminCommandComplete);
		callback.WriteAdminCommand(commandType);
		m_connection->SendMessage(callback);
		break;
	}
	default:
	{
		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::AdminCommandFail);
		m_packet.WriteErrorCode(EFireNetTcpErrorCode::AdminCommandNotFound);
		m_connection->SendMessage(m_packet);
		break;
	}
	}
}

void RemoteClientQuerys::onGameServerRegister(CTcpPacket &packet)
{
	SGameServer gameServerInfo;
	gameServerInfo.name = packet.ReadString();
	gameServerInfo.ip = packet.ReadString();
	gameServerInfo.port = packet.ReadInt();
	gameServerInfo.map = packet.ReadString();
	gameServerInfo.gamerules = packet.ReadString();
	gameServerInfo.online = packet.ReadInt();
	gameServerInfo.maxPlayers = packet.ReadInt();
	gameServerInfo.status = static_cast<EFireNetGameServerStatus>(packet.ReadInt());
	gameServerInfo.currentPID = packet.ReadInt();

	if (!gameServerInfo.IsValid())
	{
		LogWarning("Wrong packet data! Some values empty!");
		LogDebug("Server name = %s, ip = %s, map = %s, gamerules = %s", gameServerInfo.name.toStdString().c_str(), gameServerInfo.ip.toStdString().c_str(), gameServerInfo.map.toStdString().c_str(), gameServerInfo.gamerules.toStdString().c_str());
		return;
	}

	gameServerInfo.name = _strFormat("%s_%s_%d", gameServerInfo.name.toStdString().c_str(), gameServerInfo.ip.toStdString().c_str(), gameServerInfo.port).c_str();

	if (gEnv->pConsole->GetBool("bUseTrustedServers") && !CheckInTrustedList(gameServerInfo.name, gameServerInfo.ip, gameServerInfo.port))
	{
		LogWarning("Game server <%s> not found in trusted server list. Registration not posible!", gameServerInfo.name.toStdString().c_str());

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::RegisterServerFail);
		m_packet.WriteErrorCode(EFireNetTcpErrorCode::GameServerNotFoundInTrustedList);
		m_connection->SendMessage(m_packet);
		return;
	}

	if (!gEnv->pRemoteServer->CheckGameServerExists(gameServerInfo.name, gameServerInfo.ip, gameServerInfo.port))
	{
		m_client->isGameServer = true;
		m_client->server->socket = m_client->socket;
		m_client->server->name = gameServerInfo.name;
		m_client->server->ip = gameServerInfo.ip;
		m_client->server->port = gameServerInfo.port;
		m_client->server->map = gameServerInfo.map;
		m_client->server->gamerules = gameServerInfo.gamerules;
		m_client->server->online = gameServerInfo.online;
		m_client->server->maxPlayers = gameServerInfo.maxPlayers;
		m_client->server->status = gameServerInfo.status;
		m_client->server->currentPID = gameServerInfo.currentPID;

		gEnv->pRemoteServer->UpdateClient(m_client);

		gEnv->m_GameServersRegistered++;

		CTcpPacket m_packet(EFireNetTcpPacketType::Result);
		m_packet.WriteResult(EFireNetTcpResult::RegisterServerComplete);
		m_connection->SendMessage(m_packet);

		LogInfo("Game server <%s> registered!", gameServerInfo.name.toStdString().c_str());

		connect(&m_Timer, &QTimer::timeout, this, &RemoteClientQuerys::OnUpdate);
		m_Timer.start(gEnv->pConsole->GetInt("remote_ping_timeout"));
		m_ServerAlive = true;
	}
	else
	{
		LogError("Can't register gamer server <%s>. Server with some information alredy registered!", gameServerInfo.name.toStdString().c_str());

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::RegisterServerFail);
		m_packet.WriteErrorCode(EFireNetTcpErrorCode::GameServerAlredyRegistered);
		m_connection->SendMessage(m_packet);
	}
}

void RemoteClientQuerys::onGameServerUpdateInfo(CTcpPacket &packet)
{
	if (!m_client->isGameServer)
	{
		LogWarning("Only registered game servers can update info");
		return;
	}

	SGameServer gameServerInfo;
	gameServerInfo.name = packet.ReadString();
	gameServerInfo.ip = packet.ReadString();
	gameServerInfo.port = packet.ReadInt();
	gameServerInfo.map = packet.ReadString();
	gameServerInfo.gamerules = packet.ReadString();
	gameServerInfo.online = packet.ReadInt();
	gameServerInfo.maxPlayers = packet.ReadInt();
	gameServerInfo.status = static_cast<EFireNetGameServerStatus>(packet.ReadInt());
	gameServerInfo.currentPID = packet.ReadInt();

	if (!gameServerInfo.IsValid())
	{
		LogWarning("Wrong packet data! Some values empty!");
		LogDebug("Server name = %s, ip = %s, map = %s, gamerules = %s", gameServerInfo.name.toStdString().c_str(), gameServerInfo.ip.toStdString().c_str(), gameServerInfo.map.toStdString().c_str(), gameServerInfo.gamerules.toStdString().c_str());
		return;
	}

	gameServerInfo.name = _strFormat("%s_%s_%d", gameServerInfo.name.toStdString().c_str(), gameServerInfo.ip.toStdString().c_str(), gameServerInfo.port).c_str();

	if (gEnv->pRemoteServer->CheckGameServerExists(gameServerInfo.name, gameServerInfo.ip, gameServerInfo.port))
	{
		if (m_client->server->status != gameServerInfo.status)
		{
			LogDebug("Game server <%s> update game status from <%s> to <%s>", gameServerInfo.name.toStdString().c_str(), GetStatusString(m_client->server->status), GetStatusString(gameServerInfo.status));
		}

		m_client->server->name = gameServerInfo.name;
		m_client->server->ip = gameServerInfo.ip;
		m_client->server->port = gameServerInfo.port;
		m_client->server->map = gameServerInfo.map;
		m_client->server->gamerules = gameServerInfo.gamerules;
		m_client->server->online = gameServerInfo.online;
		m_client->server->maxPlayers = gameServerInfo.maxPlayers;
		m_client->server->status = gameServerInfo.status;
		m_client->server->currentPID = gameServerInfo.currentPID;

		gEnv->pRemoteServer->UpdateClient(m_client);

		CTcpPacket Result(EFireNetTcpPacketType::Result);
		Result.WriteResult(EFireNetTcpResult::UpdateServerComplete);
		m_connection->SendMessage(Result);

		LogDebug("Game server <%s> updated info", gameServerInfo.name.toStdString().c_str());

		// When server finish game we need reload it
		if (gameServerInfo.status == EGStatus_GameFinished  && !bReloadRequestSended)
		{
			LogDebug("Game server <%s> finished game. Reload it...", gameServerInfo.name.toStdString().c_str());

			CTcpPacket ReloadRequest(EFireNetTcpPacketType::Query);
			ReloadRequest.WriteQuery(EFireNetTcpQuery::RequestServerReload);
			m_connection->SendMessage(ReloadRequest);

			bReloadRequestSended = true;
		}
		else
		{
			bReloadRequestSended = false;
		}
	}
	else
	{
		LogError("Update game server failed - server <%s> not found", gameServerInfo.name.toStdString().c_str());

		CTcpPacket Error(EFireNetTcpPacketType::Error);
		Error.WriteError(EFireNetTcpError::UpdateServerFail);
		Error.WriteErrorCode(EFireNetTcpErrorCode::GameServerNotRegistered);
		m_connection->SendMessage(Error);
	}
}

void RemoteClientQuerys::onGameServerGetOnlineProfile(CTcpPacket &packet)
{
	if (!m_client->isGameServer)
	{
		LogWarning("Only registered game servers can get online profiles");
		return;
	}

	int uid = packet.ReadInt();

	SProfile* pProfile = gEnv->pServer->GetProfileByUid(uid);

	if (pProfile != nullptr)
	{
		CTcpPacket profile(EFireNetTcpPacketType::Result);
		profile.WriteResult(EFireNetTcpResult::GetProfileComplete);
		profile.WriteInt(pProfile->uid);
		profile.WriteStdString(pProfile->nickname.toStdString());
		profile.WriteStdString(pProfile->fileModel.toStdString());
		profile.WriteInt(pProfile->lvl);
		profile.WriteInt(pProfile->xp);
		profile.WriteInt(pProfile->money);
		profile.WriteStdString(pProfile->items.toStdString());
#ifndef STEAM_SDK_ENABLED
		profile.WriteStdString(pProfile->friends.toStdString());
#endif

		m_connection->SendMessage(profile);
		return;
	}
	LogDebug("Failed get online profile");

	CTcpPacket m_packet(EFireNetTcpPacketType::Error);
	m_packet.WriteError(EFireNetTcpError::GetProfileFail);
	m_packet.WriteErrorCode(EFireNetTcpErrorCode::CantGetProfile);
	m_connection->SendMessage(m_packet);
}

void RemoteClientQuerys::onGameServerUpdateOnlineProfile(CTcpPacket &packet)
{
	if (!m_client->isGameServer)
	{
		LogWarning("Only registered game servers can update profiles");
		return;
	}

	int uid = packet.ReadInt();
	QString nickname = packet.ReadString();
	QString fileModel = packet.ReadString();
	int lvl = packet.ReadInt();
	int xp = packet.ReadInt();
	int money = packet.ReadInt();
	QString items = packet.ReadString();
#ifndef STEAM_SDK_ENABLED
	QString friends = packet.ReadString();
#endif

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
#ifndef STEAM_SDK_ENABLED
		pNewProfile->friends = pOldProfile->friends;
#endif

		if (gEnv->pServer->UpdateProfile(pNewProfile))
		{
			CTcpPacket m_packet(EFireNetTcpPacketType::Result);
			m_packet.WriteResult(EFireNetTcpResult::UpdateProfileComplete);
			m_connection->SendMessage(m_packet);
		}
		else
		{
			LogDebug("Failed update online profile!");

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::UpdateProfileFail);
			m_packet.WriteErrorCode(EFireNetTcpErrorCode::CantUpdateProfile);
			m_connection->SendMessage(m_packet);
		}
	}
	else
	{
		LogDebug("Failed get online profile");

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::UpdateProfileFail);
		m_packet.WriteErrorCode(EFireNetTcpErrorCode::CantGetProfile);
		m_connection->SendMessage(m_packet);
	}
}

void RemoteClientQuerys::onPingPong(CTcpPacket & packet)
{
	if (!m_client->isGameServer)
	{
		LogWarning("Only registered game servers can use ping pong");
		return;
	}

	m_ServerAlive = true;
}

void RemoteClientQuerys::onArbitratorRegister(CTcpPacket & packet)
{
	if (gEnv->pConsole->GetBool("bUseDedicatedArbitrators"))
	{
		if (gEnv->m_ArbitratorsCount >= gEnv->pConsole->GetInt("sv_max_arbitrators"))
		{
			LogWarning("Can't register dedicated arbitrator - Maximum arbitrators for registering = <%d>", gEnv->pConsole->GetInt("sv_max_arbitrators"));

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::RegisterArbitratorFail);
			m_packet.WriteErrorCode(EFireNetTcpErrorCode::ArbitratorBlockedRegister);
			m_connection->SendMessage(m_packet);

			return;
		}

		if (!m_client->isArbitrator)
		{
			LogDebug("Remote client with socket <%p> registered in arbitrator mode", m_socket);

			QString name = packet.ReadString();
			int maxGameServersCount = packet.ReadInt();
			int gameServersCount = packet.ReadInt();

			m_client->isArbitrator = true;
			m_client->pArbitrator->socket = m_client->socket;
			m_client->pArbitrator->name = name;
			m_client->pArbitrator->m_GameServersMaxCount = maxGameServersCount;
			m_client->pArbitrator->m_GameServersCount = gameServersCount;

			gEnv->pRemoteServer->UpdateClient(m_client);

			LogInfo("Arbitrator <%s> registered!", name.toStdString().c_str());

			gEnv->m_ArbitratorsCount++;

			CTcpPacket m_packet(EFireNetTcpPacketType::Result);
			m_packet.WriteResult(EFireNetTcpResult::RegisterArbitratorComplete);
			m_connection->SendMessage(m_packet);
		}
		else
		{
			LogWarning("Can't register dedicated arbitrator - this client alredy register in arbitrator mode");

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::RegisterArbitratorFail);
			m_packet.WriteErrorCode(EFireNetTcpErrorCode::ArbitratorDoubleRegistration);
			m_connection->SendMessage(m_packet);
		}
	}
	else
	{
		LogWarning("Can't register dedicated arbitrator - this future disabled");

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::RegisterArbitratorFail);
		m_packet.WriteErrorCode(EFireNetTcpErrorCode::ArbitratorFunctionalityDisabled);
		m_connection->SendMessage(m_packet);
	}
}

void RemoteClientQuerys::onArbitratorUpdate(CTcpPacket & packet)
{
	if (gEnv->pConsole->GetBool("bUseDedicatedArbitrators"))
	{
		if (m_client->isArbitrator)
		{
			LogDebug("Remote client with socket <%p> updated arbitrator data", m_socket);

			QString name = packet.ReadString();
			int maxGameServersCount = packet.ReadInt();
			int gameServersCount = packet.ReadInt();

			m_client->pArbitrator->name = name;
			m_client->pArbitrator->m_GameServersMaxCount = maxGameServersCount;
			m_client->pArbitrator->m_GameServersCount = gameServersCount;

			gEnv->pRemoteServer->UpdateClient(m_client);

			CTcpPacket m_packet(EFireNetTcpPacketType::Result);
			m_packet.WriteResult(EFireNetTcpResult::UpdateArbitratorComplete);
			m_connection->SendMessage(m_packet);
		}
		else
		{
			LogWarning("Can't update dedicated arbitrator - this client not registered in arbitrator mode");

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::UpdateArbitratorFail);
			m_packet.WriteErrorCode(EFireNetTcpErrorCode::ArbitratorNotRegistered);
			m_connection->SendMessage(m_packet);
		}
	}
	else
	{
		LogWarning("Can't update dedicated arbitrator - this future disabled");

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::UpdateArbitratorFail);
		m_packet.WriteErrorCode(EFireNetTcpErrorCode::ArbitratorFunctionalityDisabled);
		m_connection->SendMessage(m_packet);
	}
}

bool RemoteClientQuerys::CheckInTrustedList(const QString &name, const QString &ip, int port)
{
	std::vector<STrustedServer> m_server = gEnv->pScripts->GetTrustedList();

	if (m_server.size() > 0)
	{
		for (auto it = m_server.begin(); it != m_server.end(); ++it)
		{
			if (it->name == name && it->ip == ip && it->port == port)
			{
				LogDebug("Server <%s> found in trusted server list", name.toStdString().c_str());
				return true;
			}
		}
	}

	LogWarning("Server <%s> not found in trusted server list!", name.toStdString().c_str());
	return false;
}

const char * RemoteClientQuerys::GetStatusString(EFireNetGameServerStatus status) const
{
	static const char* const s_Statuses[] =
	{
		"EGStatus_Empty",           // EGStatus_Empty,
		"EGStatus_PreparingToPlay", // EGStatus_PreparingToPlay,
		"EGStatus_GameStarted",     // EGStatus_GameStarted,
		"EGStatus_GameFinished",    // EGStatus_GameFinished,
		"EGStatus_Reloading",       // EGStatus_Reloading,
		"EGStatus_WaitingPlayers",  // EGStatus_WaitingPlayers,
		"EGStatus_Unknown",         // EGStatus_Unknown,
	};

	try
	{
		return s_Statuses[status];
	}
	catch (const std::exception&)
	{
		return nullptr;
	}
}

void RemoteClientQuerys::OnConnectionClosed()
{
	m_Timer.stop();
}

void RemoteClientQuerys::OnUpdate()
{
	if (m_client->isGameServer && m_ServerAlive)
	{
		CTcpPacket ping(EFireNetTcpPacketType::Query);
		ping.WriteQuery(EFireNetTcpQuery::PingPong);
		m_connection->SendMessage(ping);

		m_ServerAlive = false;
	}
	else if (m_client->isGameServer && !m_ServerAlive)
	{
		// Server R.I.P.
		LogWarning("Game server <%s> R.I.P. Ping timeout = %d ms.", 
			m_client->server->name.toStdString().c_str(), 
			gEnv->pConsole->GetInt("remote_ping_timeout"));

		// Finish him!
		gEnv->pConsole->ExecuteCommand(_strFormat("killPID %d", m_client->server->currentPID).c_str());

		// Spawn new game server if need
		if (gEnv->pConsole->GetBool("bAutoSpawningGameSevers"))
		{
			QString exeArgs = _strFormat("spawnGameServer -dedicated -simple_console +sv_gamerules %s +map %s", m_client->server->gamerules.toStdString().c_str(), m_client->server->map.toStdString().c_str()).c_str();
			gEnv->pConsole->ExecuteCommand(exeArgs.toStdString().c_str());
		}

		m_Timer.stop();
	}
}