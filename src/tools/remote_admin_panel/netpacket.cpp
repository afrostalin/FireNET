// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "Core/tcppacket.h"


CTcpPacket::CTcpPacket(const EFireNetTcpPacketType type)
{
	m_Separator = '|';
	m_Type = type;

	// Only for reading
	bInitFromData = false;
	bIsGoodPacket = false;
	m_LastIndex = 0;

	CTcpPacket::GenerateSession();
	WriteHeader();
	WritePacketType(type);
}

CTcpPacket::CTcpPacket(const char * data)
{
	if (data)
	{
		m_Data = data;
		m_Type = EFireNetTcpPacketType::Empty;
		m_Separator = '|';

		bInitFromData = true;
		bIsGoodPacket = false;
		m_LastIndex = 0;

		CTcpPacket::GenerateSession();
		CTcpPacket::ReadPacket();
	}
	else
	{
		LogWarning("Empty TCP packet!");
		m_Type = EFireNetTcpPacketType::Empty;
	}
}

void CTcpPacket::WriteStdString(const std::string & value)
{
	m_Data = m_Data + value + m_Separator;
}

void CTcpPacket::WriteString(const char * value)
{
	m_Data = m_Data + value + m_Separator;
}

void CTcpPacket::WriteInt(const int value)
{
	m_Data = m_Data + std::to_string(value) + m_Separator;
}

void CTcpPacket::WriteBool(const bool value)
{
	const int m_value = value ? 1 : 0;
	m_Data = m_Data + std::to_string(m_value) + m_Separator;
}

void CTcpPacket::WriteFloat(const float value)
{
	m_Data = m_Data + std::to_string(value) + m_Separator;
}

void CTcpPacket::WriteDouble(const double value)
{
	m_Data = m_Data + std::to_string(value) + m_Separator;
}

const char * CTcpPacket::ReadString()
{
	if (bInitFromData && bIsGoodPacket)
	{
		if (m_Packet.size() - 1 > m_LastIndex)
		{
			m_LastIndex++;
			return m_Packet.at(m_LastIndex - 1).c_str();
		}

		LogWarning("Error reading string from TCP packet. Last index wrong");
		return nullptr;
	}

	LogWarning("Error reading string from TCP packet. Bad packet.");
	return nullptr;
}

int CTcpPacket::ReadInt()
{
	if (bInitFromData && bIsGoodPacket)
	{
		if (m_Packet.size() - 1 > m_LastIndex)
		{
			m_LastIndex++;

			try
			{
				return stoi(m_Packet.at(m_LastIndex - 1));
			}
			catch (std::invalid_argument &)
			{
				LogWarning("Error reading int from TCP packet. Can't convert string to int");
				return 0;
			}
		}

		LogWarning("Error reading int from TCP packet. Last index wrong");
		return 0;
	}

	LogWarning("Error reading int from TCP packet. Bad packet.");
	return 0;
}

bool CTcpPacket::ReadBool()
{
	return ReadInt() == 1 ? true : false;
}

float CTcpPacket::ReadFloat()
{
	if (bInitFromData && bIsGoodPacket)
	{
		if (m_Packet.size() - 1 > m_LastIndex)
		{
			m_LastIndex++;

			try
			{
				return stof(m_Packet.at(m_LastIndex - 1));
			}
			catch (std::invalid_argument &)
			{
				LogWarning("Error reading float from TCP packet. Can't convert string to float");
				return 0.0f;
			}
		}

		LogWarning("Error reading float from TCP packet. Last index wrong");
		return 0.0f;
	}

	LogWarning("Error reading float from TCP packet. Bad packet.");
	return 0.0f;
}

double CTcpPacket::ReadDouble()
{
	if (bInitFromData && bIsGoodPacket)
	{
		if (m_Packet.size() - 1 > m_LastIndex)
		{
			m_LastIndex++;

			try
			{
				return stod(m_Packet.at(m_LastIndex - 1));
			}
			catch (std::invalid_argument &)
			{
				LogWarning("Error reading double from TCP packet. Can't convert string to double");
				return 0.0;
			}
		}

		LogWarning("Error reading double from TCP packet. Last index wrong");
		return 0.0;
	}

	LogWarning("Error reading double from packet. Bad packet.");
	return 0.0;
}

const char * CTcpPacket::toString()
{
	if (!bInitFromData)
	{
		WriteFooter();

		// Debugging packet
		if (gEnv->bUsePacketDebug)
		{
			LogDebug("TCP packet data : <%s>", m_Data.c_str());
			LogDebug("TCP packet size : <%d>", getLength());
		}

		return m_Data.c_str();
	}

	return m_Data.c_str();
}

void CTcpPacket::GenerateSession()
{
	const std::string networkVersion = gEnv->net_version.toStdString().c_str();

	m_Header = "!0x" + networkVersion;
	m_Footer = "0x0!";
}

void CTcpPacket::ReadPacket()
{
	if (!m_Data.empty())
	{
		m_Packet = Split(m_Data, m_Separator);

		// Debugging packet
		if (gEnv->bUsePacketDebug)
		{
			LogDebug("TCP packet data : <%s>", m_Data.c_str());
			LogDebug("TCP packet size : <%d>", getLength());
		}

		if (m_Packet.size() >= 3)
		{
			const std::string packet_header = m_Packet.at(0);
			const std::string packet_type = m_Packet.at(1);
			const std::string packet_footer = m_Packet.at(m_Packet.size() - 1);

			if (packet_header == m_Header && packet_footer == m_Footer)
			{
				m_Type = static_cast<EFireNetTcpPacketType>(stoi(packet_type));

				if (m_Type != EFireNetTcpPacketType::Empty)
				{
					bIsGoodPacket = true;
					m_LastIndex = 2; // 0 - header, 1 - type, 2 - start data
				}
				else
				{
					LogWarning("Error reading TCP packet. Empty packet type");
					bIsGoodPacket = false;
				}
			}
			else
			{
				LogWarning("Error reading TCP packet. Wrong session key!");
				bIsGoodPacket = false;
			}
		}
		else
		{
			LogWarning("Error reading TCP packet. Packet soo small!");
			bIsGoodPacket = false;
		}
	}
	else
	{
		LogWarning("Error reading TCP packet. Packet empty!");
		bIsGoodPacket = false;
	}
}