// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QApplication>
#include <QThread>

#include "global.h"
#include "serverThread.h"

#include "UI/mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication* pApp = new QApplication(argc, argv);

	// Server buid version, number and type
	QString buildVersion = "v.2.1.5";
	int buildNumber = 25;
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

#ifndef NO_UI
	// Init and show UI
	gEnv->pUI = new MainWindow();
	gEnv->pUI->show();
#endif
	gEnv->m_serverFullName = "FireNET " + buildVersion + ". Build " + QString::number(buildNumber) + buildType;

#ifndef NO_UI
	// Connect quit signal with clean up slot
	QObject::connect(pApp, &QApplication::aboutToQuit, gEnv->pUI, &MainWindow::CleanUp);
#endif

	// Start server thread
	QThread* m_Thread = new QThread();
	CServerThread* pServerThread = new CServerThread();
	pServerThread->moveToThread(m_Thread);
#ifndef NO_UI
	QObject::connect(pServerThread, &CServerThread::EnableStressMode, gEnv->pUI, &MainWindow::EnableStressMode);
#endif
	QObject::connect(m_Thread, &QThread::started, pServerThread, &CServerThread::start);
#ifndef NO_UI
	QObject::connect(gEnv->pUI, &MainWindow::stop, pServerThread, &CServerThread::stop);
#endif
	QObject::connect(m_Thread, &QThread::finished, pServerThread, &CServerThread::deleteLater);
	m_Thread->start();

	return pApp->exec();
}
