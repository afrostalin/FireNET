// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "netpacket.h"
#include "global.h"

#include "Tools/settings.h"

NetPacket::NetPacket(ENetPacketType type)
{
	m_separator = "|";
	m_type = net_Empty;

	// Only for reading
	bInitFromData = false;
	bIsGoodPacket = false;
	lastIndex = 0;

	// First generate magic keys
	GenerateMagic();
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

		bInitFromData = true;
		bIsGoodPacket = false;
		lastIndex = 0;

		GenerateMagic();
		ReadPacket();
	}
	else
	{
		qCritical() << "Empty packet!";
		m_type = net_Empty;
	}
}

NetPacket::~NetPacket()
{
	qDebug() << "~NetPacket";

	m_packet.clear();
	m_data.clear();
	m_separator.clear();
	m_MagicHeader.clear();
	m_MagicFooter.clear();
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

		if (gEnv->pSettings->GetVariable("bUsePacketDebug").toBool())
		{
			qDebug() << "Packet : " << m_data.c_str();
		}

		return m_data.c_str();
	}
	else
		return m_data.c_str();
}

ENetPacketType NetPacket::getType()
{
	return m_type;
}

void NetPacket::SetMagicHeader()
{
	WriteString(m_MagicHeader);
}

void NetPacket::SetPacketType(ENetPacketType type)
{
	WriteInt(type);
}

void NetPacket::SetMagicFooter()
{
	m_data = m_data + m_MagicFooter;
}

void NetPacket::GenerateMagic()
{
	int m_MagicValue = gEnv->pSettings->GetVariable("net_magic_key").toInt();
	char m_MagicKeyH[10] = ""; // Header
	char m_MagicKeyF[10] = ""; // Footer
	itoa(m_MagicValue, m_MagicKeyH, 16);
	itoa((m_MagicValue * 2.5) / 0.7 + 1945, m_MagicKeyF, 16);
	m_MagicHeader = "!0x" + std::string(m_MagicKeyH);
	m_MagicFooter = "0x" + std::string(m_MagicKeyF) + "!";
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

			if (packet_header == m_MagicHeader && packet_footer == m_MagicFooter)
			{
				m_type = (ENetPacketType)std::stoi(packet_type);

				if (m_type > 0)
				{
					if (gEnv->pSettings->GetVariable("bUsePacketDebug").toBool())
					{
						qDebug() << "Packet : " << m_data.c_str();
					}

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
				qDebug() << "Header" << packet_header.c_str();
				qDebug() << "Footer" << packet_footer.c_str();
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
