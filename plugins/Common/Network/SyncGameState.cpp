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

	for (const auto &it : m_NetPlayers)
	{
		RemoveNetPlayer(it.m_PlayerUID);
	}

	m_NetPlayers.clear();
}

bool CGameStateSynchronization::SpawnNetPlayer(SFireNetSyncronizationClient & player)
{
	CryLog(TITLE "Spawning FireNet player (%d)", player.m_PlayerUID);

	if (auto pActorSystem = gEnv->pGameFramework->GetIActorSystem())
	{
		auto pActor = pActorSystem->CreateActor(player.m_ChanelId, player.m_PlayerNickname, "FireNetPlayer", player.m_PlayerSpawnPos, player.m_PlayerSpawnRot, Vec3(1, 1, 1));

		if (pActor)
		{
			CryLog(TITLE "FireNet player (%s : %d) successfully spawned", player.m_PlayerNickname, player.m_PlayerUID);

			m_NetPlayers.push_back(player);

			return true;
		}
		else
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't spawn FireNet player (%s : %d) - Can't spawn actor", player.m_PlayerNickname, player.m_PlayerUID);
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't spawn FireNet player (%s : %d) - Can't get actor system pointer", player.m_PlayerNickname, player.m_PlayerUID);


	return false;
}

void CGameStateSynchronization::RemoveNetPlayer(uint uid)
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

void CGameStateSynchronization::HideNetPlayer(uint uid)
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

void CGameStateSynchronization::UnhideNetPlayer(uint uid)
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

void CGameStateSynchronization::SyncNetPlayerAction(uint uid, SFireNetClientAction & action)
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
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync action for FireNet player (%d). pPlayer = nullptr or pInput = nullptr", uid);
		return;
	}

	/*pPlayer->OnPlayerAction(action.m_action);

	if (action.m_action & E_ACTION_JUMP)
	{
		pPlayer->OnPlayerJump();
	}
	if (action.m_action & E_ACTION_SPRINT)
	{
		pPlayer->OnPlayerSprint();
	}
	if (action.m_action & E_ACTION_SHOOT)
	{
		pPlayer->OnPlayerShoot();
	}
	if (action.m_action & E_ACTION_MOUSE_ROTATE_YAW)
	{
		pPlayer->OnPlayerMoveMouseYaw(action.m_value);
	}
	if (action.m_action & E_ACTION_MOUSE_ROTATE_PITCH)
	{
		pPlayer->OnPlayerMoveMousePitch(action.m_value);
	}*/
}

void CGameStateSynchronization::SyncNetPlayerPos(uint uid, Vec3 & pos)
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
		if (pEntity->GetPos() != pos)
			pEntity->SetPos(pos);
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync position FireNet player (%d)", uid);
}

void CGameStateSynchronization::SyncNetPlayerRot(uint uid, Quat & rot)
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
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync rotation for FireNet player (%d). pPlayer = nullptr", uid);
		return;
	}

	if (auto* pEntity = pPlayer->GetEntity())
	{
		if (pEntity->GetRotation() != rot)
			pEntity->SetRotation(rot);
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync position FireNet player (%d)", uid);
}
