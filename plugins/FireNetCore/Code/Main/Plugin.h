// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNet>

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

	CRYGENERATE_SINGLETONCLASS_GUID(CFireNetCorePlugin, "FireNetCore_Plugin", "37245B6D-988B-4334-A83B-66408C343F40"_cry_guid)

	virtual ~CFireNetCorePlugin();
public:
	// ICryPlugin
	const char*      GetName() const override { return "CryFireNetCore"; }
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
}; 