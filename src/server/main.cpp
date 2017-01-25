// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QApplication>
#include <QThread>

#include "global.h"
#include "serverThread.h"

#include "UI/mainwindow.h"

QApplication *pApp;

int main(int argc, char *argv[])
{
    pApp = new QApplication(argc, argv);

	// Server buid version, number and type
	QString buildVersion = "v.2.1.1";
	int buildNumber = 71;
	QString appVersion = buildVersion + "." + QString::number(buildNumber);
#ifdef QT_NO_DEBUG
	QString buildType = ". Release profile";
#else
	QString buildType = ". Debug profile";
#endif 

	// Config QAppligcation
	pApp->addLibraryPath("plugins");
	pApp->setApplicationName("FireNET");
	pApp->setApplicationVersion(appVersion);
	
	// Init and show UI
	gEnv->pUI = new MainWindow();
	gEnv->pUI->show();
	gEnv->m_serverFullName = "FireNET " + buildVersion + ". Build " + QString::number(buildNumber) + buildType;

	// Connect quit signal with clean up slot
	QObject::connect(pApp, &QApplication::aboutToQuit, gEnv->pUI, &MainWindow::CleanUp);

	// Start server thread
	QThread* m_Thread = new QThread();
	CServerThread* pServerThread = new CServerThread();
	pServerThread->moveToThread(m_Thread);
	QObject::connect(m_Thread, &QThread::started, pServerThread, &CServerThread::start);
	QObject::connect(gEnv->pUI, &MainWindow::stop, pServerThread, &CServerThread::stop);
	QObject::connect(m_Thread, &QThread::finished, pServerThread, &CServerThread::deleteLater);
	m_Thread->start();

	return pApp->exec();
}
