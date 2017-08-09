// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "FireNetGamerules.h"
#include "Main/Plugin.h"

#include <Entities/FireNetPlayer/FireNetPlayer.h>

#include <IActorSystem.h>

#ifndef USE_DEFAULT_DEDICATED_SERVER
class CFireNetRulesRegistrator : public IEntityRegistrator
{
	virtual void Register() override
	{
		CFireNetCorePlugin::RegisterEntityWithDefaultComponent<CFireNetGameRules>("FireNetGameRules", "GameRules");

		ICVar *pDefaultGameRulesVar = gEnv->pConsole->GetCVar("sv_gamerulesdefault");

		gEnv->pGameFramework->GetIGameRulesSystem()->RegisterGameRules(pDefaultGameRulesVar->GetString(), "FireNetGameRules");
		gEnv->pGameFramework->GetIGameRulesSystem()->AddGameRulesAlias(pDefaultGameRulesVar->GetString(), pDefaultGameRulesVar->GetString());
	}

	virtual void Unregister() override {}
};

CFireNetRulesRegistrator g_gameRulesRegistrator;

CFireNetGameRules::~CFireNetGameRules()
{
	gEnv->pGameFramework->GetIGameRulesSystem()->SetCurrentGameRules(nullptr);
}

bool CFireNetGameRules::Init(IGameObject * pGameObject)
{
	SetGameObject(pGameObject);

	if (!pGameObject->BindToNetwork())
			return false;

	gEnv->pGameFramework->GetIGameRulesSystem()->SetCurrentGameRules(this);

	return true;
}

bool CFireNetGameRules::OnClientConnect(int channelId, bool isReset)
{
	if (gEnv->IsEditor())
	{
		auto pActorSystem = gEnv->pGameFramework->GetIActorSystem();

		auto pActor = pActorSystem->CreateActor(channelId, "EditorPlayer", "FireNetPlayer", ZERO, IDENTITY, Vec3(1, 1, 1));

		if (pActor)
		{
			if (auto pPlayer = dynamic_cast<CFireNetPlayer*>(pActor))
			{
				pPlayer->SetLocalPlayer(true);
			}
		}

		return pActor ? true : false;
	}

	return true;
}

void CFireNetGameRules::OnClientDisconnect(int channelId, EDisconnectionCause cause, const char * desc, bool keepClient)
{
	if (gEnv->IsEditor())
	{
		auto *pActorSystem = gEnv->pGameFramework->GetIActorSystem();
		if (IActor *pActor = pActorSystem->GetActorByChannelId(channelId))
		{
			pActorSystem->RemoveActor(pActor->GetEntityId());
		}
	}
}

bool CFireNetGameRules::OnClientEnteredGame(int channelId, bool isReset)
{
	if (gEnv->IsEditor())
	{
		if (!gEnv->IsEditing())
		{
			auto *pActorSystem = gEnv->pGameFramework->GetIActorSystem();

			if (auto *pActor = pActorSystem->GetActorByChannelId(channelId))
			{
				pActor->SetHealth(pActor->GetMaxHealth());
			}
		}
	}

	return true;
}
#endif