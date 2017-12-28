// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QVariant>
#include <QMutex>

typedef void(*ConsoleCommandCallback)(QStringList&);
typedef void(*ConsoleVarianleCallback)(QVariant);

struct SVariable
{
	QString  key;
	QVariant value;
	QString  description;
	bool     bCanChangeOnline = false;
	ConsoleVarianleCallback callback = nullptr;
};

struct SConsoleCommand
{
	QString name;
	QString description;
	ConsoleCommandCallback callback = nullptr;
	bool canHaveMoreOneArgs = false;
};

class CConsole : public QObject
{
	Q_OBJECT
public:
    explicit CConsole(QObject *parent = nullptr);
	~CConsole();
public:
	void               Clear();

	QVariant           GetVariable(const QString &key);	
	int                GetInt(const char* key);
	std::string        GetString(const char* key);
	bool               GetBool(const char* key);
	float              GetFloat(const char* key);
	double             GetDouble(const char* key);
	QString            GetDescription(const QString &key);

	bool               FindVariabelMatches(const QString &key);
	bool               CheckVariableExists(const QString &key);
	QStringList        GetVariablesList();
public:
	void               SetVariable(const QString &key, const QVariant &value);
	void               BlockOnlineUpdate() { bBlockOnlineUpdate = true; }
public:
	void               RegisterVariable(const QString &key, const QVariant &value, const QString &description, bool bCanChangeOnline, ConsoleVarianleCallback pCallback = nullptr);
	void               RegisterCommand(const QString &name, ConsoleCommandCallback callback, const QString &description, bool canHaveMoreOneArgs = false);
	bool               ExecuteCommand(const QString &str);
	void               PrintAllCommands();
private:
	std::vector<SVariable>       m_Variables;
	std::vector<SConsoleCommand> m_Commands;
	QMutex                       m_Mutex;
	bool                         bBlockOnlineUpdate;
};