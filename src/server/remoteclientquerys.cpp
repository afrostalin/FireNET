// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include <QDebug>
#include "remoteclientquerys.h"
#include "clientquerys.h"
#include "global.h"
#include "tcpserver.h"
#include "remoteserver.h"

RemoteClientQuerys::RemoteClientQuerys(QObject *parent) : QObject(parent)
{
	m_socket = nullptr;
	isAdmin = false;
	isGameServer = false;
}

RemoteClientQuerys::~RemoteClientQuerys()
{
}

void RemoteClientQuerys::onAdminLogining(QByteArray & bytes)
{
	qWarning() << "Client (" << m_socket << ") trying login in administrator mode!";

	if (gEnv->pRemoteServer->bHaveAdmin)
	{
		qCritical() << "Error authorization in administator mode! Reason = Administrator alredy has entered";

		QByteArray msg;
		msg.append("Error authorization in administator mode! Reason = Administrator alredy has entered.");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);
		return;
	}

	QXmlStreamReader xml(bytes);
	
	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "data")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			QString login = attributes.value("login").toString();
			QString password = attributes.value("password").toString();

			if (login.isEmpty() || password.isEmpty())
			{
				qWarning() << "Wrong packet data! Some values empty!";
				qDebug() << "Login = " << login << "Password = " << password;

				QByteArray msg;
				msg.append("Wrong packet data! Some values empty!");
				gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);

				return;
			}

			if (login == gEnv->pServer->serverRootUser)
			{
				if (password == gEnv->pServer->serverRootPassword)
				{
					qWarning() << "Client (" << m_socket << ") success login in administator mode!";
					isAdmin = true;
					gEnv->pRemoteServer->bHaveAdmin = true;

					QByteArray msg;
					msg.append("You success login in administator mode!");
					gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);

					return;
				}
				else
				{
					qCritical() << "Error authorization in administator mode! Reason = Wrong password. Password = " << password;

					QByteArray msg;
					msg.append("Error authorization in administator mode! Reason = Wrong password.");
					gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);

					return;
				}

			}
			else
			{
				qCritical() << "Error authorization in administator mode! Reason = Wrong login. Login = " << login;

				QByteArray msg;
				msg.append("Error authorization in administator mode! Reason = Wrong login.");
				gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);

				return;
			}

			return;
		}
	}
}

void RemoteClientQuerys::onConsoleCommandRecived(QByteArray & bytes)
{
	if (!isAdmin)
	{
		qCritical() << "Only administrator can use console commands!";

		QByteArray msg;
		msg.append("Only administrator can use console commands!");
		gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);

		return;
	}

	QXmlStreamReader xml(bytes);

	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "data")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			QString command = attributes.value("command").toString();

			if (command == "status")
			{
				QByteArray msg;
				msg.append("***Server status***\n");
				msg.append("server ip = " + gEnv->pServer->serverIP + "\n");
				msg.append("server port = " + QString::number(gEnv->pServer->serverPort) + "\n");
				msg.append("*******************");

				gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);

				return;
			}

			QByteArray msg;
			msg.append("Command = " + command + " not found!");
			gEnv->pRemoteServer->sendMessageToRemoteClient(m_socket, msg);

			return;
		}
	}
}
