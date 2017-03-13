// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#pragma once

#include <CrySystem/Scaleform/IFlashUI.h>
#include <CryThreading/CryThread.h>

class IUIPage;

class CUIManager
{
public:
	CUIManager();
	~CUIManager();
public:
	bool                  RegisterUIPage(const char* name, IUIPage* page);
public:
	void                  UnloadPage(const char* name);
	void                  ShowPage(const char* name);
	void                  HidePage(const char* name);
	void                  ReloadPage(const char* name);

	void                  CallFunction(const char* page, const char* functionName, const SUIArguments& args = SUIArguments());

	void                  HideAll();
	void                  UnhideAll();

	void                  UnloadAll();

	int                   GetPagesCount();
	IUIPage*              GetPage(const char* name);
	IUIElement*           GetUIElement(const char* name);
	IUIPage*              GetCurrentPage() { return m_CurrentPage; }
private:
	std::vector<IUIPage*> m_Pages;
	IUIPage*              m_CurrentPage;
	CryMutexFast          m_Mutex;
};