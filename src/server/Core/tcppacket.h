// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
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
	virtual void               WriteString(const std::string &value) override;
	virtual void               WriteInt(int value) override;
	virtual void               WriteBool(bool value) override;
	virtual void               WriteFloat(float value) override;
	virtual void               WriteDouble(double value) override;
public:
	virtual const char*        ReadString() override;
	virtual int                ReadInt() override;
	virtual bool               ReadBool() override;
	virtual float              ReadFloat() override;
	virtual double             ReadDouble() override;
public:
	virtual const char*        toString() override;
private:
	virtual void               GenerateSession() override;
	virtual void               ReadPacket() override;
};