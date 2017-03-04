// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#pragma once

#include "Entities/ISimpleWeapon.h"

class CFireNetRifle 
	: public CGameObjectExtensionHelper<CFireNetRifle, ISimpleWeapon>
{
public:
	virtual ~CFireNetRifle() {}

	// IWeapon
	virtual void RequestFire(const Vec3 &initialBulletPosition, const Quat &initialBulletRotation) override;
	// ~IWeapon
};