// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include "IFireNetBase.h"
#include "IFireNetListener.h"

#include "IFireNetTcpPacket.h"
#include "IFireNetUdpPacket.h"

struct IFireNetEnv;
class CTcpPacket;

//! Base interface for using in FireNet-Core plugin
struct IFireNetCore
{
	virtual ~IFireNetCore() {}

	//! Register FireNet listener for work with UI or somethings else
	virtual void RegisterFireNetListener(IFireNetListener *listener) = 0;

	//! Connect to master server
	virtual void ConnectToMasterServer() = 0;

	//! Send authorization request to master server
	virtual void Authorization(const std::string &login, const std::string &password) = 0;

	//! Send register request to master server
	virtual void Registration(const std::string &login, const std::string &password) = 0;

	//! Send create profile request to master server
	virtual void CreateProfile(const std::string &nickname, const std::string &character) = 0;

	//! Send get profile request to master server
	virtual void GetProfile(int uid = 0) = 0;

	//! Get local profile
	virtual SFireNetProfile* GetLocalProfile() = 0;

	//! Send get shop request to master server
	virtual void GetShop() = 0;

	//! Send buy item request to master server
	virtual void BuyItem(const std::string &item) = 0;

	//! Send remove item request to master server
	virtual void RemoveItem(const std::string &item) = 0;

	//! Send invite to other player
	virtual void SendInvite(EFireNetInviteType type, int uid) = 0;

	//! Decline invite from other player
	virtual void DeclineInvite() = 0;

	//! Accept invite from other player
	virtual void AcceptInvite() = 0;

	//! Send remove friend request to master server
	virtual void RemoveFriend(int uid) = 0;

	//! Send chat message
	virtual void SendChatMessage(EFireNetChatMsgType type, int uid = 0) = 0;

	//! Send get game server request to master server
	virtual void GetGameServer(const std::string &map, const std::string &gamerules) = 0;

	//! Send some raw request to master server. For e.g. this may be login or register request
	//! You need add to you project TcpPacket.h for using this function
	virtual void SendRawRequestToMasterServer(CTcpPacket &packet) = 0;

	//! Return connection status
	virtual bool IsConnected() = 0;

	//! Send FireNet event
	virtual void SendFireNetEvent(EFireNetEvents event, SFireNetEventArgs& args = SFireNetEventArgs()) = 0;

	//! Normaly shutdown plugin
	virtual bool Quit() = 0;
};