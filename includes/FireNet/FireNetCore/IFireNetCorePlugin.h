// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <CrySystem/ICryPlugin.h>

struct IFireNetEnv;

struct IFireNetCorePlugin : public ICryPlugin
{
	CRYINTERFACE_DECLARE(IFireNetCorePlugin, 0x11780A2402EE4B94, 0xA80F0E9B34706316);
public:
	virtual IFireNetEnv* GetFireNetEnv() const = 0;
};