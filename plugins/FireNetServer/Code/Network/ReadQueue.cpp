// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "ReadQueue.h"

#include "Network/UdpServer.h"
#include "Network/UdpPacket.h"

#include <CryEntitySystem/IEntitySystem.h>
#include <CryGame/IGameFramework.h>
#include <ILevelSystem.h>

#include "Entities/FireNetSpawnPoint/FireNetSpawnPoint.h"
#include "SyncGameState.h"

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

void CReadQueue::ReadPing()
{
	CUdpPacket packet(m_LastOutputPacketNumber, EFireNetUdpPacketType::Ping);
	SendPacket(packet);
}

void CReadQueue::ReadRequest(CUdpPacket & packet, EFireNetUdpRequest request)
{
	switch (request)
	{
	case EFireNetUdpRequest::GetMap:
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
				packet.WriteResult(EFireNetUdpResult::MapToLoad);
				packet.WriteString(m_LevelName);

				SendPacket(packet);
			} 
			else
			{
				CUdpPacket packet(m_LastOutputPacketNumber, EFireNetUdpPacketType::Error);
				packet.WriteError(EFireNetUdpError::CantGetMap);

				SendPacket(packet);
			}
		}

		break;
	}
	case EFireNetUdpRequest::Spawn:
	{
		CryLog(TITLE "Client (%d) request spawn", m_ClientID);

		auto m_SpawnPosition = FindSpawnPosition();

		if (m_SpawnPosition.b_Finded)
		{
			CryLog(TITLE "Spawn point for client (%d) found. Position(%f,%f,%f)", m_ClientID, m_SpawnPosition.m_Pos.x, m_SpawnPosition.m_Pos.y, m_SpawnPosition.m_Pos.z);

			SFireNetSyncronizationClient player;
			player.m_PlayerUID = 1;
			player.m_ChanelId = 2;
			player.m_PlayerSpawnPos = m_SpawnPosition.m_Pos;
			player.m_PlayerSpawnRot = m_SpawnPosition.m_Rot;
			player.m_PlayerModel = "Test";
			player.m_PlayerNickname = "Test";

			if (mEnv->pGameSync->SpawnNetPlayer(player))
			{
				CUdpPacket packet(m_LastOutputPacketNumber, EFireNetUdpPacketType::Result);
				packet.WriteResult(EFireNetUdpResult::ClientSpawned);
				packet.WriteInt(player.m_PlayerUID);                 // uid
				packet.WriteInt(player.m_ChanelId);                  // cryengine channel id
				packet.WriteVec3(player.m_PlayerSpawnPos);           // spawn position
				packet.WriteQuat(player.m_PlayerSpawnRot);           // spawn rotation
				packet.WriteString(player.m_PlayerModel.c_str());    // file model
				packet.WriteString(player.m_PlayerNickname.c_str()); // nickname

				SendPacket(packet);
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

void CReadQueue::SendPacket(CUdpPacket & packet)
{
	mEnv->pUdpServer->SendToClient(packet, m_ClientID);
	m_LastOutputPacketNumber++;
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
			m_SpawnPosition.m_Pos = pSpawner->GetEntity()->GetPos();
			m_SpawnPosition.m_Rot = pSpawner->GetEntity()->GetRotation();
			m_SpawnPosition.b_Finded = true;

			return m_SpawnPosition;
		}
	}

	return m_SpawnPosition;
}
