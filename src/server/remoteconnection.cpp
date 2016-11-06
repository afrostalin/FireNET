// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "global.h"
#include "remoteconnection.h"
#include "remoteserver.h"
#include "remoteclientquerys.h"

#include <QXmlStreamReader>

RemoteConnection::RemoteConnection(QObject *parent) : QObject(parent)
{
	m_socket = nullptr;
	pQuerys = nullptr;
}

RemoteConnection::~RemoteConnection()
{
	qDebug() << "~RemoteConnection";
}

void RemoteConnection::accept(qint64 socketDescriptor)
{
	m_socket = new QSslSocket(this);
	
	if (!m_socket->setSocketDescriptor(socketDescriptor))
	{
		qCritical() << "Can't accept socket!";
		return;
	}

	m_socket->setLocalCertificate("key.pem");
	m_socket->setPrivateKey("key.key");
	m_socket->startServerEncryption();

	connect(m_socket, &QSslSocket::encrypted, this, &RemoteConnection::connected);
	connect(m_socket, &QSslSocket::disconnected, this, &RemoteConnection::disconnected);
	connect(m_socket, &QSslSocket::readyRead, this, &RemoteConnection::readyRead);
	connect(m_socket, &QSslSocket::bytesWritten, this, &RemoteConnection::bytesWritten);
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

	qInfo() << "Remote client" << m_socket << "connected.";
	qInfo() << "Remote client count " << gEnv->pRemoteServer->GetClientCount();
}

void RemoteConnection::disconnected()
{
	if (!m_socket)
		return;

	if (gEnv->pRemoteServer->bHaveAdmin && m_Client.isAdmin)
	{
		gEnv->pRemoteServer->bHaveAdmin = false;
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

	qDebug() << "Read message from remote client" << m_socket;

	QByteArray bytes = m_socket->readAll();
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

				qDebug() << "Remote query type = " << type;
				qDebug() << "Remote query data = " << bytes;

				if (type == "admin_auth")
					pQuerys->onAdminLogining(bytes);
				if (type == "console_command")
					pQuerys->onConsoleCommandRecived(bytes);
				if (type == "register_game_server")
					pQuerys->onGameServerRegister(bytes);

				return;
			}
		}
	}
}

void RemoteConnection::bytesWritten(qint64 bytes)
{
	if (!m_socket)
		return;

	qDebug() << "Message to remote client" << m_socket << "sended! Size =" << bytes;
}

void RemoteConnection::close()
{
	if (!m_socket)
		return;

	m_socket->close();
}
