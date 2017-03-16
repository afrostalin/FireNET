// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNet>

class CGameStateSynchronization
{
public:
	CGameStateSynchronization();
	~CGameStateSynchronization();
public:
	void Reset();

	bool SpawnNetPlayer(const SFireNetSyncronizationClient& player);
	void RemoveNetPlayer(uint uid);

	void HideNetPlayer(uint uid);
	void UnhideNetPlayer(uint uid);

	void SyncNetPlayerAction(uint uid, SFireNetClientAction &action);
	void SyncNetPlayerPos(uint uid, Vec3 &pos);
	void SyncNetPlayerRot(uint uid, Quat &rot);
private:
	std::vector<SFireNetSyncronizationClient> m_NetPlayers;
};