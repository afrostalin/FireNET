// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "global.h"
#include "remoteclientquerys.h"
#include "clientquerys.h"
#include "tcpserver.h"
#include "remoteserver.h"
#include "dbworker.h"
#include "settings.h"

#include <QDebug>
#include <QCoreApplication>

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

			if (login == gEnv->pSettings->GetVariable("sv_root_user").toString())
			{
				if (password == gEnv->pSettings->GetVariable("sv_root_password").toString())
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
				msg.append("Server version = " + qApp->applicationVersion() + "\n");
				msg.append("Server ip = " + gEnv->pSettings->GetVariable("sv_ip").toString() + "\n");
				msg.append("Server port = " + gEnv->pSettings->GetVariable("sv_port").toString() + "\n");
				msg.append("Server tickrate = " + gEnv->pSettings->GetVariable("sv_tickrate").toString() + " per/sec.\n" );
				//msg.append("Active thread count = ");
				//msg.append("Database mode = ");
				//msg.append("Authrorization type = ");
				//msg.append("Players ammount = . Maximum players count = ");
				//msg.append("Game servers ammount = . Maximum game servers count = ");
				//msg.append("Average time for reading = ");
				//msg.append("Average time for sending = ");
				//msg.append("Server start time = ");
				//msg.append("Errors count = ");
				//msg.append("Warnings count = ");
				//msg.append("Server work");
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
