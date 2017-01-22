// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "settings.h"
#include "global.h"

SettingsManager::SettingsManager(QObject *parent) : QObject(parent)
{
}

SettingsManager::~SettingsManager()
{
	qDebug() << "~SettingsManager";
}

void SettingsManager::Clear()
{
	m_Variables.clear();
}

QVariant SettingsManager::GetVariable(QString key)
{
	QMutexLocker locker(&m_Mutex);

	for (auto it = m_Variables.begin(); it != m_Variables.end(); ++it)
	{
		if (it->key == key)
			return it->value;
	}

	qWarning() << "Variable" << key << "not found!";
	return QVariant();
}

QStringList SettingsManager::GetVariablesList()
{
	QMutexLocker locker(&m_Mutex);

	QStringList varList;

	for (auto it = m_Variables.begin(); it != m_Variables.end(); ++it)
	{
		varList.push_back(it->key);
	}

	return varList;
}

bool SettingsManager::CheckVariableExists(QString key)
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

void SettingsManager::SetVariable(QString key, QVariant value)
{
	QMutexLocker locker(&m_Mutex);

	for (auto it = m_Variables.begin(); it != m_Variables.end(); ++it)
	{
		if (it->key == key)
		{
			if (it->value != value)
			{
				qInfo() << "Variable" << key << "changed value from" << it->value.toString() << "to" << value.toString();
				it->value = value;
				return;
			}
			else
			{
				//qWarning() << "Variable" << key << "not changed, return.";
				return;
			}
		}
	}

	qWarning() << "Can't set variable " << key << ". Variable not found!";
}

void SettingsManager::RegisterVariable(QString key, QVariant value)
{
	QMutexLocker locker(&m_Mutex);

	for (auto it = m_Variables.begin(); it != m_Variables.end(); ++it)
	{
		if (it->key == key)
		{
			qCritical() << "Variable" << key << "alredy registered!";
			return;
		}
	}

	SVariable newKey;
	newKey.key = key;
	newKey.value = value;

	m_Variables.push_back(newKey);

	qDebug() << "Registering new variable" << key;
}
