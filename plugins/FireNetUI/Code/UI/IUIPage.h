// Copyright (C) 2016-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/EasyShooter/blob/master/LICENCE.md

#pragma once

#include "UIManager.h"
#include <CrySystem/Scaleform/IFlashUI.h>

enum class EUIPageStatus
{
	None,
	Init,
	Unloaded,
	Visible,
	Invisible,
};

class IUIPage
{
public:
	IUIPage(const char* name, IUIElementEventListener* eventListener = nullptr) 
		: pElement(nullptr)
		, pEventListener(eventListener)
	{
		this->m_Name = name;
		m_Status = EUIPageStatus::None;

		Init();
	}
public:
	virtual void             Init()
	{
		CryLog("[FireNet-UI] Init page (%s)", m_Name);

		if (gEnv->pFlashUI)
		{
			pElement = gEnv->pFlashUI->GetUIElement(m_Name);

			if (pElement)
			{
				if (pEventListener)
					pElement->AddEventListener(pEventListener, "EventListener");
					
				CryLog("[FireNet-UI] Success init page (%s)", m_Name);

				OnInitPage(true);
				m_Status = EUIPageStatus::Init;
			}
			else
			{
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "[FireNet-UI] Failed init page (%s). UIElement not found", m_Name);

				OnInitPage(false);
			}
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "[FireNet-UI] Failed init page (%s). Flash not loaded", m_Name);

			OnInitPage(false);
		}
	}
	virtual void             UnloadPage()
	{
		CryLog("[FireNet-UI] Unloading page (%s)", m_Name);

		if (gEnv->pSystem->IsQuitting())
			return;

		if (pElement)
		{
			pElement->SetVisible(false);
			pElement->Unload();

			OnUnloadPage(true);

			m_Status = EUIPageStatus::Unloaded;
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "[FireNet-UI] Failed unload page (%s). UIElement not found", m_Name);

			OnUnloadPage(false);
		}
	}
	virtual void             ShowPage()
	{
		CryLog("[FireNet-UI] Show page (%s)", m_Name);

		if (pElement && !gEnv->pSystem->IsQuitting())
		{
			pElement->SetVisible(true);

			OnShowPage(true);

			m_Status = EUIPageStatus::Visible;
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "[FireNet-UI] Failed show page (%s). UIElement not found", m_Name);

			OnShowPage(false);
		}
	}
	virtual void             HidePage()
	{
		if (m_Status == EUIPageStatus::Visible)
		{
			CryLog("[FireNet-UI] Hide page (%s)", m_Name);

			if (pElement && !gEnv->pSystem->IsQuitting())
			{
				pElement->SetVisible(false);

				OnHidePage(true);

				m_Status = EUIPageStatus::Invisible;
			}
			else
			{
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "[FireNet-UI] Failed hide page (%s). UIElement not found", m_Name);

				OnHidePage(false);
			}
		}
	}
	virtual void             UnhidePage()
	{
		if (m_Status == EUIPageStatus::Invisible)
		{
			ShowPage();
		}
	}
	virtual void             ReloadPage()
	{
		CryLog("[FireNet-UI] Reloading page (%s)", m_Name);

		if (pElement && !gEnv->pSystem->IsQuitting())
		{
			pElement->Reload(true);

			OnReloadPage(true);
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "[FireNet-UI] Failed reload page (%s). UIElement not found", m_Name);

			OnReloadPage(false);
		}
	}
	void                     CallFunction(const char* functionName, const SUIArguments& args = SUIArguments())
	{
		if (pElement)
		{
			CryLog("[FireNet-UI] Caling UI function (%s) on page (%s)", functionName, m_Name);
			pElement->CallFunction(functionName, args);
		}
		else
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "[FireNet-UI] Failed call function (%s). UI element = nullptr", functionName);
	}
public:
	virtual void             OnInitPage(bool init) {}
	virtual void             OnUnloadPage(bool unloaded) {}
	virtual void             OnShowPage(bool show) {}
	virtual void             OnHidePage(bool hide) {}
	virtual void             OnReloadPage(bool reloaded) {}
public:
	virtual const char*      GetName() { return this->m_Name; };
	virtual IUIElement*      GetUIElement() { return this->pElement; };
private:
	const char*              m_Name;
	EUIPageStatus            m_Status;
public:
	IUIElement*              pElement;
	IUIElementEventListener* pEventListener;
};