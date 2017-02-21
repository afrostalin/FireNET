// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNet-Core>

#include <CrySystem/ICryPlugin.h>
#include <CryEntitySystem/IEntityClass.h>
#include <CryGame/IGameFramework.h>

class CFireNetCorePlugin 
	: public ICryPlugin
	, public ISystemEventListener
	, public IGameFrameworkListener
	, public IFireNetCore	
{
public:
	CRYINTERFACE_SIMPLE(ICryPlugin)
	CRYGENERATE_SINGLETONCLASS(CFireNetCorePlugin, "FireNetCore_Plugin", 0x2799ED0066B04E6B, 0xA5722E51345346A8)
	PLUGIN_FLOWNODE_REGISTER
	PLUGIN_FLOWNODE_UNREGISTER

	virtual ~CFireNetCorePlugin();
	
	// ICryPlugin
	virtual const char*      GetName() const override { return "FireNetCore"; }
	virtual const char*      GetCategory() const override { return "Network"; }
	virtual bool             Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams) override;
	virtual void             OnPluginUpdate(EPluginUpdateType updateType) override {};
	// ~ICryPlugin

	// ISystemEventListener
	virtual void             OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	// IGameFrameworkListener
	virtual void             OnPostUpdate(float fDeltaTime) override;
	virtual void             OnSaveGame(ISaveGame* pSaveGame) override {};
	virtual void             OnLoadGame(ILoadGame* pLoadGame) override {};
	virtual void             OnLevelEnd(const char* nextLevel) override {};
	virtual void             OnActionEvent(const SActionEvent& event) override {};
	// ~IGameFrameworkListener

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
	virtual void             SendFireNetEvent(EFireNetEvents event, SFireNetEventArgs& args = SFireNetEventArgs()) override;
	// ~IFireNetCore
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