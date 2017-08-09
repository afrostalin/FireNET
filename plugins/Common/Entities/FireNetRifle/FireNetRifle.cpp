// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#include "StdAfx.h"
#include "FireNetRifle.h"

#include "Main/Plugin.h"

#include "CryEntitySystem/IEntitySystem.h"

#ifndef USE_DEFAULT_DEDICATED_SERVER
class CFireNetRifleRegistrator
	: public IEntityRegistrator
{
	virtual void Register() override
	{
		CFireNetCorePlugin::RegisterEntityWithDefaultComponent<CFireNetRifle>("FireNetRifle");

		RegisterCVars();
	}

	virtual void Unregister() override
	{
		UnregisterCVars();
	}

	void RegisterCVars()
	{
		REGISTER_CVAR2("w_rifleBulletScale", &m_bulletScale, 0.05f, VF_CHEAT, "Determines the scale of the bullet geometry");
	}

	void UnregisterCVars()
	{
		IConsole* pConsole = gEnv->pConsole;
		if (pConsole)
		{
			pConsole->UnregisterVariable("w_rifleBulletScale");
		}
	}

public:
	float m_bulletScale;
};

CFireNetRifleRegistrator g_rifleRegistrator;

void CFireNetRifle::RequestFire(const Vec3 &initialBulletPosition, const Quat &initialBulletRotation)
{
	SEntitySpawnParams spawnParams;
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("FireNetBullet");

	spawnParams.vPosition = initialBulletPosition;
	spawnParams.qRotation = initialBulletRotation;

	spawnParams.vScale = Vec3(g_rifleRegistrator.m_bulletScale);

	// Spawn the entity, bullet is propelled in CBullet based on the rotation and position here
	gEnv->pEntitySystem->SpawnEntity(spawnParams);
}
#endif