// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
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
	const char*      GetName() const override { return "FireNet-Core"; }
	const char*      GetCategory() const override { return "Network"; }
	bool             Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams) override;
	void             OnPluginUpdate(EPluginUpdateType updateType) override;
	// ~ICryPlugin
public:
	// ISystemEventListener
	void             OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener
public:
	// IFireNetCore
	void             RegisterFireNetListener(IFireNetListener *listener) override;
	void             UnregisterFireNetListener(IFireNetListener *listener) override;
	void             ConnectToMasterServer() override;
	void             Authorization(const char* login, const char* password) override;
	void             Registration(const char* login, const char* password) override;
	void             CreateProfile(const char* nickname, const char* character) override;
	void             GetProfile(int uid = 0) override;
	SFireNetProfile* GetLocalProfile() override;
	void             GetShop() override;
	void             BuyItem(const char* item) override;
	void             RemoveItem(const char* item) override;
	void             SendInvite(EFireNetInviteType type, int uid) override;
	void             DeclineInvite() override;
	void             AcceptInvite() override;
	void             RemoveFriend(int uid) override;
	void             SendChatMessage(EFireNetChatMsgType type, int uid = 0) override;
	void             RegisterGameServer(SFireNetGameServer serverInfo) override;
	void             UpdateGameServer(SFireNetGameServer serverInfo) override;
	void             GetGameServer(const char* map, const char* gamerules) override;
	void             SendRawRequestToMasterServer(CTcpPacket &packet) override;
	bool             IsConnected() override;
	void             GetFireNetEventListeners(std::vector<IFireNetListener*> &vector) override;
	void             GetHWID(string &hwid) override;
	void             ExecuteEvent(EFireNetEvents eventName) override;
	bool             Quit() override;
	// ~IFireNetCore
public:
	IFireNetEnv*     GetFireNetEnv() const override { return gFireNet; };
private:
	void                     RegisterCVars();
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