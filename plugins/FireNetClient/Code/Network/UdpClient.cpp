// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "UdpClient.h"

#include "Network/SyncGameState.h"
#include "ReadQueue.h"

#include <FireNet>

CUdpClient::CUdpClient(BoostIO& io_service, const char* ip, short port) : m_IO_service(io_service)
	, m_UdpSocket(io_service, BoostUdpEndPoint(boost::asio::ip::udp::v4(), 0))
	, m_ServerEndPoint(BoostUdpEndPoint(boost::asio::ip::address::from_string(ip), port))
	, bIsConnected(false)
	, bIsStopped(false)
	, pReadQueue(new CReadQueue())
{
	m_Status = EUdpClientStatus::NotConnected;

	m_ConnectionTimeout = 0.f;
	m_LastSendedMessageTime = 0.f;
	m_LastOutPacketNumber = 0;

	Do_Connect();
}

CUdpClient::~CUdpClient()
{
	SAFE_DELETE(mEnv->pGameSync);
	SAFE_DELETE(pReadQueue);
}

void CUdpClient::Update()
{
	if (gEnv->pSystem->IsQuitting() || bIsStopped)
		return;

	float m_CurrentTime = gEnv->pTimer->GetAsyncCurTime();

	auto pTickrate = gEnv->pConsole->GetCVar("firenet_game_server_tickrate");
	double tickrate = pTickrate ? 1000 / pTickrate->GetIVal() * 0.001 : 33 * 0.001;

	//! Sending queue
	if (m_CurrentTime > m_LastSendedMessageTime + tickrate)
	{	
		if (!m_Queue.empty())
		{
			Do_Write();
		}
	}

	//! Send ping packet every second
	if (m_CurrentTime > m_LastSendedMessageTime + 1)
	{
		//! Connection timeout
		if (m_ConnectionTimeout == 0.f)
		{
			auto pNetTimeout = gEnv->pConsole->GetCVar("firenet_game_server_timeout");
			int m_Timeout = pNetTimeout ? pNetTimeout->GetIVal() : 10;

			m_ConnectionTimeout = m_CurrentTime + m_Timeout;

			if (m_Status == EUdpClientStatus::Connecting)
			{
				//! Sending connect request packet to game server
				CUdpPacket packet(m_LastOutPacketNumber, EFireNetUdpPacketType::Ask);
				packet.WriteAsk(EFireNetUdpAsk::Ask_Connect);
				SendNetMessage(packet);
			}
			else if (m_Status == (EUdpClientStatus::Connected | EUdpClientStatus::WaitStart) || m_Status == (EUdpClientStatus::Connected | EUdpClientStatus::Playing))
			{
				//! Send ping packet to server
				CUdpPacket packet(m_LastOutPacketNumber, EFireNetUdpPacketType::Ping);
				SendNetMessage(packet);
			}
		}
		else if (m_ConnectionTimeout < m_CurrentTime)
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Connection timeout!");
			CloseConnection();
		}
	}
}

void CUdpClient::SendNetMessage(CUdpPacket & packet)
{
	if (gEnv->pSystem->IsQuitting() || bIsStopped)
		return;

	m_LastOutPacketNumber++;
	m_Queue.push(packet);
}

void CUdpClient::CloseConnection()
{
	CryLog(TITLE "Closing UDP client...");

	m_Status = NotConnected;
	bIsConnected = false;
	bIsStopped = true;

	mEnv->pGameSync->Reset();

	if (!gEnv->pSystem->IsQuitting())
		gEnv->pConsole->ExecuteString("unload", true, true);

	m_UdpSocket.close();
	m_IO_service.stop();
}

void CUdpClient::Do_Connect()
{
	ICVar* ip = gEnv->pConsole->GetCVar("firenet_game_server_ip");
	ICVar* port = gEnv->pConsole->GetCVar("firenet_game_server_port");

	CryLogAlways(TITLE "Connecting to game server <%s : %d>", ip->GetString(), port->GetIVal());

	UpdateStatus(EUdpClientStatus::Connecting);

	Do_Read();
}

void CUdpClient::Do_Read()
{
	if (gEnv->pSystem->IsQuitting() || bIsStopped)
		return;

	std::memset(m_ReadBuffer, 0, static_cast<int>(EFireNetTcpPackeMaxSize::SIZE));

	m_UdpSocket.async_receive_from(boost::asio::buffer(m_ReadBuffer, static_cast<int>(EFireNetUdpPackeMaxSize::SIZE)), m_UdpSenderEndPoint, [this](boost::system::error_code ec, std::size_t length)
	{
		if (!ec && length > 0)
		{
			CUdpPacket packet(m_ReadBuffer);
			pReadQueue->ReadPacket(packet);		

			Do_Read();
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't read UDP packet : %s", ec.message().c_str());
			On_Disconnected();
		}	
	});
}

void CUdpClient::Do_Write()
{
	m_LastSendedMessageTime = gEnv->pTimer->GetAsyncCurTime();

	CUdpPacket  packet = m_Queue.front();	
	m_Queue.pop();

	const char* packetData = packet.toString();
	size_t      packetSize = strlen(packetData);

	m_UdpSocket.async_send_to(boost::asio::buffer(packetData, packetSize), m_ServerEndPoint, [this](boost::system::error_code ec, std::size_t length)
	{
		if (ec)
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't send UDP packet : %s", ec.message().c_str());
			On_Disconnected();
		}		
	});
}

void CUdpClient::On_Connected(bool connected)
{
	if (connected)
	{
		UpdateStatus(EUdpClientStatus(Connected | WaitStart));

		bIsConnected = connected;

		// Create game state syncronizator
		mEnv->pGameSync = new CGameStateSynchronization();

		CryLogAlways(TITLE "Connection with game server established");

		// Get map to load from game server
		CUdpPacket packet(m_LastOutPacketNumber, EFireNetUdpPacketType::Request);
		packet.WriteRequest(EFireNetUdpRequest::Request_GetMap);
		SendNetMessage(packet);
	}
	else
		CloseConnection();
}

void CUdpClient::On_Disconnected()
{
	CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Connection with game server lost");

	CloseConnection();
}

void CUdpClient::UpdateStatus(EUdpClientStatus newStatus)
{
	m_ConnectionTimeout = 0.f;
	m_Status = newStatus;
}
