// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include "ISimpleActor.h"
#include <CryMath/Cry_Camera.h>

#include <FireNet>

class CFireNetPlayerInput;
class CFireNetPlayerMovement;
class CFireNetPlayerAnimations;
class CFireNetPlayerView;

class CFireNetPlayer 
	: public CGameObjectExtensionHelper<CFireNetPlayer, ISimpleActor>
	, public IFireNetPlayerEntity
{
public:
	enum EGeometrySlots
	{
		eGeometry_ThirdPerson = 0,
	};

	struct SExternalCVars
	{
		float m_mass;

		float m_moveSpeed;

		float m_jumpHeightMultiplier;

		float m_viewOffsetY;
		float m_viewOffsetZ;

		float m_cameraOffsetY;
		float m_cameraOffsetZ;
		

		float m_rotationSpeedYaw;
		float m_rotationSpeedPitch;

		float m_rotationLimitsMinPitch;
		float m_rotationLimitsMaxPitch;

		float m_playerEyeHeight;

		ICVar *m_pCameraJointName;

		ICVar *m_pFirstPersonGeometry;
		ICVar *m_pFirstPersonMannequinContext;
		ICVar *m_pFirstPersonAnimationDatabase;
		ICVar *m_pFirstPersonControllerDefinition;
		
		ICVar *m_pThirdPersonGeometry;
		ICVar *m_pThirdPersonMannequinContext;
		ICVar *m_pThirdPersonAnimationDatabase;
		ICVar *m_pThirdPersonControllerDefinition;
	};

public:
	CFireNetPlayer();
	virtual ~CFireNetPlayer();

	// ISimpleActor
	virtual bool              Init(IGameObject* pGameObject) override;
	virtual void              PostInit(IGameObject* pGameObject) override;
	virtual void              ProcessEvent(SEntityEvent& event) override;
	virtual void              SetHealth(float health) override;
	virtual float             GetHealth() const override { return m_bAlive ? GetMaxHealth() : 0.f; }
	// ~ISimpleActor

	// IFireNetPlayerEntity
	virtual void OnPlayerAction(uint flags) override;
	virtual void OnPlayerJump() override;
	virtual void OnPlayerSprint() override;
	virtual void OnPlayerShoot() override;
	virtual void OnPlayerMoveMouseYaw(float value) override;
	virtual void OnPlayerMoveMousePitch(float value) override;
	// ~IFireNetPlayerEntity

	CFireNetPlayerInput*      GetInput() const { return m_pInput; }
	CFireNetPlayerMovement*   GetMovement() const { return m_pMovement; }
	const SExternalCVars&     GetCVars() const;
protected:
	void                      SetPlayerModel();
protected:
	CFireNetPlayerInput*      m_pInput;
	CFireNetPlayerMovement*   m_pMovement;
	CFireNetPlayerAnimations* m_pAnimations;
	CFireNetPlayerView*       m_pView;

	bool                      m_bAlive;
};
