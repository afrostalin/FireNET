// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include "IFireNetClientBase.h"

//! Base interface for using in FireNet-Client plugin
struct IFireNetClientCore
{
	virtual ~IFireNetClientCore() {}

	//! Connect to game server
	virtual void ConnectToGameServer() = 0;

	//! Disconnect from game server
	virtual void DisconnectFromServer() = 0;

	//! Send update input request
	virtual void SendUpdateInputRequest(const SFireNetClientInput &input) = 0;

	//! Send spawn request
	virtual void SendSpawnRequest() = 0;

	//! Return connection status
	virtual bool IsConnected() = 0;

	//! Return local player status
	virtual bool IsLocalPlayerSpawned() = 0;

	//! Normaly shutdown plugin
	virtual bool Quit() = 0;
};