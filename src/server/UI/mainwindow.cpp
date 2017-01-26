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
	connect(this, &MainWindow::scroll, ui->Output, &QListWidget::scrollToBottom);

	m_UpdateTimer.start(500);
}

MainWindow::~MainWindow()
{
	SAFE_DELETE(ui);
}

void MainWindow::LogToOutput(ELogType type, const QString& msg)
{
	QMutexLocker locker(&m_Mutex);

	// If log level 0 or 1 - We not need use scroll and save all lines
	if (m_OutputItemID >= 23 && gEnv->m_LogLevel < 2)
	{
		auto pItem = ui->Output->item(0);

		if (pItem)
		{
			SAFE_DELETE(pItem);
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

	if (ui && ui->Output && gEnv && !gEnv->isQuiting && gEnv->m_LogLevel == 2)
	{
		emit scroll();
	}
}

void MainWindow::UpdateServerStatus()
{
	if (gEnv->isQuiting)
		return;

	ui->Status->clear();


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

	QString status = "MainServer : " + gEnv->m_ServerStatus.m_MainServerStatus +
		" | RemoteServer : " + gEnv->m_ServerStatus.m_RemoteServerStatus +
		" | DB mode : " + gEnv->m_ServerStatus.m_DBMode +
		" | DB status : " + gEnv->m_ServerStatus.m_DBStatus +
		" | Clients : " + QString::number(clientCount) + "/" + QString::number(maxClientCount) + 
		" | GameServers : " + QString::number(gsCount) + "/" + QString::number(maxGsCount) ;

	ui->Status->addItem(status);

	auto pItem = ui->Status->item(0);

	if (pItem)
		pItem->setForeground(Qt::darkCyan);
}

void MainWindow::CleanUp()
{
	emit stop();
	m_UpdateTimer.stop();
	
	// Wait server thread here
	while (!gEnv->isReadyToClose)
	{
		QEventLoop loop;
		QTimer::singleShot(33, &loop, &QEventLoop::quit);
		loop.exec();
	}

	ClearOutput();
	ClearStatus();

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

	if (input == "status")
	{
		if (gEnv->m_LogLevel < 2)
		{
			qWarning() << "Before get full server status change log level to 2";
			return;
		}

		qInfo() << "***FULL SERVER STATUS***";

		// Main server status
		qInfo() << "Main server (" << gEnv->pSettings->GetVariable("sv_ip").toString().toStdString().c_str() << ":" << gEnv->pSettings->GetVariable("sv_port").toInt() << ") - " << gEnv->m_ServerStatus.m_MainServerStatus.toStdString().c_str();
		qInfo() << "Clients :" << gEnv->pServer->GetClientCount() << "/" << gEnv->pServer->GetMaxClientCount();
		qInfo() << "Thread count :" << gEnv->pSettings->GetVariable("sv_thread_count").toInt();
		qInfo() << "Server tickrate :" << gEnv->pSettings->GetVariable("sv_tickrate").toInt() << "per/sec.";

		// Remote server status
		QString remoteAdminStatus = gEnv->pRemoteServer->IsHaveAdmin() ? "online" : "offline";

		qInfo() << "Remote server (" << gEnv->pSettings->GetVariable("sv_ip").toString().toStdString().c_str() << ":" << gEnv->pSettings->GetVariable("remote_server_port").toInt() << ") - " << gEnv->m_ServerStatus.m_RemoteServerStatus.toStdString().c_str();
		qInfo() << "Remote admin - " << remoteAdminStatus.toStdString().c_str();
		qInfo() << "Game servers :" << gEnv->pRemoteServer->GetClientCount() << "/" << gEnv->pRemoteServer->GetMaxClientCount();
		
		// Databases mode
		qInfo() << "Database mode :" << gEnv->m_ServerStatus.m_DBMode.toStdString().c_str();

		// Databases status
		qInfo() << "Database status :" << gEnv->m_ServerStatus.m_DBStatus.toStdString().c_str();

		// Authorization type
		qInfo() << "Authorization type :" << gEnv->pSettings->GetVariable("auth_mode").toString().toStdString().c_str();
	}
	else if (input.contains("send_message")) // TODO
	{
		QString message = input.remove("send_message ");

		qInfo() << "Try send message to all clients <" << message.toStdString().c_str() << ">";		
	}
	else if (input.contains("send_command")) // TODO
	{
		QString command = input.remove("send_command ");

		qInfo() << "Try send command to all clients <" << command.toStdString().c_str() << ">";
	}
	else if (input == "players")
	{
		QStringList players = gEnv->pServer->GetPlayersList();

		if (players.size() <= 0)
		{
			qWarning() << "No available  players for display";
			return;
		}

		for (int i = 0; i < players.size(); i++)
		{
			qInfo() << players[i];
		}
	}
	else if (input == "servers")
	{
		QStringList servers = gEnv->pRemoteServer->GetServerList();

		if (servers.size() <= 0)
		{
			qWarning() << "No available  server for display";
			return;
		}

		for (int i = 0; i < servers.size(); i++)
		{
			qInfo() << servers[i];
		}
	}
	else if (input == "clear")
	{
		ClearOutput();
	}
	else if (input == "quit")
	{
		CleanUp();
	}
	else
	{
		QStringList keys = input.split(" ");

		// Get variable description
		if (keys.size() > 0 && keys[0].contains("?"))
		{
			qInfo() << keys[0].toStdString().c_str();
			QString key = keys[0].remove("?");
			QString description = gEnv->pSettings->GetDescription(key);

			if (!description.isEmpty())
				qInfo() << key.toStdString().c_str() << "-" << description.toStdString().c_str();

			return;
		}

		// Find variable and get type, value or set new value
		if (keys.size() > 0 && gEnv->pSettings->CheckVariableExists(keys[0]))
		{
			if (keys.size() == 1)
			{
				// Get variable type and value
				QVariant::Type var_type = gEnv->pSettings->GetVariable(keys[0]).type();

				switch (var_type)
				{
				case QVariant::Bool:
				{
					qInfo() << keys[0].toStdString().c_str() << "=" << gEnv->pSettings->GetVariable(keys[0]).toBool();
					break;
				}
				case QVariant::Int:
				{
					qInfo() << keys[0].toStdString().c_str() << "=" << gEnv->pSettings->GetVariable(keys[0]).toInt();
					break;
				}
				case QVariant::Double:
				{
					qInfo() << keys[0].toStdString().c_str() << "=" << gEnv->pSettings->GetVariable(keys[0]).toDouble();
					break;
				}
				case QVariant::String:
				{
					qInfo() << keys[0].toStdString().c_str() << "=" << gEnv->pSettings->GetVariable(keys[0]).toString().toStdString().c_str();
					break;
				}
				default:
					break;
				}
			}
			else if (keys.size() == 2) // Set new value for variable
			{
				qInfo() << keys[0].toStdString().c_str() << "=" << keys[1].toStdString().c_str();

				QVariant::Type var_type = gEnv->pSettings->GetVariable(keys[0]).type();

				switch (var_type)
				{
				case QVariant::Bool:
				{
					bool value = (keys[1].toInt() == 1) ? true : false;
					gEnv->pSettings->SetVariable(keys[0], value);
					break;
				}
				case QVariant::Int:
				{
					int value = keys[1].toInt();
					gEnv->pSettings->SetVariable(keys[0], value);
					break;
				}
				case QVariant::Double:
				{
					double value = keys[1].toDouble();
					gEnv->pSettings->SetVariable(keys[0], value);
					break;
				}
				case QVariant::String:
				{
					QString value = keys[1];
					gEnv->pSettings->SetVariable(keys[0], value);
					break;
				}
				default:
					break;
				}
			}
		}
		else 
		{
			qWarning() << "Unknown console command or variable" << input;
		}		
	}
}
