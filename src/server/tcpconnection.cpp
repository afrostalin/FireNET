// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "tcpconnection.h"
#include <QXmlStreamReader>
#include "global.h"
#include "clientquerys.h"

TcpConnection::TcpConnection(QObject *parent) : QObject(parent)
{
	pQuery = new ClientQuerys();
	m_socket = nullptr;
}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::connected()
{
    if(!m_socket) return;

	SClient client;
	client.socket = m_socket;
	client.profile = nullptr;
	client.status = 0;
	
	vClients.push_back(client);

	pQuery->SetSocket(m_socket);
	
	qDebug() << "[TcpConnection] Online = " << vClients.size();
}

void TcpConnection::disconnected()
{
	if (!m_socket) return;

	QVector<SClient>::iterator it;
	for (it = vClients.begin(); it != vClients.end(); ++it)
	{
		if (it->socket == m_socket)
		{
			vClients.erase(it);
			qDebug() << "[TcpConnection] Online = " << vClients.size();
			break;
		}
	}

	emit finished();
}

void TcpConnection::readyRead()
{
    if(!m_socket) 
		return;

	if (!pRedis->connectStatus)
	{
        qDebug() << "[ClientQuerys] Client can't use database functions without connection to Redis!!!";
		return;
	}
	QByteArray bytes;

    qDebug() << "[TcpConnection] Read message from client...";

	bytes = m_socket->readAll();

    QXmlStreamReader xml(bytes);

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

                qDebug() << "[TcpConnection] Query type = " << type;

				// Authorization
                if(type == "auth")
                    pQuery->onLogin(bytes);
                if(type == "register")
                    pQuery->onRegister(bytes);
				// Profile
                if(type == "create_profile")
                    pQuery->onCreateProfile(bytes);
                if(type == "get_profile")
                    pQuery->onGetProfile(bytes);
				// Items
                if(type == "get_shop_items")
                    pQuery->onGetShopItems(bytes);
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

            }

        }
    }
}

void TcpConnection::bytesWritten(qint64 bytes)
{
    if(!m_socket) return;
    //qDebug() << "[TcpConnection] Send message to client";
}

void TcpConnection::stateChanged(QAbstractSocket::SocketState socketState)
{
    if(!m_socket) return;
    //qDebug() << "[TcpConnection] Socket state changed";
}

void TcpConnection::accept(qint64 socketDescriptor)
{
	m_socket = new QSslSocket(this);

	if (!m_socket->setSocketDescriptor(socketDescriptor))
	{
		qDebug() << "[TcpConnection] Can't accept socket!";
		return;
	}

	m_socket->setLocalCertificate("key.pem");
	m_socket->setPrivateKey("key.key");
	m_socket->startServerEncryption();

	connect(m_socket, &QSslSocket::encrypted, this, &TcpConnection::connected);
	connect(m_socket, &QSslSocket::disconnected, this, &TcpConnection::disconnected);
	connect(m_socket, &QSslSocket::readyRead, this, &TcpConnection::readyRead);
	connect(m_socket, &QSslSocket::bytesWritten, this, &TcpConnection::bytesWritten);

	connect(m_socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(socketSslErrors(QList<QSslError>)));
	connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
}

void TcpConnection::socketSslErrors(const QList<QSslError> list)
{
	qDebug() << "[TcpConnection] Soket ssl error";
	foreach(QSslError item, list) {
		qDebug() << item.errorString();
	}
}

void TcpConnection::socketError(QAbstractSocket::SocketError error)
{
    qDebug() << "[TcpConnection] SocketError: " << error;
}

void TcpConnection::close()
{
    if(!m_socket) return;
    m_socket->close();
}

