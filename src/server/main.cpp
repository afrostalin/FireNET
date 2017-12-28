// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
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
	const QString buildVersion = "v.2.1.6";
	int buildNumber = 9;
	const QString appVersion = buildVersion + "." + QString::number(buildNumber);

	gEnv->m_buildVersion = buildVersion;
	gEnv->m_buildNumber = buildNumber;
#ifdef QT_NO_DEBUG
	const QString buildType = ". Release profile";
	gEnv->m_buildType = "Release";
#else
	const QString buildType = ". Debug profile";
	gEnv->m_buildType = "Debug";
#endif 

	// Config QApplication
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

	gEnv->pMainThread = pServerThread;
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
