// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "StdAfx.h"
#include <CryCore/Platform/CryLibrary.h>
#include "PluginLoader.h"

CPLuginLoader* pPluginLoader = new CPLuginLoader;

CPLuginLoader::CPLuginLoader()
{
	hndl = nullptr;
	pInitModule = nullptr;
	pRegisterFlowNodes = nullptr;
}

CPLuginLoader::~CPLuginLoader()
{
	SAFE_DELETE(pPluginLoader);
}

void CPLuginLoader::LoadPlugin()
{
	CryLogAlways("[" PLUGIN_NAME "] - Start loading plugin");

	hndl = CryLoadLibrary(PLUGIN_NAME);

	if (hndl != nullptr)
	{
		pInitModule = (void(*)(SSystemGlobalEnvironment&)) CryGetProcAddress(hndl, "InitModule");
		pRegisterFlowNodes = (void(*)(void)) CryGetProcAddress(hndl, "RegisterFlowNodes");

		if (pInitModule != nullptr && pRegisterFlowNodes != nullptr)
		{
			pInitModule(*gEnv);
			CryLogAlways(PLUGIN_LOADED);
		}
		else
		{
			hndl = nullptr;
			CryLogAlways(PLUGIN_ERROR " - POINTERS ERROR");
		}
	}
	else
	{
		CryLogAlways(PLUGIN_ERROR " - ERROR LOADING DLL");
		hndl = nullptr;
	}
}


void CPLuginLoader::ReloadPlugins()
{
	CryLogAlways("CPluginLoader :: Reload plugins");
	FreePlugins();
	LoadPlugin();
}

void CPLuginLoader::FreePlugins()
{
	if (hndl != nullptr)
		CryFreeLibrary(hndl);
}