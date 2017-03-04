// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#pragma once

#include "Entities/ISimpleExtension.h"

class CFireNetBullet
	: public ISimpleExtension
{
public:
	virtual ~CFireNetBullet() {}

	// ISimpleExtension	
	virtual void PostInit(IGameObject *pGameObject) override;
	virtual void HandleEvent(const SGameObjectEvent &event) override;
	// ~ISimpleExtension

protected:
	void Physicalize();
};