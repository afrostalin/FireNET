// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "client.h"

#include <QTcpServer>

RemoteClient::RemoteClient(QObject *parent) : QObject(parent),
	m_socket(nullptr),
	bLastMsgSended(true),
	bConnected(false)
{
	connect(&m_Timer, &QTimer::timeout, this, &RemoteClient::Update);

	m_Timer.start(33);
}

void RemoteClient::Update()
{
	if (bConnected && bLastMsgSended && m_packets.size() > 0)
	{
		CTcpPacket packet = m_packets.front();
		m_packets.pop();
		bLastMsgSended = false;

		m_socket->write(packet.toString());
	}
}

void RemoteClient::ConnectToServer(const QString &ip, int port)
{
    m_socket = new QSslSocket(this);
    connect(m_socket, &QSslSocket::encrypted, this, &RemoteClient::onConnectedToServer);
    connect(m_socket, &QSslSocket::readyRead, this, &RemoteClient::onReadyRead);
    connect(m_socket, &QSslSocket::disconnected, this, &RemoteClient::onDisconnected);
    connect(m_socket, &QSslSocket::bytesWritten, this, &RemoteClient::onBytesWritten);

    m_socket->addCaCertificates("key.pem");
    m_socket->connectToHostEncrypted(ip ,port);
    if(!m_socket->waitForEncrypted(3000))
    {
        LogError("Connection timeout");
        return;
    }
}

void RemoteClient::SendMessage(CTcpPacket &packet)
{
	m_packets.push(packet);
}

void RemoteClient::onConnectedToServer()
{
    LogInfo("You connected to FireNET");
    bConnected = true;
}

void RemoteClient::onReadyRead()
{
	if (m_socket)
	{
		CTcpPacket packet(m_socket->readAll());

		EFireNetTcpPacketType packetType = packet.getType();

		switch (packetType)
		{
		case EFireNetTcpPacketType::Empty:
			break;
		case EFireNetTcpPacketType::Query:
			break;
		case EFireNetTcpPacketType::Result:
			ReadResult(packet);
			break;
		case EFireNetTcpPacketType::Error:
			ReadError(packet);
			break;
		case EFireNetTcpPacketType::ServerMessage:
			break;
		default:
			break;
		}
	}
}

void RemoteClient::onBytesWritten(qint64 bytes)
{
	bLastMsgSended = true;
}

void RemoteClient::onDisconnected()
{
    LogWarning("Connection with FireNET lost!");
    bConnected = false;
}

void RemoteClient::ReadResult(CTcpPacket & packet) const
{
	EFireNetTcpResult result = packet.ReadResult();

	switch (result)
	{
	case EFireNetTcpResult::AdminLoginComplete:
	{
		LogInfo("You successfully log in!");
		break;
	}
	case EFireNetTcpResult::AdminCommandComplete:
	{
		LogInfo("Command successfully executed on server");

		EFireNetAdminCommands commandType = packet.ReadAdminCommand();

		switch (commandType)
		{
		case EFireNetAdminCommands::CMD_Status:
		{
			std::vector<std::string> status = packet.ReadArray();

			for (const auto &it : status)
			{
				LogInfo("%s", it.c_str());
			}

			break;
		}
		case EFireNetAdminCommands::CMD_SendGlobalMessage:
		{
			LogInfo("Global message sended");
			break;
		}
		case EFireNetAdminCommands::CMD_SendGlobalCommand:
		{
			LogInfo("Global command sended");
			break;
		}
		case EFireNetAdminCommands::CMD_SendRemoteMessage:
		{
			LogInfo("Global remote message sended");
			break;
		}
		case EFireNetAdminCommands::CMD_SendRemoteCommand:
		{
			LogInfo("Global remote command sended");
			break;
		}
		case EFireNetAdminCommands::CMD_GetPlayersList:
		{
			std::vector<std::string> players = packet.ReadArray();

			for (const auto &it : players)
			{
				LogInfo("%s", it.c_str());
			}

			break;
		}
		case EFireNetAdminCommands::CMD_GetGameServersList:
		{
			std::vector<std::string> servers = packet.ReadArray();

			for (const auto &it : servers)
			{
				LogInfo("%s", it.c_str());
			}

			break;
		}
		case EFireNetAdminCommands::CMD_RawMasterServerCommand:
		{
			LogInfo("Raw command executed!");
			break;
		}
		default:
			break;
		}

		break;
	}
	default:
		break;
	}
}

void RemoteClient::ReadError(CTcpPacket & packet) const
{
	EFireNetTcpError error = packet.ReadError();

	switch (error)
	{
	case EFireNetTcpError::AdminLoginFail:
	{
		int reason = packet.ReadInt();

		if (reason == 0)
		{
			LogWarning("Can't log in in FireNet! Login not found!");
		}
		else if (reason == 1)
		{
			LogWarning("Can't log in in FireNet! Incorrect password!");
		}
		else if (reason == 2)
		{
			LogWarning("Can't log in in FireNet! Aministator alredy log in!");
		}

		break;
	}
	case EFireNetTcpError::AdminCommandFail:
	{
		LogWarning("Command not found in FireNET!");
		break;
	}
	default:
		break;
	}
}
