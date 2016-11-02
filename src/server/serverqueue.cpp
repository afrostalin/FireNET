#include <QDebug>
#include <QThread>
#include "serverqueue.h"
#include "global.h"

ServerQueue::ServerQueue(QObject *parent) : QObject(parent)
{

}

ServerQueue::~ServerQueue()
{
}

void  ServerQueue::Update()
{
	//qDebug() << "Update from" << QThread::currentThread();
}