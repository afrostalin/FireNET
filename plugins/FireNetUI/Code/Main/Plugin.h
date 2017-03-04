// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <CrySystem/ICryPlugin.h>
#include <CryGame/IGameFramework.h>
#include <CryEntitySystem/IEntityClass.h>

#include <FireNet>

class CFireNetUIPlugin 
	: public ICryPlugin
	, public ISystemEventListener
	, public IFireNetListener
{
public:
	CRYINTERFACE_SIMPLE(ICryPlugin)
	CRYGENERATE_SINGLETONCLASS(CFireNetUIPlugin, "FireNetUI_Plugin", 0x04EC567175F5456B, 0x9D253790D4441100)

	virtual ~CFireNetUIPlugin();
	
	// ICryPlugin
	virtual const char* GetName() const override { return "FireNet-UI"; }
	virtual const char* GetCategory() const override { return "Game"; }
	virtual bool        Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams) override;
	virtual void        OnPluginUpdate(EPluginUpdateType updateType) override {}
	// ~ICryPlugin

	// ISystemEventListener
	virtual void        OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	// IFireNetListener
	virtual void OnFireNetEvent(EFireNetEvents event, SFireNetEventArgs& args = SFireNetEventArgs()) override;
	// ~IFireNetListener
};

struct IEntityRegistrator
{
	IEntityRegistrator()
	{
		if (g_pFirst == nullptr)
		{
			g_pFirst = this;
			g_pLast = this;
		}
		else
		{
			g_pLast->m_pNext = this;
			g_pLast = g_pLast->m_pNext;
		}
	}

	virtual void Register() = 0;
	virtual void Unregister() = 0;

public:
	IEntityRegistrator *m_pNext;

	static IEntityRegistrator *g_pFirst;
	static IEntityRegistrator *g_pLast;
};