// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>

#include "IFireNetBase.h"

//! FireNet events
enum EFireNetEvents
{
	FIRENET_EVENT_MASTER_SERVER_START_CONNECTION = 0,
	FIRENET_EVENT_MASTER_SERVER_CONNECTED,
	FIRENET_EVENT_AUTHORIZATION_COMPLETE,
	FIRENET_EVENT_AUTHORIZATION_COMPLETE_WITH_PROFILE,
	FIRENET_EVENT_REGISTRATION_COMPLETE,
	FIRENET_EVENT_CREATE_PROFILE_COMPLETE,
	FIRENET_EVENT_GET_PROFILE_COMPLETE,
	FIRENET_EVENT_GET_SHOP_COMPLETE,
	FIRENET_EVENT_BUY_ITEM_COMPLETE,
	FIRENET_EVENT_REMOVE_ITEM_COMPLETE,
	FIRENET_EVENT_SEND_INVITE_COMPLETE,
	FIRENET_EVENT_DECLINE_INVITE_COMPLETE,
	FIRENET_EVENT_ACCEPT_INVITE_COMPLETE,
	FIRENET_EVENT_REMOVE_FRIEND_COMPLETE,
	FIRENET_EVENT_SEND_CHAT_MSG_COMPLETE,

	// Dedicated only
	FIRENET_EVENT_SERVER_REGISTERED,
	FIRENET_EVENT_SERVER_NEED_RELOAD,
};

struct IFireNetListener
{
	virtual ~IFireNetListener() {}

	virtual void OnEmptyEvent(EFireNetEvents event) = 0;
	virtual void OnConnectionError(EFireNetCoreErrorCodes errorType, const char* errorStr = nullptr) = 0;
	virtual void OnReadyToReconnect() = 0;
	virtual void OnDisconnect(EFireNetCoreErrorCodes reason, const char* errorStr = nullptr) = 0;
	virtual void OnAuthorizationFailed(int errorType, const char* errorStr = nullptr) = 0;
	virtual void OnRegistrationFailed(int errorType, const char* errorStr = nullptr) = 0;
	virtual void OnPlayerProfileUpdated(SFireNetProfile &profile) = 0;
	virtual void OnPlayerProfileCreationFailed(int errorType, const char* errorStr = nullptr) = 0;
	virtual void OnGetPlayerProfileFailed(int errorType, const char* errorStr = nullptr) = 0;
	virtual void OnGetShopItemsFailed(int errorType, const char* errorStr = nullptr) = 0;
	virtual void OnBuyItemFailed(int errorType, const char* errorStr = nullptr) = 0;
	virtual void OnRemoveItemFailed(int errorType, const char* errorStr = nullptr) = 0;
	virtual void OnSendInviteFailed(int errorType, const char* errorStr = nullptr) = 0;
	virtual void OnDeclineInviteFailed(int errorType, const char* errorStr = nullptr) = 0;
	virtual void OnAcceptInviteFailed(int errorType, const char* errorStr = nullptr) = 0;
	virtual void OnRemoveFriendFailed(int errorType, const char* errorStr = nullptr) = 0;
	virtual void OnSendChatMsgFailed(int errorType, const char* errorStr = nullptr) = 0;
	virtual void OnGetGameServerFailed(int errorType, const char* errorStr = nullptr) = 0;
	virtual void OnGlobalChatMsgReceived(const char* from, const char* msg) = 0;
	virtual void OnPrivateChatMsgReceived(const char* from, const char* msg) = 0;
	virtual void OnClanChatMsgReceived(const char* from, const char* msg) = 0;
	virtual void OnConsoleCommandReceived(const char* command) = 0;
	virtual void OnServerMsgReceived(const char* msg) = 0;
	virtual void OnGameServerInfoReceived(SFireNetGameServer &server) = 0; 
	// Dedicated server only
	virtual void OnGameServerRegisterFailed(int errorType, const char* errorStr = nullptr) = 0;
	virtual void OnGameServerStartPrepare(int pvpSession) = 0;
};