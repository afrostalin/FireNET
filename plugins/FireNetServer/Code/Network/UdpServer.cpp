// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "UdpServer.h"

#include "Network/SyncGameState.h"
#include "Network/UdpPacket.h"

#include "Entities/FireNetPlayer/FireNetPlayer.h"
#include "Entities/FireNetPlayer/Input/FireNetPlayerInput.h"

CUdpServer::CUdpServer(BoostIO& io_service, const char* ip, short port)
	: m_IO_service(io_service)
	, m_UdpSocket(io_service, BoostUdpEndPoint(boost::asio::ip::address::from_string(ip), port))
	, m_NextClientID(1L)
	, m_Status(EFireNetUdpServerStatus::None)
	, m_LastSendedMessageTime(0.f)
{
	Do_Receive();

	// Game synchronization system
	mEnv->pGameSync = new CGameStateSynchronization();

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
	auto   pTickrate = gEnv->pConsole->GetCVar("firenet_game_server_tickrate");

	double tickrate = pTickrate ? 1000 / pTickrate->GetIVal() * 0.001 : 33 * 0.001;

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
			CryLogAlways(TITLE "Client (Channel ID = %d) disconnecting - Connection timeout", m_ID);
			RemoveClient(m_ID);
		}
		else if (bNeedToRemove)
		{
			CryLogAlways(TITLE "Client (Channel ID = %d) removing - Client marked for removing", m_ID);
			RemoveClient(m_ID);
		}
	}

	//! Set tickrate
	if (m_CurTime > m_LastSendedMessageTime + tickrate)
	{
		//! Update world state queue
		if (auto mPlayers = mEnv->pGameSync->GetAllNetPlayers())
		{
			for (const auto &it : *mPlayers)
			{
				CUdpPacket sync_packet(1, EFireNetUdpPacketType::Request);
				sync_packet.WriteRequest(EFireNetUdpRequest::Request_SyncPlayer);
				sync_packet.WriteInt(it.second.m_ChanelId);
				sync_packet.WriteInt(it.second.pPlayer->GetInput()->GetInputFlags());
				sync_packet.WriteQuat(it.second.pPlayer->GetInput()->GetLookOrientation());
				sync_packet.WriteVec3(it.second.pActor->GetEntity()->GetWorldPos());
				sync_packet.WriteQuat(it.second.pActor->GetEntity()->GetWorldRotation());

				SendToAllExcept(it.second.m_ChanelId, sync_packet);
			}
		}

		//! Send messages queue
		if (!m_PacketQueue.empty())
		{
			Do_Send();
		}		
	}
}

void CUdpServer::SendToClient(CUdpPacket & packet, uint32 clientID)
{
	if (auto pClient = GetClient(clientID))
	{
		//CryLog(TITLE "Sending UDP packet to client (%d) ...", clientID);

		SFireNetUdpMessage message;
		message.m_ClientID = clientID;
		message.m_Packet = packet;
		message.m_EndPoint = pClient->m_EndPoint;

		m_PacketQueue.push(message);

	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't send message to client (Channel ID = %d) - client not found", clientID);
}

void CUdpServer::SendToAll(CUdpPacket & packet)
{
	//CryLog(TITLE "Sending UDP packet to all clients ...");

	for (const auto &it : m_Clients)
	{
		SFireNetUdpMessage message;
		message.m_ClientID = it.first;
		message.m_Packet = packet;
		message.m_EndPoint = it.second.m_EndPoint;

		m_PacketQueue.push(message);
	}
}

void CUdpServer::SendToAllExcept(uint32 id, CUdpPacket & packet)
{
	//CryLog(TITLE "Sending UDP packet to all clients except client (%d) ...", id);

	for (const auto &it : m_Clients)
	{
		if (it.first != id)
		{
			SFireNetUdpMessage message;
			message.m_ClientID = it.first;
			message.m_Packet = packet;
			message.m_EndPoint = it.second.m_EndPoint;

			m_PacketQueue.push(message);
		}
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
	client.bInGame = false;
	client.bNeedToRemove = false;

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
		return nullptr;
	}
}

void CUdpServer::RemoveClient(uint32 id)
{
	try
	{
		CryLog(TITLE "Removing client (Channel ID = %d)...", id);

		if (auto pClient = GetClient(id))
		{
			if (pClient->bConnected)
			{
				mEnv->pGameSync->RemoveNetPlayer(id);
				On_ClientDisconnect(id);			
			}
		
			m_Clients.erase(id);

			CryLog(TITLE "Client (Channel ID = %d) removed.", id);
		}		

		//! Reset client IDs
		if (m_Clients.size() == 0)
		{
			m_NextClientID = 1L;
			mEnv->pGameSync->Reset();
		}
	}
	catch (std::out_of_range)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Can't remove client (Channel ID = %d) - unknown id", id);
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
			if (ec.value() == 10061)
			{
				uint32 clientId = GetOrCreateClientID(m_RemoteEndPoint);

				RemoveClient(clientId);
			}
			else
			{
				CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Can't receive message from target!");
				On_RemoteError(ec, m_RemoteEndPoint);
			}
		}

		Do_Receive();
	});
}

void CUdpServer::Do_Send()
{
	m_LastSendedMessageTime = gEnv->pTimer->GetAsyncCurTime();

	SFireNetUdpMessage message = m_PacketQueue.front();
	m_PacketQueue.pop();

	uint32           clientID = message.m_ClientID;
	const char*      packetData = message.m_Packet.toString();
	BoostUdpEndPoint endPoint = message.m_EndPoint;
	size_t           packetSize = strlen(packetData);

	if (!GetClient(clientID))
		return;

	m_UdpSocket.async_send_to(boost::asio::buffer(packetData, packetSize), endPoint, [this, endPoint, clientID](boost::system::error_code ec, std::size_t length)
	{
		if (ec)
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Can't send message to target!");
			On_RemoteError(ec, endPoint);
		}
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
	CryLogAlways(TITLE "Client (Channel ID = %d) disconnected", id);
}

void CUdpServer::MessageProcess(const char* data, uint32 id)
{
	auto pClient = GetClient(id);
	CUdpPacket packet(data);

	//! Mark to remove if client send broken packet
	if (!packet.IsGoodPacket())
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Client (Channel ID = %d) send bad packet. Marked to remove", id);

		if (pClient)
			pClient->bNeedToRemove = true;

		return;
	}

	//! Try accept new client
	if (pClient && !pClient->bConnected)
	{	
		if (packet.getType() == EFireNetUdpPacketType::Ask && packet.ReadAsk() == EFireNetUdpAsk::Ask_Connect)
		{
			auto pMaxPlayers = gEnv->pConsole->GetCVar("firenet_game_server_max_players");
			int m_MaxPlayers = pMaxPlayers ? pMaxPlayers->GetIVal() : 64;

			if (m_Status == EFireNetUdpServerStatus::LevelLoaded && GetClientCount() < m_MaxPlayers)
			{
				pClient->bConnected = true;
				pClient->pReader = new CReadQueue(id);

				CryLogAlways(TITLE "Client (Channel ID = %d) accepted", id);

				CUdpPacket packet(0, EFireNetUdpPacketType::Result);
				packet.WriteResult(EFireNetUdpResult::Result_ClientAccepted);

				SendToClient(packet, id);
			}
			else if (m_Status == EFireNetUdpServerStatus::LevelLoaded && GetClientCount() >= m_MaxPlayers)
			{
				CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Can't accept client (Channel ID = %d) - server full", id);

				CUdpPacket packet(0, EFireNetUdpPacketType::Error);
				packet.WriteError(EFireNetUdpError::Error_ServerFull);

				SendToClient(packet, id);

				//! Set client to remove for next frame
				pClient->bNeedToRemove = true;
			}
			else if (m_Status != EFireNetUdpServerStatus::LevelLoaded)
			{
				CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Can't accept client (Channel ID = %d) - server not ready to accepting new players", id);

				CUdpPacket packet(0, EFireNetUdpPacketType::Error);
				packet.WriteError(EFireNetUdpError::Error_ServerBlockNewConnection);

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