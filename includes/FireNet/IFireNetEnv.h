// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNetCore/IFireNetCore.h>
#include <FireNetCore/IFireNetCorePlugin.h>

#include <SSL_CERTIFICATE.h>

struct IFireNetEnv
{
	IFireNetEnv()
		: pCore(nullptr)
	{
	}
	virtual ~IFireNetEnv() {}

	IFireNetCore*       pCore;
};

extern IFireNetEnv* gFireNet;