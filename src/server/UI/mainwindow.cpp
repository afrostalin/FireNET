// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "UILogger.h"

#include "Core/tcpserver.h"
#include "Core/remoteserver.h"

#include "Tools/console.h"

MainWindow::MainWindow(QWidget *parent) 
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	ui->Output->setTextElideMode(Qt::ElideNone);
	ui->Output->setResizeMode(QListView::Adjust);

	m_OutputItemID = -1;

	connect(&m_UpdateTimer, &QTimer::timeout, this, &MainWindow::UpdateServerStatus);
	connect(this, &MainWindow::scroll, ui->Output, &QListWidget::scrollToBottom);

	m_UpdateTimer.start(250);
}

MainWindow::~MainWindow()
{
	SAFE_DELETE(ui);
}

void MainWindow::LogToOutput(ELogType type, const QString& msg)
{
	if (!ui || !ui->Output)
		return;

	QMutexLocker locker(&m_Mutex);

	// If log level 0 or 1 - We not need use scroll and save all lines
	if (m_OutputItemID >= 24 && gEnv->m_UILogLevel < 2)
	{
		try
		{
			auto pItem = ui->Output->item(0);

			if (pItem)
			{
				SAFE_DELETE(pItem);
				m_OutputItemID--;
			}
		}
		catch (const std::exception&)
		{

		}
	}

	ui->Output->addItem(msg);
	m_OutputItemID++;

	switch (type)
	{
	case ELog_Debug:
	{
		try
		{
			auto pItem = ui->Output->item(m_OutputItemID);

			if (pItem)
			{
				pItem->setForeground(Qt::darkGreen);
			}
		}
		catch (const std::exception&)
		{

		}	

		break;
	}
	case ELog_Info:
	{
		try
		{
			auto pItem = ui->Output->item(m_OutputItemID);

			if (pItem)
			{
				pItem->setForeground(Qt::white);
			}
		}
		catch (const std::exception&)
		{

		}	

		break;
	}
	case ELog_Warning:
	{
		try
		{
			auto pItem = ui->Output->item(m_OutputItemID);

			if (pItem)
			{
				pItem->setForeground(Qt::darkYellow);
			}
		}
		catch (const std::exception&)
		{

		}		

		break;
	}
	case ELog_Error:
	{
		try
		{
			auto pItem = ui->Output->item(m_OutputItemID);

			if (pItem)
			{
				pItem->setForeground(Qt::red);
			}
		}
		catch (const std::exception&)
		{

		}

		break;
	}
	default:
		break;
	};

	if (gEnv && !gEnv->isQuiting && gEnv->m_UILogLevel >= 2)
	{
		emit scroll();
	}
}

void MainWindow::UpdateServerStatus() const
{
	if (gEnv->isQuiting || !ui || !ui->Status)
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

	QString status = _strFormat("MServer : %s | MClients : %d/%d | RServer : %s | RClients : %d/%d | DBMode : %s | DBStatus : %s | IPackets : %d | OPackets : %d | ISpeed : %d | OSpeed : %d",
		gEnv->m_ServerStatus.m_MainServerStatus.toStdString().c_str(),
		clientCount, maxClientCount,
		gEnv->m_ServerStatus.m_RemoteServerStatus.toStdString().c_str(), 
		gsCount, maxGsCount,
		gEnv->m_ServerStatus.m_DBMode.toStdString().c_str(),
		gEnv->m_ServerStatus.m_DBStatus.toStdString().c_str(),
		gEnv->m_InputPacketsCount,
		gEnv->m_OutputPacketsCount,
		gEnv->m_InputSpeed,
		gEnv->m_OutputSpeed).c_str();

	ui->Status->addItem(status);

	auto pItem = ui->Status->item(0);

	if (pItem)
		pItem->setForeground(Qt::darkCyan);
}

void MainWindow::EnableStressMode()
{
	ClearOutput();
	LogWarning("YOU ENABLED STRESS TEST MODE, RESTART SERVER IF YOU NEED DISABLE IT");
}

void MainWindow::CleanUp()
{
	emit stop();
	m_UpdateTimer.stop();
	
	// Wait server thread here
	while (!gEnv->isReadyToClose)
	{
		QEventLoop loop;
		QTimer::singleShot(100, &loop, &QEventLoop::quit);
		loop.exec();
	}

	ClearOutput();
	ClearStatus();

	QCoreApplication::quit();
}

void MainWindow::ClearOutput()
{
	QMutexLocker locker(&m_Mutex);

	if (ui && ui->Output)
	{
		ui->Output->clear();
		m_OutputItemID = -1;
	}
}

void MainWindow::ClearStatus() const
{
	if (ui && ui->Status)
		ui->Status->clear();
}

void MainWindow::on_Input_returnPressed()
{
	QString input = ui->Input->text();
	ui->Input->clear();

	CConsole* pConsole = gEnv->pConsole;
	if (pConsole == nullptr)
	{
		LogError("Can't get console!");
		return;
	}


	if (!pConsole->ExecuteCommand(input))
	{
		QStringList keys = input.split(" ");

		// Get variable description
		if (keys.size() > 0 && keys[0].contains("?"))
		{
			LogInfo("%s", keys[0].toStdString().c_str());

			QString key = keys[0].remove("?");
			QString description = gEnv->pConsole->GetDescription(key);

			if (!description.isEmpty())
			{
				LogInfo("%s - %s", key.toStdString().c_str(), description.toStdString().c_str());
			}
			else
			{
				if (!gEnv->pConsole->FindVariabelMatches(key))
				{
					LogWarning("No one similar variable is found");
				}
			}

			return;
		}

		// Find variable and get type, value or set new value
		if (keys.size() > 0 && gEnv->pConsole->CheckVariableExists(keys[0]))
		{
			if (keys.size() == 1)
			{
				// Get variable type and value
				QVariant::Type var_type = gEnv->pConsole->GetVariable(keys[0]).type();

				switch (var_type)
				{
				case QVariant::Bool:
				{
					LogInfo("%s = %s", keys[0].toStdString().c_str(), gEnv->pConsole->GetBool(keys[0].toStdString().c_str()) ? "true" : "false");
					break;
				}
				case QVariant::Int:
				{
					LogInfo("%s = %d", keys[0].toStdString().c_str(), gEnv->pConsole->GetInt(keys[0].toStdString().c_str()));
					break;
				}
				case QVariant::Double:
				{
					LogInfo("%s = %f", keys[0].toStdString().c_str(), gEnv->pConsole->GetDouble(keys[0].toStdString().c_str()));
					break;
				}
				case QVariant::String:
				{
					LogInfo("%s = %s", keys[0].toStdString().c_str(), gEnv->pConsole->GetString(keys[0].toStdString().c_str()).c_str());
					break;
				}
				default:
					break;
				}
			}
			else if (keys.size() == 2 && !keys[1].isEmpty()) // Set new value for variable
			{
				LogInfo("%s = %s", keys[0].toStdString().c_str(), keys[1].toStdString().c_str());

				QVariant::Type var_type = gEnv->pConsole->GetVariable(keys[0]).type();

				switch (var_type)
				{
				case QVariant::Bool:
				{
					bool value = (keys[1].toInt() == 1) ? true : false;
					gEnv->pConsole->SetVariable(keys[0], value);
					break;
				}
				case QVariant::Int:
				{
					int value = keys[1].toInt();
					gEnv->pConsole->SetVariable(keys[0], value);
					break;
				}
				case QVariant::Double:
				{
					double value = keys[1].toDouble();
					gEnv->pConsole->SetVariable(keys[0], value);
					break;
				}
				case QVariant::String:
				{
					QString value = keys[1];
					gEnv->pConsole->SetVariable(keys[0], value);
					break;
				}
				default:
					break;
				}
			}
		}
		else 
		{
			LogWarning("Unknown console command or variable <%s>", input.toStdString().c_str());
		}		
	}
}
