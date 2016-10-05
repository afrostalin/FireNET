// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef _FireNET_Module_H_
#define _FireNET_Module_H_
#define FireNET_API extern "C" __declspec(dllexport)
#ifndef SAFESTR
#define SAFESTR(x) (((const char*)x)?((const char*)x):"")
#endif

#include "StdAfx.h"
#include "CryFlowGraph\IFlowBaseNode.h"

#include "Global.h"

FireNET_API void RegisterFlowNodes()
{
	gEnv->pLog->Log(TITLE "Register flow nodes...");

	/*if ()
	{
		gEnv->pLog->LogWarning(TITLE "It's dedicated server, not need register flow graph!");
	return;
	}*/

	if (IFlowSystem* pFlow= gEnv->pGame->GetIGameFramework()->GetIFlowSystem())
	{
		for (CAutoRegFlowNodeBase* pFactory = CAutoRegFlowNodeBase::m_pFirst; pFactory; pFactory = pFactory->m_pNext )
		{
			TFlowNodeTypeId nTypeId = pFlow->RegisterType( pFactory->m_sClassName, pFactory );

			if ( nTypeId != InvalidFlowNodeTypeId )
			{
				gEnv->pLog->Log( TITLE "Flow node class(%s) registered!", SAFESTR( pFactory->m_sClassName ));
			}

			else
			{
				gEnv->pLog->LogError( TITLE "Flow node class(%s) couldn't register!", SAFESTR( pFactory->m_sClassName ) );
			}
		}
	}
	else
	{
		gEnv->pLog->LogError( TITLE "Error register flow nodes!" );
	}
}

FireNET_API void InitModule(SSystemGlobalEnvironment& gCryEnv)
{
	gEnv = &gCryEnv;

	if(gEnv)
	{
		gCryModule->Init();

		// Register CVars, commands
		gCryModule->pCVars->RegisterCVars();
		gCryModule->pCVars->RegisterCComands();

		gCryModule->pUIEvents->RegisterUIEvents();
	}
	else
		gEnv->pLog->LogError(TITLE "Failed init module!");
}

FireNET_API void SendRequestToServer(const char* message)
{
	if (gCryModule->pNetwork != nullptr)
		gCryModule->pNetwork->SendQuery(message);
}

#endif