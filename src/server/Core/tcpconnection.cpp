// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "tcpconnection.h"
#include "tcpserver.h"
#include "tcppacket.h"

#include "Workers/Packets/clientquerys.h"
#include "Tools/console.h"

TcpConnection::TcpConnection(QObject *parent) 
	: QObject(parent)
	, m_Socket(nullptr)
	, m_PeerPort(0)
	, pQuery(nullptr)
	, bConnected(false)
	, bIsQuiting(false)
	, bLastMsgSended(true)
{
	Q_UNUSED(parent);

	m_maxPacketSize = gEnv->pConsole->GetInt("net_max_packet_size");
	m_maxBadPacketsCount = gEnv->pConsole->GetInt("net_max_bad_packets_count");
	m_BadPacketsCount = 0;

	m_Time = QTime::currentTime();
	m_InputPacketsCount = 0;
	m_PacketsSpeed = 0;
	m_maxPacketSpeed = gEnv->pConsole->GetInt("net_max_packets_speed");
}

TcpConnection::~TcpConnection()
{
	SAFE_RELEASE(m_Socket);
	SAFE_RELEASE(pQuery);
}

void TcpConnection::Update()
{
	if (!m_Packets.empty() && m_Socket && bConnected && bLastMsgSended)
	{
		bLastMsgSended = false;
		CTcpPacket packet = m_Packets.front();
		m_Packets.pop();
		m_Socket->write(packet.toString());
	}

	if (m_Time.elapsed() >= 1000)
	{
		m_Time = QTime::currentTime();
		CalculateStatistic();
	}
}

void TcpConnection::SendMessage(CTcpPacket& packet)
{
	m_Packets.push(packet);
}

SIpPort TcpConnection::GetPeerAddress() const
{
	return SIpPort(m_PeerIP, m_PeerPort);
}

void TcpConnection::quit()
{
	bIsQuiting = true;
	m_Socket->close();
}

void TcpConnection::accept(const qint64 socketDescriptor)
{
	LogDebug("[TcpConnection] Accepting new client...");

	m_Socket = CreateSocket();

	if (!m_Socket->setSocketDescriptor(socketDescriptor))
	{
		LogWarning("[TcpConnection] Can't accept socket <%p>!", m_Socket);
		return;
	}

	m_Socket->setLocalCertificate("key.pem");
	m_Socket->setPrivateKey("key.key");
	m_Socket->startServerEncryption();

	const int timeout = gEnv->pConsole->GetInt("net_encryption_timeout");

	if (!m_Socket->waitForEncrypted(timeout * 1000))
	{
		LogDebug("[TcpConnection] Can't accept socket <%p>! Encryption timeout!", m_Socket);
		emit quit();
		return;
	}

	m_PeerIP = m_Socket->peerAddress().toString().toStdString();
	m_PeerPort = m_Socket->peerPort();
}

void TcpConnection::connected()
{
    if(m_Socket == nullptr)
		return;

	m_Client.socket = m_Socket;
	m_Client.profile = nullptr;
	m_Client.status = 0;	

	// Add client to server client list
	gEnv->pServer->AddNewClient(m_Client);

	// Create client querys worker
	pQuery = new ClientQuerys(this);
	// Set socket for client querys worker
	pQuery->SetSocket(m_Socket);
	// Set client
	pQuery->SetClient(&m_Client);
	// Set connection
	pQuery->SetConnection(this);

	bConnected = true;

	LogDebug("[TcpConnection] Client with socket <%p> (%s:%d) connected.", m_Socket, m_PeerIP.c_str(), m_PeerPort);

	emit opened();
}

void TcpConnection::disconnected()
{
	if (m_Socket == nullptr || !bConnected)
	{
		emit closed();
		return;
	}

	// Remove client from server client list
	gEnv->pServer->RemoveClient(m_Client);

	LogDebug("[TcpConnection] Client with socket <%p> (%s:%d) disconnected.", m_Socket, m_PeerIP.c_str(), m_PeerPort);

	emit closed();
}

void TcpConnection::readyRead()
{
    if(m_Socket == nullptr || bIsQuiting)
		return;

	m_InputPacketsCount++;

	emit received();

	// If client send a lot bad packet we need disconnect him
	if (m_BadPacketsCount >= m_maxBadPacketsCount)
	{
		LogWarning("[TcpConnection] Exceeded the number of bad packets from a client with socket <%p> (%s:%d). Connection will be closed!", m_Socket, m_PeerIP.c_str(), m_PeerPort);
		quit();
		return;
	}

	// Check bytes count before reading
	if (m_Socket->bytesAvailable() > m_maxPacketSize)
	{
		LogWarning("[TcpConnection] Very big packet from client with socket <%p> (%s:%d)", m_Socket, m_PeerIP.c_str(), m_PeerPort);
		m_BadPacketsCount++;
		return;
	}

	if (m_Socket->bytesAvailable() <= 0)
	{
		LogWarning("[TcpConnection] Very small packet from client with socket <%p> (%s:%d)", m_Socket, m_PeerIP.c_str(), m_PeerPort);
		m_BadPacketsCount++;
		return;
	}

	// Read packet
	CTcpPacket packet(m_Socket->readAll());

	if (!pQuery->ReadPacket(packet))
	{
		m_BadPacketsCount++;
	}
}

void TcpConnection::bytesWritten(const qint64 bytes)
{
    if(!m_Socket)
		return;

	emit sended();

	bLastMsgSended = true;

	if (gEnv->pConsole->GetBool("net_packet_debug"))
	{
		LogDebug("[TcpConnection] Message to client with socket <%p> (%s:%d) sended! Size <%d>", m_Socket, m_PeerIP.c_str(), m_PeerPort, bytes);
	}
}

void TcpConnection::stateChanged(const QAbstractSocket::SocketState socketState) const
{
    if(!m_Socket)
		return;

	LogDebug("[TcpConnection] Client with socket <%p> (%s:%d) changed socket state to <%d>", m_Socket, m_PeerIP.c_str(), m_PeerPort, static_cast<int>(socketState));
}

void TcpConnection::socketError(const QAbstractSocket::SocketError error)
{
	if (error == QAbstractSocket::SocketError::RemoteHostClosedError)
	{
		quit();
	}
	else
	{
		LogDebug("[TcpConnection] Client with socket <%p> (%s:%d) return socket error <%d>", m_Socket, m_PeerIP.c_str(), m_PeerPort, static_cast<int>(error));
		quit();
	}
}

QSslSocket * TcpConnection::CreateSocket()
{
	QSslSocket* socket = new QSslSocket(this);
	connect(socket, &QSslSocket::encrypted, this, &TcpConnection::connected, Qt::QueuedConnection);
	connect(socket, &QSslSocket::disconnected, this, &TcpConnection::disconnected, Qt::QueuedConnection);
	connect(socket, &QSslSocket::readyRead, this, &TcpConnection::readyRead, Qt::QueuedConnection);
	connect(socket, &QSslSocket::bytesWritten, this, &TcpConnection::bytesWritten, Qt::QueuedConnection);
	connect(socket, &QSslSocket::stateChanged, this, &TcpConnection::stateChanged, Qt::QueuedConnection);
	connect(socket, static_cast<void (QSslSocket::*)(QAbstractSocket::SocketError)>(&QSslSocket::error), this, &TcpConnection::socketError, Qt::QueuedConnection);

	return socket;
}

void TcpConnection::CalculateStatistic()
{
	m_PacketsSpeed = m_InputPacketsCount;

	if (m_PacketsSpeed >= m_maxPacketSpeed)
	{
		LogWarning("[TcpConnection] Client with socket <%p> (%s:%d) exceeded the limit of the number of packets per second. Speed <%d>. Maximum speed <%d>", m_Socket, m_PeerIP.c_str(), m_PeerPort, m_PacketsSpeed, m_maxPacketSpeed);
		quit();
	}

	m_InputPacketsCount = 0;
}