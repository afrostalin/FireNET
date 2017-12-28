// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QEventLoop>

#include "inputlistener.h"
#include "global.h"
#include "client.h"

#include "Core/tcppacket.h"

InputListener::InputListener(QObject *parent) : QObject(parent)
{
}

void InputListener::Run() const
{
	gEnv->pClient = new RemoteClient();

	while (true)
	{
		QTextStream s(stdin);

		QString input = s.readLine();

		if (input == "list")
		{
			LogInfo("connect <ip> <port> - connecting to FireNET with specific address");
			LogInfo("connect - connecting to FireNET with default address (%s:%d)", gEnv->m_ServerIP.toStdString().c_str(), gEnv->m_ServerPort);
			LogInfo("login <login> <password> - authorization in FireNET by login and password");
			LogInfo("status - get full FireNET status");
			LogInfo("send_message <message> - Send message to all connected players");
			LogInfo("send_command <command> <arguments> - Send console command to all connected players");
			LogInfo("players - Get player list");
			LogInfo("servers - Get game server list");
		}
		else if (input.contains("connect"))
		{
			QStringList list = input.split(" ");
			if (list.size() == 3)
			{
				QString ip = list[1];
				QString port = list[2];

				LogInfo("Start connection to FireNET <%s : %d>", ip.toStdString().c_str(), port.toInt());

				gEnv->pClient->ConnectToServer(ip, port.toInt());
			}
			else
			{
				LogInfo("Start connection to FireNET <%s : %d>", gEnv->m_ServerIP.toStdString().c_str(), gEnv->m_ServerPort);
				gEnv->pClient->ConnectToServer(gEnv->m_ServerIP, gEnv->m_ServerPort);
			}
		}
		else if (input.contains("login"))
		{
			QStringList list = input.split(" ");
			if (list.size() == 3)
			{
				QString login = list[1];
				QString password = list[2];

				CTcpPacket packet(EFireNetTcpPacketType::Query);
				packet.WriteQuery(EFireNetTcpQuery::AdminLogin);
				packet.WriteString(login.toStdString().c_str());
				packet.WriteString(password.toStdString().c_str());

				gEnv->pClient->SendMessage(packet);
			}
			else
			{
				LogError("Syntax error! Use login <login> <password>");
			}
		}
		else if (input == "status")
		{
			CTcpPacket packet(EFireNetTcpPacketType::Query);
			packet.WriteQuery(EFireNetTcpQuery::AdminCommand);
			packet.WriteAdminCommand(EFireNetAdminCommands::CMD_Status);

			gEnv->pClient->SendMessage(packet);
		}
		else if (input.contains("send_message"))
		{
			QString message = input.remove("send_message");

			if (!message.isEmpty())
			{
				CTcpPacket packet(EFireNetTcpPacketType::Query);
				packet.WriteQuery(EFireNetTcpQuery::AdminCommand);
				packet.WriteAdminCommand(EFireNetAdminCommands::CMD_SendGlobalMessage);
				packet.WriteString("send_message");
				packet.WriteString(message.toStdString().c_str());

				gEnv->pClient->SendMessage(packet);
			}
			else
			{
				LogError("Syntax error! Use send_message <message>");
			}
		}
		else if (input.contains("send_command"))
		{
			QString message = input.remove("send_command");

			if (!message.isEmpty())
			{
				CTcpPacket packet(EFireNetTcpPacketType::Query);
				packet.WriteQuery(EFireNetTcpQuery::AdminCommand);
				packet.WriteAdminCommand(EFireNetAdminCommands::CMD_SendGlobalCommand);
				packet.WriteString("send_command");
				packet.WriteString(message.toStdString().c_str());

				gEnv->pClient->SendMessage(packet);
			}
			else
			{
				LogError("Syntax error! Use send_console_command <command> <arguments>");
			}
		}
		else if (input == "players")
		{
			CTcpPacket packet(EFireNetTcpPacketType::Query);
			packet.WriteQuery(EFireNetTcpQuery::AdminCommand);
			packet.WriteAdminCommand(EFireNetAdminCommands::CMD_GetPlayersList);
			packet.WriteString("players");

			gEnv->pClient->SendMessage(packet);
		}
		else if (input == "servers")
		{
			CTcpPacket packet(EFireNetTcpPacketType::Query);
			packet.WriteQuery(EFireNetTcpQuery::AdminCommand);
			packet.WriteAdminCommand(EFireNetAdminCommands::CMD_GetGameServersList);
			packet.WriteString("servers");

			gEnv->pClient->SendMessage(packet);
		}
		else
		{
			LogError("Unknown command");
		}

		QEventLoop loop;
		QTimer::singleShot(100, &loop, SLOT(quit()));
		loop.exec();
	}
}
