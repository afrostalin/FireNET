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