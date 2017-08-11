// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNet>

#include <CryEntitySystem/IEntityClass.h>
#include <CryGame/IGameFramework.h>

class CFireNetCorePlugin 
	: public IFireNetCorePlugin	
	, public ISystemEventListener
	, public IFireNetCore
{
public:
	CRYINTERFACE_BEGIN()
	CRYINTERFACE_ADD(IFireNetCorePlugin)
	CRYINTERFACE_ADD(ICryPlugin)
	CRYINTERFACE_END()

	CRYGENERATE_SINGLETONCLASS(CFireNetCorePlugin, "FireNetCore_Plugin", 0x2799ED0066B04E6B, 0xA5722E51345346A8)

	PLUGIN_FLOWNODE_REGISTER
	PLUGIN_FLOWNODE_UNREGISTER

	virtual ~CFireNetCorePlugin();
public:
	// ICryPlugin
	virtual const char*      GetName() const override { return "FireNet-Core"; }
	virtual const char*      GetCategory() const override { return "Network"; }
	virtual bool             Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams) override;
	virtual void             OnPluginUpdate(EPluginUpdateType updateType) override;
	// ~ICryPlugin
public:
	// ISystemEventListener
	virtual void             OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener
public:
	// IFireNetCore
	virtual void             RegisterFireNetListener(IFireNetListener *listener) override;
	virtual void             ConnectToMasterServer() override;
	virtual void             Authorization(const std::string &login, const std::string &password) override;
	virtual void             Registration(const std::string &login, const std::string &password) override;
	virtual void             CreateProfile(const std::string &nickname, const std::string &character) override;
	virtual void             GetProfile(int uid = 0) override;
	virtual SFireNetProfile* GetLocalProfile() override;
	virtual void             GetShop() override;
	virtual void             BuyItem(const std::string &item) override;
	virtual void             RemoveItem(const std::string &item) override;
	virtual void             SendInvite(EFireNetInviteType type, int uid) override;
	virtual void             DeclineInvite() override;
	virtual void             AcceptInvite() override;
	virtual void             RemoveFriend(int uid) override;
	virtual void             SendChatMessage(EFireNetChatMsgType type, int uid = 0) override;
	virtual void             GetGameServer(const std::string &map, const std::string &gamerules) override;
	virtual void             SendRawRequestToMasterServer(CTcpPacket &packet) override;
	virtual bool             IsConnected() override;
	virtual bool             GetHWID(std::string &hwid) override;
	virtual void             SendFireNetEvent(EFireNetEvents event, SFireNetEventArgs& args = SFireNetEventArgs()) override;
	virtual bool             Quit() override;
	// ~IFireNetCore
public:
	virtual IFireNetEnv*     GetFireNetEnv() const override { return gFireNet; };
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