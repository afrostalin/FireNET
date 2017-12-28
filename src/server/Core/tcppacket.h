// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNetCore/IFireNetTcpPacket.h>

class CTcpPacket : public IFireNetTcpPacket
{
public:
	CTcpPacket::CTcpPacket() {}
	CTcpPacket::CTcpPacket(EFireNetTcpPacketType type);
	CTcpPacket::CTcpPacket(const char* data);
public:
	// IFireNetTcpPacket
	void               WriteStdString(const std::string &value) override;
	void               WriteString(const char* value) override;
	void               WriteInt(int value) override;
	void               WriteBool(bool value) override;
	void               WriteFloat(float value) override;
	void               WriteDouble(double value) override;
	// ~IFireNetTcpPacket

	void               WriteAdminCommand(EFireNetAdminCommands command) { WriteInt(static_cast<int>(command)); }
public:
	// IFireNetTcpPacket
	const char*        ReadString() override;
	int                ReadInt() override;
	bool               ReadBool() override;
	float              ReadFloat() override;
	double             ReadDouble() override;
	// ~IFireNetTcpPacket

	EFireNetAdminCommands      ReadAdminCommand() { return static_cast<EFireNetAdminCommands>(ReadInt()); }
public:
	const char*        toString() override;
private:
	void               GenerateSession() override;
	void               ReadPacket() override;
};