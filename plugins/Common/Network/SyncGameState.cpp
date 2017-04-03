// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "SyncGameState.h"

#include "Entities/FireNetPlayer/FireNetPlayer.h"



CGameStateSynchronization::CGameStateSynchronization()
{
	pActorSystem = gEnv->pGameFramework->GetIActorSystem();
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

	m_NetPlayers.clear();
}

SFireNetClientPlayer* CGameStateSynchronization::GetNetPlayer(uint32 channel)
{
	try
	{
		return &m_NetPlayers.at(channel);
	}
	catch (std::out_of_range)
	{
		return nullptr;
	}
}

bool CGameStateSynchronization::SpawnNetPlayer(uint32 channel, SFireNetClientPlayer & player)
{
	CryLog(TITLE "Spawning FireNet player <%s> (Channel ID = %d)", player.m_PlayerNickname, channel);

	if (pActorSystem)
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
			player.pPlayer = dynamic_cast<CFireNetPlayer*>(pActor);

			if (player.pPlayer)
			{
				CryLog(TITLE "FireNet player <%s>  (Channel ID = %d) successfully spawned", player.m_PlayerNickname, channel);
				m_NetPlayers.insert(NetClientPlayer(channel, player));
				return true;
			}
			else
				CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't spawn FireNet player <%s>  (Channel ID = %d) - Can't get player pointer", player.m_PlayerNickname, channel);
		}
		else
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't spawn FireNet player <%s>  (Channel ID = %d) - Can't spawn actor", player.m_PlayerNickname, channel);
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't spawn FireNet player <%s> (Channel ID = %d) - Can't get actor system pointer", player.m_PlayerNickname, channel);


	return false;
}

void CGameStateSynchronization::RemoveNetPlayer(uint32 channel)
{
	CryLog(TITLE "Removing FireNet player by channel (Channel ID = %d) ...", channel);

	if (pActorSystem)
	{
		if (auto pNetPlayer = GetNetPlayer(channel))
		{
			if (pNetPlayer->pActor && pNetPlayer->pActor->GetEntity())
			{
				pActorSystem->RemoveActor(pNetPlayer->pActor->GetEntityId());
				CryLog(TITLE "FireNet player removed by channel (Channel ID = %d)", channel);
			}

			RemoveNetPlayerForMap(channel);
		}
		else
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't remove FireNet player by channel (Channel ID = %d) - Can't get player pointer", channel);
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't remove FireNet player by channel (Channel ID = %d) - Can't get actor system pointer", channel);
}

void CGameStateSynchronization::HideNetPlayer(uint32 channel, bool hide)
{
	CryLog(TITLE "Hiding (%s) FireNet player by channel (Channel ID = %d)", hide ? "true" : "false", channel);

	if (pActorSystem)
	{
		if (auto pNetPlayer = GetNetPlayer(channel))
		{
			if (pNetPlayer->pActor && pNetPlayer->pActor->GetEntity())
				pNetPlayer->pActor->GetEntity()->Hide(hide);
		}
		else
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't hide (%s) FireNet player by channel (Channel ID = %d) - Can't get player pointer", hide ? "true" : "false", channel);
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't hide (%s) FireNet player by channel (Channel ID = %d) - Can't get actor system pointer", hide ? "true" : "false", channel);
}

void CGameStateSynchronization::SyncNetPlayerInput(uint32 channel, const SFireNetClientInput &input)
{
	//CryLog(TITLE "Sync FireNet player input by channel (Channel ID = %d)", channel);

	if (pActorSystem)
	{
		if (auto pNetPlayer = GetNetPlayer(channel))
		{
			if (pNetPlayer->pPlayer)
				pNetPlayer->pPlayer->SyncNetInput(input);
		}
		else
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync FireNet player input by channel (Channel ID = %d) - Can't get player pointer", channel);
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync FireNet player input by channel (Channel ID = %d) - Can't get actor system pointer", channel);
}

void CGameStateSynchronization::SyncNetPlayerPosRot(uint32 channel, const Vec3 & pos, const Quat & rot)
{
	//CryLog(TITLE "Sync FireNet player pos/rot by channel (Channel ID = %d)", channel);

	if (pActorSystem)
	{
		if (auto pNetPlayer = GetNetPlayer(channel))
		{
			if (pNetPlayer->pActor && pNetPlayer->pActor->GetEntity())
				pNetPlayer->pActor->GetEntity()->SetPosRotScale(pos, rot, Vec3(1, 1, 1));
		}
		else
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync FireNet player pos/rot by channel (Channel ID = %d) - Can't get player pointer", channel);
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Can't sync FireNet player pos/rot by channel (Channel ID = %d) - Can't get actor system pointer", channel);
}

void CGameStateSynchronization::RemoveNetPlayerForMap(uint32 channel)
{
	try
	{
		m_NetPlayers.erase(channel);
	}
	catch (std::out_of_range)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, TITLE "Can't remove FireNet player for map - unknown channel (Channel ID = %d)", channel);
	}
}
