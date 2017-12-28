// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"

#include "TcpClient.h"
#include "ReadQueue.h"

CTcpClient::CTcpClient(BoostIO & io_service, BoostSslContex & context) : m_SslSocket(io_service, context) 
	, m_IO_service(io_service)
	, m_Timer(io_service)
	, m_AnswerTimer(io_service)
	, pReadQueue(nullptr)
	, bIsConnected(false)
	, bIsClosing(false)
{
	m_Status = ETcpClientStatus::NotConnected;
	m_MessageStatus = ETcpMessageStatus::None;

	fConTimeout = 0.f;
	fMsgTimeout = 0.f;
	fMsgEndTime = 0.f;

	m_SslSocket.set_verify_mode(boost::asio::ssl::verify_peer);
	m_SslSocket.set_verify_callback(boost::bind(&CTcpClient::Do_VerifyCertificate, this, _1, _2));

	FireNetLog(TITLE "TCP client successfully init.");

	// Start connection
	Do_Connect();
}

CTcpClient::~CTcpClient()
{
	SAFE_DELETE(pReadQueue);
}

void CTcpClient::TimeOutCheck()
{
	if ((m_Timer.expires_at() <= BoostTimer::traits_type::now()) && !bIsConnected)
	{
		m_SslSocket.lowest_layer().close();
		m_Timer.expires_at(boost::posix_time::pos_infin);
	}
}

void CTcpClient::AnswerTimeout()
{
	if ((m_Timer.expires_at() <= BoostTimer::traits_type::now()) 
		&& bIsConnected 
		&& (m_MessageStatus == ETcpMessageStatus::Wait || m_MessageStatus == ETcpMessageStatus::Sending))
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Answer timeout!");
		CloseConnection();
		m_AnswerTimer.expires_at(boost::posix_time::pos_infin);
		m_AnswerTimer.cancel();
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
	FireNetLog(TITLE "Closing TCP client...");

	m_Status = ETcpClientStatus::NotConnected;
	m_MessageStatus = ETcpMessageStatus::None;
	bIsConnected = false;
	bIsClosing = true;

	m_Timer.cancel();
	m_AnswerTimer.cancel();
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
	FireNetLog(TITLE "Verifyng ssl certificate...");

	if (preverified)
		FireNetLog(TITLE "Ssl certificate success verified");

	return preverified;
}

void CTcpClient::Do_Connect()
{ 
	ICVar* ip = mEnv->net_master_ip;
	ICVar* port = gEnv->pConsole->GetCVar("net_firenetMasterPort");
	ICVar* timeout = gEnv->pConsole->GetCVar("net_firenetConnectionTimeout");

	if (ip && port && timeout)
	{
		FireNetLogAlways(TITLE "Connecting to master server <%s : %d> ...", ip->GetString(), port->GetIVal());

		m_Status = ETcpClientStatus::Connecting;

		BoostTcpEndPoint ep(boost::asio::ip::address::from_string(ip->GetString()), port->GetIVal());	
		BoostTcpResolver resolver(m_IO_service);
		auto epIt = resolver.resolve(ep);

		m_Timer.async_wait(boost::bind(&CTcpClient::TimeOutCheck, this));
		m_Timer.expires_from_now(boost::posix_time::seconds(timeout->GetIVal()));

		async_connect(m_SslSocket.lowest_layer(), epIt, [this](boost::system::error_code ec, BoostTcpResolver::iterator)
		{
			if (!m_SslSocket.lowest_layer().is_open())
			{
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Connection timeout!");
				
				for (const auto &it : mEnv->m_Listeners)
				{
					it->OnConnectionError(EFireNetCoreErrorCodes::ConnectionTimeout);
				}

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
					CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't connect to FireNet - Connection refused by host!");

					for (const auto &it : mEnv->m_Listeners)
					{
						it->OnConnectionError(EFireNetCoreErrorCodes::ConnectionRefused);
					}
				}
				else if(ec.value() == 10060)
				{
					CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't connect to FireNet - Connection timeout");

					for (const auto &it : mEnv->m_Listeners)
					{
						it->OnConnectionError(EFireNetCoreErrorCodes::ConnectionTimeout);
					}
				}
				else
				{
					CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't connect to FireNet - Unknown error. Error code : %d", ec.value());

					for (const auto &it : mEnv->m_Listeners)
					{
						it->OnConnectionError(EFireNetCoreErrorCodes::UnknownError);
					}
				}

				CloseConnection();
			}
		});
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't connect to FireNet. Check CVars");

		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnConnectionError(EFireNetCoreErrorCodes::WrongCVars);
		}
	}
}

void CTcpClient::Do_Handshake()
{
	FireNetLog(TITLE "Start ssl handshake...");

	m_SslSocket.async_handshake(boost::asio::ssl::stream_base::handshake_type::client, [this](boost::system::error_code ec)
	{
		if (!ec)
		{
			FireNetLog(TITLE "Success ssl handshake");

			pReadQueue = new CReadQueue();

			m_Status = ETcpClientStatus::Connected;
			bIsConnected = true;

			FireNetLogAlways(TITLE "Connection with master server established");
			FireNet::SendEmptyEvent(FIRENET_EVENT_MASTER_SERVER_CONNECTED);

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
	std::memset(m_ReadBuffer, 0, MAX_TCP_PACKET_SIZE);

	m_SslSocket.async_read_some(boost::asio::buffer(m_ReadBuffer, MAX_TCP_PACKET_SIZE), [this](boost::system::error_code ec, std::size_t length)
	{
		if (!ec)
		{
			FireNetLogDebug(TITLE "TCP packet received. Size = %d", length);

			m_AnswerTimer.cancel();
			m_MessageStatus = ETcpMessageStatus::Recieved;

			CTcpPacket packet(m_ReadBuffer);
			pReadQueue->ReadPacket(packet);

			CrySleep(10);
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
	bool        isWaitable = m_Queue.front().IsWaitable();
	const char* packetData = m_Queue.front().toString();
	size_t      packetSize = strlen(packetData);

	async_write(m_SslSocket, boost::asio::buffer(packetData, packetSize), [this, isWaitable] (boost::system::error_code ec, std::size_t length)
	{
		if (!ec)
		{
			FireNetLogDebug(TITLE "TCP packet sended (Waitable = %s). Size = %d", isWaitable ? "true" : "false", length);

			m_MessageStatus = ETcpMessageStatus::Sended;

			ICVar* timeout = gEnv->pConsole->GetCVar("net_firenetAnswerTimeout");
			if (timeout && isWaitable)
			{
				FireNetLogDebug(TITLE "Start timer for this packet... (%d)", timeout->GetIVal());

				m_AnswerTimer.cancel();
				m_AnswerTimer.async_wait(boost::bind(&CTcpClient::AnswerTimeout, this));
				m_AnswerTimer.expires_from_now(boost::posix_time::seconds(timeout->GetIVal()));

				m_MessageStatus = ETcpMessageStatus::Wait;
			}

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

	for (const auto &it : mEnv->m_Listeners)
	{
		it->OnDisconnect(EFireNetCoreErrorCodes::ConnectionLost);
	}

	CloseConnection();
}