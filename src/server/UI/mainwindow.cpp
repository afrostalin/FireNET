// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Core/tcpserver.h"
#include "Core/remoteserver.h"

#include "Workers/Databases/dbworker.h"
#include "Workers/Databases/mysqlconnector.h"
#include "Workers/Databases/httpconnector.h"

#include "Tools/settings.h"
#include "Tools/scripts.h"
#include "Tools/UILogger.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	ui->Output->setTextElideMode(Qt::ElideNone);
	ui->Output->setResizeMode(QListView::Adjust);

	m_OutputItemID = -1;

	connect(&m_UpdateTimer, &QTimer::timeout, this, &MainWindow::UpdateServerStatus);
	m_UpdateTimer.start(500);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::LogToOutput(ELogType type, const QString& msg)
{
	// If log level 0 or 1 - We not need use scroll and save all lines
	if (m_OutputItemID >= 23 && gEnv->m_LogLevel < 2)
	{
		auto pItem = ui->Output->item(0);

		if (pItem)
		{
			pItem->~QListWidgetItem();
			m_OutputItemID--;
		}
	}

	ui->Output->addItem(msg);
	m_OutputItemID++;

	switch (type)
	{
	case ELog_Debug:
	{
		auto pItem = ui->Output->item(m_OutputItemID);

		if (pItem)
		{
			pItem->setForeground(Qt::darkGreen);
		}

		break;
	}
	case ELog_Info:
	{
		auto pItem = ui->Output->item(m_OutputItemID);

		if (pItem)
		{
			pItem->setForeground(Qt::white);
		}

		break;
	}
	case ELog_Warning:
	{
		auto pItem = ui->Output->item(m_OutputItemID);

		if (pItem)
		{
			pItem->setForeground(Qt::darkYellow);
		}

		break;
	}
	case ELog_Error:
	{
		auto pItem = ui->Output->item(m_OutputItemID);

		if (pItem)
		{
			pItem->setForeground(Qt::red);
		}

		break;
	}
	default:
		break;
	};

	if (!gEnv->isQuiting && gEnv->m_LogLevel == 2)
		ui->Output->scrollToBottom();
}

void MainWindow::UpdateServerStatus()
{
	if (gEnv->isQuiting)
		return;

	ui->Status->clear();

	// Server status
	QString serverStatus = "offline";
	if (gEnv->pServer && gEnv->pServer->m_Status == EServer_Online)
		serverStatus = "online";
	else if (gEnv->pServer && gEnv->pServer->m_Status == EServer_Offline)
		serverStatus = "offline";

	// Remote server status
	QString remoteServerStatus = "offline";
	if (gEnv->pRemoteServer && gEnv->pRemoteServer->m_Status == ERServer_Online)
		remoteServerStatus = "online";
	else if (gEnv->pRemoteServer && gEnv->pRemoteServer->m_Status == ERServer_Offline)
		remoteServerStatus = "offline";

	// Databases mode
	QString dbMode = "None";
	if (gEnv->pSettings)
		dbMode = gEnv->pSettings->GetVariable("db_mode").toString();

	// Databases status
	QString dbStatus = "not init";
	if (gEnv->pDBWorker && gEnv->pDBWorker->m_Status == EDB_Init)
		dbStatus = "init";
	else if (gEnv->pDBWorker && gEnv->pDBWorker->m_Status == EDB_StartConnecting)
		dbStatus = "connecting";
	else if (gEnv->pDBWorker && gEnv->pDBWorker->m_Status == EDB_Connected)
		dbStatus = "connected";
	else if (gEnv->pDBWorker && gEnv->pDBWorker->m_Status == EDB_NoConnection)
		dbStatus = "offline";

	// Global status
	int clientCount = 0;
	int maxClientCount = 0;

	int gsCount = 0;
	int maxGsCount = 0;

	if (gEnv->pServer)
	{
		clientCount = gEnv->pServer->GetClientCount();
		maxClientCount = gEnv->pServer->GetMaxClientCount();
	}

	if (gEnv->pRemoteServer)
	{
		gsCount = gEnv->pRemoteServer->GetClientCount();
		maxGsCount = gEnv->pRemoteServer->GetMaxClientCount();
	}

	QString status = "MainServer : " + serverStatus +
		" | RemoteServer : " + remoteServerStatus +
		" | DB mode : " + dbMode +
		" | DB status : " + dbStatus +
		" | Clients : " + QString::number(clientCount) + "/" + QString::number(maxClientCount) + 
		" | GameServers : " + QString::number(gsCount) + "/" + QString::number(maxGsCount) ;

	ui->Status->addItem(status);

	auto pItem = ui->Status->item(0);

	if (pItem)
		pItem->setForeground(Qt::darkCyan);
}

void MainWindow::CleanUp()
{
	if (gEnv->isQuiting)
		return;
	else
		gEnv->isQuiting = true;

	qInfo() << "FireNET quiting...";

	m_UpdateTimer.stop();

	if (gEnv->pTimer)
		gEnv->pTimer->stop();


	SAFE_CLEAR(gEnv->pServer);
	SAFE_CLEAR(gEnv->pRemoteServer);
	SAFE_CLEAR(gEnv->pSettings);
	SAFE_CLEAR(gEnv->pScripts);
	SAFE_CLEAR(gEnv->pDBWorker);

	while (!gEnv->isReadyToClose)
	{
		QEventLoop loop;
		QTimer::singleShot(33, &loop, &QEventLoop::quit);
		loop.exec();
	}

	SAFE_RELEASE(gEnv->pTimer);
	SAFE_RELEASE(gEnv->pServer);
	SAFE_RELEASE(gEnv->pRemoteServer);
	SAFE_RELEASE(gEnv->pSettings);
	SAFE_RELEASE(gEnv->pScripts);
	SAFE_RELEASE(gEnv->pDBWorker);

	QThreadPool::globalInstance()->waitForDone(1000);

	QCoreApplication::quit();
}

void MainWindow::ClearOutput()
{
	ui->Output->clear();
	m_OutputItemID = -1;
}

void MainWindow::ClearStatus()
{
	ui->Status->clear();
}

void MainWindow::on_Input_returnPressed()
{
	QString input = ui->Input->text();
	ui->Input->clear();

	if (input == "quit")
	{
		CleanUp();
	}
}
