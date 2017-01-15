// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "remoteserver.h"
#include "remoteconnection.h"
#include "netpacket.h"

#include "Tools/settings.h"

RemoteServer::RemoteServer(QObject *parent) : QTcpServer(parent)
{
	m_Server = nullptr;
	m_Thread = nullptr;
	bHaveAdmin = false;
}

void RemoteServer::Clear()
{
	qInfo() << "~RemoteServer";

	m_connections.clear();
	m_Clients.clear();

	m_Server->deleteLater();
}

void RemoteServer::Update()
{
}

void RemoteServer::run()
{
	if (CreateServer())
	{
		m_Thread = QThread::currentThread();

		qInfo() << "Remote server started on" << gEnv->pSettings->GetVariable("sv_ip").toString();
		qInfo() << "Remote server thread " << m_Thread;
	}
	else
	{
		qCritical() << "Failed start remote server! Reason = " << m_Server->errorString();
		return;
	}
}

bool RemoteServer::CreateServer()
{
	m_Server = new QTcpServer(this);
	return QTcpServer::listen(QHostAddress(gEnv->pSettings->GetVariable("sv_ip").toString()), 
		gEnv->pSettings->GetVariable("remote_server_port").toInt());
}

void RemoteServer::incomingConnection(qintptr socketDescriptor)
{
	qInfo() << "New incomining connection to remote server. Try accept...";
	
	RemoteConnection* m_remoteConnection = new RemoteConnection();
	connect(this, &RemoteServer::close, m_remoteConnection, &RemoteConnection::close);
	connect(m_remoteConnection, &RemoteConnection::finished, this, &RemoteServer::CloseConnection);

	m_connections.append(m_remoteConnection);
	m_remoteConnection->accept(socketDescriptor);
}

void RemoteServer::sendMessageToRemoteClient(QSslSocket * socket, NetPacket &paket)
{
	if (socket != nullptr)
	{
		socket->write(paket.toString());
		socket->waitForBytesWritten(10);
	}
}

void RemoteServer::AddNewClient(SRemoteClient client)
{
	if (client.socket == nullptr)
	{
		qWarning() << "Can't add remote client. Remote client socket = nullptr";
		return;
	}

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->socket == client.socket)
		{
			qWarning() << "Can't add remote client" << client.socket << ". Remote client alredy added";
			return;
		}
	}

	qDebug() << "Adding new remote client" << client.socket;
	m_Clients.push_back(client);
}

void RemoteServer::RemoveClient(SRemoteClient client)
{
	if (client.socket == nullptr)
	{
		qWarning() << "Can't remove  remove client. Remove client socket = nullptr";
		return;
	}

	if (m_Clients.size() > 0)
	{
		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			if (it->socket == client.socket)
			{
				qDebug() << "Removing remote client" << client.socket;
				m_Clients.erase(it);
				return;
			}
		}
	}

	qWarning() << "Can't remove remote client. Remote client not found";
}

void RemoteServer::UpdateClient(SRemoteClient* client)
{
	if (client->socket == nullptr)
	{
		qWarning() << "Can't update client. Client socket = nullptr";
		return;
	}

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->socket == client->socket)
		{
			it->isGameServer = client->isGameServer;
			it->isAdmin = client->isAdmin;

			if (client->server != nullptr)
			{	
				it->server = client->server;			
			}

			qDebug() << "Remote client" << it->socket << "updated.";
			return;
		}
	}

	qWarning() << "Can't update client. Client" << client->socket << "not found";
}

bool RemoteServer::CheckGameServerExists(QString name, QString ip, int port)
{
	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->server != nullptr)
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
	return m_Clients.size();
}

QStringList RemoteServer::GetServerList()
{
	QStringList serverList;

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->server != nullptr && it->isGameServer)
		{
			serverList.push_back(it->server->name + " <" + it->server->ip + ":" + QString::number(it->server->port) + ">"
			" Map <" + it->server->map + ":" + it->server->gamerules + ">"
			" Online <" + QString::number(it->server->online) + "/" + QString::number(it->server->maxPlayers) + ">");
		}
	}

	return serverList;
}

SGameServer* RemoteServer::GetGameServer(QString name, QString map, QString gamerules)
{
	SGameServer gameServer;
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
			// by map
			if (byMap && it->server->map == map)
				return it->server;
			// by gamerules
			if (byGameRules && it->server->gamerules == gamerules)
				return it->server;
			// by name
			if (byName && it->server->name == name)
				return it->server;
		}
	}

	return nullptr;
}

void RemoteServer::CloseConnection()
{
	if (!QObject::sender())
	{
		qCritical() << "Sender is not a QObject*";
		return;
	}

	RemoteConnection *connection = qobject_cast<RemoteConnection*>(QObject::sender());
	if (!connection)
	{
		qCritical() << "Sender is not a RemoteConnection*";
		return;
	}

	m_connections.removeOne(connection);
	connection->deleteLater();
}