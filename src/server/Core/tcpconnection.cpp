// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "tcpconnection.h"
#include "tcpserver.h"
#include "netpacket.h"

#include "Workers/Packets/clientquerys.h"
#include "Workers/Databases/mysqlconnector.h"
#include "Workers/Databases/dbworker.h"
#include "Tools/settings.h"

TcpConnection::TcpConnection(QObject *parent) : QObject(parent),
	pQuery(nullptr),
	m_Socket(nullptr),
	bConnected(nullptr)
{
	Q_UNUSED(parent);
	Active();
}

TcpConnection::~TcpConnection()
{
	qDebug() << "~TcpConnection";
	SAFE_RELEASE(m_Socket);
	SAFE_RELEASE(pQuery);
}

int TcpConnection::IdleTime()
{
	return m_activity.secsTo(QTime::currentTime());
}

void TcpConnection::quit()
{
	qDebug() << "Quit called, closing client";

	m_Socket->close();
	emit closed();
}

void TcpConnection::accept(qint64 socketDescriptor)
{
	qDebug() << "Accepting new client...";

	m_Socket = CreateSocket();

	if (!m_Socket)
	{
		qWarning() << "Could not find created socket!";
	}

	if (!m_Socket->setSocketDescriptor(socketDescriptor))
	{
		qCritical() << "Can't accept socket!";
		return;
	}

	m_Socket->setLocalCertificate("key.pem");
	m_Socket->setPrivateKey("key.key");
	m_Socket->startServerEncryption();

	int timeout = gEnv->pSettings->GetVariable("net_encryption_timeout").toInt();

	if (!m_Socket->waitForEncrypted(timeout * 1000))
	{
		qCritical() << "Can't accept socket! Encryption timeout!";
		emit quit();
		return;
	}

	qDebug() << "Client accepted. Socket " << m_Socket;
	m_activity = QTime::currentTime();
}

void TcpConnection::connected()
{
    if(!m_Socket)
		return;

	m_Client.socket = m_Socket;
	m_Client.profile = nullptr;
	m_Client.status = 0;	

	// Add client to server client list
	gEnv->pServer->AddNewClient(m_Client);

	// Create client querys worker
	pQuery = new ClientQuerys(this);
	// Set socket for client querys worker
	pQuery->SetSocket(m_Socket);
	// Set client
	pQuery->SetClient(&m_Client);

	bConnected = true;

	qInfo() << "Client" << m_Socket << "connected.";

	m_activity = QTime::currentTime();
	emit opened();
}

void TcpConnection::disconnected()
{
	if (!m_Socket || !bConnected)
	{
		emit closed();
		return;
	}

	// Remove client from server client list
	gEnv->pServer->RemoveClient(m_Client);

	qInfo() << "Client" << m_Socket << "disconnected.";

	m_activity = QTime::currentTime();
	emit closed();
}

void TcpConnection::readyRead()
{
    if(!m_Socket)
		return;

    qDebug() << "Read message from client" << m_Socket;
	qDebug() << "Available bytes for read" << m_Socket->bytesAvailable();

	NetPacket packet(m_Socket->readAll());

	if(packet.getType() == net_Query)
	{
		switch ((ENetPacketQueryType)packet.ReadInt())
		{
		case net_query_auth :
		{
			pQuery->onLogin(packet);
			break;
		}
		case net_query_register :
		{
			pQuery->onRegister(packet);
			break;
		}
		case net_query_create_profile :
		{
			pQuery->onCreateProfile(packet);
			break;
		}
		case net_query_get_profile :
		{
			pQuery->onGetProfile();
			break;
		}
		case net_query_get_shop :
		{
			pQuery->onGetShopItems();
			break;
		}
		case net_query_buy_item :
		{
			pQuery->onBuyItem(packet);
			break;
		}
		case net_query_remove_item :
		{
			pQuery->onRemoveItem(packet);
			break;
		}		
		case net_query_send_invite :
		{
			pQuery->onInvite(packet);
			break;
		}
		case net_query_decline_invite :
		{
			pQuery->onDeclineInvite(packet);
			break;
		}
		case net_query_accept_invite :
		{
			break;
		}
		case net_query_remove_friend :
		{
			pQuery->onRemoveFriend(packet);
			break;
		}
		case net_query_send_chat_msg :
		{
			pQuery->onChatMessage(packet);
			break;
		}
		case net_query_get_server :
		{
			pQuery->onGetGameServer(packet);
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

	Active();
}

void TcpConnection::bytesWritten(qint64 bytes)
{
    if(!m_Socket)
		return;

    qDebug() << "Message to client" << m_Socket << "sended! Size =" << bytes;

	Active();
}

void TcpConnection::stateChanged(QAbstractSocket::SocketState socketState)
{
    if(!m_Socket)
		return;

    qDebug() << "Client" << m_Socket << "changed socket state to " << socketState;

	Active();
}

void TcpConnection::socketSslErrors(const QList<QSslError> list)
{
	foreach(QSslError item, list)
	{
		qCritical() << "Client" << m_Socket << "return socket error" << item.errorString();
	}

	Active();
}

void TcpConnection::socketError(QAbstractSocket::SocketError error)
{
	if (error == QAbstractSocket::SocketError::RemoteHostClosedError)
		quit();
	else
	{
		qCritical() << "Client" << m_Socket << "return socket error" << error;
		quit();
	}

	Active();
}

void TcpConnection::Active()
{
	m_activity = QTime::currentTime();
}

QSslSocket * TcpConnection::CreateSocket()
{
	qDebug() << "Creating socket for client";

	QSslSocket *socket = new QSslSocket(this);
	connect(socket, &QSslSocket::encrypted, this, &TcpConnection::connected, Qt::QueuedConnection);
	connect(socket, &QSslSocket::disconnected, this, &TcpConnection::disconnected, Qt::QueuedConnection);
	connect(socket, &QSslSocket::readyRead, this, &TcpConnection::readyRead, Qt::QueuedConnection);
	connect(socket, &QSslSocket::bytesWritten, this, &TcpConnection::bytesWritten, Qt::QueuedConnection);
	connect(socket, &QSslSocket::stateChanged, this, &TcpConnection::stateChanged, Qt::QueuedConnection);
	connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(socketSslErrors(QList<QSslError>)));
	connect(socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &TcpConnection::socketError, Qt::QueuedConnection);

	return socket;
}