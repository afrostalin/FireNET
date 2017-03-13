// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

typedef unsigned int uint;

//! FireNet player class
class CFireNetPlayer;

//! Client actions for network syncronizations
enum EFireNetClientActions : uint
{
	E_ACTION_MOVE_LEFT = 1 << 0,
	E_ACTION_MOVE_RIGHT = 1 << 1,
	E_ACTION_MOVE_FORWARD = 1 << 2,
	E_ACTION_MOVE_BACK = 1 << 3,
	E_ACTION_JUMP = 1 << 4,
	E_ACTION_SPRINT = 1 << 5,
	E_ACTION_SHOOT = 1 << 6,
	E_ACTION_MOUSE_ROTATE_YAW = 1 << 7,
	E_ACTION_MOUSE_ROTATE_PITCH = 1 << 8,
};

//! Structure for using in syncronization system
struct SFireNetSyncronizationClient
{
	uint            m_PlayerUID;
	uint            m_ChanelId;

	Vec3            m_PlayerSpawnPos;
	Quat            m_PlayerSpawnRot;

	string          m_PlayerModel;
	string          m_PlayerNickname;
};

//! Structure for send/read action
struct SFireNetClientAction
{
	EFireNetClientActions m_action;
	float                 m_value;
};