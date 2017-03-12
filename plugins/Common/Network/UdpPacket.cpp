// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "UdpPacket.h"

CUdpPacket::CUdpPacket(int packetNumber, EFireNetUdpPacketType type)
{
	m_Separator = '|';
	m_Type = type;

	// Only for reading
	bInitFromData = false;
	bIsGoodPacket = false;

	m_LastIndex = 0;
	m_PacketNumber = 0;

	GenerateSession();

	WriteHeader();
	WritePacketType(type);
	WriteInt(packetNumber);
}

CUdpPacket::CUdpPacket(const char * data)
{
	if (data)
	{
		m_Data = data;
		m_Type = EFireNetUdpPacketType::Empty;
		m_Separator = '|';

		bInitFromData = true;
		bIsGoodPacket = false;

		m_LastIndex = 0;
		m_PacketNumber = 0;

		GenerateSession();
		ReadPacket();
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Empty UDP packet!");
		m_Type = EFireNetUdpPacketType::Empty;
	}
}

void CUdpPacket::WriteString(const std::string & value)
{
	m_Data = m_Data + value + m_Separator;
}

void CUdpPacket::WriteInt(int value)
{
	m_Data = m_Data + std::to_string(value) + m_Separator;
}

void CUdpPacket::WriteBool(bool value)
{
	int m_value = value ? 1 : 0;
	m_Data = m_Data + std::to_string(m_value) + m_Separator;
}

void CUdpPacket::WriteFloat(float value)
{
	m_Data = m_Data + std::to_string(value) + m_Separator;
}

void CUdpPacket::WriteDouble(double value)
{
	m_Data = m_Data + std::to_string(value) + m_Separator;
}

const char * CUdpPacket::ReadString()
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
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading string from UDP packet. Last index wrong");
			return nullptr;
		}
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading string from UDP packet. Bad packet.");
		return nullptr;
	}
}

int CUdpPacket::ReadInt()
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
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading int from UDP packet. Can't convert string to int");
				return 0;
			}
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading int from UDP packet. Last index wrong");
			return 0;
		}
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading int from UDP packet. Bad packet.");
		return 0;
	}
}

bool CUdpPacket::ReadBool()
{
	return ReadInt() == 1 ? true : false;
}

float CUdpPacket::ReadFloat()
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
				CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Error reading float from UDP packet. Can't convert string to float");
				return 0.0f;
			}
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Error reading float from UDP packet. Last index wrong");
			return 0.0f;
		}
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Error reading float from UDP packet. Bad packet.");
		return 0.0f;
	}
}

double CUdpPacket::ReadDouble()
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

void CUdpPacket::WriteVec3(Vec3 vector)
{
	WriteFloat(vector.x);
	WriteFloat(vector.y);
	WriteFloat(vector.z);
}

void CUdpPacket::WriteQuat(Quat quat)
{
	WriteFloat(quat.w);
	WriteVec3(quat.v);
}

Vec3 CUdpPacket::ReadVec3()
{
	Vec3 m_Vector;
	m_Vector.x = ReadFloat();
	m_Vector.y = ReadFloat();
	m_Vector.z = ReadFloat();

	return m_Vector;
}

Quat CUdpPacket::ReadQuat()
{
	Quat m_Quat;
	m_Quat.w = ReadFloat();
	m_Quat.v = ReadVec3();

	return m_Quat;
}

const char * CUdpPacket::toString()
{
	if (!bInitFromData)
	{
		WriteFooter();
		EncryptPacket();

		ICVar* debug = gEnv->pConsole->GetCVar("firenet_packet_debug");

		if (debug && debug->GetIVal() > 0)
		{
			CryLog(TITLE "Output UDP packet data : %s", m_Data.c_str());
			CryLog(TITLE "Output UDP packet size : %d", getLength());
		}

		return m_Data.c_str();
	}
	else
		return m_Data.c_str();
}

void CUdpPacket::GenerateSession()
{
	// TODO
	m_Header = "!0x0";
	m_Footer = "0x0!";
}

void CUdpPacket::ReadPacket()
{
	if (!m_Data.empty())
	{
		DecryptPacket();
		m_Packet = Split(m_Data, m_Separator);

		ICVar* debug = gEnv->pConsole->GetCVar("firenet_packet_debug");

		if (debug && debug->GetIVal() > 0)
		{
			CryLog(TITLE "Input UDP packet data : %s", m_Data.c_str());
			CryLog(TITLE "Input UDP packet size : %d", getLength());
		}

		if (m_Packet.size() >= 4)
		{
			std::string packet_header = m_Packet.at(0);
			std::string packet_type = m_Packet.at(1);
			std::string packet_number = m_Packet.at(2);
			std::string packet_footer = m_Packet.at(m_Packet.size() - 1);

			if (packet_header == m_Header && packet_footer == m_Footer)
			{
				m_Type = (EFireNetUdpPacketType)std::stoi(packet_type);
				m_PacketNumber = std::stoi(packet_number);

				if (m_Type != EFireNetUdpPacketType::Empty)
				{
					bIsGoodPacket = true;
					m_LastIndex = 3; // 0 - header, 1 - type, 2 - packet number, 3 - start data
				}
				else
				{
					CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading UDP packet. Empty packet type");
					bIsGoodPacket = false;
				}
			}
			else
			{
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading UDP packet. Wrong session key!");
				bIsGoodPacket = false;
			}
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading UDP packet. Packet soo small!");
			bIsGoodPacket = false;
		}
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error reading UDP packet. Packet empty!");
		bIsGoodPacket = false;
	}
}

void CUdpPacket::EncryptPacket()
{
	// TODO
}

void CUdpPacket::DecryptPacket()
{
	// TODO
}
