// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "settings.h"
#include "global.h"

SettingsManager::SettingsManager(QObject *parent) : QObject(parent),
	bBlockOnlineUpdate(false)
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

QVariant SettingsManager::GetVariable(const QString &key)
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

QString SettingsManager::GetDescription(const QString &key)
{
	QMutexLocker locker(&m_Mutex);

	for (auto it = m_Variables.begin(); it != m_Variables.end(); ++it)
	{
		if (it->key == key)
		{
			return it->description;
		}
	}

	return QString();
}

bool SettingsManager::FindVariabelMatches(const QString & key)
{
	QMutexLocker locker(&m_Mutex);

	bool result = false;

	for (auto it = m_Variables.begin(); it != m_Variables.end(); ++it)
	{
		if (it->key.contains(key))
		{
			qWarning() << it->key.toStdString().c_str() << "-" << it->description;

			result = true;
		}
	}

	return result;
}

bool SettingsManager::CheckVariableExists(const QString &key)
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

void SettingsManager::SetVariable(const QString &key, const QVariant &value)
{
	QMutexLocker locker(&m_Mutex);

	for (auto it = m_Variables.begin(); it != m_Variables.end(); ++it)
	{
		if (it->key == key)
		{
			// If variable have callback - execute 
			if (it->pCallback)
			{
				it->pCallback(value);
			}

			if (bBlockOnlineUpdate && !it->bCanChangeOnline)
			{
				qWarning() << "Can't change variable" << key << ". Blocked for online update. For change this variable use config file";
				return;
			}
			else if (it->value != value)
			{
				qInfo() << "Variable" << key << "changed value from" << it->value.toString() << "to" << value.toString();
				it->value = value;
				return;
			}
			else
			{
				qDebug() << "Variable" << key << "not changed, return.";
				return;
			}			
		}
	}

	qWarning() << "Can't set variable " << key << ". Variable not found!";
}

void SettingsManager::RegisterVariable(const QString &key, const QVariant &value, const QString &description, bool bCanChangeOnline, void(*pCallback)(QVariant))
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
	newKey.description = description;
	newKey.bCanChangeOnline = bCanChangeOnline;
	newKey.pCallback = pCallback;

	m_Variables.push_back(newKey);

	qDebug() << "Registering new variable" << key;
}
