// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "TcpPacket.h"

CTcpPacket::CTcpPacket(EFireNetTcpPacketType type, bool waitable)
{
	m_Separator = '|';
	m_Type = type;
	m_isWaitable = waitable;

	// Only for reading
	bInitFromData = false;
	bIsGoodPacket = false;
	m_LastIndex = 0;

	GenerateSession();
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

		GenerateSession();
		ReadPacket();
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Empty TCP packet!");
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

void CTcpPacket::WriteInt(int value)
{
	m_Data = m_Data + std::to_string(value) + m_Separator;
}

void CTcpPacket::WriteBool(bool value)
{
	int m_value = value ? 1 : 0;
	m_Data = m_Data + std::to_string(m_value) + m_Separator;
}

void CTcpPacket::WriteFloat(float value)
{
	m_Data = m_Data + std::to_string(value) + m_Separator;
}

void CTcpPacket::WriteDouble(double value)
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
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading string from TCP packet. Last index wrong");
			return nullptr;
		}
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading string from TCP packet. Bad packet.");
		return nullptr;
	}
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
				return std::stoi(m_Packet.at(m_LastIndex - 1));
			}
			catch (std::invalid_argument &)
			{
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading int from TCP packet. Can't convert string to int");
				return 0;
			}
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading int from TCP packet. Last index wrong");
			return 0;
		}
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading int from TCP packet. Bad packet.");
		return 0;
	}
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
				return std::stof(m_Packet.at(m_LastIndex - 1));
			}
			catch (std::invalid_argument &)
			{
				CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Error reading float from TCP packet. Can't convert string to float");
				return 0.0f;
			}
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Error reading float from TCP packet. Last index wrong");
			return 0.0f;
		}
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Error reading float from TCP packet. Bad packet.");
		return 0.0f;
	}
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
				return std::stod(m_Packet.at(m_LastIndex - 1));
			}
			catch (std::invalid_argument &)
			{
				CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Error reading double from TCP packet. Can't convert string to double");
				return 0.0;
			}
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Error reading double from TCP packet. Last index wrong");
			return 0.0;
		}
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Error reading double from packet. Bad packet.");
		return 0.0;
	}
}

const char * CTcpPacket::toString()
{
	if (!bInitFromData)
	{
		WriteFooter();

#ifndef NDEBUG
		// Packet debugging
		if (mEnv->net_debug > 0)
		{
			CryLog(TITLE "Output TCP packet data : %s", m_Data.c_str());
			CryLog(TITLE "Output TCP packet size : %d", getLength());
		}
#endif
		return m_Data.c_str();
	}
	else
		return m_Data.c_str();
}

void CTcpPacket::GenerateSession()
{
	std::string version = mEnv->net_version ? mEnv->net_version->GetString() : nullptr;
	m_Header = "!0x" + version;
	m_Footer = "0x0!";
}

void CTcpPacket::ReadPacket()
{
	if (!m_Data.empty())
	{
		m_Packet = Split(m_Data, m_Separator);

#ifndef NDEBUG
		// Packet debugging
		if (mEnv->net_debug > 0)
		{
			CryLog(TITLE "Input TCP packet data : %s", m_Data.c_str());
			CryLog(TITLE "Input TCP packet size : %d", getLength());
		}
#endif

		if (m_Packet.size() >= 3)
		{
			std::string packet_header = m_Packet.at(0);
			std::string packet_type = m_Packet.at(1);
			std::string packet_footer = m_Packet.at(m_Packet.size() - 1);

			if (packet_header == m_Header && packet_footer == m_Footer)
			{
				m_Type = (EFireNetTcpPacketType)std::stoi(packet_type);

				if (m_Type != EFireNetTcpPacketType::Empty)
				{
					bIsGoodPacket = true;
					m_LastIndex = 2; // 0 - header, 1 - type, 2 - start data
				}
				else
				{
					CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading TCP packet. Empty packet type");
					bIsGoodPacket = false;
				}
			}
			else
			{
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading TCP packet. Wrong session key!");

				if (gEnv->IsDedicated())
				{
					CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Packet header <%s> - real header <%s>, packet footer <%s> - real footer <%s>", packet_footer.c_str(),
						m_Header.c_str(), packet_footer.c_str(), m_Footer.c_str());
				}

				bIsGoodPacket = false;
			}
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading TCP packet. Packet soo small!");
			bIsGoodPacket = false;
		}
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading TCP packet. Packet empty!");
		bIsGoodPacket = false;
	}
}