// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

// FireNet events
enum EFireNetEvents
{
	//! Event when start connection to FireNet
	FIRENET_EVENT_MASTER_SERVER_START_CONNECTION,
    //! Event when FireNet success connected
	FIRENET_EVENT_MASTER_SERVER_CONNECTED,
	//! Event when connection to FireNet failed
	FIRENET_EVENT_MASTER_SERVER_CONNECTION_ERROR,
	//! Event when connection with FireNet lost
	FIRENET_EVENT_MASTER_SERVER_DISCONNECTED,

	// Special events

	//! Event when authorization complete
	FIRENET_EVENT_AUTHORIZATION_COMPLETE,
	//! Event when authorization complete and profile exist
	FIRENET_EVENT_AUTHORIZATION_COMPLETE_WITH_PROFILE,
	//! Event when authorization failed
	FIRENET_EVENT_AUTHORIZATION_FAILED,
	//! Event when registration complete
	FIRENET_EVENT_REGISTRATION_COMPLETE,
	//! Event when registration failed
	FIRENET_EVENT_REGISTRATION_FAILED,
	//! Event when client update profile (with args)
	FIRENET_EVENT_UPDATE_PROFILE,
	//! Event when create profile complete
	FIRENET_EVENT_CREATE_PROFILE_COMPLETE,
	//! Event when create profile failed
	FIRENET_EVENT_CREATE_PROFILE_FAILED,
	//! Event when get profile complete
	FIRENET_EVENT_GET_PROFILE_COMPLETE,
	//! Event when get profile failed
	FIRENET_EVENT_GET_PROFILE_FAILED,
	//! Event when get shop complete
	FIRENET_EVENT_GET_SHOP_COMPLETE,
	//! Event when get shop failed
	FIRENET_EVENT_GET_SHOP_FAILED,
	//! Event when buy item complete
	FIRENET_EVENT_BUY_ITEM_COMPLETE,
	//! Event when buy item failed
	FIRENET_EVENT_BUY_ITEM_FAILED,
	//! Event when remove item complete
	FIRENET_EVENT_REMOVE_ITEM_COMPLETE,
	//! Event when remove item failed
	FIRENET_EVENT_REMOVE_ITEM_FAILED,
	//! Event when send invite complete
	FIRENET_EVENT_SEND_INVITE_COMPLETE,
	//! Event when send invite failed
	FIRENET_EVENT_SEND_INVITE_FAILED,
	//! Event when decline invite complete
	FIRENET_EVENT_DECLINE_INVITE_COMPLETE,
	//! Event when decline invite failed
	FIRENET_EVENT_DECLINE_INVITE_FAILED,
	//! Event when accept invite complete
	FIRENET_EVENT_ACCEPT_INVITE_COMPLETE,
	//! Event when accept invite failed
	FIRENET_EVENT_ACCEPT_INVITE_FAILED,
	//! Event when remove friend complete
	FIRENET_EVENT_REMOVE_FRIEND_COMPLETE,
	//! Event when remove friend failed
	FIRENET_EVENT_REMOVE_FRIEND_FAILED,	
	//! Event when send chat msg complete
	FIRENET_EVENT_SEND_CHAT_MSG_COMPLETE,
	//! Event when send chat msg failed
	FIRENET_EVENT_SEND_CHAT_MSG_FAILED,
	//! Event when get game server complete
	FIRENET_EVENT_GET_GAME_SERVER_COMPLETE,
	//! Event when get game server failed
	FIRENET_EVENT_GET_GAME_SERVER_FAILED,

	// ~Special events

	//! Event when client received global chat message
	FIRENET_EVENT_GLOBAL_CHAT_MSG_RECEIVED,
	//! Event when client received private chat message
	FIRENET_EVENT_PRIVATE_CHAT_MSG_RECEIVED,
	//! Event when client received clan chat message
	FIRENET_EVENT_CLAN_CHAT_MSG_RECEIVED,
	//! Event when client received console command
	FIRENET_EVENT_CONSOLE_COMMAND_RECEIVED,
	//! Event when client received server message
	FIRENET_EVENT_SERVER_MESSAGE_RECEIVED,
	//! Event when client received game server information
	FIRENET_EVENT_GAME_SERVER_INFO_RECEIVED,

	//! Event when start connection to game server
	FIRENET_EVENT_GAME_SERVER_START_CONNECTION,
	//! Event when success connected to game server
	FIRENET_EVENT_GAME_SERVER_CONNECTED,
	//! Event when connection to game server failed
	FIRENET_EVENT_GAME_SERVER_CONNECTION_ERROR,
	//! Event when connection with game server lost
	FIRENET_EVENT_GAME_SERVER_DISCONNECTED,
};

struct SFireNetEventArgs
{
	std::vector<int> mInt;
	std::vector<const char*> mString;
	std::vector<float> mFloat;

	inline void AddInt(int i)
	{
		mInt.insert(mInt.begin(), i);
	}

	inline void AddString(const char* string)
	{
		mString.insert(mString.begin(), string);
	}

	inline void AddFloat(float f)
	{
		mFloat.insert(mFloat.begin(), f);
	}

	int GetInt()
	{
		auto it = mInt.begin();

		if (it != mInt.end())
		{
			int result = (*it);
			mInt.erase(it);

			return result;
		}

		return 0;
	}

	const char* GetString()
	{
		auto it = mString.begin();

		if (it != mString.end())
		{
			const char* result = (*it);
			mString.erase(it);

			return result;
		}

		return nullptr;
	}

	float GetFloat()
	{
		auto it = mFloat.begin();

		if (it != mFloat.end())
		{
			float result = (*it);
			mFloat.erase(it);

			return result;
		}

		return 0;
	}
};

struct IFireNetListener
{
	virtual void OnFireNetEvent(EFireNetEvents event, SFireNetEventArgs& args = SFireNetEventArgs()) = 0;
};