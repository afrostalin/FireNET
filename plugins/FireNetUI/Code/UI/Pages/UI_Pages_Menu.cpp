// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#include "StdAfx.h"
#include "UI/IUIPage.h"
#include "Main/Plugin.h"

// Loading page
class CPageLoading : public IUIPage
{
public:
	CPageLoading(const char* name) : IUIPage(name)
	{
		m_Background = "backgrounds/loading.png";
		m_LoadingText = "@ui_level_loading";
	}
public:
	virtual void OnShowPage(bool loaded)
	{
		if (loaded && pElement)
		{
			// Call set loading text
			SUIArguments load_args;
			load_args.AddArgument(m_LoadingText);
			CallFunction("SetLoadingStatus", load_args);

			// Call load background function
			SUIArguments bg_args;
			bg_args.AddArgument(m_Background);
			CallFunction("LoadBackground", bg_args);
		}
	}

	void SetBackground(const char* name) { m_Background = name; };
	void SetLoadingText(const char* text) { m_LoadingText = text; };
private:
	const char* m_Background;
	const char* m_LoadingText;
};

// Error page
class CPageErrorEventListener : public IUIElementEventListener
{
public:
	virtual void OnUIEvent(IUIElement* pSender, const SUIEventDesc& event, const SUIArguments& args)
	{
		CryLog("[UIEvents] Event (%s) called from flash", event.sDisplayName);

		if (strcmp(event.sDisplayName, "OnAccept") == 0)
		{
			mEnv->pUIManager->GetCurrentPage()->UnloadPage();
			gEnv->pSystem->Quit();
		}
	}
};

class CPageError : public IUIPage
{
public:
	CPageError(const char* name) : IUIPage(name, &m_EventListener) {}
public:
	virtual void OnShowPage(bool loaded) override
	{
		if (loaded)
		{
			// Call load background function
			SUIArguments args;
			args.AddArgument("backgrounds/error.png");
			CallFunction("LoadBackground", args);
		}
	}
private:
	CPageErrorEventListener m_EventListener;
};

//! UI page registrator
class CUIPagesMenuRegistrator : public IEntityRegistrator
{
	// Register UI pages here
	virtual void Register() override
	{
		UIManager::RegisterUIPage("ErrorPage", new CPageError("ErrorPage"));
		UIManager::RegisterUIPage("LoadingPage", new CPageLoading("LoadingPage"));
	}

	virtual void Unregister() override {}
};

CUIPagesMenuRegistrator gUIPagesMenuRegistrator;