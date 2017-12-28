// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "remoteconnection.h"
#include "remoteserver.h"
#include "tcppacket.h"

#include "Workers/Packets/remoteclientquerys.h"
#include "Tools/console.h"

RemoteConnection::RemoteConnection(QObject *parent) 
	: QObject(parent)
	, m_socket(nullptr)
	, m_PeerPort(0)
	, pQuerys(nullptr)
	, m_Client()
	, bConnected(false)
	, bLastMsgSended(true)
{

	m_maxPacketSize = gEnv->pConsole->GetInt("net_max_packet_size");
	m_maxBadPacketsCount = gEnv->pConsole->GetInt("net_max_bad_packets_count");
	m_BadPacketsCount = 0;

	m_Time = QTime::currentTime();
	m_InputPacketsCount = 0;
	m_PacketsSpeed = 0;
	m_maxPacketSpeed = gEnv->pConsole->GetInt("net_max_packets_speed");
}

RemoteConnection::~RemoteConnection()
{
	SAFE_RELEASE(m_socket);
	SAFE_RELEASE(pQuerys);
}

void RemoteConnection::Update()
{
	if (!m_Packets.empty() && m_socket && bConnected && bLastMsgSended)
	{
		bLastMsgSended = false;
		CTcpPacket packet = m_Packets.front();
		m_Packets.pop();
		m_socket->write(packet.toString());
	}

	if (m_Time.elapsed() >= 1000)
	{
		m_Time = QTime::currentTime();
		CalculateStatistic();
	}
}

void RemoteConnection::SendMessage(CTcpPacket & packet)
{
	m_Packets.push(packet);
}

SIpPort RemoteConnection::GetPeerAddress() const
{
	return SIpPort(m_PeerIP, m_PeerPort);
}

void RemoteConnection::accept(const qint64 socketDescriptor)
{
	LogDebug("[RemoteConnection] Accepting new client...");

	m_socket = new QSslSocket(this);
	connect(m_socket, &QSslSocket::encrypted, this, &RemoteConnection::connected);
	connect(m_socket, &QSslSocket::disconnected, this, &RemoteConnection::disconnected);
	connect(m_socket, &QSslSocket::readyRead, this, &RemoteConnection::readyRead);
	connect(m_socket, &QSslSocket::bytesWritten, this, &RemoteConnection::bytesWritten);
	connect(m_socket, static_cast<void (QSslSocket::*)(QAbstractSocket::SocketError)>(&QSslSocket::error), this, &RemoteConnection::socketError);
	
	if (!m_socket->setSocketDescriptor(socketDescriptor))
	{
		LogDebug("[RemoteConnection] Can't accept socket <%p>!", m_socket);
		return;
	}

	m_socket->setLocalCertificate("key.pem");
	m_socket->setPrivateKey("key.key");
	m_socket->startServerEncryption();

	const int timeout = gEnv->pConsole->GetVariable("net_encryption_timeout").toInt();

	if (!m_socket->waitForEncrypted(timeout * 1000))
	{
		LogDebug("[RemoteConnection] Can't accept socket <%p>! Encryption timeout!", m_socket);
		emit finished();
	}
}

void RemoteConnection::connected()
{
	if (m_socket == nullptr)
		return;

	m_Client.socket = m_socket;

	// Add client to remote server client list
	gEnv->pRemoteServer->AddNewClient(m_Client);

	pQuerys = new RemoteClientQuerys(this);
	pQuerys->SetSocket(m_socket);
	pQuerys->SetClient(&m_Client);
	pQuerys->SetConnection(this);

	bConnected = true;

	m_PeerIP = m_socket->peerAddress().toString().toStdString();
	m_PeerPort = m_socket->peerPort();

	LogDebug("[RemoteConnection] Remote client with socket <%p> (%s:%d) connected.", m_socket, m_PeerIP.c_str(), m_PeerPort);
}

void RemoteConnection::disconnected()
{
	if (!m_socket || !bConnected)
	{
		emit finished();
		return;
	}
		
	if (gEnv->pRemoteServer->IsHaveAdmin() && m_Client.isAdmin)
	{
		gEnv->pRemoteServer->SetAdmin (false);
		LogWarning("[RemoteConnection] Remote administrator disconnected!");
	}

	if (m_Client.isArbitrator)
	{
		gEnv->m_ArbitratorsCount--;
	}
	else if (m_Client.isGameServer)
	{
		gEnv->m_GameServersRegistered--;
	}

	// Remove client from server client list
	gEnv->pRemoteServer->RemoveClient(m_Client);	

	LogDebug("[RemoteConnection] Remote client with socket <%p>(%s:%d) disconnected.", m_socket, m_PeerIP.c_str(), m_PeerPort);

	emit finished();
}

void RemoteConnection::readyRead()
{
	if (!m_socket)
		return;

	m_InputPacketsCount++;

	emit received();

	// If client send a lot bad packet we need disconnect him
	if (m_BadPacketsCount >= m_maxBadPacketsCount)
	{
		LogDebug("[RemoteConnection] Exceeded the number of bad packets from a client with socket <%p> (%s:%d). Connection will be closed", m_socket, m_PeerIP.c_str(), m_PeerPort);
		close();
		return;
	}

	// Check bytes count before reading
	if (m_socket->bytesAvailable() > m_maxPacketSize)
	{
		LogDebug("[RemoteConnection] Very big packet from client with socket <%p> (%s:%d)", m_socket, m_PeerIP.c_str(), m_PeerPort);
		m_BadPacketsCount++;
		return;
	}

	if (m_socket->bytesAvailable() <= 0)
	{
		LogDebug("[RemoteConnection] Very small packet from client with socket <%p> (%s:%d)", m_socket, m_PeerIP.c_str(), m_PeerPort);
		m_BadPacketsCount++;
		return;
	}

	if (gEnv->pConsole->GetBool("net_packet_debug"))
	{
		LogDebug("[RemoteConnection] Read message from remote client with socket <%p> (%s:%d)", m_socket, m_PeerIP.c_str(), m_PeerPort);
	}

	CTcpPacket packet(m_socket->readAll());

	if (!pQuerys->ReadPacket(packet))
	{
		m_BadPacketsCount++;
	}
}

void RemoteConnection::bytesWritten(const qint64 bytes)
{
	if (m_socket == nullptr)
		return;

	emit sended();

	bLastMsgSended = true;

	if (gEnv->pConsole->GetBool("net_packet_debug"))
	{
		LogDebug("[RemoteConnection] Message to remote client with socket <%p> (%s:%d) sended!. Size <%d>", m_socket, m_PeerIP.c_str(), m_PeerPort, bytes);
	}
}

void RemoteConnection::socketError(const QAbstractSocket::SocketError error) const
{
	if (error == QAbstractSocket::SocketError::RemoteHostClosedError)
	{
		close();
	}
	else
	{
		LogDebug("[RemoteConnection] Client with socket <%p> (%s:%d) return socket error <%d>", m_socket, m_PeerIP.c_str(), m_PeerPort, static_cast<int>(error));
		close();
	}
}

void RemoteConnection::CalculateStatistic()
{
	m_PacketsSpeed = m_InputPacketsCount;

	if (m_PacketsSpeed >= m_maxPacketSpeed)
	{
		LogWarning("[RemoteConnection] Client with socket <%p> (%s:%d) exceeded the limit of the number of packets per second. Speed <%d>. Maximum speed <%d>", m_socket, m_PeerIP.c_str(), m_PeerPort, m_PacketsSpeed, m_maxPacketSpeed);
		close();
	}

	m_InputPacketsCount = 0;
}

void RemoteConnection::close() const
{
	if (m_socket == nullptr)
		return;

	m_socket->close();
}
