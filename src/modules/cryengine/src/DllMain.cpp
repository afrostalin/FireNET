// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include <StdAfx.h>
#include <Windows.h>
#include "Global.h"
#include "CFireNET_Base.h"
#include <CryFlowGraph/IFlowBaseNode.h>

#define FireNET_API extern "C" __declspec(dllexport)

// Need for flow graph node system
CAutoRegFlowNodeBase* CAutoRegFlowNodeBase::m_pFirst = 0;
CAutoRegFlowNodeBase* CAutoRegFlowNodeBase::m_pLast = 0;

namespace FireNET
{
	CFireNETBase* gFireNET = nullptr;

	FireNET_API IFireNETBase* GetModuleEnv()
	{
		static CFireNETBase fireNETmodule;
		gFireNET = &fireNETmodule;

		return fireNETmodule.GetBase();
	}

	FireNET_API void InitModule(SSystemGlobalEnvironment& gCryEnv)
	{
		gEnv = &gCryEnv;

		if (gEnv)
		{
			// Register CVars, commands
			gModuleEnv->pCVars->RegisterCVars();
			gModuleEnv->pCVars->RegisterCComands();
			// UI system not need for server
#ifndef DEDICATED_SERVER
			gModuleEnv->pUIEvents->RegisterUIEvents();
#endif
			// Register in ISystemEventListener
			if (gFireNET)
			{
				gFireNET->Init();
			}
			else
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Can't register listeners. Module environment empty!");
		}
		else
			gEnv->pLog->LogError(TITLE "Failed init module!");
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
