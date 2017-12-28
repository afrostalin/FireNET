// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QTimer>

#include "global.h"
#include "remoteserver.h"
#include "remoteconnection.h"
#include "tcppacket.h"
#include "tcpserver.h"

#include "Tools/console.h"
#include "Workers/Packets/remoteclientquerys.h"

RemoteServer::RemoteServer(QObject *parent) 
	: QTcpServer(parent)
	, m_Server(nullptr)
	, m_MaxClinetCount(0)
	, bHaveAdmin(false)
{
}

RemoteServer::~RemoteServer()
{
	SAFE_RELEASE(m_Server);
}

void RemoteServer::Clear()
{
	emit close();
	QTcpServer::close();

	m_connections.clear();
	m_Clients.clear();
}

void RemoteServer::Update() const
{
}

void RemoteServer::run()
{
	if (CreateServer())
	{
		gEnv->m_ServerStatus.m_RemoteServerStatus = "Online";

		LogInfo("[RemoteServer] Server started on %s : %d", gEnv->pConsole->GetString("sv_ip").c_str(), gEnv->pConsole->GetInt("remote_server_port"));
		LogInfo("[RemoteServer] Server thread <%d>", this->thread()->currentThreadId());
	}
	else
	{
		LogError("[RemoteServer] Failed start remote server! Reason = %s", m_Server->errorString().toStdString().c_str());
		return;
	}
}

bool RemoteServer::CreateServer()
{
	m_Server = new QTcpServer(this);
	return listen(QHostAddress(gEnv->pConsole->GetString("sv_ip").c_str()), 
		gEnv->pConsole->GetInt("remote_server_port"));
}

void RemoteServer::incomingConnection(qintptr socketDescriptor)
{
	LogDebug("[RemoteServer] New incomining connection to remote server. Try accept...");
	
	RemoteConnection* m_remoteConnection = new RemoteConnection();
	connect(this, &RemoteServer::close, m_remoteConnection, &RemoteConnection::close);
	connect(m_remoteConnection, &RemoteConnection::finished, this, &RemoteServer::CloseConnection);

	connect(m_remoteConnection, &RemoteConnection::received, gEnv->pServer, &TcpServer::MessageReceived);
	connect(m_remoteConnection, &RemoteConnection::sended, gEnv->pServer, &TcpServer::MessageSended);
	connect(gEnv->pTimer, &QTimer::timeout, m_remoteConnection, &RemoteConnection::Update);

	m_connections.append(m_remoteConnection);
	m_remoteConnection->accept(socketDescriptor);
}

void RemoteServer::sendMessageToRemoteClient(QSslSocket * socket, CTcpPacket &paket) const
{
	if (socket)
	{
		socket->write(paket.toString());
	}
}

void RemoteServer::SendMessageToAllRemoteClients(CTcpPacket & packet)
{
	const char* data = packet.toString();

	for (const auto &it : m_Clients)
	{
		if (it.socket)
		{
			it.socket->write(data);
		}
	}
}

void RemoteServer::AddNewClient(SRemoteClient &client)
{
	QMutexLocker locker(&m_Mutex);

	if (client.socket == nullptr)
	{
		LogWarning("[RemoteServer] Can't add remote client. Remote client socket = nullptr");
		return;
	}

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->socket == client.socket)
		{
			LogWarning("[RemoteServer] Can't add remote client with socket <%p> (%s:%d). Remote client alredy added", client.socket, client.GetAddress().ip.c_str(), client.GetAddress().port);
			return;
		}
	}

	LogDebug("[RemoteServer] Adding new remote client with socket <%p> (%s:%d)", client.socket, client.GetAddress().ip.c_str(), client.GetAddress().port);
	m_Clients.push_back(client);
}

void RemoteServer::RemoveClient(SRemoteClient &client)
{
	QMutexLocker locker(&m_Mutex);

	if (client.socket == nullptr)
	{
		LogWarning("[RemoteServer] Can't remove  remove client. Remove client socket = nullptr");
		return;
	}

	if (m_Clients.size() > 0)
	{
		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			if (it->socket == client.socket)
			{
				LogDebug("[RemoteServer] Removing remote client with socket <%p> (%s:%d)", client.socket, client.GetAddress().ip.c_str(), client.GetAddress().port);
				m_Clients.erase(it);
				return;
			}
		}
	}

	LogWarning("[RemoteServer] Can't remove remote client. Remote client not found");
}

void RemoteServer::UpdateClient(SRemoteClient* client)
{
	QMutexLocker locker(&m_Mutex);

	if (client->socket == nullptr)
	{
		LogWarning("[RemoteServer] Can't update client. Client socket = nullptr");
		return;
	}

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->socket == client->socket)
		{
			it->isGameServer = client->isGameServer;
			it->isAdmin = client->isAdmin;
			it->pQuerys = client->pQuerys;
			it->pArbitrator = client->pArbitrator;

			if (client->server != nullptr)
			{	
				it->server = client->server;			
			}

			LogDebug("[RemoteServer] Remote client with socket <%p> (%s:%d) updated.", it->socket, client->GetAddress().ip.c_str(), client->GetAddress().port);
			return;
		}
	}

	LogDebug("[RemoteServer] Can't update client with socket <%p> (%s:%d). Client not found", client->socket, client->GetAddress().ip.c_str(), client->GetAddress().port);
}

bool RemoteServer::CheckGameServerExists(const QString &name, const QString &ip, int port)
{
	QMutexLocker locker(&m_Mutex);

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->server)
		{
			if ((it->server->ip == ip && it->server->port == port) || it->server->name == name)
			{
				return true;
			}
		}
	}

	return false;
}

int RemoteServer::GetClientCount()
{
	QMutexLocker locker(&m_Mutex);
	return bHaveAdmin ? static_cast<int>(m_Clients.size()) - 1 : static_cast<int>(m_Clients.size());
}

std::vector<std::string> RemoteServer::DumpServerList()
{
	std::vector<std::string> serverList;

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->server != nullptr && it->isGameServer)
		{
			std::string serverStatus = it->pQuerys ? it->pQuerys->GetStatusString(it->server->status) : "Unknown";
			const std::string serverItem = _strFormat("[%s:%d] - [%s:%s] - [%d] - [%d/%d] - [%s]",
				it->server->ip.toStdString().c_str(), 
				it->server->port,
				it->server->map.toStdString().c_str(),
				it->server->gamerules.toStdString().c_str(), 
				it->server->currentPID, 
				it->server->online,
				it->server->maxPlayers,
				serverStatus.c_str());

			serverList.push_back(serverItem);
		}
	}

	return serverList;
}

SGameServer* RemoteServer::GetGameServer(const QString &name, const QString &map, const QString &gamerules, bool onlyEmpty)
{
	bool byMap = false;
	bool byGameRules = false;
	bool byName = false;

	if (!map.isEmpty())
		byMap = true;
	if (!gamerules.isEmpty() && !byMap)
		byGameRules = true;
	if (!name.isEmpty() && !byMap && !byGameRules)
		byName = true;

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->server != nullptr && it->isGameServer)
		{
			if (!onlyEmpty)
			{
				// by map
				if (byMap && _qstricmp(it->server->map, map))
					return it->server;
				// by gamerules
				if (byGameRules && _qstricmp(it->server->gamerules, gamerules))
					return it->server;
				// by name
				if (byName && _qstricmp(it->server->name, name))
					return it->server;
			}
			else
			{
				// by map
				if (byMap && _qstricmp(it->server->map, map) && it->server->online == 0 && it->server->status == EGStatus_Empty)
					return it->server;
				// by gamerules
				if (byGameRules && _qstricmp(it->server->gamerules, gamerules) && it->server->online == 0 && it->server->status == EGStatus_Empty)
					return it->server;
				// by name
				if (byName && _qstricmp(it->server->name, name) && it->server->online == 0 && it->server->status == EGStatus_Empty)
					return it->server;
			}
		}
	}

	return nullptr;
}

void RemoteServer::GetAllGameServers(std::vector<SGameServer>& list)
{
	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->server != nullptr && it->isGameServer)
		{
			list.push_back(*it->server);
		}
	}
}

void RemoteServer::CloseConnection()
{
	if (sender() == nullptr)
	{
		return;
	}

	RemoteConnection* connection = qobject_cast<RemoteConnection*>(sender());
	if (connection == nullptr)
	{
		return;
	}

	m_connections.removeOne(connection);

	SAFE_RELEASE(connection);
}