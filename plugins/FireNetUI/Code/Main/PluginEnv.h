// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#pragma once

#include "UI/UIManager.h"
#include "UI/IUIPage.h"

#define TITLE "[FireNet-UI] "

struct SPluginEnv
{
	SPluginEnv()
	{
		pUIManager = nullptr;
	}

	//! Pointers
	CUIManager* pUIManager;
};

extern SPluginEnv* mEnv;

namespace UIManager
{
	//! Register UI page in manager
	static void RegisterUIPage(const char* name, IUIPage* page)
	{
		if (mEnv->pUIManager)
			mEnv->pUIManager->RegisterUIPage(name, page);
	}
}