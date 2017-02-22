// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"

#include "TcpClient.h"
#include "ReadQueue.h"

using namespace boost::system;
using namespace boost::asio;

CTcpClient::CTcpClient(io_service & io_service, ssl::context & context) : m_SslSocket(io_service, context) 
	, m_IO_service(io_service)
	, m_Timer(io_service)
	, pReadQueue(nullptr)
	, bIsConnected(false)
	, bIsClosing(false)
{
	m_Status = ETcpClientStatus::NotConnected;
	m_MessageStatus = ETcpMessageStatus::None;

	fConTimeout = 0.f;
	fMsgTimeout = 0.f;
	fMsgEndTime = 0.f;

	m_SslSocket.set_verify_mode(ssl::verify_peer);
	m_SslSocket.set_verify_callback(boost::bind(&CTcpClient::Do_VerifyCertificate, this, _1, _2));

	CryLog(TITLE "TCP client successfully init.");

	// Start connection
	Do_Connect();

	// Start timer
	m_Timer.async_wait(boost::bind(&CTcpClient::TimeOutCheck, this));
}

CTcpClient::~CTcpClient()
{
	SAFE_DELETE(pReadQueue);
}

void CTcpClient::TimeOutCheck()
{
	if ((m_Timer.expires_at() <= deadline_timer::traits_type::now()) && !bIsConnected)
	{
		m_SslSocket.lowest_layer().close();
		m_Timer.expires_at(boost::posix_time::pos_infin);
	}

	m_Timer.async_wait(boost::bind(&CTcpClient::TimeOutCheck, this));
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
	CryLog(TITLE "Closing TCP client...");

	m_Status = ETcpClientStatus::NotConnected;
	m_MessageStatus = ETcpMessageStatus::None;
	bIsConnected = false;
	bIsClosing = true;
	
	m_SslSocket.lowest_layer().close();
	m_Timer.cancel();
	m_IO_service.stop();
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

bool CTcpClient::Do_VerifyCertificate(bool preverified, boost::asio::ssl::verify_context & ctx)
{
	CryLog(TITLE "Verifyng ssl certificate...");

	if (preverified)
		CryLog(TITLE "Ssl certificate success verified");

	return preverified;
}

// Errors : 0 - connection timeout, 1 - connection refused, 2 - unknown error
void CTcpClient::Do_Connect()
{ 
	ICVar* ip = gEnv->pConsole->GetCVar("firenet_ip");
	ICVar* port;

	if (gEnv->IsDedicated())
		port = gEnv->pConsole->GetCVar("firenet_remote_port");
	else
		port = gEnv->pConsole->GetCVar("firenet_port");

	ICVar* timeout = gEnv->pConsole->GetCVar("firenet_timeout");

	if (ip && port && timeout)
	{
		CryLog(TITLE "Connecting to master server <%s : %d> ...", ip->GetString(), port->GetIVal());

		m_Status = ETcpClientStatus::Connecting;

		ip::tcp::endpoint ep(ip::address::from_string(ip->GetString()), port->GetIVal());	
		ip::tcp::resolver resolver(m_IO_service);
		auto epIt = resolver.resolve(ep);

		m_Timer.expires_from_now(boost::posix_time::seconds(timeout->GetIVal()));

		async_connect(m_SslSocket.lowest_layer(), epIt, [this](error_code ec, ip::tcp::resolver::iterator)
		{
			SFireNetEventArgs args;

			if (!m_SslSocket.lowest_layer().is_open())
			{
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Connection timeout!");
				
				args.AddInt(0);
				mEnv->SendFireNetEvent(FIRENET_EVENT_MASTER_SERVER_CONNECTION_ERROR, args);

				CloseConnection();
			} 
			else if (!ec)
			{
				Do_Handshake();
			}
			else
			{
				if (ec.value() == 10061)
				{
					CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Connection refused by host!");
			
					args.AddInt(1);
					mEnv->SendFireNetEvent(FIRENET_EVENT_MASTER_SERVER_CONNECTION_ERROR, args);
				}
				else
				{
					CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Connection error : %s", ec.message().c_str());

					args.AddInt(2);
					mEnv->SendFireNetEvent(FIRENET_EVENT_MASTER_SERVER_CONNECTION_ERROR, args);
				}

				CloseConnection();
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
		else if(ec && !bIsClosing)
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't ssl handshake : %s", ec.message().c_str());
			On_Disconnected();
		}
	});
}

void CTcpClient::Do_Read()
{
	std::memset(m_ReadBuffer, 0, static_cast<int>(EFireNetTcpPackeMaxSize::SIZE));

	m_SslSocket.async_read_some(buffer(m_ReadBuffer, static_cast<int>(EFireNetTcpPackeMaxSize::SIZE)), [this](boost::system::error_code ec, std::size_t length)
	{
		if (!ec)
		{
			CryLog(TITLE "TCP packet received. Size = %d", length);

			m_MessageStatus = ETcpMessageStatus::Recieved;

			CTcpPacket packet(m_ReadBuffer);
			pReadQueue->ReadPacket(packet);

			Do_Read();
		}
		else if (ec && !bIsClosing)
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't read TCP packet : %s", ec.message().c_str());

			On_Disconnected();
		}
	});
}

void CTcpClient::Do_Write()
{
	const char* packetData = m_Queue.front().toString();
	size_t      packetSize = strlen(packetData);

	async_write(m_SslSocket, buffer(packetData, packetSize), [this](error_code ec, std::size_t length)
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
		else if(ec && !bIsClosing)
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't send TCP packet : %s", ec.message().c_str());

			On_Disconnected();
		}
	});
}

void CTcpClient::On_Disconnected()
{
	CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Connection with master server lost");

	mEnv->SendFireNetEvent(FIRENET_EVENT_MASTER_SERVER_DISCONNECTED);

	CloseConnection();
}