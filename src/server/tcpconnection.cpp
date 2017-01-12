// Copyright (Ñ) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "tcpconnection.h"
#include "clientquerys.h"
#include "mysqlconnector.h"
#include "dbworker.h"
#include "tcpserver.h"
#include "settings.h"
#include "netpacket.h"

TcpConnection::TcpConnection(QObject *parent) : QObject(parent)
{
	pQuery = nullptr;
	m_Socket = nullptr;
	bConnected = true;
}

TcpConnection::~TcpConnection()
{
	qDebug() << "~TcpConnection";
}

void TcpConnection::accept(qint64 socketDescriptor)
{
	m_Socket = new QSslSocket(this);
	connect(m_Socket, &QSslSocket::encrypted, this, &TcpConnection::connected);
	connect(m_Socket, &QSslSocket::disconnected, this, &TcpConnection::disconnected);
	connect(m_Socket, &QSslSocket::readyRead, this, &TcpConnection::readyRead);
	connect(m_Socket, &QSslSocket::bytesWritten, this, &TcpConnection::bytesWritten);
	connect(m_Socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(socketSslErrors(QList<QSslError>)));
	connect(m_Socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));

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
		emit finished();
		return;
	}
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
	qInfo() << "Client count " << gEnv->pServer->GetClientCount();
}

void TcpConnection::disconnected()
{
	if (!m_Socket || !bConnected)
	{
		emit finished();
		return;
	}

	// Remove client from server client list
	gEnv->pServer->RemoveClient(m_Client);

	qInfo() << "Client" << m_Socket << "disconnected.";
	qInfo() << "Client count " << gEnv->pServer->GetClientCount();

	emit finished();
}

void TcpConnection::readyRead()
{
    if(!m_Socket)
		return;

    qDebug() << "Read message from client" << m_Socket;

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
}

void TcpConnection::bytesWritten(qint64 bytes)
{
    if(!m_Socket)
		return;

    qDebug() << "Message to client" << m_Socket << "sended! Size =" << bytes;
}

void TcpConnection::stateChanged(QAbstractSocket::SocketState socketState)
{
    if(!m_Socket)
		return;

    qDebug() << "Client" << m_Socket << "changed socket state to " << socketState;
}

void TcpConnection::socketSslErrors(const QList<QSslError> list)
{
	foreach(QSslError item, list)
	{
		qCritical() << "Client" << m_Socket << "return socket error" << item.errorString();
	}
}

void TcpConnection::socketError(QAbstractSocket::SocketError error)
{
	if (error == QAbstractSocket::SocketError::RemoteHostClosedError)
		close();
	else
	{
		qCritical() << "Client" << m_Socket << "return socket error" << error;
		close();
	}
}

void TcpConnection::close()
{
    if(!m_Socket)
		return;

	m_Socket->close();
}