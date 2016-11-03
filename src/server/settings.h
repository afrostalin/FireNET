// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QVector>
#include <QVariant>

struct SVariable
{
	QString key;
	QVariant value;
};

class SettingsManager : public QObject
{
	Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = 0);
public:
	QVariant GetVariable(QString key);
	QStringList GetVariablesList();
public:
	void SetVariable(QString key, QVariant value);
public:
	void RegisterVariable(QString key, QVariant value);
private:
	QVector<SVariable> m_Variables;
};

#endif // SETTINGS_H