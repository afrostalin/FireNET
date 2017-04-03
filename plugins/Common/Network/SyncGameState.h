// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNet>

#include <IActorSystem.h>

typedef std::map<uint32, SFireNetClientPlayer> NetClientPlayersMap;
typedef NetClientPlayersMap::value_type NetClientPlayer;

class CGameStateSynchronization
{
public:
	CGameStateSynchronization();
	~CGameStateSynchronization();
public:
	void                  Reset();

	bool                  SpawnNetPlayer(uint32 channel, SFireNetClientPlayer &player);
	void                  RemoveNetPlayer(uint32 channel);

	void                  HideNetPlayer(uint32 channel, bool hide);

	void                  SyncNetPlayerInput(uint32 channel, const SFireNetClientInput &input);
	void                  SyncNetPlayerPosRot(uint32 channel, const Vec3 &pos, const Quat &rot);
public:
	SFireNetClientPlayer* GetNetPlayer(uint32 channel);
	NetClientPlayersMap*  GetAllNetPlayers() { return &m_NetPlayers; }
private:
	void                  RemoveNetPlayerForMap(uint32 channel);
private:
	NetClientPlayersMap   m_NetPlayers;
	IActorSystem*         pActorSystem;
};