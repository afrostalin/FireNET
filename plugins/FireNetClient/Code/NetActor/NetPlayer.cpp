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
		m_mass = gEnv->pConsole->GetCVar("pl_mass")->GetFVal();

		m_moveSpeed = gEnv->pConsole->GetCVar("pl_moveSpeed")->GetFVal();

		m_jumpHeightMultiplier = gEnv->pConsole->GetCVar("pl_jumpHeightMultiplayer")->GetFVal();

		m_rotationSpeedYaw = gEnv->pConsole->GetCVar("pl_rotationSpeedYaw")->GetFVal();
		m_rotationSpeedPitch = gEnv->pConsole->GetCVar("pl_rotationSpeedPitch")->GetFVal();

		m_rotationLimitsMinPitch = gEnv->pConsole->GetCVar("pl_rotationLimitsMinPitch")->GetFVal();
		m_rotationLimitsMaxPitch = gEnv->pConsole->GetCVar("pl_rotationLimitsMaxPitch")->GetFVal();

		m_playerEyeHeight = gEnv->pConsole->GetCVar("pl_eyeHeight")->GetFVal();

		// Create new CVars for NetPlayer
		m_pThirdPersonGeometry = REGISTER_STRING("net_pl_thirdPersonGeometry", "Objects/Characters/SampleCharacter/thirdperson.cdf", VF_CHEAT, "Sets the third person geometry to load");
		m_pThirdPersonMannequinContext = REGISTER_STRING("net_pl_thirdPersonMannequinContext", "FirstPersonCharacter", VF_CHEAT, "The name of the FP context used in Mannequin");
		m_pThirdPersonAnimationDatabase = REGISTER_STRING("net_pl_thirdPersonAnimationDatabase", "Animations/Mannequin/ADB/FirstPerson.adb", VF_CHEAT, "Path to the animation database file to load");
		m_pThirdPersonControllerDefinition = REGISTER_STRING("net_pl_thirdPersonControllerDefinition", "Animations/Mannequin/ADB/FirstPersonControllerDefinition.xml", VF_CHEAT, "Path to the controller definition file to load");
	}

	void UnregisterCVars()
	{
		IConsole* pConsole = gEnv->pConsole;
		if (pConsole)
		{
			pConsole->UnregisterVariable("net_pl_thirdPersonGeometry");
			pConsole->UnregisterVariable("net_pl_thirdPersonMannequinContext");
			pConsole->UnregisterVariable("net_pl_thirdPersonAnimationDatabase");
			pConsole->UnregisterVariable("net_pl_thirdPersonControllerDefinition");
		}
	}
};

CNetPlayerRegistrator g_netPlayerRegistrator;

CNetPlayer::CNetPlayer()
	: m_pInput(nullptr)
	, m_pMovement(nullptr)
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