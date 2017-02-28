// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#include "StdAfx.h"
#include "FireNetSpawnPoint.h"
#include "Main/Plugin.h"
#include "Actors/FireNetPlayer.h"

#include <CryEntitySystem/IEntitySystem.h>

class CFireNetSpawnPointRegistrator : public IEntityRegistrator
{
	virtual void Register() override
	{
		CFireNetCorePlugin::RegisterEntityWithDefaultComponent<CFireNetSpawnPoint>("FireNetSpawnPoint", "Gameplay", "SpawnPoint.bmp", true);
	}

	virtual void Unregister() override {}
};

CFireNetSpawnPointRegistrator g_spawnerRegistrator;

void CFireNetSpawnPoint::PostInit(IGameObject * pGameObject)
{
	if (gEnv->IsEditor())
		LoadMesh(0, "Editor/Objects/spawnpointhelper.cgf");
}

void CFireNetSpawnPoint::ProcessEvent(SEntityEvent & event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_RESET:
	{
		switch (event.nParam[0])
		{
		case 0: // Game ends
			GetEntity()->Hide(false);
			break;
		case 1: // Game starts
			GetEntity()->Hide(true);
			break;
		default:
			break;
		}
	}
	}
}

void CFireNetSpawnPoint::SpawnEntity(IEntity &otherEntity)
{
	if (bEnabled)
		otherEntity.SetWorldTM(GetEntity()->GetWorldTM());
}