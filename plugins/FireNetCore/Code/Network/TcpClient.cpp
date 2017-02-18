// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"

#include "TcpClient.h"
#include "ReadQueue.h"

using namespace boost::system;
using namespace boost::asio;

CTcpClient::CTcpClient(io_service & io_service, ssl::context & context) : m_SslSocket(io_service, context) 
	, m_IO_service(io_service)
	, pReadQueue(nullptr)
{
	m_Status = ETcpClientStatus::NotConnected;
	m_MessageStatus = ETcpMessageStatus::None;

	fConTimeout = 0.f;
	fMsgTimeout = 0.f;
	fMsgEndTime = 0.f;

	m_SslSocket.set_verify_mode(ssl::verify_peer);
	m_SslSocket.set_verify_callback(boost::bind(&CTcpClient::Do_VerifyCertificate, this, _1, _2));

	// Start connection
	Do_Connect();
}

CTcpClient::~CTcpClient()
{
	SAFE_DELETE(pReadQueue);
}

void CTcpClient::Update(float fDeltaTime)
{
	// Connection timeout
	if (m_Status == ETcpClientStatus::Connecting)
	{
		if (fConTimeout == 0.f)
		{
			fConTimeout = gEnv->pTimer->GetAsyncCurTime() + gEnv->pConsole->GetCVar("firenet_timeout")->GetFVal();
			CryLog(TITLE "Wait connection. Start time = %f. End time = %f", fDeltaTime, fConTimeout);
		}

		if (fConTimeout < gEnv->pTimer->GetAsyncCurTime())
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Connection timeout!");

			m_Status = ETcpClientStatus::NotConnected;

			SFireNetEventArgs args;
			args.AddInt(0);
			mEnv->SendFireNetEvent(FIRENET_EVENT_MASTER_SERVER_CONNECTION_ERROR, args);
		}
	}

	// Message sending timeout
	if (m_Status == ETcpClientStatus::Connected && m_MessageStatus == ETcpMessageStatus::Wait)
	{
		if (fMsgEndTime == 0.f)
		{
			fMsgEndTime = gEnv->pTimer->GetAsyncCurTime() + fMsgTimeout;
			CryLog(TITLE "Wait server answer. Start time = %f. End time = %f", fDeltaTime, fConTimeout);
		}

		if (fMsgEndTime < gEnv->pTimer->GetAsyncCurTime())
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Answer timeout!");
			m_MessageStatus = ETcpMessageStatus::Error;
		}
	}
}

void CTcpClient::AddToSendQueue(CTcpPacket & packet)
{
	m_IO_service.post([this, packet]()
	{
		bool write_in_progress = !m_Queue.empty();
		m_Queue.push(packet);
		if (!write_in_progress)
		{
			Do_Write();
		}
	});
}

void CTcpClient::CloseConnection()
{
	if (bIsConnected)
	{
		CryLog(TITLE "Closing connection with master server");
		m_Status = ETcpClientStatus::NotConnected;
		bIsConnected = false;
		m_MessageStatus = ETcpMessageStatus::None;

		m_IO_service.stop();
	}
}

void CTcpClient::SendQuery(CTcpPacket & packet)
{
	if (bIsConnected)
	{
		m_MessageStatus = ETcpMessageStatus::Sending;
		AddToSendQueue(packet);
	}
	else
	{
		m_MessageStatus = ETcpMessageStatus::Error;
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't send message to master server. No connection");
	}
}

void CTcpClient::SendSyncQuery(CTcpPacket & packet, float timeout)
{
	if (bIsConnected)
	{
		fMsgTimeout = timeout;
		fMsgEndTime = 0.f;
		m_MessageStatus = ETcpMessageStatus::Wait;
		AddToSendQueue(packet);

		while (m_MessageStatus != ETcpMessageStatus::Error || m_MessageStatus != ETcpMessageStatus::Recieved)
		{
			CrySleep(15);
		}
	}
	else
	{
		m_MessageStatus = ETcpMessageStatus::Error;
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't send message to master server. No connection");
		return;
	}
}

bool CTcpClient::Do_VerifyCertificate(bool preverified, boost::asio::ssl::verify_context & ctx)
{
	CryLog(TITLE "Verifyng ssl certificate...");

	if (preverified)
		CryLog(TITLE "Ssl certificate success verified");

	return preverified;
}

void CTcpClient::Do_Connect()
{ 
	ICVar* ip = gEnv->pConsole->GetCVar("firenet_ip");
	ICVar* port = gEnv->pConsole->GetCVar("firenet_port");

	if (ip && port)
	{
		CryLog(TITLE "Start connecting to master server <%s : %d>", ip->GetString(), port->GetIVal());

		m_Status = ETcpClientStatus::Connecting;

		ip::tcp::endpoint ep(ip::address::from_string(ip->GetString()), port->GetIVal());	
		ip::tcp::resolver resolver(m_IO_service);
		auto epIt = resolver.resolve(ep);

		async_connect(m_SslSocket.lowest_layer(), epIt, [this](error_code ec, ip::tcp::resolver::iterator)
		{
			if (!ec)
			{
				Do_Handshake();
			}
			else
			{
				CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Connection error : %s", ec.message().c_str());
			}
		});
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't connect to master server. Check CVars");
		mEnv->SendFireNetEvent(FIRENET_EVENT_MASTER_SERVER_CONNECTION_ERROR);
	}
}

void CTcpClient::Do_Handshake()
{
	CryLog(TITLE "Start ssl handshake...");

	m_SslSocket.async_handshake(ssl::stream_base::handshake_type::client, [this](boost::system::error_code ec)
	{
		if (!ec)
		{
			CryLog(TITLE "Success ssl handshake");

			pReadQueue = new CReadQueue();

			mEnv->SendFireNetEvent(FIRENET_EVENT_MASTER_SERVER_CONNECTED);
			m_Status = ETcpClientStatus::Connected;
			bIsConnected = true;

			CryLogAlways(TITLE "Connection with master server established");

			Do_Read();
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't ssl handshake : %s", ec.message().c_str());
			m_SslSocket.lowest_layer().close();

			On_Disconnected();
		}
	});
}

void CTcpClient::Do_Read()
{
	async_read(m_SslSocket, buffer(m_ReadBuffer, static_cast<int>(ETcpPacketMaxSize::SIZE)), [this](boost::system::error_code ec, std::size_t length)
	{
		if (!ec)
		{
			CryLog(TITLE "TCP packet received. Size = %d", length);

			m_MessageStatus = ETcpMessageStatus::Recieved;

			CTcpPacket packet(m_ReadBuffer);
			pReadQueue->ReadPacket(packet);

			Do_Read();
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't read TCP packet : %s", ec.message().c_str());
			m_SslSocket.lowest_layer().close();

			On_Disconnected();
		}
	});
}

void CTcpClient::Do_Write()
{
	async_write(m_SslSocket, buffer(m_Queue.front().toString(), m_Queue.front().getLength()), [this](error_code ec, std::size_t length)
	{
		if (!ec)
		{
			CryLog(TITLE "TCP packet sended. Size = %d", length);

			m_MessageStatus = ETcpMessageStatus::Sended;

			m_Queue.pop();

			if (!m_Queue.empty())
			{
				Do_Write();
			}
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't send TCP packet : %s", ec.message().c_str());
			m_SslSocket.lowest_layer().close();

			On_Disconnected();
		}
	});
}

void CTcpClient::On_Disconnected()
{
	m_IO_service.stop();
	m_Status = ETcpClientStatus::NotConnected;
	m_MessageStatus = ETcpMessageStatus::None;
	bIsConnected = false;

	CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Connection with master server lost");

	mEnv->SendFireNetEvent(FIRENET_EVENT_MASTER_SERVER_DISCONNECTED);
}