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
	LogInfo("Connecting to FireNet...");

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
    }
}

void RemoteClient::SendMessage(CTcpPacket &packet)
{
	m_packets.push(packet);
}

void RemoteClient::onConnectedToServer()
{
    LogInfo("Connected to FireNET");
    bConnected = true;

	LogInfo("Registering in arbitrator mode...");
	CTcpPacket packet(EFireNetTcpPacketType::Query);
	packet.WriteQuery(EFireNetTcpQuery::RegisterArbitrator);
	packet.WriteString(gEnv->m_Name.toStdString().c_str());
	packet.WriteInt(gEnv->m_GameServersMaxCount);
	packet.WriteInt(gEnv->m_GameServersCount);

	SendMessage(packet);
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
	case EFireNetTcpResult::RegisterArbitratorComplete:
	{
		LogInfo("Succesfully register in arbitrator mode");
		break;
	}
	case EFireNetTcpResult::UpdateArbitratorComplete:
	{
		LogInfo("Succesfully update arbitrator data");
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
	case EFireNetTcpError::RegisterArbitratorFail:
	{
		LogError("Can't register in arbitrator mode!");
		break;
	}
	case EFireNetTcpError::UpdateArbitratorFail:
	{
		LogError("Can't update arbitrator data in FireNet");
		break;
	}
	default:
		break;
	}
}
