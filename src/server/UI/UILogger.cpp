// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "UILogger.h"
#include "mainwindow.h"
#include "Core/tcpserver.h"
#include "Core/remoteserver.h"
#include "Tools/console.h"

UILogger::UILogger(QObject *parent)
	: QObject(parent)
	, m_EnabledRuntimeStatus(false)
{
	connect(&m_Timer, &QTimer::timeout, this, &UILogger::update);

	m_Timer.start(250);
}

UILogger::~UILogger()
{
	m_Timer.stop();
}

void UILogger::update() const
{
	if (m_EnabledRuntimeStatus)
	{
		const Logger::LogLevel logLevel = Logger::Info;

		CConsole* pConsole = gEnv->pConsole;
		if (pConsole == nullptr || gEnv->pUI == nullptr)
		{
			LogError("Can't get use runtime status!");
			return;
		}

		gEnv->pUI->ClearOutput();


		// Main server status
		PrintToOutput(logLevel, _strFormat("Server version : %s", gEnv->m_serverFullName.toStdString().c_str()).c_str());
		PrintToOutput(logLevel, _strFormat("Main server (%s:%d) - %s", pConsole->GetString("sv_ip").c_str(), pConsole->GetInt("sv_port"), gEnv->m_ServerStatus.m_MainServerStatus.toStdString().c_str()).c_str());
		PrintToOutput(logLevel, _strFormat("Clients count : %d/%d", gEnv->pServer->GetClientCount(), gEnv->pServer->GetMaxClientCount()).c_str());
		PrintToOutput(logLevel, _strFormat("Maximum active clients count : %d", gEnv->m_MaxClientCount).c_str());
		PrintToOutput(logLevel, _strFormat("Thread count : %d", gEnv->pConsole->GetInt("sv_thread_count")).c_str());
		PrintToOutput(logLevel, _strFormat("Server tickrate : %d per/sec.", gEnv->pConsole->GetInt("sv_tickrate")).c_str());

		// Packets info
		PrintToOutput(logLevel, _strFormat("Input packets count : %d", gEnv->m_InputPacketsCount).c_str());
		PrintToOutput(logLevel, _strFormat("Input packets current speed : %d packets/sec.", gEnv->m_InputSpeed).c_str());
		PrintToOutput(logLevel, _strFormat("Input packets max speed : %d packets/sec.", gEnv->m_InputMaxSpeed).c_str());

		PrintToOutput(logLevel, _strFormat("Output packets : %d", gEnv->m_OutputPacketsCount).c_str());
		PrintToOutput(logLevel, _strFormat("Output packets current speed : %d packets/sec.", gEnv->m_OutputSpeed).c_str());
		PrintToOutput(logLevel, _strFormat("Output packets max speed : %d packets/sec.", gEnv->m_OutputMaxSpeed).c_str());

		// Remote server status
		PrintToOutput(logLevel, _strFormat("Remote server (%s:%d) - %s", pConsole->GetString("sv_ip").c_str(), pConsole->GetInt("remote_server_port"), gEnv->m_ServerStatus.m_RemoteServerStatus.toStdString().c_str()).c_str());
		PrintToOutput(logLevel, _strFormat("Remote admin : %s", gEnv->pRemoteServer->IsHaveAdmin() ? "online" : "offline").c_str());
		PrintToOutput(logLevel, _strFormat("Game servers : %d/%d", gEnv->pRemoteServer->GetClientCount(), gEnv->pRemoteServer->GetMaxClientCount()).c_str());

		// Databases mode
		PrintToOutput(logLevel, _strFormat("Database mode : %s", gEnv->m_ServerStatus.m_DBMode.toStdString().c_str()).c_str());

		// Databases status
		PrintToOutput(logLevel, _strFormat("Database status : %s", gEnv->m_ServerStatus.m_DBStatus.toStdString().c_str()).c_str());

		// Debug messages
		PrintToOutput(logLevel, _strFormat("Debug messages : %d", gEnv->m_DebugsCount).c_str());
		PrintToOutput(logLevel, _strFormat("Warning messages : %d", gEnv->m_WarningsCount).c_str());
		PrintToOutput(logLevel, _strFormat("Error messages : %d", gEnv->m_ErrorsCount).c_str());
	}
}

void UILogger::append(const QDateTime & timeStamp, Logger::LogLevel logLevel, const char * file, int line, const char * function, const QString & category, const QString & message)
{
#ifndef NO_UI
	if (gEnv->pUI == nullptr || gEnv->pConsole == nullptr)
		return;

	// If we run runtime status - we need calculate statistic without update UI
	if (m_EnabledRuntimeStatus)
	{
		switch (logLevel)
		{
		case Logger::Debug:
		{
			gEnv->m_DebugsCount++;
			break;
		}
		case Logger::Warning:
		{
			gEnv->m_WarningsCount++;
			break;
		}
		case Logger::Error:
		{
			gEnv->m_ErrorsCount++;
			break;
		}
		default:
			break;
		}

		return;
	}
	//
#endif
	QString formatedString = formattedString(timeStamp, logLevel, file, line, function, category, message);

	// Write info to debug window
	OutputDebugStringA(_strFormat("%s\n", formatedString.toStdString().c_str()).c_str());

#ifndef NO_UI
	const int lenght = gEnv->pConsole->GetInt("sv_ui_width_text_size");

	for (unsigned i = 0; i < formatedString.toStdString().length(); i += lenght)
	{
		const QString formatedMessage = formatedString.toStdString().substr(i, lenght).c_str();
		PrintToOutput(logLevel, formatedMessage);
	}
#else
	PrintToOutput(logLevel, message);
#endif 
}

void UILogger::PrintToOutput(const Logger::LogLevel logLevel, const QString & formatedMessage)
{
#ifndef NO_UI
	switch (logLevel)
	{
	case Logger::Trace:
	{
		gEnv->pUI->LogToOutput(ELog_Debug, formatedMessage);
		break;
	}
	case Logger::Debug:
	{
		gEnv->pUI->LogToOutput(ELog_Debug, formatedMessage);
		gEnv->m_DebugsCount++;
		break;
	}
	case Logger::Info:
	{
		gEnv->pUI->LogToOutput(ELog_Info, formatedMessage);
		break;
	}
	case Logger::Warning:
	{
		gEnv->pUI->LogToOutput(ELog_Warning, formatedMessage);
		gEnv->m_WarningsCount++;
		break;
	}
	case Logger::Error:
	{
		gEnv->pUI->LogToOutput(ELog_Error, formatedMessage);
		gEnv->m_ErrorsCount++;
		break;
	}
	case Logger::Fatal:
	{
		gEnv->pUI->LogToOutput(ELog_Error, formatedMessage);
		break;
	}
	default:
		break;
	}
#else
	printf_s("%s\n", formatedString.toStdString().c_str());
#endif
}

void UILogger::EnableRuntimeStatus()
{
#ifndef NO_UI
	if (!m_EnabledRuntimeStatus)
	{
		m_EnabledRuntimeStatus = true;
	}
	else
	{
		m_EnabledRuntimeStatus = false;
	}
#endif
}
