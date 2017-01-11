#ifndef WORKER_H
#define WORKER_H

#include <QObject>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = 0);
public:
    void StartWork();
signals:

public slots:
};

#endif // WORKER_H
