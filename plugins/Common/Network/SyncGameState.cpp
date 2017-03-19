// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "SyncGameState.h"

#include "Entities/FireNetPlayer/FireNetPlayer.h"

#include <IActorSystem.h>

CGameStateSynchronization::CGameStateSynchronization()
{
}

CGameStateSynchronization::~CGameStateSynchronization()
{
	m_NetPlayers.clear();
}

void CGameStateSynchronization::Reset()
{
	CryLog(TITLE "CGameStateSynchronization::Reset()");

#ifdef CryFireNetClient_EXPORTS
	mEnv->pLocalPlayer = nullptr;
#endif

	for (const auto &it : m_NetPlayers)
	{
		RemoveNetPlayer(it.m_PlayerUID);
	}

	m_NetPlayers.clear();
}

bool CGameStateSynchronization::SpawnNetPlayer(SFireNetClientPlayer & player)
{
	CryLog(TITLE "Spawning FireNet player (%d)", player.m_PlayerUID);

	if (auto pActorSystem = gEnv->pGameFramework->GetIActorSystem())
	{
		auto pActor = pActorSystem->CreateActor(player.m_ChanelId, player.m_PlayerNickname, "FireNetPlayer", player.m_PlayerSpawnPos, player.m_PlayerSpawnRot, Vec3(1, 1, 1));

		if (pActor)
		{
#ifdef CryFireNetClient_EXPORTS
			if (mEnv->pLocalPlayer == nullptr)
			{
				mEnv->pLocalPlayer = dynamic_cast<CFireNetPlayer*>(pActor);
				
				if (mEnv->pLocalPlayer)
				{
					mEnv->pLocalPlayer->SetLocalPlayer(true);

					CryLog(TITLE "Local player spawned by server");
					CryLog(TITLE "Client uid : %d", player.m_PlayerUID);
					CryLog(TITLE "Channel id : %d", player.m_ChanelId);
					CryLog(TITLE "Spawn position (%f,%f,%f)", player.m_PlayerSpawnPos.x, player.m_PlayerSpawnPos.y, player.m_PlayerSpawnPos.z);
					CryLog(TITLE "Spawn rotation (%f,%f,%f,%f)", player.m_PlayerSpawnRot.w, player.m_PlayerSpawnRot.v.x, player.m_PlayerSpawnRot.v.y, player.m_PlayerSpawnRot.v.z);
					CryLog(TITLE "File model : %s", player.m_PlayerModel);
					CryLog(TITLE "Nickname : %s", player.m_PlayerNickname);
				}
				else
					CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Error spawning local player - Can't get player from actor");
			}
#endif

			pActor->SetHealth(pActor->GetMaxHealth());

			player.pActor = pActor;
			m_NetPlayers.push_back(player);

			CryLog(TITLE "FireNet player (%s : %d) successfully spawned", player.m_PlayerNickname, player.m_PlayerUID);

			return true;
		}
		else
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't spawn FireNet player (%s : %d) - Can't spawn actor", player.m_PlayerNickname, player.m_PlayerUID);
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't spawn FireNet player (%s : %d) - Can't get actor system pointer", player.m_PlayerNickname, player.m_PlayerUID);


	return false;
}

void CGameStateSynchronization::RemoveNetPlayer(uint32 uid)
{
	CryLog(TITLE "Removing FireNet player (%d)", uid);

	auto pActorSystem = gEnv->pGameFramework->GetIActorSystem();

	for (const auto &it : m_NetPlayers)
	{
		if (it.m_PlayerUID == uid && pActorSystem)
		{
			if (auto pActor = pActorSystem->GetActorByChannelId(it.m_ChanelId))
			{
				pActorSystem->RemoveActor(pActor->GetEntityId());
			}
			break;
		}
	}
}

void CGameStateSynchronization::HideNetPlayer(uint32 uid)
{
	CryLog(TITLE "Hiding FireNet player (%d)", uid);

	IEntity* pEntity = nullptr;

	auto pActorSystem = gEnv->pGameFramework->GetIActorSystem();

	for (const auto &it : m_NetPlayers)
	{
		if (it.m_PlayerUID == uid && pActorSystem)
		{
			if (auto pActor = pActorSystem->GetActorByChannelId(it.m_ChanelId))
			{
				pEntity = pActor->GetEntity();
			}
			
			break;
		}
	}

	if (pEntity)
		pEntity->Hide(true);
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't hide FireNet player (%d)", uid);
}

void CGameStateSynchronization::UnhideNetPlayer(uint32 uid)
{
	CryLog(TITLE "Unhiding FireNet player (%d)", uid);

	IEntity* pEntity = nullptr;

	auto pActorSystem = gEnv->pGameFramework->GetIActorSystem();

	for (const auto &it : m_NetPlayers)
	{
		if (it.m_PlayerUID == uid && pActorSystem)
		{
			if (auto pActor = pActorSystem->GetActorByChannelId(it.m_ChanelId))
			{
				pEntity = pActor->GetEntity();
			}

			break;
		}
	}

	if (pEntity)
		pEntity->Hide(false);
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't unhide FireNet player (%d)", uid);
}

void CGameStateSynchronization::SyncNetPlayerInput(uint32 uid, const SFireNetClientInput &input)
{
	IActor* pActor = nullptr;
	auto pActorSystem = gEnv->pGameFramework->GetIActorSystem();

	for (const auto &it : m_NetPlayers)
	{
		if (it.m_PlayerUID == uid && pActorSystem)
		{
			pActor = pActorSystem->GetActorByChannelId(it.m_ChanelId);
			break;
		}
	}

	CFireNetPlayer* pPlayer = pActor ? dynamic_cast<CFireNetPlayer*>(pActor) : nullptr;

	if (!pPlayer)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync input for FireNet player (%d). Can't get player instance", uid);
		return;
	}

	//! Sync input
	pPlayer->SyncNetInput(input);
}

void CGameStateSynchronization::SyncNetPlayerPosRot(uint32 uid, const Vec3 & pos, const Quat & rot)
{
	IActor* pActor = nullptr;
	auto pActorSystem = gEnv->pGameFramework->GetIActorSystem();

	for (const auto &it : m_NetPlayers)
	{
		if (it.m_PlayerUID == uid && pActorSystem)
		{
			pActor = pActorSystem->GetActorByChannelId(it.m_ChanelId);
			break;
		}
	}

	CFireNetPlayer* pPlayer = pActor ? dynamic_cast<CFireNetPlayer*>(pActor) : nullptr;

	if (!pPlayer)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync position for FireNet player (%d). pPlayer = nullptr", uid);
		return;
	}

	if (auto* pEntity = pPlayer->GetEntity())
	{
		pEntity->SetPosRotScale(pos, rot, Vec3(1, 1, 1));
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync position FireNet player (%d)", uid);
}