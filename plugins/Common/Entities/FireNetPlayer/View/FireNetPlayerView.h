// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#pragma once

#include "Entities/ISimpleExtension.h"

class CFireNetPlayer;

////////////////////////////////////////////////////////
// Player extension to manage the local client's view / camera
////////////////////////////////////////////////////////
class CFireNetPlayerView
	: public CGameObjectExtensionHelper<CFireNetPlayerView, ISimpleExtension>
	, public IGameObjectView
{
public:
	CFireNetPlayerView();
	virtual ~CFireNetPlayerView();

	// ISimpleExtension
	virtual void PostInit(IGameObject* pGameObject) override;
	// ~ISimpleExtension

	// IGameObjectView
	virtual void UpdateView(SViewParams &viewParams) override;
	virtual void PostUpdateView(SViewParams &viewParams) override {}
	// ~IGameObjectView

	void OnPlayerModelChanged();

protected:
	CFireNetPlayer *m_pPlayer;

	// Identifier of the camera joint inside the Character rig
	// We use this to move the camera position in UpdateView
	uint32 m_cameraJointId;
};
