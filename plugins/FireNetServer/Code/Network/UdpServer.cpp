// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "UdpServer.h"
#include "SyncGameState.h"
#include "Network/UdpPacket.h"

CUdpServer::CUdpServer(BoostIO& io_service, const char* ip, short port)
	: m_IO_service(io_service)
	, m_UdpSocket(io_service, BoostUdpEndPoint(boost::asio::ip::address::from_string(ip), port))
	, m_NextClientID(0L)
	, m_Status(EFireNetUdpServerStatus::None)
{
	Do_Receive();

	CryLog(TITLE "UDP server successfully init.");
}

CUdpServer::~CUdpServer()
{
	SAFE_DELETE(mEnv->pGameSync);
}

void CUdpServer::Update()
{
	float  m_CurTime = gEnv->pTimer->GetAsyncCurTime();
	auto   pTimeout = gEnv->pConsole->GetCVar("firenet_game_server_timeout");

	//! Connection timeout
	if (m_CurTime > 0.f && pTimeout)
	{
		bool   bFinded = false;
		bool   bNeedToRemove = false;
		uint32 m_ID;
		for (const auto &it : m_Clients)
		{
			if (it.second.bConnected && it.second.pReader && it.second.pReader->GetLastTime() + pTimeout->GetFVal() < m_CurTime)
			{
				bFinded = true;
				m_ID = it.first;
				break;
			}
			else if (it.second.bNeedToRemove)
			{
				bNeedToRemove = true;
				m_ID = it.first;
				break;
			}
		}

		if (bFinded)
		{
			CryLogAlways(TITLE "Client (%d) disconnecting - Connection timeout", m_ID);
			RemoveClient(m_ID);
		}
		else if (bNeedToRemove)
		{
			CryLogAlways(TITLE "Client (%d) removing - Client marked for removing", m_ID);
			RemoveClient(m_ID);
		}
	}
}

void CUdpServer::SendToClient(CUdpPacket & packet, uint32 clientID)
{
	if (auto pClient = GetClient(clientID))
	{
		m_IO_service.post([this, packet, pClient]()
		{
			bool b_IsInProgress = !m_Queue.empty();

			m_Queue.push(packet);

			if (!b_IsInProgress)
			{
				Do_Send(pClient->m_EndPoint);
			}
		});
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't send message to client %d - client not found", clientID);
}

void CUdpServer::SendToAll(CUdpPacket & packet)
{
	for (const auto &it : m_Clients)
	{
		m_IO_service.post([this, packet, it]()
		{
			bool b_IsInProgress = !m_Queue.empty();

			m_Queue.push(packet);

			if (!b_IsInProgress)
			{
				Do_Send(it.second.m_EndPoint);
			}
		});
	}
}

uint32 CUdpServer::GetOrCreateClientID(BoostUdpEndPoint endpoint)
{
	for (const auto &it : m_Clients)
	{
		if (it.second.m_EndPoint == endpoint)
			return it.first;
	}

	auto id = ++m_NextClientID;

	SFireNetUdpServerClient client;
	client.m_ID = id;
	client.m_EndPoint = endpoint;
	client.pReader = nullptr;
	client.bConnected = false;
	client.bNeedToRemove = false;

	CryLog(TITLE "Add new client (%d)", id);

	m_Clients.insert(UdpClient(id, client));

	return id;
}

SFireNetUdpServerClient* CUdpServer::GetClient(uint32 id)
{
	try 
	{
		return &m_Clients.at(id);
	}
	catch (std::out_of_range)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't get client - unknown id!");
		return nullptr;
	}
}

void CUdpServer::RemoveClient(uint32 id)
{
	try
	{
		CryLog(TITLE "Removing client (%d)...", id);

		if (auto pClient = GetClient(id))
		{
			if (pClient->bConnected)
				On_ClientDisconnect(id);		

			CryLog(TITLE "Client (%d) removed.", id);
			m_Clients.erase(id);
		}		
	}
	catch (std::out_of_range)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Can't remove client - unknown id (%d)", id);
	}
}

void CUdpServer::Do_Receive()
{
	std::memset(m_ReadBuffer, 0, static_cast<int>(EFireNetTcpPackeMaxSize::SIZE));

	m_UdpSocket.async_receive_from(boost::asio::buffer(m_ReadBuffer, static_cast<int>(EFireNetUdpPackeMaxSize::SIZE)), m_RemoteEndPoint, [this](boost::system::error_code ec, std::size_t length)
	{
		if (!ec)
		{
			if (length > 0)
			{
				uint32 clientId = GetOrCreateClientID(m_RemoteEndPoint);
				MessageProcess(m_ReadBuffer, clientId);
			}	
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Can't receive message from target!");
			On_RemoteError(ec, m_RemoteEndPoint);
		}

		Do_Receive();
	});
}

void CUdpServer::Do_Send(BoostUdpEndPoint target)
{
	const char*      packetData = m_Queue.front().toString();
	size_t           packetSize = strlen(packetData);

	m_UdpSocket.async_send_to(boost::asio::buffer(packetData, packetSize), target, [this](boost::system::error_code ec, std::size_t length)
	{
		if (ec)
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Can't send message to target!");
			On_RemoteError(ec, m_RemoteEndPoint);
		}

		m_Queue.pop();
	});
}

void CUdpServer::On_RemoteError(const boost::system::error_code error_code, const BoostUdpEndPoint endPoint)
{
	bool bFound = false;
	uint32 id;

	CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "ErrorID : %d, Error message : %s", error_code.value(), error_code.message().c_str());

	for (const auto &it : m_Clients)
	{
		if (it.second.m_EndPoint == endPoint)
		{
			bFound = true;
			id = it.first;
			break;
		}
	}

	if (!bFound)
		return;

	RemoveClient(id);
}

void CUdpServer::On_ClientDisconnect(uint32 id)
{
	CryLogAlways(TITLE "Client (%d) disconnected", id);
}

void CUdpServer::MessageProcess(const char* data, uint32 id)
{
	auto pClient = GetClient(id);
	CUdpPacket packet(data);

	//! Mark to remove if client send broken packet
	if (!packet.IsGoodPacket())
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Client (%d) send bad packet. Marked to remove", id);

		if (pClient)
			pClient->bNeedToRemove = true;

		return;
	}

	//! Try accept new client
	if (pClient && !pClient->bConnected)
	{	
		if (packet.getType() == EFireNetUdpPacketType::Ask && packet.ReadAsk() == EFireNetUdpAsk::ConnectToServer)
		{
			if (m_Status == EFireNetUdpServerStatus::LevelLoaded && GetClientCount() < mEnv->net_max_players)
			{
				pClient->bConnected = true;
				pClient->pReader = new CReadQueue(id);

				CryLogAlways(TITLE "Client (%d) accepted", id);

				CUdpPacket packet(0, EFireNetUdpPacketType::Result);
				packet.WriteResult(EFireNetUdpResult::ClientAccepted);

				SendToClient(packet, id);
			}
			else if (m_Status == EFireNetUdpServerStatus::LevelLoaded && GetClientCount() >= mEnv->net_max_players)
			{
				CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Can't accept client - server full");

				CUdpPacket packet(0, EFireNetUdpPacketType::Error);
				packet.WriteError(EFireNetUdpError::ServerFull);

				SendToClient(packet, id);

				//! Set client to remove for next frame
				pClient->bNeedToRemove = true;
			}
			else if (m_Status != EFireNetUdpServerStatus::LevelLoaded)
			{
				CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Can't accept client - server not ready to accepting new players");

				CUdpPacket packet(0, EFireNetUdpPacketType::Error);
				packet.WriteError(EFireNetUdpError::ServerBlockNewConnection);

				SendToClient(packet, id);

				//! Set client to remove for next frame
				pClient->bNeedToRemove = true;
			}

			return;
		}
	}
	//! Reading messages from connected clients
	else if (pClient && pClient->bConnected) 
	{
		if (auto pClient = GetClient(id))
		{
			pClient->pReader->ReadPacket(packet);
		}
	}
}

void CUdpServer::Close()
{
	CryLog(TITLE "Closing UDP server...");

	m_Clients.clear();
	m_UdpSocket.close();
	m_IO_service.stop();
}