// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>

//! FireNet events
enum EFireNetEvents
{
	//! Event when start connection to FireNet
	FIRENET_EVENT_MASTER_SERVER_START_CONNECTION,
    //! Event when FireNet success connected
	FIRENET_EVENT_MASTER_SERVER_CONNECTED,
	//! Event when connection to FireNet failed
	//! Params 1 - reason
	FIRENET_EVENT_MASTER_SERVER_CONNECTION_ERROR,
	//! Event when connection with FireNet lost
	FIRENET_EVENT_MASTER_SERVER_DISCONNECTED,

	// Special events

	//! Event when authorization complete
	FIRENET_EVENT_AUTHORIZATION_COMPLETE,
	//! Event when authorization complete and profile exist
	FIRENET_EVENT_AUTHORIZATION_COMPLETE_WITH_PROFILE,
	//! Event when authorization failed
	//! Params 1 - error type (int), 2 - error string (string)
	FIRENET_EVENT_AUTHORIZATION_FAILED,
	//! Event when registration complete
	FIRENET_EVENT_REGISTRATION_COMPLETE,
	//! Event when registration failed
	//! Params 1 - error type (int), 2 - error string (string)
	FIRENET_EVENT_REGISTRATION_FAILED,
	//! Event when client update profile (with args)
	FIRENET_EVENT_UPDATE_PROFILE,
	//! Event when create profile complete
	FIRENET_EVENT_CREATE_PROFILE_COMPLETE,
	//! Event when create profile failed
	//! Params 1 - error type (int), 2 - error string (string)
	FIRENET_EVENT_CREATE_PROFILE_FAILED,
	//! Event when get profile complete
	FIRENET_EVENT_GET_PROFILE_COMPLETE,
	//! Event when get profile failed
	//! Params 1 - error type (int), 2 - error string (string)
	FIRENET_EVENT_GET_PROFILE_FAILED,
	//! Event when get shop complete
	FIRENET_EVENT_GET_SHOP_COMPLETE,
	//! Event when get shop failed
	//! Params 1 - error type (int), 2 - error string (string)
	FIRENET_EVENT_GET_SHOP_FAILED,
	//! Event when buy item complete
	FIRENET_EVENT_BUY_ITEM_COMPLETE,
	//! Event when buy item failed
	//! Params 1 - error type (int), 2 - error string (string)
	FIRENET_EVENT_BUY_ITEM_FAILED,
	//! Event when remove item complete
	FIRENET_EVENT_REMOVE_ITEM_COMPLETE,
	//! Event when remove item failed
	//! Params 1 - error type (int), 2 - error string (string)
	FIRENET_EVENT_REMOVE_ITEM_FAILED,
	//! Event when send invite complete
	FIRENET_EVENT_SEND_INVITE_COMPLETE,
	//! Event when send invite failed
	//! Params 1 - error type (int), 2 - error string (string)
	FIRENET_EVENT_SEND_INVITE_FAILED,
	//! Event when decline invite complete
	FIRENET_EVENT_DECLINE_INVITE_COMPLETE,
	//! Event when decline invite failed
	//! Params 1 - error type (int), 2 - error string (string)
	FIRENET_EVENT_DECLINE_INVITE_FAILED,
	//! Event when accept invite complete
	FIRENET_EVENT_ACCEPT_INVITE_COMPLETE,
	//! Event when accept invite failed
	//! Params 1 - error type (int), 2 - error string (string)
	FIRENET_EVENT_ACCEPT_INVITE_FAILED,
	//! Event when remove friend complete
	FIRENET_EVENT_REMOVE_FRIEND_COMPLETE,
	//! Event when remove friend failed
	//! Params 1 - error type (int), 2 - error string (string)
	FIRENET_EVENT_REMOVE_FRIEND_FAILED,	
	//! Event when send chat msg complete
	FIRENET_EVENT_SEND_CHAT_MSG_COMPLETE,
	//! Event when send chat msg failed
	//! Params 1 - error type (int), 2 - error string (string)
	FIRENET_EVENT_SEND_CHAT_MSG_FAILED,
	//! Event when get game server complete
	FIRENET_EVENT_GET_GAME_SERVER_COMPLETE,
	//! Event when get game server failed
	//! Params 1 - error type (int), 2 - error string (string)
	FIRENET_EVENT_GET_GAME_SERVER_FAILED,

	// ~Special events

	//! Event when client received global chat message
	//! Params 1 - from (string), 2 - message (string)
	FIRENET_EVENT_GLOBAL_CHAT_MSG_RECEIVED,
	//! Event when client received private chat message
	//! Params 1 - from (string), 2 - message (string)
	FIRENET_EVENT_PRIVATE_CHAT_MSG_RECEIVED,
	//! Event when client received clan chat message
	//! Params 1 - from (string), 2 - message (string)
	FIRENET_EVENT_CLAN_CHAT_MSG_RECEIVED,
	//! Event when client received console command
	//! Params 1 - command (string)
	FIRENET_EVENT_CONSOLE_COMMAND_RECEIVED,
	//! Event when client received server message
	//! Params 1 - message (string)
	FIRENET_EVENT_SERVER_MESSAGE_RECEIVED,
	//! Event when client received game server information
	FIRENET_EVENT_GAME_SERVER_INFO_RECEIVED,

	//! Event when start connection to game server
	FIRENET_EVENT_GAME_SERVER_START_CONNECTION,
	//! Event when success connected to game server
	FIRENET_EVENT_GAME_SERVER_CONNECTED,
	//! Event when connection to game server failed
	//! Params 1 - reason
	FIRENET_EVENT_GAME_SERVER_CONNECTION_ERROR,
	//! Event when connection with game server lost
	FIRENET_EVENT_GAME_SERVER_DISCONNECTED,
};

struct SFireNetEventArgs
{
public:
	//! Write int variable
	inline void AddInt(const int& value)
	{
		m_Data.push_back(std::to_string(value));
	}

	//! Write string variable
	inline void AddString(const char* value)
	{
		m_Data.push_back(value);
	}

	//! Write double variable
	inline void AddDouble(const double& value)
	{
		m_Data.push_back(std::to_string(value));
	}

	//! Write float variable
	inline void AddFloat(const float& value)
	{
		m_Data.push_back(std::to_string(value));
	}
public:
	//! Get int variable
	inline int GetInt()
	{
		m_LastIndex++;

		if (CheckIndex())
		{
			try
			{
				return std::stoi(m_Data.at(m_LastIndex - 1));
			}
			catch (std::invalid_argument &)
			{
				return 0;
			}
		}
		else
			return 0;
	}

	//! Get string variable
	inline const char* GetString()
	{
		m_LastIndex++;

		if (CheckIndex())
		{
			return m_Data.at(m_LastIndex - 1).c_str();
		}
		else
			return nullptr;
	}

	//! Get double variable
	inline double GetDouble()
	{
		m_LastIndex++;

		if (CheckIndex())
		{
			try
			{
				return std::stod(m_Data.at(m_LastIndex - 1));
			}
			catch (std::invalid_argument &)
			{
				return 0.0;
			}
		}
		else
			return 0.0;
	}

	//! Get float variable
	inline float GetFloat()
	{
		m_LastIndex++;

		if (CheckIndex())
		{
			try
			{
				return std::stof(m_Data.at(m_LastIndex - 1));
			}
			catch (std::invalid_argument &)
			{
				return 0.0f;
			}
		}
		else
			return 0.0f;
	}
private:
	std::vector<std::string> m_Data;
	int                      m_LastIndex = 0;
private:
	bool CheckIndex()
	{
		if (m_LastIndex < 0)
			return false;
		else if (m_LastIndex > m_Data.size())
			return false;
		else
			return true;
	}
};

struct IFireNetListener
{
	virtual ~IFireNetListener() {}

	virtual void OnFireNetEvent(EFireNetEvents event, SFireNetEventArgs& args = SFireNetEventArgs()) = 0;
};