// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "UILogger.h"
#include "UI/mainwindow.h"

UILogger::UILogger()
{
}

UILogger::~UILogger()
{
}

void UILogger::append(const QDateTime & timeStamp, Logger::LogLevel logLevel, const char * file, int line, const char * function, const QString & category, const QString & message)
{
	if (!gEnv->pUI)
		return;

	switch (logLevel)
	{
	case Logger::Trace:
	{
		gEnv->pUI->LogToOutput(ELog_Debug, formattedString(timeStamp, logLevel, file, line, function, category, message));
		break;
	}
	case Logger::Debug:
	{
		gEnv->pUI->LogToOutput(ELog_Debug, formattedString(timeStamp, logLevel, file, line, function, category, message));
		break;
	}
	case Logger::Info:
	{
		gEnv->pUI->LogToOutput(ELog_Info, formattedString(timeStamp, logLevel, file, line, function, category, message));
		break;
	}
	case Logger::Warning:
	{
		gEnv->pUI->LogToOutput(ELog_Warning, formattedString(timeStamp, logLevel, file, line, function, category, message));
		break;
	}
	case Logger::Error:
	{
		gEnv->pUI->LogToOutput(ELog_Error, formattedString(timeStamp, logLevel, file, line, function, category, message));
		break;
	}
	case Logger::Fatal:
	{
		gEnv->pUI->LogToOutput(ELog_Error, formattedString(timeStamp, logLevel, file, line, function, category, message));
		break;
	}
	default:
		break;
	}
}