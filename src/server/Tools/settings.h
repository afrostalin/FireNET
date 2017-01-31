// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QVector>
#include <QVariant>
#include <QMutex>

struct SVariable
{
	QString key;
	QVariant value;
	QString description;
	bool bCanChangeOnline;
	void(*pCallback)(QVariant);
};

class SettingsManager : public QObject
{
	Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = 0);
	~SettingsManager();
public:
	void Clear();
public:
	QVariant GetVariable(const QString &key);	
	QString GetDescription(const QString &key);
	bool FindVariabelMatches(const QString &key);
	bool CheckVariableExists(const QString &key);

	QStringList GetVariablesList();
public:
	void SetVariable(const QString &key, const QVariant &value);
	void BlockOnlineUpdate() { bBlockOnlineUpdate = true; }
public:
	void RegisterVariable(const QString &key, const QVariant &value, const QString &description, bool bCanChangeOnline, void (*pCallback)(QVariant) = nullptr);
private:
	QVector<SVariable> m_Variables;
	QMutex m_Mutex;
	bool bBlockOnlineUpdate;
};

#endif // SETTINGS_H