// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

// Item types for shop/loadout items
enum EFireNetItemType
{
	EItem_None,
	EItem_Weapon,
	EItem_Cloth,
	EItem_Booster,
	EItem_Other,
};

// Invite types
enum EFireNetInviteType
{
	EInvite_AddToFriends,
	EInvite_PlayInGame,
	EInvite_AddToClan,
};

// Chat message types
enum EFireNetChatMsgType
{
	EMsg_GlobalChat,
	EMsg_PrivateChat,
	EMsg_ClanChat,
};

// Player status
enum EFireNetPlayerStatus
{
	EStatus_Online,
	EStatus_InGame,
	EStatus_InRoom,
	EStatus_AFK,
	EStatus_Offline,
};

// Player status
enum EFireNetGameServerStatus
{
	EGStatus_Empty = 0,
	EGStatus_PreparingToPlay,
	EGStatus_GameStarted,
	EGStatus_GameFinished,
	EGStatus_Reloading,
	EGStatus_WaitingPlayers,
	EGStatus_Unknown,
};

// Item structure
struct SFireNetItem
{
	SFireNetItem()
		: name(nullptr)
		, type(EItem_None)
		, cost(0)
		, minLvlForBuy(0)
	{}

	std::string name;
	EFireNetItemType type;
	int cost;
	int minLvlForBuy;
};

#ifndef STEAM_SDK_ENABLED
// Friend stucture
struct SFireNetFriend
{
	SFireNetFriend()
		: uid(0)
		, name(0)
		, status(EStatus_Offline)
	{}
	int uid;
	std::string name;
	EFireNetPlayerStatus status;
};
#endif

// Player statistic
struct SFireNetStats
{
	SFireNetStats()
		: kills(0)
		, deaths(0)
		, kd(0.0)
	{}
	int kills;
	int deaths;
	double kd;
};

// Player profile
struct SFireNetProfile
{
	SFireNetProfile()
		: uid(0)
		, nickname(nullptr)
		, fileModel(nullptr)
		, lvl(0)
		, xp(0)
		, money(0)
	{}

	int uid;
	std::string nickname;
	std::string fileModel;
	int lvl;
	int xp;
	int money;
	std::vector<SFireNetItem> items;
#ifndef STEAM_SDK_ENABLED
	std::vector<SFireNetFriend> friends;
#endif
};

// Game server structure
struct SFireNetGameServer
{
	SFireNetGameServer()
		: name(nullptr)
		, ip(nullptr)
		, port(0)
		, map(nullptr)
		, gamerules(nullptr)
		, online(0)
		, maxPlayers(0)
		, status(EGStatus_Unknown)
		, currentPID(0)
	{}

	virtual ~SFireNetGameServer() {};

	bool IsValid()
	{
		return name && ip && port && map && gamerules && maxPlayers;
	}

	const char* name;
	const char* ip;
	int port;
	const char* map;
	const char* gamerules;
	int online;
	int maxPlayers;
	EFireNetGameServerStatus status;
	int currentPID;
};

enum class EFireNetCoreErrorCodes : int
{
	ConnectionTimeout,
	ConnectionRefused,
	ConnectionLost,
	CantSpawnThread,
	CantRemoveThread,
	UnknownError,
	WrongCVars,
};