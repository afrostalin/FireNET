// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNetCore/IFireNetCore.h>
#include <FireNetCore/IFireNetCorePlugin.h>

#include <FireNetClient/IFireNetClientCore.h>
#include <FireNetServer/IFireNetServerCore.h>

#include <SSL_CERTIFICATE.h>

struct IFireNetEnv
{
	IFireNetEnv()
	{
		pCore = nullptr;
		pClient = nullptr;
		pServer = nullptr;
	}
	virtual ~IFireNetEnv() {}

	IFireNetCore*       pCore;
	IFireNetClientCore* pClient;
	IFireNetServerCore* pServer;
};

extern IFireNetEnv* gFireNet;