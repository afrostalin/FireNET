// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#pragma once

#include "Entities/ISimpleExtension.h"

class CFireNetPlayer;

////////////////////////////////////////////////////////
// Player extension to manage movement
////////////////////////////////////////////////////////
class CFireNetPlayerMovement : public CGameObjectExtensionHelper<CFireNetPlayerMovement, ISimpleExtension>
{
public:
	CFireNetPlayerMovement();
	virtual ~CFireNetPlayerMovement() {}

	//ISimpleExtension
	virtual void PostInit(IGameObject* pGameObject) override;

	virtual void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	//~ISimpleExtension

	void Physicalize();

	// Gets the requested movement direction based on input data
	Vec3 GetLocalMoveDirection() const;
	bool IsOnGround() const { return m_bOnGround; }
	Vec3 GetGroundNormal() const { return m_groundNormal; }

	void SetSprint(float value) { m_SprintValue = value; }
protected:
	// Get the stats from latest physics thread update
	void GetLatestPhysicsStats(IPhysicalEntity &physicalEntity);
	void UpdateMovementRequest(float frameTime, IPhysicalEntity &physicalEntity);

protected:
	CFireNetPlayer *m_pPlayer;

	bool m_bOnGround;
	Vec3 m_groundNormal;

	float m_SprintValue = 0.f;
};
