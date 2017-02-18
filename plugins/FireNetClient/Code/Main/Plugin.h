// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include "Entities/ISimpleExtension.h"

#include <CrySystem/ICryPlugin.h>
#include <CryGame/IGameFramework.h>
#include <CryEntitySystem/IEntityClass.h>

#include <FireNet-Client>

class CFireNetClientPlugin 
	: public ICryPlugin
	, public ISystemEventListener
	, public IFireNetClientCore
	, public IGameFrameworkListener
{
public:
	CRYINTERFACE_SIMPLE(ICryPlugin)
	CRYGENERATE_SINGLETONCLASS(CFireNetClientPlugin, "FireNetClient_Impl", 0x0D7D9B7F70664BCF, 0x91C7F28F7EF39D72)

	virtual ~CFireNetClientPlugin();
	
	// ICryPlugin
	virtual const char* GetName() const override { return "FireNetClient"; }
	virtual const char* GetCategory() const override { return "Network"; }
	virtual bool        Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams) override;
	virtual void        OnPluginUpdate(EPluginUpdateType updateType) override {}
	// ~ICryPlugin

	// ISystemEventListener
	virtual void        OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	// IFireNetClientCore
	virtual void        ConnectToServer(const char* ip, int port) override;
	virtual void        DisconnectFromServer() override;
	virtual void        SendMovementRequest(EFireNetClientActions action, float value = 0.f) override;
	virtual bool        IsConnected() override { return false; }
	// ~IFireNetClientCore

	// IGameFrameworkListener
	virtual void        OnPostUpdate(float fDeltaTime) override;
	virtual void        OnSaveGame(ISaveGame* pSaveGame) override {};
	virtual void        OnLoadGame(ILoadGame* pLoadGame) override {};
	virtual void        OnLevelEnd(const char* nextLevel) override {};
	virtual void        OnActionEvent(const SActionEvent& event) override {};
	// ~IGameFrameworkListener
public:
	template<class T>
	struct CObjectCreator : public IGameObjectExtensionCreatorBase
	{
		IGameObjectExtension* Create(IEntity* pEntity)
		{
			return pEntity->CreateComponentClass<T>();
		}

		void GetGameObjectExtensionRMIData(void** ppRMI, size_t* nCount)
		{
			*ppRMI = nullptr;
			*nCount = 0;
		}
	};

	template<class T>
	static void RegisterEntityWithDefaultComponent(const char *name, const char* category = nullptr, const char* icon = nullptr, bool iconOnTop = false)
	{
		IEntityClassRegistry::SEntityClassDesc clsDesc;
		clsDesc.sName = name;

		clsDesc.editorClassInfo.sCategory = category;
		clsDesc.editorClassInfo.sIcon = icon;
		clsDesc.editorClassInfo.bIconOnTop = iconOnTop;

		static CObjectCreator<T> _creator;

		gEnv->pGameFramework->GetIGameObjectSystem()->RegisterExtension(name, &_creator, &clsDesc);
	}

	template<class T>
	static void RegisterEntityComponent(const char *name)
	{
		static CObjectCreator<T> _creator;

		gEnv->pGameFramework->GetIGameObjectSystem()->RegisterExtension(name, &_creator, nullptr);
		T::SetExtensionId(gEnv->pGameFramework->GetIGameObjectSystem()->GetID(name));
	}
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