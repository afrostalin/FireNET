// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNet-Client>

class CGameStateSynchronization
{
public:
	CGameStateSynchronization();
	~CGameStateSynchronization();
public:
	void Reset();

	void SpawnNetPlayer(SNetPlayer &player);
	void RemoveNetPlayer(uint uid);

	void HideNetPlayer(uint uid);
	void UnhideNetPlayer(uint uid);

	void SyncNetPlayerAction(uint uid, SNetPlayerAction &action);
	void SyncNetPlayerPos(uint uid, Vec3 &pos);
	void SyncNetPlayerRot(uint uid, Quat &rot);
private:
	std::vector<SNetPlayer> m_NetPlayers;
};