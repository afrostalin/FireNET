#include <QCoreApplication>
#include <QDebug>
#include <QTextStream>
#include <QThread>

#include "global.h"
#include "client.h"
#include "inputlistener.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    qDebug() << "Remote administration panel for FireNET";
    qDebug() << "Created by Ilya Chernetsov";
    qDebug() << "Copyright (c) All rights reserved";
    qDebug() << "Use 'list' command to get full command list";

    QThread *pListenerThread = new QThread();
    gEnv->pListener = new InputListener();
    gEnv->pListener->moveToThread(pListenerThread);
    QObject::connect(pListenerThread, &QThread::started, gEnv->pListener, &InputListener::Run);
    QObject::connect(pListenerThread, &QThread::finished, gEnv->pListener, &InputListener::deleteLater);
    pListenerThread->start();

    return a.exec();
}
