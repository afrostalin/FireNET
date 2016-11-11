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
#ifdef CLIENT
		gCryModule->pUIEvents->RegisterUIEvents();
#endif
	}
	else
		gEnv->pLog->LogError(TITLE "Failed init module!");
}

FireNET_API void SendRequestToServer(const char* message)
{
	if (gCryModule->pNetwork != nullptr)
		gCryModule->pNetwork->SendQuery(message);
}

FireNET_API int GetUID()
{
	if (gCryModule->m_profile != nullptr)
		return gCryModule->m_profile->uid;
	else
		return 0;
}

#ifdef DEDICATED

FireNET_API void GameServerUpdateInfo()
{
	gEnv->pLog->LogWarning(TITLE "Update game server info in FireNET...");

	// Get server ip adress 
	QString ip = gEnv->pConsole->GetCVar("sv_bind")->GetString();
	// Get server port
	int port = gEnv->pConsole->GetCVar("sv_port")->GetIVal();
	// Get server name 
	QString serverName = gEnv->pConsole->GetCVar("sv_servername")->GetString();
	// Get current players count 
	int online = 0;
	// Get max players
	int maxPlayers = gEnv->pConsole->GetCVar("sv_maxplayers")->GetIVal();
	// Get map name 
	QString mapName = gEnv->pConsole->GetCVar("sv_map")->GetString();
	// Get gamerules
	QString gamerules = (char*)gEnv->pConsole->GetCVar("sv_gamerules")->GetString();

	QString query = "<query type='update_game_server'><data name = '" + serverName +
		"' ip = '" + ip +
		"' port = '" + QString::number(port) +
		"' map = '" + mapName +
		"' gamerules = '" + gamerules +
		"' online = '" + QString::number(online) +
		"' maxPlayers = '" + QString::number(maxPlayers) + "'/></query>";

	if (gCryModule->pNetwork != nullptr)
		gCryModule->pNetwork->SendQuery(query.toStdString().c_str());
	else
		gEnv->pLog->LogError(TITLE "Can't update game server because you not connected to FireNET");
}

FireNET_API SProfile* GameServerGetOnlineProfile(int uid)
{
	QString query = "<query type='get_online_profile'><data uid ='" + QString::number(uid) + "'/></query>";

	if (gCryModule->pNetwork != nullptr)
		gCryModule->pNetwork->SendQuery(query.toStdString().c_str());
	else
		gEnv->pLog->LogError(TITLE "Can't get online profile because you not connected to FireNET");

	// TODO

	return nullptr;
}

FireNET_API void GameServerUpdateOnlineProfile(SProfile* profile)
{
	QString query = "<query type='update_online_profile'>"
		"<data uid ='" + QString::number(profile->uid) +
		"' nickname ='" + profile->nickname +
		"' fileModel ='" + profile->fileModel +
		"' lvl ='" + QString::number(profile->lvl) + 
		"' xp ='" + QString::number(profile->xp) + 
		"' money ='" + QString::number(profile->money) + 
		"' items ='" + profile->items + 
		"' friends ='" + profile->friends + "'/>"
		"</query>";

	if (gCryModule->pNetwork != nullptr)
		gCryModule->pNetwork->SendQuery(query.toStdString().c_str());
	else
		gEnv->pLog->LogError(TITLE "Can't update online profile because you not connected to FireNET");
}

#endif

#endif