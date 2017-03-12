// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#include "StdAfx.h"

#include "UIManager.h"
#include "IUIPage.h"

#include <CrySystem/IConsole.h>

void CmdHideAll(IConsoleCmdArgs* args)
{
	mEnv->pUIManager->HideAll();
}

void CmdUnhideAll(IConsoleCmdArgs* args)
{
	mEnv->pUIManager->UnhideAll();
}

CUIManager::CUIManager() 
	: m_CurrentPage(nullptr)
{
	CryLogAlways(TITLE "Start initialization UI manager...");

	if (gEnv->pFlashUI)
	{
		REGISTER_COMMAND("ui_hide_all", CmdHideAll, VF_NULL, "Hide all ui pages");
		REGISTER_COMMAND("ui_unhide_all", CmdUnhideAll, VF_NULL, "Unhide all ui pages");

		CryLogAlways(TITLE "Success init UI manager");
	}
	else 
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Failed init UI manager. Flash not loaded");
	}
}

CUIManager::~CUIManager()
{
	UnloadAll();

	for (auto &it : m_Pages)
	{
		SAFE_DELETE(it);
	}

	m_Pages.clear();
}

bool CUIManager::RegisterUIPage(const char * name, IUIPage * page)
{
	if(!name || !page)
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Can't register UI page. Empty nama or nullpointer");

	for (const auto &it : m_Pages)
	{
		if (strcmp(it->GetName(), name) == 0)
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Can't register %s UI page. Page alredy registered", name);
			return false;
		}
	}

	m_Pages.push_back(page);

	CryLog(TITLE "Successfully register UI page (%s)", name);

	return true;
}

void CUIManager::UnloadPage(const char * name)
{
	for (const auto &it : m_Pages)
	{
		if (strcmp(it->GetName(), name) == 0)
		{
			it->UnloadPage();
			break;
		}
	}
}

void CUIManager::ShowPage(const char * name)
{
	int size = m_Pages.size();

	for (const auto &it : m_Pages)
	{
		if (strcmp(it->GetName(), name) == 0)
		{
			m_CurrentPage = it;
			it->ShowPage();
			break;
		}
	}
}

void CUIManager::HidePage(const char * name)
{
	for (const auto &it : m_Pages)
	{
		if (strcmp(it->GetName(), name) == 0)
		{
			it->HidePage();
			break;
		}
	}
}

void CUIManager::ReloadPage(const char * name)
{
	for (const auto &it : m_Pages)
	{
		if (strcmp(it->GetName(), name) == 0)
		{
			it->ReloadPage();
			break;
		}
	}
}

void CUIManager::CallFunction(const char* page, const char * functionName, const SUIArguments & args)
{
	for (const auto &it : m_Pages)
	{
		if (strcmp(it->GetName(), page) == 0)
		{
			it->CallFunction(functionName, args);
			break;
		}
	}
}

void CUIManager::HideAll()
{
	for (const auto &it : m_Pages)
	{
		it->HidePage();
	}
}

void CUIManager::UnhideAll()
{
	for (const auto &it : m_Pages)
	{
		it->UnhidePage();
	}
}

void CUIManager::UnloadAll()
{
	for (const auto &it : m_Pages)
	{
		it->UnloadPage();
	}
}

int CUIManager::GetPagesCount()
{
	return m_Pages.size();
}

IUIPage * CUIManager::GetPage(const char * name)
{
	for (const auto &it : m_Pages)
	{
		if (strcmp(it->GetName(), name) == 0)
			return it;
	}

	return nullptr;
}

IUIElement * CUIManager::GetUIElement(const char * name)
{
	for (const auto &it : m_Pages)
	{
		if (strcmp(it->GetName(), name) == 0)
			return it->GetUIElement();
	}

	return nullptr;
}