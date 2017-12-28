// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>

enum class EFireNetTcpPacketType : int
{
	Empty,
	Query,
	Result,
	Error,
	ServerMessage,
};

enum class EFireNetTcpQuery : int
{
	Login,
	Register,
	CreateProfile,
	GetProfile,
	GetShop,
	BuyItem,
	RemoveItem,
	SendInvite,
	DeclineInvite,
	AcceptInvite,
	RemoveFriend,
	GetServer,
	SendChatMsg,
	// Remote client (Remote admin / Dedicated game server)
	AdminLogin,
	AdminCommand,
	RegisterServer,
	UpdateServer,
	UpdateProfile,
	StartServerPrepare,
	RequestServerReload,
	PingPong,
	// Dedicated arbitrator
	RegisterArbitrator,
	UpdateArbitrator,
	RunGameServersPool,
	RunStandaloneGameServer,
	KillServer,
	KillAllServers,
	RestartGameServersPool,
};

enum class EFireNetTcpResult : int
{
	LoginComplete,
	LoginCompleteWithProfile,
	RegisterComplete,
	ProfileCreationComplete,
	GetProfileComplete,
	GetShopComplete,
	BuyItemComplete,
	RemoveItemComplete,
	SendInviteComplete,
	DeclineInviteComplete,
	AcceptInviteComplete,
	RemoveFriendComplete,
	SendChatMsgComplete,
	GetServerComplete,
	// Remote client (Remote admin / Dedicated game server)
	AdminLoginComplete,
	AdminCommandComplete,
	RegisterServerComplete,
	UpdateServerComplete,
	UpdateProfileComplete,
	// Dedicated arbitrator
	RegisterArbitratorComplete,
	UpdateArbitratorComplete,
};

enum class EFireNetTcpError : int
{
	LoginFail,
	RegisterFail,
	ProfileCreationFail,
	GetProfileFail,
	GetShopFail,
	BuyItemFail,
	RemoveItemFail,
	SendInviteFail,
	DeclineInviteFail,
	AcceptInviteFail,
	RemoveFriendFail,
	SendChatMsgFail,
	GetServerFail,
	// Remote client (Remote admin / Dedicated game server)
	AdminLoginFail,
	AdminCommandFail,
	RegisterServerFail,
	UpdateServerFail,
	UpdateProfileFail,
	// Dedicated arbitrator
	RegisterArbitratorFail,
	UpdateArbitratorFail,
};

enum class EFireNetTcpErrorCode : int
{
	// Client querys - Account
	LoginNotFound,
	AccountBlocked,
	IncorrectPassword,
	DoubleAuthorization,
	LoginAlredyRegistered,
	CantCreateAccount,
	DoubleRegistration,
	ProfileNotFound,
	// Client querys - Profile
	ProfileAlredyCreated,
	NicknameAlredyRegistered,
	CantCreateProfile,
	DoubleProfileCreation,
	CantGetProfile,
	CantUpdateProfile,
	// Client querys - Shop 
	CantGetShopItems,
	ItemAlredyPurchased,
	PlayerLevelBlock,
	InsufficientMoneyForBuy,
	ItemNotFoundInShop,
	ItemNotFoundInInventory,
	// Client querys - Friends 
	UserNotFound,
	UserNotOnline,
	FriendAlredyExist,
	CantAddYourseldToFriend,
	CantSendMessageYourself,
	// Client querys - Matchmaking
	NoAnyOnlineServers,
	GameServerNotFound,

	// Remote client querys - Admin login
	AdminLoginNotFound,
	AdminIncorrectPassword,
	AdminAlredyLogined,
	AdminCommandNotFound,

	// Remote client querys - Game server functionality
	GameServerNotFoundInTrustedList,
	GameServerAlredyRegistered,
	GameServerNotRegistered,

	// Remote client querys - Dedicated arbitrator
	ArbitratorFunctionalityDisabled,
	ArbitratorDoubleRegistration,
	ArbitratorNotRegistered,
	ArbitratorBlockedRegister,
};

// Only server to client
enum class EFireNetTcpSMessage : int
{
	GlobalChatMsg,
	PrivateChatMsg,
	ClanChatMsg,
	ServerMessage,
	ServerCommand,
};

// Remote admin panel commands
enum class EFireNetAdminCommands : int
{
	CMD_Status,
	CMD_SendGlobalMessage,
	CMD_SendGlobalCommand,
	CMD_SendRemoteMessage,
	CMD_SendRemoteCommand, 
	CMD_GetPlayersList,
	CMD_GetGameServersList,
	CMD_RawMasterServerCommand,
};

class IFireNetTcpPacket
{
public:
	virtual ~IFireNetTcpPacket() {}
public:
	virtual void                       WriteQuery(EFireNetTcpQuery query) { WriteInt(static_cast<int>(query)); }
	virtual void                       WriteResult(EFireNetTcpResult result) { WriteInt(static_cast<int>(result)); }
	virtual void                       WriteError(EFireNetTcpError error) { WriteInt(static_cast<int>(error)); }
	virtual void                       WriteErrorCode(EFireNetTcpErrorCode errorCode) { WriteInt(static_cast<int>(errorCode)); }
	virtual void                       WriteServerMessage(EFireNetTcpSMessage msg) { WriteInt(static_cast<int>(msg)); }
public:
	virtual void                       WriteArray(std::vector<std::string> &data)
	{
		WriteInt(static_cast<int>(data.size()));

		for (const auto &it : data)
		{
			WriteStdString(it);
		}
	}
public:
	virtual EFireNetTcpQuery           ReadQuery() { return static_cast<EFireNetTcpQuery>(ReadInt()); }
	virtual EFireNetTcpResult          ReadResult() { return static_cast<EFireNetTcpResult>(ReadInt()); }
	virtual EFireNetTcpError           ReadError() { return static_cast<EFireNetTcpError>(ReadInt()); }
	virtual EFireNetTcpErrorCode       ReadErrorCode() { return static_cast<EFireNetTcpErrorCode>(ReadInt()); }
	virtual EFireNetTcpSMessage        ReadSMessage() { return static_cast<EFireNetTcpSMessage>(ReadInt()); }
public:
	virtual std::vector<std::string>   ReadArray()
	{
		int size = ReadInt();

		std::vector<std::string> m_Array;

		for (int i = 0; i < size; i++)
		{
			m_Array.push_back(ReadString());
		}

		return m_Array;
	}
public:
	virtual void                       WriteStdString(const std::string &value) = 0;
	virtual void                       WriteString(const char* value) = 0;
	virtual void                       WriteInt(int value) = 0;
	virtual void                       WriteBool(bool value) = 0;
	virtual void                       WriteFloat(float value) = 0;
	virtual void                       WriteDouble(double value) = 0;
public:
	virtual const char*                ReadString() = 0;
	virtual int                        ReadInt() = 0;
	virtual bool                       ReadBool() = 0;
	virtual float                      ReadFloat() = 0;
	virtual double                     ReadDouble() = 0;
public: 
	virtual const char*                toString() = 0;
	virtual std::size_t                getLength() { return strlen(m_Data.c_str()); }
public:
	EFireNetTcpPacketType              getType() { return m_Type; }
protected:
	void                               WritePacketType(EFireNetTcpPacketType type) { WriteInt(static_cast<int>(type)); }
	void                               WriteHeader() { WriteStdString(m_Header); }
	void                               WriteFooter() { m_Data = m_Data + m_Footer; };
protected:
	virtual void                       GenerateSession() = 0;
	virtual void                       ReadPacket() = 0;
protected:
	std::vector<std::string>           Split(const std::string &s, char separator)
	{
		std::stringstream ss;
		ss.str(s);
		std::string item;
		std::vector<std::string> m_vector;

		while (std::getline(ss, item, separator))
			m_vector.push_back(item);

		return m_vector;
	}
protected:
	std::string                        m_Data;
	char                               m_Separator;
	std::string                        m_Header;
	std::string                        m_Footer;

	std::vector<std::string>           m_Packet;
	EFireNetTcpPacketType              m_Type;

	// Only for reading
	bool                               bInitFromData;
	bool                               bIsGoodPacket;
	int                                m_LastIndex;
}; 

static const char* ErrorCodeToString(EFireNetTcpErrorCode code)
{
	static const char* const s_errorCodes[] =
	{
		"LoginNotFound",
		"AccountBlocked",
		"IncorrectPassword",
		"DoubleAuthorization",
		"LoginAlredyRegistered",
		"CantCreateAccount",
		"DoubleRegistration",
		"ProfileNotFound",

		"ProfileAlredyCreated",
		"NicknameAlredyRegistered",
		"CantCreateProfile",
		"DoubleProfileCreation",
		"CantGetProfile",
		"CantUpdateProfile",

		"CantGetShopItems",
		"ItemAlredyPurchased",
		"PlayerLevelBlock",
		"InsufficientMoneyForBuy",
		"ItemNotFoundInShop",
		"ItemNotFoundInInventory",

		"UserNotFound",
		"UserNotOnline",
		"FriendAlredyExist",
		"CantAddYourseldToFriend",
		"CantSendMessageYourself",

		"NoAnyOnlineServers",
		"GameServerNotFound",

		"AdminLoginNotFound",
		"AdminIncorrectPassword",
		"AdminAlredyLogined",
		"AdminCommandNotFound",

		"GameServerNotFoundInTrustedList",
		"GameServerAlredyRegistered",
		"GameServerNotRegistered",

		"ArbitratorFunctionalityDisabled",
		"ArbitratorDoubleRegistration",
		"ArbitratorNotRegistered",
		"ArbitratorBlockedRegister",
	};

	try
	{
		return s_errorCodes[static_cast<int>(code)];
	}
	catch (const std::exception&)
	{
		return nullptr;
	}
}