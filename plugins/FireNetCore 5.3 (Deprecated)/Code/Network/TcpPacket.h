// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNet>

class CTcpPacket : public IFireNetTcpPacket
{
public:
	CTcpPacket::CTcpPacket(EFireNetTcpPacketType type, bool waitable = false);
	CTcpPacket::CTcpPacket(const char* data);
public:
	void               WriteStdString(const std::string &value) override;
	void               WriteString(const char* value) override;
	void               WriteInt(int value) override;
	void               WriteBool(bool value) override;
	void               WriteFloat(float value) override;
	void               WriteDouble(double value) override;
public:
	const char*        ReadString() override;
	int                ReadInt() override;
	bool               ReadBool() override;
	float              ReadFloat() override;
	double             ReadDouble() override;
public:
	const char*        toString() override;
	bool               IsWaitable() { return m_isWaitable; };
private:
	void               GenerateSession() override;
	void               ReadPacket() override;
private:
	bool               m_isWaitable;
};