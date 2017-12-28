// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QDebug>
#include <stdarg.h>
#include <stdio.h>

class RemoteClient;
class InputListener;

#define MAX_LOG_BUFFER_SIZE 1024

inline void LogDebug(const char* format, ...)
{
	int nSize = 0;
	char buff[MAX_LOG_BUFFER_SIZE];
	memset(buff, 0, sizeof(buff));
	va_list args;
	va_start(args, format);
	nSize = vsnprintf_s(buff, _countof(buff), _TRUNCATE, format, args);
	qDebug() << buff;
	va_end(args);
}

inline void LogInfo(const char* format, ...)
{
	int nSize = 0;
	char buff[MAX_LOG_BUFFER_SIZE];
	memset(buff, 0, sizeof(buff));
	va_list args;
	va_start(args, format);
	nSize = vsnprintf_s(buff, _countof(buff), _TRUNCATE, format, args);
	qInfo() << buff;
	va_end(args);
}

inline void LogWarning(const char* format, ...)
{
	int nSize = 0;
	char buff[MAX_LOG_BUFFER_SIZE];
	memset(buff, 0, sizeof(buff));
	va_list args;
	va_start(args, format);
	nSize = vsnprintf_s(buff, _countof(buff), _TRUNCATE, format, args);
	qWarning() << buff;
	va_end(args);
}

inline void LogError(const char* format, ...)
{
	int nSize = 0;
	char buff[MAX_LOG_BUFFER_SIZE];
	memset(buff, 0, sizeof(buff));
	va_list args;
	va_start(args, format);
	nSize = vsnprintf_s(buff, _countof(buff), _TRUNCATE, format, args);
	qCritical() << buff;
	va_end(args);
}

struct SGlobalEnvironment
{
    RemoteClient*  pClient = nullptr;
    InputListener* pListener = nullptr;

    bool           bUsePacketDebug = false;
	QString        net_version = "1.0.0.0";
	QString        m_ServerIP = "127.0.0.1";
	int            m_ServerPort = 64000;
};

extern SGlobalEnvironment* gEnv;