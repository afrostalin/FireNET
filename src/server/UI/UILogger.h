// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include "CuteLogger_global.h"
#include <AbstractStringAppender.h>

class UILogger : public AbstractStringAppender
{
public:
	UILogger();
	~UILogger();
protected:
	virtual void append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
		const char* function, const QString& category, const QString& message);
};