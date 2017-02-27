// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "DummyPlayerInput.h"
#include "DummyPlayer/DummyPlayer.h"
#include "DummyPlayer/Movement/DummyPlayerMovement.h"
#include <CryAnimation/ICryAnimation.h>

void CNetPlayerInput::PostInit(IGameObject *pGameObject)
{
	m_pPlayer = static_cast<CNetPlayer *>(pGameObject->QueryExtension("DummyPlayer"));
	m_moveSpeed = m_pPlayer->GetCVars().m_moveSpeed;

	// Make sure that this extension is updated regularly via the Update function below
	GetGameObject()->EnableUpdateSlot(this, 0);
}

void CNetPlayerInput::Update(SEntityUpdateContext &ctx, int updateSlot)
{
	Ang3 ypr = CCamera::CreateAnglesYPR(Matrix33(m_lookOrientation));
	
	ypr.x += m_mouseDeltaRotation.x * m_pPlayer->GetCVars().m_rotationSpeedYaw * ctx.fFrameTime;
	ypr.y = CLAMP(ypr.y + m_mouseDeltaRotation.y * m_pPlayer->GetCVars().m_rotationSpeedPitch * ctx.fFrameTime, m_pPlayer->GetCVars().m_rotationLimitsMinPitch, m_pPlayer->GetCVars().m_rotationLimitsMaxPitch);
	ypr.z = 0;

	m_lookOrientation = Quat(CCamera::CreateOrientationYPR(ypr));

	// Reset every frame
	m_mouseDeltaRotation = ZERO;
	m_inputFlags = 0;

	if (auto* pMovement = m_pPlayer->GetMovement())
	{
		pMovement->SetSprint(0.f);
	}
}

void CNetPlayerInput::OnPlayerRespawn()
{
	m_inputFlags = 0;
	m_mouseDeltaRotation = ZERO;
	m_lookOrientation = IDENTITY;
}

void CNetPlayerInput::OnActionJump()
{
	auto *pMovement = m_pPlayer->GetMovement();
	auto *pEntity = m_pPlayer->GetEntity();
	auto *pPhysics = GetEntity()->GetPhysics();

	if (pPhysics && pEntity &&  pMovement->IsOnGround())
	{
		pe_action_impulse impulseAction;
		Vec3 impulsePos = pEntity->GetWorldPos();
		impulsePos.x = 0;
		impulsePos.y = 0;
		impulsePos.z = impulsePos.z + 400 * m_pPlayer->GetCVars().m_jumpHeightMultiplier;
		impulseAction.impulse = impulsePos;

		pPhysics->Action(&impulseAction);
	}
}

void CNetPlayerInput::OnActionSprint()
{
	if (auto* pMovement = m_pPlayer->GetMovement())
	{
		pMovement->SetSprint(30.f);
	}
}

void CNetPlayerInput::OnActionMouseRotateYaw(float value)
{
	m_mouseDeltaRotation.x -= value;
}

void CNetPlayerInput::OnActionMouseRotatePitch(float value)
{
	m_mouseDeltaRotation.y -= value;
}

void CNetPlayerInput::OnActionShoot(EntityId entityId, const ActionId& actionId, int activationMode, float value)
{
}