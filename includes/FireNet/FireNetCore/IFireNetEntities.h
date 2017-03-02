// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

typedef unsigned int uint;

// Basic interface for FireNet player class
struct IFireNetEntityPlayer
{
	//! On player action
	//! Need to syncronizate action flags
	//! Execute this function every time, when client/server received action request
	virtual void OnPlayerAction(uint flags) = 0;

	//! On player jump
	virtual void OnPlayerJump() = 0;

	//! On player sprint
	virtual void OnPlayerSprint() = 0;

	//! On player shoot
	virtual void OnPlayerShoot() = 0;

	//! On player move mouse yaw
	virtual void OnPlayerMoveMouseYaw(float value) = 0;

	//! On player move mouse pitch
	virtual void OnPlayerMoveMousePitch(float value) = 0;
};