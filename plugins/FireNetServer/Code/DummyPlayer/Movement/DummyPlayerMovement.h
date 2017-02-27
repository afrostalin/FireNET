// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include "Entities/ISimpleExtension.h"

class CNetPlayer;

////////////////////////////////////////////////////////
// Player extension to manage movement
////////////////////////////////////////////////////////
class CNetPlayerMovement : public CGameObjectExtensionHelper<CNetPlayerMovement, ISimpleExtension>
{
public:
	CNetPlayerMovement();
	virtual ~CNetPlayerMovement() {}

	//ISimpleExtension
	virtual void PostInit(IGameObject* pGameObject) override;
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	//~ISimpleExtension

	void Physicalize();

	Vec3 GetLocalMoveDirection() const;
	bool IsOnGround() const { return m_bOnGround; }
	Vec3 GetGroundNormal() const { return m_groundNormal; }

	void SetSprint(float value) { m_SprintValue = value; }
protected:
	void GetLatestPhysicsStats(IPhysicalEntity &physicalEntity);
	void UpdateMovementRequest(float frameTime, IPhysicalEntity &physicalEntity);

protected:
	CNetPlayer *m_pPlayer;

	bool m_bOnGround;
	Vec3 m_groundNormal;

	float m_SprintValue = 0.f;
};
