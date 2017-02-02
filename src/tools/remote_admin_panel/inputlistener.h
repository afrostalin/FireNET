#ifndef INPUTLISTENER_H
#define INPUTLISTENER_H

#include <QObject>

class InputListener : public QObject
{
    Q_OBJECT
public:
    explicit InputListener(QObject *parent = 0);
public slots:
    void Run();
public:
    void StartListen();
};

#endif // INPUTLISTENER_H
