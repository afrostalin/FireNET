// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef NETPACKET_H
#define NETPACKET_H

#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>

enum ENetPacketType
{
	net_Empty,
	net_Query,
	net_Result,
	net_Error,
	net_Server,
};

enum ENetPacketQueryType
{
	net_query_auth,
	net_query_register,
	net_query_create_profile,
	net_query_get_profile,
	net_query_get_shop,
	net_query_buy_item,
	net_query_remove_item,
	net_query_send_invite,
	net_query_decline_invite,
	net_query_accept_invite,
	net_query_add_friend,
	net_query_remove_friend,
	net_query_send_chat_msg,
	net_query_get_server,
	// Remote server only
	net_query_remote_admin_login,
	net_query_remote_server_command,
	net_query_remote_register_server,
	net_query_remote_update_server,
	net_query_remote_get_profile,
	net_query_remote_update_profile,
};

enum ENetPacketResultType
{
	net_result_auth_fail,
	net_result_auth_complete,
	net_result_register_fail,
	net_result_register_complete,
	net_result_profile_creation_fail,
	net_result_profile_creation_complete,
	net_result_get_profile_fail,
	net_result_get_profile_complete,
	net_result_get_shop_fail,
	net_result_get_shop_complete,
	net_result_buy_item_fail,
	net_result_buy_item_complete,
	net_result_remove_item_fail,
	net_result_remove_item_complete,
	net_result_send_invite_fail,
	net_result_send_invite_complete,
	net_result_decline_invite_fail,
	net_result_decline_invite_complete,
	net_result_accept_invite_fail,
	net_result_accept_invite_complete,
	net_result_add_friend_fail,
	net_result_add_friend_complete,
	net_result_remove_friend_fail,
	net_result_remove_friend_complete,
	net_result_send_chat_msg_fail,
	net_result_send_chat_msg_complete,
	net_result_get_server_fail,
	net_result_get_server_complete,
	// Remote server only
	net_result_remote_admin_login_fail,
	net_result_remote_admin_login_complete,
	net_result_remote_command_fail,
	net_result_remote_command_complete,
	net_result_remote_register_server_fail,
	net_result_remote_register_server_complete,
	net_result_remote_update_server_fail,
	net_result_remote_update_server_complete,
	net_result_remote_get_profile_fail,
	net_result_remote_get_profile_complete,
	net_result_remote_update_profile_fail,
	net_result_remote_update_profile_complete,
};

enum ENetPacketServerType
{
	net_server_global_chat_msg,
	net_server_private_chat_msg,
	net_server_message,
	net_server_command,
	net_server_register_game_server,
};

class NetPacket
{
public:
	NetPacket::NetPacket(ENetPacketType type);
	NetPacket::NetPacket(const char* data);
public:
	void WriteString(std::string value);
	void WriteInt(int value);
	void WriteBool(bool value);
	void WriteFloat(float value);
	void WriteDouble(double value);
public:
	const char* ReadString();
	int ReadInt();
	bool ReadBool();
	float ReadFloat();
	double ReadDouble();
public:
	const char* toString();
	ENetPacketType getType();
	int size();
private:
	void SetMagicHeader();
	void SetPacketType(ENetPacketType type);
	void SetMagicFooter();
private:
	void ReadPacket();
	std::vector<std::string> Split(const std::string &s, char delim);
private:
	std::string m_data;
	std::string m_separator;
	std::string m_MagicHeader;
	std::string m_MagicFooter;

	std::vector<std::string> m_packet;

	ENetPacketType m_type;

	// Only for reading
	bool bInitFromData;
	bool bIsGoodPacket;
	int lastIndex;
};

#endif // NETPACKET_H