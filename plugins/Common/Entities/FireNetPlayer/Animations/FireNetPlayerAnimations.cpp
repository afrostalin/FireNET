// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#include "StdAfx.h"
#include "FireNetPlayerAnimations.h"

#include "Entities/FireNetPlayer/FireNetPlayer.h"
#include "Entities/FireNetPlayer/Input/FireNetPlayerInput.h"
#include "Entities/FireNetPlayer/Movement/FireNetPlayerMovement.h"

#include <CryAnimation/ICryAnimation.h>
#include <ICryMannequin.h>

#ifndef USE_DEFAULT_DEDICATED_SERVER
CFireNetPlayerAnimations::CFireNetPlayerAnimations()
	: m_pActionController(nullptr)
	, m_pAnimationContext(nullptr)
{
}

CFireNetPlayerAnimations::~CFireNetPlayerAnimations()
{
	SAFE_RELEASE(m_pActionController);
	SAFE_DELETE(m_pAnimationContext);
}

void CFireNetPlayerAnimations::PostInit(IGameObject *pGameObject)
{
	m_pPlayer = static_cast<CFireNetPlayer *>(pGameObject->QueryExtension("FireNetPlayer"));
	pGameObject->EnableUpdateSlot(this, 0);
}

void CFireNetPlayerAnimations::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	if (auto *pPhysEnt = m_pPlayer->GetEntity()->GetPhysics())
	{
		Ang3 ypr = CCamera::CreateAnglesYPR(Matrix33(m_pPlayer->GetInput()->GetLookOrientation()));

		Quat correctedOrientation = Quat(CCamera::CreateOrientationYPR(ypr));

		auto *pCharacter = m_pPlayer->GetEntity()->GetCharacter(CFireNetPlayer::eGeometry_Default);

		pe_status_dynamics playerDynamics;
		if (pPhysEnt->GetStatus(&playerDynamics) != 0 && pCharacter != nullptr)
		{
			float turnAngle = Ang3::CreateRadZ(GetEntity()->GetForwardDir(), correctedOrientation.GetColumn1()) / ctx.fFrameTime;
			float travelAngle = Ang3::CreateRadZ(GetEntity()->GetForwardDir(), playerDynamics.v.GetNormalized());
			float travelSpeed = playerDynamics.v.GetLength2D();

			pCharacter->GetISkeletonAnim()->SetDesiredMotionParam(eMotionParamID_TravelSpeed, travelSpeed, 0.f);
			pCharacter->GetISkeletonAnim()->SetDesiredMotionParam(eMotionParamID_TurnAngle, turnAngle, 0.f);
			pCharacter->GetISkeletonAnim()->SetDesiredMotionParam(eMotionParamID_TravelAngle, travelAngle, 0.f);

			if (m_pPlayer->GetMovement()->IsOnGround())
			{
				Vec3 groundNormal = m_pPlayer->GetMovement()->GetGroundNormal() * correctedOrientation;
				groundNormal.x = 0.0f;
				float cosine = Vec3Constants<float>::fVec3_OneZ | groundNormal;
				Vec3 sine = Vec3Constants<float>::fVec3_OneZ % groundNormal;

				float travelSlope = atan2f(sgn(sine.x) * sine.GetLength(), cosine);

				pCharacter->GetISkeletonAnim()->SetDesiredMotionParam(eMotionParamID_TravelSlope, travelSlope, 0.f);
			}

			m_pAnimationContext->state.Set(m_rotateTagId, abs(turnAngle) > 0);
			m_pAnimationContext->state.Set(m_walkTagId, travelSpeed > 0.2f && m_pPlayer->GetMovement()->IsOnGround());
		}

		GetEntity()->SetPosRotScale(GetEntity()->GetWorldPos(), correctedOrientation, Vec3(1, 1, 1));
	}

	if (m_pActionController != nullptr)
	{
		m_pActionController->Update(ctx.fFrameTime);
	}
}

void CFireNetPlayerAnimations::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
		case ENTITY_EVENT_ANIM_EVENT:
		{
			const AnimEventInstance *pAnimEvent = reinterpret_cast<const AnimEventInstance *>(event.nParam[0]);
			ICharacterInstance *pCharacter = reinterpret_cast<ICharacterInstance *>(event.nParam[1]);

			m_pActionController->OnAnimationEvent(pCharacter, *pAnimEvent);
		}
		break;
	}
}

void CFireNetPlayerAnimations::OnPlayerModelChanged()
{
	// Release previous controller and context, if we are respawning
	SAFE_RELEASE(m_pActionController);
	SAFE_DELETE(m_pAnimationContext);

	// Now start loading the Mannequin data
	IMannequin &mannequinInterface = gEnv->pGameFramework->GetMannequinInterface();
	IAnimationDatabaseManager &animationDatabaseManager = mannequinInterface.GetAnimationDatabaseManager();

	const char *mannequinControllerDefinition = m_pPlayer->GetCVars().m_pControllerDefinition->GetString();

	// Load the Mannequin controller definition.
	// This is owned by the animation database manager
	const SControllerDef *pControllerDefinition = animationDatabaseManager.LoadControllerDef(mannequinControllerDefinition);
	if (pControllerDefinition == nullptr)
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to load controller definition.");
		return;
	}

	const char *mannequinContextName = m_pPlayer->GetCVars().m_pMannequinContext->GetString();
	const char *animationDatabasePath = m_pPlayer->GetCVars().m_pAnimationDatabase->GetString();

	// Load the animation database
	auto *pAnimationDatabase = animationDatabaseManager.Load(animationDatabasePath);
	if (pAnimationDatabase == nullptr)
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to load animation database %s!", animationDatabasePath);
		return;
	}

	// Create a new animation context for the controller definition we loaded above
	m_pAnimationContext = new SAnimationContext(*pControllerDefinition);

	// Now create the controller that will be handling animation playback
	m_pActionController = mannequinInterface.CreateActionController(GetEntity(), *m_pAnimationContext);
	CRY_ASSERT(m_pActionController != nullptr);

	ICharacterInstance *pCharacterInstance = GetEntity()->GetCharacter(CFireNetPlayer::eGeometry_Default);
	CRY_ASSERT(pCharacterInstance != nullptr);

	// Activate the Main context we'll be playing our animations in
	ActivateMannequinContext(mannequinContextName, *pCharacterInstance, *pControllerDefinition, *pAnimationDatabase);

	// Create this idle fragment
	// This implementation handles switching to various sub-fragments by itself, based on input and physics data
	int priority = 0;
	auto idleFragmentId = pControllerDefinition->m_fragmentIDs.Find("Idle");

	m_pIdleFragment = new TAction<SAnimationContext>(priority, idleFragmentId, TAG_STATE_EMPTY, 0);

	// Queue the idle fragment to start playing immediately on next update
	m_pActionController->Queue(*m_pIdleFragment.get());

	// Acquire tag identifiers to avoid doing so each update
	m_rotateTagId = m_pAnimationContext->state.GetDef().Find("Rotate");
	m_walkTagId = m_pAnimationContext->state.GetDef().Find("Walk");

	// Disable movement coming from the animation (root joint offset), we control this entirely via physics
	pCharacterInstance->GetISkeletonAnim()->SetAnimationDrivenMotion(1);
}

void CFireNetPlayerAnimations::ActivateMannequinContext(const char *contextName, ICharacterInstance &character, const SControllerDef &controllerDefinition, const IAnimationDatabase &animationDatabase)
{
	IEntity &entity = *GetEntity();

	const TagID scopeContextId = controllerDefinition.m_scopeContexts.Find(contextName);
	if (scopeContextId == TAG_ID_INVALID)
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to find %s scope context id in controller definition.", contextName);
		return;
	}

	// Setting Scope contexts can happen at any time, and what entity or character instance we have bound to a particular scope context
	// can change during the lifetime of an action controller.
	m_pActionController->SetScopeContext(scopeContextId, entity, &character, &animationDatabase);
}
#endif