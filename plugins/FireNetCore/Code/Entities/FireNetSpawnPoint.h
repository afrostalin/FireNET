// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#pragma once

#include <CryEntitySystem/IEntitySystem.h>
#include "Entities/ISimpleExtension.h"

class CFireNetSpawnPoint : public ISimpleExtension
	, public IEntityPropertyGroup
{
public:
	virtual ~CFireNetSpawnPoint() {}

	//! Entity properties
	virtual IEntityPropertyGroup* GetPropertyGroup() { return this; }
	virtual const char*           GetLabel() const override { return "FireNetSpawnPoint"; }
	virtual void                  SerializeProperties(Serialization::IArchive& archive) override
	{
		archive(bEnabled, "Enabled", "Enabled");
	};

	// ISimpleExtension	
	virtual void PostInit(IGameObject *pGameObject) override;
	virtual void ProcessEvent(SEntityEvent& event) override;
	// ~ISimpleExtension

	void         SpawnEntity(IEntity &otherEntity);
	bool         IsEnabled() { return bEnabled; }
protected:
	bool         bEnabled = false;
};