// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

class CFireNetPlayer;

struct IActor;

typedef uint32 TInputFlags;

//! Input flags
enum EFireNetClientInputFlags : TInputFlags
{
	E_FIRENET_INPUT_MOVE_LEFT = 1 << 0,
	E_FIRENET_INPUT_MOVE_RIGHT = 1 << 1,
	E_FIRENET_INPUT_MOVE_FORWARD = 1 << 2,
	E_FIRENET_INPUT_MOVE_BACK = 1 << 3,
	E_FIRENET_INPUT_JUMP = 1 << 4,
	E_FIRENET_INPUT_SPRINT = 1 << 5,
	E_FIRENET_INPUT_SHOOT = 1 << 6,
	E_FIRENET_INPUT_MOUSE_ROTATE_YAW = 1 << 7,
	E_FIRENET_INPUT_MOUSE_ROTATE_PITCH = 1 << 8,
};

//! Client player structure
struct SFireNetClientPlayer
{
	uint32          m_PlayerUID;
	uint32          m_ChanelId;

	Vec3            m_PlayerSpawnPos;
	Quat            m_PlayerSpawnRot;

	string          m_PlayerModel;
	string          m_PlayerNickname;

	IActor*         pActor;
	CFireNetPlayer* pPlayer;
};

//! Structure for send/read input
struct SFireNetClientInput
{
	EFireNetClientInputFlags m_flags;
	Quat                     m_LookOrientation;
};