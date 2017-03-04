// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#include "StdAfx.h"
#include "FireNetBullet.h"

#include "Main/Plugin.h"

class CFireNetBulletRegistrator
	: public IEntityRegistrator
{
	virtual void Register() override
	{
		CFireNetCorePlugin::RegisterEntityWithDefaultComponent<CFireNetBullet>("FireNetBullet");

		RegisterCVars();
	}

	virtual void Unregister() override
	{
		UnregisterCVars();
	}

	void RegisterCVars()
	{
		m_pGeometry = gEnv->pConsole->RegisterString("w_bulletGeometry", "Objects/Default/primitive_sphere.cgf", VF_CHEAT, "Sets the path to the geometry assigned to every weapon bullet");

		REGISTER_CVAR2("w_bulletMass", &m_mass, 5000.f, VF_CHEAT, "Sets the mass of each individual bullet fired by weapons");
		REGISTER_CVAR2("w_bulletInitialVelocity", &m_initialVelocity, 10.f, VF_CHEAT, "Sets the initial velocity of each individual bullet fired by weapons");
	}

	void UnregisterCVars()
	{
		IConsole* pConsole = gEnv->pConsole;
		if (pConsole)
		{
			pConsole->UnregisterVariable("w_bulletGeometry");
			pConsole->UnregisterVariable("w_bulletMass");
			pConsole->UnregisterVariable("w_bulletInitialVelocity");
		}
	}

public:
	ICVar *m_pGeometry;

	float m_mass;
	float m_initialVelocity;
};

CFireNetBulletRegistrator g_bulletRegistrator;

void CFireNetBullet::PostInit(IGameObject *pGameObject)
{
	// Make sure we get logged collision events
	// Note the difference between immediate (directly on the physics thread) and logged (delayed to run on main thread) physics events.
	pGameObject->EnablePhysicsEvent(true, eEPE_OnCollisionLogged);

	const int requiredEvents[] = { eGFE_OnCollision };
	pGameObject->UnRegisterExtForEvents(this, NULL, 0);
	pGameObject->RegisterExtForEvents(this, requiredEvents, sizeof(requiredEvents) / sizeof(int));

	// Set the model
	const int geometrySlot = 0;

	LoadMesh(geometrySlot, g_bulletRegistrator.m_pGeometry->GetString());

	// Load the custom bullet material.
	// This material has the 'mat_bullet' surface type applied, which is set up to play sounds on collision with 'mat_default' objects in Libs/MaterialEffects
	auto *pBulletMaterial = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("Materials/bullet");
	GetEntity()->SetMaterial(pBulletMaterial);

	// Now create the physical representation of the entity
	Physicalize();

	// Make sure that bullets are always rendered regardless of distance
	// Ratio is 0 - 255, 255 being 100% visibility
	GetEntity()->SetViewDistRatio(255);

	// Apply an impulse so that the bullet flies forward
	if (auto *pPhysics = GetEntity()->GetPhysics())
	{
		pe_action_impulse impulseAction;

		// Set the actual impulse, in this cause the value of the initial velocity CVar in bullet's forward direction
		impulseAction.impulse = GetEntity()->GetWorldRotation().GetColumn1() * g_bulletRegistrator.m_initialVelocity;

		// Send to the physical entity
		pPhysics->Action(&impulseAction);
	}
}

void CFireNetBullet::HandleEvent(const SGameObjectEvent &event)
{
	// Handle the OnCollision event, in order to have the entity removed on collision
	if (event.event == eGFE_OnCollision)
	{
		// Collision info can be retrieved using the event pointer
		//EventPhysCollision *physCollision = reinterpret_cast<EventPhysCollision *>(event.ptr);

		// Queue removal of this entity, unless it has already been done
		gEnv->pEntitySystem->RemoveEntity(GetEntityId());
	}
}

void CFireNetBullet::Physicalize()
{
	SEntityPhysicalizeParams physParams;
	physParams.type = PE_RIGID;

	physParams.mass = g_bulletRegistrator.m_mass;

	GetEntity()->Physicalize(physParams);
}