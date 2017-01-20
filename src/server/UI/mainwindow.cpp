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
    m_OutputItemID = -1;
    SetServerStatus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::LogToOutput(ELogType type, const QString& msg)
{
	ui->Output->addItem(msg);
	m_OutputItemID++;

    switch (type)
    {
    case ELog_Debug:
    {
		auto pItem = ui->Output->item(m_OutputItemID);

		if (pItem)
		{
			pItem->setForeground(Qt::green);
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
			pItem->setForeground(Qt::yellow);
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

	if (!gEnv->isQuiting)
		ui->Output->scrollToBottom();
}

void MainWindow::SetServerStatus()
{
	ui->Status->clear();
    ui->Status->addItem("Server status : null");

	auto pItem = ui->Status->item(0);

	if (pItem)
		pItem->setForeground(Qt::green);
}

void MainWindow::CleanUp()
{
	if (gEnv->isQuiting)
		return;
	else
		gEnv->isQuiting = true;

	qInfo() << "FireNET quiting...";


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
