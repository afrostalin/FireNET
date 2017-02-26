// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>

enum class EFireNetUdpPacketType : int
{
	Empty,
	Ask,
	Ping,
	Request,
};

enum class EFireNetUdpAsk : int
{
	ConnectToServer,
	ChangeTeam,
};

enum class EFireNetUdpRequest : int
{
	Spawn,
	Movement,
	Action,
};

// Max UDP packet size
enum class EFireNetUdpPackeMaxSize : int { SIZE = 512 };

enum class EFireNetUdpServerError : int 
{ 
	None,
	ServerFull,
	PlayerBanned,
	ServerBlockNewConnection
};

class IFireNetUdpPacket
{
public:
	virtual ~IFireNetUdpPacket() {}
public:
	virtual void                       WriteAsk(EFireNetUdpAsk ask) { WriteInt(static_cast<int>(ask)); }
	virtual void                       WriteRequest(EFireNetUdpRequest request) { WriteInt(static_cast<int>(request)); }
public:
	virtual EFireNetUdpAsk             ReadAsk() { return (EFireNetUdpAsk)ReadInt(); }
	virtual EFireNetUdpRequest         ReadRequest() { return (EFireNetUdpRequest)ReadInt(); }
public:
	virtual void                       WriteString(const std::string &value) = 0;
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
	EFireNetUdpPacketType              getType() { return m_Type; }
	int                                getPacketNumber() { return m_PacketNumber; }
protected:
	void                               WritePacketType(EFireNetUdpPacketType type) { WriteInt(static_cast<int>(type)); }
	void                               WriteHeader() { WriteString(m_Header); }
	void                               WriteFooter() { m_Data = m_Data + m_Footer; };
protected:
	virtual void                       GenerateSession() = 0;
	virtual void                       ReadPacket() = 0;
	virtual void                       EncryptPacket() = 0;
	virtual void                       DecryptPacket() = 0;
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
	EFireNetUdpPacketType              m_Type;

	// Only for reading
	bool                               bInitFromData;
	bool                               bIsGoodPacket;
	int                                m_LastIndex;
	int                                m_PacketNumber;
};