// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#include "StdAfx.h"
#include "FireNetPlayer.h"

#include "Movement/FireNetPlayerMovement.h"
#include "Input/FireNetPlayerInput.h"
#include "View/FireNetPlayerView.h"
#include "Animations/FireNetPlayerAnimations.h"

#include "Main/Plugin.h"
#include "GameRules/FireNetGameRules.h"

#include "Entities/FireNetSpawnPoint/FireNetSpawnPoint.h"
#include "Entities/ISimpleWeapon.h"

#include <CryRenderer/IRenderAuxGeom.h>

#include <FireNet>

class CPlayerRegistrator
	: public IEntityRegistrator
	, public CFireNetPlayer::SExternalCVars
{
	virtual void Register() override
	{
		CFireNetCorePlugin::RegisterEntityWithDefaultComponent<CFireNetPlayer>("FireNetPlayer", "Characters", "character.bmp", true);

		CFireNetCorePlugin::RegisterEntityComponent<CFireNetPlayerMovement>("FireNetPlayerMovement");
		CFireNetCorePlugin::RegisterEntityComponent<CFireNetPlayerInput>("FireNetPlayerInput");
		CFireNetCorePlugin::RegisterEntityComponent<CFireNetPlayerView>("FireNetPlayerView");
		CFireNetCorePlugin::RegisterEntityComponent<CFireNetPlayerAnimations>("FireNetPlayerAnimations");
		
		RegisterCVars();
	}

	virtual void Unregister() override 
	{
		UnregisterCVars();
	}

	void RegisterCVars()
	{
		REGISTER_CVAR2("pl_mass", &m_mass, 90.f, VF_CHEAT, "Mass of the player entity in kg");

		REGISTER_CVAR2("pl_moveSpeed", &m_moveSpeed, 20.5f, VF_CHEAT, "Speed at which the player moves");

		REGISTER_CVAR2("pl_jumpHeightMultiplayer", &m_jumpHeightMultiplier, 1.3f, VF_CHEAT, "Player jump height multiplier");
		REGISTER_CVAR2("pl_viewOffsetForward", &m_viewOffsetY, -1.5f, VF_CHEAT, "View offset along the forward axis from the player entity");
		REGISTER_CVAR2("pl_viewOffsetUp", &m_viewOffsetZ, 2.f, VF_CHEAT, "View offset along the up axis from the player entity");

		REGISTER_CVAR2("pl_rotationSpeedYaw", &m_rotationSpeedYaw, 0.05f, VF_CHEAT, "Speed at which the player rotates entity yaw");
		REGISTER_CVAR2("pl_rotationSpeedPitch", &m_rotationSpeedPitch, 0.05f, VF_CHEAT, "Speed at which the player rotates entity pitch");

		REGISTER_CVAR2("pl_rotationLimitsMinPitch", &m_rotationLimitsMinPitch, -0.84f, VF_CHEAT, "Minimum entity pitch limit");
		REGISTER_CVAR2("pl_rotationLimitsMaxPitch", &m_rotationLimitsMaxPitch, 1.5f, VF_CHEAT, "Maximum entity pitch limit");

		REGISTER_CVAR2("pl_eyeHeight", &m_playerEyeHeight, 0.935f, VF_CHEAT, "Height of the player's eyes from ground");

		REGISTER_CVAR2("pl_cameraOffsetY", &m_cameraOffsetY, 0.01f, VF_CHEAT, "Forward positional offset from camera joint");
		REGISTER_CVAR2("pl_cameraOffsetZ", &m_cameraOffsetZ, 0.26f, VF_CHEAT, "Vertical positional offset from camera joint");

		m_pGeometry = REGISTER_STRING("pl_geometry", "Objects/Characters/SampleCharacter/thirdperson.cdf", VF_CHEAT, "Sets the first person geometry to load");
		m_pCameraJointName = REGISTER_STRING("pl_cameraJointName", "head", VF_CHEAT, "Sets the name of the joint managing the player's view position");

		m_pMannequinContext = REGISTER_STRING("pl_MannequinContext", "FirstPersonCharacter", VF_CHEAT, "The name of the FP context used in Mannequin");
		m_pAnimationDatabase = REGISTER_STRING("pl_AnimationDatabase", "Animations/Mannequin/ADB/FirstPerson.adb", VF_CHEAT, "Path to the animation database file to load");
		m_pControllerDefinition = REGISTER_STRING("pl_ControllerDefinition", "Animations/Mannequin/ADB/FirstPersonControllerDefinition.xml", VF_CHEAT, "Path to the controller definition file to load");
	}

	void UnregisterCVars()
	{
		IConsole* pConsole = gEnv->pConsole;
		if (pConsole)
		{
			pConsole->UnregisterVariable("pl_mass");
			pConsole->UnregisterVariable("pl_moveSpeed");

			pConsole->UnregisterVariable("pl_jumpHeightMultiplayer");
			pConsole->UnregisterVariable("pl_viewOffsetForward");
			pConsole->UnregisterVariable("pl_viewOffsetUp");

			pConsole->UnregisterVariable("pl_rotationSpeedYaw");
			pConsole->UnregisterVariable("pl_rotationSpeedPitch");
			pConsole->UnregisterVariable("pl_rotationLimitsMinPitch");
			pConsole->UnregisterVariable("pl_rotationLimitsMaxPitch");
			pConsole->UnregisterVariable("pl_eyeHeight");
			pConsole->UnregisterVariable("pl_cameraOffsetY");
			pConsole->UnregisterVariable("pl_cameraOffsetZ");
			pConsole->UnregisterVariable("pl_geometry");
			pConsole->UnregisterVariable("pl_cameraJointName");
			pConsole->UnregisterVariable("pl_MannequinContext");
			pConsole->UnregisterVariable("pl_AnimationDatabase");
			pConsole->UnregisterVariable("pl_ControllerDefinition");
		}
	}
};

CPlayerRegistrator g_playerRegistrator;

CFireNetPlayer::CFireNetPlayer()
	: m_pInput(nullptr)
	, m_pMovement(nullptr)
	, m_pView(nullptr)
	, m_bAlive(false)
	, m_bThirdPerson(false)
	, m_pCurrentWeapon(nullptr)
{
}

CFireNetPlayer::~CFireNetPlayer()
{
	gEnv->pGameFramework->GetIActorSystem()->RemoveActor(GetEntityId());
}

const CFireNetPlayer::SExternalCVars &CFireNetPlayer::GetCVars() const
{
	return g_playerRegistrator;
}

bool CFireNetPlayer::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);
	return pGameObject->BindToNetwork();
}

void CFireNetPlayer::PostInit(IGameObject *pGameObject)
{
	m_pMovement = static_cast<CFireNetPlayerMovement *>(GetGameObject()->AcquireExtension("FireNetPlayerMovement"));
	m_pAnimations = static_cast<CFireNetPlayerAnimations *>(GetGameObject()->AcquireExtension("FireNetPlayerAnimations"));
	m_pInput = static_cast<CFireNetPlayerInput *>(GetGameObject()->AcquireExtension("FireNetPlayerInput"));
	m_pView = static_cast<CFireNetPlayerView *>(GetGameObject()->AcquireExtension("FireNetPlayerView"));

	// Register with the actor system
	gEnv->pGameFramework->GetIActorSystem()->AddActor(GetEntityId(), this);

	pGameObject->EnableUpdateSlot(this, 0);

	if (gEnv->IsEditor())
	{
		SetHealth(GetMaxHealth());
	}
}

void CFireNetPlayer::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
		case ENTITY_EVENT_RESET:
		{
			switch (event.nParam[0])
			{
			case 0: // Game ends
			{
				m_pCurrentWeapon = nullptr;
				break;
			}
			case 1: // Game starts
			{
				if (gEnv->IsEditor())
				{
					SetHealth(GetMaxHealth());
					if (!m_pCurrentWeapon)
						CreateWeapon("FireNetRifle");
				}

				break;
			}
			default:
				break;
			}
		}
		break;
		case ENTITY_EVENT_HIDE:
		{
			// Hide the weapon too
			if (m_pCurrentWeapon != nullptr)
			{
				m_pCurrentWeapon->GetEntity()->Hide(true);
			}
		}
		break;
		case ENTITY_EVENT_UNHIDE:
		{
			// Unhide the weapon too
			if (m_pCurrentWeapon != nullptr)
			{
				m_pCurrentWeapon->GetEntity()->Hide(false);
			}
		}
		break;
	}
}

void CFireNetPlayer::Update(SEntityUpdateContext & ctx, int updateSlot)
{
	// Send moveming request every frame
	if (!gEnv->IsDedicated() && !gEnv->IsEditor() && gFireNet && gFireNet->pClient)
	{
		gFireNet->pClient->SendMovementRequest((EFireNetClientActions)m_pInput->GetInputFlags(), m_pInput->GetInputValues());
	}
}

void CFireNetPlayer::SetHealth(float health)
{
	if (m_bAlive)
		return;

	m_bAlive = true;

	GetEntity()->Hide(false);
	GetEntity()->SetWorldTM(Matrix34::Create(Vec3(1, 1, 1), GetEntity()->GetRotation(), GetEntity()->GetWorldPos()));

	if (!gEnv->IsEditor())
		SetPlayerModel();

	m_pInput->OnPlayerRespawn();

	if (m_pCurrentWeapon == nullptr)
	{
		CreateWeapon("FireNetRifle");
	}
}

void CFireNetPlayer::SetPlayerModel()
{
	GetEntity()->LoadCharacter(eGeometry_Default, GetCVars().m_pGeometry->GetString());
	
	m_pView->OnPlayerModelChanged();
	m_pAnimations->OnPlayerModelChanged();
	m_pMovement->Physicalize();
}

void CFireNetPlayer::CreateWeapon(const char *name)
{
	SEntitySpawnParams spawnParams;

	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(name);

	IEntity *pWeaponEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams);

	if (pWeaponEntity)
	{
		if (auto *pGameObject = gEnv->pGameFramework->GetGameObject(pWeaponEntity->GetId()))
		{
			if (auto *pWeapon = pGameObject->QueryExtension(name))
				m_pCurrentWeapon = static_cast<ISimpleWeapon *>(pWeapon);
			else
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to query game object extension for weapon %s!", name);
		}
		else
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Spawned weapon of type %s but failed to get game object!", name);
	}
	else
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Can't spawn weapon of type %s - class not found", name);
}