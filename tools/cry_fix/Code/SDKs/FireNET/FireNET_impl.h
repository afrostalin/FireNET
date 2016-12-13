// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include <FireNET_Base.h>
#include <CryCore/Platform/CryLibrary.h>

// Definition only once 
#if defined(__GAMESTARTUP_H__)
FireNET::IFireNETBase* gFireNET = nullptr;
#endif

#if defined(DEDICATED_SERVER)
#define PLUGIN_NAME "FireNET_Dedicated.dll"
#else
#define PLUGIN_NAME "FireNET.dll"
#endif

#define TITLE "[FireNET] "
#define PLUGIN_LOADED TITLE "Plugin loaded"
#define PLUGIN_ERROR TITLE "Error loading plugin"

namespace FireNET
{
	static bool InitFireNETModule()
	{
		CryLogAlways(TITLE "Start loading plugin");

		HINSTANCE hndl = CryLoadLibrary(PLUGIN_NAME);

		if (hndl)
		{
			auto pInitModule = (void(*)(SSystemGlobalEnvironment&)) CryGetProcAddress(hndl, "InitModule");
			auto pGetModuleEnv = (IFireNETBase*(*)(void)) CryGetProcAddress(hndl, "GetModuleEnv");

			if (pInitModule && pGetModuleEnv)
			{
				// First init module environment
				gFireNET = pGetModuleEnv();

				// Init global environment
				pInitModule(*gEnv);

				if (gFireNET)
				{
					CryLogAlways(PLUGIN_LOADED);
					return true;
				}
				else
				{
					CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, PLUGIN_ERROR " - Can't get module environment");
					return false;
				}
			}
			else
			{
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, PLUGIN_ERROR " - Can't get entry point");
				return false;
			}
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, PLUGIN_ERROR " - Dll not loaded!");
			return false;
		}
	}

	static void RegisterFlowNodes()
	{
		if (gFireNET)
		{
			gFireNET->RegisterFlowGraphNodes();
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, PLUGIN_ERROR " - Can't register flow nodes, module environment not loaded");
		}
	}
};