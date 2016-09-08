/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2016

*************************************************************************/

#ifndef _GamePlay_Module_H_
#define _GamePlay_Module_H_
#define GamePlay_API extern "C" __declspec(dllexport)
#ifndef SAFESTR
#define SAFESTR(x) (((const char*)x)?((const char*)x):"")
#endif

#include "StdAfx.h"
#include "CryFlowGraph\IFlowBaseNode.h"

#include "Global.h"

GamePlay_API void RegisterFlowNodes()
{
	gEnv->pLog->Log(TITLE "Register flownodes");

	if (IFlowSystem* pFlow= gEnv->pGame->GetIGameFramework()->GetIFlowSystem())
	{
		for (CAutoRegFlowNodeBase* pFactory = CAutoRegFlowNodeBase::m_pFirst; pFactory; pFactory = pFactory->m_pNext )
		{
			TFlowNodeTypeId nTypeId = pFlow->RegisterType( pFactory->m_sClassName, pFactory );

			if ( nTypeId != InvalidFlowNodeTypeId )
			{
				gEnv->pLog->Log( TITLE "Flownode class(%s) registered!", SAFESTR( pFactory->m_sClassName ));
			}

			else
			{
				gEnv->pLog->LogError( TITLE "Flownode class(%s) couldn't register!", SAFESTR( pFactory->m_sClassName ) );
			}
		}
	}
	else
	{
		gEnv->pLog->LogError( TITLE "Error register flow nodes!" );
	}
}

GamePlay_API void InitModule(SSystemGlobalEnvironment& gCryEnv)
{
	gEnv = &gCryEnv;

	if(gEnv)
	{
		gCryModule->Init();

		// Register CVars, commands
		gCryModule->pCVars->RegisterCVars();
		gCryModule->pCVars->RegistevCComands();
		gCryModule->pUIEvents->RegisterUIEvents();
	}
	else
		gEnv->pLog->LogError(TITLE "Failed init module!");
}

#endif