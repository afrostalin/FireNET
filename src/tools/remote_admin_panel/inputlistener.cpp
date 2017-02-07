// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QTextStream>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

#include "inputlistener.h"
#include "global.h"
#include "client.h"
#include "Core/netpacket.h"

InputListener::InputListener(QObject *parent) : QObject(parent)
{
}

void InputListener::Run()
{
	gEnv->pClient = new RemoteClient();

	while (true)
	{
		QTextStream s(stdin);
		QString input;

		input = s.readLine();

		if (input == "list")
		{
			qDebug() << "connect <ip> <port> - connecting to FireNET with specific address";
			qDebug() << "connect - connecting to FireNET with default address (127.0.0.1:64000)";
			qDebug() << "login <login> <password> - authorization in FireNET by login and password";
			qDebug() << "status - get full FireNET status";
			qDebug() << "send_message <message> - Send message to all connected players";
			qDebug() << "send_command <command> <arguments> - Send console command to all connected players";
			qDebug() << "players - Get player list";
			qDebug() << "servers - Get game server list";
		}
		else if (input.contains("connect"))
		{
			QStringList list = input.split(" ");
			if (list.size() == 3)
			{
				QString ip = list[1];
				QString port = list[2];

				qDebug() << "Start connection to FireNET (" << ip << ":" << port.toInt() << ")";

				gEnv->pClient->ConnectToServer(ip, port.toInt());
			}
			else
			{
				qDebug() << "Start connection to FireNET (127.0.0.1:64000)";
				gEnv->pClient->ConnectToServer("127.0.0.1", 64000);
			}
		}
		else if (input.contains("login"))
		{
			QStringList list = input.split(" ");
			if (list.size() == 3)
			{
				QString login = list[1];
				QString password = list[2];

				NetPacket packet(net_Query);
				packet.WriteInt(net_query_remote_admin_login);
				packet.WriteString(login.toStdString());
				packet.WriteString(password.toStdString());

				gEnv->pClient->SendMessage(packet);
			}
			else
			{
				qDebug() << "Syntax error! Use login <login> <password>";
			}
		}
		else if (input == "status")
		{
			NetPacket packet(net_Query);
			packet.WriteInt(net_query_remote_server_command);
			packet.WriteString("status");
			packet.WriteString("");

			gEnv->pClient->SendMessage(packet);
		}
		else if (input.contains("send_message"))
		{
			QString message = input.remove("send_message");

			if (!message.isEmpty())
			{
				NetPacket packet(net_Query);
				packet.WriteInt(net_query_remote_server_command);
				packet.WriteString("send_message");
				packet.WriteString(message.toStdString());

				gEnv->pClient->SendMessage(packet);
			}
			else
				qDebug() << "Syntax error! Use send_message <message>";
		}
		else if (input.contains("send_command"))
		{
			QString message = input.remove("send_command");

			if (!message.isEmpty())
			{
				NetPacket packet(net_Query);
				packet.WriteInt(net_query_remote_server_command);
				packet.WriteString("send_command");
				packet.WriteString(message.toStdString());

				gEnv->pClient->SendMessage(packet);
			}
			else
				qDebug() << "Syntax error! Use send_console_command <command> <arguments>";
		}
		else if (input == "players")
		{
			NetPacket packet(net_Query);
			packet.WriteInt(net_query_remote_server_command);
			packet.WriteString("players");
			packet.WriteString("");

			gEnv->pClient->SendMessage(packet);
		}
		else if (input == "servers")
		{
			NetPacket packet(net_Query);
			packet.WriteInt(net_query_remote_server_command);
			packet.WriteString("servers");
			packet.WriteString("");

			gEnv->pClient->SendMessage(packet);
		}
		else
		{
			qCritical() << "Unknown command";
		}

		QEventLoop loop;
		QTimer::singleShot(100, &loop, SLOT(quit()));
		loop.exec();
	}
}