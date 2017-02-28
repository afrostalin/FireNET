// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "ReadQueue.h"

#include "Network/UdpClient.h"
#include "Network/UdpPacket.h"

#include "Network/SyncGameState.h"

void CReadQueue::ReadPacket(CUdpPacket & packet)
{
	//! Check packet number
	if (packet.getPacketNumber() < m_LastInputPacketNumber)
	{
		CryLog(TITLE "Packet from server can't be readed, because packet too old. Packet number : %d, last number : %d", packet.getPacketNumber(), m_LastInputPacketNumber);
		return;
	}
	else if (packet.getPacketNumber() >= m_LastInputPacketNumber)
	{
		m_LastInputPacketNumber = packet.getPacketNumber();
	}

	//! Server can't send to client empty packet, it's wrong, but you can see that if it happened
	switch (packet.getType())
	{
	case EFireNetUdpPacketType::Empty :
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Packet type = EFireNetUdpPacketType::Empty");
		break;
	}
	case EFireNetUdpPacketType::Ping :
	{
		mEnv->pUdpClient->ResetTimeout();
		break;
	}
	case EFireNetUdpPacketType::Ask :
	{
		ReadAsk(packet, packet.ReadAsk());
		break;
	}
	case EFireNetUdpPacketType::Request :
	{
		ReadRequest(packet, packet.ReadRequest());
		break;
	}
	case EFireNetUdpPacketType::Result :
	{
		ReadResult(packet, packet.ReadResult());
		break;
	}
	case EFireNetUdpPacketType::Error :
	{
		ReadError(packet, packet.ReadError());
		break;
	}
	default:
		break;
	}
}

void CReadQueue::ReadAsk(CUdpPacket & packet, EFireNetUdpAsk ask)
{
	switch (ask)
	{
	case EFireNetUdpAsk::ConnectToServer:
	{
		break;
	}
	case EFireNetUdpAsk::ChangeTeam:
	{
		break;
	}
	default:
		break;
	}
}

void CReadQueue::ReadRequest(CUdpPacket & packet, EFireNetUdpRequest request)
{
	switch (request)
	{
	case EFireNetUdpRequest::Spawn:
	{
		CryLog(TITLE "Server request spawn new player");

		Vec3 m_SpawnPos;
		Quat m_SpawnRot;

		uint m_FireNetUID = packet.ReadInt();
		uint m_ChanelID = packet.ReadInt();

		m_SpawnPos.x = packet.ReadFloat();
		m_SpawnPos.y = packet.ReadFloat();
		m_SpawnPos.z = packet.ReadFloat();

		m_SpawnRot.w = packet.ReadFloat();
		m_SpawnRot.v.x = packet.ReadFloat();
		m_SpawnRot.v.y = packet.ReadFloat();
		m_SpawnRot.v.z = packet.ReadFloat();

		string m_FileModel = packet.ReadString();
		string m_Nickname = packet.ReadString();

		SFireNetSyncronizationClient player;
		player.m_PlayerUID = m_FireNetUID;
		player.m_ChanelId = m_ChanelID;
		player.m_PlayerSpawnPos = m_SpawnPos;
		player.m_PlayerSpawnRot = m_SpawnRot;
		player.m_PlayerModel = m_FileModel;
		player.m_PlayerNickname = m_Nickname;

		mEnv->pGameSync->SpawnNetPlayer(player);

		break;
	}
	case EFireNetUdpRequest::Movement:
	{
		break;
	}
	case EFireNetUdpRequest::Action:
	{
		break;
	}
	default:
		break;
	}
}

void CReadQueue::ReadResult(CUdpPacket & packet, EFireNetUdpResult result)
{
	switch (result)
	{
	case EFireNetUdpResult::ClientAccepted:
	{
		mEnv->pUdpClient->On_Connected(true);
		break;
	}
	case EFireNetUdpResult::ClientSpawned:
		break;
	case EFireNetUdpResult::ClientMoved:
		break;
	default:
		break;
	}
}

void CReadQueue::ReadError(CUdpPacket & packet, EFireNetUdpError error)
{
	switch (error)
	{
	case EFireNetUdpError::ServerFull:
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Game server can't accept new client - server full");
		mEnv->pUdpClient->On_Connected(false);
		break;
	}
	case EFireNetUdpError::PlayerBanned:
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Game server can't accept new client - player banned");
		mEnv->pUdpClient->On_Connected(false);
		break;
	}
	case EFireNetUdpError::ServerBlockNewConnection:
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Game server can't accept new client - server not ready");
		mEnv->pUdpClient->On_Connected(false);
		break;
	}
	default:
		break;
	}
}
