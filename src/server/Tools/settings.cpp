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

QString SettingsManager::GetDescription(QString key)
{
	QMutexLocker locker(&m_Mutex);

	for (auto it = m_Variables.begin(); it != m_Variables.end(); ++it)
	{
		if (it->key == key)
		{
			return it->description;
		}
	}

	qWarning() << "Can't get description for" << key << "variable";

	return QString();
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
			if (bBlockOnlineUpdate && !it->bCanChangeOnline)
			{
				qWarning() << "Can't change variable" << key << ". Blocked for online update. For change this variable use config file";
				return;
			}

			if (it->value != value)
			{
				qInfo() << "Variable" << key << "changed value from" << it->value.toString() << "to" << value.toString();
				it->value = value;

				// If variable have callback - execute 
				if (it->pCallback)
				{
					it->pCallback(it->value);
				}

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

void SettingsManager::RegisterVariable(QString key, QVariant value, QString description, bool bCanChangeOnline, void(*pCallback)(QVariant))
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

	if (pCallback)
		newKey.pCallback = pCallback;

	m_Variables.push_back(newKey);

	qDebug() << "Registering new variable" << key;
}
