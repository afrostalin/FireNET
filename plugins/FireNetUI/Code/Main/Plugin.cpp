// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "Plugin.h"
#include "UI/UIManager.h"
#include <CryCore/Platform/platform_impl.inl>
#include <CryExtension/ICryPluginManager.h>
#include <FireNet.inl>

IEntityRegistrator *IEntityRegistrator::g_pFirst = nullptr;
IEntityRegistrator *IEntityRegistrator::g_pLast = nullptr;

CFireNetUIPlugin::~CFireNetUIPlugin()
{
	// Unregister entities
	IEntityRegistrator* pTemp = IEntityRegistrator::g_pFirst;
	while (pTemp != nullptr)
	{
		pTemp->Unregister();
		pTemp = pTemp->m_pNext;
	}

	// Destroy UI manager
	SAFE_DELETE(mEnv->pUIManager);

	// Unregister listeners
	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);

	CryLogAlways(TITLE "Unloaded.");
}

bool CFireNetUIPlugin::Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams)
{
	if (initParams.bEditor && !gEnv->IsEditor())
		gEnv->SetIsEditor(true);

	if (initParams.bDedicatedServer && !gEnv->IsDedicated())
		gEnv->SetIsDedicated(true);

	//! Get FireNet
	if (auto pPluginManager = gEnv->pSystem->GetIPluginManager())
	{
		if (auto pPlugin = pPluginManager->QueryPlugin<IFireNetCorePlugin>())
		{
			gFireNet = pPlugin->GetFireNetEnv();
			if (!gFireNet)
				CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Error init FireNet - Can't get FireNet environment pointer!");
			else
				gFireNet->pCore->RegisterFireNetListener(this);
		}
		else
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Error init FireNet - Can't get Plugin!");
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Error init FireNet - Can't get factory!");

	//! UI manager can't work with dedicated server
	if (!gEnv->IsDedicated())
		gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this);

	return true;
}

void CFireNetUIPlugin::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	// Init UI manager and register entities / UI pages
	case ESYSTEM_EVENT_GAME_POST_INIT:
	{		
		mEnv->pUIManager = new CUIManager();

		IEntityRegistrator* pTemp = IEntityRegistrator::g_pFirst;
		while (pTemp != nullptr)
		{
			pTemp->Register();
			pTemp = pTemp->m_pNext;
		}	
	}
	// Show loading page on level loading
	case ESYSTEM_EVENT_LEVEL_LOAD_START :
	{
		if (!gEnv->IsEditor() && mEnv->pUIManager)
		{
			SUIArguments load_args;
			load_args.AddArgument("@ui_level_loading");

			mEnv->pUIManager->ShowPage("LoadingPage");
			mEnv->pUIManager->CallFunction("LoadingPage", "SetLoadingStatus", load_args);
		}
	}
	// Hide HUD and load loading page when level unloading
	case ESYSTEM_EVENT_LEVEL_UNLOAD:
	{
		if (!gEnv->IsEditor() && mEnv->pUIManager)
		{
			auto pHUD = mEnv->pUIManager->GetPage("HUDPage");

			if (pHUD)
				pHUD->UnloadPage();

			SUIArguments load_args;
			load_args.AddArgument("@ui_level_unloading");

			mEnv->pUIManager->ShowPage("LoadingPage");
			mEnv->pUIManager->CallFunction("LoadingPage", "SetLoadingStatus", load_args);
		}
		break;
	}
	// Hide loading page and show HUD when gameplay started
	case ESYSTEM_EVENT_LEVEL_GAMEPLAY_START:
	{
		if (!gEnv->IsEditor() && mEnv->pUIManager)
			mEnv->pUIManager->HidePage("LoadingPage");
		else if (mEnv->pUIManager)
			mEnv->pUIManager->ShowPage("HUDPage");
		break;
	}
	// Hide loading page and show main menu after level unloaded
	case ESYSTEM_EVENT_LEVEL_POST_UNLOAD:
	{
		if (!gEnv->IsEditor() && mEnv->pUIManager)
		{
			mEnv->pUIManager->HidePage("LoadingPage");
			mEnv->pUIManager->ShowPage("MainPage");
		}
		break;
	}
	// Hide HUD and show in-game menu when game paused
	case ESYSTEM_EVENT_GAME_PAUSED:
	{
		if (mEnv->pUIManager)
		{
			mEnv->pUIManager->HidePage("HUDPage");
			mEnv->pUIManager->ShowPage("InGamePage");
		}
		break;
	}
	// Hide in-game menu and show HUD when game resumed
	case ESYSTEM_EVENT_GAME_RESUMED:
	{
		if (!gEnv->IsDedicated() && mEnv->pUIManager)
		{
			mEnv->pUIManager->ShowPage("HUDPage");
			mEnv->pUIManager->HidePage("InGamePage");
		}

		break;
	}
	break;
	}
}

void CFireNetUIPlugin::OnFireNetEvent(EFireNetEvents event, SFireNetEventArgs & args)
{
	switch (event)
	{
	//! Show loading page when start connection to master server
	case FIRENET_EVENT_MASTER_SERVER_START_CONNECTION:
	{
		SUIArguments load_args;
		load_args.AddArgument("@ui_connecting_to_master_server");

		mEnv->pUIManager->ShowPage("LoadingPage");
		mEnv->pUIManager->CallFunction("LoadingPage", "SetLoadingStatus", load_args);

		break;
	}
	//! Hide loading page and show auth page when master server connected
	case FIRENET_EVENT_MASTER_SERVER_CONNECTED:
	{
		mEnv->pUIManager->HidePage("LoadingPage");
		mEnv->pUIManager->ShowPage("AuthorizationPage");

		break;
	}
	//! Show error page when connection error
	case FIRENET_EVENT_MASTER_SERVER_CONNECTION_ERROR:
	{
		int reason = args.GetInt();
		string error = args.GetString();

		SUIArguments errorString;
		errorString.AddArgument("@ui_" + error);

		mEnv->pUIManager->HideAll();
		mEnv->pUIManager->ShowPage("ErrorPage");
		mEnv->pUIManager->CallFunction("ErrorPage", "SetErrorText", errorString);

		break;
	}
	// Show error page when connection lose
	case FIRENET_EVENT_MASTER_SERVER_DISCONNECTED:
	{
		SUIArguments errorString;
		errorString.AddArgument("ui_lose_connection_with_master_server");

		mEnv->pUIManager->HideAll();
		mEnv->pUIManager->ShowPage("ErrorPage");
		mEnv->pUIManager->CallFunction("ErrorPage", "SetErrorText", errorString);

		break;
	}
	// If authorization complete without profile show create profile page
	case FIRENET_EVENT_AUTHORIZATION_COMPLETE:
	{
		mEnv->pUIManager->UnloadPage("AuthorizationPage");
		mEnv->pUIManager->ShowPage("CreateProfilePage");
		break;
	}
	// If authorization complete with profile show main menu
	case FIRENET_EVENT_AUTHORIZATION_COMPLETE_WITH_PROFILE:
	{
		mEnv->pUIManager->UnloadPage("AuthorizationPage");
		mEnv->pUIManager->ShowPage("MainMenuPage");
		break;
	}
	// If authorization failed show error
	case FIRENET_EVENT_AUTHORIZATION_FAILED:
	{
		int reason = args.GetInt();
		string error = args.GetString();

		CryLog(TITLE "Error : %s", error.c_str());

		SUIArguments errorString;
		errorString.AddArgument("@ui_" + error);

		mEnv->pUIManager->CallFunction("AuthorizationPage", "SetServerResultText", errorString);

		break;
	}
	// If registration complete show login page
	case FIRENET_EVENT_REGISTRATION_COMPLETE:
	{
		SUIArguments resultString;
		resultString.AddArgument("@ui_registration_complete");

		mEnv->pUIManager->CallFunction("AuthorizationPage", "ShowLoginPage");
		mEnv->pUIManager->CallFunction("AuthorizationPage", "SetServerResultText", resultString);

		break;
	}
	// If registration failed show error
	case FIRENET_EVENT_REGISTRATION_FAILED:
	{
		int reason = args.GetInt();
		string error = args.GetString();

		SUIArguments errorString;
		errorString.AddArgument("@ui_" + error);

		mEnv->pUIManager->CallFunction("AuthorizationPage", "SetServerResultText", errorString);

		break;
	}
	case FIRENET_EVENT_UPDATE_PROFILE:
		break;
	case FIRENET_EVENT_CREATE_PROFILE_COMPLETE:
		break;
	case FIRENET_EVENT_CREATE_PROFILE_FAILED:
		break;
	case FIRENET_EVENT_GET_PROFILE_COMPLETE:
		break;
	case FIRENET_EVENT_GET_PROFILE_FAILED:
		break;
	case FIRENET_EVENT_GET_SHOP_COMPLETE:
		break;
	case FIRENET_EVENT_GET_SHOP_FAILED:
		break;
	case FIRENET_EVENT_BUY_ITEM_COMPLETE:
		break;
	case FIRENET_EVENT_BUY_ITEM_FAILED:
		break;
	case FIRENET_EVENT_REMOVE_ITEM_COMPLETE:
		break;
	case FIRENET_EVENT_REMOVE_ITEM_FAILED:
		break;
	case FIRENET_EVENT_SEND_INVITE_COMPLETE:
		break;
	case FIRENET_EVENT_SEND_INVITE_FAILED:
		break;
	case FIRENET_EVENT_DECLINE_INVITE_COMPLETE:
		break;
	case FIRENET_EVENT_DECLINE_INVITE_FAILED:
		break;
	case FIRENET_EVENT_ACCEPT_INVITE_COMPLETE:
		break;
	case FIRENET_EVENT_ACCEPT_INVITE_FAILED:
		break;
	case FIRENET_EVENT_REMOVE_FRIEND_COMPLETE:
		break;
	case FIRENET_EVENT_REMOVE_FRIEND_FAILED:
		break;
	case FIRENET_EVENT_SEND_CHAT_MSG_COMPLETE:
		break;
	case FIRENET_EVENT_SEND_CHAT_MSG_FAILED:
		break;
	case FIRENET_EVENT_GET_GAME_SERVER_COMPLETE:
		break;
	case FIRENET_EVENT_GET_GAME_SERVER_FAILED:
		break;
	case FIRENET_EVENT_GLOBAL_CHAT_MSG_RECEIVED:
		break;
	case FIRENET_EVENT_PRIVATE_CHAT_MSG_RECEIVED:
		break;
	case FIRENET_EVENT_CLAN_CHAT_MSG_RECEIVED:
		break;
	case FIRENET_EVENT_CONSOLE_COMMAND_RECEIVED:
		break;
	case FIRENET_EVENT_SERVER_MESSAGE_RECEIVED:
		break;
	case FIRENET_EVENT_GAME_SERVER_INFO_RECEIVED:
		break;
	case FIRENET_EVENT_GAME_SERVER_START_CONNECTION:
		break;
	case FIRENET_EVENT_GAME_SERVER_CONNECTED:
		break;
	case FIRENET_EVENT_GAME_SERVER_CONNECTION_ERROR:
		break;
	case FIRENET_EVENT_GAME_SERVER_DISCONNECTED:
		break;
	default:
		break;
	}
}

CRYREGISTER_SINGLETON_CLASS(CFireNetUIPlugin)