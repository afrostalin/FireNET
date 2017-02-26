// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "NetPlayer.h"

#include "Movement/NetPlayerMovement.h"
#include "Input/NetPlayerInput.h"
#include "Animations/NetPlayerAnimations.h"

#include "Main/Plugin.h"

#include <CryRenderer/IRenderAuxGeom.h>

class CNetPlayerRegistrator : public IEntityRegistrator , public CNetPlayer::SExternalCVars
{
	virtual void Register() override
	{
		CFireNetClientPlugin::RegisterEntityWithDefaultComponent<CNetPlayer>("NetPlayer", "Characters", "character.bmp", true);

		CFireNetClientPlugin::RegisterEntityComponent<CNetPlayerMovement>("NetPlayerMovement");
		CFireNetClientPlugin::RegisterEntityComponent<CNetPlayerInput>("NetPlayerInput");
		CFireNetClientPlugin::RegisterEntityComponent<CNetPlayerAnimations>("NetPlayerAnimations");
		
		RegisterCVars();
	}

	virtual void Unregister() override 
	{
		UnregisterCVars();
	}

	void RegisterCVars()
	{
		// Load default player params
		auto pMass = gEnv->pConsole->GetCVar("pl_mass");
		m_mass = pMass ? pMass->GetFVal() : 0.f;

		auto pMoveSpeed = gEnv->pConsole->GetCVar("pl_moveSpeed");
		m_moveSpeed = pMoveSpeed ? pMoveSpeed->GetFVal() : 0.f;

		auto pJumpHeight = gEnv->pConsole->GetCVar("pl_jumpHeightMultiplayer");
		m_jumpHeightMultiplier = pJumpHeight ? pJumpHeight->GetFVal() : 0.f;

		auto pRotSpeedYaw = gEnv->pConsole->GetCVar("pl_rotationSpeedYaw");
		m_rotationSpeedYaw = pRotSpeedYaw ? pRotSpeedYaw->GetFVal() : 0.f;
		auto pRotSpeedPitch = gEnv->pConsole->GetCVar("pl_rotationSpeedPitch");
		m_rotationSpeedPitch = pRotSpeedPitch ? pRotSpeedPitch->GetFVal() : 0.f;

		auto pRotLimitsMin = gEnv->pConsole->GetCVar("pl_rotationLimitsMinPitch");
		m_rotationLimitsMinPitch = pRotLimitsMin ? pRotLimitsMin->GetFVal() : 0.f;
		auto pRotLimitsMax = gEnv->pConsole->GetCVar("pl_rotationLimitsMaxPitch");
		m_rotationLimitsMaxPitch = pRotLimitsMax ? pRotLimitsMax->GetFVal() : 0.f;

		auto pPlayerEyeHeight = gEnv->pConsole->GetCVar("pl_eyeHeight");
		m_playerEyeHeight = pPlayerEyeHeight ? pPlayerEyeHeight->GetFVal() : 0.f;

		// Create new CVars for NetPlayer
		m_pThirdPersonGeometry = REGISTER_STRING("net_pl_thirdPersonGeometry", "Objects/Characters/SampleCharacter/thirdperson.cdf", VF_CHEAT, "Sets the third person geometry to load");
		m_pThirdPersonMannequinContext = REGISTER_STRING("net_pl_thirdPersonMannequinContext", "FirstPersonCharacter", VF_CHEAT, "The name of the FP context used in Mannequin");
		m_pThirdPersonAnimationDatabase = REGISTER_STRING("net_pl_thirdPersonAnimationDatabase", "Animations/Mannequin/ADB/FirstPerson.adb", VF_CHEAT, "Path to the animation database file to load");
		m_pThirdPersonControllerDefinition = REGISTER_STRING("net_pl_thirdPersonControllerDefinition", "Animations/Mannequin/ADB/FirstPersonControllerDefinition.xml", VF_CHEAT, "Path to the controller definition file to load");
	}

	void UnregisterCVars()
	{
		if (gEnv && gEnv->pConsole)
		{
			gEnv->pConsole->UnregisterVariable("net_pl_thirdPersonGeometry");
			gEnv->pConsole->UnregisterVariable("net_pl_thirdPersonMannequinContext");
			gEnv->pConsole->UnregisterVariable("net_pl_thirdPersonAnimationDatabase");
			gEnv->pConsole->UnregisterVariable("net_pl_thirdPersonControllerDefinition");
		}
	}
};

CNetPlayerRegistrator g_netPlayerRegistrator;

CNetPlayer::CNetPlayer()
	: m_pInput(nullptr)
	, m_pMovement(nullptr)
	, m_pAnimations(nullptr)
	, m_bAlive(false)
{
	
}

CNetPlayer::~CNetPlayer()
{
	gEnv->pGameFramework->GetIActorSystem()->RemoveActor(GetEntityId());
}

const CNetPlayer::SExternalCVars &CNetPlayer::GetCVars() const
{
	return g_netPlayerRegistrator;
}

bool CNetPlayer::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);

	return true;
}

void CNetPlayer::PostInit(IGameObject *pGameObject)
{
	m_pMovement = static_cast<CNetPlayerMovement *>(GetGameObject()->AcquireExtension("NetPlayerMovement"));
	m_pAnimations = static_cast<CNetPlayerAnimations *>(GetGameObject()->AcquireExtension("NetPlayerAnimations"));
	m_pInput = static_cast<CNetPlayerInput *>(GetGameObject()->AcquireExtension("NetPlayerInput"));

	gEnv->pGameFramework->GetIActorSystem()->AddActor(GetEntityId(), this);

	SetHealth(GetMaxHealth());
}

void CNetPlayer::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
		case ENTITY_EVENT_RESET:
		{
			switch (event.nParam[0])
			{
			case 0:
				break;
			case 1:
				SetHealth(GetMaxHealth());
				break;
			default:
				break;
			}
		}
		break;
		case ENTITY_EVENT_HIDE:
		{
		}
		break;
		case ENTITY_EVENT_UNHIDE:
		{
		}
		break;
	}
}

void CNetPlayer::SetHealth(float health)
{
	if (m_bAlive)
		return;

	m_bAlive = true;

	GetEntity()->Hide(false);
	GetEntity()->SetWorldTM(Matrix34::Create(Vec3(1, 1, 1), IDENTITY, GetEntity()->GetWorldPos()));

	SetPlayerModel();

	m_pInput->OnPlayerRespawn();
}

void CNetPlayer::SetPlayerModel()
{
	GetEntity()->LoadCharacter(eGeometry_ThirdPerson, GetCVars().m_pThirdPersonGeometry->GetString());
	m_pAnimations->OnPlayerModelChanged();
	m_pMovement->Physicalize();
}