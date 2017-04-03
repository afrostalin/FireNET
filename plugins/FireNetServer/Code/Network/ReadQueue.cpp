// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "ReadQueue.h"

#include "Network/UdpServer.h"
#include "Network/UdpPacket.h"
#include "Network/SyncGameState.h"

#include "Entities/FireNetPlayer/FireNetPlayer.h"
#include "Entities/FireNetSpawnPoint/FireNetSpawnPoint.h"

#include <CryEntitySystem/IEntitySystem.h>
#include <CryGame/IGameFramework.h>
#include <IActorSystem.h>
#include <ILevelSystem.h>

void CReadQueue::ReadPacket(CUdpPacket & packet)
{
	m_LastPacketTime = gEnv->pTimer->GetAsyncCurTime();

	//! Check packet number
	if (packet.getPacketNumber() < m_LastInputPacketNumber)
	{
		CryLog(TITLE "Packet from client can't be readed, because packet too old. Packet number : %d, last number : %d", packet.getPacketNumber(), m_LastInputPacketNumber);
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
		ReadPing();
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
	default:
		break;
	}
}

void CReadQueue::ReadAsk(CUdpPacket & packet, EFireNetUdpAsk ask)
{
	switch (ask)
	{
	case EFireNetUdpAsk::Ask_ChangeTeam:
	{
		break;
	}
	default:
		break;
	}
}

void CReadQueue::ReadPing()
{
	CUdpPacket packet(m_LastOutputPacketNumber, EFireNetUdpPacketType::Ping);
	SendPacket(packet);
}

void CReadQueue::ReadRequest(CUdpPacket & packet, EFireNetUdpRequest request)
{
	switch (request)
	{
	case EFireNetUdpRequest::Request_GetMap:
	{
		CryLog(TITLE "Client (Channel ID = %d) request map", m_ClientChannelID);

		auto pLevel = gEnv->pGameFramework->GetILevelSystem()->GetCurrentLevel();

		if (pLevel)
		{
			const char* m_LevelName = pLevel->GetName();
			CryLog(TITLE "Current map (%s)", m_LevelName);

			if (m_LevelName)
			{
				CUdpPacket packet(m_LastOutputPacketNumber, EFireNetUdpPacketType::Result);
				packet.WriteResult(EFireNetUdpResult::Result_MapToLoad);
				packet.WriteString(m_LevelName);

				SendPacket(packet);
			} 
			else
			{
				CUdpPacket packet(m_LastOutputPacketNumber, EFireNetUdpPacketType::Error);
				packet.WriteError(EFireNetUdpError::Error_CantGetMap);

				SendPacket(packet);
			}
		}

		break;
	}
	case EFireNetUdpRequest::Request_SpawnPlayer:
	{
		CryLog(TITLE "Client (Channel ID = %d) request spawn", m_ClientChannelID);

		auto m_SpawnPosition = FindSpawnPosition();

		if (m_SpawnPosition.b_Finded)
		{
			CryLog(TITLE "Spawn point for client (Channel ID = %d) found. Position(%f,%f,%f)", m_ClientChannelID, m_SpawnPosition.m_Pos.x, m_SpawnPosition.m_Pos.y, m_SpawnPosition.m_Pos.z);

			// TODO - Get info from master server and FireNet profile
			SFireNetClientPlayer player;
			player.m_PlayerUID = m_ClientChannelID;
			player.m_ChanelId = m_ClientChannelID;
			player.m_PlayerSpawnPos = m_SpawnPosition.m_Pos;
			player.m_PlayerSpawnRot = m_SpawnPosition.m_Rot;
			player.m_PlayerModel = "Test";
			player.m_PlayerNickname = string().Format("Test_%d", m_ClientChannelID);

			if (mEnv->pGameSync->SpawnNetPlayer(m_ClientChannelID, player))
			{
				//! Spawn player on client
				CUdpPacket packet(m_LastOutputPacketNumber, EFireNetUdpPacketType::Result);
				packet.WriteResult(EFireNetUdpResult::Result_PlayerSpawned);
				packet.WriteInt(player.m_PlayerUID);
				packet.WriteInt(player.m_ChanelId);
				packet.WriteVec3(player.m_PlayerSpawnPos);
				packet.WriteQuat(player.m_PlayerSpawnRot);
				packet.WriteString(player.m_PlayerModel.c_str());
				packet.WriteString(player.m_PlayerNickname.c_str());

				SendPacket(packet);

				//! Spawn all players connected to game server on client
				auto pAllPlayers = mEnv->pGameSync->GetAllNetPlayers();
				if (pAllPlayers)
				{
					for (const auto &it : *pAllPlayers)
					{
						if (it.second.m_ChanelId != player.m_ChanelId)
						{
							CUdpPacket packet(m_LastOutputPacketNumber, EFireNetUdpPacketType::Request);
							packet.WriteRequest(EFireNetUdpRequest::Request_SpawnPlayer);
							packet.WriteInt(it.second.m_PlayerUID);
							packet.WriteInt(it.second.m_ChanelId);
							packet.WriteVec3(it.second.pActor->GetEntity()->GetWorldPos());
							packet.WriteQuat(it.second.pActor->GetEntity()->GetWorldRotation());
							packet.WriteString(it.second.m_PlayerModel.c_str());
							packet.WriteString(it.second.m_PlayerNickname.c_str());

							SendPacket(packet);
						}
					}
				}

				//! Spawn player on other clients
				CUdpPacket packetToAll(m_LastOutputPacketNumber, EFireNetUdpPacketType::Request);
				packetToAll.WriteRequest(EFireNetUdpRequest::Request_SpawnPlayer);
				packetToAll.WriteInt(player.m_PlayerUID);
				packetToAll.WriteInt(player.m_ChanelId);
				packetToAll.WriteVec3(player.m_PlayerSpawnPos);
				packetToAll.WriteQuat(player.m_PlayerSpawnRot);
				packetToAll.WriteString(player.m_PlayerModel.c_str());
				packetToAll.WriteString(player.m_PlayerNickname.c_str());

				SendPacketToAllExcept(m_ClientChannelID, packetToAll);
			
			}	
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Spawn point for client (Channel ID = %d) not found!", m_ClientChannelID);
		}

		break;
	}
	case EFireNetUdpRequest::Request_UpdateInput:
	{
		SFireNetClientInput input;
		input.m_flags = static_cast<EFireNetClientInputFlags>(packet.ReadInt());
		input.m_LookOrientation = packet.ReadQuat();

		//! Sync input in server
		mEnv->pGameSync->SyncNetPlayerInput(m_ClientChannelID, input);

		break;
	}
	default:
		break;
	}
}

SFireNetSpawnPosition CReadQueue::FindSpawnPosition()
{
	SFireNetSpawnPosition m_SpawnPosition;

	auto *pEntityIterator = gEnv->pEntitySystem->GetEntityIterator();
	pEntityIterator->MoveFirst();

	auto *pSpawnerClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("FireNetSpawnPoint");
	auto extensionId = gEnv->pGameFramework->GetIGameObjectSystem()->GetID("FireNetSpawnPoint");

	while (!pEntityIterator->IsEnd())
	{
		IEntity *pEntity = pEntityIterator->Next();

		if (pEntity->GetClass() != pSpawnerClass)
			continue;

		auto *pGameObject = gEnv->pGameFramework->GetGameObject(pEntity->GetId());
		if (pGameObject == nullptr)
			continue;

		auto *pSpawner = static_cast<CFireNetSpawnPoint*>(pGameObject->QueryExtension(extensionId));
		if (pSpawner == nullptr)
			continue;

		if (pSpawner && pSpawner->IsEnabled())
		{
			m_SpawnPosition.m_Pos = pSpawner->GetEntity()->GetWorldPos();
			m_SpawnPosition.m_Rot = pSpawner->GetEntity()->GetWorldRotation();
			m_SpawnPosition.b_Finded = true;

			return m_SpawnPosition;
		}
	}

	return m_SpawnPosition;
}

void CReadQueue::SendPacket(CUdpPacket & packet)
{
	mEnv->pUdpServer->SendToClient(packet, m_ClientChannelID);
	m_LastOutputPacketNumber++;
}

void CReadQueue::SendPacketToAll(CUdpPacket &packet)
{
	mEnv->pUdpServer->SendToAll(packet);
	m_LastOutputPacketNumber++;
}

void CReadQueue::SendPacketToAllExcept(uint32 id, CUdpPacket &packet)
{
	mEnv->pUdpServer->SendToAllExcept(id, packet);
	m_LastOutputPacketNumber++;
}