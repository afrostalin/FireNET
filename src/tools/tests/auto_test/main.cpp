#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

#include "tcpclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTimer* pTimer = new QTimer();

    qDebug() << "Starting auto test...";

    QVector<tcpclient*> m_clients;

    for(int i = 0; i < 10; i++)
    {
        tcpclient* pClient = new tcpclient();
        QObject::connect(pTimer, &QTimer::timeout, pClient, &tcpclient::update);

        pClient->CreateClient();
        pClient->CreateTestList();

        m_clients.push_back(pClient);
    }

    pTimer->start(10);

    return a.exec();
}
