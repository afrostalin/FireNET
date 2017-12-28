// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <CrySystem/ICryPlugin.h>

#define USE_DEPRICATED_CRYENGINE_MACROS 0

struct IFireNetEnv;

struct IFireNetCorePlugin : public ICryPlugin
{
#if !USE_DEPRICATED_CRYENGINE_MACROS
	CRYINTERFACE_DECLARE_GUID(IFireNetCorePlugin, "E074643F-88DB-4F78-A381-3EE36BC650E4"_cry_guid);
#else
	CRYINTERFACE_DECLARE(IFireNetCorePlugin, 0x11780A2402EE4B94, 0xA80F0E9B34706316);
#endif 
public:
	virtual IFireNetEnv* GetFireNetEnv() const = 0;
};