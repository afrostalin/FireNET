// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "global.h"
#include "remoteserver.h"
#include "remoteconnection.h"
#include "settings.h"

#include <QDebug>

RemoteServer::RemoteServer(QObject *parent) : QTcpServer(parent)
{
	m_Server = nullptr;
	m_Thread = nullptr;
	bHaveAdmin = false;
}

void RemoteServer::Update()
{
}

void RemoteServer::run()
{
	if (CreateServer())
	{
		m_Thread = QThread::currentThread();

		qInfo() << "Remote server started on" << gEnv->pSettings->GetVariable("remote_server_ip").toString();
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
	return QTcpServer::listen(QHostAddress(gEnv->pSettings->GetVariable("remote_server_ip").toString()), 
		gEnv->pSettings->GetVariable("remote_server_port").toInt());
}

void RemoteServer::incomingConnection(qintptr socketDescriptor)
{
	qInfo() << "New incomining connection to remote server. Try accept...";
	
	RemoteConnection* m_remoteConnection = new RemoteConnection();
	connect(this, &QTcpServer::close, m_remoteConnection, &RemoteConnection::close);
	connect(m_remoteConnection, &RemoteConnection::finished, this, &RemoteServer::CloseConnection);

	m_connections.append(m_remoteConnection);
	m_remoteConnection->accept(socketDescriptor);
}

void RemoteServer::sendMessageToRemoteClient(QSslSocket * socket, QByteArray data)
{
	qDebug() << "Send message to remote client. Original size = " << data.size();
	socket->write(data);
	socket->waitForBytesWritten(3);
}

void RemoteServer::AddNewClient(SRemoteClient client)
{
	QMutexLocker locker(&m_Mutex);

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
	QMutexLocker locker(&m_Mutex);

	if (client.socket == nullptr)
	{
		qWarning() << "Can't remove  remove client. Remove client socket = nullptr";
		return;
	}

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->socket == client.socket)
		{
			qDebug() << "Removing remote client" << client.socket;
			m_Clients.erase(it);
			return;
		}
	}

	qWarning() << "Can't remove remote client. Remote client" << client.socket << "not found";
}

void RemoteServer::UpdateClient(SRemoteClient* client)
{
	QMutexLocker locker(&m_Mutex);

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

int RemoteServer::GetClientCount()
{
	QMutexLocker locker(&m_Mutex);
	return m_Clients.size();
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