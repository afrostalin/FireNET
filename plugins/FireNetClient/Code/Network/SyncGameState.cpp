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

	for (auto it = m_NetPlayers.begin(); it != m_NetPlayers.end(); ++it)
	{
		RemoveNetPlayer(it->m_PlayerUID);
	}

	m_NetPlayers.clear();
}

void CGameStateSynchronization::SpawnNetPlayer(SFireNetSyncronizationClient & player)
{
	CryLog(TITLE "Spawning FireNet player (%d)", player.m_PlayerUID);

	if (auto *pActorSystem = gEnv->pGameFramework->GetIActorSystem())
	{
		auto* pPlayer = reinterpret_cast<CFireNetPlayer*> (pActorSystem->CreateActor(player.m_ChanelId, player.m_PlayerNickname, "FireNetPlayer", player.m_PlayerSpawnPos, player.m_PlayerSpawnRot, Vec3(1, 1, 1)));

		if (pPlayer)
		{
			player.pPlayer = pPlayer;
			m_NetPlayers.push_back(player);
		}
		else
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't spawn FireNet player");
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't spawn FireNet player");
}

void CGameStateSynchronization::RemoveNetPlayer(uint uid)
{
	CryLog(TITLE "Removing FireNet player (%d)", uid);

	auto *pActorSystem = gEnv->pGameFramework->GetIActorSystem();

	for (auto it = m_NetPlayers.begin(); it != m_NetPlayers.end(); ++it)
	{
		if (it->m_PlayerUID == uid && it->pPlayer && pActorSystem)
		{			
			if (pActorSystem->GetActorByChannelId(it->m_ChanelId))
			{
				pActorSystem->RemoveActor(it->pPlayer->GetEntityId());
			}		
			break;
		}
	}
}

void CGameStateSynchronization::HideNetPlayer(uint uid)
{
	CryLog(TITLE "Hiding FireNet player (%d)", uid);

	IEntity* pEntity = nullptr;

	for (auto it = m_NetPlayers.begin(); it != m_NetPlayers.end(); ++it)
	{
		if (it->m_PlayerUID == uid && it->pPlayer)
		{
			pEntity = it->pPlayer->GetEntity();
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

	for (auto it = m_NetPlayers.begin(); it != m_NetPlayers.end(); ++it)
	{
		if (it->m_PlayerUID == uid && it->pPlayer)
		{
			pEntity = it->pPlayer->GetEntity();
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
	CFireNetPlayer* pPlayer = nullptr;

	for (auto it = m_NetPlayers.begin(); it != m_NetPlayers.end(); ++it)
	{
		if (it->m_PlayerUID == uid && it->pPlayer)
		{
			pPlayer = it->pPlayer;
			break;
		}
	}

	if (!pPlayer)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync action for FireNet player (%d). pPlayer = nullptr", uid);
		return;
	}

/*	pPlayer->OnPlayerAction(action.m_action);

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
	CFireNetPlayer* pPlayer = nullptr;

	for (auto it = m_NetPlayers.begin(); it != m_NetPlayers.end(); ++it)
	{
		if (it->m_PlayerUID == uid && it->pPlayer)
		{
			pPlayer = it->pPlayer;
			break;
		}
	}

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
	CFireNetPlayer* pPlayer = nullptr;

	for (auto it = m_NetPlayers.begin(); it != m_NetPlayers.end(); ++it)
	{
		if (it->m_PlayerUID == uid && it->pPlayer)
		{
			pPlayer = it->pPlayer;
			break;
		}
	}

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
