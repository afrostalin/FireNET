// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#include "StdAfx.h"
#include "UI/IUIPage.h"
#include "Main/Plugin.h"
#include <FireNet>

// Authorization page
class CPageAuthorizationEventListener : public IUIElementEventListener
{
public:
	// IUIElementEventListener
	virtual void OnUIEvent(IUIElement* pSender, const SUIEventDesc& event, const SUIArguments& args)
	{
		CryLog(TITLE "Event (%s) called from flash", event.sDisplayName);

		if (strcmp(event.sDisplayName, "OnLogin") == 0)
		{
			string login;
			string password;

			if (!gFireNet || !gFireNet->pCore)
			{
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Can't authorizate - FireNet-Core plugin not init!");
				return;
			}

			if (args.GetArg(0, login) && args.GetArg(1, password))
				gFireNet->pCore->Authorization(login.c_str(), password.c_str());
		}
		else if (strcmp(event.sDisplayName, "OnRegister") == 0)
		{
			string login;
			string password;

			if (!gFireNet || !gFireNet->pCore)
			{
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Can't register - FireNet-Core plugin not init!");
				return;
			}

			if (args.GetArg(0, login) && args.GetArg(1, password))
			{
				gFireNet->pCore->Registration(login.c_str(), password.c_str());
			}
		}
		else if (strcmp(event.sDisplayName, "OnExit") == 0)
		{
			mEnv->pUIManager->GetCurrentPage()->UnloadPage();
			gEnv->pSystem->Quit();
		}
	}
	// ~IUIElementEventListener
};

class CPageAuthorization : public IUIPage
{
public:
	CPageAuthorization(const char* name) : IUIPage(name, &m_EventListener) {}
public:
	// IUIPage
	virtual void OnShowPage(bool show) override
	{
		if (show)
		{
			// Call load background function
			SUIArguments args;
			args.AddArgument("backgrounds/main_menu.png");
			CallFunction("LoadBackground", args);
		}
	}
	// ~IUIPage
private:
	CPageAuthorizationEventListener m_EventListener;
};

//! UI page registrator
class CUIPagesOnlineRegistrator : public IEntityRegistrator
{
	// Register UI pages here
	virtual void Register() override
	{
		UIManager::RegisterUIPage("AuthorizationPage", new CPageAuthorization("AuthorizationPage"));
	}

	virtual void Unregister() override {}
};

CUIPagesOnlineRegistrator gUIPagesOnlineRegistrator;