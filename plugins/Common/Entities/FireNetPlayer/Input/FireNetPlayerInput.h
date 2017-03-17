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
	CFireNetPlayerInput();
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
public:
	void                         StartActionCapture();
protected:
	void                         InitializeActionHandler();
	void                         HandleInputFlagChange(EFireNetClientInputFlags flags, int activationMode, EInputFlagType type = eInputFlagType_Hold);
public:
	void                         SyncInput(const SFireNetClientInput& input);
private:
	void                         DoJump();
	void                         DoSprint();
	void                         DoMouseYaw(float value);
	void                         DoMousePitch(float value);
	void                         DoShoot();
protected:
	bool                         OnActionMoveLeft(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionMoveRight(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionMoveForward(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionMoveBack(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionJump(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionSprint(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionToggleThirdPersonMode(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionGamePaused(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionPhysicDebug(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionMouseRotateYaw(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionMouseRotatePitch(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool                         OnActionShoot(EntityId entityId, const ActionId& actionId, int activationMode, float value);
protected:
	Quat                         m_lookOrientation;
	TActionHandler<CFireNetPlayerInput> m_actionHandler;

	CFireNetPlayer*              m_pPlayer;

	uint32                       m_inputFlags;
	float                        m_inputValues;

	Vec2                         m_mouseDeltaRotation;
	float                        m_moveSpeed;

	bool                         bGamePaused;
	bool                         bPhysDebug;
};
