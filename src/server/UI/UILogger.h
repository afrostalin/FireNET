// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <AbstractStringAppender.h>

#include <QObject>
#include <QTimer>

class UILogger : public QObject, public AbstractStringAppender
{
public:
	Q_OBJECT
public:
	explicit UILogger(QObject *parent = nullptr);
	~UILogger();
protected:
	void append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
		const char* function, const QString& category, const QString& message) override;
	static void PrintToOutput(Logger::LogLevel logLevel, const QString &formatedMessage);
public slots:
	void         update() const;
public:
	void         EnableRuntimeStatus();
private:
	QTimer       m_Timer;
	bool         m_EnabledRuntimeStatus;
};