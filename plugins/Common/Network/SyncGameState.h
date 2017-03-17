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

	bool SpawnNetPlayer(SFireNetClientPlayer &player);
	void RemoveNetPlayer(uint32 uid);

	void HideNetPlayer(uint32 uid);
	void UnhideNetPlayer(uint32 uid);

	void SyncNetPlayerInput(uint32 uid, const SFireNetClientInput &input);
	void SyncNetPlayerPos(uint32 uid, Vec3 &pos);
	void SyncNetPlayerRot(uint32 uid, Quat &rot);
public:
	std::vector<SFireNetClientPlayer>* GetAllPlayers() {return &m_NetPlayers;}
private:
	std::vector<SFireNetClientPlayer> m_NetPlayers;
};