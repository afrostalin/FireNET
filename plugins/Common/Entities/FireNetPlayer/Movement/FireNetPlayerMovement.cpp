// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#include "StdAfx.h"

#include "FireNetPlayerMovement.h"
#include "Entities/FireNetPlayer/FireNetPlayer.h"
#include "Entities/FireNetPlayer/Input/FireNetPlayerInput.h"

CFireNetPlayerMovement::CFireNetPlayerMovement()
	: m_bOnGround(false)
{
}

void CFireNetPlayerMovement::PostInit(IGameObject *pGameObject)
{
	m_pPlayer = static_cast<CFireNetPlayer *>(pGameObject->QueryExtension("FireNetPlayer"));

	// Make sure that this extension is updated regularly via the Update function below
	pGameObject->EnableUpdateSlot(this, 0);
}

void CFireNetPlayerMovement::Physicalize()
{
	// Physicalize the player as type Living.
	// This physical entity type is specifically implemented for players
	SEntityPhysicalizeParams physParams;
	physParams.type = PE_LIVING;
	physParams.mass = m_pPlayer->GetCVars().m_mass;

	pe_player_dimensions playerDimensions;

	// Prefer usage of a cylinder instead of capsule
	playerDimensions.bUseCapsule = 0;

	// Specify the size of our cylinder
	playerDimensions.sizeCollider = Vec3(0.35f, 0.35f, m_pPlayer->GetCVars().m_playerEyeHeight * 0.5f);

	// Keep pivot at the player's feet (defined in player geometry) 
	playerDimensions.heightPivot = 0.f;
	// Offset collider upwards
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

	// Obtain stats from the living entity implementation
	GetLatestPhysicsStats(*pPhysicalEntity);

	// Send latest input data to physics indicating desired movement direction
	UpdateMovementRequest(ctx.fFrameTime, *pPhysicalEntity);
}

void CFireNetPlayerMovement::GetLatestPhysicsStats(IPhysicalEntity &physicalEntity)
{
	pe_status_living livingStatus;
	if(physicalEntity.GetStatus(&livingStatus) != 0)
	{
		m_bOnGround = !livingStatus.bFlying;

		// Store the ground normal in case it is needed
		// Note that users have to check if we're on ground before using, is considered invalid in air.
		m_groundNormal = livingStatus.groundSlope;
	}
}

void CFireNetPlayerMovement::UpdateMovementRequest(float frameTime, IPhysicalEntity &physicalEntity)
{
	if (m_bOnGround)
	{
		pe_action_move moveAction;

		// Apply movement request directly to velocity
		moveAction.iJump = 2;

		const float moveSpeed = m_pPlayer->GetCVars().m_moveSpeed;
		moveAction.dir = m_pPlayer->GetInput()->GetLookOrientation() * GetLocalMoveDirection() *  (moveSpeed + m_SprintValue) * frameTime;

		// Dispatch the movement request
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