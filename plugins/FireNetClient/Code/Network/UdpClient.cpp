// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "UdpClient.h"

#include "SyncGameState.h"
#include "ReadQueue.h"

#include <FireNet-Core>
#include <FireNet-Client>

using namespace boost::asio;
using namespace boost::system;

CUdpClient::CUdpClient(boost::asio::io_service& io_service) : m_IO_service(io_service)
	, bIsConnected(false)
	, m_UdpSocket(nullptr)
	, pReadQueue(new CReadQueue())
{
	m_Status = EUdpClientStatus::NotConnected;

	Do_Connect();
}

CUdpClient::~CUdpClient()
{
	SAFE_DELETE(mEnv->pGameSync);

	SAFE_DELETE(m_UdpSocket);
	SAFE_DELETE(m_UdpEndPoint);

	SAFE_DELETE(pReadQueue);
}

void CUdpClient::Update(float fDeltaTime)
{
	if (m_Status == EUdpClientStatus::Connecting)
	{
		CryLog(TITLE "Sending ask packet to game server...");

		CUdpPacket packet(mEnv->m_LastOutPacketNumber, EFireNetUdpPacketType::Ask);
		packet.WriteAsk(EFireNetUdpAsk::ConnectToServer);

		SendNetMessage(packet);
	}
}

void CUdpClient::SendNetMessage(CUdpPacket & packet)
{
	mEnv->m_LastOutPacketNumber++;

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

void CUdpClient::CloseConnection()
{
	if (bIsConnected)
	{
		CryLog(TITLE "Closing connection with game server");
		m_Status = EUdpClientStatus::NotConnected;
		bIsConnected = false;

		m_IO_service.stop();
	}
}

void CUdpClient::Do_Connect()
{
	ICVar* ip = gEnv->pConsole->GetCVar("firenet_game_server_ip");
	ICVar* port = gEnv->pConsole->GetCVar("firenet_game_server_port");

	if (ip && port)
	{
		CryLog(TITLE "Connecting to game server <%s : %d>", ip->GetString(), port->GetIVal());
		
		m_UdpEndPoint = new ip::udp::endpoint(ip::address::from_string(ip->GetString()), port->GetIVal());
		m_UdpSocket = new ip::udp::socket(m_IO_service, ip::udp::endpoint(ip::udp::v4(), 0));

		m_Status = EUdpClientStatus::Connecting;

		Do_Read();
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't connect to game server. Check CVars");
	}
}

void CUdpClient::Do_Read()
{
	m_UdpSocket->async_receive_from( buffer(m_ReadBuffer, static_cast<int>(EFireNetUdpPackeMaxSize::SIZE)), m_UdpSenderEndPoint, [this](boost::system::error_code ec, std::size_t length)
	{
		if (!ec && length > 0)
		{
			CryLog(TITLE "UDP packet received. Size = %d", length);

			CUdpPacket packet(m_ReadBuffer);
			pReadQueue->ReadPacket(packet);
		}
		else
		{
			Do_Read();
		}
	});
}

void CUdpClient::Do_Write()
{
	m_UdpSocket->async_send_to(buffer(m_Queue.front().toString(), m_Queue.front().getLength()), *m_UdpEndPoint,[this](boost::system::error_code ec, std::size_t length)
	{
		if (!ec)
		{
			CryLog(TITLE "UDP packet sended. Size = %d", length);

			m_Queue.pop();

			if (!m_Queue.empty())
			{
				Do_Write();
			}
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't send UDP packet : %s", ec.message().c_str());
			m_UdpSocket->close();
			On_Disconnected();
		}		
	});
}

void CUdpClient::On_Connected(bool connected)
{
	if(connected)
	{
		m_Status = EUdpClientStatus::Connected;
		bIsConnected = connected;

		// Create game state syncronizator
		mEnv->pGameSync = new CGameStateSynchronization();

		CryLogAlways(TITLE "Connection with game server established");
	}
	else
	{
		On_Disconnected();
	}
}

void CUdpClient::On_Disconnected()
{
	m_IO_service.stop();
	m_Status = EUdpClientStatus::NotConnected;
	bIsConnected = false;

	CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Connection with game server lost");
}