// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "StdAfx.h"
#include <CryCore/Platform/CryLibrary.h>
#include "PluginLoader.h"

CPLuginLoader* pPluginLoader = new CPLuginLoader;

CPLuginLoader::CPLuginLoader()
{
	hndl = nullptr;
	InitModule = nullptr;
	SendRequest = nullptr;
	RegisterFlowNodes = nullptr;

#if defined(DEDICATED_SERVER)
	GameServerUpdateInfo = nullptr;
	GameServerGetOnlineProfile = nullptr;
	GameServerUpdateOnlineProfile = nullptr;
#endif	
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

		InitModule = (void(*)(SSystemGlobalEnvironment&)) CryGetProcAddress(hndl, "InitModule");
		SendRequest = (void(*)(const char*)) CryGetProcAddress(hndl, "SendRequestToServer");
		RegisterFlowNodes = (void(*)(void)) CryGetProcAddress(hndl, "RegisterFlowNodes");

#if defined(DEDICATED_SERVER)
		GameServerUpdateInfo = (void(*)(void)) CryGetProcAddress(hndl, "GameServerUpdateInfo");
		GameServerGetOnlineProfile = (SProfile*(*)(int)) CryGetProcAddress(hndl, "GameServerGetOnlineProfile");
		GameServerUpdateOnlineProfile = (void(*)(SProfile*)) CryGetProcAddress(hndl, "GameServerUpdateOnlineProfile");

		if (InitModule != nullptr && SendRequest != nullptr  && RegisterFlowNodes != nullptr &&
			GameServerUpdateInfo != nullptr && GameServerGetOnlineProfile != nullptr && GameServerUpdateOnlineProfile != nullptr)
		{
			InitModule(*gEnv);
			CryLogAlways(PLUGIN_LOADED);
		}
		else
		{
			hndl = nullptr;
			CryLogAlways(PLUGIN_ERROR " - POINTERS ERROR");
		}
#else
		if (InitModule != nullptr && RegisterFlowNodes != nullptr && SendRequest != nullptr)
		{
			InitModule(*gEnv);
			CryLogAlways(PLUGIN_LOADED);
		}
		else
		{
			hndl = nullptr;
			CryLogAlways(PLUGIN_ERROR " - POINTERS ERROR");
		}
#endif	
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