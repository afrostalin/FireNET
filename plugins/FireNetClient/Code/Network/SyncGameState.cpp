// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "SyncGameState.h"

#include "NetActor/NetPlayer.h"
#include "NetActor/Input/NetPlayerInput.h"

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

void CGameStateSynchronization::SpawnNetPlayer(SNetPlayer & player)
{
	CryLog(TITLE "Spawning NetPlayer %d", player.m_PlayerUID);

	if (auto *pActorSystem = gEnv->pGameFramework->GetIActorSystem())
	{
		auto* pPlayer = reinterpret_cast<CNetPlayer*> (pActorSystem->CreateActor(player.m_ChanelId, player.m_PlayerNickname, "NetPlayer", player.m_PlayerSpawnPos, player.m_PlayerSpawnRot, Vec3(1, 1, 1)));

		if (pPlayer)
		{
			player.pPlayer = pPlayer;
			m_NetPlayers.push_back(player);
		}
		else
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't spawn NetPlayer");
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't spawn NetPlayer");
}

void CGameStateSynchronization::RemoveNetPlayer(uint uid)
{
	CryLog(TITLE "Removing NetPlayer %d", uid);

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
	CryLog(TITLE "Hiding NetPlayer %d", uid);

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
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't hide NetPlayer %d", uid);
}

void CGameStateSynchronization::UnhideNetPlayer(uint uid)
{
	CryLog(TITLE "Unhiding NetPlayer %d", uid);

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
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't unhide NetPlayer %d", uid);
}

void CGameStateSynchronization::SyncNetPlayerAction(uint uid, SNetPlayerAction & action)
{
	CNetPlayer* pPlayer = nullptr;
	CNetPlayerInput* pInput = nullptr;

	for (auto it = m_NetPlayers.begin(); it != m_NetPlayers.end(); ++it)
	{
		if (it->m_PlayerUID == uid && it->pPlayer)
		{
			pPlayer = it->pPlayer;
			pInput = pPlayer->GetInput();
			break;
		}
	}

	if (!pPlayer || !pInput)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync action for NetPlayer. pPlayer = nullptr or pInput = nullptr", uid);
		return;
	}

	pInput->SetInputFlags(action.m_action);

	if (action.m_action & E_ACTION_JUMP)
	{
		pInput->OnActionJump();
	}
	if (action.m_action & E_ACTION_SPRINT)
	{
		pInput->OnActionSprint();
	}
	if (action.m_action & E_ACTION_SHOOT)
	{
		/*pInput->OnActionShoot();*/
	}
	if (action.m_action & E_ACTION_MOUSE_ROTATE_YAW)
	{
		pInput->OnActionMouseRotateYaw(action.m_value);
	}
	if (action.m_action & E_ACTION_MOUSE_ROTATE_PITCH)
	{
		pInput->OnActionMouseRotatePitch(action.m_value);
	}
}

void CGameStateSynchronization::SyncNetPlayerPos(uint uid, Vec3 & pos)
{
	CNetPlayer* pPlayer = nullptr;

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
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync position for NetPlayer. pPlayer = nullptr", uid);
		return;
	}

	if (auto* pEntity = pPlayer->GetEntity())
	{
		if (pEntity->GetPos() != pos)
			pEntity->SetPos(pos);
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync position NetPlayer %d", uid);
}

void CGameStateSynchronization::SyncNetPlayerRot(uint uid, Quat & rot)
{
	CNetPlayer* pPlayer = nullptr;

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
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync rotation for NetPlayer. pPlayer = nullptr", uid);
		return;
	}

	if (auto* pEntity = pPlayer->GetEntity())
	{
		if (pEntity->GetRotation() != rot)
			pEntity->SetRotation(rot);
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync position NetPlayer %d", uid);
}
