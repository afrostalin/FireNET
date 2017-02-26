// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include "IFireNetServerBase.h"

//! Base interface for using in FireNet-Server plugin
struct IFireNetServerCore
{
	virtual ~IFireNetServerCore() {}

	//! Register game server in FireNet master server
	virtual void RegisterGameServer() = 0;

	//! Normaly shutdown plugin
	virtual bool Quit() = 0;
};