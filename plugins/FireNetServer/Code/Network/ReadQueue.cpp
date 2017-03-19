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
		CryLog(TITLE "Client %d request map", m_ClientID);

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
		CryLog(TITLE "Client (%d) request spawn", m_ClientID);

		auto m_SpawnPosition = FindSpawnPosition();

		if (m_SpawnPosition.b_Finded)
		{
			CryLog(TITLE "Spawn point for client (%d) found. Position(%f,%f,%f)", m_ClientID, m_SpawnPosition.m_Pos.x, m_SpawnPosition.m_Pos.y, m_SpawnPosition.m_Pos.z);

			// TODO - Get info from master server and FireNet profile
			SFireNetClientPlayer player;
			player.m_PlayerUID = m_ClientID;
			player.m_ChanelId = m_ClientID + 1;
			player.m_PlayerSpawnPos = m_SpawnPosition.m_Pos;
			player.m_PlayerSpawnRot = m_SpawnPosition.m_Rot;
			player.m_PlayerModel = "Test";
			player.m_PlayerNickname = "Test";

			if (mEnv->pGameSync->SpawnNetPlayer(player))
			{
				//! Spawn local player in client
				CUdpPacket packet(m_LastOutputPacketNumber, EFireNetUdpPacketType::Result);
				packet.WriteResult(EFireNetUdpResult::Result_PlayerSpawned);
				packet.WriteInt(player.m_PlayerUID);
				packet.WriteInt(player.m_ChanelId + 1);
				packet.WriteVec3(player.m_PlayerSpawnPos);
				packet.WriteQuat(player.m_PlayerSpawnRot);
				packet.WriteString(player.m_PlayerModel.c_str());
				packet.WriteString(player.m_PlayerNickname.c_str());

				SendPacket(packet);

				//! Spawn all players connected to game server in local client
				auto pAllPlayers = mEnv->pGameSync->GetAllPlayers();
				if (pAllPlayers)
				{
					for (const auto &it : *pAllPlayers)
					{
						if (it.m_PlayerUID != player.m_PlayerUID)
						{
							CUdpPacket packet(m_LastOutputPacketNumber, EFireNetUdpPacketType::Request);
							packet.WriteRequest(EFireNetUdpRequest::Request_SpawnPlayer);
							packet.WriteInt(it.m_PlayerUID);
							packet.WriteInt(it.m_ChanelId + 1);
							packet.WriteVec3(it.pActor->GetEntity()->GetWorldPos());
							packet.WriteQuat(it.pActor->GetEntity()->GetWorldRotation());
							packet.WriteString(it.m_PlayerModel.c_str());
							packet.WriteString(it.m_PlayerNickname.c_str());

							SendPacket(packet);
						}
					}
				}

				//! Spawn player on other clients
				CUdpPacket packetToAll(m_LastOutputPacketNumber, EFireNetUdpPacketType::Request);
				packetToAll.WriteRequest(EFireNetUdpRequest::Request_SpawnPlayer);
				packetToAll.WriteInt(player.m_PlayerUID);
				packetToAll.WriteInt(player.m_ChanelId + 1);
				packetToAll.WriteVec3(player.m_PlayerSpawnPos);
				packetToAll.WriteQuat(player.m_PlayerSpawnRot);
				packetToAll.WriteString(player.m_PlayerModel.c_str());
				packetToAll.WriteString(player.m_PlayerNickname.c_str());

				SendPacketToAllExcept(m_ClientID, packetToAll);
			
			}	
			else
			{
			}
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Spawn point for client (%d) not found!", m_ClientID);
		}

		break;
	}
	case EFireNetUdpRequest::Request_UpdateInput:
	{
		SFireNetClientInput input;
		input.m_flags = static_cast<EFireNetClientInputFlags>(packet.ReadInt());
		input.m_value = packet.ReadFloat();

		//! Sync input in server
		mEnv->pGameSync->SyncNetPlayerInput(m_ClientID, input);

		if (auto mPlayers = mEnv->pGameSync->GetAllPlayers())
		{
			for (const auto &it : *mPlayers)
			{
				if (it.m_PlayerUID == m_ClientID)
				{
					//! Sync player position with all clients
					CUdpPacket sync_packet(m_LastOutputPacketNumber, EFireNetUdpPacketType::Request);
					sync_packet.WriteRequest(EFireNetUdpRequest::Request_SyncPlayer);
					sync_packet.WriteInt(m_ClientID);
					sync_packet.WriteInt(input.m_flags);
					sync_packet.WriteFloat(input.m_value);
					sync_packet.WriteVec3(it.pActor->GetEntity()->GetWorldPos());
					sync_packet.WriteQuat(it.pActor->GetEntity()->GetWorldRotation());

					SendPacketToAllExcept(m_ClientID, sync_packet);

					break;
				}
			}
		}
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
	mEnv->pUdpServer->SendToClient(packet, m_ClientID);
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