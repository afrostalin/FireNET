#include <QCoreApplication>
#include "global.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    gEnv->m_patchList = argv[1];

    if(!gEnv->m_patchList.isEmpty())
    {
        qDebug() << "[Main] Patch list" << gEnv->m_patchList;

        gEnv->pWorker = new Worker;
        gEnv->pWorker->StartWork();
    }
    else
        qCritical() << "Wrong arguments!";

    return a.exec();
}
