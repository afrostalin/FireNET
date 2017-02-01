// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef UITHREAD_H
#define UITHREAD_H

#include <QObject>
#include <QEventLoop>

class CServerThread : public QObject
{
	Q_OBJECT
public:
	explicit CServerThread(QObject *parent = nullptr);
	~CServerThread();
public slots:
	void start();
	void stop();
signals:
	void EnableStressMode();
private:
	bool Init();
	void StartLogging();
	void RegisterVariables();
	void ReadServerCFG();
	void EnableStressTest();
private:
	QEventLoop* m_loop;
};

#endif