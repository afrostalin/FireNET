// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#pragma once

#include "Entities/ISimpleExtension.h"

#include <ICryMannequin.h>

class CFireNetPlayer;


class CFireNetPlayerAnimations
	: public CGameObjectExtensionHelper<CFireNetPlayerAnimations, ISimpleExtension>
{
public:
	CFireNetPlayerAnimations();
	virtual ~CFireNetPlayerAnimations();

	// ISimpleExtension
	virtual void PostInit(IGameObject* pGameObject) override;
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	virtual void ProcessEvent(SEntityEvent& event) override;
	// ~ISimpleExtension

	void OnPlayerModelChanged();

protected:
	void ActivateMannequinContext(const char *contextName, ICharacterInstance &character, const SControllerDef &controllerDefinition, const IAnimationDatabase &animationDatabase);

protected:
	CFireNetPlayer *m_pPlayer;

	IActionController *m_pActionController;
	SAnimationContext *m_pAnimationContext;

	_smart_ptr<IAction> m_pIdleFragment;

	TagID m_rotateTagId;
	TagID m_walkTagId;
};
