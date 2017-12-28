// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QEventLoop>

class CServerThread : public QObject
{
	Q_OBJECT
public:
	explicit CServerThread(QObject *parent = nullptr);
	~CServerThread();
private:
	bool        Init() const;
	void        StartLogging();
	void        RegisterVariables();
public:	
	void        EnableStressTest();
	void        ReadServerCFG(bool online = false);
	std::vector<std::string> DumpStatistic();
public slots:
	void        start();
	void        stop() const;
signals:
	void        EnableStressMode();
private:
	QEventLoop* m_loop;
};