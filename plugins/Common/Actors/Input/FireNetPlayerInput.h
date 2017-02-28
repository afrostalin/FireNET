// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include "Entities/ISimpleExtension.h"

class CFireNetPlayer;

class CFireNetPlayerInput : public CGameObjectExtensionHelper<CFireNetPlayerInput, ISimpleExtension>
{
	enum EInputFlagType
	{
		eInputFlagType_Hold = 0,
		eInputFlagType_Toggle
	};

public:
	typedef uint16 TInputFlags;

	enum EInputFlags
		: TInputFlags
	{
		eInputFlag_MoveLeft = 1 << 0,
		eInputFlag_MoveRight = 1 << 1,
		eInputFlag_MoveForward = 1 << 2,
		eInputFlag_MoveBack = 1 << 3,
	};

public:
	// TODO - move it 
	CFireNetPlayerInput() : m_moveSpeed(0.0f)
		, m_pPlayer(nullptr)
	{}
	virtual ~CFireNetPlayerInput() {}

	// ISimpleExtension
	virtual void       PostInit(IGameObject* pGameObject) override;
	virtual void       Update(SEntityUpdateContext &ctx, int updateSlot) override;
	// ~ISimpleExtension

	void               OnPlayerRespawn();

	const TInputFlags  GetInputFlags() const { return m_inputFlags; }
	void               SetInputFlags(uint flags) { m_inputFlags = flags; }

	const Vec2         GetMouseDeltaRotation() const { return m_mouseDeltaRotation; }
	const Quat&        GetLookOrientation() const { return m_lookOrientation; }
public:
	void               OnActionJump();
	void               OnActionSprint();

	void               OnActionMouseRotateYaw(float value);
	void               OnActionMouseRotatePitch(float value);
	
	void               OnActionShoot(EntityId entityId, const ActionId& actionId, int activationMode, float value);
protected:
	CFireNetPlayer*    m_pPlayer;
	TInputFlags        m_inputFlags;
	Vec2               m_mouseDeltaRotation;
	float              m_moveSpeed;

	Quat               m_lookOrientation;
};
