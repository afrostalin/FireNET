// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef SERVERQUEUE_H
#define SERVERQUEUE_H

#include <QObject>

class ServerQueue : public QObject
{
    Q_OBJECT
public:
    explicit ServerQueue(QObject *parent = 0);
	~ServerQueue();

public slots:
    // Server Update functions, executed by server tick rate count
    virtual void Update();
};

#endif // SERVERQUEUE_H