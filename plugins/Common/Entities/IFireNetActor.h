// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

class CFireNetPlayer;
class CFireNetPlayerInput;
class CFireNetPlayerMovement;
class CFireNetPlayerView;
class CFireNetPlayerAnimations;

struct SFireNetClientInput;
struct ISimpleWeapon;

//! Base FireNet actor structure
struct IFireNetActor
{
	//! Get input pointer
	virtual CFireNetPlayerInput* GetInput() = 0;

	//! Get movement pointer
	virtual CFireNetPlayerMovement* GetMovement() = 0;

	//! Get view pointer
	virtual CFireNetPlayerView* GetView() = 0;

	//! Get animations pointer
	virtual CFireNetPlayerAnimations* GetAnimations() = 0;

	//! Get current weapon pointer
	virtual ISimpleWeapon* GetCurrentWeapon() = 0;

	//! Is local player ?
	virtual bool IsLocalPlayer() = 0;

	//! Set player to local
	virtual void SetLocalPlayer(bool local) = 0;

	//! Sync net input with player
	virtual void SyncNetInput(const SFireNetClientInput &input) = 0;
};