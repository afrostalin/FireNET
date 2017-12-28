// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "console.h"

CConsole::CConsole(QObject *parent) 
	: QObject(parent)
	, bBlockOnlineUpdate(false)
{
}

CConsole::~CConsole()
{
	m_Variables.clear();
}

void CConsole::Clear()
{
	m_Variables.clear();
}

QVariant CConsole::GetVariable(const QString &key)
{
	for (const auto it : m_Variables)
	{
		if (it.key == key)
			return it.value;
	}

	LogWarning("Variable <%s> not found!", key.toStdString().c_str());
	return QVariant();
}

int CConsole::GetInt(const char * key)
{
	return GetVariable(key).toInt();
}

std::string CConsole::GetString(const char * key)
{
	return GetVariable(key).toString().toStdString();
}

bool CConsole::GetBool(const char * key)
{
	return GetVariable(key).toBool();
}

float CConsole::GetFloat(const char * key)
{
	return GetVariable(key).toFloat();
}

double CConsole::GetDouble(const char * key)
{
	return GetVariable(key).toDouble();
}

QStringList CConsole::GetVariablesList()
{
	QMutexLocker locker(&m_Mutex);

	QStringList varList;

	for (auto it = m_Variables.begin(); it != m_Variables.end(); ++it)
	{
		varList.push_back(it->key);
	}

	return varList;
}

QString CConsole::GetDescription(const QString &key)
{
	for (const auto &it : m_Variables)
	{
		if (it.key == key)
		{
			return it.description;
		}
	}

	return QString();
}

bool CConsole::FindVariabelMatches(const QString & key)
{
	bool result = false;

	for (const auto &it : m_Variables)
	{
		if (it.key.contains(key))
		{
			LogWarning("%s - %s", it.key.toStdString().c_str(), it.description.toStdString().c_str());
			result = true;
		}
	}

	return result;
}

bool CConsole::CheckVariableExists(const QString &key)
{
	QMutexLocker locker(&m_Mutex);

	for (auto it = m_Variables.begin(); it != m_Variables.end(); ++it)
	{
		if (it->key == key)
		{
			return true;
		}
	}

	return false;
}

void CConsole::SetVariable(const QString &key, const QVariant &value)
{
	QMutexLocker locker(&m_Mutex);

	for (auto& it : m_Variables)
	{
		if (it.key == key)
		{
			if (bBlockOnlineUpdate && !it.bCanChangeOnline)
			{
				LogWarning("Can't change variable <%s>. Blocked for online update", it.key.toStdString().c_str());
				return;
			}
			if (it.value != value)
			{
				it.value = value;

				if (it.callback)
				{
					it.callback(value);
				}

				return;
			}
			return;
		}
	}

	LogWarning("Can't set variable <%s>. Variable not found!", key.toStdString().c_str());
}

void CConsole::RegisterVariable(const QString &key, const QVariant &value, const QString &description, bool bCanChangeOnline, ConsoleVarianleCallback pCallback)
{
	QMutexLocker locker(&m_Mutex);

	for (auto it = m_Variables.begin(); it != m_Variables.end(); ++it)
	{
		if (it->key == key)
		{
			LogError("Variable <%s> alredy registered!", key.toStdString().c_str());
			return;
		}
	}

	SVariable newKey;
	newKey.key = key;
	newKey.value = value;
	newKey.description = description;
	newKey.bCanChangeOnline = bCanChangeOnline;
	newKey.callback = pCallback;

	m_Variables.push_back(newKey);
}

void CConsole::RegisterCommand(const QString & name, ConsoleCommandCallback callback, const QString &description, bool canHaveMoreOneArgs)
{
	QMutexLocker locker(&m_Mutex);

	if (name.isEmpty() || !callback)
	{
		LogError("Can't register console command <%s> - empty name or callback pointer", name);
		return;
	}

	for (const auto &it : m_Commands)
	{
		if (it.name == name)
		{
			LogError("Can't register console command <%s> - alredy registered!", name);
		}
	}

	SConsoleCommand command;
	command.name = name;
	command.callback = callback;
	command.description = description;
	command.canHaveMoreOneArgs = canHaveMoreOneArgs;

	m_Commands.push_back(command);
}

bool CConsole::ExecuteCommand(const QString & str)
{
	LogInfo("Executing command <%s>", str.toStdString().c_str());

	QStringList tmpList = str.split(" ");
	QString commandName;

	if (tmpList.size() >= 1)
	{
		commandName = tmpList[0];
	}

	for (const auto &it : m_Commands)
	{
		if (commandName.isEmpty())
		{
			if (strstr(str.toStdString().c_str(), it.name.toStdString().c_str()))
			{
				if (it.canHaveMoreOneArgs)
				{
					QStringList cmdArgs = str.split(" ");
					if (!cmdArgs.isEmpty())
					{
						it.callback(cmdArgs);
						return true;
					}
				}
				else
				{
					QString strTMP = str;
					QStringList cmdArgs;
					cmdArgs.push_back(it.name);

					QString args = strTMP.remove(it.name + " ");
					if (args.contains(it.name))
					{
						args = "";
					}

					cmdArgs.push_back(args);

					it.callback(cmdArgs);
					return true;
				}
			}
		}
		else
		{
			if (_qstricmp(commandName, it.name))
			{
				if (it.canHaveMoreOneArgs)
				{
					QStringList cmdArgs = str.split(" ");
					if (!cmdArgs.isEmpty())
					{
						it.callback(cmdArgs);
						return true;
					}
				}
				else
				{
					QString strTMP = str;
					QStringList cmdArgs;
					cmdArgs.push_back(it.name);

					QString args = strTMP.remove(it.name + " ");
					if (args.contains(it.name))
					{
						args = "";
					}

					cmdArgs.push_back(args);

					it.callback(cmdArgs);
					return true;
				}
			}
		}
	}

	return false;
}

void CConsole::PrintAllCommands()
{
	for (const auto &it : m_Commands)
	{
		LogWarning("%s - %s", it.name.toStdString().c_str(), it.description.toStdString().c_str());
	}
}
