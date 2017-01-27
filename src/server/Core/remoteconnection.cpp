// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "remoteconnection.h"
#include "remoteserver.h"
#include "netpacket.h"

#include "Workers/Packets/remoteclientquerys.h"
#include "Tools/settings.h"

RemoteConnection::RemoteConnection(QObject *parent) : QObject(parent),
	m_socket(nullptr),
	pQuerys(nullptr),
	bConnected(false)
{

	m_maxPacketSize = gEnv->pSettings->GetVariable("net_max_packet_size_for_read").toInt();
	m_maxBadPacketsCount = gEnv->pSettings->GetVariable("net_max_bad_packet_count").toInt();
	m_BadPacketsCount = 0;
}

RemoteConnection::~RemoteConnection()
{
	qDebug() << "~RemoteConnection";
	SAFE_RELEASE(m_socket);
	SAFE_RELEASE(pQuerys);
}

void RemoteConnection::accept(qint64 socketDescriptor)
{
	m_socket = new QSslSocket(this);
	connect(m_socket, &QSslSocket::encrypted, this, &RemoteConnection::connected);
	connect(m_socket, &QSslSocket::disconnected, this, &RemoteConnection::disconnected);
	connect(m_socket, &QSslSocket::readyRead, this, &RemoteConnection::readyRead);
	connect(m_socket, &QSslSocket::bytesWritten, this, &RemoteConnection::bytesWritten);
	connect(m_socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(socketSslErrors(QList<QSslError>)));
	connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
	
	if (!m_socket->setSocketDescriptor(socketDescriptor))
	{
		qCritical() << "Can't accept socket!";
		return;
	}

	m_socket->setLocalCertificate("key.pem");
	m_socket->setPrivateKey("key.key");
	m_socket->startServerEncryption();
	
	int timeout = gEnv->pSettings->GetVariable("net_encryption_timeout").toInt();

	if (!m_socket->waitForEncrypted(timeout * 1000))
	{
		qCritical() << "Can't accept socket! Encryption timeout!";
		emit finished();
		return;
	}
}

void RemoteConnection::connected()
{
	if (!m_socket)
		return;

	m_Client.socket = m_socket;
	m_Client.server = nullptr;
	m_Client.isAdmin = false;
	m_Client.isGameServer = false;

	// Add client to remote server client list
	gEnv->pRemoteServer->AddNewClient(m_Client);

	pQuerys = new RemoteClientQuerys(this);
	pQuerys->SetSocket(m_socket);
	pQuerys->SetClient(&m_Client);

	bConnected = true;

	qInfo() << "Remote client" << m_socket << "connected.";
	qInfo() << "Remote client count " << gEnv->pRemoteServer->GetClientCount();
}

void RemoteConnection::disconnected()
{
	if (!m_socket || !bConnected)
	{
		emit finished();
		return;
	}
		

	if (gEnv->pRemoteServer->IsHaveAdmin() && m_Client.isAdmin)
	{
		gEnv->pRemoteServer->SetAdmin (false);
		qWarning() << "Remote administrator disconnected!";
	}

	// Remove client from server client list
	gEnv->pRemoteServer->RemoveClient(m_Client);	

	qInfo() << "Remote client" << m_socket << "disconnected.";
	qInfo() << "Remote client count " << gEnv->pRemoteServer->GetClientCount();

	emit finished();
}

void RemoteConnection::readyRead()
{
	if (!m_socket)
		return;

	gEnv->m_InputPacketsCount++;

	// If client send a lot bad packet we need disconnect him
	if (m_BadPacketsCount >= m_maxBadPacketsCount)
	{
		qWarning() << "Exceeded the number of bad packets from a client. Connection will be closed" << m_socket;
		close();
		return;
	}

	qDebug() << "Read message from client" << m_socket;
	qDebug() << "Available bytes for read" << m_socket->bytesAvailable();

	// Check bytes count before reading
	if (m_socket->bytesAvailable() > m_maxPacketSize)
	{
		qWarning() << "Very big packet from client" << m_socket;
		m_BadPacketsCount++;
		return;
	}
	else if (m_socket->bytesAvailable() <= 0)
	{
		qDebug() << "Very small packet from client" << m_socket;
		m_BadPacketsCount++;
		return;
	}

	qDebug() << "Read message from remote client" << m_socket;

	NetPacket packet(m_socket->readAll());

	if (packet.getType() == net_Query)
	{
		switch ((ENetPacketQueryType)packet.ReadInt())
		{
		case net_query_remote_admin_login:
		{
			pQuerys->onAdminLogining(packet);
			break;
		}
		case net_query_remote_server_command:
		{
			pQuerys->onConsoleCommandRecived(packet);
			break;
		}
		case net_query_remote_register_server:
		{
			pQuerys->onGameServerRegister(packet);
			break;
		}
		case net_query_remote_update_server:
		{
			pQuerys->onGameServerUpdateInfo(packet);
			break;
		}
		case net_query_remote_get_profile:
		{
			pQuerys->onGameServerGetOnlineProfile(packet);
			break;
		}
		case net_query_remote_update_profile:
		{
			pQuerys->onGameServerUpdateOnlineProfile(packet);
			break;
		}
		default:
		{
			qCritical() << "Error reading query. Can't get query type!";
			break;
		}
		}
	}
	else
	{
		qCritical() << "Error reading packet. Can't get packet type!";
	}
}

void RemoteConnection::bytesWritten(qint64 bytes)
{
	if (!m_socket)
		return;

	gEnv->m_OutputPacketsCount++;

	qDebug() << "Message to remote client" << m_socket << "sended! Size =" << bytes;
}

void RemoteConnection::socketSslErrors(const QList<QSslError> list)
{
	foreach(QSslError item, list)
	{
		qCritical() << "Client" << m_socket << "return socket error" << item.errorString();
	}
}

void RemoteConnection::socketError(QAbstractSocket::SocketError error)
{
	if (error == QAbstractSocket::SocketError::RemoteHostClosedError)
		close();
	else
	{
		qCritical() << "Client" << m_socket << "return socket error" << error;
		close();
	}
}

void RemoteConnection::close()
{
	if (!m_socket)
		return;

	m_socket->close();
}
