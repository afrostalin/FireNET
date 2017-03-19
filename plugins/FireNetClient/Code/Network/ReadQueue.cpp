// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "ReadQueue.h"

#include "Network/UdpClient.h"
#include "Network/UdpPacket.h"
#include "Network/SyncGameState.h"

#include "Entities/FireNetPlayer/FireNetPlayer.h"

#include <CryGame/IGameFramework.h>
#include <IActorSystem.h>

#include "ILevelSystem.h"

void CReadQueue::ReadPacket(CUdpPacket & packet)
{
	//! Check packet number
	/*if (packet.getPacketNumber() < m_LastInputPacketNumber)
	{
		CryLog(TITLE "Packet from server can't be readed, because packet too old. Packet number : %d, last number : %d", packet.getPacketNumber(), m_LastInputPacketNumber);
		return;
	}
	else if (packet.getPacketNumber() >= m_LastInputPacketNumber)
	{
		m_LastInputPacketNumber = packet.getPacketNumber();
	}*/

	//! Reset timeout
	mEnv->pUdpClient->ResetTimeout();

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
}

void CReadQueue::ReadRequest(CUdpPacket & packet, EFireNetUdpRequest request)
{
	switch (request)
	{
	case EFireNetUdpRequest::Request_SpawnPlayer:
	{
		CryLog(TITLE "Server request spawn new player");

		SFireNetClientPlayer player;
		player.m_PlayerUID = packet.ReadInt();
		player.m_ChanelId = packet.ReadInt();
		player.m_PlayerSpawnPos = packet.ReadVec3();
		player.m_PlayerSpawnRot = packet.ReadQuat();
		player.m_PlayerModel = packet.ReadString();
		player.m_PlayerNickname = packet.ReadString();

		//! Spawn other player
		mEnv->pGameSync->SpawnNetPlayer(player);

		break;
	}
	case EFireNetUdpRequest::Request_SyncPlayer:
	{
		//! Sync net input from other players
		int playerUID = packet.ReadInt();
		SFireNetClientInput input;
		input.m_flags = static_cast<EFireNetClientInputFlags>(packet.ReadInt());
		input.m_value = packet.ReadFloat();
		Vec3 playerPos = packet.ReadVec3();
		Quat playerRot = packet.ReadQuat();

		if (playerUID != m_LocalPlayerUID)
		{
			mEnv->pGameSync->SyncNetPlayerInput(playerUID, input);
			mEnv->pGameSync->SyncNetPlayerPosRot(playerUID, playerPos, playerRot);
		}


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
	case EFireNetUdpResult::Result_ClientAccepted:
	{
		mEnv->pUdpClient->On_Connected(true);
		break;
	}
	case EFireNetUdpResult::Result_MapToLoad:
	{
		string m_MapName = packet.ReadString();

		//! Load server map
		if (!m_MapName.IsEmpty() && gEnv->pConsole)
		{
			CryLog(TITLE "Map to load = %s. Loding...", m_MapName);

			mEnv->pUdpClient->UpdateStatus(CUdpClient::EUdpClientStatus::WaitStart);
			gEnv->pConsole->ExecuteString(string().Format("map %s", m_MapName), true, true);
		}
		else
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't load map - map empty!");

		break;
	}
	case EFireNetUdpResult::Result_PlayerSpawned:
	{		
		SFireNetClientPlayer player;
		player.m_PlayerUID = packet.ReadInt();
		player.m_ChanelId = packet.ReadInt();
		player.m_PlayerSpawnPos = packet.ReadVec3();
		player.m_PlayerSpawnRot = packet.ReadQuat();
		player.m_PlayerModel = packet.ReadString();
		player.m_PlayerNickname = packet.ReadString();

		//! Spawn local player
		if (mEnv->pGameSync->SpawnNetPlayer(player))
		{
			m_LocalPlayerUID = player.m_PlayerUID;

			mEnv->pUdpClient->UpdateStatus(CUdpClient::EUdpClientStatus(CUdpClient::EUdpClientStatus::Connected | CUdpClient::EUdpClientStatus::Playing));
		}

		break;
	}
	default:
		break;
	}
}

void CReadQueue::ReadError(CUdpPacket & packet, EFireNetUdpError error)
{
	switch (error)
	{
	case EFireNetUdpError::Error_ServerFull:
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Game server can't accept new client - server full");
		mEnv->pUdpClient->On_Connected(false);
		break;
	}
	case EFireNetUdpError::Error_PlayerBanned:
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Game server can't accept new client - player banned");
		mEnv->pUdpClient->On_Connected(false);
		break;
	}
	case EFireNetUdpError::Error_ServerBlockNewConnection:
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Game server can't accept new client - server not ready");
		mEnv->pUdpClient->On_Connected(false);
		break;
	}
	default:
		break;
	}
}
