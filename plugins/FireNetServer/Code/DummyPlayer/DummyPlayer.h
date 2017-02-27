// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include "Actors/ISimpleActor.h"
#include <CryMath/Cry_Camera.h>

class CNetPlayerInput;
class CNetPlayerMovement;
class CNetPlayerAnimations;
class CNetPlayer;

class CNetPlayer : public CGameObjectExtensionHelper<CNetPlayer, ISimpleActor>
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

		float m_rotationSpeedYaw;
		float m_rotationSpeedPitch;

		float m_rotationLimitsMinPitch;
		float m_rotationLimitsMaxPitch;

		float m_playerEyeHeight;


		ICVar *m_pThirdPersonGeometry;

		ICVar *m_pThirdPersonMannequinContext;
		ICVar *m_pThirdPersonAnimationDatabase;
		ICVar *m_pThirdPersonControllerDefinition;
	};

public:
	CNetPlayer();
	virtual ~CNetPlayer();

	// ISimpleActor
	virtual bool          Init(IGameObject* pGameObject) override;
	virtual void          PostInit(IGameObject* pGameObject) override;
	virtual void          ProcessEvent(SEntityEvent& event) override;
	virtual void          SetHealth(float health) override;
	virtual float         GetHealth() const override { return m_bAlive ? GetMaxHealth() : 0.f; }
	// ~ISimpleActor

	CNetPlayerInput*      GetInput() const { return m_pInput; }
	CNetPlayerMovement*   GetMovement() const { return m_pMovement; }
	const SExternalCVars& GetCVars() const;
protected:
	void                  SetPlayerModel();
protected:
	CNetPlayerInput*      m_pInput;
	CNetPlayerMovement*   m_pMovement;
	CNetPlayerAnimations* m_pAnimations;

	bool                  m_bAlive;
};
