// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "remoteconnection.h"
#include "global.h"
#include "remoteserver.h"
#include "remoteclientquerys.h"
#include <QXmlStreamReader>

RemoteConnection::RemoteConnection(QObject *parent) : QObject(parent)
{
	m_socket = nullptr;
	pQuerys = nullptr;
}

void RemoteConnection::accept(qint64 socketDescriptor)
{
	m_socket = new QSslSocket(this);
	pQuerys = new RemoteClientQuerys(this);

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
	gEnv->pRemoteServer->clientCount++;

	qInfo() << "New remote client connected (Socket =" << m_socket << ").";
	qInfo() << "Remote client count = " << gEnv->pRemoteServer->clientCount;

	pQuerys->SetSocket(m_socket);
}

void RemoteConnection::disconnected()
{
	if (!m_socket)
		return;
	gEnv->pRemoteServer->clientCount--;

	if (gEnv->pRemoteServer->bHaveAdmin && pQuerys->isAdmin)
	{
		gEnv->pRemoteServer->bHaveAdmin = false;
		qWarning() << "Remote administrator disconnected!";
	}

	qInfo() << "Remote client disconnected (Socket =" << m_socket << ").";
	qInfo() << "Remote client count = " << gEnv->pRemoteServer->clientCount;
}

void RemoteConnection::readyRead()
{
	if (!m_socket)
		return;

	qDebug() << "Read message from remote client (Socket =" << m_socket << ")";

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

				return;
			}
		}
	}
}

void RemoteConnection::bytesWritten(qint64 bytes)
{
	qDebug() << "Message (Size =" << bytes << ") sended to remote client (Socket =" << m_socket << ")";
}

void RemoteConnection::close()
{
	if (!m_socket)
		return;

	m_socket->close();
}
