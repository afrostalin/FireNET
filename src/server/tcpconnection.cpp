// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "global.h"
#include "tcpconnection.h"
#include "clientquerys.h"
#include "mysqlconnector.h"
#include "dbworker.h"
#include "tcpserver.h"
#include "settings.h"

#include <QXmlStreamReader>

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

	// Create client querts worker
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

	QByteArray bytes = m_Socket->readAll();
    QXmlStreamReader xml(bytes);

	// Check message type
    xml.readNext();
    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartDocument)
            continue;
        if (token == QXmlStreamReader::StartElement)
        {
            if (xml.name() == "query")
            {
                QXmlStreamAttributes attributes = xml.attributes();
                QString type = attributes.value("type").toString();

                qDebug() << "Query type = " << type;
				qDebug() << "Query data = " << bytes;

				// Authorization
                if(type == "auth")
                    pQuery->onLogin(bytes);
                if(type == "register")
                    pQuery->onRegister(bytes);
				// Profile
                if(type == "create_profile")
                    pQuery->onCreateProfile(bytes);
                if(type == "get_profile")
                    pQuery->onGetProfile();
				// Shop
                if(type == "get_shop_items")
                    pQuery->onGetShopItems();
				// Items
                if(type == "buy_item")
                    pQuery->onBuyItem(bytes);
				if (type == "remove_item")
					pQuery->onRemoveItem(bytes);
				// Invites
				if (type == "invite")
					pQuery->onInvite(bytes);
				if (type == "decline_invite")
					pQuery->onDeclineInvite(bytes);
				// Friends
                if(type == "add_friend")
                    pQuery->onAddFriend(bytes);
                if(type == "remove_friend")
                    pQuery->onRemoveFriend(bytes);
				// Chat
				if (type == "chat_message")
					pQuery->onChatMessage(bytes);
				// Matchmaking
				if (type == "get_game_server")
					pQuery->onGetGameServer(bytes);


				return;
            }

        }
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

