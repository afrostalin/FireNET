// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#pragma once

#include "ISimpleExtension.h"

struct ISimpleWeapon : public ISimpleExtension
{
	virtual ~ISimpleWeapon() {}

	// Call to request that the weapon is fired
	virtual void RequestFire(const Vec3 &initialBulletPosition, const Quat &initialBulletRotation) = 0;
};