// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#include "StdAfx.h"
#include "FireNetPlayerView.h"

#include "Entities/FireNetPlayer/FireNetPlayer.h"
#include "Entities/FireNetPlayer/Input/FireNetPlayerInput.h"
#include "Entities/FireNetPlayer/Movement/FireNetPlayerMovement.h"

#include <IViewSystem.h>
#include <CryAnimation/ICryAnimation.h>

CFireNetPlayerView::CFireNetPlayerView()
	: m_cameraJointId(0)
{
}

CFireNetPlayerView::~CFireNetPlayerView()
{
	GetGameObject()->ReleaseView(this);
}

void CFireNetPlayerView::PostInit(IGameObject *pGameObject)
{
	m_pPlayer = static_cast<CFireNetPlayer *>(pGameObject->QueryExtension("FireNetPlayer"));

	// Register for UpdateView callbacks
	GetGameObject()->CaptureView(this);
}

void CFireNetPlayerView::OnPlayerModelChanged()
{
	if (auto *pCharacter = GetEntity()->GetCharacter(CFireNetPlayer::eGeometry_Default))
	{
		// Cache the camera joint id so that we don't need to look it up every frame in UpdateView
		const char *cameraJointName = m_pPlayer->GetCVars().m_pCameraJointName->GetString();

		m_cameraJointId = pCharacter->GetIDefaultSkeleton().GetJointIDByName(cameraJointName);
	}
}

void CFireNetPlayerView::UpdateView(SViewParams &viewParams)
{
	IEntity &entity = *GetEntity();

	if (!m_pPlayer->IsThirdPerson())
	{
		// Start with changing view rotation to the requested mouse look orientation
		viewParams.rotation = Quat(m_pPlayer->GetInput()->GetLookOrientation());

		if (auto *pCharacter = entity.GetCharacter(CFireNetPlayer::eGeometry_Default))
		{
			const QuatT &cameraOrientation = pCharacter->GetISkeletonPose()->GetAbsJointByID(m_cameraJointId);

			Vec3 localCameraPosition = cameraOrientation.t;
			localCameraPosition.y += m_pPlayer->GetCVars().m_cameraOffsetY;
			localCameraPosition.z += m_pPlayer->GetCVars().m_cameraOffsetZ;

			viewParams.position = entity.GetWorldTM().TransformPoint(localCameraPosition);
		}
	}
	else
	{
		// Start with changing view rotation to the requested mouse look orientation
		viewParams.rotation = Quat(m_pPlayer->GetInput()->GetLookOrientation());

		// Start with matching view to the entity position
		viewParams.position = entity.GetWorldPos();

		// Offset the player along the forward axis (normally back)
		viewParams.position += entity.GetWorldRotation().GetColumn1() * m_pPlayer->GetCVars().m_viewOffsetY;

		// Offset the view upwards
		viewParams.position.z += m_pPlayer->GetCVars().m_viewOffsetZ;
	}
}