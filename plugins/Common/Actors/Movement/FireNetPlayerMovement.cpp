// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "FireNetPlayerMovement.h"

#include "Actors/FireNetPlayer.h"
#include "Actors/Input/FireNetPlayerInput.h"

// TODO init all members
CFireNetPlayerMovement::CFireNetPlayerMovement()
	: m_bOnGround(false)
{
}

void CFireNetPlayerMovement::PostInit(IGameObject *pGameObject)
{
	m_pPlayer = static_cast<CFireNetPlayer *>(pGameObject->QueryExtension("FireNetPlayer"));
	pGameObject->EnableUpdateSlot(this, 0);
}

void CFireNetPlayerMovement::Physicalize()
{
	SEntityPhysicalizeParams physParams;
	physParams.type = PE_LIVING;
	physParams.mass = m_pPlayer->GetCVars().m_mass;

	pe_player_dimensions playerDimensions;
	playerDimensions.bUseCapsule = 0;
	playerDimensions.sizeCollider = Vec3(0.35f, 0.35f, m_pPlayer->GetCVars().m_playerEyeHeight * 0.5f);
	playerDimensions.heightPivot = 0.f;
	playerDimensions.heightCollider = 1.f;
	playerDimensions.groundContactEps = 0.004f;

	physParams.pPlayerDimensions = &playerDimensions;
	
	pe_player_dynamics playerDynamics;
	playerDynamics.kAirControl = 0.f;
	playerDynamics.mass = physParams.mass;

	physParams.pPlayerDynamics = &playerDynamics;

	GetEntity()->Physicalize(physParams);
}

void CFireNetPlayerMovement::Update(SEntityUpdateContext &ctx, int updateSlot)
{
	IEntity &entity = *GetEntity();
	IPhysicalEntity *pPhysicalEntity = entity.GetPhysics();
	if(pPhysicalEntity == nullptr)
		return;

	GetLatestPhysicsStats(*pPhysicalEntity);
	UpdateMovementRequest(ctx.fFrameTime, *pPhysicalEntity);
}

void CFireNetPlayerMovement::GetLatestPhysicsStats(IPhysicalEntity &physicalEntity)
{
	pe_status_living livingStatus;
	if(physicalEntity.GetStatus(&livingStatus) != 0)
	{
		m_bOnGround = !livingStatus.bFlying;
		m_groundNormal = livingStatus.groundSlope;
	}
}

void CFireNetPlayerMovement::UpdateMovementRequest(float frameTime, IPhysicalEntity &physicalEntity)
{
	if (m_bOnGround)
	{
		pe_action_move moveAction;
		moveAction.iJump = 2;

		const float moveSpeed = m_pPlayer->GetCVars().m_moveSpeed;
		moveAction.dir = m_pPlayer->GetInput()->GetLookOrientation() * GetLocalMoveDirection() * (moveSpeed + m_SprintValue) * frameTime;

		physicalEntity.Action(&moveAction);
	}
}

Vec3 CFireNetPlayerMovement::GetLocalMoveDirection() const
{
	Vec3 moveDirection = ZERO;

	uint32 inputFlags = m_pPlayer->GetInput()->GetInputFlags();

	if (inputFlags & CFireNetPlayerInput::eInputFlag_MoveLeft)
	{
		moveDirection.x -= 1;
	}
	if (inputFlags & CFireNetPlayerInput::eInputFlag_MoveRight)
	{
		moveDirection.x += 1;
	}
	if (inputFlags & CFireNetPlayerInput::eInputFlag_MoveForward)
	{
		moveDirection.y += 1;
	}
	if (inputFlags & CFireNetPlayerInput::eInputFlag_MoveBack)
	{
		moveDirection.y -= 1;
	}

	return moveDirection;
}