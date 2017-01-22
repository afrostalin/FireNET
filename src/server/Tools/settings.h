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
	QVariant GetVariable(QString key);
	QStringList GetVariablesList();
	QString GetDescription(QString key);

	bool CheckVariableExists(QString key);
public:
	void SetVariable(QString key, QVariant value);
	void BlockOnlineUpdate() { bBlockOnlineUpdate = true; }
public:
	void RegisterVariable(QString key, QVariant value, QString description, bool bCanChangeOnline);
private:
	QVector<SVariable> m_Variables;
	QMutex m_Mutex;
	bool bBlockOnlineUpdate;
};

#endif // SETTINGS_H