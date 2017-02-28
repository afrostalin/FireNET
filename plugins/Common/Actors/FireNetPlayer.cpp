// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "FireNetPlayer.h"

#include "Movement/FireNetPlayerMovement.h"
#include "Input/FireNetPlayerInput.h"
#include "Animations/FireNetPlayerAnimations.h"

#include "Main/Plugin.h"

#include <CryRenderer/IRenderAuxGeom.h>

class CFireNetPlayerRegistrator : public IEntityRegistrator , public CFireNetPlayer::SExternalCVars
{
	virtual void Register() override
	{
		CFireNetCorePlugin::RegisterEntityWithDefaultComponent<CFireNetPlayer>("FireNetPlayer", "Characters", "character.bmp", true);

		CFireNetCorePlugin::RegisterEntityComponent<CFireNetPlayerMovement>("FireNetPlayerMovement");
		CFireNetCorePlugin::RegisterEntityComponent<CFireNetPlayerInput>("FireNetPlayerInput");
		CFireNetCorePlugin::RegisterEntityComponent<CFireNetPlayerAnimations>("FireNetPlayerAnimations");
		
		RegisterCVars();
	}

	virtual void Unregister() override 
	{
		UnregisterCVars();
	}

	void RegisterCVars()
	{
		REGISTER_CVAR2("firenet_player_mass", &m_mass, 90.f, VF_CHEAT, "Mass of the player entity in kg");

		REGISTER_CVAR2("firenet_player_moveSpeed", &m_moveSpeed, 20.5f, VF_CHEAT, "Speed at which the player moves");

		REGISTER_CVAR2("firenet_player_jumpHeightMultiplayer", &m_jumpHeightMultiplier, 1.3f, VF_CHEAT, "Player jump height multiplier");

		REGISTER_CVAR2("firenet_player_viewOffsetForward", &m_viewOffsetY, -1.5f, VF_CHEAT, "View offset along the forward axis from the player entity");
		REGISTER_CVAR2("firenet_player_viewOffsetUp", &m_viewOffsetZ, 2.f, VF_CHEAT, "View offset along the up axis from the player entity");

		REGISTER_CVAR2("firenet_player_rotationSpeedYaw", &m_rotationSpeedYaw, 0.05f, VF_CHEAT, "Speed at which the player rotates entity yaw");
		REGISTER_CVAR2("firenet_player_rotationSpeedPitch", &m_rotationSpeedPitch, 0.05f, VF_CHEAT, "Speed at which the player rotates entity pitch");

		REGISTER_CVAR2("firenet_player_rotationLimitsMinPitch", &m_rotationLimitsMinPitch, -0.84f, VF_CHEAT, "Minimum entity pitch limit");
		REGISTER_CVAR2("firenet_player_rotationLimitsMaxPitch", &m_rotationLimitsMaxPitch, 1.5f, VF_CHEAT, "Maximum entity pitch limit");

		REGISTER_CVAR2("firenet_player_eyeHeight", &m_playerEyeHeight, 0.935f, VF_CHEAT, "Height of the player's eyes from ground");

		REGISTER_CVAR2("firenet_player_cameraOffsetY", &m_cameraOffsetY, 0.01f, VF_CHEAT, "Forward positional offset from camera joint");
		REGISTER_CVAR2("firenet_player_cameraOffsetZ", &m_cameraOffsetZ, 0.26f, VF_CHEAT, "Vertical positional offset from camera joint");

		m_pFirstPersonGeometry = REGISTER_STRING("firenet_player_firstPersonGeometry", "Objects/Characters/SampleCharacter/thirdperson.cdf", VF_CHEAT, "Sets the first person geometry to load");
		m_pCameraJointName = REGISTER_STRING("firenet_player_cameraJointName", "head", VF_CHEAT, "Sets the name of the joint managing the player's view position");

		m_pFirstPersonMannequinContext = REGISTER_STRING("firenet_player_firstPersonMannequinContext", "FirstPersonCharacter", VF_CHEAT, "The name of the FP context used in Mannequin");
		m_pFirstPersonAnimationDatabase = REGISTER_STRING("firenet_player_firstPersonAnimationDatabase", "Animations/Mannequin/ADB/FirstPerson.adb", VF_CHEAT, "Path to the animation database file to load");
		m_pFirstPersonControllerDefinition = REGISTER_STRING("firenet_player_firstPersonControllerDefinition", "Animations/Mannequin/ADB/FirstPersonControllerDefinition.xml", VF_CHEAT, "Path to the controller definition file to load");
	}

	void UnregisterCVars()
	{
		if (auto pConsole = gEnv->pConsole)
		{
			pConsole->UnregisterVariable("firenet_player_mass");
			pConsole->UnregisterVariable("firenet_player_moveSpeed");

			pConsole->UnregisterVariable("firenet_player_jumpHeightMultiplayer");
			pConsole->UnregisterVariable("firenet_player_viewOffsetForward");
			pConsole->UnregisterVariable("firenet_player_viewOffsetUp");

			pConsole->UnregisterVariable("firenet_player_rotationSpeedYaw");
			pConsole->UnregisterVariable("firenet_player_rotationSpeedPitch");
			pConsole->UnregisterVariable("firenet_player_rotationLimitsMinPitch");
			pConsole->UnregisterVariable("firenet_player_rotationLimitsMaxPitch");
			pConsole->UnregisterVariable("firenet_player_eyeHeight");
			pConsole->UnregisterVariable("firenet_player_cameraOffsetY");
			pConsole->UnregisterVariable("firenet_player_cameraOffsetZ");
			pConsole->UnregisterVariable("firenet_player_firstPersonGeometry");
			pConsole->UnregisterVariable("firenet_player_cameraJointName");
			pConsole->UnregisterVariable("firenet_player_firstPersonMannequinContext");
			pConsole->UnregisterVariable("firenet_player_firstPersonAnimationDatabase");
			pConsole->UnregisterVariable("firenet_player_firstPersonControllerDefinition");
		}
	}
};

CFireNetPlayerRegistrator g_netPlayerRegistrator;

CFireNetPlayer::CFireNetPlayer()
	: m_pInput(nullptr)
	, m_pMovement(nullptr)
	, m_pAnimations(nullptr)
	, m_bAlive(false)
{
	
}

CFireNetPlayer::~CFireNetPlayer()
{
	gEnv->pGameFramework->GetIActorSystem()->RemoveActor(GetEntityId());
}

const CFireNetPlayer::SExternalCVars &CFireNetPlayer::GetCVars() const
{
	return g_netPlayerRegistrator;
}

bool CFireNetPlayer::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);

	return true;
}

void CFireNetPlayer::PostInit(IGameObject *pGameObject)
{
	m_pMovement = static_cast<CFireNetPlayerMovement *>(GetGameObject()->AcquireExtension("FireNetPlayerMovement"));
	m_pAnimations = static_cast<CFireNetPlayerAnimations *>(GetGameObject()->AcquireExtension("FireNetPlayerAnimations"));
	m_pInput = static_cast<CFireNetPlayerInput *>(GetGameObject()->AcquireExtension("FireNetPlayerInput"));

	gEnv->pGameFramework->GetIActorSystem()->AddActor(GetEntityId(), this);

	SetHealth(GetMaxHealth());
}

void CFireNetPlayer::ProcessEvent(SEntityEvent& event)
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

void CFireNetPlayer::SetHealth(float health)
{
	if (m_bAlive)
		return;

	m_bAlive = true;

	GetEntity()->Hide(false);
	GetEntity()->SetWorldTM(Matrix34::Create(Vec3(1, 1, 1), IDENTITY, GetEntity()->GetWorldPos()));

	SetPlayerModel();

	m_pInput->OnPlayerRespawn();
}

void CFireNetPlayer::OnPlayerAction(uint flags)
{
}

void CFireNetPlayer::OnPlayerJump()
{
}

void CFireNetPlayer::OnPlayerSprint()
{
}

void CFireNetPlayer::OnPlayerShoot()
{
}

void CFireNetPlayer::OnPlayerMoveMouseYaw(float value)
{
}

void CFireNetPlayer::OnPlayerMoveMousePitch(float value)
{
}

void CFireNetPlayer::SetPlayerModel()
{
	GetEntity()->LoadCharacter(eGeometry_ThirdPerson, GetCVars().m_pThirdPersonGeometry->GetString());
	m_pAnimations->OnPlayerModelChanged();
	m_pMovement->Physicalize();
}