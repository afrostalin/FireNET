// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "netpacket.h"
#include "global.h"
#include "settings.h"

/* Simple packet
* // Create packet
* NetPacket packet(net_query);
* packet.WriteInt(net_query_auth);
* packet.WriteString("login");
* packet.WriteSrting("password");
* // Read packet
* NetPacket packet("recived_packet_data");
* packet.getType();
* packet.ReadInt();
* packet.ReadString();
* packet.ReadString();
*/

NetPacket::NetPacket(ENetPacketType type)
{
	m_data = "";
	m_separator = "|";
	m_type = net_Empty;
	m_MagicHeader = "";
	m_MagicFooter = "";

	// Only for reading
	bInitFromData = false;
	bIsGoodPacket = false;
	lastIndex = 0;
	//

	SetMagicHeader();
	SetPacketType(type);
}

NetPacket::NetPacket(const char * data)
{
	if (data)
	{
		m_data = data;
		m_type = net_Empty;
		m_separator = "|";
		m_MagicHeader = "";
		m_MagicFooter = "";

		bInitFromData = true;
		bIsGoodPacket = false;
		lastIndex = 0;

		ReadPacket();
	}
	else
	{
		qCritical() << "Empty packet!";
		m_data = "";
		m_type = net_Empty;
		m_separator = "";
	}
}

void NetPacket::WriteString(std::string value)
{
	m_data = m_data + value + m_separator;
}

void NetPacket::WriteInt(int value)
{
	m_data = m_data + std::to_string(value) + m_separator;
}

void NetPacket::WriteBool(bool value)
{
	std::string m_value;
	value ? m_value = "true" : m_value = "false";

	m_data = m_data + m_value + m_separator;
}

void NetPacket::WriteFloat(float value)
{
	m_data = m_data + std::to_string(value) + m_separator;
}

void NetPacket::WriteDouble(double value)
{
	m_data = m_data + std::to_string(value) + m_separator;
}

const char* NetPacket::ReadString()
{
	if (bInitFromData && bIsGoodPacket)
	{
		if (m_packet.size() - 1 > lastIndex)
		{
			lastIndex++;
			return m_packet.at(lastIndex - 1).c_str();
		}
		else
		{
			qCritical() << "Error reading string from packet. Last index wrong";
			return nullptr;
		}
	}
	else
	{
		qCritical() << "Error reading string from packet. Bad packet.";
		return nullptr;
	}
}

int NetPacket::ReadInt()
{
	if (bInitFromData && bIsGoodPacket)
	{
		if (m_packet.size() - 1 > lastIndex)
		{
			lastIndex++;

			try
			{
				return std::stoi(m_packet.at(lastIndex - 1));
			}
			catch (std::invalid_argument &)
			{
				qCritical() << "Error reading int from packet. Can't convert string to int";
				return 0;
			}
		}
		else
		{
			qCritical() << "Error reading int from packet. Last index wrong";
			return 0;
		}
	}
	else
	{
		qCritical() << "Error reading int from packet. Bad packet.";
		return 0;
	}
}

bool NetPacket::ReadBool()
{
	std::string m_value = ReadString();
	bool result;

	m_value == "true" ? result = true : result = false;
	return result;
}

float NetPacket::ReadFloat()
{
	if (bInitFromData && bIsGoodPacket)
	{
		if (m_packet.size() - 1 > lastIndex)
		{
			lastIndex++;

			try
			{
				return std::stof(m_packet.at(lastIndex - 1));
			}
			catch (std::invalid_argument &)
			{
				qCritical() << "Error reading float from packet. Can't convert string to float";
				return 0.0f;
			}
		}
		else
		{
			qCritical() << "Error reading float from packet. Last index wrong";
			return 0.0f;
		}
	}
	else
	{
		qCritical() << "Error reading float from packet. Bad packet.";
		return 0.0f;
	}
}

double NetPacket::ReadDouble()
{
	if (bInitFromData && bIsGoodPacket)
	{
		if (m_packet.size() - 1 > lastIndex)
		{
			lastIndex++;

			try
			{
				return std::stod(m_packet.at(lastIndex - 1));
			}
			catch (std::invalid_argument &)
			{
				qCritical() << "Error reading double from packet. Can't convert string to double";
				return 0.0;
			}
		}
		else
		{
			qCritical() << "Error reading double from packet. Last index wrong";
			return 0.0;
		}
	}
	else
	{
		qCritical() << "Error reading double from packet. Bad packet.";
		return 0.0;
	}
}

const char* NetPacket::toString()
{
	if (!bInitFromData)
	{
		SetMagicFooter();
		return m_data.c_str();
	}
	else
		return m_data.c_str();
}

ENetPacketType NetPacket::getType()
{
	return m_type;
}

int NetPacket::size()
{
	return sizeof(m_data);
}

void NetPacket::SetMagicHeader()
{
	int m_MagicValue = gEnv->pSettings->GetVariable("net_magic_key").toInt();
	char m_MagicKey[10] = "";
	itoa(m_MagicValue, m_MagicKey, 16);
	m_MagicHeader = "0x" + std::string(m_MagicKey);

	WriteString("!" + m_MagicHeader);
}

void NetPacket::SetPacketType(ENetPacketType type)
{
	WriteInt(type);
}

void NetPacket::SetMagicFooter()
{
	int m_MagicValue = gEnv->pSettings->GetVariable("net_magic_key").toInt();
	char m_MagicKey[10] = "";
	// Abracadabra
	itoa((m_MagicValue * 2.5) / 0.7 + 1945, m_MagicKey, 16);
	m_MagicFooter = "0x" + std::string(m_MagicKey);

	m_data = m_data + m_MagicFooter + "!";
}

void NetPacket::ReadPacket()
{
	if (!m_data.empty())
	{
		m_packet = Split(m_data, m_separator.at(0));

		if (m_packet.size() >= 3)
		{
			std::string packet_header = m_packet.at(0);
			std::string packet_type = m_packet.at(1);
			std::string packet_footer = m_packet.at(m_packet.size() - 1);

			if (packet_header == "!" + m_MagicHeader && packet_footer == m_MagicFooter + "!")
			{
				m_type = (ENetPacketType)std::stoi(packet_type);

				if (m_type > 0)
				{
					bIsGoodPacket = true;
					lastIndex = 2; // 0 - header, 1 - type, 2 - start data
				}
				else
				{
					qCritical() << "Error reading packet. Empty packet type";
					bIsGoodPacket = false;
				}
			}
			else
			{
				qCritical() << "Error reading packet. Wrong magic key!";
				bIsGoodPacket = false;
			}
		}
		else
		{
			qCritical() << "Error reading packet. Packet soo small!";
			bIsGoodPacket = false;
		}
	}
	else
	{
		qCritical() << "Error reading packet. Packet empty!";
		bIsGoodPacket = false;
	}
}

std::vector<std::string> NetPacket::Split(const std::string & s, char delim)
{
	std::stringstream ss;
	ss.str(s);
	std::string item;
	std::vector<std::string> m_vector;

	while (std::getline(ss, item, delim)) 
	{
		m_vector.push_back(item);
	}

	return m_vector;
}
