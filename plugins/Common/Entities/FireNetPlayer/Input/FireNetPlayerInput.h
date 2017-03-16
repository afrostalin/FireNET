// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#pragma once

#include "Entities/ISimpleExtension.h"

class CFireNetPlayer;

class CFireNetPlayerInput : public CGameObjectExtensionHelper<CFireNetPlayerInput, ISimpleExtension> , public IActionListener
{
	enum EInputFlagType
	{
		eInputFlagType_Hold = 0,
		eInputFlagType_Toggle
	};

public:
	typedef uint32 TInputFlags;

	enum EInputFlags
		: TInputFlags
	{
		eInputFlag_MoveLeft = 1 << 0,
		eInputFlag_MoveRight = 1 << 1,
		eInputFlag_MoveForward = 1 << 2,
		eInputFlag_MoveBack = 1 << 3,
		eInputFlag_Jump = 1 << 4,
		eInputFlag_Sprint = 1 << 5,
		eInputFlag_Shoot = 1 << 6,
		eInputFlag_MouseRotateYaw = 1 << 7,
		eInputFlag_MouseRotatePitch = 1 << 8,
	};

public:
	CFireNetPlayerInput() : bPhysDebug(false),
		m_moveSpeed(0.0f) ,
		bGamePaused(false)
	{}
	virtual ~CFireNetPlayerInput() {}

	// ISimpleExtension
	virtual void                 PostInit(IGameObject* pGameObject) override;
	virtual void                 Update(SEntityUpdateContext &ctx, int updateSlot) override;
	// ~ISimpleExtension

	// IActionListener
	virtual void                 OnAction(const ActionId &action, int activationMode, float value) override;
	// ~IActionListener

	void                         OnPlayerRespawn();

	const TInputFlags            GetInputFlags() const { return m_inputFlags; }
	float                        GetInputValues() { return m_inputValues; }

	const Vec2                   GetMouseDeltaRotation() const { return m_mouseDeltaRotation; }
	const Quat&                  GetLookOrientation() const { return m_lookOrientation; }
protected:
	void                         InitializeActionHandler();
	void                         HandleInputFlagChange(EInputFlags flags, int activationMode, EInputFlagType type = eInputFlagType_Hold);
protected:
	bool                         OnActionMoveLeft(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionMoveRight(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionMoveForward(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionMoveBack(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionJump(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionSprint(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnToggleThirdPersonMode(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionGamePaused(EntityId entityId, const ActionId& actionId, int activationMode, float value);

	// Debug functions
	bool                         OnPhysicDebug(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	// --------------

	bool                         OnActionMouseRotateYaw(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionMouseRotatePitch(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	
	bool                         OnActionShoot(EntityId entityId, const ActionId& actionId, int activationMode, float value);
protected:
	Quat                         m_lookOrientation;
	TActionHandler<CFireNetPlayerInput> m_actionHandler;

	CFireNetPlayer*              m_pPlayer;

	TInputFlags                  m_inputFlags;
	float                        m_inputValues;

	Vec2                         m_mouseDeltaRotation;
	float                        m_moveSpeed;

	bool                         bGamePaused;
	bool                         bPhysDebug;
};
